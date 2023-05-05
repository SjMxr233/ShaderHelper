#include "CommonHeader.h"
#include "MetalShader.h"
#include "MetalDevice.h"

namespace FRAMEWORK
{
    bool CompileShader(TRefCountPtr<MetalShader> InShader)
    {
        ns::AutoReleasedError Err;
        mtlpp::CompileOptions CompOpt;
        mtlpp::Library ByteCodeLib = GDevice.NewLibrary(TCHAR_TO_ANSI(*InShader->GetSourceText()), CompOpt, &Err);
        
        if(!ByteCodeLib) {
            SH_LOG(LogMetal, Error, TEXT("Shader compilation failed: %s"), ConvertOcError(Err.GetPtr()));
        }
        
        mtlpp::Function ByteCodeFunc = ByteCodeLib.NewFunction(TCHAR_TO_ANSI(*InShader->GetEntryPoint()));
        InShader->SetCompilationResult(MoveTemp(ByteCodeFunc));
    }
}