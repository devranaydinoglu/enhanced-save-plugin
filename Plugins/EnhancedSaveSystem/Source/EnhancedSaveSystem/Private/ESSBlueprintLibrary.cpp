// Copyright 2023 devran. All Rights Reserved.


#include "ESSBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/UnrealType.h"
#include "UObject/UnrealTypePrivate.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"

#include "ESSSaveGame.h"
#include "ESSSavableInterface.h"

bool UESSBlueprintLibrary::SaveActorsInWorld(const UObject* WorldContextObject, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex)
{
	if (IsValid(WorldContextObject) && IsValid(SaveGame))
	{
		SaveGame->ActorsInWorldSaveData.Empty();

		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

		// Get all savable actors
		TArray<AActor*> SavableActors;
		UGameplayStatics::GetAllActorsWithInterface(World, UESSSavableInterface::StaticClass(), SavableActors);

		// Save actors' save data
		for (AActor* Actor : SavableActors)
		{
			SaveGame->ActorsInWorldSaveData.Add(SaveActorData(Actor, SaveGame));
		}

		// Save SaveGame to save slot
		bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

		if (bSaved)
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("World saved!"));
#endif // !UE_BUILD_SHIPPING
			return true;
		}
		else
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("World not saved!"));
#endif // !UE_BUILD_SHIPPING
			return false;
		}
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("World not saved! Save game object not valid!"));
#endif // !UE_BUILD_SHIPPING
		return false;
	}
}

void UESSBlueprintLibrary::LoadActorsInWorld(const UObject* WorldContextObject, const FString& SlotName, const int32 UserIndex, bool bDestroyLoad)
{
	if (IsValid(WorldContextObject) && UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UESSSaveGame* SaveGame = Cast<UESSSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

		// Get all savable actors
		TArray<AActor*> SavableActors;
		UGameplayStatics::GetAllActorsWithInterface(World, UESSSavableInterface::StaticClass(), SavableActors);

		if (bDestroyLoad)
		{
			// Destroy savable actors in the level
			for (AActor* Actor : SavableActors)
			{
				Actor->Destroy();
			}

			// Respawn actors in the level which have saved data and restore the data
			for (FActorSaveData& ActorData : SaveGame->ActorsInWorldSaveData)
			{
				FActorSpawnParameters SpawnParams;
				AActor* SpawnedActor = World->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);

				if (IsValid(SpawnedActor))
				{
					LoadActorData(SpawnedActor, ActorData);
				}
			}
		}
		else
		{
			TArray<FActorSaveData> TempActorData = SaveGame->ActorsInWorldSaveData;

			// Load data of actors with save data that are currently in the world
			for (int32 i = TempActorData.Num() - 1; i >= 0; --i)
			{
				for (int32 j = SavableActors.Num() - 1; j >= 0; --j)
				{
					if (SavableActors[j]->GetFName() == TempActorData[i].Name || TempActorData[i].SoftEquals(SavableActors[j]->GetActorTransform(), SavableActors[j]->GetClass()))
					{
						LoadActorData(SavableActors[j], TempActorData[i]);
						SavableActors.RemoveAt(j, 1, true);
						TempActorData.RemoveAt(i, 1, true);
						break;
					}
				}
			}

			// Respawn actors that were destroyed after saving
			for (int32 i = TempActorData.Num() - 1; i >= 0; --i)
			{
				FActorSpawnParameters SpawnParams;
				AActor* SpawnedActor = World->SpawnActor<AActor>(TempActorData[i].Class, TempActorData[i].Transform, SpawnParams);
				if (IsValid(SpawnedActor)) LoadActorData(SpawnedActor, TempActorData[i]);
				TempActorData.RemoveAt(i, 1, true);
			}

			// Savable actors which were placed in the level by hand, destroyed, and then saved, need to be redestroyed.
			// This is because they will always spawn when starting a level.
			for (int32 i = SavableActors.Num() - 1; i >= 0; --i)
			{
				if (SavableActors[i]->bNetStartup)
				{
					SavableActors[i]->Destroy();
					SavableActors.RemoveAt(i, 1, true);
				}
			}

			UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
		}

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("World loaded!"));
#endif // !UE_BUILD_SHIPPING
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("World not loaded! Save game does not exist!"));
#endif // !UE_BUILD_SHIPPING
	}
}

bool UESSBlueprintLibrary::SaveActor(AActor* Actor, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex)
{
	if (IsValid(Actor) && IsValid(SaveGame) && Actor->GetClass()->ImplementsInterface(UESSSavableInterface::StaticClass()))
	{
		// Delete actor's save data if it already exists
		for (int32 i = 0; i < SaveGame->ActorSaveData.Num(); ++i)
		{
			if (SaveGame->ActorSaveData[i].Name == Actor->GetFName())
			{
				SaveGame->ActorSaveData.RemoveAt(i);
			}
		}

		// Save actor's data
		SaveGame->ActorSaveData.Add(SaveActorData(Actor, SaveGame));

		// Save SaveGame to save slot
		bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

		if (bSaved)
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("Actor saved!"));
#endif // !UE_BUILD_SHIPPING

			return true;
		}
		else
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("Actor not saved!"));
#endif // !UE_BUILD_SHIPPING

			return false;
		}
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Actor not saved! Save game object not valid!"));
#endif // !UE_BUILD_SHIPPING

		return false;
	}
}

