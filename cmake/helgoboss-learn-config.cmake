include(CMakeFindDependencyMacro)
find_dependency(helgoboss-midi 0.1.0 CONFIG REQUIRED)
find_dependency(rxcpp CONFIG REQUIRED)
find_dependency(wdl-eel2 CONFIG REQUIRED)
include("${CMAKE_CURRENT_LIST_DIR}/helgoboss-learn-targets.cmake")