workspace "V-Shade"
	architecture "x64"

	configurations {
		"Debug",
		"Release"
	}

output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" 
includeDir = {}
includeDir["GLFW"]   	= "%{wks.name}/vendors/glfw/include"
includeDir["SPDLOG"]  	= "%{wks.name}/vendors/spdlog/include"
includeDir["GLM"]  		= "%{wks.name}/vendors/glm/glm"
includeDir["IMGUI"]  	= "%{wks.name}/vendors/ImGui"
includeDir["IMGUIZMO"]  = "%{wks.name}/vendors/ImGuizmo"
includeDir["VULKAN"]  	= "$(VK_SDK_PATH)/include/"

--Include path for premake project
include "V-Shade/vendors/glfw"
include "V-Shade/vendors/ImGui"
include "Editor/vendors/assimp"

group "Engine"
project "V-Shade"
	location	"V-Shade"
	kind		"SharedLib"
	language	"C++"

	targetdir ("bin/" .. output_dir .. "/%{prj.name}")
	objdir    ("bin-int/" .. output_dir .. "/%{prj.name}")

	
	pchheader "shade_pch.h"
	pchsource "V-Shade/include/shade_pch.cpp"
	
	files {
		"%{prj.name}/include/**.h",
		"%{prj.name}/include/**.cpp",
		"%{prj.name}/vendors/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendors/ImGuizmo/ImGuizmo.cpp"
	}
	
	includedirs {
		"%{prj.name}/include",
		"%{prj.name}/vendors",
		"%{includeDir.GLFW}",
		"%{includeDir.GLM}",
		"%{includeDir.IMGUI}",
		"%{includeDir.IMGUIZMO}",
		"%{includeDir.VULKAN}",
		"%{includeDir.SPDLOG}",
	}
	
	libdirs {
		"$(VK_SDK_PATH)/Lib"
	}

	links {
		"glfw",
		"vulkan-1.lib",
		"ImGui"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"

	defines {
			"SHADE_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"SHADE_WINDOWS_PLATFORM",
			"GLM_FORCE_SSE2",
			"GLM_FORCE_AVX",
			"GLM_FORCE_DEPTH_ZERO_TO_ONE",
			"SPDLOG_USE_STD_FORMAT"
	}

	postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. output_dir .. "/Editor")
	}

	filter "configurations:Debug"
		defines "SHADE_DEBUG"
		symbols "On"
		links {
			"shaderc_sharedd.lib",
			"spirv-cross-glsld.lib",
			"spirv-cross-cored.lib",
		}
		
	filter "configurations:Release"
		defines "SHADE_RELEASE"
		optimize "On"
		links {
			"shaderc_shared.lib",
			"spirv-cross-glsl.lib",
			"spirv-cross-core.lib",
		}
		
group "Clients"	
project "Editor"
	location	"Editor"
	language	"C++"

	targetdir ("bin/" .. output_dir .. "/%{prj.name}")
	objdir    ("bin-int/" .. output_dir .. "/%{prj.name}")

	files {
		"%{prj.name}/include/**.h",
		"%{prj.name}/include/**.cpp"
	}

	includedirs {
		"%{includeDir.SPDLOG}",
		"%{includeDir.GLM}",
		"%{prj.name}/include",
		"%{prj.name}/include/vendors",
		"%{prj.name}/vendors/assimp/include",
		"%{wks.name}/include",
		"%{wks.name}/vendors",
	}

	links {
		"V-Shade",
		"ImGui",
		"assimp"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"

		defines {
			"SHADE_WINDOWS_PLATFORM",
			"GLM_FORCE_SSE2",
			"GLM_FORCE_AVX",
			"GLM_FORCE_DEPTH_ZERO_TO_ONE",
			"SPDLOG_USE_STD_FORMAT"
		}

	filter "configurations:Debug"
		defines "SHADE_DEBUG"
		symbols "On"
		kind	"ConsoleApp"
		linkoptions '/ENTRY:"mainCRTStartup"'

	filter "configurations:Release"
		defines "SHADE_RELAESE"
		optimize "On"
		kind	 "WindowedApp"
		linkoptions '/ENTRY:"mainCRTStartup"'
		group "Clients/Scripts"			

project "Scripts"
	location	"Editor/Scripts"
	kind		"SharedLib"
	language	"C++"

	targetdir ("bin/" .. output_dir .. "/%{prj.name}")
	objdir    ("bin-int/" .. output_dir .. "/%{prj.name}")

	files {
		"Editor/%{prj.name}/**.h",
		"Editor/%{prj.name}/**.cpp",
	}

	includedirs {
		"%{includeDir.SPDLOG}",
		"%{includeDir.GLM}",
		"%{wks.name}/include",
		"%{wks.name}/vendors",
	}

	defines {
		"GLM_FORCE_SSE2",
		"GLM_FORCE_AVX",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"SPDLOG_USE_STD_FORMAT"
	}

	links {
		"V-Shade"
	}
	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SHADE_DEBUG"
		symbols "On"

	postbuildcommands {
		"{COPY} %{cfg.targetdir}/Scripts.dll ../resources/scripts/"
	}

	filter "configurations:Release"
		defines "SHADE_RELAESE"
		optimize "On"

	postbuildcommands {
		"{COPY} %{cfg.targetdir}/Scripts.dll ../resources/scripts/"
	}

