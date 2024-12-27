// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ENHANCEDSAVESYSTEM_API EssUtil
{
public:
	static bool IsRuntimeActor(const TObjectPtr<AActor> Actor);
	static FGuid GetGuid(const TObjectPtr<UObject> Obj);
	static FProperty* GetGuidProperty(const TObjectPtr<UObject> Obj);
	static bool SetGuid(TObjectPtr<UObject> Obj, const FGuid& NewGuid);

private:
	static void SetGuid(TObjectPtr<UObject> Obj, const FGuid& NewGuid, FProperty* Prop);
};