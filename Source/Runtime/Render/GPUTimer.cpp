#include "EnginePCH.h"
#include "Render/GPUTimer.h"

bool FGPUTimer::Init(ID3D11Device* Device, ID3D11DeviceContext* InContext)
{
    Shutdown();
    if (!Device || !InContext)
        return false;

    for (FQuerySet& Set : Queries)
    {
        D3D11_QUERY_DESC Desc{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };
        HRESULT Hr = Device->CreateQuery(&Desc, Set.Disjoint.GetAddressOf());
        Desc.Query = D3D11_QUERY_TIMESTAMP;
        for (auto& Pass : Set.Passes)
        {
            for (FPassQuery& Query : Pass)
            {
                if (SUCCEEDED(Hr))
                    Hr = Device->CreateQuery(&Desc, Query.Start.GetAddressOf());
                if (SUCCEEDED(Hr))
                    Hr = Device->CreateQuery(&Desc, Query.End.GetAddressOf());
            }
        }
        if (FAILED(Hr))
        {
            Shutdown();
            return false;
        }
    }
    Context = InContext;
    return true;
}

void FGPUTimer::Shutdown()
{
    if (Context && ActiveIndex >= 0)
        EndFrame();
    for (FQuerySet& Set : Queries)
        Set = FQuerySet{};
    Context = nullptr;
    ActiveIndex = -1;
    NextFrameId = 0;
    LastResolvedFrameId = 0;
    for (double& Ms : PassMs)
        Ms = 0.0;
    bHasResult = false;
}

void FGPUTimer::BeginFrame()
{
    if (!Context || ActiveIndex >= 0)
        return;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FQuerySet& Set = Queries[Index];
        if (Set.bPending)
            continue;
        ActiveIndex = Index;
        Set.FrameId = ++NextFrameId;
        for (int32& Calls : Set.PassCalls)
            Calls = 0;
        Context->Begin(Set.Disjoint.Get());
        return;
    }
    // 모든 세트가 GPU에서 처리 중이면 이번 측정만 생략한다.
}

void FGPUTimer::EndFrame()
{
    if (!Context || ActiveIndex < 0)
        return;
    FQuerySet& Set = Queries[ActiveIndex];
    Context->End(Set.Disjoint.Get());
    Set.bPending = true;
    ActiveIndex = -1;
}

int32 FGPUTimer::BeginPass(EPass Pass)
{
    if (!Context || ActiveIndex < 0)
        return -1;
    FQuerySet& Set = Queries[ActiveIndex];
    const int32 PassIndex = static_cast<int32>(Pass);
    if (Set.PassCalls[PassIndex] >= MaxPassCalls)
        return -1;
    const int32 QueryIndex = Set.PassCalls[PassIndex]++;
    FPassQuery& Query = Set.Passes[PassIndex][QueryIndex];
    Query.bClosed = false;
    Context->End(Query.Start.Get());
    return QueryIndex;
}

void FGPUTimer::EndPass(EPass Pass, int32 QueryIndex)
{
    if (!Context || ActiveIndex < 0 || QueryIndex < 0)
        return;
    FQuerySet& Set = Queries[ActiveIndex];
    const int32 PassIndex = static_cast<int32>(Pass);
    if (QueryIndex >= Set.PassCalls[PassIndex])
        return;
    FPassQuery& Query = Set.Passes[PassIndex][QueryIndex];
    if (!Query.bClosed)
    {
        Context->End(Query.End.Get());
        Query.bClosed = true;
    }
}

void FGPUTimer::CollectCompletedFrames()
{
    if (!Context)
        return;
    for (FQuerySet& Set : Queries)
    {
        if (!Set.bPending)
            continue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT Timing{};
        HRESULT Hr = Context->GetData(Set.Disjoint.Get(), &Timing, sizeof(Timing), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (Hr == S_FALSE)
            continue;
        if (FAILED(Hr) || Timing.Disjoint || Timing.Frequency == 0)
        {
            Set.bPending = false;
            continue;
        }

        double ResolvedPassMs[PassCount] = {};
        bool bReady = true;
        bool bValid = true;
        for (int32 Pass = 0; Pass < PassCount; ++Pass)
        {
            for (int32 Call = 0; Call < Set.PassCalls[Pass]; ++Call)
            {
                FPassQuery& Query = Set.Passes[Pass][Call];
                if (!Query.bClosed)
                {
                    bValid = false;
                    continue;
                }
                uint64 PassStart = 0;
                uint64 PassEnd = 0;
                const HRESULT A = Context->GetData(Query.Start.Get(), &PassStart, sizeof(PassStart), D3D11_ASYNC_GETDATA_DONOTFLUSH);
                const HRESULT B = Context->GetData(Query.End.Get(), &PassEnd, sizeof(PassEnd), D3D11_ASYNC_GETDATA_DONOTFLUSH);
                if (FAILED(A) || FAILED(B))
                    bValid = false;
                else if (A != S_OK || B != S_OK)
                    bReady = false;
                else if (PassEnd < PassStart)
                    bValid = false;
                else
                    ResolvedPassMs[Pass] += static_cast<double>(PassEnd - PassStart) * 1000.0 / static_cast<double>(Timing.Frequency);
            }
        }
        if (bValid && !bReady)
            continue;
        if (bValid && Set.FrameId > LastResolvedFrameId)
        {
            for (int32 Pass = 0; Pass < PassCount; ++Pass)
                PassMs[Pass] = ResolvedPassMs[Pass];
            LastResolvedFrameId = Set.FrameId;
            bHasResult = true;
        }
        Set.bPending = false;
    }
}
