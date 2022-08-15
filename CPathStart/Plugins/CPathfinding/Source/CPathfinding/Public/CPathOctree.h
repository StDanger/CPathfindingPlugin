// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */




class CPATHFINDING_API CPathOctree
{
public:
	CPathOctree();

	//bool IsLeaf = false

	bool IsFree = false;

	uint32 Data = 0;

	//char FreeNeighbours = 0;

	//char Depth = 0;


	

	CPathOctree* Children = nullptr;




	~CPathOctree()
	{
		delete(Children);
	};
};
