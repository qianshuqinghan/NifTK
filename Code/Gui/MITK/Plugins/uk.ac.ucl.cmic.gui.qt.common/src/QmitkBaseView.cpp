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

 Original author   : Miklos Espak <espakm@gmail.com>

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkBaseView.h"
#include "internal/VisibilityChangedCommand.h"
#include <itkCommand.h>
#include <mitkGlobalInteraction.h>
#include <mitkFocusManager.h>
#include <mitkSliceNavigationController.h>
#include <QmitkRenderWindow.h>

class QmitkBaseViewPrivate
{
public:

  QmitkBaseViewPrivate();
  ~QmitkBaseViewPrivate();

  QMap<const mitk::DataNode*, unsigned long> visibilityObserverTags;

  mitk::MessageDelegate1<QmitkBaseView, const mitk::DataNode*>* addNodeEventListener;
  mitk::MessageDelegate1<QmitkBaseView, const mitk::DataNode*>* removeNodeEventListener;

  /// \brief Used to store the parent of this view, and should normally be set from within CreateQtPartControl().
  QWidget *m_Parent;

  /// \brief Stores the activation status.
  bool m_IsActivated;

  /// \brief Stores the visible status.
  bool m_IsVisible;

  /// \brief Used for the mitkFocusManager to register callbacks to track the currently focussed window.
  unsigned long m_FocusManagerObserverTag;

  /// \brief Used to track the currently focussed renderer.
  mitk::BaseRenderer* m_Focussed2DRenderer;

  /// \brief Used to track the previously focussed renderer.
  mitk::BaseRenderer* m_PreviouslyFocussed2DRenderer;
};


//-----------------------------------------------------------------------------
QmitkBaseViewPrivate::QmitkBaseViewPrivate()
{
  m_Parent = NULL;
  m_IsActivated = false;
  m_IsVisible = false;
  m_FocusManagerObserverTag = 0;
  m_Focussed2DRenderer = NULL;
  m_PreviouslyFocussed2DRenderer = NULL;
}


//-----------------------------------------------------------------------------
QmitkBaseViewPrivate::~QmitkBaseViewPrivate()
{
}


//-----------------------------------------------------------------------------
QmitkBaseView::QmitkBaseView()
: QmitkAbstractView(),
  d_ptr(new QmitkBaseViewPrivate)
{
  Q_D(QmitkBaseView);

  mitk::DataStorage* dataStorage = GetDataStorage();
  if (dataStorage) {

    mitk::DataStorage::SetOfObjects::ConstPointer everyNode = dataStorage->GetAll();
    mitk::DataStorage::SetOfObjects::ConstIterator it = everyNode->Begin();
    mitk::DataStorage::SetOfObjects::ConstIterator end = everyNode->End();
    while (it != end) {
      onNodeAddedInternal(it->Value());
      ++it;
    }

    d->addNodeEventListener =
        new mitk::MessageDelegate1<QmitkBaseView, const mitk::DataNode*>(this, &QmitkBaseView::onNodeAddedInternal);
    dataStorage->AddNodeEvent.AddListener(*d->addNodeEventListener);

    d->removeNodeEventListener =
        new mitk::MessageDelegate1<QmitkBaseView, const mitk::DataNode*>(this, &QmitkBaseView::onNodeRemovedInternal);
    dataStorage->RemoveNodeEvent.AddListener(*d->removeNodeEventListener);
  }
  else {
    MITK_INFO << "QmitkBaseView() data storage not ready";
  }

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (focusManager != NULL)
  {
    itk::SimpleMemberCommand<QmitkBaseView>::Pointer onFocusChangedCommand =
      itk::SimpleMemberCommand<QmitkBaseView>::New();
    onFocusChangedCommand->SetCallbackFunction( this, &QmitkBaseView::OnFocusChanged );

    d->m_FocusManagerObserverTag = focusManager->AddObserver(mitk::FocusEvent(), onFocusChangedCommand);
  }
}


//-----------------------------------------------------------------------------
QmitkBaseView::~QmitkBaseView() {
  Q_D(QmitkBaseView);

  mitk::DataStorage* dataStorage = GetDataStorage();
  if (dataStorage)
  {
    dataStorage->AddNodeEvent.RemoveListener(*d->addNodeEventListener);
    dataStorage->RemoveNodeEvent.RemoveListener(*d->removeNodeEventListener);

    delete d->addNodeEventListener;
    delete d->removeNodeEventListener;
  }

  foreach (const mitk::DataNode* node, d->visibilityObserverTags.keys()) {
    mitk::BaseProperty* property = node->GetProperty("visible");
    if (property) {
      property->RemoveObserver(d->visibilityObserverTags[node]);
    }
  }

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (focusManager != NULL)
  {
    focusManager->RemoveObserver(d->m_FocusManagerObserverTag);
  }
}


