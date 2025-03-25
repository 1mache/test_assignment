workspace "PPMRayTracer"
	configurations { "Debug", "Release" }
	platforms { "Win32","Win64" }  

project "RayTracer"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	files {
		"*.h",
		"*.cpp",
	}

    vpaths {
        ["Header Files"] = { "*.h" },
        ["Source Files"] = { "*.cpp" },
    }

	filter ("configurations:Debug")
		defines{"DEBUG"}
		targetdir "bin/debug"

	filter ("configurations:Release")
		optimize "On"
		targetdir "bin/release"