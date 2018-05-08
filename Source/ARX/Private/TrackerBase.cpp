// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackerBase.h"

TrackerBase::TrackerBase()
{
}

TrackerBase::~TrackerBase()
{
}

void
TrackerBase::Initialize()
{
  
}
bool 
TrackerBase::StartTracking()
{
  return false;
}

bool 
TrackerBase::Update()
{
  return false;
}

void 
TrackerBase::StopTracking()
{
  
}

void 
TrackerBase::Deinitialize()
{
  
}
  


bool TrackerBase::SetupCamera(FIntPoint cameraSize)
{
    // make default camera settings.
    // M_PI_4 = 45 vertical view
    arParamClearWithFOVy(&cameraParameter, cameraSize.X, cameraSize.Y, M_PI_4); 
    if (cameraParameter.xsize != cameraSize.X || cameraParameter.ysize != cameraSize.Y) {
        ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cameraParameter.xsize, cameraParameter.ysize);
        arParamChangeSize(&cameraParameter, cameraSize.X, cameraSize.Y, &cameraParameter);
    }
		
		
		UE_LOG(LogTemp, Warning, TEXT("*** Camera parameters ****"));
		// display camera param
    arParamDisp(&cameraParameter);


    if ((cameraParameterLT = arParamLTCreate(&cameraParameter, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL) {
		
		UE_LOG(LogTemp, Error, TEXT("setupCamera(): Error: arParamLTCreate.\n"));
		return false;
	}

	if ((handle_ = arCreateHandle(cameraParameterLT)) == NULL) {
		UE_LOG(LogTemp, Error, TEXT("setupCamera(): Error: arCreateHandle.\n"));
		return false;
	}
	pixelFormat_ = AR_PIXEL_FORMAT_RGBA;
	arSetPixelFormat(handle_, pixelFormat_);
	arSetDebugMode(handle_, AR_DEBUG_DISABLE);

	if ((handle3d_ = ar3DCreateHandle(&cameraParameter)) == NULL) {
		UE_LOG(LogTemp, Error, TEXT("setupCamera(): Error: ar3DCreateHandle.\n"));
		return false;
	}
    
	return true;
}

bool
TrackerBase::GetImage(UTextureRenderTarget2D * RenderTargetTexture)
{
  if ( RenderTargetTexture == nullptr )
  {
      UE_LOG(LogTemp, Error, TEXT("RenderTargetTexture is null"));
    return false;  
  }
  //RenderTargetTexture->UpdateResource();
  FRenderTarget  *renderTarget = RenderTargetTexture->GameThread_GetRenderTargetResource();
  
  //RenderTargetTexture->Resource
  FIntPoint mysize = renderTarget->GetSizeXY();
  //UE_LOG(LogTemp, Warning, TEXT("RenderTarget size: %d by %d"), mysize.X, mysize.Y);
  
  FReadSurfaceDataFlags readFlags(RCM_MinMax);
  ColorBuffer.Reset(mysize.X * mysize.Y);
  if ( renderTarget != nullptr && renderTarget->ReadPixels(ColorBuffer, readFlags) ) 
  {
      
      if ( LuminanceBuffer.Num() == 0 ) 
      {
        LuminanceBuffer.Init(0, ColorBuffer.Num());
      }
      //UE_LOG(LogTemp, Warning, TEXT("Pixels read cout %d"), ColorBuffer.Num());
      bool printed = false;
      int count = 0;
      
      
#if PLATFORM_ANDROID
      // In case of android, reverse order of vertical rows since textures are flipped.
      for( int rowFromStart=0;rowFromStart<RenderTargetTexture->SizeY/2;rowFromStart++)
      {
          int rowFromEnd = RenderTargetTexture->SizeY-1-rowFromStart;
          
          for( int i=0;i<RenderTargetTexture->SizeX;i++)
          {
            ColorBuffer.SwapMemory(rowFromStart*RenderTargetTexture->SizeX+i, rowFromEnd*RenderTargetTexture->SizeX+i);
          }
      }
#endif  
      for( int i=0;i<ColorBuffer.Num();i++)
      {
          
        FColor & color = ColorBuffer[i];
        color.A = 255;
        //ColorBuffer[i].G=255;
        LuminanceBuffer[i] = Luminance(color);
        
      }

    
  }
  else 
  {
      UE_LOG(LogTemp, Error, TEXT("FAILURE: Pixels NOT read"));
      return false;
  }
  return true;
}

uint8 
TrackerBase::Luminance( FColor & color )
{
  // BT 709.
  //return uint8(color.R*0.2126f+color.G*0.7152f+color.B*0.0722f);
  
  // CCIR_601 
  return uint8(color.R*0.299f+color.G*0.587f+color.B*0.114f);
}
