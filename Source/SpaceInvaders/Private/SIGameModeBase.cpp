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
		AInvaderSquad* squad = Cast<AInvaderSquad>(InvaderSquadClass->GetDefaultObject());
		if (squad) {
			this->spawnedInvaderSquad = Cast<AInvaderSquad>(GetWorld()->SpawnActor(InvaderSquadClass, &spawnLocation));
		}
	}
}


// void ASIGameModeBase::RegenerateSquad() {

//     if (spawnedInvaderSquad != nullptr)
//         spawnedInvaderSquad->Destroy();

//     if (InvaderSquadClass) {
//         spawnedInvaderSquad = Cast<AInvaderSquad>(
//             GetWorld()->SpawnActor(InvaderSquadClass, &spawnLocation)
//         );

//         if (spawnedInvaderSquad) {
//             // 设置新行数和列数
//             spawnedInvaderSquad->SetRows(nInvaderRows);   // 例如默认 3
//             spawnedInvaderSquad->SetCols(nInvaderCols);   // 根据难度递增
//             spawnedInvaderSquad->Initialize();            // 真正生成 invaders
//         }
//     }
// }

void ASIGameModeBase::OnNewSquad(int32 lifes) {
	// Add more invader in new round
	// GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("nInvaderCols: %d"), nInvaderCols));
	
	// // Increased difficulty: more invaders per row
	// if (nInvaderCols < 6)
	// {
		// 	nInvaderCols++;
		// }
		// RegenerateSquad();

	// 增加轮次
	CurrentRound++;
	InvaderHP = CurrentRound;
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("CurrentRound: %d"), CurrentRound));
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("InvaderHP: %d"), InvaderHP));
	
	RegenerateSquad();

	// 设置新一轮所有 Invader 的 HP
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
    // 检查存档是否存在
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        USISaveGame* LoadInstance = Cast<USISaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
        if (LoadInstance)
        {
            return LoadInstance->HighScore;
        }
    }
    return 0; // 没存档就返回 0
}

// High Scores
void ASIGameModeBase::UpdateHighScore(int32 CurrentScore)
{
    USISaveGame* SaveInstance = nullptr;

    // 如果有存档，先加载它；没有就创建一个新的
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
        // 只有当前分数超过最高分时才更新
        if (CurrentScore > SaveInstance->HighScore)
        {
            SaveInstance->HighScore = CurrentScore;
            UGameplayStatics::SaveGameToSlot(SaveInstance, SaveSlotName, 0);
        }
    }
}
