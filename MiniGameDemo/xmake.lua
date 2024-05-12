add_rules("mode.debug", "mode.release")
set_languages("cxxlatest")

target("MiniGameDemo")
    set_kind("binary")
    add_files("MiniGameDemo/*.cpp")
    add_files("MiniGameDemo/*.ixx")
    add_deps("Dumpling")
    add_deps("Noodles")
    add_deps("DumplingImGui")
target_end()
    