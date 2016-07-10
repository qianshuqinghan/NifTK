/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef IGIUltrasoundOverlayEditorActivator_h
#define IGIUltrasoundOverlayEditorActivator_h

#include <ctkPluginActivator.h>

namespace niftk
{

/**
 * \class IGIUltrasoundOverlayEditorActivator
 * \brief Activator class for the IGIUltrasoundOverlayEditor.
 * \ingroup uk_ac_ucl_cmic_igiultrasoundoverlayeditor_internal
 */
class IGIUltrasoundOverlayEditorActivator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "uk_ac_ucl_cmic_igiultrasoundoverlayeditor")
#endif

public:

  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;
  static ctkPluginContext* getContext();

private:
  static ctkPluginContext* m_PluginContext;

};

} // end namespace

#endif

