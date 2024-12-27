// Copyright 2023 devran. All Rights Reserved.


#include "EssSubsystem.h"

#include "EssSavableInterface.h"
#include "EssSaveData.h"
#include "EssSaveGame.h"
#include "EssUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

void UEssSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEssSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEssSubsystem::SaveGame(const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("World not saved. SlotName is empty."));
		return false;
	}

	UEssSaveGame* SaveGame = GetSaveGameAndCreateIfNotExists(SlotName, UserIndex);
	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("World not saved. SaveGame is not valid."));
		return false;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("World not saved. World is not valid."));
		return false;
	}

	FString WorldName = World->GetFName().ToString();

	SaveGame->DeleteWorldData(SlotName, WorldName);

	FEssWorldData WorldData;
	WorldData.Name = WorldName;

	for (auto Level : World->GetLevels())
	{
		FEssLevelData LevelData = GetLevelData(Level);
		WorldData.LevelsData.Add(LevelData.Name, LevelData);
	}

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);
	if (FoundSaveData)
	{
		FoundSaveData->WorldsData.Add(WorldData.Name, WorldData);
		FEssSaveSlotData* FoundSaveSlotData = SaveGame->SaveSlotsData.Find(SlotName);
		FoundSaveSlotData->DateTimeOfSave = FDateTime::Now();
	}
	else
	{
		FEssSaveData SaveData;
		SaveData.SlotName = SlotName;
		SaveData.WorldsData.Add(WorldData.Name, WorldData);

		FEssSaveSlotData SaveSlotData;
		SaveSlotData.SlotName = SlotName;
		SaveSlotData.DateTimeOfSave = FDateTime::Now();

		SaveGame->SaveSlotsData.Add(SlotName, SaveSlotData);
		SaveGame->SaveData.Add(SlotName, SaveData);
	}

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

	if (bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("World saved."));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("World not saved."));
	return false;
}

bool UEssSubsystem::LoadGame(const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("World not loaded. SlotName is empty."));
		return false;
	}

	TObjectPtr<UWorld> World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("World not loaded. World is not valid."));
		return false;
	}

	UEssSaveGame* SaveGame = GetSaveGame(SlotName, UserIndex);
	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("World not loaded. SaveGame is not valid."));
		return false;
	}

	FEssSaveData* SaveData = SaveGame->SaveData.Find(SlotName);
	FEssWorldData* WorldData = SaveData->WorldsData.Find(World->GetFName().ToString());

	if (WorldData)
	{
		for (auto Level : World->GetLevels())
		{
			FEssLevelData* LevelData = WorldData->LevelsData.Find(Level->GetOutermost()->GetName());
			RestoreLevelData(Level, LevelData);
		}

		UE_LOG(LogTemp, Warning, TEXT("World loaded."));
		return true;
	}

	return false;
}

bool UEssSubsystem::DeleteSave(const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Save not deleted. SlotName is empty."));
		return false;
	}

	return GetSaveGame(SlotName, UserIndex)->DeleteSave(SlotName);
}

bool UEssSubsystem::SaveGlobalObject(UObject* Obj, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not saved. SlotName is empty."));
		return false;
	}

	UEssSaveGame* SaveGame = GetSaveGameAndCreateIfNotExists(SlotName, UserIndex);
	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("World not saved. SaveGame is not valid."));
		return false;
	}

	if (!IsValid(Obj) || !Obj->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
		return false;

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not saved. Object doesn't have a valid GUID set."));
		return false;
	}

	FEssGlobalObjectData ObjectData = ExtractGlobalObjectData(Obj);
	if (!ObjectData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not saved. Save data couldn't be extracted."));
		return false;
	}

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);
	if (FoundSaveData)
	{
		FoundSaveData->GlobalObjectData.RemoveSwap(ObjectData);
		FoundSaveData->GlobalObjectData.Add(ObjectData);
		FEssSaveSlotData* FoundSaveSlotData = SaveGame->SaveSlotsData.Find(SlotName);
		FoundSaveSlotData->DateTimeOfSave = FDateTime::Now();
	}
	else
	{
		FEssSaveData SaveData;
		SaveData.SlotName = SlotName;
		SaveData.GlobalObjectData.Add(ObjectData);

		FEssSaveSlotData SaveSlotData;
		SaveSlotData.SlotName = SlotName;
		SaveSlotData.DateTimeOfSave = FDateTime::Now();

		SaveGame->SaveSlotsData.Add(SlotName, SaveSlotData);
		SaveGame->SaveData.Add(SlotName, SaveData);
	}

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

	if (bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object saved."));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Global object not saved."));
	return false;
}

bool UEssSubsystem::LoadGlobalObject(UObject* Obj, const FString& SlotName, const int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not loaded. SlotName is empty."));
		return false;
	}

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not loaded. SaveGame does not exist."));
		return false;
	}

	UEssSaveGame* SaveGame = GetSaveGame(SlotName, UserIndex);
	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not loaded. SaveGame is not valid."));
		return false;
	}

	if (!IsValid(Obj) || !Obj->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
		return false;

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object not loaded. Object doesn't have a valid GUID set."));
		return false;
	}

	FEssSaveData* FoundSaveData = SaveGame->SaveData.Find(SlotName);

	for (auto& ObjectData : FoundSaveData->GlobalObjectData)
	{
		if (ObjectData.Guid == Guid)
		{
			RestoreGlobalObjectData(ObjectData, Obj);
			return true;
		}
	}

	return false;
}

