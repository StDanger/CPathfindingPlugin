// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAsyncVolumeGeneration.h"
#include "CPathVolume.h"

FCPathAsyncVolumeGenerator::FCPathAsyncVolumeGenerator(ACPathVolume* Volume, uint32 StartIndex, uint32 EndIndex)
	:
	FCPathAsyncVolumeGenerator(Volume)
{
	Start = StartIndex;
	End = EndIndex;
	UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR, %d %d"), End, Start);
}

// Sets default values
FCPathAsyncVolumeGenerator::FCPathAsyncVolumeGenerator(ACPathVolume* Volume)
{
	VolumeRef = Volume;

}

bool FCPathAsyncVolumeGenerator::Init()
{
	return true;
}

uint32 FCPathAsyncVolumeGenerator::Run()
{
	auto GenerationStart = TIMENOW;


	if (End > 0)
	{
		for (uint32 Index = Start; Index < End && !bStop; Index++)
		{
			RefreshTree(Index);
		}
	}

	auto b = TIMEDIFF(GenerationStart, TIMENOW);

	UE_LOG(LogTemp, Warning, TEXT("New trace time - %lf ms"), b);

	return 0;
}

void FCPathAsyncVolumeGenerator::Stop()
{
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

void FCPathAsyncVolumeGenerator::RefreshTreeRec(CPathOctree* OctreeRef, uint32 Depth, FVector TreeLocation)
{

	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(VolumeRef->GetVoxelSizeByDepth(Depth) / 2.f));
	bool IsFree = !VolumeRef->GetWorld()->OverlapAnyTestByChannel(TreeLocation, FQuat(FRotator(0)), VolumeRef->TraceChannel, VolumeRef->TraceBoxByDepth[Depth]);
	OctreeRef->SetIsFree(IsFree);

	VolumeRef->VoxelCountAtDepth[Depth]++;

	/*if (VolumeRef->DepthsToDraw[Depth])
	{
		if (IsFree && DrawFree)
			DrawDebugBox(GetWorld(), TreeLocation, FVector(GetVoxelSizeByDepth(Depth) / (2.f)), FColor::Green, true, 10);

		if (!IsFree && DrawOccupied)
			DrawDebugBox(GetWorld(), TreeLocation, FVector(GetVoxelSizeByDepth(Depth) / (2.f)), FColor::Red, true, 10);
	}*/


	if (IsFree)
	{
		return;
	}
	else if (++Depth <= (uint32)VolumeRef->OctreeDepth)
	{
		float HalfSize = VolumeRef->GetVoxelSizeByDepth(Depth) / 2.f;

		if (!OctreeRef->Children)
			OctreeRef->Children = new CPathOctree[8];

		// Checking children
		for (uint32 ChildIndex = 0; ChildIndex < 8; ChildIndex++)
		{
			FVector Location = TreeLocation + VolumeRef->LookupTable_ChildPositionOffsetMaskByIndex[ChildIndex] * HalfSize;
			RefreshTreeRec(&OctreeRef->Children[ChildIndex], Depth, Location);
		}
	}

}