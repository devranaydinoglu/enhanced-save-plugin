// Copyright 2023 devran. All Rights Reserved.

#include "EssSubsystem.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "EssSavableInterface.h"
#include "EssSaveData.h"
#include "EssSaveGame.h"
#include "EssUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "EssLog.h"

void UEssSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEssSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEssSubsystem::SaveWorld(UEssSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "World not saved. SlotName is empty.");
		return false;
	}

	if (!IsValid(SaveGame))
	{
		SaveGame = GetSaveGameAndCreateIfNotExists(SlotName, UserIndex);
		if (!IsValid(SaveGame))
		{
			ESS_LOG(Warning, "World not saved. SaveGame is not valid.");
			return false;
		}
	}

	UWorld* World = GetWorld();
	FString WorldName = World->GetFName().ToString();

	SaveGame->DeleteWorldData(SlotName, WorldName);

	FEssWorldData WorldData;
	WorldData.Name = WorldName;

	for (ULevel* Level : World->GetLevels())
	{
		FEssLevelData LevelData = GetLevelData(Level);
		WorldData.LevelsData.Add(LevelData.Name, LevelData);
	}

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);
	if (FoundSaveData)
	{
		FoundSaveData->WorldsData.Add(WorldData.Name, WorldData);
		FEssSaveSlotData* FoundSaveSlotData = SaveGame->SaveSlotsData.Find(SlotName);
		FoundSaveSlotData->Timestamp = FDateTime::Now();
	}
	else
	{
		FEssSaveData SaveData;
		SaveData.SlotName = SlotName;
		SaveData.WorldsData.Add(WorldData.Name, WorldData);

		FEssSaveSlotData SaveSlotData;
		SaveSlotData.SlotName = SlotName;
		SaveSlotData.Timestamp = FDateTime::Now();

		SaveGame->SaveSlotsData.Add(SlotName, SaveSlotData);
		SaveGame->SaveData.Add(SlotName, SaveData);
	}

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

	if (bSaved)
	{
		ESS_LOG(Warning, "World saved.");
		return true;
	}

	ESS_LOG(Warning, "World not saved.");
	return false;
}

bool UEssSubsystem::LoadWorld(UEssSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "World not loaded. SlotName is empty.");
		return false;
	}

	if (!IsValid(SaveGame))
	{
		SaveGame = GetSaveGame(SlotName, UserIndex);
		if (!IsValid(SaveGame))
		{
			ESS_LOG(Warning, "World not loaded. SaveGame is not valid.");
			return false;
		}
	}

	UWorld* World = GetWorld();

	FEssSaveData* SaveData = SaveGame->SaveData.Find(SlotName);
	FEssWorldData* WorldData = SaveData->WorldsData.Find(World->GetFName().ToString());

	if (WorldData)
	{
		for (ULevel* Level : World->GetLevels())
		{
			FEssLevelData* LevelData = WorldData->LevelsData.Find(EssUtil::GetLevelName(Level));
			if (LevelData)
				RestoreLevelData(Level, LevelData);
		}

		ESS_LOG(Warning, "World loaded.");
		return true;
	}

	return false;
}

bool UEssSubsystem::DeleteSave(const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "Save not deleted. SlotName is empty.");
		return false;
	}

	return GetSaveGame(SlotName, UserIndex)->DeleteSave(SlotName);
}

bool UEssSubsystem::SaveGlobalObject(UEssSaveGame* SaveGame, UObject* Obj, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "Global object %s not saved. SlotName is empty.", *Obj->GetFName().ToString());
		return false;
	}

	if (!IsValid(Obj) || !Obj->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
		return false;

	if (!IsValid(SaveGame))
	{
		SaveGame = GetSaveGameAndCreateIfNotExists(SlotName, UserIndex);
		if (!IsValid(SaveGame))
		{
			ESS_LOG(Warning, "Global object %s not saved. SaveGame is not valid.", *Obj->GetFName().ToString());
			return false;
		}
	}

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		ESS_LOG(Warning, "Global object %s not saved. Object doesn't have a valid GUID set.", *Obj->GetFName().ToString());
		return false;
	}

	FEssGlobalObjectData ObjectData = ExtractGlobalObjectData(Obj);
	if (!ObjectData)
	{
		ESS_LOG(Warning, "Global object %s not saved. Save data couldn't be extracted.", *Obj->GetFName().ToString());
		return false;
	}

	Cast<IEssSavableInterface>(Obj)->Execute_PreSaveGame(Obj);

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);
	if (FoundSaveData)
	{
		FoundSaveData->GlobalObjectData.RemoveSwap(ObjectData);
		FoundSaveData->GlobalObjectData.Add(ObjectData);
		FEssSaveSlotData* FoundSaveSlotData = SaveGame->SaveSlotsData.Find(SlotName);
		FoundSaveSlotData->Timestamp = FDateTime::Now();
	}
	else
	{
		FEssSaveData SaveData;
		SaveData.SlotName = SlotName;
		SaveData.GlobalObjectData.Add(ObjectData);

		FEssSaveSlotData SaveSlotData;
		SaveSlotData.SlotName = SlotName;
		SaveSlotData.Timestamp = FDateTime::Now();

		SaveGame->SaveSlotsData.Add(SlotName, SaveSlotData);
		SaveGame->SaveData.Add(SlotName, SaveData);
	}

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

	if (bSaved)
	{
		ESS_LOG(Warning, "Global object %s saved.", *Obj->GetFName().ToString());
		Cast<IEssSavableInterface>(Obj)->Execute_PostSaveGame(Obj);
		return true;
	}

	ESS_LOG(Warning, "Global object %s not saved.", *Obj->GetFName().ToString());
	return false;
}

