# Загрузка Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# Эта строка избавляет от необходимости устанавливать gmock
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Если используем MSVC, указываем использовать DLL версию C runtime
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)
endif()

# Загрузка и подготовка библиотеки
FetchContent_MakeAvailable(googletest)

# Добавление тестов для общей функциональности
add_executable(common_tests common_tests.cpp)
target_link_libraries(common_tests common gtest gtest_main)

# Добавление тестов в CTest
add_test(NAME common_tests COMMAND common_tests)