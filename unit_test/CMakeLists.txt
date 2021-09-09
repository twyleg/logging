set(TARGET_NAME test_logging)

#
# set cmake settings
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(GTest CONFIG REQUIRED)
find_package(Boost COMPONENTS REQUIRED system filesystem)

add_executable(${TARGET_NAME}
	main.cc
	logger_test.cc
)

target_link_libraries(${TARGET_NAME}
	logging
	GTest::gmock
	GTest::gtest
	GTest::gmock_main
	GTest::gtest_main
	Boost::system
	Boost::filesystem
)
