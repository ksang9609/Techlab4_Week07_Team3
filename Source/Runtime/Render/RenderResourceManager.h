#pragma once

#include "Shader.h"

class FRenderResourceManager
{
public:
	static FRenderResourceManager& Get()
	{
		static FRenderResourceManager* Instance = new FRenderResourceManager();
		return *Instance;
	}

	static void Init()
	{
		Get().ScanShaders("Resources/Shader");
	}

	void ScanShaders(const fs::path& ShaderRoot);

	static void Shutdown();

	static FShaderProgram* GetShaderProgram(const FString& InPath);
	static FComputeShader* GetComputeShader(const FString& InPath);

private:
	void LoadOrCompileShader(const FString& Path, const D3D_SHADER_MACRO* Defines = nullptr);
	void LoadOrCompileComputeShader(const FString& Path, const D3D_SHADER_MACRO* Defines = nullptr);

	TMap<FString, TUniquePtr<FVertexShader>> VertexShaderMap;
	TMap<FString, TUniquePtr<FPixelShader>>  PixelShaderMap;
	TMap<FString, TUniquePtr<FShaderProgram>> ShaderProgramMap;
	TMap<FString, TUniquePtr<FComputeShader>> ComputeShaderMap;
}; 