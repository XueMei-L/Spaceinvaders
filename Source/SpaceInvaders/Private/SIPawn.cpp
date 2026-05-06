// Fill out your copyright notice in the Description page of Project Settings.


#include "SIPawn.h"
#include "SIGameModeBase.h"
#include "Bullet.h"
#include "Invader.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ASIPawn::ASIPawn()
	: pointsPerInvader { 100 },
	pointsPerSquad{ 1000 },
	velocity{ 1000 },
	bulletVelocity{ 3000 },
	playerLifes{ 3 },

	AudioShoot{}, //nullptr if(AudioShoot)
	AudioExplosion{},
	bFrozen{ false },
	bPause{ false },
	MyGameMode{},
	playerPoints{ 0 }
{
	
	PrimaryActorTick.bCanEverTick = true;

	SetStaticMesh(); // Default mesh (SetStaticMesh with no arguments)

	// Audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("Audio");
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

// Set a static mesh.
void ASIPawn::SetStaticMesh(UStaticMesh* staticMesh, FString path, FVector scale) {
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	const TCHAR* tpath;
	tpath = ASIPawn::defaultStaticMeshPath; // default route
	if (!Mesh) // No Mesh component
		return;

	if (!staticMesh) {
		if (!path.IsEmpty())
			tpath = *path;
		auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(tpath);
		staticMesh = MeshAsset.Object;
	}
	if (staticMesh) {
		Mesh->SetStaticMesh(staticMesh);

		Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}


// Called when the game starts or when spawned
void ASIPawn::BeginPlay()
{
	Super::BeginPlay();

	// Generate a Bullet Template of the correct class
	if (bulletClass->IsChildOf<ABullet>())
		bulletTemplate = NewObject<ABullet>(this, bulletClass);
	else
		bulletTemplate = NewObject<ABullet>(this);

	bulletTemplate->bulletType = BulletType::PLAYER;

	UWorld* TheWorld = GetWorld();
	if (TheWorld != nullptr) {
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		MyGameMode = Cast<ASIGameModeBase>(GameMode);
		if (MyGameMode) {
			MyGameMode->InvaderDestroyed.AddUObject(this, &ASIPawn::InvaderDestroyed);
			MyGameMode->SquadSuccessful.BindUObject(this, &ASIPawn::SquadSuccessful);
			MyGameMode->NewSquad.AddUObject(this, &ASIPawn::SquadDissolved);
		}
	}
}

// Called every frame
void ASIPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASIPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis(TEXT("SIRight"), this, &ASIPawn::OnMove);
	PlayerInputComponent->BindAction(TEXT("SIFire"), IE_Pressed, this, &ASIPawn::OnFire);
	PlayerInputComponent->BindAction(TEXT("SIPause"), IE_Pressed, this, &ASIPawn::OnPause);

}

// test
// void ASIPawn::QuickDebugDeath()
// {
//     // this->playerPoints = 1234; 

//     if (MyGameMode) {
//         MyGameMode->UpdateHighScore(this->playerPoints);

//         int32 SavedScore = MyGameMode->GetSavedHighScore();

//         if (GEngine)
//         {
//             GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, 
//                 FString::Printf(TEXT("Current Score: %lld | Saved High Score: %d"), this->playerPoints, SavedScore));
//         }
//     }
// 	this->playerLifes = 0;
// 	PostPlayerDestroyed();
// }


void ASIPawn::OnMove(float value) {

	if (bFrozen)
		return;

	float deltaTime = GetWorld()->GetDeltaSeconds();

	float delta = velocity * value * deltaTime;
	FVector dir = FVector(0.0f, 1.0f, 0.0f);

	AddMovementInput(dir, delta);
}

void ASIPawn::OnFire() {
	if (bFrozen)
		return;
	
	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = GetActorRotation();
	ABullet* spawnedBullet;
	this->bulletTemplate->velocity = bulletVelocity;
	this->bulletTemplate->dir = GetActorForwardVector();
	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParameters.Template = this->bulletTemplate;
	if(bulletClass!= nullptr)
		spawnedBullet = Cast<ABullet>(GetWorld()->SpawnActor<ABullet>(bulletClass, spawnLocation, spawnRotation, spawnParameters));
	else
		spawnedBullet = Cast<ABullet>(GetWorld()->SpawnActor<ABullet>(spawnLocation, spawnRotation, spawnParameters));
	if (AudioComponent != nullptr && AudioShoot != nullptr) {
		AudioComponent->SetSound(AudioShoot);
	}
	AudioComponent->Play();

}

void ASIPawn::OnPause() {
	bPause = !bPause;
}


int64 ASIPawn::GetPoints() {
	return this->playerPoints;

}

int32 ASIPawn::GetLifes() {
	return this->playerLifes;

}

void ASIPawn::NotifyActorBeginOverlap(AActor* OtherActor) {

	if (!bFrozen) {
		// Collision with an enemy
		if (OtherActor->IsA(ABullet::StaticClass())) { // ABullet::StaticClass() obtengo un puntero a la UCLASS en memoria de Abullet
			ABullet* bullet = Cast<ABullet>(OtherActor);
			if (bullet->bulletType == BulletType::INVADER) {
				OtherActor->Destroy();
				DestroyPlayer();
			}
		}
		// Collision with an invader
		if (OtherActor->IsA(AInvader::StaticClass())) {
			OtherActor->Destroy();
			DestroyPlayer();

		}
	}

}

void ASIPawn::DestroyPlayer() {
	UWorld* TheWorld;
	TheWorld = GetWorld();

	if (TheWorld) {
		bFrozen = true; // Pawn can'tmove or fire while being destroyed
		--this->playerLifes;
		UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
		// Hide Static Mesh Component
		if (LocalMeshComponent != nullptr) {
			LocalMeshComponent->SetVisibility(false);
		}
		//Audio
		if (AudioComponent != nullptr && AudioExplosion != nullptr) {
			AudioComponent->SetSound(AudioExplosion);
			AudioComponent->Play();
		}
		// Wait:
		TheWorld->GetTimerManager().SetTimer(timerHandle, this, &ASIPawn::PostPlayerDestroyed, 3.0f, false);
	}
}

// change with save high score data
void ASIPawn::PostPlayerDestroyed() {

    if (this->playerLifes == 0) {
        
        if (MyGameMode) {
			// save data
			UE_LOG(LogTemp, Warning, TEXT("Saving Score: %lld"), this->playerPoints);
            MyGameMode->UpdateHighScore(this->playerPoints); 
            MyGameMode->PlayerZeroLifes.ExecuteIfBound();
        }

        UGameplayStatics::OpenLevel(GetWorld(), FName("GameOver"));

        return;
    }

	// reborn
    UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
    if (LocalMeshComponent != nullptr) {
        LocalMeshComponent->SetVisibility(true);
    }
    bFrozen = false;
}

// Delegate responses:
void ASIPawn::InvaderDestroyed(int32 id) {
	this->playerPoints += this->pointsPerInvader;
}


void ASIPawn::SquadSuccessful() {
	DestroyPlayer();
	if (MyGameMode)
		MyGameMode->NewSquad.Broadcast(this->playerLifes);
}

void ASIPawn::SquadDissolved(int32 val) {
	this->playerPoints += this->pointsPerSquad;
}