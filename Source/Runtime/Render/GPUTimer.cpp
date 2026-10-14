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
        if (SUCCEEDED(Hr))
        {
            Desc.Query = D3D11_QUERY_TIMESTAMP;
            Hr = Device->CreateQuery(&Desc, Set.Start.GetAddressOf());
        }
        if (SUCCEEDED(Hr))
            Hr = Device->CreateQuery(&Desc, Set.End.GetAddressOf());
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
    LastMs = 0.0;
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
        Context->Begin(Set.Disjoint.Get());
        Context->End(Set.Start.Get());
        return;
    }
    // 모든 세트가 GPU에서 처리 중이면 이번 측정만 생략한다.
}

void FGPUTimer::EndFrame()
{
    if (!Context || ActiveIndex < 0)
        return;
    FQuerySet& Set = Queries[ActiveIndex];
    Context->End(Set.End.Get());
    Context->End(Set.Disjoint.Get());
    Set.bPending = true;
    ActiveIndex = -1;
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

        uint64 Start = 0;
        uint64 End = 0;
        const HRESULT StartHr = Context->GetData(Set.Start.Get(), &Start, sizeof(Start), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        const HRESULT EndHr = Context->GetData(Set.End.Get(), &End, sizeof(End), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (FAILED(StartHr) || FAILED(EndHr))
        {
            Set.bPending = false;
            continue;
        }
        if (StartHr != S_OK || EndHr != S_OK)
            continue;

        if (End >= Start && Set.FrameId > LastResolvedFrameId)
        {
            LastMs = static_cast<double>(End - Start) * 1000.0 / static_cast<double>(Timing.Frequency);
            LastResolvedFrameId = Set.FrameId;
            bHasResult = true;
        }
        Set.bPending = false;
    }
}
