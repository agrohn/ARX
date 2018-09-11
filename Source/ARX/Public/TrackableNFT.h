#pragma once
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <ARX/AR/ar.h>
#include <ARX/AR/config.h>
#include <ARX/AR/param.h>			// arParamDisp()
#include <ARX/AR/ar.h>
#include <ARX/AR/arFilterTransMat.h>
#include <ARX/AR2/tracking.h>
#include <Trackable.h>
#include <string>
#include <TickLimiter.h>
#include "TrackableNFT.generated.h"
//////////////////////////////////////////////////////////////////////////////////////
UCLASS()
class ARX_API ATrackableNFT : public AActor, public Trackable
{
  GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason) override;
  TickLimiter tickLimiter;
  float initialTransform[3][4];
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override; 
  ATrackableNFT();
  virtual ~ATrackableNFT();
  
  void Initialize();
  void Update() ;
  bool UpdateWithDetectedMarkers(AR2HandleT * handle, TArray<uint8> & LuminanceBuffer);   
  
  void Load( const FString & path );
  void Unload();
  bool IsTrackedContinously() const;
  // After KPM detection has found marker, this is used to allow ar2Tracking to start kicking.
  void SetInitialTransform( float markerTransform[3][4]);
public:
    
    // ARMarker private
    ARFilterTransMatInfo *ftmi{nullptr};
    /// Jitter filter cutoff frequency. Set negative for AR_FILTER_TRANS_MAT_CUTOFF_FREQ_DEFAULT.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX)
    float   filterCutoffFrequency{AR_FILTER_TRANS_MAT_CUTOFF_FREQ_DEFAULT};
    /// Jitter filter sample rate. Set negative for AR_FILTER_TRANS_MAT_SAMPLE_RATE_DEFAULT.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX)
    float   filterSampleRate{AR_FILTER_TRANS_MAT_SAMPLE_RATE_DEFAULT};
    
    // Actual NFT tracking surface set.
    AR2SurfaceSetT  *surfaceSet; 
    /** Actor that gets placed to marker. */ 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
    AActor * matchingActor; 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ARX)
    FTransform    arTransform;
    /** Offset for matching actor over the marker. */ 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ARX)
    FTransform    offset;
    
    /// This connects data to KPM detection.
    int        			 markerIndex{0}; 
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX )
    FString      datasetPath;
    
    /// Limits matchin actor update interval in seconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ARX)
    float       tickLimit;
    
    UFUNCTION(BlueprintNativeEvent)
    void OnMarkerDetected();
  
    UFUNCTION(BlueprintNativeEvent)
    void OnMarkerLost();
    virtual FTransform GetOffset() const override;
    
};
//////////////////////////////////////////////////////////////////////////////////////


