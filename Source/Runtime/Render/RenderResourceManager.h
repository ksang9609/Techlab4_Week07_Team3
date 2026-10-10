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
private:
	void LoadOrCompileShader(const FString& Path);
	void CompileUberShader(); // UberShader.hlsl를 컴파일하고, 각 조합별로 ShaderProgram을 생성한다

	TMap<FString, TUniquePtr<FVertexShader>> VertexShaderMap;
	TMap<FString, TUniquePtr<FPixelShader>>  PixelShaderMap;
	TMap<FString, TUniquePtr<FShaderProgram>> ShaderProgramMap;
}; 