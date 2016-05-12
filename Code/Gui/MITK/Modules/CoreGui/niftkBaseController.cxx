/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkBaseController.h"

#include <QApplication>

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkFocusManager.h>
#include <mitkGlobalInteraction.h>
#include <QmitkRenderWindow.h>

#include <mitkDataStorageListener.h>
#include <mitkDataNodePropertyListener.h>

#include "niftkBaseGUI.h"
#include "niftkIBaseView.h"

namespace niftk
{

class BaseControllerPrivate
{
  Q_DECLARE_PUBLIC(BaseController);

  BaseController* const q_ptr;

  BaseGUI* m_GUI;

  niftkIBaseView* m_View;

  class DataStorageListener : private mitk::DataStorageListener
  {
  public:
    DataStorageListener(BaseControllerPrivate* d, mitk::DataStorage* dataStorage)
      : mitk::DataStorageListener(dataStorage),
        m_D(d)
    {
    }
  private:
    virtual void OnNodeAdded(mitk::DataNode* node) override
    {
      m_D->OnNodeAdded(node);
    }

    virtual void OnNodeChanged(mitk::DataNode* node) override
    {
      m_D->OnNodeChanged(node);
    }

    virtual void OnNodeRemoved(mitk::DataNode* node) override
    {
      m_D->OnNodeRemoved(node);
    }

    virtual void OnNodeDeleted(mitk::DataNode* node) override
    {
      m_D->OnNodeDeleted(node);
    }

    BaseControllerPrivate* m_D;
  };

  class VisibilityListener : private mitk::DataNodePropertyListener
  {
  public:
    VisibilityListener(BaseControllerPrivate* d, mitk::DataStorage* dataStorage)
      : mitk::DataNodePropertyListener(dataStorage, "visible"),
        m_D(d)
    {
    }
  private:
    virtual void OnPropertyChanged(mitk::DataNode* node, const mitk::BaseRenderer* renderer) override
    {
      m_D->OnNodeVisibilityChanged(node, renderer);
    }

    BaseControllerPrivate* m_D;
  };


  void OnFocusedRendererDeleted();

  void OnFocusChanged();

  void OnSelectedSliceChanged();

  void OnNodeAdded(mitk::DataNode* node);

  void OnNodeChanged(mitk::DataNode* node);

  void OnNodeRemoved(mitk::DataNode* node);

  void OnNodeDeleted(mitk::DataNode* node);

  void OnNodeVisibilityChanged(const mitk::DataNode* node, const mitk::BaseRenderer* renderer);

public:

  BaseControllerPrivate(BaseController* q, niftkIBaseView* view);
  ~BaseControllerPrivate();

  /// \brief Used to track the currently focused renderer.
  mitk::BaseRenderer* m_FocusedRenderer;

  /// \brief Keep track of this to SliceNavigationController register and unregister event listeners.
  mitk::SliceNavigationController* m_SliceNavigationController;

  /// \brief Observer to get notified of the slice change in the focused renderer.
  unsigned long m_SliceChangeObserverTag;

private:

  /// \brief Used for the mitkFocusManager to register callbacks to track the currently focused window.
  unsigned long m_FocusChangeObserverTag;

  /// \brief Observer to get notified of the deletion of the focused renderer.
  unsigned long m_FocusedRendererDeletionObserverTag;

  DataStorageListener* m_DataStorageListener;

  VisibilityListener* m_VisibilityListener;

};

}

//-----------------------------------------------------------------------------
niftk::BaseControllerPrivate::BaseControllerPrivate(BaseController* baseController, niftkIBaseView* view)
  : q_ptr(baseController),
    m_GUI(nullptr),
    m_View(view),
    m_FocusChangeObserverTag(0ul),
    m_FocusedRenderer(nullptr),
    m_FocusedRendererDeletionObserverTag(0ul),
    m_SliceNavigationController(nullptr),
    m_SliceChangeObserverTag(0ul),
    m_DataStorageListener(new DataStorageListener(this, view->GetDataStorage())),
    m_VisibilityListener(new VisibilityListener(this, view->GetDataStorage()))
{
  Q_Q(BaseController);

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  assert(focusManager);

  itk::SimpleMemberCommand<BaseControllerPrivate>::Pointer onFocusChangedCommand =
    itk::SimpleMemberCommand<BaseControllerPrivate>::New();
  onFocusChangedCommand->SetCallbackFunction(this, &BaseControllerPrivate::OnFocusChanged);

  m_FocusChangeObserverTag = focusManager->AddObserver(mitk::FocusEvent(), onFocusChangedCommand);
}


