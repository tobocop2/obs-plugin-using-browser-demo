cmake_minimum_required(VERSION 3.16...3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (WIN32 AND NOT CYGWIN)
  set (OS_WINDOWS 1)
else ()
  set (OS_WINDOWS 0)
endif ()

set(
  CMAKE_TOOLCHAIN_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE
  STRING
  "Vcpkg toolchain file"
)

project(obs-plugin-using-browser-demo VERSION 1.0.0)
add_library(${CMAKE_PROJECT_NAME} MODULE)

set(PLUGIN_AUTHOR "Tobias Perelstein")

set(MACOS_BUNDLEID "com.example.${CMAKE_PROJECT_NAME}")

set(LINUX_MAINTAINER_EMAIL "tobias@perelste.in")

# Add your custom source files here - header files are optional and only
# required for visibility e.g. in Xcode or Visual Studio

find_package(nlohmann_json 3.10.5 CONFIG REQUIRED)

if(OS_WINDOWS)
  find_package(unofficial-libuv CONFIG REQUIRED)
  get_filename_component(
    LIBUV_INCLUDE_DIR
    "${unofficial-libuv_DIR}/../../include"
    ABSOLUTE
  )

  target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    unofficial::libuv::libuv
  )
endif()

# Fetch uwebsockets
include(FetchContent)
FetchContent_Declare(
  uwebsockets
  GIT_REPOSITORY
  https://github.com/uNetworking/uWebSockets.git
)
FetchContent_GetProperties(uwebsockets)
if(NOT uwebsockets_POPULATED)
  FetchContent_Populate(uwebsockets)
endif()

# Add libaries from uwebsockets
add_library(
  usockets
  ${uwebsockets_SOURCE_DIR}/uSockets/src/bsd.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/context.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/loop.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/socket.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/crypto/openssl.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/crypto/sni_tree.cpp
  ${uwebsockets_SOURCE_DIR}/uSockets/src/crypto/wolfssl.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/eventing/epoll_kqueue.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/eventing/gcd.c
  ${uwebsockets_SOURCE_DIR}/uSockets/src/eventing/libuv.c)


target_include_directories(
  usockets
  PUBLIC
  ${uwebsockets_SOURCE_DIR}/uSockets/src
  ${LIBUV_INCLUDE_DIR}
)

if(NOT(OS_WINDOWS))
  target_compile_options(usockets PUBLIC -flto -O3)
endif()

target_compile_features(usockets PRIVATE cxx_std_11)
target_compile_definitions(usockets PRIVATE -DLIBUS_NO_SSL)

add_library(uwebsockets INTERFACE)
target_include_directories(
  uwebsockets
  INTERFACE
  ${uwebsockets_SOURCE_DIR}/src
  ${uwebsockets_SOURCE_DIR}/uSockets/src
  ${LIBUV_INCLUDE_DIR}
)

target_compile_features(uwebsockets INTERFACE cxx_std_20)
target_compile_definitions(uwebsockets INTERFACE -DUWS_NO_ZLIB)
target_link_libraries(uwebsockets INTERFACE usockets)

if(OS_WINDOWS)
  target_link_libraries(uwebsockets INTERFACE unofficial::libuv::libuv)
endif()


target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  src/obs-browser.cpp
  src/plugin-main.cpp
  src/BrowserDialog.cpp
  src/DemoObject.cpp
)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  nlohmann_json::nlohmann_json
  uwebsockets
)


# /!\ TAKE NOTE: No need to edit things past this point /!\

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
include(cmake/ObsPluginHelpers.cmake)
find_qt(VERSION ${QT_VERSION} COMPONENTS Widgets Core)


if(OS_WINDOWS)
  # TODO BUT WHY????????
  set_target_properties(
    OBS::libobs
    OBS::obs-frontend-api
    PROPERTIES
    IMPORTED_IMPLIB MinSizeRel;Release;Debug;RelWithDebInfo
    IMPORTED_LOCATION MinSizeRel;Release;Debug;RelWithDebInfo
  )
endif()


configure_file(src/plugin-macros.h.in
  ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# --- Platform-independent build settings ---

target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  OBS::libobs
  OBS::obs-frontend-api
  Qt::Core
  Qt::Widgets
)

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
  AUTOUIC ON
  AUTORCC ON)
add_compile_options(-std=c++17)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
    ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /MP /d2FH4-)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macOS.pkgproj.in
    ${CMAKE_BINARY_DIR}/installer-macOS.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE -Wall -Wextra -Werror-implicit-function-declaration -stdlib=libc++
    -fvisibility=default -std=c++17)

  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)

  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})

if(OS_WINDOWS)
  install(IMPORTED_RUNTIME_ARTIFACTS unofficial::libuv::libuv
          RUNTIME DESTINATION ${OBS_PLUGIN_DESTINATION})
endif()

