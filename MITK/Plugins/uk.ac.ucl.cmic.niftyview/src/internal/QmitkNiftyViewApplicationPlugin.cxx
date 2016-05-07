/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkNiftyViewApplicationPlugin.h"
#include <QmitkCommonAppsMinimalPerspective.h>
#include <QmitkCommonAppsIGIPerspective.h>
#include <QmitkMIDASSegmentationPerspective.h>
#include <QmitkNiftyViewApplicationPreferencePage.h>
#include <QmitkCommonAppsApplicationPreferencePage.h>
#include "../QmitkNiftyViewApplication.h"

//-----------------------------------------------------------------------------
QmitkNiftyViewApplicationPlugin::QmitkNiftyViewApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QmitkNiftyViewApplicationPlugin::~QmitkNiftyViewApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QString QmitkNiftyViewApplicationPlugin::GetHelpHomePageURL() const
{
  return QString("qthelp://uk.ac.ucl.cmic.niftyview/bundle/uk_ac_ucl_cmic_niftyview_intro.html");
}


//-----------------------------------------------------------------------------
void QmitkNiftyViewApplicationPlugin::start(ctkPluginContext* context)
{
  QmitkCommonAppsApplicationPlugin::start(context);

  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyViewApplication, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCommonAppsMinimalPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCommonAppsIGIPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCommonAppsApplicationPreferencePage, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyViewApplicationPreferencePage, context);

  this->RegisterHelpSystem();
}


//-----------------------------------------------------------------------------
void QmitkNiftyViewApplicationPlugin::stop(ctkPluginContext* context)
{
  this->UnregisterDataStorageListener();
}


//-----------------------------------------------------------------------------
void QmitkNiftyViewApplicationPlugin::NodeAdded(const mitk::DataNode *constNode)
{
  mitk::DataNode::Pointer node = const_cast<mitk::DataNode*>(constNode);
  this->RegisterLevelWindowProperty("uk.ac.ucl.cmic.niftyview", node);
}


//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_niftyview, QmitkNiftyViewApplicationPlugin)
#endif