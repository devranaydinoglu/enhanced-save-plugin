// Copyright 2023 devran. All Rights Reserved.


#include "ESSSavableComponent.h"

// Sets default values for this component's properties
UESSSavableComponent::UESSSavableComponent()
	: GUID("")
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UESSSavableComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UESSSavableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

