#include "Enemy.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include <Sound/SoundBase.h>
#include <Components/WidgetComponent.h>
#include <DrawDebugHelpers.h>
#include <Particles/ParticleSystemComponent.h>
#include <AdvancedShooter/AI/EnemyController.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <Components/SphereComponent.h>
#include <AdvancedShooter/ShooterCharacter.h>
#include <Components/CapsuleComponent.h>
#include <Components/BoxComponent.h>
#include <Engine/SkeletalMeshSocket.h>

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	agroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Agro Sphere"));
	agroSphere->SetupAttachment(GetRootComponent());

	combatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Range Sphere"));
	combatRangeSphere->SetupAttachment(GetRootComponent());

	leftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Collision"));
	leftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	rightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Collision"));
	rightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	agroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	combatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	combatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
	leftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponBeginOverlap);
	rightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponBeginOverlap);


	leftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	leftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	leftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	leftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	rightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	rightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	rightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	rightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	health = maxHealth;

	// Get the ai controller
	enemyController = Cast<AEnemyController>(GetController());

	enemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);

	// Convert local patrol point to world patrol point
	const FVector worldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), patrolPoint);
	const FVector worldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), patrolPoint2);
	DrawDebugSphere(GetWorld(), worldPatrolPoint, 25.f, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), worldPatrolPoint2, 25.f, 12, FColor::Red, true);
	
	if (!enemyController) return;
	enemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), worldPatrolPoint);
	enemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), worldPatrolPoint2);
	enemyController->RunBehaviorTree(behaviorTree);
}

void AEnemy::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
	SetEnemyData();
	SetEnemyLevelData();
}
// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDying) return;
	UpdateDamageNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
UDataTable* AEnemy::GetDataTable(FString path)
{
	UDataTable* tableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *path));

	if (!tableObject) return NULL;

	return tableObject;
}

void AEnemy::SetEnemyData()
{
	FEnemyDataTable* enemyDataRow = NULL;

	switch (enemyType)
	{
		case EEnemyType::EET_Grux:
			enemyDataRow = GetDataTable(ENEMYPATH)->FindRow<FEnemyDataTable>(FName("Grux"), TEXT(""));
			break;

		case EEnemyType::EET_Khaimera:
			enemyDataRow = GetDataTable(ENEMYPATH)->FindRow<FEnemyDataTable>(FName("Khaimera"), TEXT(""));
			break;

		default:
			break;
	}

	if (!enemyDataRow) return;
	stunChance = enemyDataRow->stunChance;
	attackWaitTime = enemyDataRow->attackWaitTime;

	headBoneName = enemyDataRow->headBoneName;

	GetMesh()->SetSkeletalMesh(enemyDataRow->enemyMesh);
	GetMesh()->SetAnimInstanceClass(enemyDataRow->animBP);

	impactParticles = enemyDataRow->impactParicles;

	impactSound = enemyDataRow->impactSound;
	meleeImpactSound = enemyDataRow->meleeImpactSound;

	attackMontage = enemyDataRow->attackMontage;
	hitMontage = enemyDataRow->hitMontage;
	deathMontage = enemyDataRow->deathMontage;
}

void AEnemy::SetEnemyLevelData()
{
	FEnemyLevelDataTable* enemyLevelDataRow = NULL;

	switch (enemyLevel)
	{
	case EEnemyLevel::EEL_Level1:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level1"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level2:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level2"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level3:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level3"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level4:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level4"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level5:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level5"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level6:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level6"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level7:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level7"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level8:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level8"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level9:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level9"), TEXT(""));
		break;
	case EEnemyLevel::EEL_Level10:
		enemyLevelDataRow = GetDataTable(ENEMYLEVELPATH)->FindRow<FEnemyLevelDataTable>(FName("Level10"), TEXT(""));
		break;
	default:
		break;
	}

	if (!enemyLevelDataRow) return;

	maxHealth = enemyLevelDataRow->health;
	baseDamage = enemyLevelDataRow->damage;
	level = enemyLevelDataRow->level;

	UE_LOG(LogTemp, Warning, TEXT("Lvl %f"), level);
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(healthBarTimer);

	GetWorldTimerManager().SetTimer(healthBarTimer, this, &AEnemy::HideHealthBar, healthBarDisplayTime);
}

void AEnemy::Die()
{
	HideHealthBar();

	if (bIsDying) return;
	bIsDying = true;

	if (!deathMontage) return;
	GetAnimInstance()->Montage_Play(deathMontage);

	if (!enemyController) return;
	enemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Dead"), true);

	enemyController->StopMovement();
}

void AEnemy::PlayHitMontage(FName section, float playRate)
{
	if (!bCanHitReact) return;

	if (!GetAnimInstance() && !hitMontage) return;

	GetAnimInstance()->Montage_Play(hitMontage, playRate);
	GetAnimInstance()->Montage_JumpToSection(section, hitMontage);

	bCanHitReact = false;

	const float hitReactTime = FMath::FRandRange(hitReactTimeMin, hitReactTimeMax);
	GetWorldTimerManager().SetTimer(hitReactTimer, this, &AEnemy::ResetHitReactTimer, hitReactTime);
}

