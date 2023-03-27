#include "CommonHeader.h"
#include "GpuApi/GpuApiInterface.h"
#include "D3D12Device.h"
#include "D3D12CommandList.h"
#include "D3D12Texture.h"
#include "D3D12Shader.h"
#include "D3D12PSO.h"

namespace FRAMEWORK
{
namespace GpuApi
{
	void InitApiEnv()
	{
		InitDx12Core();
		InitFrameResource();
	}

	void FlushGpu()
	{
		
	}

	void StartRenderFrame()
	{
		uint32 FrameResourceIndex = GetCurFrameSourceIndex();
		GCommandListContext->BindFrameResource(FrameResourceIndex);
	}

	void EndRenderFrame()
	{
		check(CurCpuFrame >= CurCpuFrame);
		CurCpuFrame++;
		DxCheck(GGraphicsQueue->Signal(CpuSyncGpuFence, CurCpuFrame));
		const uint64 CurLag = CurCpuFrame - CurGpuFrame;
		if (CurLag > AllowableLag) {
			//Cpu is waiting for gpu to catch up a frame.
			const uint64 FenceValue = CpuSyncGpuFence->GetCompletedValue();
			if (FenceValue < CurGpuFrame + 1) {
				DxCheck(CpuSyncGpuFence->SetEventOnCompletion(CurGpuFrame + 1, CpuSyncGpuEvent));
				WaitForSingleObject(CpuSyncGpuEvent, INFINITE);
			}
			CurGpuFrame++;
		}

		//FrameResource
		uint32 FrameResourceIndex = GetCurFrameSourceIndex();
		GCommandListContext->ResetFrameResource(FrameResourceIndex);
	}

	TRefCountPtr<GpuTexture> CreateGpuTexture(const GpuTextureDesc& InTexDesc)
	{
		return AUX::StaticCastRefCountPtr<GpuTexture>(CreateDx12Texture(InTexDesc));
	}

	void* MapGpuTexture(TRefCountPtr<GpuTexture> InGpuTexture, GpuResourceMapMode InMapMode)
	{
		TRefCountPtr<Dx12Texture> Texture = AUX::StaticCastRefCountPtr<Dx12Texture>(InGpuTexture);
		void* Data{};
		if (InMapMode == GpuResourceMapMode::Write_Only) {
			const uint64 UploadBufferSize = GetRequiredIntermediateSize(Texture->GetResource(), 0, 1);
			Texture->UploadBuffer = new Dx12UploadBuffer(UploadBufferSize);
			Data = Texture->UploadBuffer->Map();
			Texture->bIsMappingForWriting = true;
		}
		return Data;
	}

	void UnMapGpuTexture(TRefCountPtr<GpuTexture> InGpuTexture)
	{
		TRefCountPtr<Dx12Texture> Texture = AUX::StaticCastRefCountPtr<Dx12Texture>(InGpuTexture);
		if (Texture->bIsMappingForWriting) {
			Texture->UploadBuffer->Unmap();
			ScopedBarrier Barrier{ Texture, D3D12_RESOURCE_STATE_COPY_DEST };
			ID3D12GraphicsCommandList* CommandListHandle = GCommandListContext->GetCommandListHandle();

			CD3DX12_TEXTURE_COPY_LOCATION DestLoc{ Texture->GetResource() };
			CD3DX12_TEXTURE_COPY_LOCATION SrcLoc{ Texture->UploadBuffer->GetResource() };
			CommandListHandle->CopyTextureRegion(&DestLoc, 0, 0, 0, &SrcLoc,nullptr);
				
			Texture->bIsMappingForWriting = false;
		}
	}

	TRefCountPtr<GpuShader> CreateShaderFromSource(ShaderType InType, FString InSourceText, FString InShaderName)
	{
		return new Dx12Shader(InType, MoveTemp(InSourceText), MoveTemp(InShaderName));
	}

	bool CompilerShader(TRefCountPtr<GpuShader> InShader)
	{
		TRefCountPtr<Dx12Shader> Shader = AUX::StaticCastRefCountPtr<Dx12Shader>(InShader);
		return GShaderCompiler.Compile(Shader);
	}


	TRefCountPtr<RenderPipelineState> CreateRenderPipelineState(const PipelineStateDesc& InPipelineStateDesc)
	{
		CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		TRefCountPtr<ID3DBlob> Signature;
		TRefCountPtr<ID3DBlob> Error;
		TRefCountPtr<ID3D12RootSignature> RootSignature;
		DxCheck(D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Signature.GetInitReference(), Error.GetInitReference()));
		DxCheck(GDevice->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(RootSignature.GetInitReference())));
		//TODO RootSignature Manager.

		TRefCountPtr<Dx12Shader> Vs = AUX::StaticCastRefCountPtr<Dx12Shader>(InPipelineStateDesc.Vs);
		TRefCountPtr<Dx12Shader> Ps = AUX::StaticCastRefCountPtr<Dx12Shader>(InPipelineStateDesc.Ps);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc{};
		PsoDesc.pRootSignature = RootSignature;
		PsoDesc.VS = D3D12_SHADER_BYTECODE{ Vs->GetCompilationResult()->GetBufferPointer(), Vs->GetCompilationResult()->GetBufferSize() };
		PsoDesc.PS = D3D12_SHADER_BYTECODE{ Ps->GetCompilationResult()->GetBufferPointer(), Ps->GetCompilationResult()->GetBufferSize() };
		PsoDesc.RasterizerState = MapRasterizerState(InPipelineStateDesc.RasterizerState);
		PsoDesc.BlendState = MapBlendState(InPipelineStateDesc.BlendState);
		PsoDesc.DepthStencilState.DepthEnable = false;
		PsoDesc.DepthStencilState.StencilEnable = false;
		PsoDesc.SampleMask = UINT_MAX;
		PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PsoDesc.NumRenderTargets = 1;
		PsoDesc.RTVFormats[0] = MapTextureFormat(InPipelineStateDesc.RtFormat);
		PsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		PsoDesc.SampleDesc.Count = 1;
		PsoDesc.SampleDesc.Quality = 0;

		TRefCountPtr<ID3D12PipelineState> Pso;
		DxCheck(GDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(Pso.GetInitReference())));
		return new Dx12Pso(MoveTemp(Pso));
	}

	void SetRenderPipelineState(TRefCountPtr<RenderPipelineState> InPipelineState)
	{
		TRefCountPtr<Dx12Pso> Pso = AUX::StaticCastRefCountPtr<Dx12Pso>(InPipelineState);
		GCommandListContext->GetCommandListHandle()->SetPipelineState(Pso->GetResource());
	}
}
}