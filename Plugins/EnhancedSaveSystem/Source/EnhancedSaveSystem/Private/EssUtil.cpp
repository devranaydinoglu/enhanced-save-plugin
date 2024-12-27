// Copyright 2023 devran. All Rights Reserved.

#include "EssUtil.h"

bool EssUtil::IsRuntimeActor(const TObjectPtr<AActor> Actor)
{
	//return Actor->bNetStartup && !Actor->HasAnyFlags(RF_WasLoaded);
	return !Actor->HasAnyFlags(RF_WasLoaded);;
}

FGuid EssUtil::GetGuid(const TObjectPtr<UObject> Obj)
{
	FProperty* Prop = GetGuidProperty(Obj);
	if (Prop)
		return *Prop->ContainerPtrToValuePtr<FGuid>(Obj);

	UE_LOG(LogTemp, Warning, TEXT("Object %s has no EssGuid property and can therefore not be saved."), *Obj->GetFName().ToString());
	FGuid Guid = FGuid::NewGuid();
	Guid.Invalidate();
	return Guid;
}

FProperty* EssUtil::GetGuidProperty(const TObjectPtr<UObject> Obj)
{
	return Obj->GetClass()->FindPropertyByName("EssGuid");
}

bool EssUtil::SetGuid(TObjectPtr<UObject> Obj, const FGuid& NewGuid)
{
	FProperty* Prop = GetGuidProperty(Obj);
	if (!Prop)
	{
		UE_LOG(LogTemp, Warning, TEXT("Object %s has no EssGuid property and can therefore not be saved."), *Obj->GetFName().ToString());
		return false;
	}

	SetGuid(Obj, NewGuid, GetGuidProperty(Obj));
	return true;
}

void EssUtil::SetGuid(TObjectPtr<UObject> Obj, const FGuid& NewGuid, FProperty* Prop)
{
	FGuid* GuidPtr = Prop->ContainerPtrToValuePtr<FGuid>(Obj);
	*GuidPtr = NewGuid;
}
