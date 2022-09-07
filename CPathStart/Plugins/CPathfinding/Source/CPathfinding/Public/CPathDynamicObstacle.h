// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPathDynamicObstacle.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CPATHFINDING_API UCPathDynamicObstacle : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPathDynamicObstacle();

	UFUNCTION()
		void OnBeginOverlap(AActor* Owner, AActor* OtherActor);

	UFUNCTION()
		void OnEndOverlap(AActor* Owner, AActor* OtherActor);

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//TArray<class ACPathVolume*> OverlappingVolumes;

public:	


	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
