add_rules("mode.debug", "mode.release")
set_languages("cxxlatest")

if os.scriptdir() == os.projectdir() then 
    includes("../Potato/")
end

target("Dumpling")
    set_kind("static")
    add_files("Dumpling/*.cpp")
    add_files("Dumpling/*.ixx", {public=true})
    if is_plat("windows") then
        add_files("Dumpling/Platform/Windows/*.ixx", {public=true})
        add_files("Dumpling/Platform/Windows/*.cpp")
        add_links("user32.lib")
        add_links("d3d12.lib")
        add_links("dxgi.lib")
        add_links("gdi32.lib")
    end
    add_deps("Potato")
target_end()

includes("Dumpling/ThirdParty/ImGui/")
includes("Dumpling/ThirdParty/HLSLComplier/")

if os.scriptdir() == os.projectdir() then
    set_project("Dumpling")

    for _, file in ipairs(os.files("Test/*.cpp")) do
        local name = "ZTest_" .. path.basename(file)
        target(name)
            set_kind("binary")
            add_files(file)
            add_deps("Dumpling")
        target_end()
    end
end 