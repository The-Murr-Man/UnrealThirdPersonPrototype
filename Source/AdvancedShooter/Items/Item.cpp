#include "Item.h"
#include <Components/BoxComponent.h>
#include <Components/WidgetComponent.h>
#include <Components/SphereComponent.h>
#include <AdvancedShooter/ShooterCharacter.h>
#include <Kismet/GameplayStatics.h>
#include "Camera/CameraComponent.h"
#include <Curves/CurveVector.h>

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	itemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(itemMesh);

	boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	boxCollision->SetupAttachment(itemMesh);
	boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	boxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	pickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	pickupWidget->SetupAttachment(GetRootComponent());


	areaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	areaSphere->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	if (!pickupWidget) return;

	pickupWidget->SetVisibility(false);

	SetActiveStars();
	
	// Setup overlap for area sphere
	areaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	areaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(itemState);

	// Set custom depth to disable
	InitCustomDepth();

	ResetPulseTimer();
}

void AItem::OnConstruction(const FTransform& transform)
{
	SetRarityData();
	if (!materialInstance) return;

	// Create a new material instance
	dynamicMaterialInstance = UMaterialInstanceDynamic::Create(materialInstance, this);
	dynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), glowColor);
	itemMesh->SetMaterial(materialIndex, dynamicMaterialInstance);

	EnableGlowMaterial();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);

	// Get curve values from pusle curve and set dynamic mat intance
	UpdatePulse();
}

void AItem::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, 
							int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (!otherActor) return;
	AShooterCharacter* shooterCharacter = Cast<AShooterCharacter>(otherActor);
	if (shooterCharacter)
	{
		shooterCharacter->IncrementOverlappedItemCount(1);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	if (!otherActor) return;
	AShooterCharacter* shooterCharacter = Cast<AShooterCharacter>(otherActor);
	if (shooterCharacter)
	{
		shooterCharacter->IncrementOverlappedItemCount(-1);
		shooterCharacter->UnHighlightInventorySlot();
	}
}

void AItem::SetActiveStars()
{
	// The zero element isnt used
	for (int32 i = 0; i <= 5; ++i)
	{
		activeStars.Add(false);
	}

	switch (itemRarity)
	{
		case EItemRarity::EIR_Damaged:
			activeStars[1] = true;
			break;
		case EItemRarity::EIR_Common:
			activeStars[1] = true;
			activeStars[2] = true;
			break;
		case EItemRarity::EIR_Uncommon:
			activeStars[1] = true;
			activeStars[2] = true;
			activeStars[3] = true;
			break;
		case EItemRarity::EIR_Rare:
			activeStars[1] = true;
			activeStars[2] = true;
			activeStars[3] = true;
			activeStars[4] = true;
			break;
		case EItemRarity::EIR_Legendary:
			activeStars[1] = true;
			activeStars[2] = true;
			activeStars[3] = true;
			activeStars[4] = true;
			activeStars[5] = true;
			break;
	}
}

void AItem::SetItemState(EItemState state)
{
	itemState = state;
	SetItemProperties(state);
}

void AItem::SetItemProperties(EItemState state)
{
	switch (state)
	{
		case EItemState::EIS_Pickup:
			// Set Mesh properties
			itemMesh->SetSimulatePhysics(false);
			itemMesh->SetEnableGravity(false);
			itemMesh->SetVisibility(true);
			itemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			itemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set area sphere properties
			areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			areaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			// Set box collision properties
			boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			boxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
			boxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;

		case EItemState::EIS_Equipped:

			pickupWidget->SetVisibility(false);
			// Set Mesh properties
			itemMesh->SetSimulatePhysics(false);
			itemMesh->SetEnableGravity(false);
			itemMesh->SetVisibility(true);
			itemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			itemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set area sphere properties
			areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set box collision properties
			boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			boxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_Falling:

			// Set mesh properties
			itemMesh->SetSimulatePhysics(true);
			itemMesh->SetEnableGravity(true);
			itemMesh->SetVisibility(true);
			itemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			itemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			itemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

			// Set area sphere properties
			areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set box collision properties
			boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			boxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_EquipInterping:
			pickupWidget->SetVisibility(false);

			// Set mesh properties
			itemMesh->SetSimulatePhysics(false);
			itemMesh->SetEnableGravity(false);
			itemMesh->SetVisibility(true);
			itemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			itemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set area sphere properties
			areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set box collision properties
			boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			boxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_PickedUp:
			pickupWidget->SetVisibility(false);

			// Set mesh properties
			itemMesh->SetSimulatePhysics(false);
			itemMesh->SetEnableGravity(false);
			itemMesh->SetVisibility(false);
			itemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			itemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set area sphere properties
			areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Set box collision properties
			boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			boxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
	}
}

