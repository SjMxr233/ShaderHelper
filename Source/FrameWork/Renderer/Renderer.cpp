#include "CommonHeader.h"
#include "Renderer.h"
#include "GpuApi/GpuRhi.h"

namespace FW
{

	Renderer::Renderer()
	{

	}

    void Renderer::Render()
    {
        RenderBegin();
        
        RenderInternal();
        
        RenderEnd();
    }

    void Renderer::RenderBegin()
    {
        
    }

    void Renderer::RenderEnd()
    {
        
    }

}


