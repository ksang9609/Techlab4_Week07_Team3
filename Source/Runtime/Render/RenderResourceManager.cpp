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
		if (Entry.path() == "Resources/Shader/UberLit.hlsl")
		{
			FString Path = Entry.path().generic_string();

			// UnLit Gouraud / Lambert / Phong
			D3D_SHADER_MACRO MacroUnLit[] = {
				{ nullptr, nullptr }
			};
			D3D_SHADER_MACRO MacroGouraud[] = {
				{ "LIGHTING_MODEL_GOURAUD", "1" }, 
				{ nullptr, nullptr } 
			};
			D3D_SHADER_MACRO MacroLambert[] = {
				{ "LIGHTING_MODEL_LAMBERT", "1" }, 
				{ nullptr, nullptr } 
			};
			D3D_SHADER_MACRO MacroPhong[] = {
				{ "LIGHTING_MODEL_PHONG", "1" },
				{ nullptr, nullptr }
			};
			D3D_SHADER_MACRO MacroLambertPhong[] = {
				{ "LIGHTING_MODEL_LAMBERT", "1" },
				{ "LIGHTING_MODEL_PHONG", "1" },
				{ nullptr, nullptr }
			};
			
			LoadOrCompileShader(Path, MacroGouraud);
			LoadOrCompileShader(Path, MacroLambert);
			LoadOrCompileShader(Path, MacroPhong);
			LoadOrCompileShader(Path, MacroLambertPhong);
			continue;
		}

		FString Path = Entry.path().generic_string();
		LoadOrCompileShader(Path);
	}
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

void FRenderResourceManager::LoadOrCompileShader(const FString& Path, D3D_SHADER_MACRO* Macro)
{ 
	FString VSCSOPath;
	FString PSCSOPath;
	FShaderByteCode VSCode = RenderUtil::GetOrCompile(Path, "mainVS", EShaderType::Vertex, VSCSOPath, Macro);
	FShaderByteCode PSCode = RenderUtil::GetOrCompile(Path, "mainPS", EShaderType::Pixel, PSCSOPath, Macro);

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

	FString NewName = RenderUtil::GetMacroName(Macro);
	NewName = Path + NewName;

	ShaderProgramMap[NewName] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

	HTR_LOG(Info, "[Shader] loaded: {}", NewName);

}
