#include "EnginePCH.h"
#include "Rotator.h"
#include "Math/EngineMath.h"

FRotator::FRotator()
{
	Pitch = 0; Yaw = 0; Roll = 0;
}

FRotator::FRotator(float P, float Y, float R)
{
	Pitch = P; Yaw = Y; Roll = R;
}

FRotator::FRotator(FVector V)
{
	Pitch = V.X; Yaw = V.Y; Roll = V.Z;
}


FQuat FRotator::Quaternion() const
{
	// Degree를 Radian으로 변환
	float PRad = Pitch * (PI / 180.0f);
	float YRad = Yaw * (PI / 180.0f);
	float RRad = Roll * (PI / 180.0f);
	
	return FQuat::MakeFromEuler(RRad, PRad, YRad);
}

FVector FRotator::GetForwardVector() const
{
    const float P = FMath::DegreesToRadians(Pitch);
    const float Y = FMath::DegreesToRadians(Yaw);

    const float SP = sinf(P);
    const float CP = cosf(P);
    const float SY = sinf(Y);
    const float CY = cosf(Y);

    return FVector(CP * CY, CP * SY, -SP);
}

FVector FRotator::GetRightVector() const
{
    const float P = FMath::DegreesToRadians(Pitch);
    const float Y = FMath::DegreesToRadians(Yaw);
    const float R = FMath::DegreesToRadians(Roll);

    const float SP = sinf(P);
    const float CP = cosf(P);
    const float SY = sinf(Y);
    const float CY = cosf(Y);
    const float SR = sinf(R);
    const float CR = cosf(R);

    return FVector(
        SR * SP * CY - CR * SY,
        SR * SP * SY + CR * CY,
        SR * CP
    );
}

FVector FRotator::GetUpVector() const
{
    const float P = FMath::DegreesToRadians(Pitch);
    const float Y = FMath::DegreesToRadians(Yaw);
    const float R = FMath::DegreesToRadians(Roll);

    const float SP = sinf(P);
    const float CP = cosf(P);
    const float SY = sinf(Y);
    const float CY = cosf(Y);
    const float SR = sinf(R);
    const float CR = cosf(R);

    return FVector(
        CR * SP * CY + SR * SY,
        CR * SP * SY - SR * CY,
        CR * CP
    );
}

FRotator FRotator::Identitiy = FRotator(0, 0, 0);

FRotator operator+(FRotator Rot, const FVector& Vec)
{
	Rot.Roll += Vec.X;
	Rot.Pitch += Vec.Y;
	Rot.Yaw += Vec.Z;

	return Rot;
}