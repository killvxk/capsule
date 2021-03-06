cmake_minimum_required(VERSION 2.8)

project(capsulerun)

if(APPLE)
  # MACOSX_RPATH is now enabled by default
  cmake_policy(SET CMP0042 NEW)
endif()

set(capsulerun_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(libcapsule_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libcapsule/src)
set(capsulerun_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(capsulerun_INCLUDE_DIRS SYSTEM)

set(libcapsule_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libcapsule/src)
set(argparse_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../argparse)

include_directories(
  ${shoom_INCLUDE_DIR}
  ${capsulerun_INCLUDE_DIRS}
  ${capsulerun_INCLUDE_DIR}
  ${libcapsule_INCLUDE_DIR}
  ${microprofile_INCLUDE_DIR}
  ${argparse_INCLUDE_DIR}
  ${lab_INCLUDE_DIR}
)

set(capsulerun_SRC
  ${capsulerun_SOURCE_DIR}/runner.cc
  ${capsulerun_SOURCE_DIR}/router.cc
  ${capsulerun_SOURCE_DIR}/main.cc
  ${capsulerun_SOURCE_DIR}/encoder.cc
  ${capsulerun_SOURCE_DIR}/main_loop.cc
  ${capsulerun_SOURCE_DIR}/video_receiver.cc
  ${capsulerun_SOURCE_DIR}/audio_intercept_receiver.cc
  ${capsulerun_SOURCE_DIR}/session.cc
  ${capsulerun_SOURCE_DIR}/connection.cc
  ${capsulerun_SOURCE_DIR}/fps_counter.cc
  ${capsulerun_SOURCE_DIR}/logging.cc
)

if(WIN32)
  list(APPEND capsulerun_SRC
    ${capsulerun_SOURCE_DIR}/windows/wasapi_receiver.cc
    ${capsulerun_SOURCE_DIR}/windows/hotkey.cc
    ${capsulerun_SOURCE_DIR}/windows/executor.cc
    ${capsulerun_SOURCE_DIR}/windows/relay.c
  )
endif()

if(APPLE)
  list(APPEND capsulerun_SRC
    ${capsulerun_SOURCE_DIR}/macos/hotkey.mm
    ${capsulerun_SOURCE_DIR}/macos/bundle_utils.mm
    ${capsulerun_SOURCE_DIR}/macos/executor.cc
  )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  list(APPEND capsulerun_SRC
    ${capsulerun_SOURCE_DIR}/linux/pulse_dynamic.cc
    ${capsulerun_SOURCE_DIR}/linux/pulse_receiver.cc
    ${capsulerun_SOURCE_DIR}/linux/hotkey.cc
    ${capsulerun_SOURCE_DIR}/linux/executor.cc
  )
  add_definitions(-D__STDC_CONSTANT_MACROS)
endif()

if(APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/")
else()
  set(CMAKE_INSTALL_RPATH "\$ORIGIN")
endif()
add_executable(capsulerun ${capsulerun_SRC})

target_link_libraries(capsulerun shoom)
target_link_libraries(capsulerun lab)
target_link_libraries(capsulerun microprofile)
target_link_libraries(capsulerun argparse)

if(WIN32)
  add_dependencies(capsulerun capsule_deps)
  target_link_libraries(capsulerun ${DEVIARE_INPROC_LIBRARY})

  foreach(NEEDED_LIB avutil.lib avcodec.lib avformat.lib swscale.lib swresample.lib)
    target_link_libraries(capsulerun ${FFMPEG_LIBRARY_DIR}/${NEEDED_LIB})
  endforeach(NEEDED_LIB)
endif()

if(APPLE)
  add_dependencies(capsulerun capsule_deps)
  foreach(NEEDED_LIB avutil avcodec avformat swscale swresample x264)
    target_link_libraries(capsulerun ${FFMPEG_LIBRARY_DIR}/lib${NEEDED_LIB}.dylib)
  endforeach(NEEDED_LIB)

  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(capsulerun ${COCOA_LIBRARY})
  find_library(CARBON_LIBRARY Carbon)
  target_link_libraries(capsulerun ${CARBON_LIBRARY})
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  include(FindPkgConfig)
  set(ENV{PKG_CONFIG_PATH} "${CAPSULE_DEPS_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

  foreach(NEEDED_LIB libavutil libavcodec libavformat libswscale libswresample x264 libpulse-simple)
    PKG_CHECK_MODULES(${NEEDED_LIB}_PKG ${NEEDED_LIB})
    include_directories(${${NEEDED_LIB}_PKG_INCLUDE_DIRS})
    target_link_libraries(capsulerun ${${NEEDED_LIB}_PKG_LDFLAGS} ${${NEEDED_LIB}_PKG_LIBRARIES})
  endforeach(NEEDED_LIB)

  PKG_CHECK_MODULES(libpulse-simple_PKG libpulse-simple)
  include_directories(${libpulse-simple_PKG_INCLUDE_DIRS})

  # looks like C++11 threads rely on that somehow
  target_link_libraries(capsulerun -lpthread)
  # for hotkey support
  target_link_libraries(capsulerun -lX11)
  # for dynamically loading some libraries (libpulse-simple, etc.)
  target_link_libraries(capsulerun -ldl)
endif()

install(TARGETS capsulerun
  DESTINATION "${CMAKE_BINARY_DIR}/dist"
)

set(CAPSULERUN_BINARY_PATH "${CMAKE_BINARY_DIR}/dist/capsulerun${CMAKE_EXECUTABLE_SUFFIX}")
set(CAPSULERUN_LIBRARY_DIRS "${CAPSULE_DEPS_PREFIX}/lib" "${FFMPEG_LIBRARY_DIR}")

message("ffmpeg library dir: ${FFMPEG_LIBRARY_DIR}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake"
  @ONLY
)
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake")
