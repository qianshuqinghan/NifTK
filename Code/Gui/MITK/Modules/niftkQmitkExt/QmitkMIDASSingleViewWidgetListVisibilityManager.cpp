/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date$
 Revision          : $Revision$
 Last modified by  : $Author$

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkMIDASSingleViewWidgetListVisibilityManager.h"
#include "QmitkMIDASSingleViewWidget.h"

//-----------------------------------------------------------------------------
QmitkMIDASSingleViewWidgetListVisibilityManager::QmitkMIDASSingleViewWidgetListVisibilityManager()
: m_DataStorage(NULL)
{
}


//-----------------------------------------------------------------------------
QmitkMIDASSingleViewWidgetListVisibilityManager::~QmitkMIDASSingleViewWidgetListVisibilityManager()
{
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::SetNodeVisibilityForWindow(mitk::DataNode* node, const unsigned int& widgetIndex, const bool& visibility)
{
  if (widgetIndex < 0 || widgetIndex >= m_Widgets.size())
  {
    return;
  }

  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);

  m_Widgets[widgetIndex]->SetRendererSpecificVisibility(nodes, visibility);
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::SetAllNodeVisibilityForWindow(const unsigned int& widgetIndex, const bool& visibility)
{
  if (m_DataStorage.IsNull())
  {
    return;
  }

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull() || it->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }
    this->SetNodeVisibilityForWindow(it->Value(), widgetIndex, visibility);
  }
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::SetNodeVisibilityForAllWindows(mitk::DataNode* node, const bool& visibility)
{
  for (unsigned int i = 0; i < m_Widgets.size(); i++)
  {
    this->SetNodeVisibilityForWindow(node, i, visibility);
  }
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::SetAllNodeVisibilityForAllWindows(const bool& visibility)
{
  if (m_DataStorage.IsNotNull())
  {
    return;
  }

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull() || it->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }
    this->SetNodeVisibilityForAllWindows(it->Value(), visibility);
  }
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::ClearWindow(const unsigned int& windowIndex)
{
  this->SetAllNodeVisibilityForWindow(windowIndex, false);
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::ClearWindows(const unsigned int& startWindowIndex, const unsigned int& endWindowIndex)
{
  for (unsigned int i = startWindowIndex; i <= endWindowIndex; i++)
  {
    this->ClearWindow(i);
  }
}


//-----------------------------------------------------------------------------
void QmitkMIDASSingleViewWidgetListVisibilityManager::ClearAllWindows()
{
  this->ClearWindows(0, m_Widgets.size()-1);
}