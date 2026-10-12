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
		if (Entry.path().filename() == "UberLit.hlsl")
		{
			const D3D_SHADER_MACRO GouraudDefines[] =
			{
				{ "LIGHTING_MODEL_GOURAUD", "1" },
				{ nullptr, nullptr }
			};
			LoadOrCompileShader(Path, GouraudDefines, "Gouraud");

			const D3D_SHADER_MACRO LambertDefines[] =
			{
				{ "LIGHTING_MODEL_LAMBERT", "1" },
				{ nullptr, nullptr }
			};
			LoadOrCompileShader(Path, LambertDefines, "Lambert");

			const D3D_SHADER_MACRO PhongDefines[] =
			{
				{ "LIGHTING_MODEL_PHONG", "1" },
				{ nullptr, nullptr }
			};
			LoadOrCompileShader(Path, PhongDefines, "Phong");

			const D3D_SHADER_MACRO UnlitDefines[] =
			{
				{ "LIGHTING_MODEL_UNLIT", "1" },
				{ nullptr, nullptr }
			};
			LoadOrCompileShader(Path, UnlitDefines, "Unlit");
		}
		else
		{
			LoadOrCompileShader(Path);
		}
	}
}

void FRenderResourceManager::Shutdown()
{
	Get().VertexShaderMap.Empty();
	Get().PixelShaderMap.Empty();
	Get().ShaderProgramMap.Empty();
}

FShaderProgram* FRenderResourceManager::GetShaderProgram(const FString& InPath, const FString& VariantName)
{
	FString SharedKey = VariantName.empty() ? InPath : InPath + "_" + VariantName;
	if (TUniquePtr<FShaderProgram>* Found = Get().ShaderProgramMap.Find(SharedKey))
		return Found->get();

	HTR_LOG(Error, "[Shader] not found: {}", InPath);

	if (TUniquePtr<FShaderProgram>* Fallback = Get().ShaderProgramMap.Find("Resources/Shader/DefaultShader.hlsl"))
		return Fallback->get();

	return nullptr;
}

void FRenderResourceManager::LoadOrCompileShader(const FString& Path, const D3D_SHADER_MACRO* Defines, const FString& VariantName)
{ 
	FString VSCSOPath;
	FString PSCSOPath;
	FShaderByteCode VSCode = RenderUtil::GetOrCompile(Path, "mainVS", EShaderType::Vertex, VSCSOPath, Defines, VariantName);
	FShaderByteCode PSCode = RenderUtil::GetOrCompile(Path, "mainPS", EShaderType::Pixel, PSCSOPath, Defines, VariantName);

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

	FString SharedKey = VariantName.empty() ? Path : Path + "_" + VariantName;
	ShaderProgramMap[SharedKey] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

	HTR_LOG(Info, "[Shader] loaded: {}", Path);

}
