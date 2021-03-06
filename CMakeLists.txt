#   Copyright (C) 2010 by Rob Hasselbaum <rob@hasselbaum.net>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>

project (SocketSentry)
cmake_minimum_required (VERSION 2.6.4)
enable_testing()

# Initialize project version
file (READ VERSION SS_PROJECT_VERSION)
add_definitions (-DPROJECT_VERSION="${SS_PROJECT_VERSION}")

# Look for required test support libraries (GMock/GTest). If successful, TEST_SUPPORT_LIBS
# will be defined with them.
find_library (GMOCK gmock)
find_library (GTEST gtest)
if (EXISTS ${GMOCK} AND EXISTS ${GTEST})
	set (TEST_SUPPORT_LIBS ${GMOCK} ${GTEST})
else (EXISTS ${GMOCK} AND EXISTS ${GTEST})
	message(WARNING "Google Mock or Google Test wasn't found. Unit tests won't be built.")
endif (EXISTS ${GMOCK} AND EXISTS ${GTEST})

# FUNCTION: add_qtestlib_tests <test_source_files> [<link_libraries> ... ]
#
# This function does nothing if TEST_SUPPORT_LIBS is undefined. Otherwise, it adds a suite of QTestLib tests 
# to the build. The required input is a list of test source files. Each source file must contain one 
# unit test class that extends QObject and includes a main function to run the tests. (The latter
# could be provided by Qt's QTEST_MAIN macro.) This function builds one executable for each input
# source file (including generating the moc source), optionally links it to a set of libraries, 
# and then adds the test to CMake. The name of each executable is run<source_filename_without_extension>.
# The link libraries should contain the code under test. The TEST_SUPPORT_LIBS are also linked
# automatically.
function (add_qtestlib_tests test_source_files)
	if (DEFINED TEST_SUPPORT_LIBS)
		foreach(test_file_src ${test_source_files})
			get_filename_component(test_file_path ${test_file_src} PATH)
			get_filename_component(test_file_base ${test_file_src} NAME_WE)
			set(test_file_header ${test_file_path}/${test_file_base}.h)
			unset(test_moc_outfiles)
			qt4_wrap_cpp(test_moc_outfiles ${test_file_header} )
			set(test_exe run${test_file_base})
			add_executable( ${test_exe} ${test_file_src} ${test_moc_outfiles} )
			target_link_libraries( ${test_exe} ${TEST_SUPPORT_LIBS} ${ARGN} )
			add_test(${test_file_base} ${EXECUTABLE_OUTPUT_PATH}/${test_exe} )
		endforeach(test_file_src)
	endif (DEFINED TEST_SUPPORT_LIBS)
endfunction(add_qtestlib_tests)

add_subdirectory (socketsentry-service)
add_subdirectory (socketsentry-plasma-engine)
add_subdirectory (socketsentry-plasma-widget)

