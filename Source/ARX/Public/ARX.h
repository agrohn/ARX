// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"


#if PLATFORM_ANDROID
// Enable platform-specific flag for ARtoolkit5
//#define TARGET_PLATFORM_ANDROID 1
#endif

class FARXModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

