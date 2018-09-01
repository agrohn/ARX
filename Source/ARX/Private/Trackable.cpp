// Fill out your copyright notice in the Description page of Project Settings.


#include "Trackable.h"
#include <Camera/CameraComponent.h>
#include <Camera/CameraTypes.h>
Trackable::Trackable() : parentCameraActor_(nullptr)
{
}

Trackable::~Trackable()
{
  
}

void
Trackable::MatrixToTransform( ARdouble transformMatrix[3][4], FTransform & transform )
{

    ARdouble transInverted[3][4];
    ARdouble quaternion[4];
    arUtilMatInv(transformMatrix, transInverted);
    
    
    FVector pos = FVector(transformMatrix[2][3], -transformMatrix[0][3], transformMatrix[1][3]);
    
    auto & tmp = transInverted;
  
   
    // Extract Head/Yaw, Pitch and Roll from matrix. 
    // UE4 FMatrix rotation conversion seem to compute it differently it some fashion, 
    // and do not provide correct result.
    float h = atan2f(-tmp[2][0], tmp[2][2]);
    float p = asinf( tmp[2][1]);
    float r = 0;
    if ( FMath::IsNearlyZero(cosf(p),  0.001f) )
    {
        r = atan2f(tmp[1][0], tmp[0][0]);
    }
    else r = atan2f( -tmp[0][1], tmp[1][1]);
    
   
   
    //UE_LOG(LogTemp, Warning, TEXT("Matrix rotator: %s"),*matT.Rotator().ToString());
    
    //Construct proper rotation for UE coordinates
    FRotator rot;
    rot.Yaw = FMath::RadiansToDegrees(h);
    rot.Pitch = FMath::RadiansToDegrees(p);
    // ARToolkitX provides matrix in right-handed coordinate system, we need to flip roll value. 
    rot.Roll = -FMath::RadiansToDegrees(r);
    
    //UE_LOG(LogTemp, Warning, TEXT("Final rotator: %s"),*rot.ToString());
    transform = GetOffset();
    // Append offset, allows relative positioning of matching actor.
    transform.Accumulate(FTransform( rot, pos, FVector(1,1,1)));

}

bool Trackable::HasBecomeVisible() const
{
  return (visible == true && visiblePrev == false);
}

bool Trackable::HasBecomeInvisible() const
{
  return (visible == false && visiblePrev == true);
}

void Trackable::SetParentCameraActor(AActor* actor)
{
  parentCameraActor_ = actor;
}

FTransform Trackable::ApplyParentCameraToTransform(const FTransform & InTransform)
{
  UCameraComponent * cameraComponent = Cast<UCameraComponent>(parentCameraActor_->GetComponentByClass(UCameraComponent::StaticClass()));
  if ( cameraComponent ) 
  {
    FMinimalViewInfo viewInfo;
    cameraComponent->GetCameraView(0.0f, viewInfo);
    FTransform cameraTransform(viewInfo.Rotation, viewInfo.Location, FVector(1,1,1));
    return  InTransform * cameraTransform;
  } 
  else 
  {
    UE_LOG( LogTemp, Warning, TEXT("ApplyParentCameraToTransform: Parent camera actor missing, cannot do anything."));
    return InTransform;
  }
}


FTransform Trackable::GetOffset() const
{
  return FTransform::Identity;
}
