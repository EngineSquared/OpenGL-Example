add_requires("entt", "gtest", "glm >=1.0.1", "glfw >=3.4", "glew", "spdlog", "fmt", "stb")

set_project("ESGL-App")
set_languages("c++20")

add_rules("mode.debug", "mode.release")

includes("../EngineSquared/xmake.lua")

-- add /W4 for windows
if is_plat("windows") then
    add_cxflags("/W4")
end

target("ESGL-App")
    set_kind("binary")
    set_default(true)
    add_deps("EngineSquared")

    add_files("src/**.cpp")
    add_includedirs("$(projectdir)/src/")

    add_packages("entt", "glm", "glfw", "glew", "spdlog", "fmt", "stb")

    set_rundir("$(projectdir)")


if is_mode("debug") then
    add_defines("ES_DEBUG")
    set_symbols("debug")
    set_optimize("none")
end

if is_mode("release") then
    set_optimize("fastest")
end