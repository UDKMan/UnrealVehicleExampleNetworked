// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"
#include "Particles/ParticleSystemComponent.h"

ABuggyPawn::ABuggyPawn(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	/** Camera strategy:
	 *  We want to keep a constant distance between car's location and camera.
	 *  We want to keep roll and pitch fixed
	 *	We want to interpolate yaw very slightly
	 *	We want to keep car almost constant in screen space width and height (i.e. if you draw a box around the car its center would be near constant and its dimensions would only vary on sharp turns or declines */

	// Create a spring arm component
	SpringArm = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 400.f);
	SpringArm->SetRelativeRotation( FRotator(0.f, 0.f, 0.f) );
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 675.0f; 
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;	

	// Create camera component 
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	EngineAC = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineAudio"));
	EngineAC->AttachParent = GetMesh();

	SkidAC = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("SkidAudio"));
	SkidAC->bAutoActivate = false;	//we don't want to start skid right away
	SkidAC->AttachParent = GetMesh();
	SkidThresholdVelocity = 30;
	SkidFadeoutTime = 0.1f;
	LongSlipSkidThreshold = 0.3f;
	LateralSlipSkidThreshold = 0.3f;
	SkidDurationRequiredForStopSound = 1.5f;
	
	SpringCompressionLandingThreshold = 250000.f;
	bTiresTouchingGround = false;

	ImpactEffectNormalForceThreshold = 100000.f;
}

void ABuggyPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (EngineAC)
	{
		EngineAC->SetSound(EngineSound);
		EngineAC->Play();
	}

	if (SkidAC)
	{
		SkidAC->SetSound(SkidSound);
		SkidAC->Stop();
	}
}

void ABuggyPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ABuggyPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABuggyPawn::MoveRight);

	InputComponent->BindAction("Handbrake", IE_Pressed, this, &ABuggyPawn::OnHandbrakePressed);
	InputComponent->BindAction("Handbrake", IE_Released, this, &ABuggyPawn::OnHandbrakeReleased);
}

void ABuggyPawn::MoveForward(float Val)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement == NULL || (MyPC && MyPC->IsHandbrakeForced()))
	{
		return;
	}

	VehicleMovement->SetThrottleInput(Val);
}

void ABuggyPawn::MoveRight(float Val)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement == NULL || (MyPC && MyPC->IsHandbrakeForced()))
	{
		return;
	}
	VehicleMovement->SetSteeringInput(Val);
}

void ABuggyPawn::OnHandbrakePressed()
{
	AVehiclePlayerController *VehicleController = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement != NULL)
	{
		VehicleMovement->SetHandbrakeInput(true);
	}
}

void ABuggyPawn::OnHandbrakeReleased()
{
	bHandbrakeActive = false;
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement != NULL)
	{
		GetVehicleMovementComponent()->SetHandbrakeInput(false);
	}
}

void ABuggyPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateWheelEffects(DeltaSeconds);
}


void ABuggyPawn::SpawnNewWheelEffect(int WheelIndex)
{
	DustPSC[WheelIndex] = NewObject<UParticleSystemComponent>(this);
	DustPSC[WheelIndex]->bAutoActivate = true;
	DustPSC[WheelIndex]->bAutoDestroy = false;
	DustPSC[WheelIndex]->RegisterComponentWithWorld(GetWorld());
	DustPSC[WheelIndex]->AttachTo(GetMesh(), GetVehicleMovement()->WheelSetups[WheelIndex].BoneName);
}

