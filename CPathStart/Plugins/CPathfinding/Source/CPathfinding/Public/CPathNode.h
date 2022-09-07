// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPathNode.generated.h"

/**
 * 
 */

class CPathAStarNode
{
public:
	CPathAStarNode();
	CPathAStarNode(uint32 ID)
		:
		TreeID(ID)
	{

	}

	uint32 TreeID = 0xFFFFFFFF;

	// We want to find a node with minimum fitness, this way distance doesnt have to be inverted
	float FitnessResult = 9999999999.f;
	float DistanceSoFar = 0;

	// This is valid ONLY during A*, probably a temporary solution
	CPathAStarNode* PreviousNode = nullptr;

	FVector WorldLocation;

	// ------ Operators for containers ----------------------------------------
    bool operator <(const CPathAStarNode& Rhs) const
	{
        return FitnessResult < Rhs.FitnessResult;
    }

	bool operator >(const CPathAStarNode& Rhs) const
	{
		return FitnessResult > Rhs.FitnessResult;
	}

	bool operator ==(const CPathAStarNode& Rhs) const
	{
		return TreeID == Rhs.TreeID;
	}

	struct Hash
	{
		size_t operator()(const CPathAStarNode& Node) const
		{
			return Node.TreeID;
		}
	};

	~CPathAStarNode();
};

USTRUCT(BlueprintType) 
struct FCPathNode
{
	GENERATED_BODY()

	FCPathNode(){}
	FCPathNode(FVector Location)
		:
		WorldLocation(Location)
	{}


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        FVector WorldLocation;


};