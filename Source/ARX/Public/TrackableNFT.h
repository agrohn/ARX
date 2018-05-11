#ifndef ARMARKERNFT_H
#define ARMARKERNFT_H
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS



#include <ARX/AR/ar.h>
#include <ARX/AR/config.h>
#include <ARX/AR/param.h>			// arParamDisp()
#include <ARX/AR/ar.h>
#include <ARX/AR/arFilterTransMat.h>
#include <ARX/AR2/tracking.h>
#include <Trackable.h>
#include <string>
//////////////////////////////////////////////////////////////////////////////////////
class TrackableNFT : public Trackable
{
public:
    bool       valid{false};  ///< Used to express validity.
    bool       validPrev{false}; ///<Used to incidate change in validity (in case other stuff relies upon it).
    
    ARdouble   marker_width{0.0};
    ARdouble   marker_height{0.0};
    // ARMarker private
    ARFilterTransMatInfo *ftmi{nullptr};
    ARdouble   filterCutoffFrequency{0.0};
    ARdouble   filterSampleRate{0.0};
    // ARMarkerNFT
    /// This connects data to KPM detection
    int        			 pageNo{0}; 
    std::string      datasetPathname;
    
    TrackableNFT();
  virtual ~TrackableNFT();
    /// \param fcutoffFrequency  Pose estimate filtering for the preceding marker. By default set to ARToolkit default.
    void Initialize( const std::string & path, 
                     float cutoffFrequency = AR_FILTER_TRANS_MAT_CUTOFF_FREQ_DEFAULT);
    void Update() ;
    bool UpdateWithDetectedMarkers(ARMarkerInfo * markerInfo, int markerNum, AR3DHandle *ar3DHandle) override;                 
};
//////////////////////////////////////////////////////////////////////////////////////

#endif // ARMARKERNFT_H

