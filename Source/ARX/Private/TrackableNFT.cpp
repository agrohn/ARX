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
ATrackableNFT::Initialize( float cutoffFrequency)
{
    filterCutoffFrequency = cutoffFrequency;
    filterSampleRate      = AR_FILTER_TRANS_MAT_SAMPLE_RATE_DEFAULT;
    
    ftmi = arFilterTransMatInit( filterSampleRate, filterCutoffFrequency);
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
}


void
ATrackableNFT::UpdateWithDetectedMarkers(AR2HandleT * handle, TArray<FColor> & ColorBuffer, float patternTransform[3][4])
{
  float errorLevel;
  visiblePrev = visible;
  // extract actual pose, store result back into patternTransform
  if ( ar2Tracking( handle, surfaceSet, (ARUint8*)ColorBuffer.GetData(), 
                            patternTransform, &errorLevel) < 0 )
  {
    
    //UE_LOG( LogTemp, Warning, TEXT("Tracking lost") );

    visible = false;
    
  }
  else 
  {
    //UE_LOG(LogTemp, Log, TEXT("Marker %d matches"), markerIndex);
    visible = true;
    // set validity and copy pose into marker structure.

    for (int j = 0; j < 3; j++) 
        for (int k = 0; k < 4; k++) 
            trans[j][k] = patternTransform[j][k];
    
    
    
      
    if (visible && ftmi != nullptr) 
    {
       if (arFilterTransMat( ftmi, trans, !visiblePrev) < 0) {
           UE_LOG( LogTemp, Warning, TEXT("arFilterTransMat error with marker %s."), *datasetPath);
       }   
    }
  }
}
// Called every frame
void ATrackableNFT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
