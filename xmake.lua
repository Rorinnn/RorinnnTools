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
add_requires("botan", {configs = {minimal = false}})

local ProjectDir = os.scriptdir()
local GeneratedDir = path.join(ProjectDir, "Generated")

target("RorinnnTools")
    set_kind("static")
    set_languages("cxx20")
    set_policy("build.c++.modules", true)
    add_rules("utils.bin2obj")
    add_defines("NOMINMAX", {public = true})
    add_files("src/**.ixx", {public = true})
    add_files("src/**.cpp")
    add_packages("imgui", {public = true})
    add_packages("botan")
    add_syslinks("kernel32", "user32", "advapi32", "psapi", "ole32", "wbemuuid", "windowscodecs", "d3dcompiler")
    on_load(function (target)
        local ResourceDir = path.join(GeneratedDir, "ImguiRorinnn/Resources")
        local SolidBin = path.join(ResourceDir, "FontAwesomeSolid.bin")
        local BrandsBin = path.join(ResourceDir, "FontAwesomeBrands.bin")
        os.mkdir(ResourceDir)
        os.cp(path.join(ProjectDir, "assets/ImguiRorinnn/Fonts/fa-solid-900.ttf"), SolidBin)
        os.cp(path.join(ProjectDir, "assets/ImguiRorinnn/Fonts/fa-brands-400.ttf"), BrandsBin)
        target:add("files", SolidBin)
        target:add("files", BrandsBin)
    end)
