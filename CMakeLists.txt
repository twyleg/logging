cmake_minimum_required(VERSION 3.1.0)

project(Logging)

# Externals
add_subdirectory(external/spdlog/)
add_subdirectory(external/simple-xercesc/libs/)

# Libs
add_subdirectory(libs/)

# Apps
add_subdirectory(apps/simple_logging_example/)
add_subdirectory(apps/qt_logging_example/)

# Unit-Test
add_subdirectory(unit_test/)
