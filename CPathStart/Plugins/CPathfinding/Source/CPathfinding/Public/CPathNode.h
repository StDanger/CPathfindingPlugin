// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class CPATHFINDING_API CPathNode
{
public:
	CPathNode();
	CPathNode(uint32 ID)
		:
		TreeID(ID)
	{

	}

	uint32 TreeID = 0xFFFFFFFF;

	// We want to find a node with minimum fitness, this way distance doesnt have to be inverted
	float FitnessResult = 9999999999.f;
	float DistanceSoFar = 0;

	// This is valid ONLY during A*, probably a temporary solution
	CPathNode* PreviousNode = nullptr;


	// ------ Operators for containers ----------------------------------------
    bool operator <(const CPathNode& Rhs) const
	{
        return FitnessResult < Rhs.FitnessResult;
    }

	bool operator >(const CPathNode& Rhs) const
	{
		return FitnessResult > Rhs.FitnessResult;
	}

	bool operator ==(const CPathNode& Rhs) const
	{
		return TreeID == Rhs.TreeID;
	}

	struct Hash
	{
		size_t operator()(const CPathNode& Node) const
		{
			return Node.TreeID;
		}
	};

	~CPathNode();
};
