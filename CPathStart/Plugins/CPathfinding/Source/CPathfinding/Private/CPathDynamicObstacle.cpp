// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathDynamicObstacle.h"
#include "CPathVolume.h"

// Sets default values for this component's properties
UCPathDynamicObstacle::UCPathDynamicObstacle()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = false;
	// ...
}


void UCPathDynamicObstacle::Activate(bool bReset)
{

	Super::Activate();
	
		TSubclassOf<ACPathVolume> Filter = ACPathVolume::StaticClass();
		GetOwner()->GetOverlappingActors(OverlappigVolumes, ACPathVolume::StaticClass());
		for (AActor* Volume : OverlappigVolumes)
		{
			Cast<ACPathVolume>(Volume)->TrackedDynamicObstacles.insert(GetOwner());
		}
	
}

void UCPathDynamicObstacle::Deactivate()
{
	Super::Deactivate();
	for (AActor* Volume : OverlappigVolumes)
	{
		Cast<ACPathVolume>(Volume)->TrackedDynamicObstacles.erase(GetOwner());
	}
	OverlappigVolumes.Empty();
}

// Called when the game starts
void UCPathDynamicObstacle::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UCPathDynamicObstacle::OnBeginOverlap);
	GetOwner()->OnActorEndOverlap.AddDynamic(this, &UCPathDynamicObstacle::OnBeginOverlap);
	if (ActivateOnBeginPlay)
	{
		Activate();
	}

	// ...
	
}



void UCPathDynamicObstacle::OnBeginOverlap(AActor* Owner, AActor* OtherActor)
{
	if (IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("BEGIN OVERLAP - This actor: %s   Other actor: %s"), *Owner->GetName(), *OtherActor->GetName());

		ACPathVolume* Volume = Cast<ACPathVolume>(OtherActor);
		if (Volume)
		{
			Volume->TrackedDynamicObstacles.insert(GetOwner());
			OverlappigVolumes.Add(Volume);
		}
	}
}

void UCPathDynamicObstacle::OnEndOverlap(AActor* Owner, AActor* OtherActor)
{
	if (IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("END OVERLAP - This actor: %s   Other actor: %s"), *Owner->GetName(), *OtherActor->GetName());
		ACPathVolume* Volume = Cast<ACPathVolume>(OtherActor);
		if (Volume)
		{
			Volume->TrackedDynamicObstacles.erase(GetOwner());
			OverlappigVolumes.Remove(Volume);
		}
	}
}

