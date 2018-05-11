// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackerNFT.h"
#include <ARX/AR/paramGL.h>
#include <TrackableNFT.h>
#include <TrackerKPM.h>
#include <iostream>
#include <fstream>
#include <Components/SceneCaptureComponent2D.h>
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Misc/FileHelper.h>
#define NFT_MARKER
const int CAMERA_DEFAULT_WIDTH  = 640;
const int CAMERA_DEFAULT_HEIGHT = 480;

// this is defined in android ndk math.h.
#ifndef M_PI_4 
#define M_PI_4 0.78539816339744830962
#endif

using namespace std;

bool ATrackerNFT::LoadMarker()
{
    
    KpmRefDataSet *refDataSet = nullptr;
    
    int surfaceSetCount = 0;
    // Load KPM data.
    KpmRefDataSet  *refDataSet2;
    
	// platforms aren't exactly friendly in this sense, for some reason content directory 
	// is not found properly on android - we need to make a hack.
    FString path;
#if PLATFORM_LINUX
    path = FPaths::ProjectContentDir();
#elif PLATFORM_ANDROID
    path = FString::Printf(TEXT("/sdcard/UE4Game/%s/%s/Content/"), FApp::GetProjectName(), FApp::GetProjectName());
#endif    

	// Actual data file 
    path.Append(FString(markersNFT[0].datasetPathname.c_str()));
    
    UE_LOG(LogTemp, Log, TEXT("Reading %s.fset3"), *path);
    
    if (kpmLoadRefDataSet(TCHAR_TO_UTF8(*path), "fset3", &refDataSet2) < 0 ) 
    {
        UE_LOG(LogTemp, Error, TEXT("Error reading %s.fset3"), *path);
        return false;
    }
    
    markersNFT[0].pageNo = surfaceSetCount;
    UE_LOG(LogTemp, Log, TEXT("  Assigned page no. %d.\n"), surfaceSetCount);
    // This one replaces one page number with another, or replaces all page 
    // numbers in case KpmChangePageNoAllPages.
    if (kpmChangePageNoOfRefDataSet(refDataSet2, KpmChangePageNoAllPages, surfaceSetCount) < 0) {
        UE_LOG(LogTemp, Error, TEXT("kpmChangePageNoOfRefDataSet"));
				return false;
        
    }
    if (kpmMergeRefDataSet(&refDataSet, &refDataSet2) < 0) {
			UE_LOG(LogTemp, Error, TEXT("kpmMergeRefDataSet"));
			return false;
    }
    
        
        // Load AR2 data.
    UE_LOG(LogTemp, Log, TEXT("Reading %s.fset"), *path);
        
    if ((surfaceSet[surfaceSetCount] = ar2ReadSurfaceSet(TCHAR_TO_UTF8(*path), "fset", NULL)) == NULL ) {
		UE_LOG(LogTemp, Error, TEXT("Reading %s.fset"), *path);
		return false;
	}
    
        

    if (kpmSetRefDataSet(initialPatternDetector.kpmHandle, refDataSet) < 0) {
			UE_LOG(LogTemp, Error, TEXT("kpmSetRefDataSet"));
			return false;
    }
    kpmDeleteRefDataSet(&refDataSet);
    
    
		return true;
}
  


void ATrackerNFT::UnloadMarkers()
{
   
    if (initialPatternDetector.Thread) {
        UE_LOG(LogTemp, Log, TEXT("Stopping NFT2 tracking thread."));
        initialPatternDetector.StopThread();
        // should we wait?
    }
   
    ar2FreeSurfaceSet(&surfaceSet[0]); // Also sets surfaceSet[i] to NULL.
   
}

// Sets default values
ATrackerNFT::ATrackerNFT()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  RenderTargetTexture = nullptr;
  VideoTexture = nullptr;
	threadCount = 2;
	
  cameraSize.X = CAMERA_DEFAULT_WIDTH;
	cameraSize.Y = CAMERA_DEFAULT_HEIGHT;
	
}

// Called when the game starts or when spawned
void ATrackerNFT::BeginPlay()
{
	Super::BeginPlay();
}

