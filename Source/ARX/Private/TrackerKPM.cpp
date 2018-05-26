#include "TrackerKPM.h"
#include <Misc/FileHelper.h>
using ELogVerbosity::Type;
using namespace std;


bool
UTrackerKPM::Prepare(ARParamLT *arParamLT )
{
    kpmHandle = kpmCreateHandle(arParamLT);
    
    if ( kpmHandle == nullptr )
    {
			UE_LOG(LogTemp, Error, TEXT("kpmCreateHandle failed"));
			return false;
    }
   
    imageSize = kpmHandleGetXSize(kpmHandle) * kpmHandleGetYSize(kpmHandle);
    
    
    UE_LOG(LogTemp, Log, TEXT("KpmMarkerDetector: image size  %d %d"), kpmHandleGetXSize(kpmHandle),kpmHandleGetYSize(kpmHandle));
    imageLumaPtr  = new ARUint8[imageSize];
    return true;
}

void
UTrackerKPM::SeekInitialMarker( uint8 * image )
{
  
  if ( imageMutex_.TryLock()  )
  {    
    // copy image to be handled
    FMemory::Memcpy( imageLumaPtr, image, imageSize );
    // wake up thread
    seek_->Trigger();
    imageMutex_.Unlock();
  }
  
}
  
void 
UTrackerKPM::StartThread()
{
    
    seek_ = FGenericPlatformProcess::GetSynchEventFromPool(false);
    if ( seek_ == nullptr  )
      UE_LOG(LogTemp, Error, TEXT("Could not create seek event "));
    found_ = FGenericPlatformProcess::GetSynchEventFromPool(false);
    if ( found_ == nullptr  )
      UE_LOG(LogTemp, Error, TEXT("Could not create found event "));
    //threadHandle = threadInit(0, trackingInitHandle, trackingInitMain);
    Thread = FRunnableThread::Create(this, TEXT("NFT marker detection worker"),0, TPri_Normal);
    
}

void 
UTrackerKPM::StopThread()
{
    threadRunning = false;
    // wake up thread
    seek_->Trigger();
    found_->Trigger();
}
bool
UTrackerKPM::Init()
{
  if (kpmSetRefDataSet(kpmHandle, kpmDataset) < 0) {
    UE_LOG(LogTemp, Error, TEXT("Merging of kpmSetRefDataSet"));
    return false;
  }
  kpmDeleteRefDataSet(&kpmDataset);
  return true;
}
void
UTrackerKPM::Stop()
{
  
    FGenericPlatformProcess::ReturnSynchEventToPool(seek_);
    FGenericPlatformProcess::ReturnSynchEventToPool(found_);
    seek_ = nullptr;    
    found_ = nullptr;
    delete [] imageLumaPtr;
    imageLumaPtr = nullptr;
}


uint32 
UTrackerKPM::Run()
{
    
    KpmResult              *kpmResult = NULL;
    int                     kpmResultNum;
    float                   errorAmount = 0.0f;
    bool                    foundResult = false;
  
    // perform sanity checks for values...
    threadRunning = true;
    
    kpmGetResult( kpmHandle, &kpmResult, &kpmResultNum );
    UE_LOG(LogTemp, Log, TEXT("KpmMarkerDetector resultnum %d"), kpmResultNum);
    
    while ( threadRunning )
    {

        // prevent another detection run unless requested.
        if ( seek_->Wait() ) 
        {   
            imageMutex_.Lock();
            // when exit is requested.
            if ( threadRunning == false ) 
            {
                UE_LOG(LogTemp, Log, TEXT("KpmMarkerDetector LOOP exiting... "));
                continue;
            }
            
            
            kpmMatching(kpmHandle, imageLumaPtr);
            
            foundResult = false;
                        
            for( int i = 0; i < kpmResultNum; i++ ) {
                
                 
                // skip if no proper result.
                if( kpmResult[i].camPoseF != 0 ) continue;
                 
                // Take the first or best result.
                if( foundResult == false || kpmResult[i].error < errorAmount   ) 
                { 
                    foundResult = true;
                    // page == marker ID of sorts, set by surface id on KPM stuff.
                    page = kpmResult[i].pageNo;
                    for (int j = 0; j < 3; j++) 
                    {
                        for (int k = 0; k < 4; k++) 
                        {
                            trans[j][k] = kpmResult[i].camPose[j][k];
                        }
                    }
                    errorAmount = kpmResult[i].error;
                }
            }
            
            if ( foundResult ) 
            {
              // mark result available 
              found_->Trigger();
            }
            
            imageMutex_.Unlock();
         }
    }
    UE_LOG(LogTemp, Log, TEXT("Kpm Detection Thread exiting..."));
    return 0;
}

bool UTrackerKPM::Load(const FString & path)
{
  
  KpmRefDataSet *tmpDataset = nullptr;
  
  UE_LOG(LogTemp, Log, TEXT("Reading %s.fset3"), *path);
  // Load data   
  if (kpmLoadRefDataSet(TCHAR_TO_UTF8(*path), "fset3", &tmpDataset) < 0 ) 
  {
    UE_LOG(LogTemp, Error, TEXT("Error reading %s.fset3"), *path);
    return false;
  }
  
  // This one replaces one page number with another, or replaces all page 
  // numbers in case KpmChangePageNoAllPages.
  if (kpmChangePageNoOfRefDataSet(tmpDataset, KpmChangePageNoAllPages, numPages) < 0) 
  {
    UE_LOG(LogTemp, Error, TEXT("kpmChangePageNoOfRefDataSet"));
    return false;
  }
  // merge loaded data into a single dataset
  if (kpmMergeRefDataSet(&kpmDataset, &tmpDataset) < 0) {
    UE_LOG(LogTemp, Error, TEXT("kpmMergeRefDataSet"));
    return false;
  }
  // increase number of loaded pages
  numPages++;
  return true;
}
