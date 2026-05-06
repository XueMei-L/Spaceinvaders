// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "InvaderMovementComponent.generated.h"

UENUM(BlueprintType)
enum class InvaderMovementType : uint8 {
	STOP = 0 UMETA(DisplayName = "Stopped"),
	RIGHT = 1 UMETA(DisplayName = "Right"),
	LEFT = 2 UMETA(DisplayName = "Left"),
	DOWN = 3 UMETA(DisplayName = "Down"),
	FREEJUMP = 4 UMETA(DisplayName = "Free Jump")
};
/**
 * 
 */
UCLASS()
class SPACEINVADERS_API UInvaderMovementComponent : public UMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInvaderMovementComponent();

	UPROPERTY(EditAnyWhere, BlueprintReadWrite,Category="Invader Movement")
	float horizontalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Invader Movement")
	float verticalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Invader Movement")
	InvaderMovementType state;

	// Down movement parameters:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	float descendingStep; // Length of the descending step

	// Free jump parameters:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	TArray<FTransform> targetPoints; // The movement will be composed by interpolation using a sequence of reference poses stored in this array.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	int32 numberOfTargetPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	float freeJumpRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	float freeJumpVelocity; // Velocity in the first stage of the free jump

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invader Movement")
	float deltaAlphaInterpolation; //1/(Number of updates between two target points)

protected:
	// for invader movement attack
	bool bHasLockedTarget;

	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void GenerateTargetPoints();

	UFUNCTION(BlueprintCallable)
	FTransform InterpolateWithTargetPoints(FTransform transform, float covered);


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	InvaderMovementType previousState; // Store state in previous frame (to know when a state is beginning)

	// Down movement state variables:
	float descendingProgress = 0.0f; // Store progress in the Down state

	// Free jump movement state variables:
	FTransform originTransform;
	float alphaInterpolation;
	int32 currentTargetPoint = 0; // It stores the index of the first reference pose (the other is currentTargetPoint+1)
	float finalAngle; // Orientation of the invader to start the final attack

	class ASIGameModeBase* MyGameMode; // to call delegates

};
