# Copyright(c) 2018 - 2019 Federico Bolelli, Costantino Grana
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met :
#
# *Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and / or other materials provided with the distribution.
#
# * Neither the name of GRAPHSGEN nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required (VERSION 3.13)

set (ProjectName "GRAPHSGEN")
project (${ProjectName})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Install prefix" FORCE)

#Set GRAPHSGEN variables
set(GRAPHSGEN_OUTPUT_PATH "${CMAKE_INSTALL_PREFIX}/outputs" CACHE PATH "'output' results path")

if(MSVC)
  set(CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "" FORCE)
endif()

# ----------------------------------------------------------------------------
# Solution folders
# ----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

# --------------------
# OpenCV 
# --------------------
set(OpenCV_REQUIRED_PACKAGES "core;imgcodecs;imgproc" CACHE STRING "OpenCV packages required by GRAPHSGEN are already set")
FIND_PACKAGE(OpenCV REQUIRED ${OpenCV_REQUIRED_PACKAGES})
include_directories( ${OpenCV_INCLUDE_DIRS} )
if(MSVC)
	if("${OpenCV_STATIC}" STREQUAL "ON" OR "${OpenCV_SHARED}" STREQUAL "OFF")
		message(STATUS "Setting static runtime for MSVC")
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO) 
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endforeach(flag_var)
	endif()
endif()

find_package(Git QUIET)

# --------------------
# 3rdparty 
# --------------------
set(CMAKE_FOLDER 3rdparty)
add_subdirectory(3rdparty)
set(CMAKE_FOLDER "")
# --------------------


# --------------------
# Project
# --------------------
add_library(GRAPHSGEN STATIC "${CMAKE_INSTALL_PREFIX}/config.yaml")
target_include_directories(GRAPHSGEN PUBLIC src/GRAPHSGEN)
add_subdirectory(src/GRAPHSGEN)

set(LABELING_ALGORITHMS SAUF PRED BBDT CACHE INTERNAL ON FORCE)
foreach(ALGO ${LABELING_ALGORITHMS})
	add_executable(${ALGO} "")
	set_target_properties(${ALGO} PROPERTIES FOLDER "Labeling")
	include_directories(src/Labeling)
	add_subdirectory(src/Labeling/${ALGO})
	target_link_libraries (${ALGO} GRAPHSGEN yaml-cpp)
endforeach()

# Check for c++17 support (TODO check if it actually works)
set_property(TARGET ${LABELING_ALGORITHMS} GRAPHSGEN PROPERTY CXX_STANDARD 17)
set_property(TARGET ${LABELING_ALGORITHMS} GRAPHSGEN PROPERTY CXX_STANDARD_REQUIRED ON)

#add_definitions(-D_CRT_SECURE_NO_WARNINGS) #To suppress 'fopen' opencv warning/bug  
# Set configuration file	
configure_file("${CMAKE_SOURCE_DIR}/CMake.config.yaml" "${CMAKE_INSTALL_PREFIX}/config.yaml")


#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName}) # Set YACCLAB as startup group instead of ALL_BUILD
#target_link_libraries (${ProjectName} ${OpenCV_LIBS})

# Creation of output directory
if (NOT EXISTS "${GRAPHSGEN_OUTPUT_PATH}")
	file(MAKE_DIRECTORY ${GRAPHSGEN_OUTPUT_PATH})
endif()

#target_include_directories(SAUF PUBLIC 
#                                $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/3rdparty/yaml-cpp/include> 
#                                $<INSTALL_INTERFACE:include>)
