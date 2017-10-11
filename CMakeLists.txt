cmake_minimum_required(VERSION 3.0.0)

project(couscous)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_build")

if(WIN32)
    add_definitions(-DWIN32
                    -D_WINDOWS
                    -DNOMINMAX                # Exclude annoying Windows macros.
                    -DWIN32_LEAN_AND_MEAN     # Strip some rarely used Windows stuff.
                    -D_CRT_SECURE_NO_WARNINGS # Shut up windows.h's warnings
                    )
endif()

if(MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS "")
    string(APPEND CMAKE_CXX_FLAGS " /WX")
    string(APPEND CMAKE_CXX_FLAGS " /W4")
    string(APPEND CMAKE_CXX_FLAGS " /wd4100") # "unreferenced formal parameter"
    string(APPEND CMAKE_CXX_FLAGS " /wd4201") # "nonstandard extension used: nameless struct/union"
    string(APPEND CMAKE_CXX_FLAGS " /wd4307") # "integral constant overflow"
    string(APPEND CMAKE_CXX_FLAGS " /wd4310") # "cast truncates constant value" TODO: Re-enable and fix these!
    string(APPEND CMAKE_CXX_FLAGS " /wd4505") # "unreferenced local function has been removed"

    set(CMAKE_CXX_FLAGS_DEBUG "")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Od")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Zi")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /wd4189") # "local variable is initialized but not referenced"
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /RTC1")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /MTd")

    set(CMAKE_CXX_FLAGS_RELEASE "")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /Ox")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /DNDEBUG")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /MT")
endif()

file(GLOB_RECURSE COUSCOUS_CODE_FILES "${CMAKE_SOURCE_DIR}/code/*.*")

# generated_code
set(GENERATED_CODE_DIR "${CMAKE_SOURCE_DIR}/code/_generated/")
add_custom_command(OUTPUT "${GENERATED_CODE_DIR}/all_generated.cpp"
                   COMMAND powershell "${CMAKE_SOURCE_DIR}/tools/generate_code.ps1" -RepoRoot "${CMAKE_SOURCE_DIR}" -OutDir "${GENERATED_CODE_DIR}")
add_custom_target(generate_code
                  DEPENDS "${GENERATED_CODE_DIR}/all_generated.cpp"
                  SOURCES "${CMAKE_SOURCE_DIR}/tools/generate_code.ps1")

# couscous
include_directories("${CMAKE_SOURCE_DIR}/code")
add_executable(couscous WIN32 "${CMAKE_SOURCE_DIR}/code/win32_main.cpp")
add_dependencies(couscous generate_code)
if(MSVC)
    set_target_properties(couscous PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif()

# couscous_assembler
include_directories("${CMAKE_SOURCE_DIR}/code")
add_executable(couscous_assembler "${CMAKE_SOURCE_DIR}/code/couscous_assembler.cpp")
add_dependencies(couscous_assembler generate_code)
if(MSVC)
    set_target_properties(couscous_assembler PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif()
