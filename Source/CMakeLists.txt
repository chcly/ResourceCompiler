include(CopyTargets)
set(TargetName ResourceCompiler)

include_directories(${Utils_INCLUDE})

set(Target_SRC
    Main.cpp
    CMake/ResourceCompiler.cmake
)

add_executable(${TargetName}  
              ${Target_SRC}
              ${Target_HDR})

target_link_libraries(${TargetName} ${Utils_LIBRARY})

copy_target(${TargetName} ${CMAKE_BINARY_DIR})

set_target_properties(${TargetName} PROPERTIES FOLDER "${TargetGroup}")
