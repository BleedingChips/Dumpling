if is_plat("windows") then
    add_requires("imgui", {configs={win32=true, dx12=true}})
end

target("DumplingImGui")
    set_kind("static")
    add_files("*.cpp")
    add_files("*.ixx", {public=true})
    if is_plat("windows") then
        add_files("Platform/Windows/*.ixx", {public=true})
        add_files("Platform/Windows/*.cpp")
        add_files("Platform/Windows/Dx12/*.ixx", {public=true})
        add_files("Platform/Windows/Dx12/*.cpp")
    end
    add_deps("Dumpling")
    add_packages("imgui", {public=true})
target_end()