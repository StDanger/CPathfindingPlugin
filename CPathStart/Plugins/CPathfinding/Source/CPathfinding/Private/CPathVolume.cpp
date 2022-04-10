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

// Called when the game starts or when spawned
void ACPathVolume::BeginPlay()
{
	Super::BeginPlay();
	UBoxComponent* tempBox = Cast<UBoxComponent>(GetRootComponent());
	DrawDebugBox(GetWorld(), GetActorLocation(), tempBox->GetScaledBoxExtent(),
		FColor::Magenta, true, 10);

	NodeCount[0] = VolumeBox->GetScaledBoxExtent().X*2 / VoxelSize + 1;
	NodeCount[1] = VolumeBox->GetScaledBoxExtent().Y*2 / VoxelSize + 1;
	NodeCount[2] = VolumeBox->GetScaledBoxExtent().Z*2 / VoxelSize + 1;
	UE_LOG(LogTemp, Warning, TEXT("Count, %d %d %d"), NodeCount[0], NodeCount[1], NodeCount[2] );
	GenerateGraph();
	
}

void ACPathVolume::GenerateGraph()
{
	FVector Start = GetActorLocation() - VolumeBox->GetScaledBoxExtent();
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
				FVector Location = FVector(Start + FVector(x, y, z)*VoxelSize);
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
				
				if(OverlapDatum.OutOverlaps.Num() > 0)
				{
					if(DrawVoxels)
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(VoxelSize/2), FColor::Red, true, 10);
				}
				else
				{
					if(DrawVoxels)
						DrawDebugBox(GetWorld(), OverlapDatum.Pos, FVector(VoxelSize/2), FColor::Green, true, 10);
				}
				
				i--;
			}
		}
	}


}