void AItem::StartItemCurve(AShooterCharacter* _character, bool bForcePlaySound)
{
	// Store ref to character
	character = _character;

	// Get array index with the lowest item amount
	interpLocIndex = character->GetInterpLocationIndex();

	// Add 1 from the item amount at index
	character->IncrementInterpLocItemCount(interpLocIndex, 1);

	PlayPickupSound(bForcePlaySound);

	// Store location of item
	itemInterpStartlocation = GetActorLocation();

	bIsInterping = true;

	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().ClearTimer(pulseTimer);
	GetWorldTimerManager().SetTimer(itemInterpTimer, this, &AItem::FinishInterping, zCurveTime);

	// Get cameras initial yaw
	const float cameraRotationYaw = character->GetFollowCamera()->GetComponentRotation().Yaw;

	// Get items initial yaw
	const float itemRotationYaw = GetActorRotation().Yaw;

	// Initial yaw offset between camera and item
	interpInitialYawOffset = itemRotationYaw - cameraRotationYaw;

	bCanChangeCustomDepth = false;
}

void AItem::UpdatePulse()
{
	if (itemState != EItemState::EIS_Pickup && !dynamicMaterialInstance) return;

	float elapsedTime = 0;
	FVector curveValue = FVector::ZeroVector;

	switch (itemState)
	{
	case EItemState::EIS_Pickup:

		if (!pulseCurve) return;

		elapsedTime = GetWorldTimerManager().GetTimerElapsed(pulseTimer);
		curveValue = pulseCurve->GetVectorValue(elapsedTime);
		break;

	case EItemState::EIS_EquipInterping:
		if (!interpPulseCurve) return;

		elapsedTime = GetWorldTimerManager().GetTimerElapsed(itemInterpTimer);
		curveValue = interpPulseCurve->GetVectorValue(elapsedTime);
		break;

	default:
		break;
	}

	dynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), curveValue.X * glowAmount);

	dynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), curveValue.Y * fresnelExponent);

	dynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), curveValue.Z * fresnelReflectFraction);
}

void AItem::SetRarityData()
{
	FItemRarityTable* rarityRow = NULL;

	switch (itemRarity)
	{
	case EItemRarity::EIR_Damaged:
		rarityRow = GetRarityDataTable()->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
		break;

	case EItemRarity::EIR_Common:
		rarityRow = GetRarityDataTable()->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
		break;

	case EItemRarity::EIR_Uncommon:
		rarityRow = GetRarityDataTable()->FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
		break;

	case EItemRarity::EIR_Rare:
		rarityRow = GetRarityDataTable()->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
		break;

	case EItemRarity::EIR_Legendary:
		rarityRow = GetRarityDataTable()->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
		break;

	default:
		break;
	}

	if (!rarityRow) return;

	glowColor = rarityRow->glowColor;
	lightColor = rarityRow->lightColor;
	darkColor = rarityRow->darkColor;
	numOfStars = rarityRow->numOfstars;
	iconBackground = rarityRow->iconBackground;
	damageScalar = rarityRow->damageScalar;
	headshotDamageScalar = rarityRow->headshotDamageScalar;

	if (GetItemMesh())
	{
		GetItemMesh()->SetCustomDepthStencilValue(rarityRow->customDepthStencil);
	}
}

UDataTable* AItem::GetRarityDataTable()
{
	// Load data in the item rarity data table

	// Path to the item rarity data table
	FString rarityTablePath = TEXT("DataTable'/Game/_Game/DataTable/DT_ItemRarity.DT_ItemRarity'");

	UDataTable* rarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *rarityTablePath));

	if (!rarityTableObject) return NULL;

	return rarityTableObject;
}

