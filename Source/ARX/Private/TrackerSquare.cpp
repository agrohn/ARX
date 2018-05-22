// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackerSquare.h"
#include <TrackableSquare.h>
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Misc/FileHelper.h>
// Sets default values
ATrackerSquare::ATrackerSquare()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  parentCameraActor = nullptr;
}

// Called when the game starts or when spawned
void ATrackerSquare::BeginPlay()
{
  Super::BeginPlay();
  tickLimiter.SetLimit( tickLimit );
  UE_LOG(LogTemp,Log, TEXT("Setting ATrackerSquare tick limit to %f"),tickLimit);
  
  if ( parentCameraActor )
  {
    UE_LOG(LogTemp,Log, TEXT("Setting Parent Camera Actor to Trackables"));
    for (auto it(trackables_.CreateIterator()); it; it++)
    {
      ATrackableSquare * square = Cast<ATrackableSquare>(*it);
      if ( square ) {
        square->SetParentCameraActor(parentCameraActor);
      } 
    }
  }
}

// Called every frame
void ATrackerSquare::Tick(float DeltaTime)
{

  Super::Tick(DeltaTime);

}

void ATrackerSquare::Initialize()
{
  /* Default values set by artoolkitx.  */
  patternHandle_ = arPattCreateHandle2( AR_PATT_SIZE1, AR_PATT_NUM_MAX );
  
  // Load each trackable
  // 
  UE_LOG(LogTemp, Log, TEXT("We have trackables: %d"),trackables_.Num());
  for (auto it(trackables_.CreateIterator()); it; it++) {
    ATrackableSquare * square = Cast<ATrackableSquare>(*it);
    if ( square ) {
      square->LoadPatternFromFile(patternHandle_);
    } else {
      UE_LOG(LogTemp, Warning, TEXT("Actor not instance of ATrackableSquare - cannot load pattern."));
    }
  }
  
  // prepare our camera 
  SetupCamera(cameraSize);
}

//arController->getSquareTracker()->setPatternDetectionMode(AR_TEMPLATE_MATCHING_MONO);
//arController->getSquareTracker()->setThresholdMode(AR_LABELING_THRESH_MODE_AUTO_BRACKETING);

