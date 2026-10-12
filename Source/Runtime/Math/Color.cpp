#include "EnginePCH.h"
#include "Color.h"


// Common colors.
const FLinearColor FLinearColor::White(1.f, 1.f, 1.f);
const FLinearColor FLinearColor::Gray(0.5f, 0.5f, 0.5f);
const FLinearColor FLinearColor::Black(0, 0, 0);
const FLinearColor FLinearColor::Transparent(0, 0, 0, 0);
const FLinearColor FLinearColor::Red(1.f, 0, 0);
const FLinearColor FLinearColor::Green(0, 1.f, 0);
const FLinearColor FLinearColor::Blue(0, 0, 1.f);
const FLinearColor FLinearColor::Yellow(1.f, 1.f, 0);

const FColor FColor::White(255, 255, 255);
const FColor FColor::Black(0, 0, 0);
const FColor FColor::Transparent(0, 0, 0, 0);
const FColor FColor::Red(255, 0, 0);
const FColor FColor::Green(0, 255, 0);
const FColor FColor::Blue(0, 0, 255);
const FColor FColor::Yellow(255, 255, 0);
const FColor FColor::Cyan(0, 255, 255);
const FColor FColor::Magenta(255, 0, 255);
const FColor FColor::Orange(243, 156, 18);
const FColor FColor::Purple(169, 7, 228);
const FColor FColor::Turquoise(26, 188, 156);
const FColor FColor::Silver(189, 195, 199);
const FColor FColor::Emerald(46, 204, 113);

FLinearColor::FLinearColor(const FColor& Color) :
	R(sRGBToLinearTable[Color.R]),
	G(sRGBToLinearTable[Color.G]),
	B(sRGBToLinearTable[Color.B]),
	A(Color.A *( 1.0f / 255.0f))
{
}

FLinearColor::FLinearColor(const FVector& Vector) :
	R(Vector.X),
	G(Vector.Y),
	B(Vector.Z),
	A(1.0f)
{
}

FLinearColor::FLinearColor(const FVector4& Vector) :
	R(Vector.X),
	G(Vector.Y),
	B(Vector.Z),
	A(Vector.W)
{
}

FColor FLinearColor::ToFColor(bool bSRGB) const
{
	float FloatR = FMath::Clamp(R, 0.0f, 1.0f);
	float FloatG = FMath::Clamp(G, 0.0f, 1.0f);
	float FloatB = FMath::Clamp(B, 0.0f, 1.0f);
	float FloatA = FMath::Clamp(A, 0.0f, 1.0f);

	if(bSRGB)
	{
		if(FloatR < 0.0031308f)
		{
			FloatR = 12.92f * FloatR;
		}
		else
		{
			FloatR = 1.055f* powf(FloatR, 1.0f/2.4f) - 0.055f;
		}
		if(FloatG < 0.0031308f)
		{
			FloatG = 12.92f * FloatG;
		}
		else
		{
			FloatG = 1.055f* powf(FloatG, 1.0f/2.4f) - 0.055f;
		}
		if(FloatB < 0.0031308f)
		{
			FloatB = 12.92f * FloatB;
		}
		else
		{
			FloatB = 1.055f* powf(FloatB, 1.0f/2.4f) - 0.055f;
		}
	}
	
	FColor OutColor;
	OutColor.R = static_cast<uint8>(FloatR * 255.0f + 0.5f);
	OutColor.G = static_cast<uint8>(FloatG * 255.0f + 0.5f);
	OutColor.B = static_cast<uint8>(FloatB * 255.0f + 0.5f);
	OutColor.A = static_cast<uint8>(FloatA * 255.0f + 0.5f);

	return OutColor;
}