//-----------------------------------------------------------------------------
bool QmitkBaseView::IsExclusiveFunctionality() const
{
  return false;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::onNodeAddedInternal(const mitk::DataNode* node)
{
  Q_D(QmitkBaseView);
  mitk::BaseProperty* property = node->GetProperty("visible");
  if (property) {
    VisibilityChangedCommand::Pointer command = VisibilityChangedCommand::New(this, node);
    d->visibilityObserverTags[node] = property->AddObserver(itk::ModifiedEvent(), command);
  }
}


//-----------------------------------------------------------------------------
void QmitkBaseView::onNodeRemovedInternal(const mitk::DataNode* node)
{
  Q_D(QmitkBaseView);
  if (d->visibilityObserverTags.contains(node)) {
    mitk::BaseProperty* property = node->GetProperty("visible");
    if (property) {
      property->RemoveObserver(d->visibilityObserverTags[node]);
    }
    d->visibilityObserverTags.remove(node);
  }
}


//-----------------------------------------------------------------------------
void QmitkBaseView::onVisibilityChanged(const mitk::DataNode* /*node*/)
{
}


//-----------------------------------------------------------------------------
QWidget* QmitkBaseView::GetParent()
{
  Q_D(QmitkBaseView);
  return d->m_Parent;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::SetParent(QWidget* parent)
{
  Q_D(QmitkBaseView);
  d->m_Parent = parent;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::OnFocusChanged()
{
  Q_D(QmitkBaseView);

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (focusManager != NULL)
  {
    mitk::BaseRenderer* base = focusManager->GetFocused();
    if (base != NULL && base->GetMapperID() == mitk::BaseRenderer::Standard2D)
    {
      if (d->m_Focussed2DRenderer != NULL)
      {
        d->m_PreviouslyFocussed2DRenderer = d->m_Focussed2DRenderer;
      }
      d->m_Focussed2DRenderer = base;
    }
  }
}


//-----------------------------------------------------------------------------
mitk::BaseRenderer* QmitkBaseView::GetCurrentlyFocussedRenderer()
{
  Q_D(QmitkBaseView);
  return d->m_Focussed2DRenderer;
}


//-----------------------------------------------------------------------------
mitk::BaseRenderer* QmitkBaseView::GetPreviouslyFocussedRenderer()
{
  Q_D(QmitkBaseView);
  return d->m_PreviouslyFocussed2DRenderer;
}


//-----------------------------------------------------------------------------
mitk::SliceNavigationController* QmitkBaseView::GetSliceNavigationController()
{
  return this->GetSliceNavigationControllerInternal();
}


//-----------------------------------------------------------------------------
mitk::SliceNavigationController* QmitkBaseView::GetSliceNavigationControllerInternal()
{
  mitk::SliceNavigationController::Pointer result = NULL;

  Q_D(QmitkBaseView);
  if (d->m_Focussed2DRenderer != NULL)
  {
    result = d->m_Focussed2DRenderer->GetSliceNavigationController();
  }
  if (result.IsNull())
  {
    mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
    if (renderWindowPart != NULL)
    {
      QmitkRenderWindow *renderWindow = renderWindowPart->GetActiveQmitkRenderWindow();
      if (renderWindow != NULL)
      {
        result = renderWindow->GetSliceNavigationController();
      }
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
int QmitkBaseView::GetSliceNumberFromSliceNavigationController()
{
  int sliceNumber = -1;

  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController();
  if (snc.IsNotNull())
  {
    sliceNumber = snc->GetSlice()->GetPos();
  }
  return sliceNumber;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::Activated()
{
  Q_D(QmitkBaseView);
  d->m_IsActivated = true;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::Deactivated()
{
  Q_D(QmitkBaseView);
  d->m_IsActivated = false;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::Visible()
{
  Q_D(QmitkBaseView);
  d->m_IsVisible = true;
}


//-----------------------------------------------------------------------------
void QmitkBaseView::Hidden()
{
  Q_D(QmitkBaseView);
  d->m_IsVisible = false;
}


//-----------------------------------------------------------------------------
bool QmitkBaseView::IsActivated()
{
  Q_D(QmitkBaseView);
  return d->m_IsActivated;
}


//-----------------------------------------------------------------------------
bool QmitkBaseView::IsVisible()
{
  Q_D(QmitkBaseView);
  return d->m_IsVisible;
}


//-----------------------------------------------------------------------------
QmitkRenderWindow* QmitkBaseView::GetRenderWindow(QString id)
{
  QmitkRenderWindow* window = NULL;

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  if (renderWindowPart != NULL)
  {
    window = renderWindowPart->GetQmitkRenderWindow(id);
  }

  return window;
}