// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCollision.h"
#include <memory>
#include <chrono>
#include <vector>
#include "CPathOctree.h"
#include "CPathVolume.generated.h"


// This limit can be up to 2^16, but 2^15 should be MORE than enough
#define DEPTH_0_LIMIT (uint32)1<<16 

#define TIMENOW std::chrono::steady_clock::now()
#define TIMEDIFF(BEGIN, END) ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(END - BEGIN).count())/1000000.0


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


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CPathGenerationSettings)
		float AdditionalTraces = 0;

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

	// Finds and draws a path from first call to 2nd call. Calls outside of volume dont count.
	UFUNCTION(BlueprintCallable)
		void DebugPathStartEnd(FVector WorldLocation);
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateGraph();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	CPathOctree* Octrees = nullptr;

	virtual void BeginDestroy() override;
	
private:

	//position of the first voxel
	FVector StartPosition;

	// Lookup tables
	static const FVector LookupTable_ChildPositionOffsetMaskByIndex[8];
	static const FVector LookupTable_NeighbourOffsetByDirection[6];

	// Positive values = ChildIndex of the same parent, negative values = (-ChildIndex - 1) of neighbour at Direction [6]
	static const int8 LookupTable_NeighbourChildIndex[8][6];

	// First index is side as in ENeighbourDirection, and then you get indices of children on that side (in ascending order)
	static const int8 LookupTable_ChildrenOnSide[6][4];

	// Left returns right, up returns down, Front returns behind, etc
	static const int8 LookupTable_OppositeSide[6];
	
	//Trace handles still waiting for execution

	TArray<FTraceHandle> TraceHandles;
	std::vector<FTraceHandle>* TraceHandlesCurr;
	std::vector<FTraceHandle>* TraceHandlesNext;
	
	//Dimension sizes of the Nodes array, XYZ 
	uint32 NodeCount[3];

	void ExpandOctree(CPathOctree* TreeToExpand, uint32 CurrentTreeID, FVector TreeLocation);

	void AfterTracePreprocess();

	void UpdateNeighbours(CPathOctree* Tree, uint32 TreeID);

	
public:
	//----------- TreeID ------------------------------------------------------------------------

	// Returns the child with this tree id, or his parent at DepthReached in case the child doesnt exist
	CPathOctree* FindTreeByID(uint32 TreeID, uint32& DepthReached);

	// Returns a tree and its TreeID by world location, returns null if location outside of volume. Only for Outer index
	CPathOctree* FindTreeByWorldLocation(FVector WorldLocation, uint32& TreeID);

	// Returns a leaf and its TreeID by world location, returns null if location outside of volume. 
	CPathOctree* FindLeafByWorldLocation(FVector WorldLocation, uint32& TreeID, bool MustBeFree = 1);

	// Returns a neighbour of the tree with TreeID in given direction, also returns  TreeID if the neighbour if found
	CPathOctree* FindNeighbourByID(uint32 TreeID, ENeighbourDirection Direction, uint32& NeighbourID);

	// Returns a list of free adjecent leafs as TreeIDs
	std::vector<uint32> FindAllNeighbourLeafs(uint32 TreeID);

	// Returns a parent of tree with given TreeID or null if TreeID has depth of 0
	inline CPathOctree* GetParentTree(uint32 TreeId);

	// Returns world location of a voxel at this TreeID. This returns CENTER of the voxel
	inline FVector WorldLocationFromTreeID(uint32 TreeID) const;

	inline FVector LocalCoordsInt3FromOuterIndex(uint32 OuterIndex) const;

	// Creates TreeID for AsyncOverlapByChannel
	inline uint32 CreateTreeID(uint32 Index, uint32 Depth) const;

	// Extracts Octrees array index from TreeID
	inline uint32 ExtractOuterIndex(uint32 TreeID) const;

	// Replaces Depth in the TreeID with NewDepth
	inline void ReplaceDepth(uint32& TreeID, uint32 NewDepth);

	// Extracts depth from TreeID
	inline uint32 ExtractDepth(uint32 TreeID) const;

	// Returns a number from  0 to 7 - a child index at requested Depth
	inline uint32 ExtractChildIndex(uint32 TreeID, uint32 Depth) const;

	// This assumes that child index at Depth is 000, if its not use ReplaceChildIndex
	inline void AddChildIndex(uint32& TreeID, uint32 Depth, uint32 ChildIndex);

	// Replaces child index at given depth
	inline void ReplaceChildIndex(uint32& TreeID, uint32 Depth, uint32 ChildIndex);

	// Replaces child index at given depth and also replaces depth to the same one
	inline void ReplaceChildIndexAndDepth(uint32& TreeID, uint32 Depth, uint32 ChildIndex);

	




	// ----------- Other helper functions ---------------------

	inline float GetVoxelSizeByDepth(int Depth) const;

private:

	// Returns an index in the Octree array from world position. NO BOUNDS CHECK
	inline int WorldLocationToIndex(FVector WorldLocation) const;

	// Multiplies local integer coordinates into index
	inline float LocalCoordsInt3ToIndex(FVector V) const;

	// Returns the X Y and Z relative to StartPosition and divided by VoxelSize. Multiply them to get the index. NO BOUNDS CHECK
	inline FVector WorldLocationToLocalCoordsInt3(FVector WorldLocation) const;

	// Returns world location of a tree at depth 0. Extracts only outer index from TreeID
	inline FVector GetOuterTreeWorldLocation(uint32 TreeID) const;

	// takes in what `WorldLocationToLocalCoordsInt3` returns and performs a bounds check
	inline bool IsInBounds(FVector LocalCoordsInt3) const;

	// Helper function for 'FindLeafByWorldLocation'. Relative location is location relative to the middle of CurrentTree
	CPathOctree* FindLeafRecursive(FVector RelativeLocation, uint32& TreeID, uint32 CurrentDepth, CPathOctree* CurrentTree);

	// Returns IDs of all free leafs on chosen side of a tree. Sides are indexed in the same way as neighbours, and adds them to passed Vector.
	// ASSUMES THAT PASSED TREE HAS CHILDREN
	void FindFreeLeafsOnSide(uint32 TreeID, ENeighbourDirection Side, std::vector<uint32>* Vector);
	
	// Same as the other version, but skips the part of getting a tree by TreeID so its faster
	void FindFreeLeafsOnSide(CPathOctree* Tree, uint32 TreeID, ENeighbourDirection Side, std::vector<uint32>* Vector);


	bool IsInBounds(int OuterIndex) const;

	FVector DebugPathStart;
	bool HasDebugPathStarted = false;



	std::chrono::steady_clock::time_point GenerationStart;
	bool PrintGenerationTime = false;
};
