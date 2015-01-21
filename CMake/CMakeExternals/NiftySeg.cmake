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
# NiftySeg
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED NIFTYSEG_ROOT AND NOT EXISTS ${NIFTYSEG_ROOT})
  message(FATAL_ERROR "NIFTYSEG_ROOT variable is defined but corresponds to non-existing disegtory \"${NIFTYSEG_ROOT}\".")
endif()

if(BUILD_NIFTYSEG)

  niftkMacroGetCommitHashOfCurrentFile(config_version)

  set(proj NiftySeg)
  set(proj_VERSION ${NIFTK_VERSION_NIFTYSEG})
  set(proj_SOURCE ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-src)
  set(proj_CONFIG ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-cmake)
  set(proj_BUILD ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-build)
  set(proj_INSTALL ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-install)
  set(proj_DEPENDENCIES Eigen)
  set(NIFTYSEG_DEPENDS ${proj})

  if(NOT DEFINED NIFTYSEG_ROOT)

    niftkMacroGetChecksum(NIFTK_CHECKSUM_NIFTYSEG ${NIFTK_LOCATION_NIFTYSEG})

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj_SOURCE}
      PREFIX ${proj_CONFIG}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${NIFTK_LOCATION_NIFTYSEG}
      URL_MD5 ${NIFTK_CHECKSUM_NIFTYSEG}
      CMAKE_GENERATOR ${GEN}
      #CONFIGURE_COMMAND ""
      UPDATE_COMMAND ${GIT_EXECUTABLE} checkout ${proj_VERSION}
      #BUILD_COMMAND ""
      #INSTALL_COMMAND ""
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DUSE_CUDA:BOOL=${NIFTK_USE_CUDA}
        -DUSE_OPENMP:BOOL=OFF
        -DINSTALL_PRIORS:BOOL=ON
        -DINSTALL_PRIORS_DIRECTORY:PATH=${proj_INSTALL}/priors
        -DCMAKE_INSTALL_PREFIX:PATH=${proj_INSTALL}
        -DUSE_SYSTEM_EIGEN=ON
        -DEigen_INCLUDE_DIR=${Eigen_INCLUDE_DIR}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(NIFTYSEG_ROOT ${proj_INSTALL})
    set(NIFTYSEG_INCLUDE_DIR "${NIFTYSEG_ROOT}/include")
    set(NIFTYSEG_LIBRARY_DIR "${NIFTYSEG_ROOT}/lib")

    message("SuperBuild loading NiftySeg from ${NIFTYSEG_ROOT}")

  else(NOT DEFINED NIFTYSEG_ROOT)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif(NOT DEFINED NIFTYSEG_ROOT)

endif(BUILD_NIFTYSEG)
