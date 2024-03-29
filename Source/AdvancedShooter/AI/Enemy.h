// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <AdvancedShooter/BulletHitInterface.h>
#include <Engine/DataTable.h>
#include "Enemy.generated.h"

class UParticleSystem;
class USoundBase;
class UBehaviorTree;
class AEnemyController;
class USphereComponent;
class UBoxComponent;
class AShooterCharacter;

const FString ENEMYLEVELPATH = TEXT("DataTable'/Game/_Game/DataTable/DT_EnemyLevel.DT_EnemyLevel'"); // NOT FINISHED
const FString ENEMYPATH = TEXT("DataTable'/Game/_Game/DataTable/DT_Enemy.DT_Enemy'"); 

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	EET_Grux UMETA(DisplayName = "Grux"),
	EET_Khaimera UMETA(DisplayName = "Khaimera"),

	EET_MAX UMETA(DisplayName = "Default Max"),
};

UENUM(BlueprintType)
enum class EEnemyLevel : uint8
{
	EEL_Level1 UMETA(DisplayName = "Level 1"),
	EEL_Level2 UMETA(DisplayName = "Level 2"),
	EEL_Level3 UMETA(DisplayName = "Level 3"),
	EEL_Level4 UMETA(DisplayName = "Level 4"),
	EEL_Level5 UMETA(DisplayName = "Level 5"),
	EEL_Level6 UMETA(DisplayName = "Level 6"),
	EEL_Level7 UMETA(DisplayName = "Level 7"),
	EEL_Level8 UMETA(DisplayName = "Level 8"),
	EEL_Level9 UMETA(DisplayName = "Level 9"),
	EEL_Level10 UMETA(DisplayName = "Level 10"),
	EEL_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT(BlueprintType)
struct FEnemyLevelDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float level;
};

USTRUCT(BlueprintType)
struct FEnemyDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float stunChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float attackWaitTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString headBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* enemyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* impactParicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* impactSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* meleeImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> animBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* hitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* attackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* deathMontage;
};

UCLASS()
class ADVANCEDSHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult hitResult, AActor* shooter, AController* instigator) override;

	virtual float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;

	FString GetHeadBoneName() const { return headBoneName; }
	UBehaviorTree* GetBehaviorTree() const { return behaviorTree; }
	UAnimInstance* GetAnimInstance() const { return GetMesh()->GetAnimInstance(); }
	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDamageNumber(int32 damage, FVector hitLocation, bool bIsHeadShot);

	UFUNCTION(BlueprintCallable)
	void SetIsStunned(bool stunned);

	void SetTarget(AActor* target);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& transform) override;

	UDataTable* GetDataTable(FString path);
	UDataTable* GetEnemyDataTable();
	void SetEnemyData();

	UDataTable* GetEnemyLevelDataTable();
	void SetEnemyLevelData();
	
	// Fuction to be used in blueprint
	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();

	// C++ version of above
	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();

	void PlayHitMontage(FName section, float playRate = 1.f);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName section, float playRate = 1.f);

	void ResetHitReactTimer();

	UFUNCTION(BlueprintCallable)
	void StoreDamageNumber(UUserWidget* damageNumber, FVector location);

	void UpdateDamageNumbers();

	UFUNCTION()
	void DestroyDamageNumber(UUserWidget* damageNumber);

	// Called when somthing overlaps with the agro sphere
	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex);

	UFUNCTION()
	void OnLeftWeaponBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);
	
	UFUNCTION()
	void OnRightWeaponBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	void SpawnBloodParticle(AShooterCharacter* character, FName socketName);

	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	void DoDamage(AShooterCharacter* character);
	void StunCharacter(AShooterCharacter* character);

	void ResetCanAttack();
private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	EEnemyType enemyType = EEnemyType::EET_Grux;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	EEnemyLevel enemyLevel = EEnemyLevel::EEL_Level1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Particles", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* impactParticles;

	// Sound of enemy getting hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sounds", meta = (AllowPrivateAccess = "true"))
	USoundBase* impactSound;

	// Sound of Enemy hitting character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sounds", meta = (AllowPrivateAccess = "true"))
	USoundBase* meleeImpactSound;

	// Current enemy health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	float health = 100.f;

	// Max enemy health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	float maxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	float level = 1;

	// Name of the head bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	FString headBoneName;

	// Time to display health bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	float healthBarDisplayTime = 4.f;

	FTimerHandle healthBarTimer;

	FTimerHandle hitReactTimer;

	// Montage for hit and death anims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* hitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* attackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* deathMontage;

	bool bCanHitReact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit", meta = (AllowPrivateAccess = "true"))
	float hitReactTimeMin = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit", meta = (AllowPrivateAccess = "true"))
	float hitReactTimeMax = 3.f;

	// Map of damage number widgets and their stored location
	UPROPERTY(VisibleAnywhere, Category = "Combat|Hit", meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> damageNumbers;

	// Time before removing a damage number
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Hit", meta = (AllowPrivateAccess = "true"))
	float damageNumberDestroyTime = 1.5f;

	// BEHAVIOR TREE VARS
	UPROPERTY(EditAnywhere, Category = "BehaviorTree", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* behaviorTree;

	// Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "BehaviorTree|Patrol", meta = (AllowPrivateAccess = "true", MakeEditWidget = "True"))
	FVector patrolPoint;

	// Second Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "BehaviorTree|Patrol", meta = (AllowPrivateAccess = "true", MakeEditWidget = "True"))
	FVector patrolPoint2;

	AEnemyController* enemyController;

	// Overlap sphere for when the enemy is hostile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USphereComponent* agroSphere;

	// Overlap sphere for when the enemy is in attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USphereComponent* combatRangeSphere;

	// True when enemy is stunned
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsStunned = false;

	// Chance to stun an enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float stunChance = 0.5f;

	// True when in attack range
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsInAttackRange = false;

	// Montage Sections
	FName attackLFast = TEXT("AttackLFast");
	FName attackRFast = TEXT("AttackRFast");
	FName attackL = TEXT("AttackL");
	FName attackR = TEXT("AttackR");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName leftWeaponSocket = TEXT("FX_Trail_L_01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName rightWeaponSocket = TEXT("FX_Trail_R_01");

	// Collision volume for left weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* leftWeaponCollision;

	// Collision volume for right weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* rightWeaponCollision;

	// Base damage for enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float baseDamage = 20.f;

	// True when enemy can attack
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanAttack = true;

	FTimerHandle attackWaitTimer;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float attackWaitTime = 1.f;

	bool bIsDying = false;

	
};
