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
    inc/Window.h
    src/Window.cc
    inc/asio.hpp
    inc/MulticastBus.h
    src/MulticastBus.cc
    ${IMGUI_SOURCES}
    inc/proto.h
    inc/KeyScaner.h
    src/KeyScaner.cc
    inc/TraficMonitor.h
    src/TraficMonitor.cc

    inc/DevicesFinder/ArpWorker/ArpWorker.h
    inc/DevicesFinder/DeviceFinder.h
    inc/DevicesFinder/VendorWorker/VendorWorker.h

    src/DeviceFinder/ArpWorker/MacosArpWorker.cc
    src/DeviceFinder/DeviceFinder.cc
    inc/DevicesFinder/SystemWorker/SystemWorker.h
    inc/DevicesFinder/SystemWorker/MacosSystemWorker.h
    src/DeviceFinder/SystemWorker/MacosSystemWorker.cc
    inc/DevicesFinder/SystemWorker/WindowsSystemWorker.h
    inc/DevicesFinder/SystemWorker/LinuxSystemWorker.h
    inc/DevicesFinder/ArpWorker/MacosArpWorker.h
    inc/DevicesFinder/ArpWorker/WindowsArpWorker.h
    inc/DevicesFinder/ArpWorker/LinuxArpWorker.h
    inc/DevicesFinder/VendorWorker/VendorWorkerDatabase.h
    inc/DevicesFinder/VendorWorker/VendorWorkerCsv.h
    src/DeviceFinder/VendorWorker/VendorWorkerDatabase.cc
    src/DeviceFinder/VendorWorker/VendorWorkerCsv.cc
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
    target_link_libraries(local_scaner_prj PRIVATE glfw ws2_32 gdi32 opengl32)
endif()
