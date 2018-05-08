// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#ifndef ARX_EXPORTS
#ifdef PLATFORM_WINDOWS
#define ARX_EXPORTS 1
#endif
#endif // !ARX_EXPORTS

#include "CoreMinimal.h"
#include <ARX/AR/ar.h>
#include <ARX/AR/arFilterTransMat.h>
/**
 * 
 */
class ARX_API Trackable
{
public:
  Trackable();
  virtual ~Trackable();
  virtual void Update() = 0;
  
  bool visible;
  bool visiblePrev;
  ARdouble   trans[3][4];
  ARFilterTransMatInfo *m_ftmi;
  
  
  void MatrixToTransform(ARdouble transformMatrix[3][4], FTransform & transform );
  virtual bool UpdateWithDetectedMarkers(ARMarkerInfo * markerInfo, int markerNum, AR3DHandle *ar3DHandle) = 0;
};
