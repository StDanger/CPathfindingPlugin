// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCollision.h"
#include <memory>
#include "CPathOctree.h"
#include "CPathVolume.generated.h"


// This limit can be up to 2^16, but 2^15 should be MORE than enough
#define DEPTH_0_LIMIT 1<<15 



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
	bool IsFree = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsGround = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector WorldLocation = {0, 0, 0};
	
	int Index = -1;
	
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CPathGenerationSettings)
		class UBoxComponent* VolumeBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CPathGenerationSettings)
		TEnumAsByte<ECollisionChannel>  TraceChannel;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CPathGenerationSettings)
		float AgentRadius = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CPathGenerationSettings)
		float AgentHeight = 150;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CPathGenerationSettings)
		bool DrawConnections = true;

	//Size of the smallest voxel size, default: AgentRadius*2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CPathGenerationSettings)
		float VoxelSize = AgentRadius*2;

	// The smaller it is, the faster pathfinding, but smaller values lead to long generation time and higher memory comsumption
	// Smaller values also limit the size of the volume.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CPathGenerationSettings, meta = (ClampMin = "0", ClampMax = "4", UIMin = "0", UIMax = "4"))
		int OctreeDepth = 2;

	// Drawing depths for debugging, if size of this array is different than OctreeDepth, this will potentially crash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CPathGenerationSettings)
		TArray<bool> DepthsToDraw;

	// This is a read only info about generated graph
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CPathGeneratedInfo)
		TArray<int> VoxelCountAtDepth;
	

	//Returns node at given world location, valid is set to false if the node is out of bounds
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = CPathUtility)
	FCPathNode& GetNodeFromPosition(FVector WorldLocation, bool& IsValid);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateGraph();

	//TArray<FCPathNode*> GetAllNeighbours; 

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Main array of nodes, used by pathfinding
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Graph)
		TArray<FCPathNode> Nodes;

	CPathOctree* Octrees = nullptr;

	virtual void BeginDestroy() override;

	

private:

	//position of the first voxel
	FVector StartPosition;

	// For generating children
	static const FVector ChildPositionOffsetMaskByIndex[8];
	
	//Trace handles still waiting for execution
	TArray<FTraceHandle> TraceHandles;
	
	//Dimension sizes of the Nodes array, XYZ 
	int NodeCount[3];

	void UpdateNeighbours(FCPathNode& Node);
	
	TArray<int> GetAdjacentIndexes(int Index) const;

	// Returns the X Y and Z relative to StartPosition and divided by VoxelSize. Multiply them to get the index. NO BOUNDS CHECK
	FVector GetXYZFromPosition(FVector WorldLocation) const;

	// Returns an index in the Node array from world position. NO BOUNDS CHECK
	int GetIndexFromPosition(FVector WorldLocation) const;

	// takes in what `GetXYZFromPosition` returns and performs a bounds check
	bool IsInBounds(FVector XYZ) const;

	inline float GetIndexFromXYZ(FVector V) const;

	inline float GetVoxelSizeByDepth(int Depth) const;

	// Creates UserParams for AsyncOverlapByChannel
	inline uint32 PackUserParams(uint32 Index, uint32 Depth) const;

	inline void UnpackUserParams(uint32 UserParams, uint32& Index, uint32& Depth);

	// Returns a number from  0 to 7 - a child index at required depth from the params
	inline uint32 GetChildIndex(uint32 UserParams, uint32 Depth) const;

	// This assumes that child index at Depth is 000
	inline void SetChildIndex(uint32& UserParams, uint32 Depth, uint32 ChildIndex);
};
