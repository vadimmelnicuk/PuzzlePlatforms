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

void UMainMenu::SetServerList(TArray<FServerData> Servers) 
{
    ServersData = Servers;

    UWorld *World = GetWorld();
    if (!ensure(World != nullptr)) return;

    ServerList->ClearChildren();

    uint32 i = 0;

    for (FServerData &Server : ServersData)
    {
        UServerRow *Row = CreateWidget<UServerRow>(World, ServerRowClass);
        if (!ensure(Row != nullptr)) return;

        Row->ServerName->SetText(FText::FromString(Server.Name));
        Row->CurrentPlayers->SetText(FText::AsNumber(Server.CurrentPlayers));
        Row->MaxPlayers->SetText(FText::AsNumber(Server.MaxPlayers));
        Row->HostUsername->SetText(FText::FromString(Server.HostUsername));
        Row->Setup(this, i);
        ++i;

        ServerList->AddChild(Row);
    }
}

void UMainMenu::SelectIndex(uint32 Index) 
{
    SelectedIndex = Index;
    UpdateChildren();
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (!ensure(HostMenuButton != nullptr)) return false;
    HostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

    if (!ensure(JoinMenuButton != nullptr)) return false;
    JoinMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(HostBackButton != nullptr)) return false;
    HostBackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(JoinBackButton != nullptr)) return false;
    JoinBackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(HostServerButton != nullptr)) return false;
    HostServerButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

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
        FString ServerName = ServerNameTextBox->Text.ToString();
        MenuInterface->Host(ServerName);
    }
}

void UMainMenu::JoinServer()
{
    if (SelectedIndex.IsSet() && MenuInterface != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Index %d"), SelectedIndex.GetValue());
        MenuInterface->Join(SelectedIndex.GetValue(), ServersData[SelectedIndex.GetValue()].Name);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Index not set"));
    }
}

void UMainMenu::OpenHostMenu() 
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(HostMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(HostMenu);
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

void UMainMenu::UpdateChildren() 
{
    for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
    {
        UServerRow *Row = Cast<UServerRow>(ServerList->GetChildAt(i));
        if (Row != nullptr)
        {
            Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
        }
    }
}