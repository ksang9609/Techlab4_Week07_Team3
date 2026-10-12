#pragma once

inline void to_json(json& J, const FVector& V)
{
	J = json::array({ V.X, V.Y, V.Z });
}

inline void from_json(const json& J, FVector& V)
{
	for (int32 i = 0; i < 3; i++)
	{
		V[i] = J[i].get<float>();
	}
}

inline void to_json(json& J, const FRotator& V)
{
	J = json::array({ V.Pitch, V.Yaw, V.Roll });
}

inline void from_json(const json& J, FRotator& V)
{
	for (int32 i = 0; i < 3; i++)
	{
		V[i] = J[i].get<float>();
	}
}

inline void to_json(json& J, const FVector4& V)
{
	J = json::array({ V.X, V.Y, V.Z, V.W });
}

inline void from_json(const json& J, FVector4& V)
{
	for (int32 i = 0; i < 4; i++)
	{
		V[i] = J[i].get<float>();
	}
}

inline void to_json(json& J, const FColor& C)
{
	J = json::array({ C.R, C.G, C.B, C.A });
}

inline void from_json(const json& J, FColor& C)
{
	if (J.is_array())
	{
		if (J[0].is_number_float() || J[1].is_number_float() || J[2].is_number_float() || J[3].is_number_float())
		{
			FLinearColor Linear(J[0].get<float>(), J[1].get<float>(), J[2].get<float>(), J[3].get<float>());
			C = Linear.ToFColor(false);
		}
		else
		{
			C.R = J[0].get<uint8>();
			C.G = J[1].get<uint8>();
			C.B = J[2].get<uint8>();
			C.A = J[3].get<uint8>();
		}
	}
	else if (J.is_object())
	{
		J.at("R").get_to(C.R);
		J.at("G").get_to(C.G);
		J.at("B").get_to(C.B);
		J.at("A").get_to(C.A);
	}
}

inline void to_json(json& J, const FLinearColor& C)
{
	J = json::array({ C.R, C.G, C.B, C.A });
}

inline void from_json(const json& J, FLinearColor& C)
{
	for (int32 i = 0; i < 4; i++)
	{
		C.RGBA[i] = J[i].get<float>();
	}
}

inline void to_json(json& J, const FTransform& T)
{
	J = json{ {"Location", T.Location}, {"Rotation", T.Rotation}, {"Scale", T.Scale} };
}

inline void from_json(const json& J, FTransform& T)
{
	J.at("Location").get_to(T.Location);
	J.at("Rotation").get_to(T.Rotation);
	J.at("Scale").get_to(T.Scale);
}