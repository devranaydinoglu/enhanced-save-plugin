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

	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool SaveGame(const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool LoadGame(const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool DeleteSave(const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Save System")
	bool SaveGlobalObject(UObject* Obj, const FString& SlotName, const int32 UserIndex = 0);

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
