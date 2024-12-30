// Copyright 2023 devran. All Rights Reserved.

#include "EssUtil.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

bool EssUtil::IsRuntimeActor(const AActor* Actor)
{
	return !Actor->HasAnyFlags(RF_WasLoaded);;
}

FGuid EssUtil::GetGuid(const UObject* Obj)
{
	FProperty* Prop = GetGuidProperty(Obj);
	if (Prop)
		return *Prop->ContainerPtrToValuePtr<FGuid>(Obj);

	UE_LOG(LogTemp, Warning, TEXT("Object %s has no EssGuid property and can therefore not be saved."), *Obj->GetFName().ToString());
	FGuid Guid = FGuid::NewGuid();
	Guid.Invalidate();
	return Guid;
}

FProperty* EssUtil::GetGuidProperty(const UObject* Obj)
{
	return Obj->GetClass()->FindPropertyByName("EssGuid");
}

bool EssUtil::SetGuid(UObject* Obj, const FGuid& NewGuid)
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

bool EssUtil::IsActorRespawnable(const AActor* Actor)
{
	return !Actor->IsA(AGameModeBase::StaticClass()) && !Actor->IsA(AGameStateBase::StaticClass()) &&
		!Actor->IsA(APlayerState::StaticClass()) && !Actor->IsA(APlayerController::StaticClass());
}

bool EssUtil::IsActorRespawnable(const TSubclassOf<AActor>& Class)
{
	return !Class->IsChildOf(AGameModeBase::StaticClass()) && !Class->IsChildOf(AGameStateBase::StaticClass()) &&
		!Class->IsChildOf(APlayerState::StaticClass()) && !Class->IsChildOf(APlayerController::StaticClass());
}
 
FString EssUtil::GetLevelName(const ULevel* Level)
{
	return Level->GetOutermost()->GetName();
}

void EssUtil::SetGuid(UObject* Obj, const FGuid& NewGuid, FProperty* Prop)
{
	FGuid* GuidPtr = Prop->ContainerPtrToValuePtr<FGuid>(Obj);
	*GuidPtr = NewGuid;
}