bool ATrackerNFT::StartTracking()
{
    
    if ( SetupCamera(cameraSize) == false ) return false;
    
    // pixelFormat_ needs to be set explicitly.
    pixelFormat_ = AR_PIXEL_PIXELFORMAT_RGBA; 
		
    handle2d_ = ar2CreateHandle( cameraParameterLT, pixelFormat_, threadCount) ;
    if (handle2d_ == nullptr  )
    {
        UE_LOG(LogTemp, Error, TEXT("ar2CreateHandle failed"));
				goto cleanup;
    }
    
    if (threadCount <= 1) {
        
        ar2SetTrackingThresh(handle2d_, 5.0);
        ar2SetSimThresh(handle2d_, 0.50);
        ar2SetSearchFeatureNum(handle2d_, 16);
        ar2SetSearchSize(handle2d_, 6);
        ar2SetTemplateSize1(handle2d_, 6);
        ar2SetTemplateSize2(handle2d_, 6);
    } else {
      
		// assume capability of executing multiple threads simultanously.
		ar2SetTrackingThresh(handle2d_, 5.0);
		ar2SetSimThresh(handle2d_, 0.50);
		ar2SetSearchFeatureNum(handle2d_, 16);
		ar2SetSearchSize(handle2d_, 12);
		ar2SetTemplateSize1(handle2d_, 6);
		ar2SetTemplateSize2(handle2d_, 6);
    }
    initialPatternDetector.Prepare(cameraParameterLT);
    
    
    VideoSize.X = cameraSize.X;
    VideoSize.Y = cameraSize.Y;
    
    Data.Init( FColor(0,0,0,255), VideoSize.X * VideoSize.Y);
    

    // setup NFT stuff
    // Initialize markers
    markersNFT[0] = TrackableNFT();
    markersNFT[0].Initialize(TCHAR_TO_UTF8(*nftMarkerName),15.0f);
    
    LoadMarker();
    // Start the KPM tracking thread.
    initialPatternDetector.StartThread();

   

    goto end;
	
 cleanup:	
	cleanup();
	return false;
 end:	
	return true;
}

void ATrackerNFT::EndPlay( const EEndPlayReason::Type EndPlayReason) 
{
    UE_LOG(LogTemp, Warning, TEXT("AMyActor Endplay registered"));
    // verify that thread has actually started prior to stoppping it...
    if ( initialPatternDetector.Thread != nullptr )
    {
      initialPatternDetector.StopThread();
      initialPatternDetector.Thread->WaitForCompletion();
      delete initialPatternDetector.Thread;
      initialPatternDetector.Thread = nullptr;      
    }
    cleanup();
    
}

void ATrackerNFT::cleanup()
{
  
  UE_LOG( LogTemp, Log, TEXT("Cleaning up..."));

  
  if ( arPatternHandle != nullptr )  arPattDetach(arhandle);
  arPattDeleteHandle(arPatternHandle);
  arPatternHandle = nullptr;
  if ( handle3d_ != nullptr )
    ar3DDeleteHandle(&handle3d_);
  handle3d_ = nullptr;
  if ( arhandle != nullptr )
    arDeleteHandle(arhandle);
  arhandle = nullptr;
  
  if ( cameraParameterLT != nullptr )
    arParamLTFree(&cameraParameterLT);
  cameraParameterLT = nullptr;

}
// Called every frame
void ATrackerNFT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool ATrackerNFT::RequestMarkerData(float transform[3][4], int & page)
{
  // updates marker data if necessary (marker lost), otherwise marks 
  // previously found available.
  if ( markerFound == false) 
  {
    UE_LOG(LogTemp, Log, TEXT("Seeking initial marker... %d"), LuminanceBuffer.Num());
    
    initialPatternDetector.SeekInitialMarker(LuminanceBuffer.GetData());
    
    if ( initialPatternDetector.found_->Wait(10, false) == true )
    {
        // get current transform and its page number
        for(int r=0;r<3;r++)
        {
          for(int c=0;c<4;c++)
          {
            
             transform[r][c] = initialPatternDetector.trans[r][c];
             page = initialPatternDetector.page;
          }
        }
        markerFound = true;
    }

  }
  return markerFound;
}


