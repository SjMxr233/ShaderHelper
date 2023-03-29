#include "CommonHeader.h"
#include "D3D12Texture.h"
#include "D3D12Device.h"
#include "D3D12CommandList.h"
namespace FRAMEWORK
{
	struct FlagSets {
		bool bShared;
		bool bRTV;
		bool bSRV;
	};
	static bool ValidateTexture(const GpuTextureDesc& InTexDesc)
	{
		if (InTexDesc.Width <= 0 || InTexDesc.Height <= 0) {
			SH_LOG(LogDx12, Error, TEXT("Invalid Texture dimensions %ux%u"), InTexDesc.Width, InTexDesc.Height);
			return false;
		}

		if (InTexDesc.NumMips != 1) {
			SH_LOG(LogDx12, Error, TEXT("Invalid Texture MipLevels. TODO: Support texture subresource"));
			return false;
		}

		if (InTexDesc.Depth > 1) {
			SH_LOG(LogDx12, Error, TEXT("Invalid Texture Depth. TODO: Support 3d texture"));
			return false;
		}
		
		return true;
	}

	static void GetDx12ResourceFlags(GpuTextureUsage InFlags, D3D12_RESOURCE_FLAGS& OutResourceFlag, FlagSets& OutFlags)
	{
		if (EnumHasAnyFlags(InFlags, GpuTextureUsage::RenderTarget)) {
			OutResourceFlag |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			OutFlags.bRTV = true;
		}

		if (EnumHasAnyFlags(InFlags, GpuTextureUsage::Shared)) {
			OutResourceFlag |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
			OutFlags.bShared = true;
		}

		if (!EnumHasAnyFlags(OutResourceFlag, D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
			OutFlags.bSRV = true;
		}
	}

	static void GetInitialResourceState(const FlagSets& InFlags, D3D12_RESOURCE_STATES& OutResourceState) 
	{
		
		bool bWritable = InFlags.bRTV;
		if (InFlags.bSRV && !bWritable) {
			OutResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}

		if (InFlags.bRTV) {
			OutResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}
		
	}

	static void CreateTextureView(const FlagSets& InFlags, Dx12Texture* InTexture)
	{
		auto& DescriptorAllocators = GCommandListContext->GetCurFrameResource().GetDescriptorAllocators();
		D3D12_CPU_DESCRIPTOR_HANDLE Hanlde{};
		if (InFlags.bSRV) {
			InTexture->HandleSRV = DescriptorAllocators.SrvAllocator->Allocate();
			GDevice->CreateShaderResourceView(InTexture->GetResource(), nullptr, InTexture->HandleSRV.CpuHandle);
		}

		if (InFlags.bRTV) {
			InTexture->HandleRTV = DescriptorAllocators.RtvAllocator->Allocate();
			GDevice->CreateRenderTargetView(InTexture->GetResource(), nullptr, InTexture->HandleRTV.CpuHandle);
		}
	}

	TRefCountPtr<Dx12Texture> CreateDx12Texture(const GpuTextureDesc& InTexDesc)
	{
		if (!ValidateTexture(InTexDesc)) {
			return nullptr;
		}

		D3D12_RESOURCE_FLAGS ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
		
		FlagSets Flags{};
		GetDx12ResourceFlags(InTexDesc.Usage, ResourceFlags, Flags);

		bool bHasInitialData = false;
		if (!InTexDesc.InitialData.IsEmpty()) {
			bHasInitialData = true;
		}

		CD3DX12_RESOURCE_DESC TexDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			MapTextureFormat(InTexDesc.Format),
			InTexDesc.Width,
			InTexDesc.Height,
			InTexDesc.Depth,
			InTexDesc.NumMips,
			1,0,
			ResourceFlags
		);

		CD3DX12_HEAP_PROPERTIES HeapType{ D3D12_HEAP_TYPE_DEFAULT };
		D3D12_HEAP_FLAGS HeapFlag = Flags.bShared ? D3D12_HEAP_FLAG_SHARED : D3D12_HEAP_FLAG_NONE;

		D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;
		GetInitialResourceState(Flags, InitialState);
		//If have initial data, we need to set state to COPY_DEST.
		//Don't call MapGpuTexture()/UnMapGpuTexture() to avoid generating extra barrier.
		D3D12_RESOURCE_STATES ActualState = bHasInitialData ? D3D12_RESOURCE_STATE_COPY_DEST : InitialState;

		//Fast Clear Optimization
		const float ClearColor[4] = { InTexDesc.ClearValues.X, InTexDesc.ClearValues.Y, InTexDesc.ClearValues.Z, InTexDesc.ClearValues.W };
		CD3DX12_CLEAR_VALUE ClearValues{ MapTextureFormat(InTexDesc.Format), ClearColor };

		TRefCountPtr<ID3D12Resource> TexResource;
		DxCheck(GDevice->CreateCommittedResource(&HeapType, HeapFlag,
			&TexDesc, ActualState, &ClearValues, IID_PPV_ARGS(TexResource.GetInitReference())));

		if (bHasInitialData) {
			const uint64 UploadBufferSize = GetRequiredIntermediateSize(TexResource, 0, 1);
			TRefCountPtr<Dx12UploadBuffer> UploadBuffer = new Dx12UploadBuffer(UploadBufferSize);
			
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = &InTexDesc.InitialData[0];
			textureData.RowPitch = InTexDesc.Width * GetFormatByteSize(InTexDesc.Format);
			textureData.SlicePitch = textureData.RowPitch * InTexDesc.Height;

			UpdateSubresources(GCommandListContext->GetCommandListHandle(), TexResource, UploadBuffer->GetResource(), 0, 0, 1, &textureData);
			GCommandListContext->Transition(TexResource, ActualState, InitialState);
		}
		
		TRefCountPtr<Dx12Texture> RetTexture = new Dx12Texture{ InitialState, TexResource };
		CreateTextureView(Flags, RetTexture);

		return RetTexture;
	}

	DXGI_FORMAT MapTextureFormat(GpuTextureFormat InTexFormat)
	{
		switch (InTexFormat)
		{
		case GpuTextureFormat::R8G8B8A8_UNORM:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GpuTextureFormat::R10G10B10A2_UNORM:	return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GpuTextureFormat::R16G16B16A16_UNORM:	return DXGI_FORMAT_R16G16B16A16_UNORM;
		case GpuTextureFormat::R16G16B16A16_UINT:	return DXGI_FORMAT_R16G16B16A16_UINT;
		case GpuTextureFormat::R32G32B32A32_UINT:	return DXGI_FORMAT_R32G32B32A32_UINT;
		case GpuTextureFormat::R16G16B16A16_FLOAT:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case GpuTextureFormat::R32G32B32A32_FLOAT:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case GpuTextureFormat::R11G11B10_FLOAT:		return DXGI_FORMAT_R11G11B10_FLOAT;
		default:
			SH_LOG(LogDx12, Fatal, TEXT("Invalid GpuTextureFormat."));
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
}