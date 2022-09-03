// Fill out your copyright notice in the Description page of Project Settings.

#include "CPathAStar.h"
#include "CPathVolume.h"
#include "CPathAsyncFindPath.h"

UCPathAsyncFindPath* UCPathAsyncFindPath::FindPathAsync(ACPathVolume* Volume, FVector StartLocation, FVector EndLocation)
{
    UCPathAsyncFindPath* Instance = NewObject<UCPathAsyncFindPath>();
    Instance->VolumeRef = Volume;
    Instance->RunnableFindPath = new FCPathRunnableFindPath(Instance);
    Instance->PathStart = StartLocation;
    Instance->PathEnd = EndLocation;
    
    return Instance;
}

void UCPathAsyncFindPath::Activate()
{
    if (!VolumeRef)
    {
        Failure.Broadcast(-1.f);
        RemoveFromRoot();
    }
    else
    {
        CurrentThread = FRunnableThread::Create(RunnableFindPath, TEXT("AStar Pathfinding Thread"));
    }
}

void UCPathAsyncFindPath::BeginDestroy()
{


    Super::BeginDestroy();
    if (CurrentThread)
    {
        CurrentThread->Suspend(true);
        if (RunnableFindPath && RunnableFindPath->AStar)
        {
            RunnableFindPath->AStar->bStop = true;
        }
        CurrentThread->Suspend(false);
        CurrentThread->WaitForCompletion();
        CurrentThread->Kill();
    }
       
    delete RunnableFindPath;
}



FCPathRunnableFindPath::FCPathRunnableFindPath(UCPathAsyncFindPath* AsyncNode)
{
    AsyncNodeRef = AsyncNode;
}

bool FCPathRunnableFindPath::Init()
{
    AStar = new CPathAStar();

    return true;
}

uint32 FCPathRunnableFindPath::Run()
{
    auto FoundPath = AStar->FindPath(AsyncNodeRef->VolumeRef, AsyncNodeRef->PathStart, AsyncNodeRef->PathEnd);

    if (FoundPath.Num() > 0)
    {
        //AStar->DrawPath(FoundPath);
        AsyncNodeRef->Success.Broadcast(1.f);
        
    }
    else
    {
        AsyncNodeRef->Failure.Broadcast(-1.f);
    }
    AsyncNodeRef->RemoveFromRoot();
    return 0;
}

void FCPathRunnableFindPath::Stop()
{
    delete AStar;
}
