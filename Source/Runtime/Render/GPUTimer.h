#pragma once

#include "Core/Types.h"
#include <d3d11.h>
#include <wrl/client.h>

// GPU 패스 시간을 기록하고, 완료된 과거 프레임 결과만 읽는다.
class FGPUTimer
{
public:
    enum class EPass { LightCulling, Opaque, Count };
    bool Init(ID3D11Device* Device, ID3D11DeviceContext* InContext);
    void Shutdown();
    void CollectCompletedFrames();
    void BeginFrame();
    void EndFrame();
    int32 BeginPass(EPass Pass);
    void EndPass(EPass Pass, int32 QueryIndex);

    bool HasResult() const { return bHasResult; }
    double GetPassMs(EPass Pass) const { return PassMs[static_cast<int32>(Pass)]; }

private:
    static constexpr int32 PassCount = static_cast<int32>(EPass::Count);
    static constexpr int32 MaxPassCalls = 5; // 네 에디터 View + 외부 PIE View
    struct FPassQuery
    {
        Microsoft::WRL::ComPtr<ID3D11Query> Start;
        Microsoft::WRL::ComPtr<ID3D11Query> End;
        bool bClosed = false;
    };
    struct FQuerySet
    {
        Microsoft::WRL::ComPtr<ID3D11Query> Disjoint;
        FPassQuery Passes[PassCount][MaxPassCalls];
        int32 PassCalls[PassCount] = {};
        uint64 FrameId = 0;
        bool bPending = false;
    };

    FQuerySet Queries[4];
    ID3D11DeviceContext* Context = nullptr; // 엔진 루프의 Device보다 먼저 해제한다.
    int32 ActiveIndex = -1;
    uint64 NextFrameId = 0;
    uint64 LastResolvedFrameId = 0;
    double PassMs[PassCount] = {};
    bool bHasResult = false;
};
