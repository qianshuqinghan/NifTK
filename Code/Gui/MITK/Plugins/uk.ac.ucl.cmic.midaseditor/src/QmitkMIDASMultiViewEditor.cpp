/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-12-15 15:03:56 +0000 (Thu, 15 Dec 2011) $
 Revision          : $Revision: 8030 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkMIDASMultiViewEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>

#include <QWidget>
#include <QGridLayout>

#include "mitkGlobalInteraction.h"
#include "mitkIDataStorageService.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkMIDASDataStorageEditorInput.h"

#include "QmitkMIDASMultiViewWidget.h"
#include "QmitkMIDASMultiViewVisibilityManager.h"
#include "QmitkMIDASMultiViewEditorPreferencePage.h"

const std::string QmitkMIDASMultiViewEditor::EDITOR_ID = "org.mitk.editors.midasmultiview";

class QmitkMIDASMultiViewEditorPrivate
{
public:
  QmitkMIDASMultiViewEditorPrivate();
  ~QmitkMIDASMultiViewEditorPrivate();
  mitk::MIDASViewKeyPressStateMachine::Pointer m_ViewKeyPressStateMachine;
  QmitkMIDASMultiViewWidget* m_MIDASMultiViewWidget;
  QmitkMIDASMultiViewVisibilityManager* m_MidasMultiViewVisibilityManager;
  mitk::RenderingManager::Pointer m_RenderingManager;
  berry::IPartListener::Pointer m_PartListener;
};

struct QmitkMIDASMultiViewEditorPartListener : public berry::IPartListener
{
  berryObjectMacro(QmitkMIDASMultiViewEditorPartListener)

  QmitkMIDASMultiViewEditorPartListener(QmitkMIDASMultiViewEditorPrivate* dd)
    : d(dd)
  {}

  Events::Types GetPartEventTypes() const
  {
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
  }

  void PartClosed (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkMIDASMultiViewEditor::EDITOR_ID)
    {
      QmitkMIDASMultiViewEditor::Pointer midasMultiViewEditor = partRef->GetPart(false).Cast<QmitkMIDASMultiViewEditor>();

      if (midasMultiViewEditor.IsNotNull()
        && midasMultiViewEditor->GetMIDASMultiViewWidget() == d->m_MIDASMultiViewWidget)
      {
        d->m_MIDASMultiViewWidget->Deactivated();
      }
    }
  }

  void PartHidden (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkMIDASMultiViewEditor::EDITOR_ID)
    {
      QmitkMIDASMultiViewEditor::Pointer midasMultiViewEditor = partRef->GetPart(false).Cast<QmitkMIDASMultiViewEditor>();

      if (midasMultiViewEditor.IsNotNull()
        && midasMultiViewEditor->GetMIDASMultiViewWidget() == d->m_MIDASMultiViewWidget)
      {
        d->m_MIDASMultiViewWidget->Deactivated();
      }
    }
  }

  void PartVisible (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkMIDASMultiViewEditor::EDITOR_ID)
    {
      QmitkMIDASMultiViewEditor::Pointer midasMultiViewEditor = partRef->GetPart(false).Cast<QmitkMIDASMultiViewEditor>();

      if (midasMultiViewEditor.IsNotNull()
        && midasMultiViewEditor->GetMIDASMultiViewWidget() == d->m_MIDASMultiViewWidget)
      {
        d->m_MIDASMultiViewWidget->Activated();
      }
    }
  }

private:

  QmitkMIDASMultiViewEditorPrivate* const d;

};

QmitkMIDASMultiViewEditorPrivate::QmitkMIDASMultiViewEditorPrivate()
: m_ViewKeyPressStateMachine(0)
, m_MIDASMultiViewWidget(0)
, m_MidasMultiViewVisibilityManager(0)
, m_RenderingManager(0)
, m_PartListener(new QmitkMIDASMultiViewEditorPartListener(this))
{
  m_RenderingManager = mitk::RenderingManager::GetInstance();
  m_RenderingManager->SetConstrainedPaddingZooming(false);
}

QmitkMIDASMultiViewEditorPrivate::~QmitkMIDASMultiViewEditorPrivate()
{
  if (m_MidasMultiViewVisibilityManager != NULL)
  {
    delete m_MidasMultiViewVisibilityManager;
  }
}

QmitkMIDASMultiViewEditor::QmitkMIDASMultiViewEditor()
: d(new QmitkMIDASMultiViewEditorPrivate)
{
}

QmitkMIDASMultiViewEditor::~QmitkMIDASMultiViewEditor()
{
  this->GetSite()->GetPage()->RemovePartListener(d->m_PartListener);
}

