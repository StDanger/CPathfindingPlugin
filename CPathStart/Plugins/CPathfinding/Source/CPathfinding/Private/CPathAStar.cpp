// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAStar.h"
#include "CPathVolume.h"
#include "DrawDebugHelpers.h"
#include <queue>
#include <vector>
#include <unordered_set>
#include <memory>


CPathAStar::CPathAStar()
{
}

CPathAStar::~CPathAStar()
{
	
}

bool CPathAStar::FindPath(ACPathVolume* VolumeRef, FVector Start, FVector End, TArray<CPathAStarNode>& FoundPath)
{
	Graph = VolumeRef;
	TargetLocation = End;
	uint32 TempID;

	
	std::priority_queue<CPathAStarNode, std::vector<CPathAStarNode>, std::greater<CPathAStarNode>> Pq;

	// Nodes visited OR added to priority queue
	std::unordered_set<CPathAStarNode, CPathAStarNode::Hash> VisitedNodes;

	// Nodes that went out of priority queue
	std::vector<std::unique_ptr<CPathAStarNode>> ProcessedNodes;

	// Finding start and end node
	if (!Graph->FindLeafByWorldLocation(Start, TempID))
		return false;
	CPathAStarNode StartNode(TempID);

	if(!Graph->FindLeafByWorldLocation(End, TempID))
		return false;
	CPathAStarNode TargetNode(TempID);
	CalcFitness(TargetNode);
	CalcFitness(StartNode);
		

	Pq.push(StartNode);
	VisitedNodes.insert(StartNode);
	
	CPathAStarNode* FoundPathEnd = nullptr;

	while (Pq.size() > 0 && !bStop)
	{
		CPathAStarNode CurrentNode = Pq.top();
		Pq.pop();
		ProcessedNodes.push_back(std::make_unique<CPathAStarNode>(CurrentNode));

		if (CurrentNode == TargetNode)
		{
			FoundPathEnd = ProcessedNodes.back().get();
			//TargetNode = *VisitedNodes.find(CurrentNode);
			break;
		}
		
		std::vector<uint32> Neighbours = VolumeRef->FindAllNeighbourLeafs(CurrentNode.TreeID);

		for (uint32 NewTreeID : Neighbours)
		{
			if (bStop)
				break;

			CPathAStarNode NewNode(NewTreeID);
			if (!VisitedNodes.count(NewNode))
			{
				//NewNode.PreviousNodeID = CurrentNode.TreeID;
				NewNode.PreviousNode = ProcessedNodes.back().get();
				NewNode.WorldLocation = Graph->WorldLocationFromTreeID(NewNode.TreeID);
				CalcFitness(NewNode);

				VisitedNodes.insert(NewNode);
				Pq.push(NewNode);
			}
		}
		
	}
	
	int debugCounter = 0;


	if (FoundPathEnd)
	{
		while (FoundPathEnd && !bStop)
		{
			
			FoundPath.Add(*FoundPathEnd);
			FoundPathEnd = FoundPathEnd->PreviousNode;
		}


		/*while (!(TargetNode == StartNode))
		{
			debugCounter++;
			if (debugCounter > 20)
			{
				debugCounter = 0;
			}
			FoundPath.Add(TargetNode);
			TargetNode = *VisitedNodes.find(CPathAStarNode(TargetNode.PreviousNodeID));
		}
		FoundPath.Add(StartNode);*/
	}
	
	// Pathfinidng has been interrupted due to premature thread kill, so we dont want to return an incomplete path
	if (bStop)
	{
		FoundPath.Empty();
		return false;
	}
		

	return true;
}

void CPathAStar::DrawPath(const TArray<CPathAStarNode>& Path) const
{
	for (int i = 0; i < Path.Num()-1; i++)
	{
		DrawDebugLine(Graph->GetWorld(), Graph->WorldLocationFromTreeID(Path[i].TreeID), Graph->WorldLocationFromTreeID(Path[i + 1].TreeID), FColor::Magenta, false, 0.5f, 3, 1.5);
	}
}

void CPathAStar::TransformToUserPath(ACPathVolume* VolumeRef, TArray<CPathAStarNode>& AStarPath, TArray<FCPathNode>& UserPath)
{
	UserPath.Reserve(AStarPath.Num());
	for (int i = AStarPath.Num() - 1; i >= 0; i--)
	{
		UserPath.Add(FCPathNode(AStarPath[i].WorldLocation));
	}
}

float CPathAStar::EucDistance(CPathAStarNode& Node, FVector Target) const
{
	return FVector::Distance(Node.WorldLocation, Target);
}

void CPathAStar::CalcFitness(CPathAStarNode& Node)
{
	

	if (Node.PreviousNode)
	{
		Node.DistanceSoFar = Node.PreviousNode->DistanceSoFar + Graph->GetVoxelSizeByDepth(Graph->ExtractDepth(Node.TreeID))/2.f;
	}

	Node.FitnessResult = EucDistance(Node, TargetLocation) + Node.DistanceSoFar;

}
