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
};

enum class EViewMode
{
	Lit_Gouraud,
	Lit_Lambert,
	Lit_Phong,
	Wireframe,
	// Unlit,
};