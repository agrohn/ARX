// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TickLimiter
{
  private:
    float limit_;
    float passedTimeSinceReset_;
public:
	TickLimiter();
	virtual ~TickLimiter();
  
  void Reset();
  void SetLimit(float seconds );
  bool Update(float passedTimeSeconds );
};
