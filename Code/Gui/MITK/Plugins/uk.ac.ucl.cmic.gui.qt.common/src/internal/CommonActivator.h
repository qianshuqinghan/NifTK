/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef COMMONACTIVATOR_H
#define COMMONACTIVATOR_H

#include <ctkPluginActivator.h>

namespace mitk {

class CommonActivator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* GetPluginContext();

private:

  static ctkPluginContext* s_PluginContext;

}; // CommonActivator

}

#endif // COMMONACTIVATOR_H
