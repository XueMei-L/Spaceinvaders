// Fill out your copyright notice in the Description page of Project Settings.

#include "SIGameModeBase.h"
#include "SIPawn.h"
#include "SIPlayerController.h"
#include "InvaderSquad.h"
#include "Kismet/GameplayStatics.h"
#include "Invader.h"
#include "SISaveGame.h"


ASIGameModeBase::ASIGameModeBase()
	: spawnLocation{ }
	, nInvaderRows{ 3 }
	, nInvaderCols{ 3 }
	, spawnedInvaderSquad{}
{
	DefaultPawnClass = ASIPawn::StaticClass();
	PlayerControllerClass = ASIPlayerController::StaticClass();
	InvaderSquadClass = AInvaderSquad::StaticClass();
}

void ASIGameModeBase::BeginPlay() {

	Super::BeginPlay();

	// Default values
	nInvaderRows = 3;
	nInvaderCols = 3;

	//Spawn a squad of invaders
	RegenerateSquad();

	// Delegate bindings:
	this->NewSquad.AddUObject(this, &ASIGameModeBase::OnNewSquad);
	this->PlayerZeroLifes.BindUObject(this, &ASIGameModeBase::OnPlayerZeroLifes);

}

void ASIGameModeBase::RegenerateSquad() {

	if (this->spawnedInvaderSquad != nullptr)
		this->spawnedInvaderSquad->Destroy();

	if (InvaderSquadClass) {
        this->spawnedInvaderSquad = GetWorld()->SpawnActor<AInvaderSquad>(InvaderSquadClass, spawnLocation, FRotator::ZeroRotator);

        if (this->spawnedInvaderSquad) {
            this->spawnedInvaderSquad->SetRows(nInvaderRows);
            this->spawnedInvaderSquad->SetCols(nInvaderCols);
			this->spawnedInvaderSquad->CreateInvaders();
        }
    }
}

// new squad
void ASIGameModeBase::OnNewSquad(int32 lifes) {
    CurrentRound++;
    // max squad is 7 * 4
    if (nInvaderRows < 7 || nInvaderCols < 4) 
    {
        if (CurrentRound % 2 == 0)
        {
            if (nInvaderCols < 4) nInvaderCols++;
            else if (nInvaderRows < 7) nInvaderRows++;
        }
        else
        {
            if (nInvaderRows < 7) nInvaderRows++;
            else if (nInvaderCols < 4) nInvaderCols++;
        }
        InvaderHP = 1; 
    }
    else 
    {
		// then every round invaders hp + 1
        InvaderHP++; 
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
        FString::Printf(TEXT("Round: %d | Grid: %dx%d | HP: %d"), 
        CurrentRound, nInvaderRows, nInvaderCols, InvaderHP));
	
	RegenerateSquad();

	// set all invader new hp
	if (spawnedInvaderSquad) {
		for (AInvader* invader : spawnedInvaderSquad->GetSquadMembers()) {
			if (invader) {
				invader->MaxHP = InvaderHP;
				invader->CurrentHP = invader->MaxHP;

				UE_LOG(LogTemp, Warning, TEXT("Invader HP set to: %d"), invader->CurrentHP);
			}
		}
	}
}


void ASIGameModeBase::EndGame() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("EndGame Called - Level Switching..."));

	if (this->spawnedInvaderSquad != nullptr)
		this->spawnedInvaderSquad->Destroy();

	// Close level and open game over level
	UGameplayStatics::OpenLevel(this, FName("GameOver"));
}

void ASIGameModeBase::OnPlayerZeroLifes() {
	EndGame();
}

// High Scores
int32 ASIGameModeBase::GetSavedHighScore()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        USISaveGame* LoadInstance = Cast<USISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
        if (LoadInstance)
        {
            return LoadInstance->HighScore;
        }
    }
    return 0;
}

// High Scores
void ASIGameModeBase::UpdateHighScore(int32 CurrentScore)
{
    USISaveGame* SaveInstance = nullptr;

    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        SaveInstance = Cast<USISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    }
    else
    {
        SaveInstance = Cast<USISaveGame>(UGameplayStatics::CreateSaveGameObject(USISaveGame::StaticClass()));
    }

    if (SaveInstance)
    {
        if (CurrentScore > SaveInstance->HighScore)
        {
            SaveInstance->HighScore = CurrentScore;
            UGameplayStatics::SaveGameToSlot(SaveInstance, SaveSlotName, 0);
        }
    }
}
