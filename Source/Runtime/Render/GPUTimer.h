#pragma once

#include "Core/Types.h"
#include <d3d11.h>
#include <wrl/client.h>

// 프레임 GPU 경과 시간을 기록하고, 완료된 과거 결과만 읽는다.
class FGPUTimer
{
public:
    bool Init(ID3D11Device* Device, ID3D11DeviceContext* InContext);
    void Shutdown();
    void CollectCompletedFrames();
    void BeginFrame();
    void EndFrame();

    bool HasResult() const { return bHasResult; }
    double GetLastMs() const { return LastMs; }

private:
    struct FQuerySet
    {
        Microsoft::WRL::ComPtr<ID3D11Query> Disjoint;
        Microsoft::WRL::ComPtr<ID3D11Query> Start;
        Microsoft::WRL::ComPtr<ID3D11Query> End;
        uint64 FrameId = 0;
        bool bPending = false;
    };

    FQuerySet Queries[4];
    ID3D11DeviceContext* Context = nullptr; // 엔진 루프의 Device보다 먼저 해제한다.
    int32 ActiveIndex = -1;
    uint64 NextFrameId = 0;
    uint64 LastResolvedFrameId = 0;
    double LastMs = 0.0;
    bool bHasResult = false;
};
