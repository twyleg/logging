set(TARGET_NAME qt_logging)

#
# set cmake settings
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#
# find packages
#
find_package(Qt5 COMPONENTS REQUIRED Core Widgets)
find_package(fmt REQUIRED)

#
# add source files to target
#
add_library(${TARGET_NAME}
	utils.cc
	utils.h
	sinks.cc
	sinks.h
)

#
# link against libs
#
target_link_libraries(${TARGET_NAME}
	logging
	fmt::fmt
	Qt::Core
	Qt::Widgets
)

#
# add parent folder to targets include directories
#
target_include_directories(${TARGET_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)