bool UEssSubsystem::LoadGlobalObject(UEssSaveGame* SaveGame, UObject* Obj, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "Global object %s not loaded. SlotName is empty.", *Obj->GetFName().ToString());
		return false;
	}

	if (!IsValid(Obj) || !Obj->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
		return false;

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		ESS_LOG(Warning, "Global object %s not loaded. SaveGame does not exist.", *Obj->GetFName().ToString());
		return false;
	}

	if (!IsValid(SaveGame))
	{
		SaveGame = GetSaveGame(SlotName, UserIndex);
		if (!IsValid(SaveGame))
		{
			ESS_LOG(Warning, "Global object %s not loaded. SaveGame is not valid.", *Obj->GetFName().ToString());
			return false;
		}
	}

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		ESS_LOG(Warning, "Global object %s not loaded. Object doesn't have a valid GUID set.", *Obj->GetFName().ToString());
		return false;
	}

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);

	for (auto& ObjectData : FoundSaveData->GlobalObjectData)
	{
		if (ObjectData.Guid == Guid)
		{
			RestoreGlobalObjectData(ObjectData, Obj);
			Cast<IEssSavableInterface>(Obj)->Execute_PostLoadGame(Obj);
			return true;
		}
	}

	return false;
}

FDateTime UEssSubsystem::GetSaveSlotTimestamp(const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ESS_LOG(Warning, "Can't get save slot data. SlotName is empty.");
		return FDateTime(0);
	}

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		ESS_LOG(Warning, "Can't get save slot data. SaveGame does not exist.");
		return FDateTime(0);
	}

	UEssSaveGame* SaveGame = GetSaveGame(SlotName, UserIndex);
	if (!IsValid(SaveGame))
	{
		ESS_LOG(Warning, "Can't get save slot data. SaveGame is not valid.");
		return FDateTime(0);
	}

	FEssSaveSlotData* SaveSlotData = SaveGame->GetSaveSlotData(SlotName);
	if (!SaveSlotData)
		return FDateTime(0);

	return SaveSlotData->Timestamp;
}

