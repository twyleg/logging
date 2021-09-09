set(TARGET_NAME qt_logging_example)

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
find_package(Qt5 COMPONENTS REQUIRED Core Widgets Qml Quick QuickControls2)
find_package(fmt REQUIRED)

#
# add source files to target
#
add_executable(${TARGET_NAME}
	main.cc
	resources/example.qrc
)

#
# link against libs
#
target_link_libraries(${TARGET_NAME}
	logging
	qt_logging
	Qt::Core
	Qt::Widgets
	Qt::Qml
	Qt::Quick
	Qt::QuickControls2
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/example_configs/config.xml
	${CMAKE_CURRENT_BINARY_DIR}
	COPYONLY
)
