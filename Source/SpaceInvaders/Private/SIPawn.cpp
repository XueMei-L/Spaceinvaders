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

	// 添加这一行：直接监听键盘 K 键
    // PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &ASIGameModeBase::EndGame());
}

// test
// void ASIPawn::QuickDebugDeath()
// {
//     // 1. 强制给一点分（方便测试是否真的存进去了）
//     // this->playerPoints = 1234; 

//     if (MyGameMode) {
//         // 2. 保存分数到硬盘
//         MyGameMode->UpdateHighScore(this->playerPoints);

//         // 3. 从硬盘读取刚才存的分数（验证是否存进去了）
//         int32 SavedScore = MyGameMode->GetSavedHighScore();

//         // 4. 在屏幕左上角打印出来
//         // 参数说明：-1(不覆盖旧消息), 5.f(显示5秒), FColor::Cyan(青色), 后面是内容
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


void ASIPawn::PostPlayerDestroyed() {

    // End game - 当玩家生命值为 0 时
    if (this->playerLifes == 0) {
        
        // --- 新增保存逻辑开始 ---
        if (MyGameMode) {
            // 1. 调用你之前在 SIGameModeBase 中写的 UpdateHighScore 函数
            // 注意：你的变量名是 playerPoints
			UE_LOG(LogTemp, Warning, TEXT("Saving Score: %lld"), this->playerPoints); // 打印日志
            MyGameMode->UpdateHighScore(this->playerPoints); 
            
            // 2. 执行你原本的委托（如果有的话）
            MyGameMode->PlayerZeroLifes.ExecuteIfBound();
        }

        // 3. 跳转到 GameOver 关卡
        // 这里的 "GameOver" 必须和你 Content Browser 里的关卡名字完全一致
        UGameplayStatics::OpenLevel(GetWorld(), FName("GameOver"));
        // --- 新增保存逻辑结束 ---

        return;
    }

    // ... 下面是重生的代码，保持不变 ...
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