// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EssSaveData.generated.h"

USTRUCT()
struct FEssSaveSlotData
{
	GENERATED_BODY()

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	FDateTime DateTimeOfSave;
};

USTRUCT()
struct FEssRuntimeActorData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	TSubclassOf<AActor> Class;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FEssRuntimeActorData& Other)
	{
		return Guid == Other.Guid;
	}

	operator bool()
	{
		return Guid.IsValid();
	}
};

USTRUCT()
struct ENHANCEDSAVESYSTEM_API FEssPlacedActorData
{
	GENERATED_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	TSubclassOf<AActor> Class;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FEssPlacedActorData& Other) const
	{
		return Name == Other.Name;
	}

	operator bool()
	{
		return Name.IsValid();
	}
};

USTRUCT()
struct FEssGlobalObjectData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	TSubclassOf<UObject> Class;

	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FEssGlobalObjectData& Other)
	{
		return Guid == Other.Guid;
	}

	operator bool()
	{
		return Guid.IsValid();
	}
};

USTRUCT()
struct ENHANCEDSAVESYSTEM_API FEssLevelData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TArray<FEssRuntimeActorData> RuntimeActorsData;

	UPROPERTY()
	TMap<FName, FEssPlacedActorData> PlacedActorsData;
};

USTRUCT()
struct ENHANCEDSAVESYSTEM_API FEssWorldData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TMap<FString /*Level name*/, FEssLevelData> LevelsData;
};

USTRUCT()
struct ENHANCEDSAVESYSTEM_API FEssSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	TMap<FString /*World name*/, FEssWorldData> WorldsData;

	UPROPERTY()
	TArray<FEssGlobalObjectData> GlobalObjectData;
};
