// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAStar.h"
#include "CPathVolume.h"
#include "DrawDebugHelpers.h"


CPathAStar::CPathAStar()
{
}

CPathAStar::~CPathAStar()
{
	
}

TArray<CPathNode> CPathAStar::FindPath(ACPathVolume* VolumeRef, FVector Start, FVector End)
{
	Graph = VolumeRef;
	TargetLocation = End;
	uint32 TempID;
	Graph->FindTreeByWorldLocation(Start, TempID);

	CPathNode StartNode(TempID);

	Graph->FindTreeByWorldLocation(End, TempID);

	CPathNode EndNode(TempID);
	
	CalcFitness(EndNode);
	CalcFitness(StartNode);
	CPathNode RandomNodeA(0);
	CPathNode RandomNodeB(3);
	CalcFitness(RandomNodeB);
	CalcFitness(RandomNodeA);

	std::priority_queue<CPathNode, std::vector<CPathNode>, std::greater<CPathNode>> Pq;

	//Pq.push(CPathNode(2));
	//Pq.push(CPathNode(5));
	Pq.push(EndNode);
	Pq.push(RandomNodeB);
	Pq.push(StartNode);
	Pq.push(RandomNodeA);
	//Pq.push(CPathNode(8));

	TArray<CPathNode> ReturnArray;

	while (Pq.size() > 0)
	{
		ReturnArray.Add(Pq.top());
		Pq.pop();
	}
	return ReturnArray;
}

void CPathAStar::DrawPath(const TArray<CPathNode>& Path) const
{
	for (int i = 0; i < Path.Num()-1; i++)
	{
		DrawDebugLine(Graph->GetWorld(), Graph->WorldLocationFromTreeID(Path[i].TreeID), Graph->WorldLocationFromTreeID(Path[i + 1].TreeID), FColor::Magenta, true);
	}
}

float CPathAStar::EucDistance(CPathNode& Node, FVector Target) const
{
	return FVector::Distance(Graph->WorldLocationFromTreeID(Node.TreeID), Target);
}

void CPathAStar::CalcFitness(CPathNode& Node)
{
	Node.FitnessResult = EucDistance(Node, TargetLocation);
}
