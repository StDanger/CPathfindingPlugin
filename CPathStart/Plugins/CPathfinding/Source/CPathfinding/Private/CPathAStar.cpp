// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAStar.h"
#include "CPathVolume.h"
#include "DrawDebugHelpers.h"
#include <memory>


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

	TArray<CPathNode> FoundPath;
	std::priority_queue<CPathNode, std::vector<CPathNode>, std::greater<CPathNode>> Pq;

	// Nodes visited OR added to priority queue
	std::unordered_set<CPathNode, CPathNode::Hash> VisitedNodes;

	// Nodes that went out of priority queue
	std::vector<std::unique_ptr<CPathNode>> ProcessedNodes;

	// Finding start and end node
	if (!Graph->FindLeafByWorldLocation(Start, TempID))
		return FoundPath;
	CPathNode StartNode(TempID);

	if(!Graph->FindLeafByWorldLocation(End, TempID))
		return FoundPath;
	CPathNode TargetNode(TempID);
	CalcFitness(TargetNode);
	CalcFitness(StartNode);
	
	

	Pq.push(StartNode);
	VisitedNodes.insert(StartNode);
	
	CPathNode* FoundPathEnd = nullptr;

	while (Pq.size() > 0)
	{
		CPathNode CurrentNode = Pq.top();
		Pq.pop();
		ProcessedNodes.push_back(std::make_unique<CPathNode>(CurrentNode));
		//VisitedNodes.insert(CurrentNode);
		
		

		if (CurrentNode == TargetNode)
		{
			FoundPathEnd = ProcessedNodes.back().get();
			//TargetNode = *VisitedNodes.find(CurrentNode);
			break;
		}
		
		std::vector<uint32> Neighbours = VolumeRef->FindAllNeighbourLeafs(CurrentNode.TreeID);

		for (uint32 NewTreeID : Neighbours)
		{
			CPathNode NewNode(NewTreeID);
			if (!VisitedNodes.count(NewNode))
			{
				//NewNode.PreviousNodeID = CurrentNode.TreeID;
				NewNode.PreviousNode = ProcessedNodes.back().get();

				CalcFitness(NewNode);

				VisitedNodes.insert(NewNode);
				Pq.push(NewNode);
			}
		}
		
	}
	
	int debugCounter = 0;


	if (FoundPathEnd)
	{
		while (FoundPathEnd)
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
			TargetNode = *VisitedNodes.find(CPathNode(TargetNode.PreviousNodeID));
		}
		FoundPath.Add(StartNode);*/
	}
	
	
	return FoundPath;
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
	

	if (Node.PreviousNode)
	{
		Node.DistanceSoFar = Node.PreviousNode->DistanceSoFar + Graph->GetVoxelSizeByDepth(Graph->ExtractDepth(Node.TreeID))/2.f;
	}

	Node.FitnessResult = EucDistance(Node, TargetLocation) + Node.DistanceSoFar;

}
