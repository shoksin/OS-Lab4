FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(common_tests common_tests.cpp)
target_link_libraries(common_tests common gtest gtest_main)

add_test(NAME common_tests COMMAND common_tests)