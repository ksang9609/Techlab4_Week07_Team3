#include "EnginePCH.h"
#include "Render/MeshTangentGenerator.h"

#include "Render/StaticMeshData.h"
#include "mikktspace.h"

namespace
{
    struct FMikkUserData
    {
        const FStaticMeshData* Mesh = nullptr;
        TArray<FVector4>* CornerTangents = nullptr;
    };

    int GetNumFaces(const SMikkTSpaceContext* Context)
    {
        const FMikkUserData* UserData = static_cast<const FMikkUserData*>(Context->m_pUserData);
        const FStaticMeshData* Mesh = UserData->Mesh;
        return Mesh->Indices.Num() / 3;
    }

    int GetNumVerticesOfFace(const SMikkTSpaceContext* Context, int FaceIndex)
    {
        return 3;
    }

    void GetPosition(const SMikkTSpaceContext* Context, float OutPosition[], int FaceIndex, int VertexIndex)
    {
        const FMikkUserData* UserData = static_cast<const FMikkUserData*>(Context->m_pUserData);
        const FStaticMeshData* Mesh = UserData->Mesh;
        const uint32 Index = Mesh->Indices[FaceIndex * 3 + VertexIndex];
        const FVector& Position = Mesh->Vertices[Index].Position;

        OutPosition[0] = Position.X;
        OutPosition[1] = Position.Y;
        OutPosition[2] = Position.Z;
    }

    void GetNormal(const SMikkTSpaceContext* Context, float OutNormal[], int FaceIndex, int VertexIndex)
    {
        const FMikkUserData* UserData = static_cast<const FMikkUserData*>(Context->m_pUserData);
        const FStaticMeshData* Mesh = UserData->Mesh;
        const uint32 Index = Mesh->Indices[FaceIndex * 3 + VertexIndex];
        const FVector& Normal = Mesh->Vertices[Index].Normal;

        OutNormal[0] = Normal.X;
        OutNormal[1] = Normal.Y;
        OutNormal[2] = Normal.Z;
    }

    void GetTexCoord(const SMikkTSpaceContext* Context, float OutTexCoord[], int FaceIndex, int VertexIndex)
    {
        const FMikkUserData* UserData = static_cast<const FMikkUserData*>(Context->m_pUserData);
        const FStaticMeshData* Mesh = UserData->Mesh;
        const uint32 Index = Mesh->Indices[FaceIndex * 3 + VertexIndex];
        const FVector2& UV = Mesh->Vertices[Index].UV;

        OutTexCoord[0] = UV.X;
        OutTexCoord[1] = UV.Y;
    }

    void SetTSpaceBasic(const SMikkTSpaceContext* Context, const float Tangent[], const float Sign, const int FaceIndex, const int VertexIndex)
    {
        FMikkUserData* UserData = static_cast<FMikkUserData*>(Context->m_pUserData);
        const uint32 CornerIndex = FaceIndex * 3 + VertexIndex;

        (*UserData->CornerTangents)[CornerIndex] = FVector4(Tangent[0], Tangent[1], Tangent[2], Sign);
    }

    bool IsSameTangent(const FVector4& A, const FVector4& B)
    {
        constexpr float Epsilon = 0.0001f;

        return std::abs(A.X - B.X) < Epsilon &&
            std::abs(A.Y - B.Y) < Epsilon &&
            std::abs(A.Z - B.Z) < Epsilon &&
            A.W == B.W;
    }

}
    void GenerateMeshTangents(FStaticMeshData& Mesh)
    {
        SMikkTSpaceInterface Interface = {};

        Interface.m_getNumFaces = GetNumFaces;
        Interface.m_getNumVerticesOfFace = GetNumVerticesOfFace;
        Interface.m_getPosition = GetPosition;
        Interface.m_getNormal = GetNormal;
        Interface.m_getTexCoord = GetTexCoord;
        Interface.m_setTSpaceBasic = SetTSpaceBasic;

        TArray<FVector4> CornerTangents;
        CornerTangents.SetNum(Mesh.Indices.Num());

        FMikkUserData UserData = {};
        UserData.Mesh = &Mesh;
        UserData.CornerTangents = &CornerTangents;

        SMikkTSpaceContext Context = {};
        Context.m_pInterface = &Interface;
        Context.m_pUserData = &UserData;

        if (!genTangSpaceDefault(&Context))
            return;

        TArray<int32> FirstCornerIndices;
        FirstCornerIndices.Init(-1, static_cast<uint32>(Mesh.Vertices.Num()));

        for (int32 CornerIndex = 0; CornerIndex < Mesh.Indices.Num(); ++CornerIndex)
        {
            const uint32 VertexIndex = Mesh.Indices[CornerIndex];

            if (FirstCornerIndices[VertexIndex] == -1)
                FirstCornerIndices[VertexIndex] = CornerIndex;
        }

        for (int32 VertexIndex = 0; VertexIndex < Mesh.Vertices.Num(); ++VertexIndex)
        {
            const int32 CornerIndex = FirstCornerIndices[VertexIndex];

            if (CornerIndex == -1)
                continue;

            Mesh.Vertices[VertexIndex].Tangent = CornerTangents[CornerIndex];
        }

        for (int32 CornerIndex = 0; CornerIndex < Mesh.Indices.Num(); ++CornerIndex)
        {
            const uint32 VertexIndex = Mesh.Indices[CornerIndex];

            const FVector4& ExistingTangent = Mesh.Vertices[VertexIndex].Tangent;
            const FVector4& RequiredTangent = CornerTangents[CornerIndex];

            if (IsSameTangent(ExistingTangent, RequiredTangent))
                continue;

            // 기존 정점의 속성을 복사하고, 이 꼭짓점에 필요한 Tangent를 적용한다.
            FVertexPNCTT NewVertex = Mesh.Vertices[VertexIndex];
            NewVertex.Tangent = RequiredTangent;

            const uint32 NewVertexIndex = Mesh.Vertices.Add(NewVertex);

            // 현재 삼각형만 복제된 정점을 참조하도록 변경한다.
            Mesh.Indices[CornerIndex] = NewVertexIndex;

            // LOD 원본 메타데이터도 정점 개수에 맞춰 복제한다.
            if (!Mesh.LODSourceVertices.IsEmpty())
            {
                const FLODSourceVertex SourceVertex = Mesh.LODSourceVertices[VertexIndex];
                Mesh.LODSourceVertices.Add(SourceVertex);
            }
        }
    }
