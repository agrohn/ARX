// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tracker2D.generated.h"
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS


UCLASS()
class ARX_API ATracker2D : public AActor
{
  GENERATED_BODY()
  
public:	
  // Sets default values for this actor's properties
  ATracker2D();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:	
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  
  
};

