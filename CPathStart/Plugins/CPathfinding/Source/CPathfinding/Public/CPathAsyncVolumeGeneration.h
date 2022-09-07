// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ACPathVolume;

class FCPathAsyncVolumeGenerator : public FRunnable
{

	
public:	
	// Geneated trees in range Start(inclusive) - End(not inclusive). If EndIndex =0 0, it will default to the other constructor
	FCPathAsyncVolumeGenerator(ACPathVolume* Volume, uint32 StartIndex, uint32 EndIndex);

	// Generates indexes from set TreesToRegenerate in the given volume
	FCPathAsyncVolumeGenerator(ACPathVolume* Volume);

	virtual bool Init();

	virtual uint32 Run();

	virtual void Stop();

	// Retraces the octree from TreeID downwards. Pass optional arguments to make it a bit faster. When OctreeRef is passed, TreeID is ignored
	void RefreshTree(uint32 OuterIndex);

	bool bStop = false;
	

protected:

	ACPathVolume* VolumeRef;

	uint32 Start = 0;
	uint32 End = 0;

	// Gets called by RefreshTree
	void RefreshTreeRec(CPathOctree* OctreeRef, uint32 Depth, FVector TreeLocation);

};
