// Fill out your copyright notice in the Description page of Project Settings.

#include "CPathAsyncFindPath.h"
#include "CPathAStar.h"
#include "CPathVolume.h"
#include <thread>

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
        Failure.Broadcast(UserPath, false);
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
    AsyncActionRef = AsyncNode;
}

bool FCPathRunnableFindPath::Init()
{
    AStar = new CPathAStar();
    return true;
}

uint32 FCPathRunnableFindPath::Run()
{
    // waiting for the volume to finish generating
    while(!AsyncActionRef->VolumeRef->SafeToAccess.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Preventing further generation while we search for a path
    AsyncActionRef->VolumeRef->PathfindersRunning++;

    TArray<CPathAStarNode> TempArray;
    auto FoundPath = AStar->FindPath(AsyncActionRef->VolumeRef, AsyncActionRef->PathStart, AsyncActionRef->PathEnd, TempArray);

    if (FoundPath)
    {
        //AStar->DrawPath(FoundPath);
        AStar->TransformToUserPath(AsyncActionRef->VolumeRef, TempArray, AsyncActionRef->UserPath);
        AsyncActionRef->Success.Broadcast(AsyncActionRef->UserPath, true);
    }
    else
    {
        AsyncActionRef->Failure.Broadcast(AsyncActionRef->UserPath, false);
    }
    AsyncActionRef->RemoveFromRoot();
    return 0;
}

void FCPathRunnableFindPath::Stop()
{
    // Decreasing the atomic running pathfinders value to enable generation
    AsyncActionRef->VolumeRef->PathfindersRunning--;
    delete AStar;
}
