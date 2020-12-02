// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"

#include "ServerRow.h"

UMainMenu::UMainMenu(const FObjectInitializer &ObjectInitializer) 
{
    ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/MenuSystem/WBP_ServerRow"));
    if (!ensure(ServerRowBPClass.Class != nullptr)) return;
    ServerRowClass = ServerRowBPClass.Class;
}

void UMainMenu::SetServerList(TMap<FString, FString> Servers) 
{
    UWorld *World = GetWorld();
    if (!ensure(World != nullptr)) return;

    ServerList->ClearChildren();

    uint32 i = 0;

    for (auto &Server : Servers)
    {
        UServerRow *Row = CreateWidget<UServerRow>(World, ServerRowClass);
        if (!ensure(Row != nullptr)) return;

        Row->ServerName->SetText(FText::FromString(Server.Key));
        Row->Owner->SetText(FText::FromString(Server.Value));
        Row->Setup(this, i);
        ++i;

        ServerList->AddChild(Row);
    }
}

void UMainMenu::SelectIndex(uint32 Index) 
{
    SelectedIndex = Index;
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (!ensure(HostButton != nullptr)) return false;
    HostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(JoinButton != nullptr)) return false;
    JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(BackButton != nullptr)) return false;
    BackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(ConnectButton != nullptr)) return false;
    ConnectButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

    if (!ensure(QuitButton != nullptr)) return false;
    QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);

    return true;
}

void UMainMenu::HostServer()
{
    if (MenuInterface != nullptr)
    {
        MenuInterface->Host();
    }
}

void UMainMenu::JoinServer()
{
    if (SelectedIndex.IsSet() && MenuInterface != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Index %d"), SelectedIndex.GetValue());
        MenuInterface->Join(SelectedIndex.GetValue());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Index not set"));
    }
}

void UMainMenu::OpenJoinMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(JoinMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(JoinMenu);

    if (MenuInterface != nullptr)
    {
        MenuInterface->RefreshServerList();
    }
}

void UMainMenu::OpenMainMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::QuitGame()
{
    UWorld *World = GetWorld();
    if (!ensure(World != nullptr)) return;

    APlayerController *PlayerController = World->GetFirstPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ConsoleCommand("quit");
}