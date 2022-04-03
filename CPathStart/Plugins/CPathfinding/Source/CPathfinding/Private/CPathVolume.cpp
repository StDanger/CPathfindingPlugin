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
	DrawDebugBox(GetWorld(), GetActorLocation(), tempBox->GetScaledBoxExtent()*2,
		FColor::Magenta, true, 10);
	
}

// Called every frame
void ACPathVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

