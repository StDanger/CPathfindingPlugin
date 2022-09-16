// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPathDynamicObstacle.generated.h"

// For better performance, call Deactivate() on this component once you dont need it to be updated anymore.
// bAutoActivate should be left unckecked for this component.
// Owning actor MUST be Movable for this to work. If you want a static/stationary actor that appears/disappears, you will need to manually add it to a CPathVolume as tracked obstacle.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CPATHFINDING_API UCPathDynamicObstacle : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCPathDynamicObstacle();

	UFUNCTION()
		void OnBeginOverlap(AActor* Owner, AActor* OtherActor);

	UFUNCTION()
		void OnEndOverlap(AActor* Owner, AActor* OtherActor);

	// CPathVolumes that consider this actor an obstacle.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CPath)
		TSet<AActor*> OverlappigVolumes;

	// If this actor gets spawned inside a CPathVolume, it will tell the volume about it.
	// If this is left unchecked, you must make sure that this actor spawns outside of a CPathVolume, or do it manually.
	// If you spawn CPathVolume AFTEr this actor, you will need to handle overlaps manually.
	// In case of Level Streaming, CPathVolume should be placed on the same level as each actor it's supposed to track.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CPath)
		bool ActivateOnBeginPlay = true;

	virtual void Activate(bool bReset = false) override;

	virtual void Deactivate() override;

	void AddIndexesToUpdate(ACPathVolume* Volume);

	virtual void EndPlay(EEndPlayReason::Type Reason) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//TArray<class ACPathVolume*> OverlappingVolumes;

public:	


	

		
};
