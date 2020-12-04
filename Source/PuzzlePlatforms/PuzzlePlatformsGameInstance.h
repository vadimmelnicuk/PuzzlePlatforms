// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PuzzlePlatformsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPuzzlePlatformsGameInstance : public UGameInstance, public IMenuInterface
{
    GENERATED_BODY()

public:
    UPuzzlePlatformsGameInstance(const FObjectInitializer &ObjectInitializer);
    virtual void Init() override;
    void StartSession();

    UFUNCTION(BlueprintCallable)
    void LoadMenu();

    UFUNCTION(BlueprintCallable)
    void InGameLoadMenu();

    UFUNCTION(Exec)
    void Host(FString ServerName) override;

    UFUNCTION(Exec)
    void Join(uint32 Index, FString ServerName) override;

    UFUNCTION(Exec)
    void End() override; 

    UFUNCTION(Exec)
    void Destroy() override;

    UFUNCTION(Exec)
    void LoadMainMenu() override;

    UFUNCTION(Exec)
    void RefreshServerList() override;

private:
    TSubclassOf<class UUserWidget> MenuClass;
    TSubclassOf<class UUserWidget> InGameMenuClass;
    class UMainMenu *Menu;
    class UInGameMenu *InGameMenu;
    class IOnlineSubsystem *Subsystem;
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;
    FString HostServerName;

    void CreateSession();
    void OnCreateSessionComplete(FName SessionName, bool Success);
    void OnStartSessionComplete(FName SessionName, bool Success);
    void OnEndSessionComplete(FName SessionName, bool Success);
    void OnDestroySessionComplete(FName SessionName, bool Success);
    void OnFindSessionsComplete(bool Success);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
