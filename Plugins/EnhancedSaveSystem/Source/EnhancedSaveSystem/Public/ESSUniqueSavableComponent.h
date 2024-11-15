// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ESSUniqueSavableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENHANCEDSAVESYSTEM_API UESSUniqueSavableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// A globally unique identifier which can be used to identify and load unique actors in the SaveGame.
	// This should be specified for "special" actors, e.g. the player character, whose data you may want to load individually instead of loading the full SaveGame.
	UPROPERTY(EditAnywhere, Category = "Attributes")
	FString GUID;

public:	
	// Sets default values for this component's properties
	UESSUniqueSavableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
