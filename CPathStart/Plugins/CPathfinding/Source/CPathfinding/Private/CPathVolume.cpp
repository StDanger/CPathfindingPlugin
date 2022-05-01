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


	NodeCount[0] = VolumeBox->GetScaledBoxExtent().X*2 / VoxelSize + 1;
	NodeCount[1] = VolumeBox->GetScaledBoxExtent().Y*2 / VoxelSize + 1;
	NodeCount[2] = VolumeBox->GetScaledBoxExtent().Z*2 / VoxelSize + 1;
	UE_LOG(LogTemp, Warning, TEXT("Count, %d %d %d"), NodeCount[0], NodeCount[1], NodeCount[2] );
	GenerateGraph();
	
}

void ACPathVolume::GenerateGraph()
{
	StartPosition = GetActorLocation() - VolumeBox->GetScaledBoxExtent();
	int Index = 0;

	//Reserving memory in array before adding elements
	Nodes.Reserve(NodeCount[0]*NodeCount[1]*NodeCount[2]);
	TraceHandles.Reserve(NodeCount[0]*NodeCount[1]*NodeCount[2]);
	FCollisionShape TraceBox = FCollisionShape::MakeBox(FVector(VoxelSize/2));
	
	
	for (int x = 0; x < NodeCount[0]; x++)
	{
		for (int y = 0; y < NodeCount[1]; y++)
		{
			for (int z = 0; z < NodeCount[2]; z++)
			{
				FVector Location = FVector(StartPosition + FVector(x, y, z)*VoxelSize);
				Nodes.Add(FCPathNode(Index, Location));
				

				TraceHandles.Add(GetWorld()->AsyncOverlapByChannel(
					Location,
					 FQuat(FRotator::ZeroRotator),
					TraceChannel,
					TraceBox,
					FCollisionQueryParams::DefaultQueryParam,
					FCollisionResponseParams::DefaultResponseParam,
					nullptr, Index)
				);
				//if(DrawVoxels)
				//	DrawDebugBox(GetWorld(), Location, TraceBox.GetExtent(), FColor::Green, true, 10);
				Index++;
			}
		}
	}

	
}

// Called every frame
void ACPathVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(TraceHandles.Num()>0)
	{
		for(int i = 0; i<TraceHandles.Num(); i++)
		{
			FOverlapDatum OverlapDatum;
			if(GetWorld()->QueryOverlapData(TraceHandles[i], OverlapDatum))
			{
				TraceHandles.RemoveAtSwap(i);

				Nodes[OverlapDatum.UserData].WorldLocation = OverlapDatum.Pos;
				
				if(OverlapDatum.OutOverlaps.Num() > 0)
				{
					if(DrawVoxels)
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(VoxelSize/2), FColor::Red, true, 10);
				}
				else
				{
					if(DrawVoxels)
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(VoxelSize/2), FColor::Green, true, 10);
					
					Nodes[OverlapDatum.UserData].IsFree = true;
					UpdateNeighbours(Nodes[OverlapDatum.UserData]);
					
					
				}
				
				i--;
			}
		}
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
	RelativePos = RelativePos / VoxelSize;
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

