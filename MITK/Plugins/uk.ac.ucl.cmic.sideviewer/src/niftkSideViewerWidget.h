/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSideViewerWidget_h
#define niftkSideViewerWidget_h

#include <uk_ac_ucl_cmic_sideviewer_Export.h>

#include <berryIPartListener.h>

#include <QMap>
#include <QWidget>

#include <niftkDataNodeVisibilityTracker.h>
#include <niftkDataNodeStringPropertyFilter.h>
#include <niftkDnDDisplayEnums.h>

namespace mitk
{
class BaseRenderer;
class DataStorage;
class IRenderWindowPart;
class RenderingManager;
class SliceNavigationController;
}

class QmitkRenderWindow;

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QRadioButton;
class QSpinBox;


namespace niftk
{
class BaseView;
class SingleViewerWidget;


/**
 * \class SideViewerWidget
 * \brief Qt Widget to provide a single SingleViewerWidget, and some associated
 * buttons controlling 2/3 view, vertical/horizontal and axial/coronal/sagittal/ortho.
 *
 * The widget will display whatever data nodes are visible in the currently focused
 * render window, not including this widget. This means:
 *
 * <pre>
 * 1. If this widget visible, when new data is added to the data storage, defaults to not-visible in this viewer.
 * 2. When the focus changes, get the current editor axial, sagittal, coronal view:
 *     a. update visibility properties so that whatever is visible in main editor is visible in this widget.
 * </pre>
 */
class SIDEVIEWER_EXPORT SideViewerWidget : public QWidget
{
  Q_OBJECT

public:

  /// \brief Constructs a SideViewerWidget object.
  ///
  /// \param view Sets the containing view for callback purposes.
  ///
  ///       The reason we do this, is so that we can ask QmitkAbstractView for the mitkIRenderWindowPart
  ///       rather than have any hard coded reference to any widget such as DnDMultiWindowWidget.
  ///
  SideViewerWidget(BaseView* view, QWidget* parent, mitk::RenderingManager* renderingManager);

  /// \brief Destructs the SideViewerWidget object.
  virtual ~SideViewerWidget();

  void SetFocused();

  /// \brief Called when the world geometry of main window changes and updates the viewer accordingly.
  void SetGeometry(const itk::EventObject& geometrySendEvent);

  /// \brief Sets the selected render window of the main display.
  /// This view then might need to change its window layout so that it shows the image
  /// of a different orientation.
  /// \param renderWindowPart The render window part (aka. editor or display) that contins the window
  /// \param mainWindow The selected render window of the main display.
  void OnMainWindowChanged(mitk::IRenderWindowPart* renderWindowPart, QmitkRenderWindow* mainWindow);

  /// \brief Gets the viewer contained by this widget.
  SingleViewerWidget* GetViewer() const;

protected slots:

  /// \brief Called when the axial window layout radio button is toggled.
  void OnAxialWindowRadioButtonToggled(bool checked);

  /// \brief Called when the sagittal window layout radio button is toggled.
  void OnSagittalWindowRadioButtonToggled(bool checked);

  /// \brief Called when the coronal window layout radio button is toggled.
  void OnCoronalWindowRadioButtonToggled(bool checked);

  /// \brief Called when the multi window layout radio button is toggled.
  void OnMultiWindowRadioButtonToggled(bool checked);

  /// \brief Called when the window layout is selected in the the combo box.
  void OnMultiWindowComboBoxIndexChanged();

  /// \brief Called when the slice is changed by the spin box.
  void OnSliceSpinBoxValueChanged(int slice);

  /// \brief Called when the magnification is changed by the spin box.
  void OnMagnificationSpinBoxValueChanged(double magnification);

  /// \brief Called when the scale factor is changed by zooming in a renderer window.
  void OnSelectedPositionChanged(const mitk::Point3D& selectedPosition);

  /// \brief Called when the scale factor is changed by zooming in a renderer window.
  void OnScaleFactorChanged(WindowOrientation orientation, double scaleFactor);

  /// \brief Called when the window layout of the viewer has changed through interaction.
  void OnWindowLayoutChanged(WindowLayout windowLayout);

private:

  /// \brief Callback for when the focus changes, where we update the geometry to match the right window.
  void OnFocusChanged();

