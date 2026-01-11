cmake_minimum_required(VERSION 3.20)
project(metadata-cleaner VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(WIN32)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# CPM.cmake
set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

# Force static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage(
    NAME taglib
    GITHUB_REPOSITORY taglib/taglib
    GIT_TAG v2.0.2
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "BUILD_TESTING OFF"
        "BUILD_EXAMPLES OFF"
)

set(SOURCES
    src/main.cpp
    src/config.cpp
    src/cleaner.cpp
    src/file_watcher.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    TAGLIB_STATIC
    PROJECT_VERSION="${PROJECT_VERSION}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${taglib_SOURCE_DIR}/taglib
    ${taglib_SOURCE_DIR}/taglib/toolkit
    ${taglib_SOURCE_DIR}/taglib/mpeg
    ${taglib_SOURCE_DIR}/taglib/flac
    ${taglib_SOURCE_DIR}/taglib/wav
    ${taglib_SOURCE_DIR}/taglib/mp4
    ${taglib_BINARY_DIR}/taglib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    tag
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES config/config.json DESTINATION share/${PROJECT_NAME})
