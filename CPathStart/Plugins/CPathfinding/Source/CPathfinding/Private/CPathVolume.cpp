// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathVolume.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"



// Sets default values
ACPathVolume::ACPathVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VolumeBox = CreateDefaultSubobject<UBoxComponent>("VolumeBox");
	RootComponent = VolumeBox;
}


const FVector ACPathVolume::ChildPositionOffsetMaskByIndex[8] =
{	{-1, -1, -1},
	{-1, 1, -1},
	{-1, -1, 1},
	{-1, 1, 1},

	{1, -1, -1},
	{1, 1, -1},
	{1, -1, 1},
	{1, 1, 1} };

FCPathNode& ACPathVolume::GetNodeFromPosition(FVector WorldLocation, bool& IsValid)
{
	FVector XYZ = GetXYZFromPosition(WorldLocation);
	IsValid = IsInBounds(XYZ);
	
	if(IsValid)
	{
		return Nodes[GetIndexFromXYZ(XYZ)];	
	}

	return Nodes[0];

}

// Called when the game starts or when spawned
void ACPathVolume::BeginPlay()
{
	Super::BeginPlay();
	UBoxComponent* tempBox = Cast<UBoxComponent>(GetRootComponent());


	/*NodeCount[0] = VolumeBox->GetScaledBoxExtent().X * 2 / VoxelSize + 1;
	NodeCount[1] = VolumeBox->GetScaledBoxExtent().Y*2 / VoxelSize + 1;
	NodeCount[2] = VolumeBox->GetScaledBoxExtent().Z*2 / VoxelSize + 1;*/

	float Divider = VoxelSize * FMath::Pow(2, OctreeDepth);

	NodeCount[0] = FMath::CeilToInt(VolumeBox->GetScaledBoxExtent().X * 2.0 / Divider);
	NodeCount[1] = FMath::CeilToInt(VolumeBox->GetScaledBoxExtent().Y * 2.0 / Divider);
	NodeCount[2] = FMath::CeilToInt(VolumeBox->GetScaledBoxExtent().Z * 2.0 / Divider);

	checkf(OctreeDepth < 16 && OctreeDepth > 0, TEXT("CPATH - Graph Generation:::OctreeDepth must be within 0 and 15"));

	for (int i = 0; i <= OctreeDepth; i++)
	{
		VoxelCountAtDepth.Add(0);
	}

	while (DepthsToDraw.Num() <= OctreeDepth)
	{
		DepthsToDraw.Add(0);
	}


	checkf((uint32)(NodeCount[0] * NodeCount[1] * NodeCount[2]) < DEPTH_0_LIMIT, TEXT("CPATH - Graph Generation:::Depth 0 is too dense, increase OctreeDepth and/or voxel size"));
	


	UE_LOG(LogTemp, Warning, TEXT("Count, %d %d %d"), NodeCount[0], NodeCount[1], NodeCount[2] );
	GenerateGraph();
	
}

void ACPathVolume::GenerateGraph()
{

	GenerationStart = TIMENOW;
	PrintGenerationTime = true;

	float CurrVoxelSize = GetVoxelSizeByDepth(0);

	StartPosition = GetActorLocation() - VolumeBox->GetScaledBoxExtent() + CurrVoxelSize /2;
	uint32 Index = 0;

	

	//Reserving memory in array before adding elements
	Octrees = new CPathOctree[(NodeCount[0] * NodeCount[1] * NodeCount[2])];
	TraceHandles.Reserve(NodeCount[0] * NodeCount[1] * NodeCount[2]);
	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(CurrVoxelSize / 2));

	delete(TraceHandlesCurr);
	delete(TraceHandlesNext);

	TraceHandlesCurr = new std::vector<FTraceHandle>;
	TraceHandlesNext = new std::vector<FTraceHandle>;
	

	for (uint32 x = 0; x < NodeCount[0]; x++)
	{
		for (uint32 y = 0; y < NodeCount[1]; y++)
		{
			for (uint32 z = 0; z < NodeCount[2]; z++)
			{
				FVector Location = FVector(StartPosition + FVector(x, y, z) * CurrVoxelSize);
				//Nodes.Add(FCPathNode(Index, Location));

				TraceHandlesCurr->push_back(GetWorld()->AsyncOverlapByChannel(
					Location,
					FQuat(FRotator::ZeroRotator),
					TraceChannel,
					TraceBox,
					FCollisionQueryParams::DefaultQueryParam,
					FCollisionResponseParams::DefaultResponseParam,
					nullptr, CreateTreeID(Index, 0)));
				
				Index++;
			}
		}
	}
	
}


void ACPathVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto TickStart = TIMENOW;
	//uint32 TraceCount = TraceHandles.Num();
	uint32 TraceCount = TraceHandlesCurr->size();
	
	int TracesHandled = 0;
	int TracesIncomplete = 0;

	int TracesAdded = 0;

	if(TraceCount > 0)
	{
		uint32 Index, Depth;

		
		for(uint32 i = 0; i < TraceCount; i++)
		{ 
			FOverlapDatum OverlapDatum;
			if(GetWorld()->QueryOverlapData(TraceHandlesCurr->operator[](i), OverlapDatum))
			{
				//TraceHandles.RemoveAtSwap<int>(i, 1, false);
				//TraceHandles.RemoveAt(i);
				//TraceHandles.RemoveAtSwap(i);

				Index = GetOuterIndex(OverlapDatum.UserData);
				Depth = GetDepth(OverlapDatum.UserData);
				VoxelCountAtDepth[Depth]++;
				
				uint32 DepthReached;
				CPathOctree* TracedTree =  GetOctreeFromID(OverlapDatum.UserData, DepthReached);

				checkf(DepthReached == Depth, TEXT("CPATH - Graph Generation:::Tracing - DepthReached not equal Depth passed in UserData"));

				TracedTree->IsFree = OverlapDatum.OutOverlaps.Num() == 0;

				if (Depth < (uint32)OctreeDepth)
				{
					if (!TracedTree->IsFree)
					{
						ExpandOctree(TracedTree, OverlapDatum.UserData, OverlapDatum.Pos);
						TracesAdded += 8;
					}
				}

				
				if(OverlapDatum.OutOverlaps.Num() > 0)
				{
					if(DepthsToDraw[Depth])
						DrawDebugBox(GetWorld(), GetWorldPositionFromTreeID(OverlapDatum.UserData), FVector(GetVoxelSizeByDepth(Depth) /2), FColor::Red, true, 10);
				}
				else
				{
					if (DepthsToDraw[Depth])
						DrawDebugBox(GetWorld(), GetWorldPositionFromTreeID(OverlapDatum.UserData), FVector(GetVoxelSizeByDepth(Depth) / 2), FColor::Green, true, 10);
					//UpdateNeighbours(Nodes[OverlapDatum.UserData]);
				}
				
				//i--;
				//TraceCount--;
				TracesHandled++;
			}
			else
			{
				TraceHandlesNext->push_back(TraceHandlesCurr->operator[](i));
				TracesIncomplete++;
			}
		}
		//if (TracesIncomplete > 10)
		//{
			//TracesIncomplete++;
			//UE_LOG(LogTemp, Warning, TEXT("KURWA CO JEST - %d"), TracesIncomplete);
		//}
		UE_LOG(LogTemp, Warning, TEXT("Traces - %d    incomplete - %d    ThisTickTime - %lf ms"), TracesHandled, TracesIncomplete, TIMEDIFF(TickStart, TIMENOW));
	}
	else
	{
		if (PrintGenerationTime)
		{
			auto b = TIMEDIFF(GenerationStart, TIMENOW);
			UE_LOG(LogTemp, Warning, TEXT("Total generation time - %lf ms"), b);
			PrintGenerationTime = false;
		}
	}


	// Clearing used handles, swapping next handles with current
	std::vector<FTraceHandle>* TempVector = TraceHandlesCurr;
	TraceHandlesCurr->clear();
	TraceHandlesCurr = TraceHandlesNext;
	TraceHandlesNext = TempVector;
}

void ACPathVolume::BeginDestroy()
{
	Super::BeginDestroy();

	delete(Octrees);
}

