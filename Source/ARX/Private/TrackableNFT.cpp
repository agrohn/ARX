#include "TrackableNFT.h"

using namespace std;

TrackableNFT::TrackableNFT()
{
}

TrackableNFT::~TrackableNFT()
{
}

void
TrackableNFT::Initialize(const string & path, float cutoffFrequency)
{
    filterCutoffFrequency = cutoffFrequency;
    filterSampleRate      = AR_FILTER_TRANS_MAT_SAMPLE_RATE_DEFAULT;
    
    ftmi = arFilterTransMatInit( filterSampleRate, filterCutoffFrequency);
    
    datasetPathname = path ;
    // yes
    pageNo = -1;
}

void
TrackableNFT::Update()
{
  
  
}

bool
TrackableNFT::UpdateWithDetectedMarkers(ARMarkerInfo * markerInfo, int markerNum, AR3DHandle *ar3DHandle) 
{
    return false;
}
