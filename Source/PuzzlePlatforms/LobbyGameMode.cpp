// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "TimerManager.h"
#include "PuzzlePlatformsGameInstance.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) 
{
    Super::PostLogin(NewPlayer);

    ++PlayersCount;
    UE_LOG(LogTemp, Warning, TEXT("Players Count: %i"), PlayersCount);

    if (PlayersCount > 1)
    {
        GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::StartGame, 5);
    }
}

void ALobbyGameMode::Logout(AController* Exiting) 
{
    Super::Logout(Exiting);

    --PlayersCount;
    UE_LOG(LogTemp, Warning, TEXT("Players Count: %i"), PlayersCount);
}

void ALobbyGameMode::StartGame() 
{
    auto GameInstance = Cast<UPuzzlePlatformsGameInstance>(GetGameInstance());
    if (GameInstance == nullptr) return;

    GameInstance->StartSession();

    UWorld *World = GetWorld();
    if (!ensure(World != nullptr)) return;

    bUseSeamlessTravel = true;
    World->ServerTravel("/Game/PuzzlePlatforms/Maps/Game?listen");
}
