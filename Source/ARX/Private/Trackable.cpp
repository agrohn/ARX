// Fill out your copyright notice in the Description page of Project Settings.


#include "Trackable.h"

Trackable::Trackable()
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
    ARdouble position[3];

    arUtilMatInv(transformMatrix, transInverted);
    arUtilMat2QuatPos(transInverted, quaternion, position);
    //float aspectRatioFix = (float)cameraSize.X / (float)cameraSize.Y;
    FVector pos = FVector(transformMatrix[2][3], transformMatrix[0][3], -transformMatrix[1][3]);
    //pos /= aspectRatioFix;
    //pos = FVector(markersNFT[0].trans[0][3]/1.777, markersNFT[0].trans[1][3]/1.777, -markersNFT[0].trans[2][3]/1.777);
    //Construct proper rotation for UE coordinates
    FRotator raw = FQuat(quaternion[0], quaternion[1], quaternion[2], quaternion[3]).Rotator();
    FRotator rot;
    rot.Yaw = -raw.Pitch;
    rot.Pitch = 180 - raw.Roll;
    rot.Roll = raw.Yaw;

    //rot = raw;
    //TODO
    // Need offset for target (from left down corner to center, for example)

    //UE_LOG(LogTemp, Log, TEXT("Pattern found, errorlevel is  %f"), errorLevel);    


    transform = FTransform( rot, pos, FVector(1,1,1));

}
