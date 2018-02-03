if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)
  file(DOWNLOAD
    https://github.com/catchorg/Catch2/releases/download/v2.1.0/catch.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp
    STATUS status
    LOG log)

  list(GET status 0 status_code)
  list(GET status 1 status_string)

  if(NOT status_code EQUAL 0)
    message(FATAL_ERROR "error downloading catch: ${status_string}" "${log}")
  endif()
endif()

set(source_files
  test_suite.cpp
  knapsack.cpp
  simple.cpp)

add_executable(ga_test_suite ${source_files})
set_target_properties(ga_test_suite PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

option(GA_TEST_COVERAGE "whether or not add coverage instrumentation" OFF)
if (GA_TEST_COVERAGE)
  target_compile_options(ga_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-O0>")
  target_compile_options(ga_test_suite PRIVATE "$<$<CONFIG:DEBUG>:--coverage>")
  set_target_properties(ga_test_suite PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
else()
  target_compile_options(ga_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-O0>")
  target_compile_options(ga_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>")
  target_compile_options(ga_test_suite PRIVATE "$<$<CONFIG:DEBUG>:-fsanitize=address,leak,undefined>")
  set_target_properties(ga_test_suite PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,leak,undefined -fuse-ld=gold")
endif()

target_include_directories(ga_test_suite PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ga_test_suite PUBLIC ga)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    target_compile_options(ga_test_suite PRIVATE "/W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(ga_test_suite PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

add_test(NAME test.ga COMMAND ga_test_suite)
