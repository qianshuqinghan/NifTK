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


#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory \"${VTK_DIR}\".")
endif()

set(proj VTK)
set(proj_DEPENDENCIES )
set(VTK_DEPENDS ${proj})

if(NOT DEFINED VTK_DIR)

  if(WIN32)
    option(VTK_USE_SYSTEM_FREETYPE OFF)
  else(WIN32)
    option(VTK_USE_SYSTEM_FREETYPE ON)
  endif(WIN32)

  set(additional_cmake_args )
  if(MINGW)
    set(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF
        -DVTK_USE_VIDEO4WINDOWS:BOOL=OFF # no header files provided by MinGW
        )
  endif(MINGW)

  if(DESIRED_QT_VERSION MATCHES 4) # current VTK package has a HARD Qt 4 dependency
    list(APPEND additional_cmake_args
        -DDESIRED_QT_VERSION:STRING=${DESIRED_QT_VERSION}
        -DVTK_USE_GUISUPPORT:BOOL=ON
        -DVTK_USE_QVTK_QTOPENGL:BOOL=OFF
        -DVTK_USE_QT:BOOL=ON
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DVTK_Group_Qt:BOOL=ON
    )
  endif()

  if(APPLE)
    set(additional_cmake_args
        ${additional_cmake_args}
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=""
        )
  endif(APPLE)

  niftkMacroGetChecksum(NIFTK_CHECKSUM_VTK ${NIFTK_LOCATION_VTK})

  ExternalProject_Add(${proj}
    SOURCE_DIR ${proj}-src
    BINARY_DIR ${proj}-build
    PREFIX ${proj}-cmake
    INSTALL_DIR ${proj}-install
    URL ${NIFTK_LOCATION_VTK}
    URL_MD5 ${NIFTK_CHECKSUM_VTK}
    INSTALL_COMMAND ""
    CMAKE_GENERATOR ${GEN}
    CMAKE_ARGS
        ${EP_COMMON_ARGS}
        -DVTK_WRAP_TCL:BOOL=OFF
        -DVTK_WRAP_PYTHON:BOOL=OFF
        -DVTK_WRAP_JAVA:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=${EP_BUILD_SHARED_LIBS}
        -DVTK_USE_RPATH:BOOL=ON
        -DVTK_USE_SYSTEM_FREETYPE:BOOL=${VTK_USE_SYSTEM_FREETYPE}
        -DVTK_USE_GUISUPPORT:BOOL=ON
        -DVTK_LEGACY_REMOVE:BOOL=ON
        -DModule_vtkTestingRendering:BOOL=ON
        -DModule_vtkGUISupportQt:BOOL=ON
        -DModule_vtkGUISupportQtWebkit:BOOL=ON
        -DModule_vtkGUISupportQtSQL:BOOL=ON
        -DModule_vtkRenderingQt:BOOL=ON
        -DVTK_MAKE_INSTANTIATORS:BOOL=ON
        ${additional_cmake_args}
        ${VTK_QT_ARGS}
    DEPENDS ${proj_DEPENDENCIES}
    )

  set(VTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  message("SuperBuild loading VTK from ${VTK_DIR}")

else(NOT DEFINED VTK_DIR)

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif(NOT DEFINED VTK_DIR)
