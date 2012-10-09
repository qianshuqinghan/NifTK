/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-12-16 09:12:58 +0000 (Fri, 16 Dec 2011) $
 Revision          : $Revision: 8039 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKMIDASSINGLEVIEWWIDGETLISTMANAGER_H_
#define QMITKMIDASSINGLEVIEWWIDGETLISTMANAGER_H_

#include <vector>
#include <set>

namespace mitk
{
class DataNode;
}

class QmitkRenderWindow;
class QmitkMIDASSingleViewWidget;

/**
 * \class QmitkMIDASSingleViewWidgetListManager
 * \brief Base class for objects that manipulate nodes over a list of QmitkMIDASSingleViewWidget.
 *
 * This class contains a vector of widgets that it knows about. So each QmitkMIDASSingleViewWidget
 * must be "registered", which means adding to this list.  This class also provides
 * a vector of sets of nodes. So derived classes can "associate" nodes with a given widget.
 * It is up to derived classes to decide what it means for a node to be "associated" with a
 * widget. For example, a class could manage renderer specific visibility properties.
 *
 * So, in this class, "registering" means simply adding a window or node to the right list.
 * "De-registering" means simply removing it. Subclasses should provide additional functionality.
 */
class QmitkMIDASSingleViewWidgetListManager
{

public:

  /// \brief Constructor.
  QmitkMIDASSingleViewWidgetListManager();

  /// \brief Destructor.
  virtual ~QmitkMIDASSingleViewWidgetListManager();

  /// \brief Each new QmitkMIDASSingleViewWidget should first be registered with this class
  virtual void RegisterWidget(QmitkMIDASSingleViewWidget *widget);

  /// \brief Used to de-register all the widgets, which means removing them from m_DataNodes and m_Widgets.
  virtual void DeRegisterAllWidgets();

  /// \brief De-registers a range of widgets.
  virtual void DeRegisterWidgets(unsigned int startWidgetIndex, unsigned int endWidgetIndex);

  /// \brief Given a window, will return the corresponding list index, or -1 if not found.
  virtual int GetIndexFromWindow(QmitkRenderWindow* window);

  /// \brief Gets the number of nodes currently "registered" with a certain QmitkMIDASSingleViewWidget.
  virtual int GetNumberOfNodesRegisteredWithWidget(int windowIndex);

  /// \brief Removes the node, or "de-registers" it from all widgets.
  virtual void RemoveNode( const mitk::DataNode* node);

protected:

  // We maintain a set of data nodes present in each window.
  // So, it's a vector of sets, as we have one set for each of the registered windows.
  std::vector< std::set<mitk::DataNode*> > m_DataNodes;

  // Additionally, we manage a list of widgets, where m_DataNodes.size() == m_Widgets.size() should always be true.
  std::vector< QmitkMIDASSingleViewWidget* > m_Widgets;

private:

};

#endif