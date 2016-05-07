/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef MITKSegmentationView_h
#define MITKSegmentationView_h

#include <QmitkMIDASBaseSegmentationFunctionality.h>
#include "ui_MITKSegmentationViewControls.h"
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

/*!
 * \class MITKSegmentationView
 * \brief A cut down version of the MITK Segmentation plugin
 * \sa QmitkMIDASBaseSegmentationFunctionality
 * \ingroup uk_ac_ucl_cmic_mitksegmentation_internal
*/
class MITKSegmentationView : public QmitkMIDASBaseSegmentationFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
public:

  /// \brief Constructor.
  MITKSegmentationView();

  /// \brief Copy constructor which deliberately throws a runtime exception, as no-one should call it.
  MITKSegmentationView(const MITKSegmentationView& other);

  /// \brief Destructor.
  virtual ~MITKSegmentationView();

  /// \brief Each View for a plugin has its own globally unique ID, this one is
  /// "uk.ac.ucl.cmic.mitksegmentation" and the .cxx file and plugin.xml should match.
  static const std::string VIEW_ID;

  /// \brief Returns the VIEW_ID = "uk.ac.ucl.cmic.mitksegmentation".
  virtual std::string GetViewID() const;

protected slots:

  /// \brief Qt slot called when the user hits the button "New segmentation", which creates the new image.
  void OnCreateNewSegmentationButtonPressed();

protected:

  /// \brief Called by framework, this method creates all the controls for this view.
  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Called by framework, this method can set the focus on a specific widget,
  /// but we currently do nothing.
  virtual void SetFocus();

  /// \brief Creates the connections of widgets in this class to the slots in this class.
  virtual void CreateConnections();

  /// \brief Retrieve's the pref values from preference service, and store locally.
  virtual void RetrievePreferenceValues();

  /// \brief Method to enable derived classes to turn all widgets off/on to signify
  /// when the view is considered enabled/disabled.
  ///
  /// \see QmitkMIDASBaseSegmentation::EnableSegmentationWidgets
  virtual void EnableSegmentationWidgets(bool b);

  /// \brief For MITK Editing, a Segmentation image should be binary and have a grey scale parent.
  virtual bool IsNodeASegmentationImage(const mitk::DataNode::Pointer node);

  /// \brief We return true if the segmentation can be "re-started", i.e. you switch between binary images
  /// in the DataManager, and if the image is a binary image with a grey scale parent, will return true,
  /// indicating that it is a valid thing to be segmenting.
  virtual bool CanStartSegmentationForBinaryNode(const mitk::DataNode::Pointer node);

  /// \brief Returns the name of the preferences node to look up.
  /// \see QmitkMIDASBaseSegmentationFunctionality::GetPreferencesNodeName
  virtual QString GetPreferencesNodeName() { return "/uk.ac.ucl.cmic.mitksegmentation"; }

private:

  /// \brief Class to contain all the specific controls for this view, apart from those in the base class.
  Ui::MITKSegmentationViewControls *m_Controls;

  /// \brief Used to put the base class widgets, and these widgets above in a common layout.
  QGridLayout *m_Layout;

  /// \brief Container for the Morphological Controls Widgets. \see QmitkMIDASBaseSegmentationFunctionality
  QWidget *m_ContainerForControlsWidget;

  /// \brief When a binary volume is created, this flag decides if it is an outline (the default), or an overlay.
  bool m_OutlineBinary;

  /// \brief When a binary volume is created, this flag determines if we are volume rendering (default to false).
  bool m_VolumeRendering;
};

#endif // MITKSegmentationView_h
