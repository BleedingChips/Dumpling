if is_plat("windows") then
    add_requires("directxshadercompiler")
end

target("DumplingHLSLComplier")
    set_kind("static")
    add_files("*.ixx")
    if is_plat("windows") then
        add_files("Platform/Dx12/*.ixx")
        add_files("Platform/Dx12/*.cpp")
    end
    add_deps("Dumpling")
    add_packages("directxshadercomplier")
target_end()