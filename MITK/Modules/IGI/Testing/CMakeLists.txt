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

MITK_CREATE_MODULE_TESTS(LABELS niftkIGI)

mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES Qt4|QtCore Qt5|Core)

mitkAddCustomModuleTest(mitkIGI-Make-Geometry-Test mitkIGIMakeGeometryTest ${NIFTK_DATA_DIR}/Input/IGI/)

