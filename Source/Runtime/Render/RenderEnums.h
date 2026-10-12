#pragma once

enum class EShaderType
{
	Vertex,
	Pixel,
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