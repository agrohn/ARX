// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS

#include "CoreMinimal.h"
#include <ARX/AR/ar.h>
#include <Trackable.h>
#include <Engine/TextureRenderTarget2D.h>
/**
 * 
 */

class ARX_API TrackerBase 
{
 
protected:
  TrackerBase();
public:
  virtual ~TrackerBase();
  
  
  
  
  
  virtual void Initialize();
  virtual bool StartTracking();
  virtual bool Update(float deltaTime);
  virtual void StopTracking();
  virtual void Deinitialize();
  
  AR_PIXEL_FORMAT pixelFormat_;
  
  ARParam				cameraParameter; // camera parameters. 
  ARParamLT *		cameraParameterLT{nullptr}; // camera parameter lookup table.
  ARHandle *handle_{nullptr};
  AR3DHandle *handle3d_{nullptr};
  
  bool SetupCamera(FIntPoint cameraSize);
  void TearDownCamera();
  
  TArray<FColor> ColorBuffer;
	TArray<uint8> LuminanceBuffer;
  bool GetImage(UTextureRenderTarget2D * RenderTargetTexture);
  uint8 Luminance( FColor & color );
};

