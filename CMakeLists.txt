cmake_minimum_required(VERSION 3.10)

project(LRCB-RPA)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

qt5_add_resources(RESOURCES rca.qrc)

add_executable(lrcb-rpa WIN32
	main.cpp
	util.cpp util.hpp
	loginwindow.cpp loginwindow.hpp
	settingswindow.cpp settingswindow.hpp
	assessmentwindow.cpp assessmentwindow.hpp
	assessmentscore.cpp assessmentscore.hpp
	${RESOURCES}
)
target_link_libraries(lrcb-rpa Qt5::Widgets Qt5::Multimedia)
