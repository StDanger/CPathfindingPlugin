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
	// ...
}


// Called when the game starts
void UCPathDynamicObstacle::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UCPathDynamicObstacle::OnBeginOverlap);
	GetOwner()->OnActorEndOverlap.AddDynamic(this, &UCPathDynamicObstacle::OnBeginOverlap);
	GetOwner()->UpdateOverlaps(true);
	// ...
	
}



void UCPathDynamicObstacle::OnBeginOverlap(AActor* Owner, AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("This actor: %s   Other actor: %s"), *Owner->GetName(), *OtherActor->GetName());

	ACPathVolume* Volume = Cast<ACPathVolume>(OtherActor);
	if (Volume)
	{
		Volume->DynamicObstaclesToAdd.Enqueue(GetOwner());
	}
}

void UCPathDynamicObstacle::OnEndOverlap(AActor* Owner, AActor* OtherActor)
{
	ACPathVolume* Volume = Cast<ACPathVolume>(OtherActor);
	if (Volume)
	{
		Volume->DynamicObstaclesToRemove.Enqueue(GetOwner());
	}
}

