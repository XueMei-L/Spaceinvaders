// Fill out your copyright notice in the Description page of Project Settings.

#include "InvaderMovementComponent.h"
#include "SIGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UInvaderMovementComponent::UInvaderMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Defaults:
	state = InvaderMovementType::STOP;
	horizontalVelocity = 1000.0f;
	verticalVelocity = 1000.0f;
	descendingStep = 100.0f;
	freeJumpRadius = 300.0f;
	freeJumpVelocity = 1000.0f;
	deltaAlphaInterpolation = 1.0f / 30.0f;
	numberOfTargetPoints = 5;
	previousState = InvaderMovementType::STOP;

	// ...
}


// Called when the game starts
// Set pointer to GameMode to call delegates
void UInvaderMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get the GameMode
	UWorld* TheWorld;

	TheWorld = GetWorld();
	if (TheWorld) {
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		MyGameMode = Cast<ASIGameModeBase>(GameMode);
	}
}


#pragma optimize("", off)
void UInvaderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Parent = GetOwner(); 
    if (!Parent) return;

    float deltaHorizontal = horizontalVelocity * DeltaTime; 
    float deltaVertical = verticalVelocity * DeltaTime;

    float deltaX = 0.0f; 
    float deltaY = 0.0f;

    switch (state) {

    case InvaderMovementType::STOP:
        deltaX = 0.0f;
        deltaY = 0.0f;
        previousState = InvaderMovementType::STOP;
        break;

    case InvaderMovementType::RIGHT:
        deltaX = 0.0f;
        deltaY = deltaHorizontal;
        previousState = InvaderMovementType::RIGHT;
        break;

    case InvaderMovementType::LEFT: 
        deltaX = 0.0f;
        deltaY = -deltaHorizontal;
        previousState = InvaderMovementType::LEFT;
        break;

    case InvaderMovementType::DOWN:
        if (previousState != InvaderMovementType::DOWN)
            descendingProgress = 0.0f;
        if (descendingProgress > descendingStep) {
            deltaVertical = 0.0f;
            MyGameMode->SquadFinishesDown.ExecuteIfBound();
        }
        
        deltaX = -deltaVertical;
        deltaY = 0.0f;
        descendingProgress += deltaVertical;
        previousState = InvaderMovementType::DOWN;
        break;

    case InvaderMovementType::FREEJUMP:
        deltaX = 0.0f; 
        deltaY = 0.0f;
        
        if (previousState != InvaderMovementType::FREEJUMP) { 
            GenerateTargetPoints();
            currentTargetPoint = 0;
            bHasLockedTarget = false;

            if (numberOfTargetPoints > 0) {
                originTransform = Parent->GetActorTransform(); 
                alphaInterpolation = 0.0f;
            }
            previousState = InvaderMovementType::FREEJUMP;
        }

        // rotacion
        if (currentTargetPoint < numberOfTargetPoints) {
            FTransform newtransform = InterpolateWithTargetPoints(originTransform, alphaInterpolation); 
            Parent->SetActorTransform(newtransform);

            alphaInterpolation += deltaAlphaInterpolation;
            if (alphaInterpolation > 1.0f) { 
                ++currentTargetPoint;
                alphaInterpolation = 0.0f; 

                if (currentTargetPoint < numberOfTargetPoints) 
                    originTransform = this->targetPoints[currentTargetPoint - 1];
            }
        }
        else {
            // lock object
            if (!bHasLockedTarget) {
                APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
                if (PlayerPawn) {
                    FVector PlayerLocation = PlayerPawn->GetActorLocation();
                    FVector InvaderPos = Parent->GetActorLocation();
                    
                    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(InvaderPos, PlayerLocation);
                    
                    Parent->SetActorRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
                    
                    bHasLockedTarget = true;
                }
            }

            // forward to user
            FVector parentLocation = Parent->GetActorLocation();
            FVector forward = Parent->GetActorForwardVector();
            parentLocation += freeJumpVelocity * DeltaTime * forward;

            Parent->SetActorLocation(parentLocation);
        }
        break;
    }

    // movemente logic
    if (Parent && state != InvaderMovementType::FREEJUMP) {
        FVector parentLocation = Parent->GetActorLocation();
        parentLocation.X += deltaX;
        parentLocation.Y += deltaY;
        Parent->SetActorLocation(parentLocation);
    }
}
#pragma optimize("", on)


// This method produce the sequence of target transform for interpolation
#pragma optimize("", off)

void UInvaderMovementComponent::GenerateTargetPoints() {
	// para obtener quien es posedor - obtener la ubicacion del padre
	AActor* Parent = GetOwner();
	FTransform initialTransform;
	FVector initialLocation;
	FVector initialScale;

	FQuat initialQuaternion;
	FVector forward;
	if (!Parent) {
		numberOfTargetPoints = 0;
		return;
	}

	initialTransform = Parent->GetActorTransform();
	initialLocation = initialTransform.GetLocation();
	initialScale = initialTransform.GetScale3D();
	initialQuaternion = initialTransform.GetRotation();
	forward = Parent->GetActorForwardVector();

	// The first stage movement is a circle
	// Calculate center of the circle from actor location
	float radio = freeJumpRadius;
	FVector center = initialLocation;
	
	// centro de circuferencia
	center.X += radio;

	if (numberOfTargetPoints > 0) {
		float theta = 0.0f;
		float deltaTheta = 2 * PI / numberOfTargetPoints;

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("X is %g Y is %g"), initialLocation.X, initialLocation.Y));

		FVector newLocation = initialLocation;
		FRotator rotation;
		FTransform newTransform = initialTransform;
		for (int32 i = 0; i < numberOfTargetPoints; i++) {
			float pc = FMath::Cos(theta);
			float ps = FMath::Sin(theta);
			newLocation.X = center.X - radio * FMath::Cos(theta);
			newLocation.Y = center.Y + radio * FMath::Sin(theta);
			//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("X is %g Y is %g"), newLocation.X, newLocation.Y));
			newTransform.SetLocation(newLocation);
			// Change the rotation of the actor to follow the tangent of the circle
			if (i != (numberOfTargetPoints - 1)) {
				// FRotator requires angles in degrees!
				rotation = FRotator(0.0f, -(theta * 180.0f / PI) - 90, 0.0f);
				FQuat newQuaternion = rotation.Quaternion() * initialQuaternion;
				newTransform.SetRotation(newQuaternion);
			}
			else
				newTransform.SetRotation(initialQuaternion); //Last transformation 
			targetPoints.Add(newTransform);
			theta += deltaTheta;
		}

	}
}


#pragma optimize("", on)

FTransform UInvaderMovementComponent::InterpolateWithTargetPoints(FTransform origin, float fraction) {

	FVector originLocation = origin.GetLocation();
	FQuat originRotation = origin.GetRotation();
	FVector targetLocation;
	FQuat targetRotation;
	if (currentTargetPoint >= 0 && currentTargetPoint < numberOfTargetPoints) {
		targetLocation = targetPoints[currentTargetPoint].GetLocation();
		targetRotation = targetPoints[currentTargetPoint].GetRotation();
	}
	else
		return origin;

	FVector newLocation = UKismetMathLibrary::VLerp(originLocation, targetLocation, fraction);
	FQuat newRotation = FQuat::Slerp(originRotation, targetRotation, fraction);

	FTransform newTransform = origin;
	newTransform.SetLocation(newLocation);
	newTransform.SetRotation(newRotation);
	return newTransform;
}


