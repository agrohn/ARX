#include "TrackableNFT.h"

using namespace std;

ATrackableNFT::ATrackableNFT()
{
  markerIndex = -1;
  PrimaryActorTick.bCanEverTick = true;
}


ATrackableNFT::~ATrackableNFT()
{
}

void
ATrackableNFT::Initialize( )
{
  
    float cutoffFrequency = filterCutoffFrequency < 0.0 ? 
                                AR_FILTER_TRANS_MAT_CUTOFF_FREQ_DEFAULT : filterCutoffFrequency;
    float sampleRate      = filterSampleRate < 0.0 ? 
                                AR_FILTER_TRANS_MAT_SAMPLE_RATE_DEFAULT : filterSampleRate;
    UE_LOG(LogTemp, Log, TEXT("Setting filter transmat values: %f / %f"), sampleRate, cutoffFrequency)
    ftmi = arFilterTransMatInit( sampleRate, cutoffFrequency);
}

void
ATrackableNFT::Update()
{
  

}

void
ATrackableNFT::Load( const FString & path )
{

  UE_LOG(LogTemp, Log, TEXT("Reading %s.fset"), *path);
  
  surfaceSet = ar2ReadSurfaceSet(TCHAR_TO_UTF8(*path), "fset", nullptr);
  
  if ( surfaceSet == nullptr )
  {
    UE_LOG(LogTemp, Error, TEXT("Could not read %s.fset"), *path);
  }
  
}

void
ATrackableNFT::Unload()
{
  ar2FreeSurfaceSet(&surfaceSet); // Also sets it to NULL.  
  
}

void
ATrackableNFT::SetInitialTransform(float markerTransform[3][4])
{
  ar2SetInitTrans(surfaceSet, markerTransform);
  for(int i=0;i<3;i++)
    for(int j=0;j<4;j++)
      initialTransform[i][j] = markerTransform[i][j];
}


bool
ATrackableNFT::UpdateWithDetectedMarkers(AR2HandleT * handle, TArray<uint8> & LuminanceBuffer)
{
  if ( surfaceSet == nullptr ) 
  {
    UE_LOG(LogTemp, Warning, TEXT("Attempt to UpdateWithDetectedMarkes without loading a marker!"));
    return false;
  }
  
  float errorLevel;
  visiblePrev = visible;
  // extract actual pose, store result back into patternTransform
  if ( ar2Tracking( handle, surfaceSet, (ARUint8*)LuminanceBuffer.GetData(), 
                            initialTransform, &errorLevel) < 0 )
  {
    
    //UE_LOG( LogTemp, Warning, TEXT("Tracking lost") );

    visible = false;
  }
  else 
  {
    //UE_LOG(LogTemp, Log, TEXT("Marker %d matches"), markerIndex);
    visible = true;
    // set validity and copy pose into marker structure.
    for(int i=0;i<3;i++)
      for(int j=0;j<4;j++)
        trans[i][j] = initialTransform[i][j];

    
    
    
      
    if (visible && ftmi != nullptr) 
    {
       if (arFilterTransMat( ftmi, trans, !visiblePrev) < 0) {
           UE_LOG( LogTemp, Warning, TEXT("arFilterTransMat error with marker %s."), *datasetPath);
       }   
    }
    
  }
  return visible;
}
// Called every frame
void ATrackableNFT::Tick(float DeltaTime)
{
  
	Super::Tick(DeltaTime);
  
  if ( surfaceSet == nullptr ) return;
  
  
  bool tickLimitPassed = tickLimiter.Update(DeltaTime);
 
  // set marker transform so it is available when event is handled.
  if ( visible )
  {
      if ( matchingActor != nullptr && tickLimitPassed )
      {
        
          //UE_LOG(LogTemp, Log, TEXT("Setting actor transform to match marker."));
          //arUtilPrintTransMat(trans);
          MatrixToTransform(trans,arTransform);
          // If camera is set, set rotation in relation to it.
          if ( HasParentCameraActor() ) {
            arTransform = ApplyParentCameraToTransform(arTransform);
          }
          matchingActor->SetActorTransform(arTransform,false);
      }
  }
  // launch event handlers
  if      ( HasBecomeVisible()  )  OnMarkerDetected();
  else if ( HasBecomeInvisible())  OnMarkerLost();
}

void ATrackableNFT::OnMarkerDetected_Implementation()
{
  
}

void ATrackableNFT::OnMarkerLost_Implementation()
{
  
}
bool ATrackableNFT::IsTrackedContinously() const
{
  return (surfaceSet->contNum > 0);
}

void ATrackableNFT::BeginPlay()
{
  Super::BeginPlay();
  tickLimiter.SetLimit(tickLimit);
}

void ATrackableNFT::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

FTransform ATrackableNFT::GetOffset() const 
{
    return offset;
}
