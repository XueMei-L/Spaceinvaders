// Fill out your copyright notice in the Description page of Project Settings.


#include "InvaderSquad.h"
#include "InvaderMovementComponent.h"
#include "Invader.h"
#include "SIGameModeBase.h"


#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"



AInvaderSquad::AInvaderSquad()
	: horizontalVelocity{ AInvaderSquad::defaultHorizontalVelocity }
	, verticalVelocity{ AInvaderSquad::defaultVerticalVelocity }
	, state{ InvaderMovementType::STOP }
	, previousState{ InvaderMovementType::STOP }
	, freeJumpRate{ 0.0001 }
	, nRows{ AInvaderSquad::defaultNRows }
	, nCols{ AInvaderSquad::defaultNCols }
	, extraSeparation(AInvaderSquad::defaultExtraSeparation)
	, numberOfMembers{ nRows * nCols }
	, timeFromLastFreeJump{ 0.0 }
{
	// Create Components in actor

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root; // We need a RootComponent to have a base transform
	Initialize();

}


void AInvaderSquad::SetRows(int32 nrows) {
	this->nRows = nrows;
	this->numberOfMembers = this->nRows * this->nCols;
}

void  AInvaderSquad::SetCols(int32 ncols) {
	this->nCols = ncols;
	this->numberOfMembers = this->nRows * this->nCols;
}

int32 AInvaderSquad::GetRows() {
	return this->nRows;
}

int32 AInvaderSquad::GetCols() {
	return this->nCols;
}

int32 AInvaderSquad::GetNumberOfMembers() {
	return this->numberOfMembers;
}
void AInvaderSquad::Initialize() {
	PrimaryActorTick.bCanEverTick = true;


}
// codigo original
// void AInvaderSquad::BeginPlay()
// {
// 	Super::BeginPlay();

// 	UWorld* TheWorld = GetWorld();

// 	// Bind to delegates
// 	if (TheWorld != nullptr) {
// 		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
// 		MyGameMode = Cast<ASIGameModeBase>(GameMode);
// 		if (MyGameMode != nullptr) {
// 			MyGameMode->SquadOnRightSide.BindUObject(this, &AInvaderSquad::SquadOnRightSide);
// 			MyGameMode->SquadOnLeftSide.BindUObject(this, &AInvaderSquad::SquadOnLeftSide);
// 			MyGameMode->SquadFinishesDown.BindUObject(this, &AInvaderSquad::SquadFinishesDown);
// 			MyGameMode->InvaderDestroyed.AddUObject(this, &AInvaderSquad::RemoveInvader);
// 		}
// 	}

// 	// Set Invader Template with Default Value for invaderClass
// 	if (invaderClass->IsChildOf<AInvader>()) {
// 		invaderTemplate = NewObject<AInvader>(this, invaderClass);
// 	}
// 	else
// 		invaderTemplate = NewObject<AInvader>();

// 	//Spawn Invaders
// 	FVector actorLocation = GetActorLocation();
// 	FVector spawnLocation = actorLocation;
// 	FRotator spawnRotation = FRotator(0.0f, 180.0f, 0.0f); // Invader Forward is oposite to Player Forward (Yaw rotation)
// 	FActorSpawnParameters spawnParameters;
// 	int32 count = 0;
// 	AInvader* spawnedInvader;
// 	float radiusX = 0.0f;
// 	float radiusY = 0.0f;
// 	for (int i = 0; i < this->nCols; i++)
// 	{

// 		for (int j = 0; j < this->nRows; j++)
// 		{
// 			spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
// 			spawnParameters.Template = invaderTemplate;
			
// 			if(invaderClass!=nullptr)
// 				spawnedInvader = GetWorld()->SpawnActor<AInvader>(invaderClass, spawnLocation, spawnRotation, spawnParameters);
// 			else
// 				spawnedInvader = GetWorld()->SpawnActor<AInvader>(spawnLocation, spawnRotation, spawnParameters);

// 			if (spawnedInvader == nullptr) {