void UESSBlueprintLibrary::LoadActor(const UObject* WorldContextObject, AActor* Actor, const FString& SlotName, const int32 UserIndex, bool bDestroyLoad)
{
	if (IsValid(Actor) && UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) && Actor->GetClass()->ImplementsInterface(UESSSavableInterface::StaticClass()))
	{
		UESSSaveGame* SaveGame = Cast<UESSSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

		// Get actor's save data
		FActorSaveData ActorData;
		for (int32 i = 0; i < SaveGame->ActorSaveData.Num(); ++i)
		{
			if (SaveGame->ActorSaveData[i].Name == Actor->GetFName())
			{
				ActorData = SaveGame->ActorSaveData[i];
			}
		}

		if (bDestroyLoad)
		{
			Actor->Destroy();

			FActorSpawnParameters SpawnParams;
			AActor* SpawnedActor = World->SpawnActor<AActor>(ActorData.Class, ActorData.Transform, SpawnParams);

			if (IsValid(SpawnedActor))
			{
				LoadActorData(SpawnedActor, ActorData);
			}
		}
		else
		{
			LoadActorData(Actor, ActorData);
		}

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Actor loaded!"));
#endif // !UE_BUILD_SHIPPING
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Actor not loaded! Save game does not exist!"));
#endif // !UE_BUILD_SHIPPING
	}
}

FActorSaveData UESSBlueprintLibrary::SaveActorData(AActor* Actor, UESSSaveGame* SaveGame)
{
	Cast<IESSSavableInterface>(Actor)->Execute_PreSaveGame(Actor);

	FActorSaveData ActorData;

	// Set generic data
	ActorData.Name = Actor->GetFName();
	ActorData.Class = Actor->GetClass();
	ActorData.Transform = Actor->GetActorTransform();

	// Pass byte array to fill with data
	FMemoryWriter MemoryWriter(ActorData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert actor variables to binary data
	Actor->Serialize(Archive);

	// Convert actor components' variables to binary data
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UESSSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	return ActorData;
}

void UESSBlueprintLibrary::LoadActorData(AActor* Actor, FActorSaveData& ActorData)
{
	// Set generic data
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
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(UESSSavableInterface::StaticClass());
	SerializeComponents(Archive, ActorComponents);

	Cast<IESSSavableInterface>(Actor)->Execute_PostLoadGame(Actor);
}

void UESSBlueprintLibrary::SerializeComponents(FObjectAndNameAsStringProxyArchive& Archive, TArray<UActorComponent*> Components)
{
	for (UActorComponent* Comp : Components)
	{
		if (IsValid(Comp))
			Comp->Serialize(Archive);
	}
}

bool UESSBlueprintLibrary::SaveObject(UObject* Object, UESSSaveGame* SaveGame, const FString& SlotName, const int32 UserIndex)
{
	if (IsValid(Object) && IsValid(SaveGame) && Object->GetClass()->ImplementsInterface(UESSSavableInterface::StaticClass()))
	{
		Cast<IESSSavableInterface>(Object)->Execute_PreSaveGame(Object);

		// Delete object's save data if it already exists
		for (int32 i = 0; i < SaveGame->ObjectSaveData.Num(); ++i)
		{
			if (SaveGame->ObjectSaveData[i].Name == Object->GetFName())
			{
				SaveGame->ObjectSaveData.RemoveAt(i);
			}
		}

		SaveObjectData(Object, SaveGame);

		// Save SaveGame to save slot
		bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

		if (bSaved)
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("Object saved!"));
#endif // !UE_BUILD_SHIPPING

			return true;
		}
		else
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Warning, TEXT("Object not saved!"));
#endif // !UE_BUILD_SHIPPING

			return false;
		}
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Object not saved! Save game object not valid!"));
#endif // !UE_BUILD_SHIPPING

		return false;
	}
}

void UESSBlueprintLibrary::LoadObject(UObject* Object, const FString& SlotName, const int32 UserIndex)
{
	if (IsValid(Object) && UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) && Object->GetClass()->ImplementsInterface(UESSSavableInterface::StaticClass()))
	{
		UESSSaveGame* SaveGame = Cast<UESSSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

		// Get object's save data
		FObjectSaveData ObjectData;
		for (int32 i = 0; i < SaveGame->ObjectSaveData.Num(); ++i)
		{
			if (SaveGame->ObjectSaveData[i].Name == Object->GetFName())
			{
				ObjectData = SaveGame->ObjectSaveData[i];
			}
		}

		LoadObjectData(Object, ObjectData);

		Cast<IESSSavableInterface>(Object)->Execute_PostLoadGame(Object);

#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Object loaded!"));
#endif // !UE_BUILD_SHIPPING
	}
	else
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Warning, TEXT("Object not loaded! Save game does not exist!"));
#endif // !UE_BUILD_SHIPPING
	}
}

void UESSBlueprintLibrary::SaveObjectData(UObject* Object, UESSSaveGame* SaveGame)
{
	FObjectSaveData ObjectData;

	// Set generic data
	ObjectData.Name = Object->GetFName();
	ObjectData.Class = Object->GetClass();

	// Pass byte array to fill with data
	FMemoryWriter MemoryWriter(ObjectData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert objects variables to binary data
	Object->Serialize(Archive);

	// Add objects data to save data in the save game object
	SaveGame->ObjectSaveData.Add(ObjectData);
}

void UESSBlueprintLibrary::LoadObjectData(UObject* Object, FObjectSaveData& ObjectData)
{
	// Pass byte array to fill with data
	FMemoryReader MemoryReader(ObjectData.ByteData);

	// Find variables with "SaveGame" property
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	Archive.ArIsSaveGame = true;
	Archive.ArNoDelta = true;

	// Convert objects variables to binary data
	Object->Serialize(Archive);
}

