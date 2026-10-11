#pragma once

#include "RenderEnums.h"

enum EShaderMacro
{

};

struct FShaderByteCode
{
	ComPtr<ID3DBlob> Blob;

	const void* GetData() const { return Blob ? Blob->GetBufferPointer() : nullptr; }
	SIZE_T      GetSize() const { return Blob ? Blob->GetBufferSize() : 0; }
	bool        IsValid() const { return Blob != nullptr; }
};

class RenderUtil
{
public:
	static FShaderByteCode CompileShader(FString Path, const char* EntryPoint,
		EShaderType ShaderType, D3D_SHADER_MACRO* Macro = nullptr);
	static void SaveByteCode(FString TargetPath, const FShaderByteCode& ByteCode);
	static FShaderByteCode LoadByteCode(const FString& CsoPath);

	static FString GetCsoPath(const FString& ShaderPath, EShaderType ShaderType, D3D_SHADER_MACRO* Macro = nullptr);
	static bool IsCsoUpToDate(const FString& ShaderPath, const FString& CsoPath);
	static FShaderByteCode GetOrCompile(const FString& ShaderPath, const char* EntryPoint,
		EShaderType ShaderType, FString& OutCSOPath, D3D_SHADER_MACRO* Macro = nullptr);

	static FString GetMacroName(D3D_SHADER_MACRO* Macro = nullptr);
	static FString MacroNameToFString(FString Name);
};