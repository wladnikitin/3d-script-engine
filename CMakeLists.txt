cmake_minimum_required(VERSION 3.10)
project(3D-Script-Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(3d-script-engine
    src/main.cpp
    src/core.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/external/json)

if (WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_WIN32)  # 👈 ЭТО ОБЯЗАТЕЛЬНО
endif()

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(3d-script-engine PRIVATE ${X11_LIBRARIES})
    target_include_directories(3d-script-engine PRIVATE ${X11_INCLUDE_DIR})
endif()