// 				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("%d %d null invader"), i, j));
// 				break;
// 			}
			
// 			// GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("%d %d valid invader"), i, j));
			
// 			spawnedInvader->SetPositionInSquad(count);
// 			++count;
// 			SquadMembers.Add(spawnedInvader);
// 			float r = spawnedInvader->GetBoundRadius();
// 			if (r > radiusX)
// 				radiusX = r;
// 			if (r > radiusY)
// 				radiusY = r;
// 			spawnLocation.X += radiusX * 2 + this->extraSeparation;
// 		}
// 		spawnLocation.X = actorLocation.X;

// 		spawnLocation.Y += radiusY * 2 + this->extraSeparation;
// 	}

// 	this->numberOfMembers = count;

// 	this->state = InvaderMovementType::RIGHT;

// }


void AInvaderSquad::BeginPlay()
{
    Super::BeginPlay();

    UWorld* TheWorld = GetWorld();
    
    // Bind GameMode Delegates
    if (TheWorld != nullptr) {
        ASIGameModeBase* GM = Cast<ASIGameModeBase>(UGameplayStatics::GetGameMode(TheWorld));
        if (GM) {
            MyGameMode = GM;
            MyGameMode->SquadOnRightSide.BindUObject(this, &AInvaderSquad::SquadOnRightSide);
            MyGameMode->SquadOnLeftSide.BindUObject(this, &AInvaderSquad::SquadOnLeftSide);
            MyGameMode->SquadFinishesDown.BindUObject(this, &AInvaderSquad::SquadFinishesDown);
            MyGameMode->InvaderDestroyed.AddUObject(this, &AInvaderSquad::RemoveInvader);
        }
    }

    // Prepare Spawning Parameters
    FVector actorLocation = GetActorLocation();
    FVector spawnLocation = actorLocation;
    FRotator spawnRotation = FRotator(0.0f, 180.0f, 0.0f);
    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 count = 0;
    float currentMaxRadius = 0.0f;

    // Nested Loop to Spawn the Squad
    for (int i = 0; i < this->nCols; i++) // X-axis direction (Columns)
    {
        // Reset Y-coordinate to the starting point for every new column
        spawnLocation.Y = actorLocation.Y; 

        for (int j = 0; j < this->nRows; j++) // Y-axis direction (Rows)
        {
            // Resolve the class to spawn and cast to UClass* to avoid compiler errors
            UClass* ClassToSpawn = invaderClass ? *invaderClass : AInvader::StaticClass();
            AInvader* spawnedInvader = GetWorld()->SpawnActor<AInvader>(ClassToSpawn, spawnLocation, spawnRotation, spawnParams);

            if (spawnedInvader != nullptr && spawnedInvader->Mesh) 
            {
                // --- Physics Masking: Disable everything before changing the mesh ---
                // This prevents instant collision triggers caused by mesh size changes
                spawnedInvader->Mesh->SetGenerateOverlapEvents(false);
                spawnedInvader->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

                // --- Dynamic Appearance: Switch mesh based on row index (j) ---
                FString MeshPath;
                if (j == 0) MeshPath = TEXT("/Game/Meshes/Invader/greenInvader");
                else if (j == 1) MeshPath = TEXT("/Game/Meshes/Invader/brightInvader");
                else MeshPath = TEXT("/Game/Meshes/Invader/darkInvader");

                // --- Apply Scale: Applying the tested scale (0.07f) ---
                spawnedInvader->SetInvaderMesh(nullptr, MeshPath, FVector(0.07f, 0.07f, 0.07f));

                // Register the invader to the squad
                spawnedInvader->SetPositionInSquad(count);
                SquadMembers.Add(spawnedInvader);
                count++;

                // Update spacing reference based on the mesh radius
                currentMaxRadius = FMath::Max(currentMaxRadius, spawnedInvader->GetBoundRadius());

                // Increment Y position for the next row
                spawnLocation.Y += (currentMaxRadius * 2) + this->extraSeparation;
            }
        }
        // Increment X position for the next column
        spawnLocation.X += (currentMaxRadius * 2) + this->extraSeparation;
    }

    // Re-enable Physics and Overlaps
    // After all invaders are positioned and meshes are set, wake up their collision components
    for (AInvader* inv : SquadMembers) 
    {
        if (inv && inv->Mesh) 
        {
            // Set collision profile to OverlapAllDynamic (common for Space Invaders logic)
            inv->Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
            inv->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            inv->Mesh->SetGenerateOverlapEvents(true);
            
            // Force update the collision bounds to match the new visual mesh
            inv->Mesh->UpdateBounds(); 
        }
    }

    this->numberOfMembers = count;
    this->state = InvaderMovementType::RIGHT;
}

