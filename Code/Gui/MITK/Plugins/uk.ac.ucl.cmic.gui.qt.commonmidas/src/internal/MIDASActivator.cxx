/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "MIDASActivator.h"
#include <QtPlugin>
#include <mitkGlobalInteraction.h>
#include <mitkMIDASTool.h>

namespace mitk {

ctkPluginContext* MIDASActivator::s_PluginContext(NULL);

//-----------------------------------------------------------------------------
void MIDASActivator::start(ctkPluginContext* context)
{
  s_PluginContext = context;

  mitk::MIDASTool::LoadBehaviourStrings();
}


//-----------------------------------------------------------------------------
void MIDASActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}


//-----------------------------------------------------------------------------
ctkPluginContext* MIDASActivator::GetPluginContext()
{
  return s_PluginContext;
}

} // end namespace

//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_gui_qt_commonmidas, mitk::MIDASActivator)
#endif
