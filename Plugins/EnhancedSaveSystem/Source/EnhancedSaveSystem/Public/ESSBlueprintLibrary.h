// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ESSSaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ESSBlueprintLibrary.generated.h"

struct FObjectAndNameAsStringProxyArchive;
class UESSSaveGame;
class UActorComponent;

/**
* Holds all functions belonging to the save system
*/
UCLASS()
class ENHANCEDSAVESYSTEM_API UESSBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* Save variables that are marked as "SaveGame" of all actors and their components in the world which implement the "SavableInterface" interface.
	* 
	* @param SaveGame Save game object to save to.
	* @param SlotName Save game slot to save to.
	* @param UserIndex Index used to identify the user doing the saving.
	* @return Saved successfully.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (WorldContext = "WorldContextObject"))
	static bool SaveActorsInWorld(const UObject* WorldContextObject, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex = 0);

	/**
	* Load variables that are marked as "SaveGame" of all actors and their components in the world which implement the "SavableInterface" interface.
	*
	* @param SlotName Save game slot to load from.
	* @param UserIndex Index used to identify the user doing the saving.
	* @param bDestroyLoad Determines the method for loading actor data. If enabled, it will destroy all savable actors in the world, respawn them, and 
	restore the save data. Else it will load save data of savable actors currently in the world, respawn actors that were destroyed 
	after the world was saved, and redestroy any actors that were destroyed but were placed in the level manually.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (WorldContext = "WorldContextObject"))
	static void LoadActorsInWorld(const UObject* WorldContextObject, const FString& SlotName, const int32 UserIndex = 0, bool bDestroyLoad = false);

	/**
	* Save an actor and its components' variables that are marked as "SaveGame".
	* Can be used to save a specific actor.
	* It will automatically save the actor in the correct data structure based on the presence of a GUID in the ESSSavableComponent.
	* 
	* @param Actor Actor to be saved.
	* @param SaveGame Save game object to save to.
	* @param SlotName Save game slot to save to.
	* @param UserIndex Index used to identify the user doing the saving.
	* @return Saved successfully.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System")
	static bool SaveActor(AActor* Actor, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex = 0);

	/**
	* Load an actor and its components' variables that are marked as “SaveGame”.
	* Can be used to load a specific actor. Actor needs to be in the world when this function is called.
	* It will automatically load the actor's data from the correct data structure based on the presence of a GUID in the ESSSavableComponent.
	* 
	* @param Actor Actor to restore data to.
	* @param SlotName Save game slot to load from.
	* @param UserIndex Index used to identify the user doing the saving.
	* @param bDestroyLoad Determines the method for loading actor data. If enabled, it will destroy the actor, respawn it, and 
	restore the save data. Else it will load save data of the actor that is currently in the world.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (WorldContext = "WorldContextObject"))
	static void LoadActor(const UObject* WorldContextObject, AActor* Actor, const FString& SlotName, const int32 UserIndex = 0, bool bDestroyLoad = false);

	/**
	* Save the actor and its components' variables that are marked as "SaveGame".
	*
	* @param Archive Archive used to serialize data.
	* @param Components Components to be saved.
	*/
	static void SerializeComponents(FObjectAndNameAsStringProxyArchive& Archive, TArray<UActorComponent*> Components);

	/**
	* Save an object's variables that are marked as "SaveGame".
	* Should only be used to save objects that aren't actors.
	*
	* @param Object Object to be saved.
	* @param SaveGame Save game object to save to.
	* @param SlotName Save game slot to save to.
	* @param UserIndex Index used to identify the user doing the saving.
	* @return Saved successfully.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System")
	static bool SaveObject(UObject* Object, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex = 0);

	/**
	* Load an object's variables that are marked as “SaveGame”.
	* Should only be used to load objects that aren't actors. Object needs to be in the world when this function is called.
	* 
	* @param Object Object to restore data to.
	* @param SlotName Save game slot to load from.
	* @param UserIndex Index used to identify the user doing the saving.
	*/
	UFUNCTION(BlueprintCallable, Category = "Save System")
	static void LoadObject(UObject* Object, const FString& SlotName, const int32 UserIndex = 0);

private:
	/**
	* Save an actor and its components' variables that are marked as "SaveGame".
	* Can be used to save a specific actor.
	*
	* @param Actor Actor to be saved.
	* @param SaveGame Save game object to save data to.
	*/
	static FActorSaveData SaveActorData(AActor* Actor);

	/**
	* Load an actor and its components' variables that are marked as “SaveGame”.
	* Can be used to load a specific actor.
	*
	* @param Actor Actor to restore data to.
	* @param ActorData Actor data to load.
	*/
	static void LoadActorData(AActor* Actor, FActorSaveData& ActorData);

	/**
	* Save an object's variables that are marked as "SaveGame".
	* Should only be used to save objects that aren't actors.
	*
	* @param Object Object to be saved.
	* @param SaveGame Save game object to save data to.
	*/
	static void SaveObjectData(UObject* Object, UESSSaveGame* SaveGame);

	/**
	* Load an object's variables that are marked as “SaveGame”.
	* Should only be used to load objects that aren't actors.
	*
	* @param Object Object to restore data to.
	* @param ObjectData Object data to load.
	*/
	static void LoadObjectData(UObject* Object, FObjectSaveData& ObjectData);

};
