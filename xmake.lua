add_rules("mode.debug", "mode.release")
set_languages("cxxlatest")


if is_plat("windows") then
    add_requires("imgui", {configs={win32=true, dx12=true}})
end

if os.scriptdir() == os.projectdir() then 
    includes("../Potato/")
end

target("Dumpling")
    set_kind("static")
    add_files("Dumpling/*.cpp")
    add_files("Dumpling/*.ixx")
    if is_plat("windows") then
        add_files("Dumpling/Platform/Windows/*.cpp")
        add_files("Dumpling/Platform/Windows/*.ixx")
        add_links("user32.lib")
        --result.syslinks =  {"user32", "shell32", "gdi32"}
    end
    
    add_deps("Potato")
    add_packages("imgui")
target_end()

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