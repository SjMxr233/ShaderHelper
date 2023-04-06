FrameWorkHierarchy = {
    ["Sources/*"] = {"**.h","**.cpp", "**.hpp"},
}

project "FrameWork"
    kind "SharedLib"   
    location "%{_WORKING_DIR}/ProjectFiles"

    vpaths(FrameWorkHierarchy)

    files {seq(FrameWorkHierarchy)}

    includedirs
    {
        "./",
    }

	defines {
		"FRAMEWORK_API=DLLEXPORT"
	}

    uses {
        "UE"
    }

    private_uses {
        "d3dx12", "D3D12MemoryAllocator",
        "DXC", "AgilitySDK", "WinPixEventRuntime"
    }

    links {
        "d3d12", "dxgi", "dxguid"
    }

    filter "system:windows"
        pchheader "CommonHeader.h"
        pchsource "CommonHeader.cpp"

usage "FrameWork"
	defines {
		"FRAMEWORK_API=DLLIMPORT"
	}
	includedirs
    {
        path.getabsolute("./"),
    }



        

