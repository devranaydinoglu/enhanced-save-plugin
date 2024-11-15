// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Templates/SubClassOf.h"
#include "GameFramework/Actor.h"
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

	// Check whether the transform and class are equal
	bool SoftEquals(const FTransform& OtherTransform, TSubclassOf<AActor> OtherClass) const
	{
		return Transform.Equals(OtherTransform, 0.001f) && Class == OtherClass;
	}

	operator bool() const
	{
		return !Name.IsNone();
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
	// Stores save data of actors in the world whose savable component don't have a GUID (world actors)
	UPROPERTY()
	TArray<FActorSaveData> ActorsInWorldSaveData;

	// Stores save data of actors whose savable component have a GUID (unique actors) with the GUID as key
	UPROPERTY()
	TMap<FString, FActorSaveData> UniqueActorsSaveData;

	UPROPERTY()
	TArray<FObjectSaveData> ObjectSaveData;
};
