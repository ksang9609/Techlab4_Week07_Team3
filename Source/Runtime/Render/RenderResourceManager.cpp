#include "EnginePCH.h"
#include "RenderResourceManager.h"

#include "RenderCommand.h"

#include "RenderUtil.h"



void FRenderResourceManager::ScanShaders(const fs::path& ShaderRoot)
{
	std::error_code ErrorCode;

	if (!fs::exists(ShaderRoot, ErrorCode))
	{
		HTR_LOG(Error, "[Shader] Scan Root Not Found : {}", ShaderRoot.generic_string());
		return;
	}

	for (const fs::directory_entry& Entry : fs::recursive_directory_iterator(ShaderRoot))
	{
		if (!Entry.is_regular_file()) continue;
		if (Entry.path().extension() != ".hlsl") continue;

		FString Path = Entry.path().generic_string();
		LoadOrCompileShader(Path);
	}

	// UberShader.hlsl를 LightingModel별로 컴파일하고 ShaderProgram을 생성한다
	CompileUberShader();
}

void FRenderResourceManager::Shutdown()
{
	Get().VertexShaderMap.Empty();
	Get().PixelShaderMap.Empty();
	Get().ShaderProgramMap.Empty();
}

FShaderProgram* FRenderResourceManager::GetShaderProgram(const FString& InPath)
{
	if (TUniquePtr<FShaderProgram>* Found = Get().ShaderProgramMap.Find(InPath))
		return Found->get();

	HTR_LOG(Error, "[Shader] not found: {}", InPath);

	if (TUniquePtr<FShaderProgram>* Fallback =
		Get().ShaderProgramMap.Find("Resources/Shader/DefaultShader.hlsl"))
		return Fallback->get();

	return nullptr;
}

void FRenderResourceManager::LoadOrCompileShader(const FString& Path)
{ 
	FString VSCSOPath;
	FString PSCSOPath;
	FShaderByteCode VSCode = RenderUtil::GetOrCompile(Path, "mainVS", EShaderType::Vertex, VSCSOPath);
	FShaderByteCode PSCode = RenderUtil::GetOrCompile(Path, "mainPS", EShaderType::Pixel, PSCSOPath);

	if (!VSCode.IsValid() || !PSCode.IsValid())
	{
		HTR_LOG(Error, "[Shader] compile failed: {}", Path);
		return;
	}

	TUniquePtr<FVertexShader> Vs = RenderCommand::CreateVertexShader(VSCode);
	TUniquePtr<FPixelShader>  Ps = RenderCommand::CreatePixelShader(PSCode);

	if (!Vs || !Vs->IsValid() || !Ps || !Ps->IsValid())
	{
		HTR_LOG(Error, "[Shader] device create failed: {}", Path);
		return;
	}

	FVertexShader* VsRaw = Vs.get();
	FPixelShader* PsRaw = Ps.get();

	VertexShaderMap[VSCSOPath] = std::move(Vs);
	PixelShaderMap[PSCSOPath] = std::move(Ps);
	ShaderProgramMap[Path] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

	HTR_LOG(Info, "[Shader] loaded: {}", Path);

}


void FRenderResourceManager::CompileUberShader()
{
	const FString LightingModelDefines[4] = { "VIEW_MODE_NORMAL", "LIGHTING_MODEL_GOURAUD", "LIGHTING_MODEL_LAMBERT", "LIGHTING_MODEL_PHONG" };
	const FString LightingModelNames[4] = { "Normal", "Gouraud", "Lambert", "Phong" };
	const FString ShaderPath = "Resources/Shader/UberLit.hlsl";

	for (int i = 0; i < 4; ++i)
	{
		// Compile shader with LightingModelDefine
		D3D_SHADER_MACRO Defines[] = {
			{ LightingModelDefines[i].c_str(), "1" },
			{ nullptr, nullptr }
		};

		FString VSCSOPath;
		FString PSCSOPath;
		
		FShaderByteCode VSCode = RenderUtil::GetOrCompileWithDefines(
			ShaderPath, "Uber_VS", EShaderType::Vertex, VSCSOPath, 
			LightingModelNames[i], Defines);
		FShaderByteCode PSCode = RenderUtil::GetOrCompileWithDefines(
			ShaderPath, "Uber_PS", EShaderType::Pixel, PSCSOPath, 
			LightingModelNames[i], Defines);

		if (!VSCode.IsValid() || !PSCode.IsValid())
		{
			HTR_LOG(Error, "[Shader] compile failed: {} with LightingModel {}", ShaderPath, LightingModelNames[i]);
			continue;
		}

		TUniquePtr<FVertexShader> Vs = RenderCommand::CreateVertexShader(VSCode);
		TUniquePtr<FPixelShader>  Ps = RenderCommand::CreatePixelShader(PSCode);

		if (!Vs || !Vs->IsValid() || !Ps || !Ps->IsValid())
		{
			HTR_LOG(Error, "[Shader] device create failed: {} with LightingModel {}", ShaderPath, LightingModelNames[i]);
			continue;
		}

		FVertexShader* VsRaw = Vs.get();
		FPixelShader* PsRaw = Ps.get();

		const FString VSCSOKey = VSCSOPath + "_" + LightingModelNames[i];
		const FString PSCSOKey = PSCSOPath + "_" + LightingModelNames[i];
		const FString ShaderKey = ShaderPath + "_" + LightingModelNames[i];
		VertexShaderMap[VSCSOKey] = std::move(Vs);
		PixelShaderMap[PSCSOKey] = std::move(Ps);
		ShaderProgramMap[ShaderKey] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

		HTR_LOG(Info, "[Shader] loaded: {} with LightingModel {}", ShaderPath, LightingModelNames[i]);
	}
}