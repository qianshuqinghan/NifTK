/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $LastChangedDate$
 Revision          : $Revision$
 Last modified by  : $Author$

 Original author   : $Author$

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
 


#include "uk_ac_ucl_cmic_imagestatistics_Activator.h"

#include <QtPlugin>

#include "ImageStatisticsView.h"
#include "ImageStatisticsViewPreferencesPage.h"

namespace mitk {

void uk_ac_ucl_cmic_imagestatistics_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(ImageStatisticsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(ImageStatisticsViewPreferencesPage, context);
}

void uk_ac_ucl_cmic_imagestatistics_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_imagestatistics, mitk::uk_ac_ucl_cmic_imagestatistics_Activator)
