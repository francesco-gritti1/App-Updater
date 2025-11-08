



-- premake5.lua
workspace "App-Updater"
   configurations { "Debug", "Release" }



project "App-Updaer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "!bin/obj/%{cfg.buildcfg}"

    files { 
        "src/main.cpp",
        "src/utils.cpp",
    }

    includedirs {
    }

    buildoptions {
        "$$(pkg-config --cflags gtk+-3.0 libconfig++ json-glib-1.0)",
        "$$(pkg-config --cflags libconfig++)",
        "-Wall",
        --"-Werror",
        --"-pedantic"
    }

    linkoptions {
        "$$(pkg-config --libs gtk+-3.0 libconfig++ json-glib-1.0)",
        "$$(pkg-config --libs libconfig++)",
        "-rdynamic"
    }

    links {
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"





    project "ui"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++23"
        targetdir "bin/%{cfg.buildcfg}"
        objdir "!bin/obj/%{cfg.buildcfg}"
    
        files { 
            "src/main_graphics.cpp",
            "src/utils.cpp",
        }
    
        includedirs {
        }
    
        buildoptions {
            "$$(pkg-config --cflags gtk+-3.0 libconfig++ json-glib-1.0)",
            "$$(pkg-config --cflags libconfig++)",
            "-Wall",
            --"-Werror",
            --"-pedantic"
        }
    
        linkoptions {
            "$$(pkg-config --libs gtk+-3.0 libconfig++ json-glib-1.0)",
            "$$(pkg-config --libs libconfig++)",
            "-rdynamic"
        }
    
        links {
        }
    
        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"
    
        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"
    




