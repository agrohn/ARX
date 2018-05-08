// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracker2D.h"


// Sets default values
ATracker2D::ATracker2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATracker2D::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATracker2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

