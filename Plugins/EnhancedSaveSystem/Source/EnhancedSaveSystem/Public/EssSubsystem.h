// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EssSubsystem.generated.h"

struct FEssGlobalObjectData;
struct FEssPlacedActorData;
struct FEssRuntimeActorData;
struct FEssLevelData;
class UEssSaveGame;
struct FObjectAndNameAsStringProxyArchive;

UCLASS()
class ENHANCEDSAVESYSTEM_API UEssSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/**
	 * Saves variables that are marked as SaveGame of all actors and their components in the world which implement ESSSavableInterface.
	 * Automatically creates a new save game object if no corresponding one can be found based on the slot name.
	 * @param SlotName Save game slot to save to.
	 * @param UserIndex Index used to identify the user doing the saving.
	 * @return Saved successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool SaveWorld(const FString& SlotName, const int32 UserIndex);

	/**
	 * Loads variables that are marked as SaveGame of all actors and their components in the world which implement ESSSavableInterface.
	 * @param SlotName Save game slot to load from.
	 * @param UserIndex Index used to identify the user doing the loading.
	 * @return Loaded successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool LoadWorld(const FString& SlotName, const int32 UserIndex);

	/**
	 * Deletes all of the corresponding save data and save slot based on the slot name.
	 * @param SlotName Save game slot to delete.
	 * @param UserIndex Index used to identify the user doing the deleting.
	 * @return Deleted successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool DeleteSave(const FString& SlotName, const int32 UserIndex);

	/**
	 * Save an object's variables that are marked as SaveGame.
	 * This should be used to save objects not in the world (e.g. GameInstance) or special actors (e.g. GameMode, GameState, PlayerState).
	 * @param Obj Object to save.
	 * @param SlotName Save game slot to save to.
	 * @param UserIndex Index used to identify the user doing the saving.
	 * @return Saved successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool SaveGlobalObject(UObject* Obj, const FString& SlotName, const int32 UserIndex = 0);

	/**
	 * Load an object's variables that are marked as SaveGame.
	 * This should be used to load objects not in the world (e.g. GameInstance) or special actors (e.g. GameMode, GameState, PlayerState).
	 * @param Obj Object to load.
	 * @param SlotName Save game slot to load from.
	 * @param UserIndex Index used to identify the user doing the loading.
	 * @return Loaded successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool LoadGlobalObject(UObject* Obj, const FString& SlotName, const int32 UserIndex = 0);

protected:
	FEssLevelData GetLevelData(const TObjectPtr<ULevel> Level);
	void RestoreLevelData(TObjectPtr<ULevel> Level, const FEssLevelData* LevelData);
	FEssRuntimeActorData ExtractRuntimeActorData(TObjectPtr<AActor> Actor);
	FEssPlacedActorData ExtractPlacedActorData(TObjectPtr<AActor> Actor);
	FEssGlobalObjectData ExtractGlobalObjectData(TObjectPtr<UObject> Obj);
	void SerializeComponents(FObjectAndNameAsStringProxyArchive& Archive, TArray <UActorComponent*> Components);
	void RespawnRuntimeActor(const FEssRuntimeActorData& ActorData, const TObjectPtr<ULevel> Level);
	void RespawnPlacedActor(const FEssPlacedActorData& ActorData, const TObjectPtr<ULevel> Level);
	void RestoreRuntimeActorData(const FEssRuntimeActorData& ActorData, TObjectPtr<AActor> Actor);
	void RestorePlacedActorData(const FEssPlacedActorData& ActorData, TObjectPtr<AActor> Actor);
	void RestoreGlobalObjectData(const FEssGlobalObjectData& ObjectData, TObjectPtr<UObject> Obj);
	UEssSaveGame* GetSaveGameAndCreateIfNotExists(const FString& SlotName, const int32 UserIndex);
	UEssSaveGame* GetSaveGame(const FString& SlotName, const int32 UserIndex);
};