  /// \brief Works out a multi window orientation for the given orientation of the main window.
  WindowLayout GetMultiWindowLayoutForOrientation(WindowOrientation mainWindowOrientation);

  /// \brief Gets the currently visible editor.
  /// Returns 0 if no editor is opened.
  mitk::IRenderWindowPart* GetSelectedEditor();

  /// \brief Updates the slice and magnification spin boxes according to the selected window.
  void OnViewerWindowChanged();

  /// \brief Method that actually changes the layout to axial, sagittal, coronal etc.
  void OnMainWindowOrientationChanged(WindowOrientation orientation);

private slots:

  /// \brief Called when one of the main 2D windows has been destroyed.
  virtual void OnAMainWindowDestroyed(QObject* mainWindow);

  /// \brief Fits the displayed region to the size of the viewer window.
  void FitToDisplay();

private:

  void SetupUi(QWidget* parent);

  /// \brief The view that contains this widget.
  BaseView* m_ContainingView;

  /// \brief The identifier of the focus change listener.
  unsigned long m_FocusManagerObserverTag;

  /// \brief Stores the currently selected window layout.
  WindowLayout m_WindowLayout;

  /// \brief Rendering manager of the tracked main window.
  /// If a main window is being tracked, the render windows of this viewer should be added
  /// to the rendering manager of the tracked window, so that they are immediately updated
  /// any time when the contents of the main window changes.
  mitk::RenderingManager* m_MainRenderingManager;

  /// \brief Stores the currently selected window of the visible editor, if any.
  QmitkRenderWindow* m_MainWindow;

  /// \brief The axial window of the selected editor.
  QmitkRenderWindow* m_MainAxialWindow;

  /// \brief The sagittal window of the selected editor.
  QmitkRenderWindow* m_MainSagittalWindow;

  /// \brief The coronal window of the selected editor.
  QmitkRenderWindow* m_MainCoronalWindow;

  /// \brief The slice navigation controller of the currently selected window  of the selected editor.
  mitk::SliceNavigationController* m_MainWindowSnc;

  /// \brief The slice navigation controller of the axial window of the selected editor.
  mitk::SliceNavigationController* m_MainAxialSnc;

  /// \brief The slice navigation controller of the sagittal window of the selected editor.
  mitk::SliceNavigationController* m_MainSagittalSnc;

  /// \brief The slice navigation controller of the coronal window of the selected editor.
  mitk::SliceNavigationController* m_MainCoronalSnc;

  DataNodeVisibilityTracker::Pointer m_VisibilityTracker;

  /// \brief Filter that tells which nodes should not be handled.
  DataNodeStringPropertyFilter::Pointer m_ToolNodeNameFilter;

  /// \brief The current magnification in the selected window of the viewer in this widget.
  /// It is used to increase or decrease the magnification value to the closest integers
  /// when using the magnification spin box.
  double m_Magnification;

  /// \brief The orientation of the selected window of the main display.
  WindowOrientation m_MainWindowOrientation;

  /// \brief Stores the last single window layout of the internal viewer,
  /// one for each orientation of the main window.
  QMap<WindowOrientation, WindowLayout> m_SingleWindowLayouts;

  /// \brief The world geometry of the selected window of the selected editor.
  /// Any time when the selected main window changes, the world geometry of this viewer
  /// needs to be set to that of the main window.
  const mitk::TimeGeometry* m_TimeGeometry;

  /// \brief Listener to catch events when an editor becomes visible or gets destroyed.
  QScopedPointer<berry::IPartListener> m_EditorLifeCycleListener;

  SingleViewerWidget* m_Viewer;
  QWidget* m_ControlsWidget;
  QWidget* m_LayoutWidget;
  QRadioButton* m_CoronalWindowRadioButton;
  QRadioButton* m_SagittalWindowRadioButton;
  QRadioButton* m_AxialWindowRadioButton;
  QRadioButton* m_MultiWindowRadioButton;
  QComboBox* m_MultiWindowComboBox;
  QLabel* m_SliceLabel;
  QSpinBox* m_SliceSpinBox;
  QLabel* m_MagnificationLabel;
  QDoubleSpinBox* m_MagnificationSpinBox;

  mitk::RenderingManager* m_RenderingManager;
};

}

#endif
