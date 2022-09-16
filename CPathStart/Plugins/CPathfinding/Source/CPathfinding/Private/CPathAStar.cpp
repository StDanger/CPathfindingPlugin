// Fill out your copyright notice in the Description page of Project Settings.


#include "CPathAStar.h"
#include "CPathVolume.h"
#include "DrawDebugHelpers.h"
#include <queue>
#include <deque>
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
	
	uint32 TempID;
	auto TimeStart = TIMENOW;

	// time limit in miliseconds
	double TimeLimitMS = Graph->PathfindingTimeLimit * 1000;

	
	
	std::priority_queue<CPathAStarNode, std::deque<CPathAStarNode>, std::greater<CPathAStarNode>> Pq;

	// Nodes visited OR added to priority queue
	std::unordered_set<CPathAStarNode, CPathAStarNode::Hash> VisitedNodes;

	// Nodes that went out of priority queue
	std::vector<std::unique_ptr<CPathAStarNode>> ProcessedNodes;

	// Finding start and end node
	if (!Graph->FindClosestFreeLeaf(Start, TempID))
		return false;
	CPathAStarNode StartNode(TempID);
	StartNode.WorldLocation = Start;

	if(!Graph->FindClosestFreeLeaf(End, TempID))
		return false;
	CPathAStarNode TargetNode(TempID);
	TargetLocation = Graph->WorldLocationFromTreeID(TargetNode.TreeID);
	TargetNode.WorldLocation = TargetLocation;

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

		auto CurrDuration = TIMEDIFF(TimeStart, TIMENOW);
		if (CurrDuration >= TimeLimitMS)
		{
			bStop = true;			
			UE_LOG(LogTemp, Warning, TEXT("Pathfinding failed - OVERTIME= %lfms   PathLength= %d  NodesVisited= %d   NodesProcessed= %d"), CurrDuration, FoundPath.Num(), VisitedNodes.size(), ProcessedNodes.size());
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
	}
	

	// Pathfinidng has been interrupted due to premature thread kill, so we dont want to return an incomplete path
	if (bStop)
	{
		FoundPath.Empty();
		return false;
	}
	auto CurrDuration = TIMEDIFF(TimeStart, TIMENOW);
	UE_LOG(LogTemp, Warning, TEXT("PATHFINDING COMPLETE:  time= %lfms   PathLength= %d  NodesVisited= %d   NodesProcessed= %d"), CurrDuration, FoundPath.Num(), VisitedNodes.size(), ProcessedNodes.size());

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
		Node.DistanceSoFar = Node.PreviousNode->DistanceSoFar + Graph->GetVoxelSizeByDepth(Graph->ExtractDepth(Node.TreeID));
	}

	Node.FitnessResult = EucDistance(Node, TargetLocation) + Node.DistanceSoFar;

}
