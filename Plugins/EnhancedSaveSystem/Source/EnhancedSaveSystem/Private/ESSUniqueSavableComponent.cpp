// Copyright 2023 devran. All Rights Reserved.


#include "ESSUniqueSavableComponent.h"

// Sets default values for this component's properties
UESSUniqueSavableComponent::UESSUniqueSavableComponent()
	: GUID("")
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UESSUniqueSavableComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UESSUniqueSavableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