void QmitkMIDASMultiViewEditor::CreateQtPartControl(QWidget* parent)
{
  if (d->m_MIDASMultiViewWidget == NULL)
  {
    mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
    assert(dataStorage);

    berry::IPreferencesService::Pointer prefService = berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
    berry::IBerryPreferences::Pointer prefs = (prefService->GetSystemPreferences()->Node(EDITOR_ID)).Cast<berry::IBerryPreferences>();
    assert( prefs );

    MIDASDefaultInterpolationType defaultInterpolation =
        (MIDASDefaultInterpolationType)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_IMAGE_INTERPOLATION, 2));
    MIDASView defaultView =
        (MIDASView)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_VIEW, 7));
    MIDASDropType defaultDropType =
        (MIDASDropType)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_DROP_TYPE, 0));

    int defaultNumberOfRows = prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_NUMBER_ROWS, 1);
    int defaultNumberOfColumns = prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_NUMBER_COLUMNS, 1);
    bool showDropTypeWidgets = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_DROP_TYPE_WIDGETS, false);
    bool showLayoutButtons = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_LAYOUT_BUTTONS, true);
    bool showMagnificationSlider = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_MAGNIFICATION_SLIDER, true);
    bool show3DViewInOrthoView = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_3D_VIEW_IN_ORTHOVIEW, false);
    bool show2DCursors = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_2D_CURSORS, true);
    bool rememberViewSettingsPerOrientation = prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_REMEMBER_VIEW_SETTINGS_PER_ORIENTATION, true);

    d->m_MidasMultiViewVisibilityManager = new QmitkMIDASMultiViewVisibilityManager(dataStorage);
    d->m_MidasMultiViewVisibilityManager->SetDefaultInterpolationType(defaultInterpolation);
    d->m_MidasMultiViewVisibilityManager->SetDefaultViewType(defaultView);
    d->m_MidasMultiViewVisibilityManager->SetDropType(defaultDropType);

    d->m_RenderingManager->SetDataStorage(dataStorage);

    // Create the QmitkMIDASMultiViewWidget
    d->m_MIDASMultiViewWidget = new QmitkMIDASMultiViewWidget(
        d->m_MidasMultiViewVisibilityManager,
        d->m_RenderingManager,
        dataStorage,
        defaultNumberOfRows,
        defaultNumberOfColumns,
        parent);

    // Setup GUI a bit more.
    d->m_MIDASMultiViewWidget->SetDropTypeWidget(defaultDropType);
    d->m_MIDASMultiViewWidget->SetShowDropTypeWidgets(showDropTypeWidgets);
    d->m_MIDASMultiViewWidget->SetShowLayoutButtons(showLayoutButtons);
    d->m_MIDASMultiViewWidget->SetShow2DCursors(show2DCursors);
    d->m_MIDASMultiViewWidget->SetShow3DViewInOrthoView(show3DViewInOrthoView);
    d->m_MIDASMultiViewWidget->SetShowMagnificationSlider(showMagnificationSlider);
    d->m_MIDASMultiViewWidget->SetRememberViewSettingsPerOrientation(rememberViewSettingsPerOrientation);

    this->GetSite()->GetPage()->AddPartListener(berry::IPartListener::Pointer(d->m_PartListener));

    QGridLayout *gridLayout = new QGridLayout(parent);
    gridLayout->addWidget(d->m_MIDASMultiViewWidget, 0, 0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);

    prefs->OnChanged.AddListener( berry::MessageDelegate1<QmitkMIDASMultiViewEditor, const berry::IBerryPreferences*>( this, &QmitkMIDASMultiViewEditor::OnPreferencesChanged ) );
    this->OnPreferencesChanged(prefs.GetPointer());

    // Create/Connect the state machine
    d->m_ViewKeyPressStateMachine = mitk::MIDASViewKeyPressStateMachine::New("MIDASKeyPressStateMachine", d->m_MIDASMultiViewWidget);
    mitk::GlobalInteraction::GetInstance()->AddListener( d->m_ViewKeyPressStateMachine );
  }
}

QmitkMIDASMultiViewWidget* QmitkMIDASMultiViewEditor::GetMIDASMultiViewWidget()
{
  return d->m_MIDASMultiViewWidget;
}

void QmitkMIDASMultiViewEditor::SetFocus()
{
  if (d->m_MIDASMultiViewWidget != 0)
  {
    d->m_MIDASMultiViewWidget->setFocus();
  }
}

