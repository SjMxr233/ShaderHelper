#pragma once
#include "MetalCommon.h"
#include "GpuApi/GpuResource.h"
#include "MetalBuffer.h"

namespace FRAMEWORK
{
	class MetalTexture : public GpuTexture
	{
    public:
        MetalTexture(mtlpp::Texture InTex, GpuTextureDesc InDesc)
            : GpuTexture(MoveTemp(InDesc))
            , Tex(MoveTemp(InTex))
        {}
        
    public:
        id<MTLTexture> GetResource() const {
            return Tex.GetPtr();
        }
        
    public:
        TRefCountPtr<MetalBuffer> ReadBackBuffer;
        
    private:
        mtlpp::Texture Tex;
	};

    TRefCountPtr<MetalTexture> CreateMetalTexture2D(const GpuTextureDesc& InTexDesc);
}
