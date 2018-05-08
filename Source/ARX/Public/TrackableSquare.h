// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Trackable.h>
#include "TrackableSquare.generated.h"
/**
 * 
 */
UCLASS()
class ARX_API ATrackableSquare : public AActor,
                                  public Trackable
{
  GENERATED_BODY()
public:
  ATrackableSquare();
  ~ATrackableSquare();
  int patternId;
  int patt_type;
  
  ARdouble patternWidth;
  ARdouble m_cf;
  ARdouble m_cfMin;
  ARPattHandle *m_arPattHandle;
  /** Actor that gets placed to marker. */ 
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
  AActor * matchingActor; 
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ARX)
  FTransform    arTransform;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= ARX )
  FString patternFileLocation;
  
  
  bool m_loaded;
  bool useContPoseEstimation{false};
  
  void Unload();
  
  bool UpdateWithDetectedMarkers(ARMarkerInfo * markerInfo, int markerNum, AR3DHandle *ar3DHandle);

  bool LoadPatternFromFile(ARPattHandle *arPattHandle);
protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:	
  // Called every frame
  virtual void Tick(float DeltaTime) override;
  virtual void Update() override;
};