FEssLevelData UEssSubsystem::GetLevelData(const TObjectPtr<ULevel> Level)
{
	// TODO: Get current data for this level for backup in case save fails

	FEssLevelData LevelData;
	LevelData.Name = EssUtil::GetLevelName(Level);

	for (auto Actor : Level->Actors)
	{
		if (!IsValid(Actor) || !Actor->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
			continue;

		if (EssUtil::IsRuntimeActor(Actor))
		{
			if (EssUtil::IsActorRespawnable(Actor))
			{
				Cast<IEssSavableInterface>(Actor)->Execute_PreSaveGame(Actor);
				FEssRuntimeActorData ActorData = ExtractRuntimeActorData(Actor);
				if (ActorData)
				{
					LevelData.RuntimeActorsData.Add(ActorData);
					ESS_LOG(Warning, "Runtime actor %s data saved.", *Actor->GetFName().ToString());
					Cast<IEssSavableInterface>(Actor)->Execute_PostSaveGame(Actor);
				}
			}
			else
			{
				FGuid Guid = EssUtil::GetGuid(Actor);
				if (Guid.IsValid())
				{
					Cast<IEssSavableInterface>(Actor)->Execute_PreSaveGame(Actor);
					FEssRuntimeActorData ActorData = ExtractRuntimeActorData(Actor);
					if (ActorData)
					{
						LevelData.RuntimeActorsData.Add(ActorData);
						ESS_LOG(Warning, "Runtime actor %s data saved.", *Actor->GetFName().ToString());
						Cast<IEssSavableInterface>(Actor)->Execute_PostSaveGame(Actor);
					}
				}
			}
		}
		else
		{
			Cast<IEssSavableInterface>(Actor)->Execute_PreSaveGame(Actor);
			FEssPlacedActorData ActorData = ExtractPlacedActorData(Actor);
			if (ActorData)
			{
				LevelData.PlacedActorsData.Add(ActorData.Name, ActorData);
				ESS_LOG(Warning, "Placed actor %s data saved.", *Actor->GetActorLabel());
				PrintActorProperties(Actor);
				Cast<IEssSavableInterface>(Actor)->Execute_PostSaveGame(Actor);
			}
		}
	}

	return LevelData;
}

void UEssSubsystem::RestoreLevelData(TObjectPtr<ULevel> Level, const FEssLevelData* LevelData)
{
	TArray<FEssPlacedActorData> RespawnablePlacedActorsData;
	LevelData->PlacedActorsData.GenerateValueArray(RespawnablePlacedActorsData);
	TArray<AActor*> PlacedActorsToBeDestroyed;

	for (auto Actor : Level->Actors)
	{
		if (!IsValid(Actor) || !Actor->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
			continue;

		if (EssUtil::IsRuntimeActor(Actor))
		{
			if (EssUtil::IsActorRespawnable(Actor))
			{
				if (Actor->Destroy())
					ESS_LOG(Warning, "Runtime actor %s was destroyed.", *Actor->GetFName().ToString());
			}
			else
			{
				FGuid Guid = EssUtil::GetGuid(Actor);
				if (Guid.IsValid())
				{
					for (const auto& ActorData : LevelData->RuntimeActorsData)
					{
						if (Guid == ActorData.Guid)
							RestoreRuntimeActorData(ActorData, Actor);
					}
				}
			}
		}
		else
		{
			// RespawnablePlacedActorsData is used to respawn placed actors with save data. If the game is
			// loaded during play (e.g. player dies), the placed actors that had been destroyed need to be
			// respawned. So we ignore placed actors that are still in the world and don't need to be respawned.
			for (int32 i = RespawnablePlacedActorsData.Num() - 1; i >= 0; --i)
			{
				if (RespawnablePlacedActorsData[i].Name == Actor->GetFName())
				{
					RespawnablePlacedActorsData.RemoveAt(i);
				}
			}

			// Placed actors that got destroyed before a save will be in the world when a level is reloaded but
			// won't have save data since they got destroyed before the save. These need to be restroyed when
			// a level is reloaded.
			const FEssPlacedActorData* PlacedActorSaveData = LevelData->PlacedActorsData.Find(Actor->GetFName());
			if (!PlacedActorSaveData)
			{
				PlacedActorsToBeDestroyed.Add(Actor);
			}
			else
			{
				RestorePlacedActorData(*PlacedActorSaveData, Actor);
				Cast<IEssSavableInterface>(Actor)->Execute_PostLoadGame(Actor);
			}
		}
	}

	// Respawn runtime actors with save data
	for (auto& ActorData : LevelData->RuntimeActorsData)
	{
		if (EssUtil::IsActorRespawnable(ActorData.Class))
			RespawnRuntimeActor(ActorData, Level);
	}

	// Respawn placed actors with save data
	for (auto& ActorData : RespawnablePlacedActorsData)
	{
		RespawnPlacedActor(ActorData, Level);
	}

	// Redestroy placed actors with no save data
	for (auto PlacedActor : PlacedActorsToBeDestroyed)
	{
		if (PlacedActor->Destroy())
			ESS_LOG(Warning, "Placed actor %s was destroyed.", *PlacedActor->GetActorLabel());
	}
}

FEssRuntimeActorData UEssSubsystem::ExtractRuntimeActorData(TObjectPtr<AActor> Actor)
{
	FEssRuntimeActorData ActorData;

	if (Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_BeginDestroyed))
		return ActorData;

	FGuid Guid = EssUtil::GetGuid(Actor);
	if (!Guid.IsValid())
	{
		Guid = FGuid::NewGuid();
		if (!EssUtil::SetGuid(Actor, Guid))
			return ActorData;
	}

	ActorData.Guid = Guid;
	ActorData.Class = Actor->GetClass();
	ActorData.Transform = Actor->GetActorTransform();

	// Pass byte array to fill with data
	FMemoryWriter MemoryWriter(ActorData.ByteData);

	// Find variables with SaveGame property
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert actor variables to binary data
	Actor->Serialize(Archive);

	// Convert actor components' variables to binary data
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UEssSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	return ActorData;
}

FEssPlacedActorData UEssSubsystem::ExtractPlacedActorData(TObjectPtr<AActor> Actor)
{
	FEssPlacedActorData ActorData;

	if (Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_BeginDestroyed))
		return ActorData;

	ActorData.Name = Actor->GetFName();
	ActorData.Class = Actor->GetClass();
	ActorData.Transform = Actor->GetActorTransform();

	// Pass byte array to fill with data
	FMemoryWriter MemoryWriter(ActorData.ByteData);

	// Find variables with SaveGame property
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert actor variables to binary data
	Actor->Serialize(Archive);

	// Convert actor components' variables to binary data
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UEssSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	return ActorData;
}