bool
ATrackerNFT::DetectMarkerNFT()
{
    
    
    static float patternTransform[3][4];
    static int patternPage;
    
    int j,k;
    float errorLevel;
    // fix this to get it from Android camera
    if ( GetImage(RenderTargetTexture) )
    { 
      
      UE_LOG( LogTemp, Log, TEXT("We have image") );
      
      if ( markerFound == false ) {
        
        markerFound = RequestMarkerData(patternTransform, patternPage);
        if ( markerFound ) UE_LOG( LogTemp, Log, TEXT("Marker found!") );


      }
      
      // need mutex for this to access transform, as it changes often.
      if ( markerFound )
      {
        
        // if we have valid page, then 
        if ( patternPage >= 0 && patternPage < MAX_PAGES )
        {
            // set initial transform (since we might have lost it before)
            ar2SetInitTrans( surfaceSet[patternPage], patternTransform);
            //UE_LOG( LogTemp, Warning, TEXT("Detected page %i"), patternPage );
            
            // extract actual pose, store result back into patternTransform 
            if ( ar2Tracking( handle2d_, 
                              surfaceSet[patternPage], 
                              (ARUint8*)ColorBuffer.GetData(), 
                              patternTransform, &errorLevel) < 0 )
            {
               //  mark marker lost
                UE_LOG( LogTemp, Warning, TEXT("Tracking lost") );
                markerFound = false;
                GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, "ar2Tracking fail");  
            }
            else 
            {
                GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, "ar2Tracking ok");    
            } 
        }
        
          
        // prepare to detect change in validity
        markersNFT[0].validPrev = markersNFT[0].valid;
        
        // in case we find visibile marker...
        if ( markersNFT[0].pageNo >= 0 && 
             markersNFT[0].pageNo == patternPage) 
        {
            // set validity and copy pose into marker structure.
            markersNFT[0].valid = true;
            for (j = 0; j < 3; j++) 
                for (k = 0; k < 4; k++) 
                    markersNFT[0].trans[j][k] = patternTransform[j][k];
        }
        else markersNFT[0].valid = false;
        
        if (markersNFT[0].valid) {
            
            // Filter the pose estimate.
            if (markersNFT[0].ftmi) 
            {
                if (arFilterTransMat( markersNFT[0].ftmi, 
                                        markersNFT[0].trans, 
                                        !markersNFT[0].validPrev) < 0) {
                    UE_LOG( LogTemp, Warning, TEXT("arFilterTransMat error with marker %f."), 0);                     

                }
            }
            
            if (!markersNFT[0].validPrev) {
                // Marker has become visible, tell any dependent objects.
                // --->
                
            }
            
						ARdouble transInverted[3][4];
						ARdouble quaternion[4];
						ARdouble position[3];
						
            
						arUtilMatInv(markersNFT[0].trans, transInverted);
						arUtilMat2QuatPos(transInverted, quaternion, position);
						float aspectRatioFix = (float)cameraSize.X / (float)cameraSize.Y;
						FVector pos = FVector(markersNFT[0].trans[2][3], markersNFT[0].trans[0][3], -markersNFT[0].trans[1][3]);
						pos /= aspectRatioFix;
						//pos = FVector(markersNFT[0].trans[0][3]/1.777, markersNFT[0].trans[1][3]/1.777, -markersNFT[0].trans[2][3]/1.777);
						//Construct proper rotation for UE coordinates
						FRotator raw = FQuat(quaternion[0], quaternion[1], quaternion[2], quaternion[3]).Rotator();
						FRotator rot;
						rot.Yaw = -raw.Pitch;
						rot.Pitch = 180 - raw.Roll;
						rot.Roll = raw.Yaw;
						
						//rot = raw;
						//TODO
						// Need offset for target (from left down corner to center, for example)
		
            UE_LOG(LogTemp, Log, TEXT("Pattern found, errorlevel is  %f"), errorLevel);    
            
            
            arTransform = FTransform( rot, pos, FVector(1,1,1));    
       
            
            if (markersNFT[0].validPrev) {
                // Marker has ceased to be visible, tell any dependent objects.
                // --->
            }
        }
          
          
         
      }
      else {
        UE_LOG( LogTemp, Warning, TEXT("Marker not found") );
          
      }

    }
    
    return markerFound; 


}

///////////////////////////////////////////////////////////////////////////////
bool ATrackerNFT::SaveRenderTargetToFile()
{
     
     FIntPoint destSize(RenderTargetTexture->GetSurfaceWidth(), RenderTargetTexture->GetSurfaceHeight());
     TArray<uint8> CompressedBitmap;
     FImageUtils::CompressImageArray(destSize.X, destSize.Y, ColorBuffer, CompressedBitmap);
     FString fileDestination = "colorbuffer.png";
     bool imageSavedOk = FFileHelper::SaveArrayToFile(CompressedBitmap, *fileDestination);
 
     return imageSavedOk;
 }
