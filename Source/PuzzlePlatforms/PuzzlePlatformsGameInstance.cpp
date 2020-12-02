// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzlePlatformsGameInstance.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"

#include "PlatformTrigger.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/InGameMenu.h"


UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance(const FObjectInitializer &ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
    if (!ensure(MainMenuBPClass.Class != nullptr)) return;
    MenuClass = MainMenuBPClass.Class;

    ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/MenuSystem/WBP_InGameMenu"));
    if (!ensure(InGameMenuBPClass.Class != nullptr)) return;
    InGameMenuClass = InGameMenuBPClass.Class;
}

void UPuzzlePlatformsGameInstance::Init()
{
    Subsystem = IOnlineSubsystem::Get();

    if (Subsystem != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *Subsystem->GetSubsystemName().ToString());
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Found session interface"));
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);
            SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnStartSessionComplete);
            SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnEndSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
    }
}

void UPuzzlePlatformsGameInstance::LoadMenu()
{
    if (!ensure(MenuClass != nullptr)) return;

    Menu = CreateWidget<UMainMenu>(this, MenuClass);
    if (!ensure(Menu != nullptr)) return;

    Menu->Setup();
    Menu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::InGameLoadMenu()
{
    if (!ensure(InGameMenuClass != nullptr)) return;

    InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
    if (!ensure(InGameMenu != nullptr)) return;

    InGameMenu->Setup();
    InGameMenu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::Host()
{
    if (SessionInterface.IsValid())
    {
        FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(CurrentSessionName);

        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(CurrentSessionName);
        }
        else
        {
            CreateSession();
        }
    }
}

void UPuzzlePlatformsGameInstance::Join(uint32 Index)
{
    if (!SessionInterface.IsValid()) return;
    if (!SessionSearch.IsValid()) return;

    if (Menu != nullptr)
    {
        Menu->TearDown();
    }

    SessionInterface->JoinSession(0, CurrentSessionName, SessionSearch->SearchResults[Index]);
}

void UPuzzlePlatformsGameInstance::End() 
{
    SessionInterface->EndSession(CurrentSessionName);
}

void UPuzzlePlatformsGameInstance::Destroy() 
{
    SessionInterface->DestroySession(CurrentSessionName);
}

void UPuzzlePlatformsGameInstance::LoadMainMenu()
{
    APlayerController *PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr))
        return;

    PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::RefreshServerList() 
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    if (SessionSearch.IsValid())
    {
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        SessionSearch->MaxSearchResults = 1000;
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    }
}

void UPuzzlePlatformsGameInstance::CreateSession() 
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        SessionSettings.bIsLANMatch = false;
        SessionSettings.NumPublicConnections = 5;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;

        SessionInterface->CreateSession(0, CurrentSessionName, SessionSettings);
    }
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
    if (!Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not create session"));
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Created session: %s"), *SessionName.ToString());
    }
    

    if (Menu != nullptr)
    {
        Menu->TearDown();
    }

    UEngine *Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Hosting %s"), *SessionName.ToString()));

    UWorld *World = GetWorld();
    if (!ensure(World != nullptr)) return;

    World->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
}

void UPuzzlePlatformsGameInstance::OnStartSessionComplete(FName SessionName, bool Success) 
{
    if (Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Started session: %s"), *SessionName.ToString());
    }
}

void UPuzzlePlatformsGameInstance::OnEndSessionComplete(FName SessionName, bool Success) 
{
    if (Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ended session: %s"), *SessionName.ToString());
    }
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
    if (Success)
    {
        // CreateSession();
        UE_LOG(LogTemp, Warning, TEXT("Destroyed session: %s"), *SessionName.ToString());
    }
}

void UPuzzlePlatformsGameInstance::OnFindSessionsComplete(bool Success) 
{
    if (Success && SessionSearch.IsValid() && Menu != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Finished sessions search"));

        TMap<FString, FString> Servers;

        for (FOnlineSessionSearchResult &SearchResult : SessionSearch->SearchResults)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found session ID: %s"), *SearchResult.GetSessionIdStr());
            Servers.Add(SearchResult.Session.GetSessionIdStr(), SearchResult.Session.OwningUserName);
        }

        Menu->SetServerList(Servers);
    }
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) 
{
    if (!SessionInterface.IsValid()) return;

    FString Address;
    if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get connect string"));
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Joining %s"), *Address);
    }

    UEngine *Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

    APlayerController *PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}
