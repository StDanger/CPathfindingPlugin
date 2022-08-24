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

	uint32 TreeID;

	float FitnessResult = 9999999999.f;

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
		return FitnessResult == Rhs.FitnessResult;
	}

	~CPathNode();
};
