// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAsyncVolumeGeneration.h"
#include "CPathVolume.h"
#include <thread>

FCPathAsyncVolumeGenerator::FCPathAsyncVolumeGenerator(ACPathVolume* Volume, uint32 StartIndex, uint32 EndIndex, bool Obstacles)
	:
	FCPathAsyncVolumeGenerator(Volume)
{
	bObstacles = Obstacles;
	FirstIndex = StartIndex;
	LastIndex = EndIndex;
	UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR, %d %d"), LastIndex, FirstIndex);
}

// Sets default values
FCPathAsyncVolumeGenerator::FCPathAsyncVolumeGenerator(ACPathVolume* Volume)
{
	VolumeRef = Volume;

}

FCPathAsyncVolumeGenerator::~FCPathAsyncVolumeGenerator()
{
	UE_LOG(LogTemp, Warning, TEXT("Thread %d - Destroying"), LastIndex);
	bStop = true;
	if (ThreadRef)
		ThreadRef->Kill(true);
}

bool FCPathAsyncVolumeGenerator::Init()
{
	return true;
}

uint32 FCPathAsyncVolumeGenerator::Run()
{
	bIncreasedGenRunning = true;
	VolumeRef->GeneratorsRunning++;
	
	// Waiting for pathfinders to finish.
	// Generators have priority over pathfinders, so we block further pathfinders from starting by incrementing GeneratorsRunning first
	while (VolumeRef->PathfindersRunning.load() > 0 && !bStop)
		std::this_thread::sleep_for(std::chrono::milliseconds(25));

	auto GenerationStart = TIMENOW;


	if (LastIndex > 0)
	{
		if (bObstacles)
		{
			auto StartIter = VolumeRef->TreesToRegenerate.begin();
			for (uint32 i = 0; i < FirstIndex; i++)
				StartIter++;

			auto EndIter = StartIter;
			for (uint32 i = FirstIndex; i < LastIndex; i++)
				EndIter++;

			for (auto Iter = StartIter; Iter != EndIter && !bStop; Iter++)
			{
				RefreshTree(*Iter);
			}
		}
		else
		{
			for (uint32 OuterIndex = FirstIndex; OuterIndex < LastIndex && !bStop; OuterIndex++)
			{
				RefreshTree(OuterIndex);
			}
		}
	}

	auto b = TIMEDIFF(GenerationStart, TIMENOW);

	//UE_LOG(LogTemp, Warning, TEXT("Thread %d - bStop= %d"), LastIndex, bStop);

	if (!bStop)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Thread %d - %lf ms"), LastIndex, b);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Thread %d - interrupted"), LastIndex);
	
	if (bIncreasedGenRunning)
		VolumeRef->GeneratorsRunning--;
	bIncreasedGenRunning = false;
	return 0;
}

void FCPathAsyncVolumeGenerator::Stop()
{
	
	// Preventing a potential deadlock if the process is killed without waiting
	if(bIncreasedGenRunning)
		VolumeRef->GeneratorsRunning--;

	bIncreasedGenRunning = false;
	//UE_LOG(LogTemp, Warning, TEXT("Generator %d stopped"), LastIndex);
}

void FCPathAsyncVolumeGenerator::Exit()
{
	//UE_LOG(LogTemp, Warning, TEXT("Generator %d Exit"), LastIndex);
	// Setting thread ref to null so that it can be collected by CleanFinishedThreads in volume
	ThreadRef = nullptr;
}

void FCPathAsyncVolumeGenerator::RefreshTree(uint32 OuterIndex)
{
	CPathOctree* OctreeRef = &VolumeRef->Octrees[OuterIndex];
	if (!OctreeRef)
	{
		return;
	}

	RefreshTreeRec(OctreeRef, 0, VolumeRef->WorldLocationFromTreeID(OuterIndex));
}

