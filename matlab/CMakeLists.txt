# Author: Ramon Casero <rcasero@gmail.com>
# Copyright © 2011-2014 University of Oxford
# Version: 0.3.1
#
# University of Oxford means the Chancellor, Masters and Scholars of
# the University of Oxford, having an administrative office at
# Wellington Square, Oxford OX1 2JD, UK. 
#
# This file is part of Gerardus.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details. The offer of this
# program under the terms of the License is subject to the License
# being interpreted in accordance with English Law and subject to any
# action against the University of Oxford being under the jurisdiction
# of the English Courts.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)

project(GERARDUS_MATLAB)

# # Find ITK.
# FIND_PACKAGE(ITK REQUIRED)
# IF(ITK_FOUND)
#   MESSAGE(STATUS "ITK found")
#   INCLUDE(${ITK_USE_FILE})
# ELSE(ITK_FOUND)
#   MESSAGE(FATAL_ERROR "ITK not found")
# ENDIF(ITK_FOUND)

# # Find Matlab
# SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
# FIND_PACKAGE(Matlab REQUIRED)
# IF(MATLAB_FOUND)
#   MESSAGE(STATUS "Matlab found")
# ELSE(MATLAB_FOUND)
#   MESSAGE(FATAL_ERROR "Matlab not found")
# ENDIF(MATLAB_FOUND)

include_directories(${MATLAB_INCLUDE_DIR})

# Add macros to build MEX files
include(${CMAKE_CURRENT_SOURCE_DIR}/MatlabMakeMacros.cmake)

# build mex functions in the toolboxes
add_subdirectory(CgalToolbox)
add_subdirectory(FileFormatToolbox)
add_subdirectory(FiltersToolbox)
add_subdirectory(ItkToolbox)
add_subdirectory(ThirdPartyToolbox)
add_subdirectory(PointsToolbox)
