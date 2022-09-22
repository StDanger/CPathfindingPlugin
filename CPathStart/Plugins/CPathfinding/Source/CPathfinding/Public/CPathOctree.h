// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

enum ENeighbourDirection
{
	Left,	// -Y
	Front,	// -X
	Right,	// +Y
	Behind, // +X
	Below,	// -Z
	Above	// +Z
};


class CPATHFINDING_API CPathOctree
{
public:
	CPathOctree();
		

	CPathOctree* Children = nullptr;

	uint32 Data = 0;



	inline void SetIsFree(bool IsFree)
	{
		Data &= 0xFFFFFFFE;
		Data |= (uint32)IsFree;
	}

	inline bool GetIsFree() const
	{
		return Data << 31;
	}

	~CPathOctree()
	{		
		delete[] Children;
	};
};
