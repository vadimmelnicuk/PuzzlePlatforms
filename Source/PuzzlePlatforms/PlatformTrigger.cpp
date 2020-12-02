// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MovingPlatform.h"

// Sets default values
APlatformTrigger::APlatformTrigger()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(FName("TriggerVolume"));
    if (!ensure(TriggerVolume != nullptr)) return;
    RootComponent = TriggerVolume;

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlatformTrigger::OnOverlapBegin);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlatformTrigger::OnOverlapEnd);

    TriggerAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("TriggerAudioComponent"));
    if (!ensure(TriggerAudioComponent != nullptr)) return;
    TriggerAudioComponent->SetupAttachment(RootComponent);

    PressurePad = CreateDefaultSubobject<UStaticMeshComponent>(FName("PressurePad"));
    if (!ensure(PressurePad != nullptr)) return;
    PressurePad->SetupAttachment(RootComponent);
}

// Called every frame
void APlatformTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PressurePad != nullptr)
    {
        FVector PressurePadLocation = PressurePad->GetRelativeLocation();

        if (PressurePadActive)
        {
            PressurePadCurrentZ = FMath::FInterpConstantTo(PressurePadCurrentZ, PressurePadInitialZ - 8.f, DeltaTime, 30);
        }
        else
        {
            PressurePadCurrentZ = FMath::FInterpConstantTo(PressurePadCurrentZ, PressurePadInitialZ, DeltaTime, 30);
        }

        PressurePadLocation.Z = PressurePadCurrentZ;
        PressurePad->SetRelativeLocation(PressurePadLocation);
    }
}

// Called when the game starts or when spawned
void APlatformTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (PressurePad != nullptr)
    {
        PressurePadInitialZ = PressurePad->GetRelativeLocation().Z;
        PressurePadCurrentZ = PressurePadInitialZ;
    }

    if (TriggerSound != nullptr)
    {
        // UE_LOG(LogTemp, Warning, TEXT("Sound Set"));
        TriggerAudioComponent->SetSound(TriggerSound);
    }
}

void APlatformTrigger::OnOverlapBegin(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
    PressurePadActive = true;

    if (TriggerSound != nullptr)
    {
        TriggerAudioComponent->Play();
    }

    for (AMovingPlatform *Platform : PlatformsToTrigger)
    {
        Platform->AddActiveTrigger();
    }
}

void APlatformTrigger::OnOverlapEnd(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
    PressurePadActive = false;

    for (AMovingPlatform *Platform : PlatformsToTrigger)
    {
        Platform->RemoveActiveTrigger();
    }
}
