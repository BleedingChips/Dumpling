add_rules("mode.debug", "mode.release")
set_languages("cxxlatest")

if os.scriptdir() == os.projectdir() then 
    includes("../Potato/")
end

local var require_imgui = true
local var require_hlsl_complier = true

if is_plat("windows") then
    if require_imgui then
        add_requires("imgui", {configs={win32=true, dx12=true}})
    end
    if require_hlsl_complier then
        add_requires("directxshadercompiler")
    end
end

target("Dumpling")

    set_kind("static")
    add_files("Dumpling/*.cpp")
    add_files("Dumpling/*.ixx", {public=true})
    add_deps("Potato")

    if is_plat("windows") then
        add_files("Dumpling/Platform/Windows/*.ixx", {public=true})
        add_files("Dumpling/Platform/Windows/*.cpp")
        add_links("user32.lib")
        add_links("d3d12.lib")
        add_links("dxgi.lib")
        add_links("gdi32.lib")
    end

    if require_imgui then
        add_defines("DUMPLING_WITH_IMGUI")
        add_files("Dumpling/ThirdParty/ImGui/*.cpp")
        add_files("Dumpling/ThirdParty/ImGui/*.ixx", {public=true})
        if is_plat("windows") then
            add_files("Dumpling/ThirdParty/ImGui/Platform/Windows/*.ixx", {public=true})
            add_files("Dumpling/ThirdParty/ImGui/Platform/Windows/*.cpp")
        end
        add_packages("imgui", {public=true})
    end

    if require_hlsl_complier then
        add_defines("DUMPLING_WITH_HLSL_COMPLIER")
        add_files("Dumpling/ThirdParty/HLSLComplier/*.ixx", {public=true})
        add_files("Dumpling/ThirdParty/HLSLComplier/*.cpp")
        if is_plat("windows") then
            add_files("Dumpling/ThirdParty/HLSLComplier/Platform/*.ixx", {public=true})
            add_files("Dumpling/ThirdParty/HLSLComplier/Platform/*.cpp")
        end
        add_packages("directxshadercomplier")
    end

    
target_end()

--[[
if os.scriptdir() == os.projectdir() then
    set_project("Dumpling")
    for _, file in ipairs(os.files("Test/*.cpp")) do
        local var name = "ZTest_" .. path.basename(file)
        target(name)
            set_kind("binary")
            add_files(file)
            add_deps("Dumpling")
        target_end()
    end
end 
]]