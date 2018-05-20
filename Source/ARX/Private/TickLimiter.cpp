// Fill out your copyright notice in the Description page of Project Settings.


#include "TickLimiter.h"
#include <algorithm> // max

using namespace std;

TickLimiter::TickLimiter() : limit_(0.0f)
{
}

TickLimiter::~TickLimiter()
{
}



void TickLimiter::Reset()
{
  limit_ = 0.0f;
}

void TickLimiter::SetLimit(float seconds)
{
  limit_ = std::max( 0.0f, seconds );
  if ( FMath::IsNearlyZero(limit_, 0.001f))
  {
      UE_LOG(LogTemp, Log, TEXT("TickLimiter limit set to zero, watch your Update() calls."));
  }
  else
  {
      UE_LOG(LogTemp, Log, TEXT("TickLimiter limit set to %f seconds. "), limit_);  
  }
  
}

bool TickLimiter::Update(float passedTimeSeconds)
{
  bool hasLimitPassed = false;
  
  passedTimeSinceReset_ += passedTimeSeconds;
  // take care that negative values do not creep in.
  passedTimeSinceReset_ = std::max(0.0f, passedTimeSinceReset_);
  
  // if limit is not set high enough, tick passes always
  if ( FMath::IsNearlyZero(limit_,0.001f) )
  {
    hasLimitPassed = true;
  }
  else if ( passedTimeSinceReset_ >= limit_)
  {
      passedTimeSinceReset_ -= limit_*FMath::DivideAndRoundDown(passedTimeSinceReset_,limit_);
      hasLimitPassed = true;
  }
  return hasLimitPassed;
}
