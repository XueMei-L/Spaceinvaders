// Fill out your copyright notice in the Description page of Project Settings.


#include "Invader.h"
#include "Bullet.h"
#include "InvaderMovementComponent.h"
#include "SIGameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"



AInvader::AInvader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default values

	bulletClass = ABullet::StaticClass();


	// Inmutable FNames for limits
	leftSideTag = FName(AInvader::leftSideTagString);
	rightSideTag = FName(AInvader::rightSideTagString);
	downSideTag = FName(AInvader::downSideTagString);

	// Create Components in actor

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMeshComponent");
	Movement = CreateDefaultSubobject<UInvaderMovementComponent>("InvaderMoveComponent");
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("Audio");
	AudioComponentJet = CreateDefaultSubobject<UAudioComponent>("AudioJet");
	RootComponent = Mesh; // We need a RootComponent to have a base transform

	SetInvaderMesh();

	// Component hierarchy
	AddOwnedComponent(Movement); // Because UInvaderMovementComponent is only an Actor Component and not a Scene Component can't Attach To.
	// Audio component

	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	AudioComponentJet->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	fireRate = 0.0001f;
	bFrozen = false;
}

// Called when the game starts or when spawned
void AInvader::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	// switch (InvaderType)
	// {
	// case 0:
	// 	SetInvaderMesh(nullptr, TEXT("StaticMesh'/Game/Meshes/Invader/brightInvader.brightInvader'"));
	// 	break;

	// case 1:
	// 	SetInvaderMesh(nullptr, TEXT("StaticMesh'/Game/Meshes/Invader/darkInvader.darkInvader'"));
	// 	break;

	// case 2:
	// 	SetInvaderMesh(nullptr, TEXT("StaticMesh'/Game/Meshes/Invader/greenInvader.greenInvader'"));
	// 	break;

	// default:
	// 	break;
	// }

	// Get size of invader
	FBoxSphereBounds meshBounds = Mesh->Bounds;
	boundOrigin = meshBounds.Origin;
	boundRadius = meshBounds.SphereRadius;

	// Generate a Bullet Template of the correct class
	if (bulletClass->IsChildOf<ABullet>())
		bulletTemplate = NewObject<ABullet>(this, bulletClass);
	else
		bulletTemplate = NewObject<ABullet>();

	bulletTemplate->bulletType = BulletType::INVADER;

	// Get InvaderMovementCompont
	InvaderMovementComponent = (UInvaderMovementComponent*)this->GetComponentByClass(UInvaderMovementComponent::StaticClass());

}

void AInvader::SetPositionInSquad(int32 index)
{
	this->positionInSquad = index;
}

int32 AInvader::GetPositionInSquad()
{
	return int32(this->positionInSquad);
}

// Called every frame
void AInvader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFrozen) { //Frozing the invader when is shooted down
		InvaderMovementComponent->state = InvaderMovementType::STOP;
		return;
	}
	this->timeFromLastShot += DeltaTime;

	// frecuencia de ataque
	float val = FMath::RandRange(0.0f, 0.5f); 
	if (val < (1.0 - FMath::Exp(-fireRate * this->timeFromLastShot)))
    Fire();

	//Jet sound
	if (AudioComponentJet != nullptr && AudioJet != nullptr) {
		if (InvaderMovementComponent != nullptr) {
			bool bFreeJump = InvaderMovementComponent->state == InvaderMovementType::FREEJUMP;
			if (bFreeJump && !AudioComponentJet->IsPlaying()) {
				AudioComponentJet->SetSound(AudioJet);
				AudioComponentJet->Play();
			}
		}
	}
}

void AInvader::Fire() {
	if (bFrozen)
		return;
	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = GetActorRotation();
	ABullet* spawnedBullet;
	if (this->bulletTemplate) {
		this->bulletTemplate->velocity = bulletVelocity;
		this->bulletTemplate->dir = GetActorForwardVector();
		FActorSpawnParameters spawnParameters;
		spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParameters.Template = this->bulletTemplate;
		if(bulletClass!= nullptr)
			spawnedBullet = (ABullet*)GetWorld()->SpawnActor<ABullet>(bulletClass,spawnLocation, spawnRotation, spawnParameters);
		else
			spawnedBullet = (ABullet*)GetWorld()->SpawnActor<ABullet>(spawnLocation, spawnRotation, spawnParameters);
		if (AudioComponent != nullptr && AudioShoot != nullptr) {
			AudioComponent->SetSound(AudioShoot);
			AudioComponent->Play();
		}

		this->timeFromLastShot = 0.0f;
	}
}

