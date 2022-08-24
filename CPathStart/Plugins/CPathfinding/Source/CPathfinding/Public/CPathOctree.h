// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

enum ENeighbourDirection
{
	Left,
	Front,
	Right,
	Behind,
	Below,
	Above
};


class CPATHFINDING_API CPathOctree
{
public:
	CPathOctree();

	//bool IsLeaf = false

	bool IsFree = false;

	uint32 Data = 0;
	
	//for debugging, delete later
	char VisitCounter = 0;

	//char Depth = 0;


	

	CPathOctree* Children = nullptr;




	~CPathOctree()
	{
		if(this && Children)
			delete(Children);
	};
};
