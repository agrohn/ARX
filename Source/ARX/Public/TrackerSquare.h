// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <ARX/AR/ar.h>
#include <TrackerBase.h>
#include <TrackableSquare.h>
#include <TickLimiter.h>
#include "TrackerSquare.generated.h"

UCLASS()
class ARX_API ATrackerSquare : public AActor,
  public TrackerBase
{
  GENERATED_BODY()

public:
  
  // Sets default values for this actor's properties
  ATrackerSquare();

  ARPattHandle *patternHandle_;
 
  
  

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
  TArray<AActor *> trackables_;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ARX )
  UTextureRenderTarget2D* RenderTargetTexture;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
  FIntPoint cameraSize;

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  TickLimiter tickLimiter;
public:
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = ARX)
  void Initialize();
  
  UFUNCTION(BlueprintCallable, Category = ARX)
  bool StartTracking();
  
  UFUNCTION(BlueprintCallable, Category = ARX)
  virtual bool Update(float deltaTime);
  
  UFUNCTION(BlueprintCallable, Category = ARX)
  void StopTracking();
  virtual void Deinitialize();
  
  UFUNCTION(BlueprintCallable, Category = ARX)
  bool SaveRenderTargetToFile();
  
  /// Limit for running update.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX)
  float tickLimit;
  
};
