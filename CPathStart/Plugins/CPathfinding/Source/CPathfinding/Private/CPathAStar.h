// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <CPathNode.h>
#include <queue>
#include <vector>
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

	TArray<CPathNode> FindPath(ACPathVolume* VolumeRef, FVector Start, FVector End);

	void DrawPath(const TArray<CPathNode>& Path) const;


private:
	ACPathVolume* Graph;
	FVector TargetLocation;



	float EucDistance(CPathNode& Node, FVector TargetWorldLocation) const;

	void CalcFitness(CPathNode& Node);

	//std::priority_queue<CPathNode, std::vector<CPathNode>, std::greater<CPathNode>> pq;
};
