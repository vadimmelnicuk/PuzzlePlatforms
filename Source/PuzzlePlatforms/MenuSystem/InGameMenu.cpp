// Fill out your copyright notice in the Description page of Project Settings.

#include "InGameMenu.h"
#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success)
        return false;

    if (!ensure(CancelButton != nullptr)) return false;
    CancelButton->OnClicked.AddDynamic(this, &UInGameMenu::CancelInGameMenu);

    if (!ensure(QuitButton != nullptr)) return false;
    QuitButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitGame);

    return true;
}

void UInGameMenu::CancelInGameMenu()
{
    TearDown();
}

void UInGameMenu::QuitGame()
{
    if (MenuInterface != nullptr)
    {
        TearDown();
        MenuInterface->LoadMainMenu();
        MenuInterface->Destroy();
    }
}
