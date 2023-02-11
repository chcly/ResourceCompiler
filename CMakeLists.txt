cmake_minimum_required(VERSION 3.15)
project(ResourceCompiler)

# Use C++-17 by default.
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set the module path to include scripts
# CMake - Contains project local modules.
#
# CMake/Modules - Contains project independent modules.
#   GitUpdate.cmake invokes python ${ResourceCompiler_SOURCE_DIR}/gitupdate.py
#   to pull all submodules and update to head.
#   If this fails, it will attempt to invoke git directly to pull 
#   submodules. 
list(APPEND CMAKE_MODULE_PATH 
  ${ResourceCompiler_SOURCE_DIR}/CMake 
  ${ResourceCompiler_SOURCE_DIR}/CMake/Modules
)
include(${ResourceCompiler_SOURCE_DIR}/CMake/Configure.cmake)
if (NOT Configure_SUCCEEDED)
   return()
endif()



if (ResourceCompiler_ExternalTarget)
    set(TargetFolders ${ResourceCompiler_TargetFolders})
    set(TargetName    ${ResourceCompiler_TargetName})
    set(TargetGroup   ${ResourceCompiler_TargetGroup})
else()
    add_subdirectory(CMake)
    add_subdirectory(Internal/Utils)

    set(TargetFolders TRUE)
    set(TargetName ResourceCompiler)
    set(TargetGroup )

    set(ResourceCompiler_INCLUDE  ${ResourceCompiler_SOURCE_DIR}/Source)
    set(ResourceCompiler_LIBRARY  ${TargetName})
endif()

add_subdirectory(Source)

if (ResourceCompiler_BUILD_TEST)
    if (NOT ResourceCompiler_ExternalTarget)
        add_gtest_source(Extern/ThirdParty)
    endif()

    set(TargetGroup Units)
    add_subdirectory(Test)
endif()
