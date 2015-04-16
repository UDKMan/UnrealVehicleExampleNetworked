// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"
#include "Particles/ParticleSystemComponent.h"

AVehiclePawn::AVehiclePawn(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UVehicleMovementComponentBoosted4w>(AWheeledVehicle::VehicleMovementComponentName))
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

	SkidAC = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("SkidAudio"));
	SkidAC->bAutoActivate = false;	//we don't want to start skid right away
	SkidThresholdVelocity = 30;
	SkidFadeoutTime = 0.1f;
	LongSlipSkidThreshold = 0.3f;
	LateralSlipSkidThreshold = 0.3f;
	SkidDurationRequiredForStopSound = 1.5f;
	
	SpringCompressionLandingThreshold = 250000.f;
	bTiresTouchingGround = false;

	ImpactEffectNormalForceThreshold = 100000.f;
}

void AVehiclePawn::PostInitializeComponents()
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

void AVehiclePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AVehiclePawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AVehiclePawn::MoveRight);

	InputComponent->BindAction("Handbrake", IE_Pressed, this, &AVehiclePawn::OnHandbrakePressed);
	InputComponent->BindAction("Handbrake", IE_Released, this, &AVehiclePawn::OnHandbrakeReleased);

	// return to track
	InputComponent->BindAction("BackOnTrack", IE_Pressed, this, &AVehiclePawn::Suicide);
}

void AVehiclePawn::MoveForward(float Val)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement == NULL || (MyPC && MyPC->IsHandbrakeForced()))
	{
		return;
	}

	VehicleMovement->SetThrottleInput(Val);
}

void AVehiclePawn::MoveRight(float Val)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement == NULL || (MyPC && MyPC->IsHandbrakeForced()))
	{
		return;
	}
	VehicleMovement->SetSteeringInput(Val);
}

void AVehiclePawn::OnHandbrakePressed()
{
	AVehiclePlayerController *VehicleController = Cast<AVehiclePlayerController>(GetController());
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement != NULL)
	{
		VehicleMovement->SetHandbrakeInput(true);
	}
}

void AVehiclePawn::OnHandbrakeReleased()
{
	bHandbrakeActive = false;
	UWheeledVehicleMovementComponent *VehicleMovement = GetVehicleMovementComponent();
	if (VehicleMovement != NULL)
	{
		GetVehicleMovementComponent()->SetHandbrakeInput(false);
	}
}

void AVehiclePawn::Suicide()
{
	if( GetWorld() != NULL )
	{
		AVehicleGameMode* const MyGame = GetWorld()->GetAuthGameMode<AVehicleGameMode>();
		if( ( MyGame != NULL )&& ( MyGame->IsRaceActive() == true ) )
		{
			ServerSuicide();
		}
	}
}

bool AVehiclePawn::ServerSuicide_Validate()
{
	return true;
}

void AVehiclePawn::ServerSuicide_Implementation()
{
	Die();
}

void AVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateWheelEffects(DeltaSeconds);
}


void AVehiclePawn::SpawnNewWheelEffect(int WheelIndex)
{
	DustPSC[WheelIndex] = NewObject<UParticleSystemComponent>(this);
	DustPSC[WheelIndex]->bAutoActivate = true;
	DustPSC[WheelIndex]->bAutoDestroy = false;
	DustPSC[WheelIndex]->RegisterComponentWithWorld(GetWorld());
	DustPSC[WheelIndex]->AttachTo(GetMesh(), GetVehicleMovement()->WheelSetups[WheelIndex].BoneName);
}

void AVehiclePawn::UpdateWheelEffects(float DeltaTime)
{
	if (bTiresTouchingGround == false && LandingSound)	//we don't update bTiresTouchingGround until later in this function, so we can use it here to determine whether we're landing
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

void AVehiclePawn::OnTrackPointReached(class AVehicleTrackPoint* NewCheckpoint)
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	if (MyPC)
	{
		MyPC->OnTrackPointReached(NewCheckpoint);
	}
}

bool AVehiclePawn::IsHandbrakeActive() const
{
	return bHandbrakeActive;
}

float AVehiclePawn::GetVehicleSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) : 0.0f;
}

float AVehiclePawn::GetEngineRotationSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->GetEngineRotationSpeed()) : 0.0f;
}

float AVehiclePawn::GetEngineMaxRotationSpeed() const
{
	return (GetVehicleMovement()) ? FMath::Abs(GetVehicleMovement()->MaxEngineRPM) : 1.f;
}

void AVehiclePawn::ReceiveHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalForce, const FHitResult& Hit)
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

float AVehiclePawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Cast<APainCausingVolume>(DamageCauser) != NULL)
	{
		Die();
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}


void AVehiclePawn::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die();
}

bool AVehiclePawn::CanDie() const
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

void AVehiclePawn::Die()
{
	if (CanDie())
	{
		OnDeath();
	}
}

void AVehiclePawn::OnRep_Dying()
{
	OnDeath();
}

void AVehiclePawn::OnDeath()
{
	AVehiclePlayerController* MyPC = Cast<AVehiclePlayerController>(GetController());
	bReplicateMovement = false;
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

void AVehiclePawn::PlayDestructionFX()
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

void AVehiclePawn::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AVehiclePawn, bIsDying );
}
