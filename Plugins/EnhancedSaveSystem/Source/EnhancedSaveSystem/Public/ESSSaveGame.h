// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Templates/SubClassOf.h"
#include "GameFramework/Actor.h"
#include "EssSaveData.h"
#include "EssSaveGame.generated.h"

/**
* Save game to be inherited from for the save system to work
*/
UCLASS()
class ENHANCEDSAVESYSTEM_API UEssSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString /*Slot name*/, FEssSaveSlotData> SaveSlotsData;

	UPROPERTY()
	TMap<FString /*Slot name*/, FEssSaveData> SaveData;

public:
	bool DeleteWorldData(const FString& SlotName, const FString& WorldName);
	bool DeleteSave(const FString& SlotName);
	void UpdateSaveSlotData(const FString& SlotName);
};
