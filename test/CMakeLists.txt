include(GoogleTest)

add_executable(${PROJECT_NAME}-test ../nanodiff.cpp nanodiff-test.cpp)
target_compile_definitions(${PROJECT_NAME}-test PRIVATE NANODIFF_TEST)
target_compile_features(${PROJECT_NAME}-test PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME}-test PRIVATE
    -Wall
    -Wextra
    -Werror=pedantic
    -pedantic-errors
    -Wno-unused-function
    -fno-omit-frame-pointer
    -fsanitize=address,undefined)
target_link_libraries(${PROJECT_NAME}-test PRIVATE gtest gtest_main)
target_link_options(${PROJECT_NAME}-test PRIVATE
    -fsanitize=address,undefined)

gtest_discover_tests(${PROJECT_NAME}-test)
