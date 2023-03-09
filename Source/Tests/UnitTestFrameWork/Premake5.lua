UnitTestFrameWorkHierarchy = {
    ["Sources/*"] = {"**.h","**.cpp"},
}

project "UnitTestFrameWork"
    kind "WindowedApp"   
    location "%{_WORKING_DIR}/ProjectFiles"

    vpaths(UnitTestFrameWorkHierarchy)

    files {seq(UnitTestFrameWorkHierarchy)}

    includedirs
    {
        "./",
    }

    uses {
		"UE",
		"Framework",
	}

    filter "system:windows"
        files {"%{_WORKING_DIR}/Resource/Misc/Windows/*"}
        vpaths {["Resources"] = "%{_WORKING_DIR}/Resource/Misc/Windows/*"}
        pchheader "CommonHeader.h"
        pchsource "CommonHeader.cpp"

    filter "system:macosx"
        files {"%{_WORKING_DIR}/Resource/Misc/Mac/*"}
        vpaths {["Resources"] = "%{_WORKING_DIR}/Resource/Misc/Mac/*"}



        
