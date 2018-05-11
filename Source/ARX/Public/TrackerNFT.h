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
#include "TrackerNFT.generated.h"


const int MAX_PAGES = 1;

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
	

	AR2HandleT *	handle2d_{nullptr};
	AR2SurfaceSetT  *surfaceSet[MAX_PAGES]; 
	//TrackableNFT    markersNFT[MAX_PAGES];
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
  TArray<AActor *> trackables_;
  
	bool markerFound{false};
	
	UTrackerKPM initialPatternDetector;
	 /** Camera image size to be used for proper perspective mapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
	FIntPoint cameraSize;

protected:
	
	void cleanup();
	
public:	

    UFUNCTION(BlueprintCallable, Category=AR)
	bool StartTracking();
	

  
	UFUNCTION(BlueprintCallable, Category=AR)
	bool DetectMarkerNFT();
  
	UPROPERTY(BlueprintReadOnly, Category=AR)
	FTransform arTransform;
	
	/** NFT marker name to use.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AR)
	FString nftMarkerName;
	

	
	UPROPERTY(BlueprintReadOnly, Category=AR)
	UTexture2D* VideoTexture;
  
	/** For optimization, how many threads are utilized in marker detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=AR)
	int threadCount;
	
	/** Render target texture for where camera view is displayed. */ 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = AR )
	UTextureRenderTarget2D* RenderTargetTexture;
  
	bool LoadMarker();
	void UnloadMarkers();
	bool RequestMarkerData(float transform[3][4], int & page);

	
	
	
	FUpdateTextureRegion2D* VideoUpdateTextureRegion;
	FVector2D VideoSize;
	
	
	
	
	bool imageSaved{false};
	UFUNCTION(BlueprintCallable, Category=AR)
	bool SaveRenderTargetToFile();
};

