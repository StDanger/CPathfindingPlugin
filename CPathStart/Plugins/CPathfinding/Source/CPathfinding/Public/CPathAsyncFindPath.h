// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPathNode.h"
#include "Core/Public/HAL/Runnable.h"
#include "Core/Public/HAL/RunnableThread.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CPathAsyncFindPath.generated.h"


/**
 * 
 */


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FResponseDelegate, float, TestFloat);


UCLASS()
class CPATHFINDING_API UCPathAsyncFindPath : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FResponseDelegate Success;

	UPROPERTY(BlueprintAssignable)
		FResponseDelegate Failure;

	

	UFUNCTION(BlueprintCallable, Category=CPath, meta = (BlueprintInternalUseOnly = "true"))
		static UCPathAsyncFindPath* FindPathAsync(class ACPathVolume* Volume, FVector StartLocation, FVector EndLocation);

	virtual void Activate() override;
	virtual void BeginDestroy() override;
	class ACPathVolume* VolumeRef;

	FVector PathStart, PathEnd;

private:

	
	class FCPathRunnableFindPath* RunnableFindPath = nullptr;
	FRunnableThread* CurrentThread = nullptr;
	TQueue<bool> ThreadData;

};

class FCPathRunnableFindPath : public FRunnable
{
public:
	FCPathRunnableFindPath(class UCPathAsyncFindPath* AsyncNode);

	virtual bool Init();

	virtual uint32 Run();

	virtual void Stop();

	//bool StopThread = false;

	class CPathAStar* AStar = nullptr;
private:
	
	class UCPathAsyncFindPath* AsyncNodeRef = nullptr;
};
