// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"


UENUM(BlueprintType)
enum class BulletType : uint8 {
	PLAYER = 0 UMETA(DisplayName = "Player Type"),
	INVADER = 1 UMETA(DisplayName = "Invader Type")
};

UCLASS()
class SPACEINVADERS_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	BulletType bulletType = BulletType::PLAYER;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector dir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float velocity;


	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UFUNCTION(BlueprintCallable)
	void SetBulletMesh(class UStaticMesh* staticMesh = nullptr, FString path = TEXT(""), FVector scale = FVector(1.0f, 1.0f, 1.0f));

private:

	FName autoDestroyTags[4] = { TEXT("BottomLimit"),TEXT("RightLimit"),TEXT("LeftLimit"),TEXT("TopLimit") };
	static constexpr const TCHAR* defaultStaticMeshPath = TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'");
};
