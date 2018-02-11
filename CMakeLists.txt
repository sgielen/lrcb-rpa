cmake_minimum_required(VERSION 3.5.2)

project(LRCB-RPA)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Core Widgets Multimedia Xml REQUIRED)

qt5_add_resources(RESOURCES rca.qrc QDarkStyleSheet/qdarkstyle/style.qrc)

add_executable(lrcb-rpa WIN32
	main.cpp
	util.cpp util.hpp
	loginwindow.cpp loginwindow.hpp
	settingswindow.cpp settingswindow.hpp
	assessmentwindow.cpp assessmentwindow.hpp
	assessmentscore.cpp assessmentscore.hpp
	excel.cpp excel.hpp
	${RESOURCES}
)
target_link_libraries(lrcb-rpa Qt5::Widgets Qt5::Multimedia Qt5::Xml)

find_file(NSIS_EXECUTABLE
	NAMES makensis makensis.exe
	PATHS $ENV{PROGRAMFILES} $ENV{ProgramFiles} $ENV{PROGRAMW6432} $ENV{ProgramW6432}
	PATH_SUFFIXES "NSIS"
)

set(QT_LIBRARY_PATH "${_qt5Core_install_prefix}/bin")

if(NOT NSIS_EXECUTABLE)
	message("NSIS executable not found, can't build launcher")
else()
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa.exe"
		COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:lrcb-rpa>" "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa.exe"
		DEPENDS lrcb-rpa
	)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lrcb-rpa-launcher.nsi.in"
		"${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa-launcher.nsi" @ONLY)
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa-launcher.exe"
		COMMAND "${NSIS_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa-launcher.nsi"
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa.exe" "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa-launcher.nsi"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		VERBATIM
	)

	add_custom_target(lrcb-rpa-launcher ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lrcb-rpa-launcher.exe")
endif()
