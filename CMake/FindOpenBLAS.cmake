#/*============================================================================
#
#  NifTK: A software platform for medical image computing.
#
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  See LICENSE.txt in the top level directory for details.
#
#============================================================================*/

set(OpenBLAS_FOUND)

set(OpenBLAS_DIR @OpenBLAS_DIRECTORY@ CACHE PATH "Directory containing OpenBLAS installation")

find_path(OpenBLAS_INC
  NAME cblas.h
  PATHS ${OpenBLAS_DIR}/include
  NO_DEFAULT_PATH
)

set(OpenBLAS_LIBRARY_DIR ${OpenBLAS_DIR}/lib)
set(OpenBLAS_LIBRARY )
if (WIN32)
  set(CMAKE_FIND_LIBRARY_PREFIXES lib)
endif()

find_library(OpenBLAS_LIBRARY NAMES openblas
            PATHS ${OpenBLAS_LIBRARY_DIR}
            NO_DEFAULT_PATH)

if(OpenBLAS_LIBRARY AND OpenBLAS_INC)
  set(OpenBLAS_FOUND 1)
  get_filename_component(_inc_dir ${OpenBLAS_INC} PATH)
  set(OpenBLAS_INCLUDE_DIR ${_inc_dir})
endif()

message( "NifTK FindOpenBLAS: OpenBLAS_INCLUDE_DIR: ${OpenBLAS_INCLUDE_DIR}" )
message( "NifTK FindOpenBLAS: OpenBLAS_LIBRARY_DIR: ${OpenBLAS_LIBRARY_DIR}" )
message( "NifTK FindOpenBLAS: OpenBLAS_LIBRARY:     ${OpenBLAS_LIBRARY}" )