void QmitkMIDASMultiViewEditor::OnPreferencesChanged( const berry::IBerryPreferences* prefs )
{
  if (d->m_MIDASMultiViewWidget != NULL)
  {
    QString backgroundColourName = QString::fromStdString (prefs->GetByteArray(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_BACKGROUND_COLOUR, ""));
    QColor backgroundColour(backgroundColourName);
    mitk::Color bgColour;
    if (backgroundColourName=="") // default values
    {
      bgColour[0] = 0;
      bgColour[1] = 0;
      bgColour[2] = 0;
    }
    else
    {
      bgColour[0] = backgroundColour.red() / 255.0;
      bgColour[1] = backgroundColour.green() / 255.0;
      bgColour[2] = backgroundColour.blue() / 255.0;
    }
    d->m_MIDASMultiViewWidget->SetBackgroundColour(bgColour);
    d->m_MIDASMultiViewWidget->SetDefaultInterpolationType((MIDASDefaultInterpolationType)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_IMAGE_INTERPOLATION, 2)));
    d->m_MIDASMultiViewWidget->SetDefaultViewType((MIDASView)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_VIEW, 7)));
    d->m_MIDASMultiViewWidget->SetDropTypeWidget((MIDASDropType)(prefs->GetInt(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_DROP_TYPE, 0)));
    d->m_MIDASMultiViewWidget->SetShowDropTypeWidgets(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_DROP_TYPE_WIDGETS, false));
    d->m_MIDASMultiViewWidget->SetShowLayoutButtons(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_LAYOUT_BUTTONS, true));
    d->m_MIDASMultiViewWidget->SetShowMagnificationSlider(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_MAGNIFICATION_SLIDER, true));
    d->m_MIDASMultiViewWidget->SetShow2DCursors(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_2D_CURSORS, true));
    d->m_MIDASMultiViewWidget->SetShow3DViewInOrthoView(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_SHOW_3D_VIEW_IN_ORTHOVIEW, false));
    d->m_MIDASMultiViewWidget->SetRememberViewSettingsPerOrientation(prefs->GetBool(QmitkMIDASMultiViewEditorPreferencePage::MIDAS_REMEMBER_VIEW_SETTINGS_PER_ORIENTATION, true));
  }
}

// -------------------  mitk::IRenderWindowPart  ----------------------

QmitkRenderWindow *QmitkMIDASMultiViewEditor::GetActiveRenderWindow() const
{
  return d->m_MIDASMultiViewWidget->GetActiveRenderWindow();
}

QHash<QString, QmitkRenderWindow *> QmitkMIDASMultiViewEditor::GetRenderWindows() const
{
  return d->m_MIDASMultiViewWidget->GetRenderWindows();
}

QmitkRenderWindow *QmitkMIDASMultiViewEditor::GetRenderWindow(const QString &id) const
{
  return d->m_MIDASMultiViewWidget->GetRenderWindow(id);
}

mitk::Point3D QmitkMIDASMultiViewEditor::GetSelectedPosition(const QString &id) const
{
  return d->m_MIDASMultiViewWidget->GetSelectedPosition(id);
}

void QmitkMIDASMultiViewEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &id)
{
  return d->m_MIDASMultiViewWidget->SetSelectedPosition(pos, id);
}

void QmitkMIDASMultiViewEditor::EnableDecorations(bool enable, const QStringList &decorations)
{
  // Deliberately do nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
}

bool QmitkMIDASMultiViewEditor::IsDecorationEnabled(const QString &decoration) const
{
  // Deliberately deny having any decorations. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  return false;
}

QStringList QmitkMIDASMultiViewEditor::GetDecorations() const
{
  // Deliberately return nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  QStringList decorations;
  return decorations;
}

mitk::IRenderingManager* QmitkMIDASMultiViewEditor::GetRenderingManager() const
{
  return mitk::MakeRenderingManagerInterface(d->m_RenderingManager);
}

void QmitkMIDASMultiViewEditor::EnableInteractors(bool enable, const QStringList& interactors)
{
  d->m_MIDASMultiViewWidget->EnableInteractors(enable, interactors);
}

bool QmitkMIDASMultiViewEditor::IsInteractorEnabled(const QString& interactor) const
{
  return d->m_MIDASMultiViewWidget->IsInteractorEnabled(interactor);
}

QStringList QmitkMIDASMultiViewEditor::GetInteractors() const
{
  return d->m_MIDASMultiViewWidget->GetInteractors();
}

mitk::SlicesRotator* QmitkMIDASMultiViewEditor::GetSlicesRotator() const
{
  // Deliberately return nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  return NULL;
}

mitk::SlicesSwiveller* QmitkMIDASMultiViewEditor::GetSlicesSwiveller() const
{
  // Deliberately return nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  return NULL;
}

void QmitkMIDASMultiViewEditor::EnableSlicingPlanes(bool enable)
{
  // Deliberately do nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  Q_UNUSED(enable);
}

bool QmitkMIDASMultiViewEditor::IsSlicingPlanesEnabled() const
{
  // Deliberately do nothing. ToDo - maybe get QmitkMIDASMultiViewWidget to support it.
  return false;
}

void QmitkMIDASMultiViewEditor::EnableLinkedNavigation(bool enable)
{
  d->m_MIDASMultiViewWidget->EnableLinkedNavigation(enable);
}

bool QmitkMIDASMultiViewEditor::IsLinkedNavigationEnabled() const
{
  return d->m_MIDASMultiViewWidget->IsLinkedNavigationEnabled();
}