void ABuggyPawn::UpdateWheelEffects(float DeltaTime)
{
	if (GetVehicleMovement() && bTiresTouchingGround == false && LandingSound)	//we don't update bTiresTouchingGround until later in this function, so we can use it here to determine whether we're landing
	{
		float MaxSpringForce = GetVehicleMovement()->GetMaxSpringForce();
		if (MaxSpringForce > SpringCompressionLandingThreshold)
		{
			UGameplayStatics::PlaySoundAtLocation(this, LandingSound, GetActorLocation());
		}
	}

	bTiresTouchingGround = false;

	if (DustType && !bIsDying &&
		GetVehicleMovement() && GetVehicleMovement()->Wheels.Num() > 0)
	{
		const float CurrentSpeed = GetVehicleSpeed();
		for (int32 i = 0; i < ARRAY_COUNT(DustPSC); i++)
		{
			UPhysicalMaterial* ContactMat = GetVehicleMovement()->Wheels[i]->GetContactSurfaceMaterial();
			if (ContactMat != NULL)
			{
				bTiresTouchingGround = true;
			}
			UParticleSystem* WheelFX = DustType->GetDustFX(ContactMat, CurrentSpeed);

			const bool bIsActive = DustPSC[i] != NULL && !DustPSC[i]->bWasDeactivated && !DustPSC[i]->bWasCompleted;
			UParticleSystem* CurrentFX = DustPSC[i] != NULL ? DustPSC[i]->Template : NULL;
			if (WheelFX != NULL && (CurrentFX != WheelFX || !bIsActive))
			{
				if (DustPSC[i] == NULL || !DustPSC[i]->bWasDeactivated)
				{
					if (DustPSC[i] != NULL)
					{
						DustPSC[i]->SetActive(false);
						DustPSC[i]->bAutoDestroy = true;
					}
					SpawnNewWheelEffect(i);
				}
				DustPSC[i]->SetTemplate(WheelFX);
				DustPSC[i]->ActivateSystem();
			}
			else if (WheelFX == NULL && bIsActive)
			{
				DustPSC[i]->SetActive(false);
			}
		}
	}

	if (SkidAC != NULL)
	{
		FVector Vel = GetVelocity();
		bool bVehicleStopped = Vel.SizeSquared2D() < SkidThresholdVelocity*SkidThresholdVelocity;
		bool TireSlipping = GetVehicleMovement()->CheckSlipThreshold(LongSlipSkidThreshold, LateralSlipSkidThreshold);
		bool bWantsToSkid = bTiresTouchingGround && !bVehicleStopped && TireSlipping;

		float CurrTime = GetWorld()->GetTimeSeconds();
		if (bWantsToSkid && !bSkidding)
		{
			bSkidding = true;
			SkidAC->Play();
			SkidStartTime = CurrTime;
		}
		if (!bWantsToSkid && bSkidding)
		{
			bSkidding = false;
			SkidAC->FadeOut(SkidFadeoutTime, 0);
			if (CurrTime - SkidStartTime > SkidDurationRequiredForStopSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, SkidSoundStop, GetActorLocation());
			}
		}
	}
}

void ABuggyPawn::OnTrackPointReached(class AVehicleTrackPoint* NewCheckpoint)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	if (MyPC)
	{
		MyPC->OnTrackPointReached(NewCheckpoint);
	}
}

bool ABuggyPawn::IsHandbrakeActive() const
{
	return bHandbrakeActive;
}

float ABuggyPawn::GetVehicleSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) : 0.0f;
}

float ABuggyPawn::GetEngineRotationSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->GetEngineRotationSpeed()) : 0.0f;
}

float ABuggyPawn::GetEngineMaxRotationSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->MaxEngineRPM) : 1.f;
}

void ABuggyPawn::ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalForce, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalForce, Hit);

	if (ImpactTemplate && NormalForce.Size() > ImpactEffectNormalForceThreshold)
	{
		AVehicleImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AVehicleImpactEffect>(ImpactTemplate, HitLocation, HitNormal.Rotation());
		if (EffectActor)
		{
			float DotBetweenHitAndUpRotation = FVector::DotProduct(HitNormal, GetMesh()->GetUpVector());
			EffectActor->HitSurface = Hit;
			EffectActor->HitForce = NormalForce;
			EffectActor->bWheelLand = DotBetweenHitAndUpRotation > 0.8;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(HitNormal.Rotation(), HitLocation));
		}
	}

	if (ImpactCameraShake)
	{
		AVehiclePlayerController* PC = Cast<AVehiclePlayerController>(Controller);
		if (PC != NULL && PC->IsLocalController())
		{
			PC->ClientPlayCameraShake(ImpactCameraShake, 1);
		}
	}
}

float ABuggyPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Cast<APainCausingVolume>(DamageCauser) != NULL)
	{
		Die();
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}


void ABuggyPawn::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die();
}

bool ABuggyPawn::CanDie() const
{
	if ( bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode() == NULL
		|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}

void ABuggyPawn::Die()
{
	if (CanDie())
	{
		OnDeath();
	}
}

void ABuggyPawn::OnRep_Dying()
{
	if (bIsDying == true)
	{
		OnDeath();
	}
}

void ABuggyPawn::TornOff()
{
	Super::TornOff();

	SetLifeSpan(1.0f);
}

void ABuggyPawn::OnDeath()
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	DetachFromControllerPendingDestroy();

	// hide and disable
	TurnOff();
	SetActorHiddenInGame(true);

	if (EngineAC)
	{
		EngineAC->Stop();
	}

	if (SkidAC)
	{
		SkidAC->Stop();
	}
	
	PlayDestructionFX();
	// Give use a finite lifespan
	SetLifeSpan( 0.2f );	
}

void ABuggyPawn::PlayDestructionFX()
{
	if (DeathFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DeathFX, GetActorLocation(), GetActorRotation());
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

void ABuggyPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuggyPawn, bIsDying);
}
