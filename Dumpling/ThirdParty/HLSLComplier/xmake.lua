if is_plat("windows") then
    add_requires("directxshadercompiler")
end

target("DumplingHLSLComplier")
    set_kind("static")
    add_files("*.ixx", {public=true})
    add_files("*.cpp")
    if is_plat("windows") then
        add_files("Platform/Dx12/*.ixx", {public=true})
        add_files("Platform/Dx12/*.cpp")
    end
    add_deps("Dumpling")
    add_packages("directxshadercomplier")
target_end()