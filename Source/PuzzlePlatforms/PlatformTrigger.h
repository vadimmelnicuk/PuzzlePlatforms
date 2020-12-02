// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformTrigger.generated.h"

UCLASS()
class PUZZLEPLATFORMS_API APlatformTrigger : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APlatformTrigger();
    virtual void Tick(float DeltaTime) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    class UBoxComponent *TriggerVolume;

    UPROPERTY(EditAnywhere)
    class UStaticMeshComponent *PressurePad;

    UPROPERTY(EditAnywhere)
    TArray<class AMovingPlatform *> PlatformsToTrigger;

    UPROPERTY(VisibleAnywhere)
    class UAudioComponent *TriggerAudioComponent;

    UPROPERTY(EditAnywhere)
    USoundBase *TriggerSound;

    bool PressurePadActive = false;
    float PressurePadInitialZ;
    float PressurePadCurrentZ;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);
};