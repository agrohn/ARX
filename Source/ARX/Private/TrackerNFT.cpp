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

bool ATrackerNFT::LoadMarkers()
{
 
  // platforms aren't exactly friendly in this sense, for some reason content directory 
  // is not found properly on android - we need to make a hack.
  FString pathPrefix;
#if PLATFORM_LINUX
  pathPrefix = FPaths::ProjectContentDir();
#elif PLATFORM_ANDROID
  pathPrefix = FString::Printf(TEXT("/sdcard/UE4Game/%s/%s/Content/"), FApp::GetProjectName(), FApp::GetProjectName());
#endif    
  
  int markerCount=0;
  
  for (auto it(trackables_.CreateIterator()); it; it++)
  {
    ATrackableNFT * nft = Cast<ATrackableNFT>(*it);
    if ( nft ) 
    {
      // Actual data file 
      FString path = pathPrefix;
      path.Append(nft->datasetPath);

      // Load KPM tracker 
      initialOrientationDetector.Load( path );
//      UE_LOG(LogTemp, Log, TEXT("  Assigned page no. %d.\n"), surfaceSetCount);
      
      // Load AR2 data.
      nft->Load(path);
      nft->markerIndex = markerCount++;
    } 
  }
  
  return true;
}

void ATrackerNFT::UnloadMarkers()
{
   
    if (initialOrientationDetector.Thread) {
        UE_LOG(LogTemp, Log, TEXT("Stopping NFT2 tracking thread."));
        initialOrientationDetector.StopThread();
        // should we wait?
    }
    for (auto it(trackables_.CreateIterator()); it; it++)
    {
      ATrackableNFT * nft = Cast<ATrackableNFT>(*it);
      if ( nft ) 
      {
        nft->Unload();
      }
    }
    
   
}

// Sets default values
ATrackerNFT::ATrackerNFT()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  RenderTargetTexture = nullptr;
  
	threadCount = 2;
	
  cameraSize.X = CAMERA_DEFAULT_WIDTH;
	cameraSize.Y = CAMERA_DEFAULT_HEIGHT;
	
}

// Called when the game starts or when spawned
void ATrackerNFT::BeginPlay()
{
	Super::BeginPlay();
  tickLimiter.SetLimit( tickLimit );
}

void ATrackerNFT::Initialize()
{
  UE_LOG(LogTemp, Log, TEXT("We have #trackables: %d"),trackables_.Num());
  LoadMarkers();
  
  // prepare our camera 
  SetupCamera(cameraSize);
}

bool ATrackerNFT::StartTracking()
{
    // pixelFormat_ needs to be set explicitly.
    pixelFormat_ = AR_PIXEL_FORMAT_RGBA; 
		
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
    initialOrientationDetector.Prepare(cameraParameterLT);
    
    
   

    // Initialize markers
    for (auto it(trackables_.CreateIterator()); it; it++)
    {
      ATrackableNFT * nft = Cast<ATrackableNFT>(*it);
      if ( nft ) 
      {
        nft->Initialize(15.0f);
      }
    }
    
    // Start the KPM tracking thread.
    initialOrientationDetector.StartThread();

   

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
    if ( initialOrientationDetector.Thread != nullptr )
    {
      initialOrientationDetector.StopThread();
      initialOrientationDetector.Thread->WaitForCompletion();
      delete initialOrientationDetector.Thread;
      initialOrientationDetector.Thread = nullptr;      
    }
    cleanup();
    
}

void ATrackerNFT::cleanup()
{
  
  UE_LOG( LogTemp, Log, TEXT("Cleaning up..."));

  if ( handle2d_ != nullptr )
    ar2DeleteHandle(&handle2d_);
  handle2d_ = nullptr;
  
  if ( cameraParameterLT != nullptr )
    arParamLTFree(&cameraParameterLT);
  cameraParameterLT = nullptr;
  
  TearDownCamera();

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
   
  bool markerFound = false;
  //UE_LOG(LogTemp, Log, TEXT("Seeking initial marker... %d"), LuminanceBuffer.Num());
    
  initialOrientationDetector.SeekInitialMarker(LuminanceBuffer.GetData());
    
  if ( initialOrientationDetector.found_->Wait(10, false) == true )
  {
      // get current transform and its page number
      for(int r=0;r<3;r++)
      {
        for(int c=0;c<4;c++)
        {
          
           transform[r][c] = initialOrientationDetector.trans[r][c];
           page = initialOrientationDetector.page;
        }
      }
      markerFound = true;
  }

  
  return markerFound;
}

ATrackableNFT * 
ATrackerNFT::FindTrackableByMarkerIndex( int markerIndex)
{
  for (auto it(trackables_.CreateIterator()); it; it++)
  {
    ATrackableNFT * nft = Cast<ATrackableNFT>(*it);
    if ( nft ) 
    { 
      if (nft->markerIndex == markerIndex)  return nft;
    }
    else UE_LOG(LogTemp, Warning, TEXT("Not ATrackableNFT in trackables!"));
  }
  return nullptr;
}

bool
ATrackerNFT::Update(float deltaTime)
{
    
  if ( tickLimiter.Update(deltaTime) == false) return true;
  
    float markerTransform[3][4];
    int whichMarker;
    
    
    // fix this to get it from Android camera
    if ( GetImage(RenderTargetTexture) )
    { 
      if ( shouldSeekInitialOrientation ) 
      {
        bool foundMarker = RequestMarkerData(markerTransform, whichMarker);
        if ( foundMarker )
        {
          // seek page from all trackables. 
          ATrackableNFT * nft = FindTrackableByMarkerIndex( whichMarker );
          // if we have trackable, then 
          if ( nft != nullptr )
          {
            if ( nft->IsTrackedContinously() == false) 
            {
              nft->SetInitialTransform( markerTransform );    
            }
            //else UE_LOG(LogTemp, Warning, TEXT("Marker %d already tracked continously."), whichMarker);
          }
          else UE_LOG(LogTemp, Error, TEXT("No trackable was found for marker %d."), whichMarker);
        }
      }
      // jonna jonna
      shouldSeekInitialOrientation = false;
      for (auto it(trackables_.CreateIterator()); it; it++)
      {
        ATrackableNFT * nft = Cast<ATrackableNFT>(*it);
        if ( nft  == nullptr ) continue;
        shouldSeekInitialOrientation |= !nft->IsTrackedContinously();
        nft->UpdateWithDetectedMarkers( handle2d_, ColorBuffer, markerTransform);
      }

    }
    
    return true; 


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