void ACPathVolume::ExpandOctree(CPathOctree* TreeToExpand, uint32 CurrentTreeID, FVector TreeLocation)
{

	uint32 NewDepth = GetDepth(CurrentTreeID) + 1;

	TreeToExpand->Children = new CPathOctree[8];

	float CurrHalfSize = GetVoxelSizeByDepth(NewDepth) / 2.f;

	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(CurrHalfSize));
	FCollisionShape TraceCapsule = FCollisionShape::MakeCapsule(FVector(CurrHalfSize) * 4);

	// Generating children
	for (uint32 ChildIndex = 0; ChildIndex < 8; ChildIndex++)
	{

		FVector Location = TreeLocation + ChildPositionOffsetMaskByIndex[ChildIndex] * CurrHalfSize;

		//TreeToExpand->Children[ChildIndex].Depth = NewDepth;
		uint32 TreeID = CurrentTreeID;

		SetDepth(TreeID, NewDepth);
		SetChildIndex(TreeID, NewDepth, ChildIndex);

		
		

		TraceHandlesNext->push_back(GetWorld()->AsyncOverlapByChannel(
			Location,
			FQuat(FRotator::ZeroRotator),
			TraceChannel,
			TraceBox,
			FCollisionQueryParams::DefaultQueryParam,
			FCollisionResponseParams::DefaultResponseParam,
			nullptr, TreeID));

		/*for (int i = 0; i < AdditionalTraces; i++)
		{
			GetWorld()->OverlapAnyTestByChannel(Location,
				FQuat(FRotator::ZeroRotator),
				TraceChannel,
				TraceCapsule,
				FCollisionQueryParams::DefaultQueryParam,
				FCollisionResponseParams::DefaultResponseParam);
		}*/
	}
}

void ACPathVolume::UpdateNeighbours(FCPathNode& Node)
{
	TArray<int> Indexes = GetAdjacentIndexes(Node.Index);
	for (int Index : Indexes)
	{
		if(Nodes[Index].IsFree)
		{
			Nodes[Index].FreeNeighbors.Add(&Nodes[Node.Index]);
			Node.FreeNeighbors.Add(&Nodes[Index]);
			if(DrawConnections)
				DrawDebugLine(GetWorld(), Node.WorldLocation, Nodes[Index].WorldLocation, FColor::Orange, true, 10);
		}
	}
	
}


TArray<int> ACPathVolume::GetAdjacentIndexes(int Index) const
{
	FVector XYZ = GetXYZFromPosition(Nodes[Index].WorldLocation);

	TArray<int> Indexes;
	Indexes.Reserve(6);
	
	for(int Comp = 0; Comp < 3; Comp++)
	{
		for(int Sign = -1; Sign < 2; Sign += 2)
		{
			FVector NewXYZ = XYZ;
			NewXYZ[Comp] += Sign;
			if(IsInBounds(NewXYZ))
			{
				Indexes.Add(GetIndexFromXYZ(NewXYZ));
			}

		}
	}
	
	return Indexes;
}

FVector ACPathVolume::GetXYZFromPosition(FVector WorldLocation) const
{
	FVector RelativePos = WorldLocation - StartPosition;
	RelativePos = RelativePos / GetVoxelSizeByDepth(0);
	return FVector(FMath::RoundToFloat(RelativePos.X),
		FMath::RoundToFloat(RelativePos.Y),
		FMath::RoundToFloat(RelativePos.Z));
	
}

int ACPathVolume::GetIndexFromPosition(FVector WorldLocation) const
{
	FVector XYZ = GetXYZFromPosition(WorldLocation);
	return GetIndexFromXYZ(XYZ);
}

bool ACPathVolume::IsInBounds(FVector XYZ) const
{
	if(XYZ.X < 0 || XYZ.X >= NodeCount[0])
		return false;

	if(XYZ.Y < 0 || XYZ.Y >= NodeCount[1])
		return false;

	if(XYZ.Z < 0 || XYZ.Z >= NodeCount[2])
		return false;

	return true;
}

float ACPathVolume::GetIndexFromXYZ(FVector V) const
{
	return (V.X * (NodeCount[1] * NodeCount[2])) + (V.Y * NodeCount[2]) + V.Z;
}

