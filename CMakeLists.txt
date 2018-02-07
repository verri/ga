cmake_minimum_required(VERSION 3.0)
project(GA VERSION 2.0.0)

configure_file("include/ga/version.hpp.in" "include/ga/version.hpp")

add_library(ga INTERFACE)

target_compile_features(ga INTERFACE cxx_std_11)

target_include_directories(ga INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>)

target_include_directories(ga SYSTEM INTERFACE
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

option(GA_BUILD_TEST "whether or not to build the test" OFF)
if(GA_BUILD_TEST)
  enable_testing()
  add_subdirectory(test)
endif()

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ga-config.cmake "
include(\${CMAKE_CURRENT_LIST_DIR}/ga-targets.cmake)
set(GA_LIBRARY ga)
set(GA_LIBRARIES ga)
")

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ga-config-version.cmake
  VERSION ${GA_VERSION}
  COMPATIBILITY ExactVersion
)

install(TARGETS ga EXPORT ga-targets DESTINATION lib)

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")

install(EXPORT ga-targets DESTINATION lib/cmake/ga)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ga-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ga-config-version.cmake
  DESTINATION lib/cmake/ga)
