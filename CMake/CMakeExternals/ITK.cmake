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
# ITK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory \"${ITK_DIR}\".")
endif()

set(version "4.7.1-20c0592")
set(location "${NIFTK_EP_TARBALL_LOCATION}/InsightToolkit-${version}.tar.gz")

niftkMacroDefineExternalProjectVariables(ITK ${version} ${location})
set(proj_DEPENDENCIES GDCM)

if(MITK_USE_Python)
  list(APPEND proj_DEPENDENCIES CableSwig)
endif()

if(MITK_USE_HDF5)
  list(APPEND proj_DEPENDENCIES HDF5)
endif()

if(BUILD_IGI)
  list(APPEND proj_DEPENDENCIES OpenCV)
endif()

if(NOT DEFINED ITK_DIR)

  set(additional_cmake_args )
  if(MINGW)
    set(additional_cmake_args
        -DCMAKE_USE_WIN32_THREADS:BOOL=ON
        -DCMAKE_USE_PTHREADS:BOOL=OFF)
  endif()

  if(MITK_USE_Python)

    list(APPEND additional_cmake_args
         -DITK_WRAPPING:BOOL=ON
         -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
         -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
         -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
         #-DPYTHON_LIBRARIES=${PYTHON_LIBRARY}
         #-DPYTHON_DEBUG_LIBRARIES=${PYTHON_DEBUG_LIBRARIES}
         -DCableSwig_DIR:PATH=${CableSwig_DIR}
         #-DITK_WRAP_JAVA:BOOL=OFF
         -DITK_WRAP_unsigned_char:BOOL=ON
         #-DITK_WRAP_double:BOOL=ON
         -DITK_WRAP_rgb_unsigned_char:BOOL=ON
         #-DITK_WRAP_rgba_unsigned_char:BOOL=ON
         -DITK_WRAP_signed_char:BOOL=ON
         #-DWRAP_signed_long:BOOL=ON
         -DITK_WRAP_signed_short:BOOL=ON
         -DITK_WRAP_short:BOOL=ON
         -DITK_WRAP_unsigned_long:BOOL=ON
        )
  else()
    list(APPEND additional_cmake_args
         -DUSE_WRAP_ITK:BOOL=OFF
        )
  endif()

  if(BUILD_IGI)
    message("Building ITK with OpenCV_DIR: ${OpenCV_DIR}")
    list(APPEND additional_cmake_args
         -DModule_ITKVideoBridgeOpenCV:BOOL=ON
         -DOpenCV_DIR:PATH=${OpenCV_DIR}
        )
  endif()

  if (BUILD_ITKFFTW)
    if(WIN32)
      # On Windows, you have to precompile one.
      # This is currently untested.
      list(APPEND additional_cmake_args
           -DUSE_SYSTEM_FFTW:BOOL=ON
          )
    else()
      # This causes FFTW to be downloaded and compiled.
      list(APPEND additional_cmake_args
           -DUSE_FFTWF:BOOL=ON
           -DUSE_FFTWD:BOOL=ON
           -DUSE_SYSTEM_FFTW:BOOL=OFF
          )
    endif()
  endif()

  # Keep the behaviour of ITK 4.3 which by default turned on ITK Review
  # see MITK bug #17338
  list(APPEND additional_cmake_args
    -DModule_ITKReview:BOOL=ON
  # for 4.7, the OpenJPEG is needed by review but the variable must be set
    -DModule_ITKOpenJPEG:BOOL=ON
  )

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
    )
  endif()

  set(vcl_constexpr_patch)
  if(GCC_VERSION VERSION_LESS 4.8 AND GCC_VERSION VERSION_GREATER 4)
    set(vcl_constexpr_patch
      COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.7.1-gcc-4.6.patch
    )
  endif()

  # this will be in ITK 4.8, see https://issues.itk.org/jira/browse/ITK-3361
  set(vcl_gcc5_patch)
  if(GCC_VERSION VERSION_GREATER 4)
    set(vcl_gcc5_patch
      COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.7.1-gcc-5.patch
    )
  endif()

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ^^
    PREFIX ${proj_CONFIG}
    SOURCE_DIR ${proj_SOURCE}
    BINARY_DIR ${proj_BUILD}
    INSTALL_DIR ${proj_INSTALL}
    URL ${proj_LOCATION}
    URL_MD5 ${proj_CHECKSUM}
     # work with external GDCM
     PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ITK-4.7.1.patch
                   ${vcl_constexpr_patch}
                   ${vcl_gcc5_patch}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${EP_COMMON_ARGS}
      -DCMAKE_PREFIX_PATH:PATH=${NifTK_PREFIX_PATH}
      ${additional_cmake_args}
      -DITK_USE_SYSTEM_GDCM:BOOL=ON
      -DGDCM_DIR:PATH=${GDCM_DIR}
    CMAKE_CACHE_ARGS
      ${EP_COMMON_CACHE_ARGS}
    CMAKE_CACHE_DEFAULT_ARGS
      ${EP_COMMON_CACHE_DEFAULT_ARGS}
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(ITK_DIR ${proj_INSTALL})
  set(NifTK_PREFIX_PATH ${proj_INSTALL}^^${NifTK_PREFIX_PATH})
  mitkFunctionInstallExternalCMakeProject(${proj})

  message("SuperBuild loading ITK from ${ITK_DIR}")

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
