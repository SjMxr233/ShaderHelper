#pragma once
#include "GpuResourceCommon.h"
#include "GpuShader.h"

namespace FRAMEWORK
{
    struct RasterizerStateDesc
    {
        RasterizerStateDesc(RasterizerFillMode InFillMode, RasterizerCullMode InCullMode)
            : FillMode(InFillMode)
            , CullMode(InCullMode)
        {}

        RasterizerFillMode FillMode;
        RasterizerCullMode CullMode;
    };

    struct BlendRenderTargetDesc
    {
        BlendRenderTargetDesc(
            bool InBlendEnable,
            BlendFactor InSrcFactor, BlendFactor InDestFactor,
            BlendFactor InSrcAlphaFactor, BlendFactor InDestAlphaFactor,
            BlendOp InColorOp, BlendOp InAlphaOp, BlendMask InMask = BlendMask::All)
            : BlendEnable(InBlendEnable)
            , SrcFactor(InSrcFactor)
            , DestFactor(InDestFactor)
            , SrcAlphaFactor(InSrcAlphaFactor)
            , DestAlphaFactor(InDestAlphaFactor)
            , Mask(InMask)
            , ColorOp(InColorOp)
            , AlphaOp(InAlphaOp)
        {
        }
        bool BlendEnable;
        BlendFactor SrcFactor;
        BlendFactor DestFactor;
        BlendFactor SrcAlphaFactor;
        BlendFactor DestAlphaFactor;
        BlendMask Mask;
        BlendOp ColorOp;
        BlendOp AlphaOp;
    };

    struct BlendStateDesc
    {
        using DescStorageType = TArray<BlendRenderTargetDesc, TFixedAllocator<GpuResourceLimit::MaxRenderTargetNum>>;
        BlendStateDesc() = default;
        BlendStateDesc(DescStorageType InDescStorage) : RtDescs(MoveTemp(InDescStorage)) {}
        DescStorageType RtDescs;
    };

    struct PipelineStateDesc
    {
        using RtFormatStorageType = TArray<GpuTextureFormat, TFixedAllocator<GpuResourceLimit::MaxRenderTargetNum>>;
        PipelineStateDesc(
            GpuShader* InVs, GpuShader* InPs,
            RasterizerStateDesc InRasterizerState, BlendStateDesc InBlendState,
            RtFormatStorageType InRtFormats)
            : Vs(InVs)
            , Ps(InPs)
            , RasterizerState(MoveTemp(InRasterizerState))
            , BlendState(MoveTemp(InBlendState))
            , RtFormats(MoveTemp(InRtFormats))
        {
        }
        GpuShader* Vs;
        GpuShader* Ps;
        RasterizerStateDesc RasterizerState;
        BlendStateDesc BlendState;
        RtFormatStorageType RtFormats;
    };

    class GpuPipelineState : public GpuResource
    {
        
    };
}

