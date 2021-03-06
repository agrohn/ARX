#pragma once
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif

#endif // !ARX_EXPORTS
#include <GameFramework/Actor.h>
#include <Engine/SceneCapture2D.h>
#include <ImageUtils.h>
#include <Engine/TextureRenderTarget2D.h>
#include <HAL/Runnable.h>
#include <HAL/RunnableThread.h>
#include <HAL/CriticalSection.h>
#include <HAL/Event.h>
#include <ARX/AR/config.h>
#include <ARX/AR/param.h>			// arParamDisp()
#include <ARX/AR/ar.h>
#include <ARX/AR/arFilterTransMat.h>
#include <ARX/AR2/tracking.h>
#include <ARX/KPM/kpm.h>
#include <vector>
/** Tracking result and its relevant data.*/
struct KPMTrackingResult 
{
    // Transform containing pose of tracked image.
    float                   trans[3][4];    
    // Assigned page number of tracked image.
    int                     page{-1};
    bool                    found{false};
    float                   error{0.0f};
};
/* Performs first feature detection for NFT marker. */
class  UTrackerKPM : public FRunnable
{
private:
  
public:
    // KPM-related data.
    KpmHandle              *kpmHandle{nullptr};      
    // Pointer to image being tracked.
    ARUint8                *imageLumaPtr{nullptr};
    
    /// Data set for all pages that will be tracked.
    KpmRefDataSet *         kpmDataset{nullptr};
    // Number of pages loaded.
    int                     numPages;
    
    FCriticalSection        imageMutex_;
    // Bytes per image.
    int                     imageSize;  

      // Transform containing pose of tracked image.
    float                   trans[3][4];    
    // Assigned page number of tracked image.
    int                     page;
        
    FEvent *                found_{nullptr};
    FEvent *                seek_{nullptr};
    // Runnable-related events.
    virtual bool    Init() override;
    virtual uint32  Run() override;
    virtual void    Stop() override;
    
    void              StopThread();
    void              StartThread();
    /// controls thread execution.
    bool              threadRunning{true}; 
    // Running thread
    FRunnableThread * Thread{nullptr};
    
    bool Prepare(ARParamLT *ptr);
    void SeekInitialMarker( uint8 *image );
    bool Load( const FString & path );
    
    
};

