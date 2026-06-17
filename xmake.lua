add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_cxflags("/utf-8")

if is_mode("debug") then
    set_runtimes("MTd")
else
    set_runtimes("MT")
end

set_targetdir("build/$(mode)")

add_requires("imgui", {configs = {dx11 = true, dx12 = true, win32 = true}})
add_requires("zlib")

local ProjectDir = os.scriptdir()
local GeneratedDir = path.join(ProjectDir, "Generated")

target("RorinnnTools")
    set_kind("static")
    set_languages("cxx20")
    set_policy("build.c++.modules", true)
    add_defines("NOMINMAX", {public = true})
    add_files("src/RorinnnTools.ixx", {public = true})
    add_files("src/**.cpp")
    add_packages("imgui", {public = true})
    add_packages("zlib", {public = true})
    add_syslinks("kernel32", "user32", "advapi32", "psapi", "ole32", "wbemuuid", "bcrypt", "windowscodecs", "d3dcompiler", "crypt32")
    on_load(function (target)
        os.execv("python", {
            path.join(ProjectDir, "GenerateBinaryResource.py"),
            path.join(ProjectDir, "assets/ImguiRorinnn/Fonts/fa-solid-900.ttf"),
            path.join(GeneratedDir, "ImguiRorinnn/Resources/FontAwesomeSolidResource.hpp"),
            "RorinnnTools::ImguiRorinnn::Resources",
            "FontAwesomeSolidData",
            "--module",
            "RorinnnTools"
        })
        os.execv("python", {
            path.join(ProjectDir, "GenerateBinaryResource.py"),
            path.join(ProjectDir, "assets/ImguiRorinnn/Fonts/fa-brands-400.ttf"),
            path.join(GeneratedDir, "ImguiRorinnn/Resources/FontAwesomeBrandsResource.hpp"),
            "RorinnnTools::ImguiRorinnn::Resources",
            "FontAwesomeBrandsData",
            "--module",
            "RorinnnTools"
        })
        target:add("files", path.join(GeneratedDir, "ImguiRorinnn/Resources/FontAwesomeSolidResource.cpp"))
        target:add("files", path.join(GeneratedDir, "ImguiRorinnn/Resources/FontAwesomeBrandsResource.cpp"))
    end)
