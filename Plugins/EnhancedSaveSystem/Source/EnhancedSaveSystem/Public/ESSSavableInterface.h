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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PreSaveGame();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PostSaveGame();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enhanced Save System")
	void PostLoadGame();
};
