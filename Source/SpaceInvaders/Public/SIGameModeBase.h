// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "SIGameModeBase.generated.h"

// Delegates of this game:
DECLARE_DELEGATE(FStandardDelegateSignature)
DECLARE_MULTICAST_DELEGATE_OneParam(FOneParamMulticastDelegateSignature, int32);
DECLARE_DELEGATE_OneParam(FOneParamDelegateSignature, int32)

/**
 * 
 */
UCLASS()
class SPACEINVADERS_API ASIGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	FStandardDelegateSignature SquadOnLeftSide; // Invader-> Squad 
	FStandardDelegateSignature SquadOnRightSide; // Invader -> Squad
	FStandardDelegateSignature SquadFinishesDown; // Invader -> Squad
	FStandardDelegateSignature SquadSuccessful; // Invader -> GameMode, Player
	FOneParamMulticastDelegateSignature InvaderDestroyed; // Invader -> Squad Invader->Player

	FOneParamMulticastDelegateSignature NewSquad; // Squad -> Game Mode, Player 
	FStandardDelegateSignature PlayerZeroLifes; // Player -> Game Mode

	//------------------------------------------------
	//Spawned squad
	//------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Layout")
	TSubclassOf<class AInvaderSquad> InvaderSquadClass;

	//------------------------------------------------
	//Point where the squad is spawned at
	//---------------------------------------------
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Level Layout")
	FVector spawnLocation;

	//------------------------------------------------
	//Layout of the squad: number of rows
	//-----------------------------------------------
	// UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Level Layout")
	// int32 nInvaderRows;

	// //-----------------------------------------------
	// //Layout of the squad: number of columns
	// //-----------------------------------------------
	// UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Level Layout")
	// int32 nInvaderCols;

	ASIGameModeBase();

	// HUD class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> GameOverClass;

	// UFUNCTION(BlueprintCallable, Category = "Game")
    // void RestartGame();

	// row
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
	int32 nInvaderRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
	int32 nInvaderCols;

	// max cols for increasing difficulty
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
	// int32 MaxInvaderCols = 6;

	// Modify: add current round and invader HP----------------------
	// current round
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentRound = 1;  

	// invader HP
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 InvaderHP = 1;
	//---------------------------------------------------------------
	// 获取当前保存的最高分
    UFUNCTION(BlueprintCallable, Category = "Score")
    int32 GetSavedHighScore();

    // 更新最高分（如果当前分数更高的话）
    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateHighScore(int32 CurrentScore);
	//---------------------------------------------------------------

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Layout")
	AInvaderSquad* spawnedInvaderSquad;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void RegenerateSquad();

	// Delegate bindings
	UFUNCTION(BlueprintCallable)
	void OnNewSquad(int32 lifes);

	void EndGame();
	
	//  GameOver HUD
    UPROPERTY()
    UUserWidget* GameOverHUD = nullptr;

	UFUNCTION(BlueprintCallable)
	void OnPlayerZeroLifes();

	// High Scores
	const FString SaveSlotName = TEXT("Slot1");
};
