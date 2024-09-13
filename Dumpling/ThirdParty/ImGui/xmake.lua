if is_plat("windows") then
    add_requires("imgui", {configs={win32=true, dx12=true}})
end

target("DumplingImGui")
    set_kind("static")
    add_files("*.cpp")
    add_files("*.ixx")
    if is_plat("windows") then
        add_files("Platform/Windows/Dx12/*.ixx")
        add_files("Platform/Windows/Dx12/*.cpp")
    end
    add_deps("Dumpling")
    add_packages("imgui")
target_end()