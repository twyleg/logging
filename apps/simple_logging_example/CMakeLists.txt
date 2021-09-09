set(TARGET_NAME simple_logging_example)

#
# set cmake settings
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# add source files to target
#
add_executable(${TARGET_NAME}
	main.cc
)

#
# link against libs
#
target_link_libraries(${TARGET_NAME}
	logging
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/example_configs/config.xml
	${CMAKE_CURRENT_BINARY_DIR}
	COPYONLY
)
