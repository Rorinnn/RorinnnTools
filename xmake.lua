add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_cxflags("/utf-8")

if is_mode("debug") then
    set_runtimes("MTd")
else
    set_runtimes("MT")
end

set_targetdir("build/$(mode)")

add_requires("imgui", {configs = {dx11 = true, win32 = true}})

target("RorinnnTools")
    set_kind("static")
    set_languages("cxx20")
    add_includedirs("inc", {public = true})
    add_files("src/**.cpp")
    add_packages("imgui", {public = true})
    add_syslinks("kernel32", "user32", "advapi32", "psapi", "ole32", "wbemuuid", "bcrypt", "windowscodecs", "d3dcompiler")