void AInvaderSquad::Destroyed() {
	Super::Destroyed();
	for (AInvader* invader : SquadMembers) {
		if (invader != nullptr)
			invader->Destroy();
	}
}
// Called every frame
void AInvaderSquad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSquadState(DeltaTime);

}

void AInvaderSquad::UpdateSquadState(float delta) {

	TArray<AInvader*> survivors;

	for (auto invader : SquadMembers) {
		//------------------------------------
		if (invader) { // very important, first nullptr is checked!.


			// First, we get de movement component
			UInvaderMovementComponent* imc = (UInvaderMovementComponent*)invader->GetComponentByClass(UInvaderMovementComponent::StaticClass());

			// Now, its state is updated
			if (imc) {
				if (imc->state != InvaderMovementType::FREEJUMP)
					survivors.Emplace(invader);
				imc->horizontalVelocity = horizontalVelocity;
				imc->verticalVelocity = verticalVelocity;
				//imc->isXHorizontal = isXHorizontal;
				if (imc->state != InvaderMovementType::FREEJUMP) // The state of the squad is copied to the invader state (except for those in FREEJUMP)
					imc->state = state;
			}
		}

		//------------------------------------
	}
	this->timeFromLastFreeJump += delta;
	float val = FMath::RandRange(0.0f, 1.0f);
	int32 countSurvivors = survivors.Num();
	if (countSurvivors > 0 && val < (1.0 - FMath::Exp(-freeJumpRate * this->timeFromLastFreeJump))) {
		int32 ind = FMath::RandRange(0, countSurvivors - 1); // Randomly select one of the living invaders
		UInvaderMovementComponent* imc = (UInvaderMovementComponent*)survivors[ind]->GetComponentByClass(UInvaderMovementComponent::StaticClass());
		if (imc) {
			//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("%s on FreeJump"), *(imc->GetName())));
			survivors[ind]->fireRate *= 100;
			imc->state = InvaderMovementType::FREEJUMP;
		}
	}
}

// Handling events

// La escuadra llega al lado derecho

void AInvaderSquad::SquadOnRightSide() {
	previousState = InvaderMovementType::RIGHT;
	state = InvaderMovementType::DOWN;
}

// La escuadra llega al lado izquierdo

void AInvaderSquad::SquadOnLeftSide() {
	previousState = InvaderMovementType::LEFT;
	state = InvaderMovementType::DOWN;
}

// Cada vez que un invasor completa el movimiento de descenso

void AInvaderSquad::SquadFinishesDown() {
	static int32 countActions = 0;
	++countActions;
	if (countActions >= numberOfMembers) {

		countActions = 0;
		switch (previousState) {
		case InvaderMovementType::RIGHT:
			state = InvaderMovementType::LEFT;
			break;
		case InvaderMovementType::LEFT:
			state = InvaderMovementType::RIGHT;
			break;
		default:
			state = InvaderMovementType::STOP;

		}
	}
}


void AInvaderSquad::RemoveInvader(int32 ind) {
	SquadMembers[ind] = nullptr;
	--this->numberOfMembers;
	if (this->numberOfMembers == 0) {
		if (MyGameMode != nullptr) {
			MyGameMode->NewSquad.Broadcast(1); // parameter larger than 0 to avoid finishing game!

		}
	}
}
