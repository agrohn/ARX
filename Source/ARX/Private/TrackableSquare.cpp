// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackableSquare.h"
#include <Misc/FileHelper.h>
#include <ARX/AR/ar.h>
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Misc/Paths.h>
#include <Misc/App.h>
#define    AR_PATTERN_TYPE_TEMPLATE    0
#define    AR_PATTERN_TYPE_MATRIX      1


ATrackableSquare::ATrackableSquare()
{
  m_loaded = false;
  PrimaryActorTick.bCanEverTick = true;
  tickLimit = 0.0f;
}

ATrackableSquare::~ATrackableSquare()
{
  
}

bool
ATrackableSquare::UpdateWithDetectedMarkers(ARMarkerInfo * markerInfo, int markerNum, AR3DHandle *ar3DHandle)
{
    
	if (patternId < 0) 
  {
    UE_LOG(LogTemp, Warning, TEXT("Whoops, there is no pattern id set."));
    return false;	// Can't update if no pattern loaded
  }

  visiblePrev = visible;

	if (markerInfo) 
  {
      //UE_LOG(LogTemp, Log, TEXT("Seeking pattern..."));
        int k = -1;
        if (patt_type == AR_PATTERN_TYPE_TEMPLATE) { 
            // Iterate over all detected markers.
            for (int j = 0; j < markerNum; j++ ) {
                if (patternId == markerInfo[j].idPatt) {
                    // The pattern of detected trapezoid matches marker[k].
                    if (k == -1) {
                        if (markerInfo[j].cfPatt > m_cfMin) k = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfo[j].cfPatt > markerInfo[k].cfPatt) k = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (k != -1) {
                markerInfo[k].id = markerInfo[k].idPatt;
                markerInfo[k].cf = markerInfo[k].cfPatt;
                markerInfo[k].dir = markerInfo[k].dirPatt;
            }
        } else {
            for (int j = 0; j < markerNum; j++) {
                if (patternId == markerInfo[j].idMatrix) {
                    if (k == -1) {
                        if (markerInfo[j].cfMatrix >= m_cfMin) k = j; // Count as a match if match confidence exceeds cfMin.
                    } else if (markerInfo[j].cfMatrix > markerInfo[k].cfMatrix) k = j; // Or if it exceeds match confidence of a different already matched trapezoid (i.e. assume only one instance of each marker).
                }
            }
            if (k != -1) {
                markerInfo[k].id = markerInfo[k].idMatrix;
                markerInfo[k].cf = markerInfo[k].cfMatrix;
                markerInfo[k].dir = markerInfo[k].dirMatrix;
            }
        }
        
		// Consider marker visible if a match was found.
        if (k != -1) 
        {
            visible = true;
            m_cf = markerInfo[k].cf;
            // If the model is visible, update its transformation matrix
            if (visiblePrev && useContPoseEstimation) {
              // If the marker was visible last time, use "cont" version of arGetTransMatSquare
              arGetTransMatSquareCont(ar3DHandle, &(markerInfo[k]), trans, patternWidth, trans);
            } else {
              // If the marker wasn't visible last time, use normal version of arGetTransMatSquare
              arGetTransMatSquare(ar3DHandle, &(markerInfo[k]), patternWidth, trans);
            }
        } 
        else
        {
            visible = false;
            m_cf = 0.0f;
        }

  } 
  else 
  {
      //UE_LOG(LogTemp, Log, TEXT("NO markerInfo"));
      visible = false;
      m_cf = 0.0f;
  }

// as well as setting 'trans'.
    if (visible) {
        
        // Filter the pose estimate.
        if (m_ftmi) {
            if (arFilterTransMat(m_ftmi, trans, !visiblePrev) < 0) {
                UE_LOG(LogTemp, Error, TEXT("arFilterTransMat error with trackable"));
            }
        }
        
        if (!visiblePrev) {
            UE_LOG(LogTemp, Log, TEXT("trackable now visible"));
        }
        
       
    } else {
        
        if (visiblePrev) {
            UE_LOG(LogTemp, Log, TEXT("Trackable no longer visible"));
        }
        
    }
    
    return true;
	
}

void
ATrackableSquare::Unload()
{
    if (m_loaded) {
        
        if (patt_type == AR_PATTERN_TYPE_TEMPLATE && patternId != -1) {
            if (m_arPattHandle) {
                arPattFree(m_arPattHandle, patternId);
                m_arPattHandle = NULL;
            }
        }
        patternId = patt_type = -1;
        m_cf = 0.0f;
        patternWidth = 0.0f;
        m_loaded = false;
    }
}

bool 
ATrackableSquare::LoadPatternFromFile(ARPattHandle *arPattHandle)
{
    m_arPattHandle = arPattHandle;
	// Ensure the pattern string is valid
	if (patternFileLocation.IsEmpty() || !m_arPattHandle) 
  {
    UE_LOG(LogTemp, Warning, TEXT("Cannot load pattern, missing pattern file or tracker handle"));
    return false;
  }
  
  if (m_loaded) Unload();

  FString tmpPath;
#if PLATFORM_LINUX
    tmpPath = FPaths::ProjectContentDir();
#elif PLATFORM_ANDROID
    tmpPath = FString::Printf(TEXT("/sdcard/UE4Game/%s/%s/Content/"), FApp::GetProjectName(), FApp::GetProjectName());
#endif    

  tmpPath.Append(patternFileLocation);
  patternWidth = 80.0;
  std::string path(TCHAR_TO_UTF8(*tmpPath));
  
  UE_LOG(LogTemp, Log, TEXT("Loading single AR marker from file '%s', width %f"), *tmpPath, patternWidth);
	 
  m_arPattHandle = arPattHandle;
	patternId = arPattLoad(m_arPattHandle, path.c_str());
	if (patternId < 0) {
		UE_LOG(LogTemp, Error, TEXT("unable to load single AR marker from file '%s'"), *tmpPath);
    arPattHandle = NULL;
		return false;
	}
	
  patt_type = AR_PATTERN_TYPE_TEMPLATE;
  
    
	visible = visiblePrev = false;
    
    // An ARPattern to hold an image of the pattern for display to the user. 
    // (NOT needed, can be accomplished via other means)
	//allocatePatterns(1);
	//patterns[0]->loadTemplate(patt_id, m_arPattHandle, (float)m_width);

    m_loaded = true;
	return true;
}
// Called when the game starts or when spawned
void ATrackableSquare::BeginPlay()
{
	Super::BeginPlay();
	tickLimiter.SetLimit(tickLimit);
}

// Called every frame
void ATrackableSquare::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
  
  bool tickLimitPassed = tickLimiter.Update(DeltaTime);
  // set marker transform so it is available when event is handled.
  if ( visible )
  {
      if ( matchingActor != nullptr && tickLimitPassed )
      {
        
          //UE_LOG(LogTemp, Log, TEXT("Setting actor transform to match marker."));
          MatrixToTransform(trans,arTransform);
          arTransform.SetScale3D(matchingActor->GetActorScale3D());
          matchingActor->SetActorTransform(arTransform,false);
      }
  }
  // launch event handlers
  if      ( HasBecomeVisible()  )  OnMarkerDetected();
  else if ( HasBecomeInvisible())  OnMarkerLost();
}

void ATrackableSquare::Update()
{
  
}

void ATrackableSquare::OnMarkerDetected_Implementation()
{
  
}
void ATrackableSquare::OnMarkerLost_Implementation()
{
  
}
