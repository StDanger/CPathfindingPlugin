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
	{-1, -1, 1},
	{-1, 1, -1},
	{-1, 1, 1},
	{1, -1, -1},
	{1, -1, 1},
	{1, 1, -1},
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

	VoxelCountAtDepth[0] = NodeCount[0] * NodeCount[1] * NodeCount[2];

	checkf(VoxelCountAtDepth[0] < DEPTH_0_LIMIT, TEXT("CPATH - Graph Generation:::Depth 0 is too dense, increase OctreeDepth and/or voxel size"));
	


	UE_LOG(LogTemp, Warning, TEXT("Count, %d %d %d"), NodeCount[0], NodeCount[1], NodeCount[2] );
	GenerateGraph();
	
}

void ACPathVolume::GenerateGraph()
{
	float CurrVoxelSize = GetVoxelSizeByDepth(0);

	StartPosition = GetActorLocation() - VolumeBox->GetScaledBoxExtent() + CurrVoxelSize /2;
	uint32 Index = 0;

	

	//Reserving memory in array before adding elements
	Octrees = new CPathOctree[(NodeCount[0] * NodeCount[1] * NodeCount[2])];
	TraceHandles.Reserve(NodeCount[0] * NodeCount[1] * NodeCount[2]);
	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(CurrVoxelSize / 2));
	

	for (int x = 0; x < NodeCount[0]; x++)
	{
		for (int y = 0; y < NodeCount[1]; y++)
		{
			for (int z = 0; z < NodeCount[2]; z++)
			{
				FVector Location = FVector(StartPosition + FVector(x, y, z) * CurrVoxelSize);
				//Nodes.Add(FCPathNode(Index, Location));


				TraceHandles.Add(GetWorld()->AsyncOverlapByChannel(
					Location,
					FQuat(FRotator::ZeroRotator),
					TraceChannel,
					TraceBox,
					FCollisionQueryParams::DefaultQueryParam,
					FCollisionResponseParams::DefaultResponseParam,
					nullptr, PackUserParams(Index, 0))
				);
				
				Index++;
			}
		}
	}
	
}


void ACPathVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	uint32 TraceCount = TraceHandles.Num();

	int TracesHandled = 0;
	int TracesIncomplete = 0;

	if(TraceCount > 0)
	{
		uint32 Index, Depth;

		
		for(uint32 i = 0; i < TraceCount; i++)
		{ 
			FOverlapDatum OverlapDatum;
			if(GetWorld()->QueryOverlapData(TraceHandles[i], OverlapDatum))
			{
				//TraceHandles.RemoveAtSwap<int>(i, 1, false);
				TraceHandles.RemoveAt(i);

				UnpackUserParams(OverlapDatum.UserData, Index, Depth);

				if (!Depth)
				{
					Octrees[Index].IsFree = OverlapDatum.OutOverlaps.Num() == 0;

					if (!Octrees[Index].IsFree)
					{
						Octrees[Index].Children = new CPathOctree[8];
						
						float CurrHalfSize = GetVoxelSizeByDepth(Depth + 1)/2.f;

						FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(CurrHalfSize));

						// Generating children
						for (uint32 ChildIndex = 0; ChildIndex < 8; ChildIndex++)
						{
							
							FVector Location = OverlapDatum.Pos + ChildPositionOffsetMaskByIndex[ChildIndex] * CurrHalfSize;

							Octrees[Index].Children[ChildIndex].Depth = Depth + 1;
							uint32 UserData = PackUserParams(Index, Depth+1);

							SetChildIndex(UserData, Depth + 1, ChildIndex);


							TraceHandles.Add(GetWorld()->AsyncOverlapByChannel(
								Location,
								FQuat(FRotator::ZeroRotator),
								TraceChannel,
								TraceBox,
								FCollisionQueryParams::DefaultQueryParam,
								FCollisionResponseParams::DefaultResponseParam,
								nullptr, UserData)
							);

						}
					}
				}
				else
				{

				}
				
				if(OverlapDatum.OutOverlaps.Num() > 0)
				{
					if(DepthsToDraw[Depth])
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(GetVoxelSizeByDepth(Depth) /2), FColor::Red, true, 10);
				}
				else
				{
					if (DepthsToDraw[Depth])
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(GetVoxelSizeByDepth(Depth) / 2), FColor::Green, true, 10);
					//UpdateNeighbours(Nodes[OverlapDatum.UserData]);
				}
				
				i--;
				TraceCount--;
				TracesHandled++;
			}
			else
			{
				TracesIncomplete++;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Traces - , %d    incomplete - %d"), TracesHandled, TracesIncomplete);
	}
	


}

void ACPathVolume::BeginDestroy()
{
	Super::BeginDestroy();

	delete(Octrees);
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

inline uint32 ACPathVolume::PackUserParams(uint32 Index, uint32 Depth) const
{
#if WITH_EDITOR
	checkf(Depth < 5, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif

	// 8 bits last are still free, index could probably also be lower than 16 so even more space
	Index |= Depth << 16;

	return Index;
}

inline void ACPathVolume::UnpackUserParams(uint32 UserParams, uint32& Index, uint32& Depth)
{
	Depth = (UserParams & 0x00070000) >> 16;
	Index = UserParams & 0x0000FFFF;

#if WITH_EDITOR
	checkf(Depth < 5, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif
}

inline uint32 ACPathVolume::GetChildIndex(uint32 UserParams, uint32 Depth) const
{
#if WITH_EDITOR
	checkf(Depth < 5 && Depth > 0, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
#endif
	uint32 DepthOffset = Depth * 3 + 19;
	uint32 Mask = 0x00000007 << DepthOffset;

	return (UserParams & Mask) >> DepthOffset;
}

inline void ACPathVolume::SetChildIndex(uint32& UserParams, uint32 Depth, uint32 ChildIndex)
{
#if WITH_EDITOR
	checkf(Depth < 5 && Depth > 0, TEXT("CPATH - Graph Generation:::DEPTH can be up to 4"));
	checkf(ChildIndex < 8, TEXT("CPATH - Graph Generation:::Child Index can be up to 7"));
#endif
	
	ChildIndex <<= Depth * 3 + 19;

	UserParams |= ChildIndex;
}

