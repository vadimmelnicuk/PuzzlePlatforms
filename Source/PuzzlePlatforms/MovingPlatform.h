// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "MovingPlatform.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API AMovingPlatform : public AStaticMeshActor
{
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere)
  float Speed = 20;

  UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
  FVector TargetLocation;

  AMovingPlatform();
  virtual void Tick(float DeltaTime) override;
  void AddActiveTrigger();
  void RemoveActiveTrigger();

protected:
  virtual void BeginPlay() override;

private:
  FVector GlobalTargetLocation;
  FVector GlobalStartLocation;

  UPROPERTY(EditAnywhere)
  int ActiveTriggers = 1;
};