FEssLevelData UEssSubsystem::GetLevelData(const TObjectPtr<ULevel> Level)
{
	// TODO: Get current data for this level for backup in case save fails

	FEssLevelData LevelData;
	LevelData.Name = Level->GetOutermost()->GetName();

	for (auto Actor : Level->Actors)
	{
		if (!IsValid(Actor) || !Actor->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
			continue;

		if (EssUtil::IsRuntimeActor(Actor))
		{
			FEssRuntimeActorData ActorData = ExtractRuntimeActorData(Actor);
			if (ActorData)
				LevelData.RuntimeActorsData.Add(ActorData);
		}
		else
		{
			FEssPlacedActorData ActorData = ExtractPlacedActorData(Actor);
			if (ActorData)
				LevelData.PlacedActorsData.Add(ActorData.Name, ActorData);
		}
	}

	return LevelData;
}

void UEssSubsystem::RestoreLevelData(TObjectPtr<ULevel> Level, const FEssLevelData* LevelData)
{
	TArray<FEssPlacedActorData> TempPlacedActorsData;
	LevelData->PlacedActorsData.GenerateValueArray(TempPlacedActorsData);
	TArray<AActor*> PlacedActorsToBeDestroyed;

	for (auto Actor : Level->Actors)
	{
		if (!IsValid(Actor) || !Actor->GetClass()->ImplementsInterface(UEssSavableInterface::StaticClass()))
			continue;

		if (EssUtil::IsRuntimeActor(Actor))
		{
			UE_LOG(LogTemp, Display, TEXT("Runtime actor %s being destroyed."), *Actor->GetFName().ToString());
			Actor->Destroy();
		}
		else
		{
			for (int32 i = TempPlacedActorsData.Num() - 1; i >= 0; --i)
			{
				if (TempPlacedActorsData[i].Name == Actor->GetFName())
				{
					TempPlacedActorsData.RemoveAt(i);
				}
			}

			const FEssPlacedActorData* PlacedActorSaveData = LevelData->PlacedActorsData.Find(Actor->GetFName());
			if (!PlacedActorSaveData)
				PlacedActorsToBeDestroyed.Add(Actor);

			const FEssPlacedActorData* ActorData = LevelData->PlacedActorsData.Find(Actor->GetFName());
			if (ActorData)
			{
				RestorePlacedActorData(*ActorData, Actor);
				Cast<IEssSavableInterface>(Actor)->Execute_PostLoadGame(Actor);
			}
		}
	}

	// Respawn runtime actors with save data
	for (auto& ActorData : LevelData->RuntimeActorsData)
	{
		RespawnRuntimeActor(ActorData, Level);
	}

	// Respawn placed actors with save data
	for (auto& ActorData : TempPlacedActorsData)
	{
		RespawnPlacedActor(ActorData, Level);
	}

	// Redestroy placed actors with no save data
	for (auto PlacedActor : PlacedActorsToBeDestroyed)
	{
		UE_LOG(LogTemp, Display, TEXT("Placed actor %s being destroyed."), *PlacedActor->GetFName().ToString());
		PlacedActor->Destroy();
	}
}

FEssRuntimeActorData UEssSubsystem::ExtractRuntimeActorData(TObjectPtr<AActor> Actor)
{
	FEssRuntimeActorData ActorData;

	if (Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_BeginDestroyed))
		return ActorData;

	Cast<IEssSavableInterface>(Actor)->Execute_PreSaveGame(Actor);

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

	Cast<IEssSavableInterface>(Actor)->Execute_PreSaveGame(Actor);

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

FEssGlobalObjectData UEssSubsystem::ExtractGlobalObjectData(TObjectPtr<UObject> Obj)
{
	FEssGlobalObjectData ObjectData;

	if (Obj->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_BeginDestroyed))
		return ObjectData;

	Cast<IEssSavableInterface>(Obj)->Execute_PreSaveGame(Obj);

	FGuid Guid = EssUtil::GetGuid(Obj);
	if (!Guid.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Global object %s has no EssGuid value set and can therefore not be saved."), *Obj->GetFName().ToString());
		return ObjectData;
	}

	ObjectData.Guid = Guid;
	ObjectData.Class = Obj->GetClass();

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
		if (IsValid(Comp)) Comp->Serialize(Archive);
	}
}

void UEssSubsystem::RespawnRuntimeActor(const FEssRuntimeActorData& ActorData, const TObjectPtr<ULevel> Level)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = Level;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TObjectPtr<UWorld> World = Level->GetWorld();
	TObjectPtr<AActor> SpawnedActor = World->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);
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

	TObjectPtr<UWorld> World = Level->GetWorld();
	TObjectPtr<AActor> SpawnedActor = World->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);\

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

	Cast<IEssSavableInterface>(Obj)->Execute_PostLoadGame(Obj);
}

UEssSaveGame* UEssSubsystem::GetSaveGameAndCreateIfNotExists(const FString& SlotName, const int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame does not exist. Creating new save game object."));

		UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::CreateSaveGameObject(UEssSaveGame::StaticClass()));
		if (!IsValid(SaveGame))
		{
			UE_LOG(LogTemp, Warning, TEXT("SaveGame is not valid."));
			return nullptr;
		}

		return SaveGame;
	}
	
	UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame is not valid."));
		return nullptr;
	}

	return SaveGame;
}

UEssSaveGame* UEssSubsystem::GetSaveGame(const FString& SlotName, const int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame does not exist. Creating new save game object."));
		return nullptr;
	}

	UEssSaveGame* SaveGame = Cast<UEssSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

	if (!IsValid(SaveGame))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame is not valid."));
		return nullptr;
	}

	return SaveGame;
}