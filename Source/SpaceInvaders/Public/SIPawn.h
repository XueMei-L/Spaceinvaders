// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "SIPawn.generated.h"

UCLASS()
class SPACEINVADERS_API ASIPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:

	//Points per invader
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	int32 pointsPerInvader;

	//Points per squad
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	int32 pointsPerSquad;

	
	// Velocity of the pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defender config")
	float velocity;

	// Bullets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defender config")
		TSubclassOf<class ABullet> bulletClass;

	// Velocity of the player bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
		float bulletVelocity;
	
		//Lifes
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
		int32 playerLifes;
	
	//Audios
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
		class USoundCue* AudioShoot;

		//Audios: Explossion
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
		class USoundCue* AudioExplosion;

	// High Scores
	UPROPERTY(BlueprintReadWrite, Category = "SpaceInvaders")
    	int32 CurrentScore = 0;


	// Sets default values for this pawn's properties
	ASIPawn();

	// It could be possible to change the static mesh component during run time.
	UFUNCTION(BlueprintCallable)
		void SetStaticMesh(class UStaticMesh* staticMesh = nullptr, FString path = TEXT(""), FVector scale = FVector(1.0f, 1.0f, 1.0f));

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Overlaps
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	//Getters and Setters

	UFUNCTION(BlueprintCallable)
	int64 GetPoints();

	UFUNCTION(BlueprintCallable)
	int32 GetLifes();

	// mesh
	// 设置行索引
    void SetRowIndex(int32 Row);
    int32 GetRowIndex() const;

    // 静态方法：根据行设置不同的 Mesh
    static UStaticMesh* GetMeshForRow(int32 RowIndex);

protected:
	

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Bindings for inputs
	void OnMove(float value);

	void OnFire();

	void OnPause();

	UFUNCTION(BlueprintCallable)
	void DestroyPlayer();

	UFUNCTION(BlueprintCallable)
	void PostPlayerDestroyed();

	// high score test
	// void QuickDebugDeath();

private:
	int32 RowIndex = -1;


	// To set a frozen state (no moving and firing capabilities)
	bool bFrozen;

	//To pause the Game
	bool bPause;

	FTimerHandle timerHandle;

	UPROPERTY()
	class ASIGameModeBase* MyGameMode;

	//Points
	int64 playerPoints;

	
	UPROPERTY() //This is necessary to control the reference counter of the ABullet and avoid garbage collector action
	class ABullet* bulletTemplate;

	// Audio component
	UPROPERTY()
		class UAudioComponent* AudioComponent;

		// Bindings to delegates
		void InvaderDestroyed(int32 id);
		void SquadDissolved(int32 val);
		void SquadSuccessful();

	
	static constexpr const TCHAR* defaultStaticMeshPath = TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'");
};
