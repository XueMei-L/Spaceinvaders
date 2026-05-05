// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InvaderSquad.generated.h"

enum class InvaderMovementType : uint8;

UCLASS()
class SPACEINVADERS_API AInvaderSquad : public AActor
{
	GENERATED_BODY()

public:

	AInvaderSquad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;


public:
	//--------------------------------------------------------
	// Root Scene Component to modify location, rotation,...
	//--------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent* Root;

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Initialize();

	UFUNCTION(BlueprintCallable)
	void UpdateSquadState(float delta);

	UFUNCTION(BlueprintCallable)
	void SetRows(int32 nrows);

	UFUNCTION(BlueprintCallable)
	void SetCols(int32 ncols);

	UFUNCTION(BlueprintCallable)
	int32 GetRows();

	UFUNCTION(BlueprintCallable)
	int32 GetCols();

	UFUNCTION(BlueprintCallable)
	int32 GetNumberOfMembers();

	// to get the members of the squad
	UFUNCTION(BlueprintCallable)
	const TArray<class AInvader*>& GetSquadMembers() const { return SquadMembers; }

protected:

	// Mesh
	// UPROPERTY(Transient)
    // bool bIsSpawningSquad = false;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float horizontalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float verticalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	InvaderMovementType state;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	InvaderMovementType previousState;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float freeJumpRate;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	TSubclassOf<class AInvader> invaderClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Squad Spawner")
	class AInvader* invaderTemplate;


	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	int32 nRows;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	int32 nCols;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	float extraSeparation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class ASIGameModeBase* MyGameMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<class AInvader*> SquadMembers;


private:

	

	int32 numberOfMembers;

	UPROPERTY(VisibleAnywhere)
	float timeFromLastFreeJump;

	void SquadOnLeftSide();

	void SquadOnRightSide();

	void SquadFinishesDown();

	void RemoveInvader(int32 ind);


	// Values for initializing defaults
	static const int32 defaultNRows = 1;
	static const int32 defaultNCols = 1;
	static  constexpr const float defaultHorizontalVelocity = 1000.0f;
	static  constexpr const float defaultVerticalVelocity = 1000.0f;
	static  constexpr const float defaultExtraSeparation = 0.0f;

};
