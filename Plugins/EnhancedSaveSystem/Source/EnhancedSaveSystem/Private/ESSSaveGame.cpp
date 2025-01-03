// Copyright 2023 devran. All Rights Reserved.

#include "EssSaveGame.h"

bool UEssSaveGame::DeleteWorldData(const FString& SlotName, const FString& WorldName)
{
	FEssSaveData* FoundSaveData = SaveData.Find(SlotName);
	if (FoundSaveData)
	{
		FoundSaveData->WorldsData.Remove(WorldName);

		if (FoundSaveData->WorldsData.Contains(SlotName))
		{
			UE_LOG(LogTemp, Warning, TEXT("World data could not be deleted."));
			return false;
		}
	}

	return true;
}

bool UEssSaveGame::DeleteSave(const FString& SlotName)
{
	if (!SaveData.Contains(SlotName))
		return false;
	SaveData.Remove(SlotName);

	if (!SaveSlotsData.Contains(SlotName))
		return false;
	SaveSlotsData.Remove(SlotName);

	return true;
}

void UEssSaveGame::UpdateSaveSlotData(const FString& SlotName)
{
	FEssSaveSlotData* FoundSaveSlotData = SaveSlotsData.Find(SlotName);
	FoundSaveSlotData->DateTimeOfSave = FDateTime::Now();
}
