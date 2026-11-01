cmake_minimum_required(VERSION 3.10)
project(local_scaner_prj)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(SQLite3 REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/extern/imgui)

if(APPLE)
    set(glfw3_DIR "/opt/homebrew/lib/cmake/glfw3")
endif()
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(local_scaner_prj
    src/main.cc
    inc/UI/Window.h
    src/UI/Window.cc
    inc/third_party/asio.hpp
        inc/Network/MCWorker.h
        src/Network/MCWorker.cc
    ${IMGUI_SOURCES}
    inc/Network/proto.h

    inc/Core/DevicesFinder/DeviceFinder.h
    inc/Core/DevicesFinder/VendorWorker/VendorWorker.h
    src/Core/DeviceFinder/DeviceFinder.cc
    inc/Core/DevicesFinder/SystemWorker/SystemWorker.h
    inc/Core/DevicesFinder/SystemWorker/MacosSystemWorker.h
    src/Core/DeviceFinder/SystemWorker/MacosSystemWorker.cc
    inc/Core/DevicesFinder/SystemWorker/WindowsSystemWorker.h
    inc/Core/DevicesFinder/SystemWorker/LinuxSystemWorker.h
    inc/Core/DevicesFinder/VendorWorker/VendorWorkerDatabase.h
    inc/Core/DevicesFinder/VendorWorker/VendorWorkerCsv.h
    src/Core/DeviceFinder/VendorWorker/VendorWorkerDatabase.cc
    src/Core/DeviceFinder/VendorWorker/VendorWorkerCsv.cc
    inc/Core/DeviceController/DeviceController.h
    inc/Core/DeviceController/KeyboardWorker/AbstractKeyboardWorker.h
    inc/Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.h
    src/Core/DeviceController/KeyboardWorker/MacosKeyboardWorker.cc
    inc/Network/TcpServer.h
    inc/Network/TcpClient.h
    src/Core/DeviceController/DeviceController.cc
    src/Core/DeviceFinder/SystemWorker/WindowsSystemWorker.cc
    inc/Core/DeviceController/KeyboardWorker/WindowsKeyboardWorker.h
    src/Core/DeviceController/KeyboardWorker/WindowsKeyboardWorker.cc
)

target_include_directories(local_scaner_prj PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${glfw3_INCLUDE_DIRS}
)
target_compile_definitions(local_scaner_prj PRIVATE ASIO_STANDALONE)

if(APPLE)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(local_scaner_prj
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework ApplicationServices"
        OpenSSL::SSL
        OpenSSL::Crypto
        SQLite::SQLite3
        glfw
        ${glfw3_LIBRARIES}
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
    )
    target_compile_definitions(local_scaner_prj PRIVATE
        BOOST_BEAST_USE_STD_ERIC=1
        BOOST_ASIO_USE_STD_ERIC=1
    )
elseif(WIN32)
if(MINGW)
    target_link_options(local_scaner_prj PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
    target_link_libraries(local_scaner_prj PRIVATE glfw ws2_32 gdi32 opengl32 SQLite::SQLite3 iphlpapi)
endif()
