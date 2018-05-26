// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Engine/Texture2D.h>
#include <Engine/SceneCapture2D.h>
#include <ImageUtils.h>
#include <Engine/TextureRenderTarget2D.h>
#include <string>
#include <TrackableNFT.h>
#include <TrackerKPM.h>
#include <TrackerBase.h>
#include <TickLimiter.h>
#include "TrackerNFT.generated.h"


const int MAX_PAGES = 64;

UCLASS()
class ARX_API ATrackerNFT : public AActor,
                            public TrackerBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrackerNFT();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override; 
		
public:
	/* ARToolkit-related member variables */	
	
	AR_PIXEL_FORMAT format;
	

	AR2HandleT *	    handle2d_{nullptr};
	AR2SurfaceSetT  *surfaceSet[MAX_PAGES]; 
	
  UTrackerKPM initialOrientationDetector;
  bool shouldSeekInitialOrientation{true};
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
  TArray<AActor *> trackables_;
  
 /** Camera image size to be used for proper perspective mapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
	FIntPoint cameraSize;

protected:
	
	void cleanup();
	 TickLimiter tickLimiter;
public:	

  ATrackableNFT * FindTrackableByMarkerIndex( int markerIndex);

  UFUNCTION(BlueprintCallable, Category=ARX)
	bool StartTracking();
  
	UFUNCTION(BlueprintCallable, Category = ARX)
  void Initialize();
  
	/** For optimization, how many threads are utilized in marker detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=ARX)
	int threadCount;
	
	/** Render target texture for where camera view is displayed. */ 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ARX )
	UTextureRenderTarget2D* RenderTargetTexture;
  
	bool LoadMarkers();
	void UnloadMarkers();
	bool RequestMarkerData(float transform[3][4], int & page);

	UFUNCTION(BlueprintCallable,Category = ARX)
  bool Update(float deltaTime);
	

	bool imageSaved{false};
	UFUNCTION(BlueprintCallable, Category=ARX)
	bool SaveRenderTargetToFile();
  
   /// Limit for running update.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX)
  float tickLimit;
  
};

