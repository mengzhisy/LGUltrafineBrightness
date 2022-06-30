cmake_minimum_required(VERSION 3.21)

if (DEFINED VCPKG_INSTALLED_DIR)
    # do nothing
else ()
    set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed)
endif ()
option(VCPKG_MANIFEST_INSTALL "VCPKG_MANIFEST_INSTALL" OFF)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
        CACHE STRING "Vcpkg toolchain file")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list(SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif ()
        endif ()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
endfunction()

project(LGUltraFineBrightness)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(hidapi REQUIRED CONFIG) # target_link_libraries(main PRIVATE hidapi::hidapi hidapi::winapi hidapi::include)

find_library(Propsys_lib Propsys REQUIRED)
find_library(PortableDeviceGUIDs_lib PortableDeviceGUIDs REQUIRED)
find_library(Sensorsapi_lib Sensorsapi REQUIRED)

add_executable(LGUltraFine main.cpp)

target_link_libraries(LGUltraFine Propsys PortableDeviceGUIDs Sensorsapi hidapi::hidapi hidapi::include)