bool ATrackerSquare::StartTracking()
{
  pixelFormat_ = AR_PIXEL_FORMAT_RGBA; 
  if (!cameraParameterLT ) 
  {
    UE_LOG(LogTemp, Error, TEXT("No cameraParameterLT"));
    return false;
  }
  if ( pixelFormat_ == AR_PIXEL_FORMAT_INVALID ) 
  {
    UE_LOG(LogTemp, Error, TEXT("Invalid pixel format"));
    return false;
  }

  // Create AR handle
  if ((handle_ = arCreateHandle(cameraParameterLT)) == nullptr) {
    UE_LOG(LogTemp, Error, TEXT("Failed to create ARHandle"));
    return false;
  }

  // Set the pixel format
  arSetPixelFormat(handle_, pixelFormat_);

  arPattAttach(handle_, patternHandle_);

  // Set initial configuration. One call for each configuration option.
  arSetLabelingThresh(        handle_, AR_DEFAULT_LABELING_THRESH);
  //arSetLabelingThreshMode(    handle_, AR_LABELING_THRESH_MODE_DEFAULT);
  arSetLabelingThreshMode(    handle_, AR_LABELING_THRESH_MODE_AUTO_BRACKETING);
  arSetImageProcMode(         handle_, AR_DEFAULT_IMAGE_PROC_MODE);
  //arSetDebugMode(handle_, m_debugMode);
  arSetLabelingMode(          handle_, AR_DEFAULT_LABELING_MODE);
  arSetPattRatio(             handle_, AR_PATT_RATIO);
  //arSetPatternDetectionMode(  handle_, AR_DEFAULT_PATTERN_DETECTION_MODE);
  arSetPatternDetectionMode(  handle_, AR_TEMPLATE_MATCHING_MONO);
  arSetMatrixCodeType(        handle_, AR_MATRIX_CODE_TYPE_DEFAULT);

  // Create 3D handle
  if ((handle3d_ = ar3DCreateHandle(&cameraParameterLT->param)) == NULL) {
    UE_LOG(LogTemp, Error, TEXT("ar3DCreateHandle failed"));

    arDeleteHandle(handle_);
    handle_ = nullptr;
    return false;
  }



  UE_LOG(LogTemp, Log, TEXT("StartTracking done."));
  return true;


}
bool ATrackerSquare::Update(float deltaTime)
{
  
  if ( tickLimiter.Update(deltaTime) == false) return true;
  
  ARMarkerInfo *markerInfo0 = nullptr;

  const int VIDEOBUFFER_IS_VALID = 1;
  int markerNum0 = 0;


  if (!handle_) 
  {
      UE_LOG(LogTemp, Error, TEXT("No handle"));
      return false;
  }
  if ( GetImage(RenderTargetTexture) == false ) 
  {
    UE_LOG(LogTemp, Error, TEXT("Could not get render target texture!"));
    return false;
  }

  AR2VideoBufferT buff;
  /// set actual RGB bytes
  buff.buff  = (ARUint8*)ColorBuffer.GetData();
  /// these are ignored, single plane
  buff.bufPlanes = nullptr;
  buff.bufPlaneCount = 0;
  /// set  LUMA version of RGB bytes
  buff.buffLuma = (ARUint8*)LuminanceBuffer.GetData();
  buff.fillFlag = VIDEOBUFFER_IS_VALID;
  // not used by arDetectMarker, but could be set to 'now' just in case it ever IS used.
  //buff.time = ;

  if (arDetectMarker(handle_, &buff) < 0) {
    UE_LOG(LogTemp, Error, TEXT("Running arDetectMarker()"));
    return false;
  }

  markerInfo0 = arGetMarker(handle_);
  markerNum0  = arGetMarkerNum(handle_);

  // Update square markers.
  bool success = true;

  for (auto it(trackables_.CreateIterator()); it; it++) {
    ATrackableSquare * square = Cast<ATrackableSquare>(*it);
    if ( square ) {
      success &= square->UpdateWithDetectedMarkers(markerInfo0, markerNum0, handle3d_);
    } else {
      UE_LOG(LogTemp, Warning, TEXT("Actor not instance of ATrackableSquare - cannot update detected markers."));
    }
  }

  return true;
}

void ATrackerSquare::StopTracking()
{
  if (patternHandle_) {
    arPattDeleteHandle(patternHandle_);
    patternHandle_ = nullptr;
  }
}

void ATrackerSquare::Deinitialize()
{

  if (handle3d_) {
    ar3DDeleteHandle(&handle3d_); // Sets ar3DHandle0 to NULL.
  }
  if (handle_) {
    arPattDetach(handle_);
    arDeleteHandle(handle_);
    handle_ = nullptr;
  }

}
void ATrackerSquare::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Deinitialize();
}


bool ATrackerSquare::SaveRenderTargetToFile()
{
     
     FIntPoint destSize(RenderTargetTexture->GetSurfaceWidth(), RenderTargetTexture->GetSurfaceHeight());
     TArray<uint8> CompressedBitmap;
  
     FImageUtils::CompressImageArray(destSize.X, destSize.Y, ColorBuffer, CompressedBitmap);
     FString fileDestination = "colorbuffer.png";
     bool imageSavedOk = FFileHelper::SaveArrayToFile(CompressedBitmap, *fileDestination);
     CompressedBitmap.Empty();
     
     TArray<FColor> tmp;
     tmp.Init(FColor(),LuminanceBuffer.Num());
     for( int i=0;i<LuminanceBuffer.Num();i++)
     {
        tmp[i].R = tmp[i].G = tmp[i].B = LuminanceBuffer[i];
        tmp[i].A = 255;
      }
     FImageUtils::CompressImageArray(destSize.X, destSize.Y, tmp, CompressedBitmap);
     fileDestination = "lumbuffer.png";
     imageSavedOk = FFileHelper::SaveArrayToFile(CompressedBitmap, *fileDestination);
 
     return imageSavedOk;
 }