//-----------------------------------------------------------------------------
niftk::BaseControllerPrivate::~BaseControllerPrivate()
{
  Q_Q(BaseController);

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  assert(focusManager);

  focusManager->RemoveObserver(m_FocusChangeObserverTag);

  delete m_DataStorageListener;
  delete m_VisibilityListener;
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnFocusChanged()
{
  MITK_INFO << "niftk::BaseControllerPrivate::OnFocusChanged()";
  Q_Q(BaseController);

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  assert(focusManager);

  mitk::BaseRenderer* focusedRenderer = focusManager->GetFocused();

  /// TODO check if the renderer is in the active editor

  if (focusedRenderer != m_FocusedRenderer)
  {
    if (m_FocusedRenderer)
    {
      m_FocusedRenderer->RemoveObserver(m_FocusedRendererDeletionObserverTag);
      m_FocusedRenderer = nullptr;
      m_FocusedRendererDeletionObserverTag = 0ul;
    }

    if (m_SliceNavigationController)
    {
      m_SliceNavigationController->RemoveObserver(m_SliceChangeObserverTag);
      m_SliceNavigationController = nullptr;
      m_SliceChangeObserverTag = 0ul;
    }

    assert(focusedRenderer);

    itk::SimpleMemberCommand<BaseControllerPrivate>::Pointer focusedRendererDeletedCommand = itk::SimpleMemberCommand<BaseControllerPrivate>::New();
    focusedRendererDeletedCommand->SetCallbackFunction(this, &BaseControllerPrivate::OnFocusedRendererDeleted);
    m_FocusedRendererDeletionObserverTag = focusedRenderer->AddObserver(itk::DeleteEvent(), focusedRendererDeletedCommand);

    m_FocusedRenderer = focusedRenderer;

    mitk::SliceNavigationController* sliceNavigationController = focusedRenderer->GetSliceNavigationController();
    assert(sliceNavigationController);

    if (focusedRenderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
    {
      m_SliceNavigationController = sliceNavigationController;

      itk::SimpleMemberCommand<BaseControllerPrivate>::Pointer sliceChangedCommand = itk::SimpleMemberCommand<BaseControllerPrivate>::New();
      sliceChangedCommand->SetCallbackFunction(this, &BaseControllerPrivate::OnSelectedSliceChanged);
      m_SliceChangeObserverTag = sliceNavigationController->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), sliceChangedCommand);

      ImageOrientation orientation = q->GetImageOrientation();
      int sliceIndex = q->GetSliceIndex();
      MITK_INFO << "niftk::BaseControllerPrivate::OnFocusChanged() emit selected slice changed: orientation: " << orientation << " slice: " << sliceIndex;
      emit q->SelectedSliceChanged(orientation, sliceIndex);
    }
  }
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnFocusedRendererDeleted()
{
  m_FocusedRenderer = nullptr;
  m_SliceNavigationController = nullptr;
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnSelectedSliceChanged()
{
  Q_Q(BaseController);
  ImageOrientation orientation = q->GetImageOrientation();
  int sliceIndex = q->GetSliceIndex();
  MITK_INFO << "niftk::BaseControllerPrivate::OnSelectedSliceChanged() emit selected slice changed: orientation: " << orientation << " slice: " << sliceIndex;
  emit q->SelectedSliceChanged(orientation, sliceIndex);
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnNodeAdded(mitk::DataNode* node)
{
  Q_Q(BaseController);
  q->OnNodeAdded(node);
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnNodeChanged(mitk::DataNode* node)
{
  Q_Q(BaseController);
  q->OnNodeChanged(node);
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnNodeRemoved(mitk::DataNode* node)
{
  Q_Q(BaseController);
  q->OnNodeRemoved(node);
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnNodeDeleted(mitk::DataNode* node)
{
  Q_Q(BaseController);
  q->OnNodeDeleted(node);
}


//-----------------------------------------------------------------------------
void niftk::BaseControllerPrivate::OnNodeVisibilityChanged(const mitk::DataNode* node, const mitk::BaseRenderer* renderer)
{
  Q_Q(BaseController);
  q->OnNodeVisibilityChanged(node, renderer);
}


//-----------------------------------------------------------------------------
niftk::BaseController::BaseController(niftkIBaseView* view)
  : d_ptr(new niftk::BaseControllerPrivate(this, view))
{
}


//-----------------------------------------------------------------------------
niftk::BaseController::~BaseController()
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::SetupGUI(QWidget* parent)
{
  Q_D(BaseController);
  d->m_GUI = this->CreateGUI(parent);
}


//-----------------------------------------------------------------------------
niftk::BaseGUI* niftk::BaseController::GetGUI() const
{
  Q_D(const BaseController);
  return d->m_GUI;
}


//-----------------------------------------------------------------------------
niftkIBaseView* niftk::BaseController::GetView() const
{
  Q_D(const BaseController);
  return d->m_View;
}


//-----------------------------------------------------------------------------
mitk::DataStorage* niftk::BaseController::GetDataStorage() const
{
  return this->GetView()->GetDataStorage();
}


//-----------------------------------------------------------------------------
void niftk::BaseController::RequestRenderWindowUpdate() const
{
  this->GetView()->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
QList<mitk::DataNode::Pointer> niftk::BaseController::GetDataManagerSelection() const
{
  return this->GetView()->GetDataManagerSelection();
}


//-----------------------------------------------------------------------------
mitk::SliceNavigationController* niftk::BaseController::GetSliceNavigationController() const
{
  Q_D(const BaseController);
  return d->m_SliceNavigationController;
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnFocusChanged()
{
}


//-----------------------------------------------------------------------------
mitk::BaseRenderer* niftk::BaseController::GetFocused2DRenderer() const
{
  Q_D(const BaseController);
  return d->m_FocusedRenderer;
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnViewGetsActivated()
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnViewGetsDeactivated()
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnViewGetsVisible()
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnViewGetsHidden()
{
  Q_D(const BaseController);
  if (d->m_SliceNavigationController)
  {
    d->m_SliceNavigationController->RemoveObserver(d->m_SliceChangeObserverTag);
  }
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnNodeAdded(const mitk::DataNode* node)
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnNodeChanged(const mitk::DataNode* node)
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnNodeRemoved(const mitk::DataNode* node)
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnNodeDeleted(const mitk::DataNode* node)
{
}


//-----------------------------------------------------------------------------
void niftk::BaseController::OnNodeVisibilityChanged(const mitk::DataNode* node, const mitk::BaseRenderer* renderer)
{
}


//-----------------------------------------------------------------------------
niftk::ImageOrientation niftk::BaseController::GetImageOrientation() const
{
  ImageOrientation orientation = IMAGE_ORIENTATION_UNKNOWN;
  const mitk::SliceNavigationController* sliceNavigationController = this->GetSliceNavigationController();
  if (sliceNavigationController != NULL)
  {
    mitk::SliceNavigationController::ViewDirection viewDirection = sliceNavigationController->GetViewDirection();

    if (viewDirection == mitk::SliceNavigationController::Axial)
    {
      orientation = IMAGE_ORIENTATION_AXIAL;
    }
    else if (viewDirection == mitk::SliceNavigationController::Sagittal)
    {
      orientation = IMAGE_ORIENTATION_SAGITTAL;
    }
    else if (viewDirection == mitk::SliceNavigationController::Frontal)
    {
      orientation = IMAGE_ORIENTATION_CORONAL;
    }
  }
  return orientation;
}


//-----------------------------------------------------------------------------
int niftk::BaseController::GetSliceIndex() const
{
  Q_D(const BaseController);

  int sliceIndex = -1;

  if (d->m_SliceNavigationController)
  {
    sliceIndex = d->m_SliceNavigationController->GetSlice()->GetPos();
  }

  return sliceIndex;
}


//-----------------------------------------------------------------------------
void niftk::BaseController::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}


//-----------------------------------------------------------------------------
void niftk::BaseController::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}


//-----------------------------------------------------------------------------
void niftk::BaseController::BusyCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
}


//-----------------------------------------------------------------------------
void niftk::BaseController::BusyCursorOff()
{
  this->RestoreOverrideCursor();
}


//-----------------------------------------------------------------------------
void niftk::BaseController::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}