void AEnemy::PlayAttackMontage(FName section, float playRate)
{
	if (!GetAnimInstance() && attackMontage) return;

	GetAnimInstance()->Montage_Play(attackMontage, playRate);
	GetAnimInstance()->Montage_JumpToSection(section, attackMontage);

	bCanAttack = false;

	GetWorldTimerManager().SetTimer(attackWaitTimer, this, &AEnemy::ResetCanAttack, attackWaitTime);

	if (!enemyController) return;
	enemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
}

FName AEnemy::GetAttackSectionName()
{
	FName sectionName;
	const int32 section = FMath::RandRange(1, 4);

	switch (section)
	{
		case 1:
			sectionName = attackLFast;
			break;
		case 2:
			sectionName = attackRFast;
			break;
		case 3:
			sectionName = attackL;
			break;
		case 4:
			sectionName = attackR;
			break;

		default:
			break;
	}

	return sectionName;
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreDamageNumber(UUserWidget* damageNumber, FVector location)
{
	damageNumbers.Add(damageNumber, location);

	FTimerHandle damageNumberTimer;

	// Create a local timer delegate to bind to our ufunction
	FTimerDelegate damageNumberDelegate;

	damageNumberDelegate.BindUFunction(this, FName("DestroyDamageNumber"), damageNumber);

	GetWorldTimerManager().SetTimer(damageNumberTimer, damageNumberDelegate, damageNumberDestroyTime, false);
}

void AEnemy::UpdateDamageNumbers()
{
	for (TPair<UUserWidget*, FVector> damagePair : damageNumbers)
	{
		UUserWidget* damageNumber = damagePair.Key;
		const FVector location = damagePair.Value;

		// Out param of project screen to world
		FVector2D screenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), location, screenPosition);
		damageNumber->SetPositionInViewport(screenPosition);
	}
}

void AEnemy::DestroyDamageNumber(UUserWidget* damageNumber)
{
	damageNumbers.Remove(damageNumber);
	damageNumber->RemoveFromParent();
}

void AEnemy::BulletHit_Implementation(FHitResult hitResult, AActor* shooter, AController* instigator)
{
	if (impactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	}
	
	if (impactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticles, hitResult.ImpactPoint, FRotator::ZeroRotator, true);
	}
}

float AEnemy::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
{
	SetTarget(damageCauser);

	if (health - damageAmount <= 0)
	{
		health = 0.f;
		Die();
	}

	else
	{
		health -= damageAmount;
	}

	if (bIsDying) return 0;
	
	ShowHealthBar();

	// Determine whether bullet hit stuns
	const float stunned = FMath::FRandRange(0.f, 1.f);
	if (stunned <= stunChance)
	{
		// Stun the enemy
		PlayHitMontage(FName("HitReactFront"));
		SetIsStunned(true);
	}
	return damageAmount;
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (!otherActor) return;
	
	SetTarget(otherActor);
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (!otherActor) return;

	AShooterCharacter* character = Cast<AShooterCharacter>(otherActor);

	if (character)
	{
		bIsInAttackRange = true;

		if (!enemyController) return;
		enemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
	}
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	if (!otherActor) return;

	AShooterCharacter* character = Cast<AShooterCharacter>(otherActor);

	if (character)
	{
		bIsInAttackRange = false;

		if (!enemyController) return;
		enemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
	}
}

void AEnemy::FinishDeath()
{
	Destroy();
}

void AEnemy::DoDamage(AShooterCharacter* character)
{
	if (!character) return;

	UGameplayStatics::ApplyDamage(character, baseDamage, enemyController, this, UDamageType::StaticClass());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), meleeImpactSound, character->GetActorLocation());
}

void AEnemy::StunCharacter(AShooterCharacter* character)
{
	if (!character) return;

	const float chance = FMath::FRandRange(0.f, 1.f);

	if (chance <= character->GetStunChance())
	{
		character->Stun();
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;
	enemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
}

void AEnemy::OnLeftWeaponBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	AShooterCharacter* character = Cast<AShooterCharacter>(otherActor);

	if (!character) return;
	DoDamage(character);

	SpawnBloodParticle(character, leftWeaponSocket);
	StunCharacter(character);
}

void AEnemy::OnRightWeaponBeginOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	AShooterCharacter* character = Cast<AShooterCharacter>(otherActor);
	
	if (!character) return;
	DoDamage(character);

	SpawnBloodParticle(character, rightWeaponSocket);
	StunCharacter(character);
}

void AEnemy::SpawnBloodParticle(AShooterCharacter* character, FName socketName)
{
	const USkeletalMeshSocket* tipSocket = GetMesh()->GetSocketByName(socketName);
	if (!tipSocket) return;

	const FTransform socketTransfom = tipSocket->GetSocketTransform(GetMesh());
	if (!character->GetBloodParticles()) return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), character->GetBloodParticles(), socketTransfom);
}

void AEnemy::ActivateLeftWeapon()
{
	leftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	leftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	rightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	rightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::SetIsStunned(bool stunned)
{
	bIsStunned = stunned;

	if (!enemyController) return;
	enemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), stunned);
}

void AEnemy::SetTarget(AActor* target)
{
	if (!enemyController) return;

	AShooterCharacter* character = Cast<AShooterCharacter>(target);

	if (character)
	{
		// Set the value of the target
		enemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), character);
	}
}

