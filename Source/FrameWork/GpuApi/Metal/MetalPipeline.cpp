#include "CommonHeader.h"
#include "MetalPipeline.h"
#include "MetalShader.h"
#include "MetalMap.h"
#include "MetalDevice.h"

namespace FRAMEWORK
{
	TRefCountPtr<MetalRenderPipelineState> CreateMetalRenderPipelineState(const GpuRenderPipelineStateDesc& InPipelineStateDesc)
    {
        MetalShader* Vs = static_cast<MetalShader*>(InPipelineStateDesc.Vs);
        MetalShader* Ps = static_cast<MetalShader*>(InPipelineStateDesc.Ps);
        
        MTLRenderPipelineDescriptorPtr PipelineDesc = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
        PipelineDesc->setVertexFunction(Vs->GetCompilationResult());
		if (Ps) {
			PipelineDesc->setFragmentFunction(Ps->GetCompilationResult());
		}
        
        MTL::RenderPipelineColorAttachmentDescriptorArray* ColorAttachments = PipelineDesc->colorAttachments();
        for(uint32 i = 0; i < InPipelineStateDesc.Targets.Num(); i++)
        {
            const PipelineTargetDesc& Target = InPipelineStateDesc.Targets[i];

            MTL::RenderPipelineColorAttachmentDescriptor* ColorAttachment = ColorAttachments->object(i);
            ColorAttachment->setBlendingEnabled(Target.BlendEnable);
            ColorAttachment->setSourceRGBBlendFactor((MTL::BlendFactor)MapBlendFactor(Target.SrcFactor));
            ColorAttachment->setSourceAlphaBlendFactor((MTL::BlendFactor)MapBlendFactor(Target.SrcAlphaFactor));
            ColorAttachment->setDestinationRGBBlendFactor((MTL::BlendFactor)MapBlendFactor(Target.DestFactor));
            ColorAttachment->setDestinationAlphaBlendFactor((MTL::BlendFactor)MapBlendFactor(Target.DestAlphaFactor));
            ColorAttachment->setRgbBlendOperation((MTL::BlendOperation)MapBlendOp(Target.ColorOp));
            ColorAttachment->setAlphaBlendOperation((MTL::BlendOperation)MapBlendOp(Target.AlphaOp));
            ColorAttachment->setWriteMask(MapWriteMask(Target.Mask));
            ColorAttachment->setPixelFormat((MTL::PixelFormat)MapTextureFormat(Target.TargetFormat));
        }

        NS::Error* err = nullptr;
        MTLRenderPipelineStatePtr PipelineState = NS::TransferPtr(GDevice->newRenderPipelineState(PipelineDesc.get(), MTL::PipelineOptionNone ,nullptr ,&err));
        if (!PipelineState)
        {
            SH_LOG(LogMetal, Fatal, TEXT("Failed to create render pipeline: %s"), *NSStringToFString(err->localizedDescription()));
            //TDOO: fallback to default pipeline.
        }
        
        return new MetalRenderPipelineState(MoveTemp(PipelineState), MapPrimitiveType(InPipelineStateDesc.Primitive));
    }
}
