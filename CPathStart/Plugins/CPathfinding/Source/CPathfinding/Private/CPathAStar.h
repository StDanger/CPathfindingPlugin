// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <CPathNode.h>
#include <unordered_set>


/**
 * 
 */

class ACPathVolume;

class CPathAStar
{
public:
	CPathAStar();
	~CPathAStar();

	//ACPathVolume* Graph;

	bool FindPath(ACPathVolume* VolumeRef, FVector Start, FVector End, TArray<CPathAStarNode>& PathArray);

	void DrawPath(const TArray<CPathAStarNode>& Path) const;

	// Set this to true to interrupt pathfinding. FindPath returns an empty array.
	bool bStop = false;

	static void TransformToUserPath(ACPathVolume* VolumeRef, TArray<CPathAStarNode>& AStarPath, TArray<FCPathNode>& UserPath);

private:
	ACPathVolume* Graph;
	FVector TargetLocation;

	

	float EucDistance(CPathAStarNode& Node, FVector TargetWorldLocation) const;

	void CalcFitness(CPathAStarNode& Node);

	

	//std::priority_queue<CPathAStarNode, std::vector<CPathAStarNode>, std::greater<CPathAStarNode>> pq;
};
