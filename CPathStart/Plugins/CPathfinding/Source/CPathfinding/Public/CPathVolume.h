// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCollision.h"
#include "CPathVolume.generated.h"


USTRUCT(BlueprintType)
struct FCPathGroundData
{
	GENERATED_BODY()
	FVector GroundSlope = FVector::ZeroVector;
	FVector GroundAxis = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCPathNode
{
	GENERATED_BODY()
public:

	FCPathNode(){};
	
	FCPathNode(int Index, FVector Location)
		:
	WorldLocation(Location),
	Index(Index)
	{};
	
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsFree = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsGround = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector WorldLocation;
	
	int Index;
	
	TArray<FCPathNode*> FreeNeighbors;
	
	
	
};


UCLASS()
class ACPathVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPathVolume();

	//Box to mark the area to generate graph in. It should not be rotated, the rotation will be ignored.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GenerationSettings)
	class UBoxComponent* VolumeBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GenerationSettings)
	TEnumAsByte<ECollisionChannel>  TraceChannel;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GenerationSettings)
	float AgentRadius = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GenerationSettings)
	float AgentHeight = 150;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GenerationSettings)
	bool DrawVoxels = true;

	//Size of the smallest voxel size, default: AgentRadius*2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GenerationSettings)
	float VoxelSize = AgentRadius*2;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateGraph();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Main array of nodes, used by pathfinding
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Graph)
	TArray<FCPathNode> Nodes;

private:
	
	
	//Trace handles still waiting for execution
	TArray<FTraceHandle> TraceHandles;
	
	//Dimension sizes of the Nodes array, XYZ 
	int NodeCount[3];

};
