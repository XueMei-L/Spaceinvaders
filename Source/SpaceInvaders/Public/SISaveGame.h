#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SISaveGame.generated.h"

UCLASS()
class SPACEINVADERS_API USISaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // 使用 BlueprintReadWrite，这样以后如果你想在蓝图里微调也很方便
    UPROPERTY(BlueprintReadWrite, Category = "High Score")
    int32 HighScore;

    USISaveGame();
};