FEssGlobalObjectData UEssSubsystem::ExtractGlobalObjectData(UObject* Obj)
{
	FEssGlobalObjectData ObjectData;

	if (Obj->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_BeginDestroyed))
		return ObjectData;

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		ESS_LOG(Warning, "Global object %s has no EssGuid value set and can therefore not be saved.", *Obj->GetFName().ToString());
		return ObjectData;
	}

	ObjectData.Guid = Guid;
	ObjectData.Class = Obj->GetClass();
	/*ObjectData.LevelName = EssUtil::GetLevelName(Obj->Level);*/

	// Pass byte array to fill with data
	FMemoryWriter MemoryWriter(ObjectData.ByteData);

	// Find variables with SaveGame property
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert object variables to binary data
	Obj->Serialize(Archive);

	return ObjectData;
}

void UEssSubsystem::SerializeComponents(FObjectAndNameAsStringProxyArchive& Archive, TArray<UActorComponent*> Components)
{
	for (UActorComponent* Comp : Components)
	{
		if (IsValid(Comp))
			Comp->Serialize(Archive);
	}
}

void UEssSubsystem::RespawnRuntimeActor(const FEssRuntimeActorData& ActorData, const TObjectPtr<ULevel> Level)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Level;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = Level->GetWorld()->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);
	if (IsValid(SpawnedActor))
	{
		RestoreRuntimeActorData(ActorData, SpawnedActor);
		Cast<IEssSavableInterface>(SpawnedActor)->Execute_PostLoadGame(SpawnedActor);
	}
}

void UEssSubsystem::RespawnPlacedActor(const FEssPlacedActorData& ActorData, const TObjectPtr<ULevel> Level)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Level;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = Level->GetWorld()->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);\

	if (IsValid(SpawnedActor))
	{
		RestorePlacedActorData(ActorData, SpawnedActor);
		Cast<IEssSavableInterface>(SpawnedActor)->Execute_PostLoadGame(SpawnedActor);
	}
}

void UEssSubsystem::RestoreRuntimeActorData(const FEssRuntimeActorData& ActorData, TObjectPtr<AActor> Actor)
{
	EssUtil::SetGuid(Actor, ActorData.Guid);

	Actor->SetActorTransform(ActorData.Transform);

	// Pass saved byte array to read from
	FMemoryReader MemoryReader(ActorData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert actor binary data back to variables
	Actor->Serialize(Archive);

	// Convert actor components' binary data back to variables
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UEssSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	ESS_LOG(Warning, "Runtime actor %s data loaded.", *Actor->GetFName().ToString());
}

void UEssSubsystem::RestorePlacedActorData(const FEssPlacedActorData& ActorData, TObjectPtr<AActor> Actor)
{
	Actor->SetActorTransform(ActorData.Transform);

	// Pass saved byte array to read from
	FMemoryReader MemoryReader(ActorData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;
	
	// Convert actor binary data back to variables
	Actor->Serialize(Archive);

	// Convert actor components' binary data back to variables
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UEssSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	ESS_LOG(Warning, "Placed actor %s data loaded.", *Actor->GetActorLabel());

	PrintActorProperties(Actor);
}

void UEssSubsystem::RestoreGlobalObjectData(const FEssGlobalObjectData& ObjectData, TObjectPtr<UObject> Obj)
{
	// Pass saved byte array to read from
	FMemoryReader MemoryReader(ObjectData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert obj binary data back to variables
	Obj->Serialize(Archive);

	ESS_LOG(Warning, "Global object %s data loaded.", *Obj->GetFName().ToString());
}

UEssSaveGame* UEssSubsystem::GetSaveGameAndCreateIfNotExists(const FString& SlotName, const int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		ESS_LOG(Warning, "SaveGame does not exist. Creating new save game object.");

		UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::CreateSaveGameObject(UEssSaveGame::StaticClass()));
		return SaveGame;
	}
	
	UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	return SaveGame;
}

UEssSaveGame* UEssSubsystem::GetSaveGame(const FString& SlotName, const int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		ESS_LOG(Warning, "SaveGame does not exist");
		return nullptr;
	}

	UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	return SaveGame;
}

void UEssSubsystem::PrintActorProperties(const AActor* Actor)
{
	if (!IsValid(Actor))
		return;

	UClass* ActorClass = Actor->GetClass();
	FString PropertyLog;
	
	for (TFieldIterator<FProperty> PropIt(ActorClass); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		PropertyLog = *Property->GetName();
		PropertyLog.Append(": ");

		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			bool Value = BoolProp->GetPropertyValue_InContainer(Actor);
			PropertyLog.Append(Value ? "true" : "false");
			ESS_LOG(Display, "%s", *PropertyLog);
		}
	}
}
