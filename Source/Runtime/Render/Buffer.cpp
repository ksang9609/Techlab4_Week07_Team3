#include "EnginePCH.h"
#include "Buffer.h"

FBuffer::FBuffer(ID3D11Device* Device, const D3D11_BUFFER_DESC& Desc, const void* InitialData)
{
	BufferSize = Desc.ByteWidth;

	HRESULT hr;
	if (InitialData)
	{
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = InitialData;

		hr = Device->CreateBuffer(&Desc, &Data, Buffer.GetAddressOf());
	}
	else
	{
		hr = Device->CreateBuffer(&Desc, nullptr, Buffer.GetAddressOf());
	}
	if (FAILED(hr))
	{
		HTR_LOG(Warning, "Failed To Create Buffer!");
	}
}

FConstantBuffer::FConstantBuffer(ID3D11Device* Device, const D3D11_BUFFER_DESC& Desc)
	:FBuffer(Device, Desc)
{
}

FVertexBuffer::FVertexBuffer(ID3D11Device* Device, const D3D11_BUFFER_DESC& Desc, uint32 InStride, const void* InitialData)
	:FBuffer(Device, Desc, InitialData)
{
	Stride = InStride;
}

FIndexBuffer::FIndexBuffer(ID3D11Device* Device, const D3D11_BUFFER_DESC& Desc, uint32 Count, const void* InitialData, DXGI_FORMAT InFormat)
	:FBuffer(Device, Desc, InitialData), Format(InFormat)
{
	IndexCount = Count;
}

FStructuredBuffer::FStructuredBuffer(ID3D11Device* Device, const D3D11_BUFFER_DESC& Desc, const void* InitialData)
	: FBuffer(Device, Desc, InitialData)
	, Stride(Desc.StructureByteStride)
	, Capacity(Desc.ByteWidth / Desc.StructureByteStride)
{
	if (!Buffer.Get())
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
		.Buffer = {
			.FirstElement = 0,
			.NumElements = Capacity
		}
	};

	const HRESULT hr = Device->CreateShaderResourceView(
		Buffer.Get(), &SRVDesc, SRV.GetAddressOf());

	if (FAILED(hr))
	{
		HTR_LOG(Warning, "Failed To Create Structured Buffer SRV!");
	}

}