inline float ACPathVolume::GetVoxelSizeByDepth(int Depth) const
{
	return VoxelSize * FMath::Pow(2, OctreeDepth - Depth);
}



inline uint32 ACPathVolume::CreateTreeID(uint32 Index, uint32 Depth) const
{
#if WITH_EDITOR
	checkf(Depth < 5, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif

	// 8 bits last are still free, index could probably also be lower than 16 so even more space
	Index |= Depth << 16;

	return Index;
}

inline uint32 ACPathVolume::GetOuterIndex(uint32 TreeID) const
{
	return TreeID & 0x0000FFFF;
}

inline void ACPathVolume::SetDepth(uint32& TreeID, uint32 NewDepth)
{
#if WITH_EDITOR
	checkf(NewDepth < 5, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif

	TreeID &= 0xFFF8FFFF;
	TreeID |= NewDepth << 16;
}


inline uint32 ACPathVolume::GetDepth(uint32 TreeID) const
{
	return (TreeID & 0x00070000) >> 16;
}

inline uint32 ACPathVolume::GetChildIndex(uint32 TreeID, uint32 Depth) const
{
#if WITH_EDITOR
	checkf(Depth < 5 && Depth > 0, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif
	uint32 DepthOffset = Depth * 3 + 19;
	uint32 Mask = 0x00000007 << DepthOffset;

	return (TreeID & Mask) >> DepthOffset;
}

inline void ACPathVolume::SetChildIndex(uint32& TreeID, uint32 Depth, uint32 ChildIndex)
{
#if WITH_EDITOR
	checkf(Depth < 5 && Depth > 0, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
	checkf(ChildIndex < 8, TEXT("CPATH - Graph Generation:::Child Index can be up to 7"));
#endif
	
	ChildIndex <<= Depth * 3 + 19;

	TreeID |= ChildIndex;
}

FVector ACPathVolume::GetWorldPositionFromTreeID(uint32 TreeID) const
{
	uint32 OuterIndex = GetOuterIndex(TreeID);
	uint32 Depth = GetDepth(TreeID);

	uint32 X = OuterIndex / (NodeCount[1] * NodeCount[2]);
	OuterIndex -= X * NodeCount[1] * NodeCount[2];
	uint32 Y = OuterIndex / NodeCount[2];
	uint32 Z = OuterIndex % NodeCount[2];

	FVector CurrPosition = StartPosition + GetVoxelSizeByDepth(0) * FVector(X, Y, Z);


	for (uint32 CurrDepth = 1; CurrDepth <= Depth; CurrDepth++)
	{
		CurrPosition += GetVoxelSizeByDepth(CurrDepth) * 0.5f * ChildPositionOffsetMaskByIndex[GetChildIndex(TreeID, CurrDepth)];
	}

	return CurrPosition;
}

inline void ACPathVolume::ReplaceChildIndex(uint32& TreeID, uint32 Depth, uint32 ChildIndex)
{
#if WITH_EDITOR
	checkf(Depth < 5 && Depth > 0, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
	checkf(ChildIndex < 8, TEXT("CPATH - Graph Generation:::Child Index can be up to 7"));
#endif

	uint32 DepthOffset = Depth * 3 + 19;

	// Clearing previous child index
	TreeID &= ~(0x00000007 << DepthOffset);

	ChildIndex <<= DepthOffset;
	TreeID |= ChildIndex;
}

CPathOctree* ACPathVolume::GetOctreeFromID(uint32 TreeID, uint32& DepthReached)
{
	uint32 Depth = GetDepth(TreeID);
	CPathOctree* CurrTree = &Octrees[GetOuterIndex(TreeID)];
	DepthReached = 0;

	for (uint32 CurrDepth = 1; CurrDepth <= Depth; CurrDepth++)
	{
		// Child not found, returning the deepest found parent
		if (!CurrTree->Children)
		{
			break;
		}

		CurrTree = &CurrTree->Children[GetChildIndex(TreeID, CurrDepth)];
		DepthReached = CurrDepth;
	}
	return CurrTree;
}