//Getters and setters

FVector AInvader::GetBoundOrigin() {
	return this->boundOrigin;

}

float AInvader::GetBoundRadius() {
	return this->boundRadius;
}

void AInvader::NotifyActorBeginOverlap(AActor* OtherActor) {
    // Debug
    //GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("%s entered me"), *(OtherActor->GetName())));
    FName actorTag;
    if (bFrozen) // If it is already a zombie invader nothing happens.
        return;

    UWorld* TheWorld = GetWorld();
    if (TheWorld != nullptr) {
        UInvaderMovementComponent* imc = (UInvaderMovementComponent*)this->GetComponentByClass(UInvaderMovementComponent::StaticClass());
        bool bFreeJump = imc->state == InvaderMovementType::FREEJUMP;
        AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
        ASIGameModeBase* MyGameMode = Cast<ASIGameModeBase>(GameMode);
        UClass* otherActorClass = OtherActor->GetClass();

        //First, bullet cases
        if (OtherActor->IsA(ABullet::StaticClass())) {
            ABullet* bullet = Cast<ABullet>(OtherActor);
            if (bullet->bulletType == BulletType::PLAYER) {
                //GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Invader %d hit"), this->positionInSquad));
                OtherActor->Destroy();
                // Modify 3: increment HP every round
				// Reduce HP each hit
                CurrentHP--;
                // If HP reaches zero, destroy invader
                if (CurrentHP <= 0) {
                    if (MyGameMode)
                        MyGameMode->InvaderDestroyed.Broadcast(this->positionInSquad);
                    InvaderDestroyed();
                }
                return;
            }
            else
                return; //It's an invader bullet, so it has to be ignored
        }

        // OVerlap with other Invader is ignored
        if (OtherActor->IsA(AInvader::StaticClass()))
            return;

        // Overlap with anything in freejump (except invaders and their own bullets) is a silent Destroy.
        if (bFreeJump) {
            MyGameMode->InvaderDestroyed.Broadcast(this->positionInSquad);
            Destroy();
            return;
        }

        // Squad collides with limits
        if (OtherActor->ActorHasTag(leftSideTag) && !bFreeJump)
            MyGameMode->SquadOnLeftSide.ExecuteIfBound();
        else if (OtherActor->ActorHasTag(rightSideTag) && !bFreeJump)
            MyGameMode->SquadOnRightSide.ExecuteIfBound();
        else if (OtherActor->ActorHasTag(downSideTag) && !bFreeJump) {
            MyGameMode->SquadSuccessful.ExecuteIfBound(); // Squad wins!

        }
    }
}


void AInvader::InvaderDestroyed() {
	UWorld* TheWorld;
	TheWorld = GetWorld();


	if (TheWorld) {
		bFrozen = true; // Invader can'tmove or fire while being destroyed

		UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
		// Hide Static Mesh Component
		if (LocalMeshComponent != nullptr) {
			LocalMeshComponent->SetVisibility(false);
		}
		if (PFXExplosion != nullptr) {
			UGameplayStatics::SpawnEmitterAtLocation(TheWorld, PFXExplosion, this->GetActorTransform(), true);
		}
		//Audio
		if (AudioComponent != nullptr && AudioExplosion != nullptr) {
			AudioComponent->SetSound(AudioExplosion);
			AudioComponent->Play();
		}
		// Wait:
		TheWorld->GetTimerManager().SetTimer(timerHandle, this, &AInvader::PostInvaderDestroyed, 2.0f, false);
	}
}
void AInvader::PostInvaderDestroyed() {
	Destroy();

}



void AInvader::SetInvaderMesh(UStaticMesh* newStaticMesh, const FString path, FVector scale) {
    if (!Mesh) return;

    UStaticMesh* MeshToUse = newStaticMesh;

    if (!MeshToUse && !path.IsEmpty()) {
        MeshToUse = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *path));
    }

    if (MeshToUse) {
        Mesh->SetStaticMesh(MeshToUse);
        Mesh->SetRelativeScale3D(scale);
        Mesh->UpdateBounds();
        
        FBoxSphereBounds meshBounds = Mesh->Bounds;
        boundOrigin = meshBounds.Origin;
        boundRadius = meshBounds.SphereRadius;
    }
}