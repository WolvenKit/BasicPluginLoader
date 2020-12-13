systemversion "10.0.18362.0"
toolset "v142"
platforms { "x64" }

workspace "CP77Fun"
    configurations { "Debug", "Release" }

    location "build"

    startproject "Loader"

    symbols "On"
    staticruntime "On"

    filter "configurations:Debug"
        targetdir "bin/debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        targetdir "bin/release"
        defines { "NDEBUG" }
        optimize "Size"

    filter {}

    includedirs
    {
        "include/"
    }

    editandcontinue "Off"

group "Projects"
    project "Loader"
        buildoptions '/std:c++latest'
        kind "SharedLib"
        language "C++"
        targetname "version"

        prebuildcommands
        {
            "call $(SolutionDir)..\\Tools\\gitrev.cmd"
        }

        staticruntime "On"

        includedirs
        {
            "vendor/injector/include/",
            "vendor/minhook/include/"
        }

        links
        {
            "version",
            "dbghelp"
        }

        defines { }

        files
        {
            "src/loader/**.*"
        }

        postbuildcommands
        {
            "if \"%COMPUTERNAME%\" == \"GENSOKYO\" ( copy /y \"$(TargetPath)\" \"G:\\Games\\Cyberpunk 2077\\bin\\x64\\version.dll\" )",
        }

    project "Experiments"
        buildoptions '/std:c++latest'
        kind "SharedLib"
        language "C++"
        targetname "Experiments"

        staticruntime "On"

        includedirs
        {
            "vendor/injector/include/",
            "vendor/minhook/include/"
        }

        links
        {
            "MinHook"
        }

        defines { }

        files
        {
            "src/experiments/**.*"
        }

        postbuildcommands
        {
            "if \"%COMPUTERNAME%\" == \"GENSOKYO\" ( copy /y \"$(TargetPath)\" \"G:\\Games\\Cyberpunk 2077\\bin\\x64\\plugins\\Experiments.dll\" )"
        }

group "Vendor"
    project "MinHook"
        targetname "MinHook"
        language "C"
        kind "StaticLib"
        staticruntime "On"
        buildoptions "/MT"

        files
        {
            "vendor/minhook/src/buffer.c",
            "vendor/minhook/src/hook.c",
            "vendor/minhook/src/trampoline.c",
            "vendor/minhook/src/hde/hde64.c"
        }