#pragma once

enum class EShaderType
{
	Vertex,
	Pixel,
	Compute,
};

enum class EViewMode
{
	UnLit,
	Wireframe,
	Lit_Gouraud,
	Lit_Lambert,
	Lit_Phong,
	Normal,
	Count,
	Max = Count,
};