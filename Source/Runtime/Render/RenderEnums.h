#pragma once

enum class EShaderType
{
	Vertex,
	Pixel,
};

enum class ELightingModel
{
	Gouraud,
	Lambert,
	Phong,
	Normal,
};

enum class EViewMode
{
	Lit_Gouraud,
	Lit_Lambert,
	Lit_Phong,
	Normal,
	Wireframe,
	// Unlit,
	Count, // Count는 마지막에 두고, 실제 값은 0~Count-1 범위로 사용한다.
};