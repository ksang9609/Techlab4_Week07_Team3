#pragma once

#include "RenderEnums.h"

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
	static FShaderByteCode CompileShader(FString Path, const char* EntryPoint, EShaderType ShaderType);
	static FShaderByteCode CompileShaderWithDefines(FString Path, const char* EntryPoint, EShaderType ShaderType, 
		D3D_SHADER_MACRO* Defines);
	static void SaveByteCode(FString TargetPath, const FShaderByteCode& ByteCode);
	static FShaderByteCode LoadByteCode(const FString& CsoPath);

	static FString GetCsoPath(const FString& ShaderPath, EShaderType ShaderType);
	static FString GetCsoPathWithVarientKey(const FString& ShaderPath, EShaderType ShaderType, const FString& VarientKey);
	static bool IsCsoUpToDate(const FString& ShaderPath, const FString& CsoPath);
	static FShaderByteCode GetOrCompile(const FString& ShaderPath, const char* EntryPoint, EShaderType ShaderType, FString& OutCSOPath);
	static FShaderByteCode GetOrCompileWithDefines(const FString& ShaderPath, const char* EntryPoint, EShaderType ShaderType, FString& OutCSOPath, 
		const FString& VarientKey, D3D_SHADER_MACRO* const Defines);
};