// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EssSavableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEssSavableInterface : public UInterface
{
	GENERATED_BODY()
};

/*
* Interface to be implemented by objects and actors which need to be saved.
* Any object which implements this interface will be saved by the save system.
*/
class ENHANCEDSAVESYSTEM_API IEssSavableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called before an actor or object is saved.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PreSaveGame();

	/**
	 * Called after an actor or object has been saved.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PostSaveGame();

	/**
	 * Called after an actor or object has been loaded.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PostLoadGame();
};
