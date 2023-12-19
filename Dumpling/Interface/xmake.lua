
set_languages("cxxlatest")

if os.scriptdir() == os.projectdir() then 
    includes("../Potato/")
end

target("DumplingInterface")
    set_kind("static")
    add_files("*.cpp")
    add_files("*.ixx")
    add_deps("Potato")
target_end()