void AItem::FinishInterping()
{
	bIsInterping = false;

	if (!character) return;

	// Subtract 1 from the item amount at index
	character->IncrementInterpLocItemCount(interpLocIndex, -1);
	character->GetPickupItem(this);
	character->UnHighlightInventorySlot();

	SetActorScale3D(FVector(1.f));

	DisableGlowMaterial();

	bCanChangeCustomDepth = true;

	DisableCustomDepth();
}

void AItem::ItemInterp(float deltaTime)
{
	if (!bIsInterping) return;

	if (!character && !itemZCurve && !itemScaleCurve) return;

	// Elapsed time since we started itemInterpTimer
	const float elapsedTime = GetWorldTimerManager().GetTimerElapsed(itemInterpTimer);

	// Get curve value at elapsed time
	const float curveValue = itemZCurve->GetFloatValue(elapsedTime);

	// Get item start location
	FVector itemLocation = itemInterpStartlocation;

	// Get location in front of camera
	const FVector cameraInterpLocation = GetInterpLocation();

	// Vector from item to camera location
	const FVector itemToCamera = FVector(0.f, 0.f, (cameraInterpLocation - itemLocation).Z);

	// Scale factor to multi with curve value
	const float deltaZ = itemToCamera.Size();

	const FVector currentLocation = GetActorLocation();

	// Interolated x and y value
	const float interpXValue = FMath::FInterpTo(currentLocation.X, cameraInterpLocation.X, deltaTime, 30.f);
	const float interpYValue = FMath::FInterpTo(currentLocation.Y, cameraInterpLocation.Y, deltaTime, 30.f);

	itemLocation.X = interpXValue;
	itemLocation.Y = interpYValue;

	// Adding curve value to the Z comp of the start location (Scaled by deltaZ)
	itemLocation.Z += curveValue * deltaZ;

	SetActorLocation(itemLocation, true, NULL, ETeleportType::TeleportPhysics);
	
	// Camera rotation this frame
	const FRotator cameraRotation = character->GetFollowCamera()->GetComponentRotation();

	// Camera rotation
	FRotator itemRotation = FRotator(0.f, cameraRotation.Yaw + interpInitialYawOffset, 0.f);
	SetActorRotation(itemRotation, ETeleportType::TeleportPhysics);

	const float scaleCurveValue = itemScaleCurve->GetFloatValue(elapsedTime);

	SetActorScale3D(FVector(scaleCurveValue, scaleCurveValue, scaleCurveValue));
}

FVector AItem::GetInterpLocation()
{
	if (!character) return FVector::ZeroVector;

	switch (itemType)
	{
		case EItemType::EIT_Ammo:
			return character->GetInterpLocation(interpLocIndex).sceneComp->GetComponentLocation();
			break;
		
		case EItemType::EIT_Weapon:
			return character->GetInterpLocation(0).sceneComp->GetComponentLocation();
			break;

		default:
			break;
	}

	return FVector();
}

void AItem::EnableCustomDepth()
{
	if (!bCanChangeCustomDepth) return;
	itemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	if (!bCanChangeCustomDepth) return;
	itemMesh->SetRenderCustomDepth(false);
}

void AItem::EnableGlowMaterial()
{
	if (!dynamicMaterialInstance) return;
	dynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
}

void AItem::DisableGlowMaterial()
{
	if (!dynamicMaterialInstance) return;
	dynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
}

void AItem::InitCustomDepth()
{
	DisableCustomDepth();
}

void AItem::ResetPulseTimer()
{
	if (itemState != EItemState::EIS_Pickup) return;
	GetWorldTimerManager().SetTimer(pulseTimer, this, &AItem::ResetPulseTimer, pulseCurveTime);
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
	if (!character && !pickupSound) return;

	if (bForcePlaySound)
	{
		UGameplayStatics::PlaySound2D(this, pickupSound);
	}

	else if (character->GetShouldPlayPickupSound())
	{
		character->StartPickupSoundTimer();

		UGameplayStatics::PlaySound2D(this, pickupSound);
	}
}

void AItem::PlayEquipSound(bool bForcePlaySound)
{
	if (!character && !equipSound) return;

	if (bForcePlaySound)
	{
		UGameplayStatics::PlaySound2D(this, equipSound);
	}

	else if (character->GetShouldPlayEquipSound())
	{
		character->StartEquipSoundTimer();

		UGameplayStatics::PlaySound2D(this, equipSound);
	}
}
