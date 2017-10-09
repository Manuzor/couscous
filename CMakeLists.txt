cmake_minimum_required(VERSION 3.0.0)

project(couscous)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_build")

if(WIN32)
    add_definitions(-DNOMINMAX                # Exclude annoying Windows macros.
                    -DWIN32_LEAN_AND_MEAN     # Strip some rarely used Windows stuff.
                    -D_CRT_SECURE_NO_WARNINGS # Shut up windows.h's warnings
                    )
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /WX") # Treat warnings as errors.
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /W4") # Warning level
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /wd4100") # "unreferenced formal parameter"
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /wd4201") # "nonstandard extension used: nameless struct/union"
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /wd4307") # "integral constant overflow"
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /wd4310") # "cast truncates constant value" TODO: Re-enable and fix these!
    set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} /wd4505") # "unreferenced local function has been removed"
endif()

# generated_code
set(GENERATED_CODE_DIR "${CMAKE_BINARY_DIR}/code/_generated/")
add_custom_command(OUTPUT "${GENERATED_CODE_DIR}/all_generated.cpp"
                   COMMAND powershell "${CMAKE_SOURCE_DIR}/tools/generate_code.ps1" -RepoRoot "${CMAKE_SOURCE_DIR}" -OutDir "${GENERATED_CODE_DIR}")
add_custom_target(generate_code
                  DEPENDS "${GENERATED_CODE_DIR}/all_generated.cpp"
                  SOURCES "${CMAKE_SOURCE_DIR}/tools/generate_code.ps1")

# couscous
include_directories("${CMAKE_SOURCE_DIR}/code" ${CMAKE_BINARY_DIR})
add_executable(couscous "${CMAKE_SOURCE_DIR}/code/win32_main.cpp")
add_dependencies(couscous generate_code)
set_target_properties(couscous PROPERTIES LINK_FLAGS
                      "/INCREMENTAL:NO /SUBSYSTEM:WINDOWS")

# couscous_assembler
include_directories("${CMAKE_SOURCE_DIR}/code" ${CMAKE_BINARY_DIR})
add_executable(couscous_assembler "${CMAKE_SOURCE_DIR}/code/couscous_assembler.cpp")
add_dependencies(couscous_assembler generate_code)
set_target_properties(couscous_assembler PROPERTIES LINK_FLAGS
                      "/INCREMENTAL:NO /SUBSYSTEM:CONSOLE")
