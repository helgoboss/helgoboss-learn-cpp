find_package(Catch2 CONFIG REQUIRED)
include(Catch)
add_executable(helgoboss-learn-tests
    tests.cpp
    ModeTest.cpp
    SourceTest.cpp
    math-util-test.cpp
    )
target_compile_features(helgoboss-learn-tests PRIVATE cxx_std_17)
set_target_properties(helgoboss-learn-tests PROPERTIES CXX_EXTENSIONS OFF)
# Disable those terrible min max macros in windows.h
target_compile_definitions(helgoboss-learn-tests PRIVATE NOMINMAX)
target_link_libraries(helgoboss-learn-tests PRIVATE Catch2::Catch2 helgoboss-learn::helgoboss-learn)
catch_discover_tests(helgoboss-learn-tests)