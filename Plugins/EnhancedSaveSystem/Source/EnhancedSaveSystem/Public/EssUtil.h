// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ENHANCEDSAVESYSTEM_API EssUtil
{
public:
	static bool IsRuntimeActor(const AActor* Actor);
	static FGuid GetGuid(const UObject* Obj);
	static FProperty* GetGuidProperty(const UObject* Obj);
	static bool SetGuid(UObject* Obj, const FGuid& NewGuid);
	static bool IsActorRespawnable(const AActor* Actor);

private:
	static void SetGuid(UObject* Obj, const FGuid& NewGuid, FProperty* Prop);
};