bool FCPathAsyncVolumeGenerator::RefreshTreeRec(CPathOctree* OctreeRef, uint32 Depth, FVector TreeLocation)
{

	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(VolumeRef->GetVoxelSizeByDepth(Depth) / 2.f));
	bool IsFree = !VolumeRef->GetWorld()->OverlapAnyTestByChannel(TreeLocation, FQuat(FRotator(0)), VolumeRef->TraceChannel, VolumeRef->TraceBoxByDepth[Depth]);
	OctreeRef->SetIsFree(IsFree);

	VolumeRef->VoxelCountAtDepth[Depth]++;

	if (IsFree)
	{
		delete[] OctreeRef->Children;
		OctreeRef->Children = nullptr;		
		return true;
	}
	else if (++Depth <= (uint32)VolumeRef->OctreeDepth)
	{
		float HalfSize = VolumeRef->GetVoxelSizeByDepth(Depth) / 2.f;

		if (!OctreeRef->Children)
			OctreeRef->Children = new CPathOctree[8];
		uint8 FreeChildren = 0;
		// Checking children
		for (uint32 ChildIndex = 0; ChildIndex < 8; ChildIndex++)
		{
			FVector Location = TreeLocation + VolumeRef->LookupTable_ChildPositionOffsetMaskByIndex[ChildIndex] * HalfSize;
			FreeChildren += RefreshTreeRec(&OctreeRef->Children[ChildIndex], Depth, Location);
		}

		if (FreeChildren)
		{
			return true;
		}
		else
		{
			delete[] OctreeRef->Children;
			OctreeRef->Children = nullptr;
			return false;
		}
		
	}
	return false;
}



void FCPathAsyncVolumeGenerator::GetIndexesToUpdateFromActor(AActor* Actor, ACPathVolume* Volume)
{
	FVector Origin, Extent;
	Actor->GetActorBounds(true, Origin, Extent);

	FVector XYZ = Volume->WorldLocationToLocalCoordsInt3(Origin);
	if (!Volume->IsInBounds(XYZ))
	{
		return;
	}
	uint32 Index = Volume->LocalCoordsInt3ToIndex(XYZ);
	FVector DistanceFromCenter = Origin - Volume->WorldLocationFromTreeID(Index);

	float VoxelSize = Volume->GetVoxelSizeByDepth(0);
	float VoxelExtent = VoxelSize / 2.f;

	// How many outer trees should we include in given direction
	int MaxOffsetInDirection[6];		
	MaxOffsetInDirection[Left] =	FMath::CeilToInt((Extent.Y - (VoxelExtent + DistanceFromCenter.Y)) / VoxelSize);
	MaxOffsetInDirection[Front] =	FMath::CeilToInt((Extent.X - (VoxelExtent + DistanceFromCenter.X)) / VoxelSize);
	MaxOffsetInDirection[Right] =	FMath::CeilToInt((Extent.Y - (VoxelExtent - DistanceFromCenter.Y)) / VoxelSize);
	MaxOffsetInDirection[Behind] =	FMath::CeilToInt((Extent.X - (VoxelExtent - DistanceFromCenter.X)) / VoxelSize);
	MaxOffsetInDirection[Below] =	FMath::CeilToInt((Extent.Z - (VoxelExtent + DistanceFromCenter.Z)) / VoxelSize);
	MaxOffsetInDirection[Above] =	FMath::CeilToInt((Extent.Z - (VoxelExtent - DistanceFromCenter.Z)) / VoxelSize);

	FVector Offset = FVector::ZeroVector;
	for (int X = -MaxOffsetInDirection[Front]; X <= MaxOffsetInDirection[Behind]; X++)
	{
		Offset.X = X;
		for (int Y = -MaxOffsetInDirection[Left]; Y <= MaxOffsetInDirection[Right]; Y++)
		{
			Offset.Y = Y;
			for (int Z = -MaxOffsetInDirection[Below]; Z <= MaxOffsetInDirection[Above]; Z++)
			{
				Offset.Z = Z;

				FVector CurrXYZ = XYZ + Offset;
				if (Volume->IsInBounds(CurrXYZ))
				{
					Index = Volume->LocalCoordsInt3ToIndex(CurrXYZ);
					Volume->TreesToRegenerate.insert(Index);
					Volume->TreesToRegeneratePreviousUpdate.insert(Index);
				}
			}
		}
	}	
}

