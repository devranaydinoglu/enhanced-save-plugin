// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Templates/SubClassOf.h"
#include "ESSSaveGame.generated.h"

/**
* Holds data of an object.
*/
USTRUCT(BlueprintType)
struct FObjectSaveData
{
	GENERATED_BODY()

public:
	FObjectSaveData() : Name(""), Class(nullptr) {};

	UPROPERTY()
	FName Name;

	UPROPERTY()
	TSubclassOf<UObject> Class;

	// Contains all "SaveGame" marked variables of the actor
	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FObjectSaveData& Other) const
	{
		return Name == Other.Name;
	}
};

/**
* Holds data of an actor.
*/
USTRUCT(BlueprintType)
struct FActorSaveData
{
	GENERATED_BODY()

public:
	FActorSaveData() : Name(""), Class(nullptr), Transform(FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f))) {};

	UPROPERTY()
	FName Name;

	UPROPERTY()
	TSubclassOf<AActor> Class;

	UPROPERTY()
	FTransform Transform;

	// Contains all "SaveGame" marked variables of the actor
	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FActorSaveData& Other) const
	{
		return Name == Other.Name;
	}
};

/**
* Save game to be inherited from for the save system to work
*/
UCLASS()
class ENHANCEDSAVESYSTEM_API UESSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FActorSaveData> ActorsInWorldSaveData;

	UPROPERTY()
	TArray<FActorSaveData> ActorSaveData;

	UPROPERTY()
	TArray<FObjectSaveData> ObjectSaveData;
};
