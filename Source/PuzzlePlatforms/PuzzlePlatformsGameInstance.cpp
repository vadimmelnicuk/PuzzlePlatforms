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

void UPuzzlePlatformsGameInstance::StartSession() 
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->StartSession(FName(*HostServerName));
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

void UPuzzlePlatformsGameInstance::Host(FString ServerName)
{
    HostServerName = ServerName;

    if (SessionInterface.IsValid())
    {
        FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(FName(*HostServerName));

        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(FName(*HostServerName));
        }
        else
        {
            CreateSession();
        }
    }
}

void UPuzzlePlatformsGameInstance::Join(uint32 Index, FString ServerName)
{
    if (!SessionInterface.IsValid()) return;
    if (!SessionSearch.IsValid()) return;

    if (Menu != nullptr)
    {
        Menu->TearDown();
    }

    HostServerName = ServerName;

    SessionInterface->JoinSession(0, FName(*ServerName), SessionSearch->SearchResults[Index]);
}

void UPuzzlePlatformsGameInstance::End() 
{
    SessionInterface->EndSession(FName(*HostServerName));
}

void UPuzzlePlatformsGameInstance::Destroy()
{
    SessionInterface->DestroySession(FName(*HostServerName));
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

        if (Subsystem->GetSubsystemName() == "NULL")
        {
            SessionSettings.bIsLANMatch = true;
        }
        else
        {
            SessionSettings.bIsLANMatch = false;
        }

        SessionSettings.NumPublicConnections = 5;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.Set(TEXT("ServerName"), HostServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        SessionInterface->CreateSession(0, FName(*HostServerName), SessionSettings);
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

    World->ServerTravel("/Game/PuzzlePlatforms/Maps/Lobby?listen");
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

        TArray<FServerData> Servers;
        for (FOnlineSessionSearchResult &SearchResult : SessionSearch->SearchResults)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found session ID: %s"), *SearchResult.GetSessionIdStr());
            FServerData ServerData;
            
            ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
            ServerData.CurrentPlayers = ServerData.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
            ServerData.HostUsername = SearchResult.Session.OwningUserName;

            FString ServerName;
            if (SearchResult.Session.SessionSettings.Get(TEXT("ServerName"), ServerName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Server name found in settings: %s"), *ServerName);
                ServerData.Name = ServerName;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Server name was not found."));
                ServerData.Name = "Server name was not found.";
            }

            Servers.Add(ServerData);
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
