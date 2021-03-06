/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCameraCalViewPreferencePage.h"
#include "niftkCameraCalView.h"
#include "ui_niftkCameraCalViewPreferencePage.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <niftkNiftyCalVideoCalibrationManager.h>
#include <QmitkIGIUtils.h>

#include <QMessageBox>
#include <QFileDialog>

namespace niftk
{
const QString CameraCalViewPreferencePage::DO_ITERATIVE_NODE_NAME("iterative");
const QString CameraCalViewPreferencePage::DO_3D_OPTIMISATION_NODE_NAME("optimise in 3D");
const QString CameraCalViewPreferencePage::DO_CLUSTERING_NODE_NAME("clustering");
const QString CameraCalViewPreferencePage::NUMBER_VIEWS_NODE_NAME("required number of views");
const QString CameraCalViewPreferencePage::MODEL_NODE_NAME("3D model points");
const QString CameraCalViewPreferencePage::SCALEX_NODE_NAME("scale factor in x to resize image");
const QString CameraCalViewPreferencePage::SCALEY_NODE_NAME("scale factor in y to resize image");
const QString CameraCalViewPreferencePage::PATTERN_NODE_NAME("pattern");
const QString CameraCalViewPreferencePage::TAG_FAMILY_NODE_NAME("tag family");
const QString CameraCalViewPreferencePage::GRIDX_NODE_NAME("grid size in x");
const QString CameraCalViewPreferencePage::GRIDY_NODE_NAME("grid size in y");
const QString CameraCalViewPreferencePage::HANDEYE_NODE_NAME("handeye method");
const QString CameraCalViewPreferencePage::MODEL_TRANSFORM_NODE_NAME("model transform");
const QString CameraCalViewPreferencePage::REFERENCE_IMAGE_NODE_NAME("reference image");
const QString CameraCalViewPreferencePage::REFERENCE_POINTS_NODE_NAME("reference points");
const QString CameraCalViewPreferencePage::MINIMUM_NUMBER_POINTS_NODE_NAME("minimum number of points");
const QString CameraCalViewPreferencePage::TEMPLATE_IMAGE_NODE_NAME("template image");
const QString CameraCalViewPreferencePage::PREVIOUS_CALIBRATION_DIR_NODE_NAME("previous calibration directory");
const QString CameraCalViewPreferencePage::OUTPUT_DIR_NODE_NAME("output directory");
const QString CameraCalViewPreferencePage::MODEL_IS_STATIONARY_NODE_NAME("model is stationary");
const QString CameraCalViewPreferencePage::CAMERA_IS_STATIONARY_NODE_NAME("camera is stationary");
const QString CameraCalViewPreferencePage::SAVE_OUTPUT_BEFORE_CALIBRATION_NODE_NAME("save output before calibration");
const QString CameraCalViewPreferencePage::RESET_CALIBRATION_IF_NODE_CHANGES_NODE_NAME("reset if node changes");

//-----------------------------------------------------------------------------
CameraCalViewPreferencePage::CameraCalViewPreferencePage()
: m_Control(nullptr)
, m_Ui(new Ui::CameraCalViewPreferencePage)
, m_Initializing(false)
, m_CameraCalViewPreferencesNode(NULL)
{
}


//-----------------------------------------------------------------------------
CameraCalViewPreferencePage::CameraCalViewPreferencePage(const CameraCalViewPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
CameraCalViewPreferencePage::~CameraCalViewPreferencePage()
{
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::Init(berry::IWorkbench::Pointer )
{
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;

  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  m_CameraCalViewPreferencesNode = prefService->GetSystemPreferences()->Node(niftk::CameraCalView::VIEW_ID);

  m_Ui->m_FeaturesComboBox->addItem("OpenCV chess board (planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::CHESS_BOARD));
  m_Ui->m_FeaturesComboBox->addItem("OpenCV asymmetric circle grid (planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::CIRCLE_GRID));
  m_Ui->m_FeaturesComboBox->addItem("April Tags (planar or non-planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::APRIL_TAGS));
  m_Ui->m_FeaturesComboBox->addItem("Template matching asymmetric circles (planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_CIRCLES));
  m_Ui->m_FeaturesComboBox->addItem("Template matching asymmetric rings (planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_RINGS));
  m_Ui->m_FeaturesComboBox->addItem("Symmetric template matching asymmetric circles (non-planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_CIRCLES));
  m_Ui->m_FeaturesComboBox->addItem("Symmetric template matching asymmetric rings (non-planar)", QVariant(niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_RINGS));
  m_Ui->m_TagFamilyComboBox->addItem("16h5");
  m_Ui->m_TagFamilyComboBox->addItem("25h7");
  m_Ui->m_TagFamilyComboBox->addItem("25h9");
  m_Ui->m_TagFamilyComboBox->addItem("36h9");
  m_Ui->m_TagFamilyComboBox->addItem("36h11");

  m_Ui->m_HandEyeComboBox->addItem("Tsai 1989", QVariant(niftk::NiftyCalVideoCalibrationManager::TSAI_1989));
  m_Ui->m_HandEyeComboBox->addItem("Shahidi 2002", QVariant(niftk::NiftyCalVideoCalibrationManager::SHAHIDI_2002));
  m_Ui->m_HandEyeComboBox->addItem("Malti 2013", QVariant(niftk::NiftyCalVideoCalibrationManager::MALTI_2013));
  m_Ui->m_HandEyeComboBox->addItem("Non-linear, extrinsic.", QVariant(niftk::NiftyCalVideoCalibrationManager::NON_LINEAR_EXTRINSIC));

  bool ok = false;
  ok = connect(m_Ui->m_FeaturesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFeaturesComboSelected()));
  assert(ok);
  ok = connect(m_Ui->m_3DModelToolButton, SIGNAL(pressed()), this, SLOT(On3DModelButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_ModelTransformToolButton, SIGNAL(pressed()), this, SLOT(OnModelTransformButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_ReferenceImagePushButton, SIGNAL(pressed()), this, SLOT(OnReferenceImageButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_ReferencePointsPushButton, SIGNAL(pressed()), this, SLOT(OnReferencePointsButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_TemplateImagePushButton, SIGNAL(pressed()), this, SLOT(OnTemplateImageButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_PreviousCalibrationDirToolButton, SIGNAL(pressed()), this, SLOT(OnPreviousCalibrationDirButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_OutputDirToolButton, SIGNAL(pressed()), this, SLOT(OnOutputDirButtonPressed()));
  assert(ok);
  ok = connect(m_Ui->m_IterativeCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnDoIterativeChecked(bool)));
  assert(ok);
  ok = connect(m_Ui->m_ModelIsStationaryCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnModelIsStationaryChecked(bool)));
  assert(ok);
  ok = connect(m_Ui->m_CameraIsStationaryCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnCameraIsStationaryChecked(bool)));
  assert(ok);

  m_Ui->m_FeaturesComboBox->setCurrentIndex(0);
  m_Ui->m_TagFamilyComboBox->setCurrentIndex(1);
  m_Ui->m_IterativeCheckBox->setChecked(false);
  m_Ui->m_Do3DOptimisationCheckBox->setChecked(false);
  m_Ui->m_ClusteringCheckBox->setChecked(false);
  m_Ui->m_HandEyeComboBox->setCurrentIndex(0);
  m_Ui->m_ModelIsStationaryCheckBox->setChecked(true);
  m_Ui->m_CameraIsStationaryCheckBox->setChecked(false);
  m_Ui->m_CameraIsStationaryCheckBox->setVisible(false);
  m_Ui->m_CameraIsStationaryLabel->setVisible(false);
  m_Ui->m_SaveOutputBeforeCalibrationCheckBox->setChecked(false);
  this->OnDoIterativeChecked(false);
  this->OnFeaturesComboSelected();

  this->Update();

  m_Initializing = false;
}


//-----------------------------------------------------------------------------
QWidget* CameraCalViewPreferencePage::GetQtControl() const
{
  return m_Control;
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::UpdateReferenceImageVisibility()
{
  bool isVisible = false;
  if (   m_Ui->m_IterativeCheckBox->isChecked()
      || m_Ui->m_FeaturesComboBox->currentIndex() == niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_CIRCLES
      || m_Ui->m_FeaturesComboBox->currentIndex() == niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_RINGS
      || m_Ui->m_FeaturesComboBox->currentIndex() == niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_CIRCLES
      || m_Ui->m_FeaturesComboBox->currentIndex() == niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_RINGS
      )
  {
    isVisible = true;
  }

  m_Ui->m_ReferenceImageLabel->setVisible(isVisible);
  m_Ui->m_ReferenceImageLineEdit->setVisible(isVisible);
  m_Ui->m_ReferenceImagePushButton->setVisible(isVisible);
  m_Ui->m_ReferencePointsLabel->setVisible(isVisible);
  m_Ui->m_ReferencePointsLineEdit->setVisible(isVisible);
  m_Ui->m_ReferencePointsPushButton->setVisible(isVisible);
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnDoIterativeChecked(bool)
{
  this->UpdateReferenceImageVisibility();
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnModelIsStationaryChecked(bool isChecked)
{
  if(m_Ui->m_CameraIsStationaryCheckBox->isChecked() && isChecked)
  {
    m_Ui->m_ModelIsStationaryCheckBox->setChecked(false);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnCameraIsStationaryChecked(bool isChecked)
{
  if(m_Ui->m_ModelIsStationaryCheckBox->isChecked() && isChecked)
  {
    m_Ui->m_CameraIsStationaryCheckBox->setChecked(false);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnFeaturesComboSelected()
{
  switch(m_Ui->m_FeaturesComboBox->currentIndex())
  {
    case niftk::NiftyCalVideoCalibrationManager::CHESS_BOARD:
    case niftk::NiftyCalVideoCalibrationManager::CIRCLE_GRID:
      m_Ui->m_GridSizeLabel->setVisible(true);
      m_Ui->m_GridPointsInXSpinBox->setVisible(true);
      m_Ui->m_ByLabel->setVisible(true);
      m_Ui->m_GridPointsInYSpinBox->setVisible(true);
      m_Ui->m_TagFamilyLabel->setVisible(false);
      m_Ui->m_TagFamilyComboBox->setVisible(false);
      m_Ui->m_MinPointsLabel->setVisible(false);
      m_Ui->m_MinPointsSpinBox->setVisible(false);
      m_Ui->m_TemplateImageLabel->setVisible(false);
      m_Ui->m_TemplateImageLineEdit->setVisible(false);
      m_Ui->m_TemplateImagePushButton->setVisible(false);
    break;
    case niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_CIRCLES:
    case niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_RINGS:
    case niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_CIRCLES:
    case niftk::NiftyCalVideoCalibrationManager::TEMPLATE_MATCHING_NON_COPLANAR_RINGS:
      m_Ui->m_GridSizeLabel->setVisible(true);
      m_Ui->m_GridPointsInXSpinBox->setVisible(true);
      m_Ui->m_ByLabel->setVisible(true);
      m_Ui->m_GridPointsInYSpinBox->setVisible(true);
      m_Ui->m_TagFamilyLabel->setVisible(false);
      m_Ui->m_TagFamilyComboBox->setVisible(false);
      m_Ui->m_MinPointsLabel->setVisible(false);
      m_Ui->m_MinPointsSpinBox->setVisible(false);      
      m_Ui->m_TemplateImageLabel->setVisible(true);
      m_Ui->m_TemplateImageLineEdit->setVisible(true);
      m_Ui->m_TemplateImagePushButton->setVisible(true);
    break;
    case niftk::NiftyCalVideoCalibrationManager::APRIL_TAGS:
      m_Ui->m_GridSizeLabel->setVisible(false);
      m_Ui->m_GridPointsInXSpinBox->setVisible(false);
      m_Ui->m_ByLabel->setVisible(false);
      m_Ui->m_GridPointsInYSpinBox->setVisible(false);
      m_Ui->m_TagFamilyLabel->setVisible(true);
      m_Ui->m_TagFamilyComboBox->setVisible(true);
      m_Ui->m_MinPointsLabel->setVisible(true);
      m_Ui->m_MinPointsSpinBox->setVisible(true);
      m_Ui->m_TemplateImageLabel->setVisible(false);
      m_Ui->m_TemplateImageLineEdit->setVisible(false);
      m_Ui->m_TemplateImagePushButton->setVisible(false);
    break;
  }
  switch(m_Ui->m_FeaturesComboBox->currentIndex())
  {
    case niftk::NiftyCalVideoCalibrationManager::CHESS_BOARD:
    case niftk::NiftyCalVideoCalibrationManager::APRIL_TAGS:
      m_Ui->m_ClusteringLabel->setVisible(false);
      m_Ui->m_ClusteringCheckBox->setVisible(false);
    break;
    default:
      m_Ui->m_ClusteringLabel->setVisible(true);
      m_Ui->m_ClusteringCheckBox->setVisible(true);
    break;
  }

  this->UpdateReferenceImageVisibility();
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::On3DModelButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Control,
      tr("3D Model File"), "", tr("3D Model Files (*.txt)"));

  if (!fileName.isEmpty())
  {
    m_Ui->m_3DModelLineEdit->setText(fileName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnModelTransformButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Control,
      tr("Model to tracker transform"), "", tr("Transform (*.4x4 *.txt)"));

  if (!fileName.isEmpty())
  {
    m_Ui->m_ModelTransformLineEdit->setText(fileName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnReferenceImageButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Control,
      tr("Reference image"), "", tr("Image Files (*.jpg *.png *.bmp)"));

  if (!fileName.isEmpty())
  {
    m_Ui->m_ReferenceImageLineEdit->setText(fileName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnTemplateImageButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Control,
      tr("Template image"), "", tr("Image Files (*.jpg *.png *.bmp)"));

  if (!fileName.isEmpty())
  {
    m_Ui->m_TemplateImageLineEdit->setText(fileName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnReferencePointsButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Control,
      tr("Reference points"), "", tr("2D Model Files (*.txt)"));

  if (!fileName.isEmpty())
  {
    m_Ui->m_ReferencePointsLineEdit->setText(fileName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnPreviousCalibrationDirButtonPressed()
{
  QString dirName = QFileDialog::getExistingDirectory(m_Control,
      tr("Previous Calibration"), "");

  if (!dirName.isEmpty())
  {
    m_Ui->m_PreviousCalibrationDirLineEdit->setText(dirName);
  }
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::OnOutputDirButtonPressed()
{
  QString dirName = QFileDialog::getExistingDirectory(m_Control,
      tr("Output Directory"), "");

  if (!dirName.isEmpty())
  {
    m_Ui->m_OutputDirLineEdit->setText(dirName);
  }
}


//-----------------------------------------------------------------------------
bool CameraCalViewPreferencePage::PerformOk()
{
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::DO_ITERATIVE_NODE_NAME, m_Ui->m_IterativeCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::DO_3D_OPTIMISATION_NODE_NAME, m_Ui->m_Do3DOptimisationCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::DO_CLUSTERING_NODE_NAME, m_Ui->m_ClusteringCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::MODEL_NODE_NAME, m_Ui->m_3DModelLineEdit->text());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::NUMBER_VIEWS_NODE_NAME, m_Ui->m_NumberViewsSpinBox->value());
  m_CameraCalViewPreferencesNode->PutDouble(CameraCalViewPreferencePage::SCALEX_NODE_NAME, m_Ui->m_ScaleImageInXSpinBox->value());
  m_CameraCalViewPreferencesNode->PutDouble(CameraCalViewPreferencePage::SCALEY_NODE_NAME, m_Ui->m_ScaleImageInYSpinBox->value());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::GRIDX_NODE_NAME, m_Ui->m_GridPointsInXSpinBox->value());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::GRIDY_NODE_NAME, m_Ui->m_GridPointsInYSpinBox->value());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::MODEL_TRANSFORM_NODE_NAME, m_Ui->m_ModelTransformLineEdit->text());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::REFERENCE_IMAGE_NODE_NAME, m_Ui->m_ReferenceImageLineEdit->text());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::REFERENCE_POINTS_NODE_NAME, m_Ui->m_ReferencePointsLineEdit->text());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::TAG_FAMILY_NODE_NAME, m_Ui->m_TagFamilyComboBox->currentText());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::PATTERN_NODE_NAME, m_Ui->m_FeaturesComboBox->currentIndex());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::HANDEYE_NODE_NAME, m_Ui->m_HandEyeComboBox->currentIndex());
  m_CameraCalViewPreferencesNode->PutInt(CameraCalViewPreferencePage::MINIMUM_NUMBER_POINTS_NODE_NAME, m_Ui->m_MinPointsSpinBox->value());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::TEMPLATE_IMAGE_NODE_NAME, m_Ui->m_TemplateImageLineEdit->text());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::PREVIOUS_CALIBRATION_DIR_NODE_NAME, m_Ui->m_PreviousCalibrationDirLineEdit->text());
  m_CameraCalViewPreferencesNode->Put(CameraCalViewPreferencePage::OUTPUT_DIR_NODE_NAME, m_Ui->m_OutputDirLineEdit->text());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::MODEL_IS_STATIONARY_NODE_NAME, m_Ui->m_ModelIsStationaryCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::CAMERA_IS_STATIONARY_NODE_NAME, m_Ui->m_CameraIsStationaryCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::SAVE_OUTPUT_BEFORE_CALIBRATION_NODE_NAME, m_Ui->m_SaveOutputBeforeCalibrationCheckBox->isChecked());
  m_CameraCalViewPreferencesNode->PutBool(CameraCalViewPreferencePage::RESET_CALIBRATION_IF_NODE_CHANGES_NODE_NAME, m_Ui->m_ResetCalibrationIfNodeChangesCheckBox->isChecked());
  return true;
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void CameraCalViewPreferencePage::Update()
{
  m_Ui->m_IterativeCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::DO_ITERATIVE_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultDoIterative));
  m_Ui->m_Do3DOptimisationCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::DO_3D_OPTIMISATION_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultDo3DOptimisation));
  m_Ui->m_ClusteringCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::DO_CLUSTERING_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultDoClustering));
  m_Ui->m_3DModelLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::MODEL_NODE_NAME, ""));
  m_Ui->m_NumberViewsSpinBox->setValue(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::NUMBER_VIEWS_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultNumberOfSnapshotsForCalibrating));
  m_Ui->m_ScaleImageInXSpinBox->setValue(m_CameraCalViewPreferencesNode->GetDouble(CameraCalViewPreferencePage::SCALEX_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultScaleFactorX));
  m_Ui->m_ScaleImageInYSpinBox->setValue(m_CameraCalViewPreferencesNode->GetDouble(CameraCalViewPreferencePage::SCALEY_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultScaleFactorY));
  m_Ui->m_GridPointsInXSpinBox->setValue(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::GRIDX_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultGridSizeX));
  m_Ui->m_GridPointsInYSpinBox->setValue(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::GRIDY_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultGridSizeY));
  m_Ui->m_ModelTransformLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::MODEL_TRANSFORM_NODE_NAME, ""));
  m_Ui->m_ReferenceImageLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::REFERENCE_IMAGE_NODE_NAME, ""));
  m_Ui->m_ReferencePointsLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::REFERENCE_POINTS_NODE_NAME, ""));
  m_Ui->m_TagFamilyComboBox->setCurrentIndex(
        m_Ui->m_TagFamilyComboBox->findText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::TAG_FAMILY_NODE_NAME, QString::fromStdString(niftk::NiftyCalVideoCalibrationManager::DefaultTagFamily))));
  m_Ui->m_FeaturesComboBox->setCurrentIndex(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::PATTERN_NODE_NAME, static_cast<int>(niftk::NiftyCalVideoCalibrationManager::DefaultCalibrationPattern)));
  m_Ui->m_HandEyeComboBox->setCurrentIndex(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::HANDEYE_NODE_NAME, static_cast<int>(niftk::NiftyCalVideoCalibrationManager::DefaultHandEyeMethod)));
  m_Ui->m_MinPointsSpinBox->setValue(m_CameraCalViewPreferencesNode->GetInt(CameraCalViewPreferencePage::MINIMUM_NUMBER_POINTS_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultMinimumNumberOfPoints));
  m_Ui->m_TemplateImageLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::TEMPLATE_IMAGE_NODE_NAME, ""));
  m_Ui->m_PreviousCalibrationDirLineEdit->setText(m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::PREVIOUS_CALIBRATION_DIR_NODE_NAME, ""));
  QString path = m_CameraCalViewPreferencesNode->Get(CameraCalViewPreferencePage::OUTPUT_DIR_NODE_NAME, "");
  if (path == "")
  {
    path = GetWritablePath();
  }
  m_Ui->m_OutputDirLineEdit->setText(path);
  m_Ui->m_ModelIsStationaryCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::MODEL_IS_STATIONARY_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultModelIsStationary));
  m_Ui->m_CameraIsStationaryCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::CAMERA_IS_STATIONARY_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultCameraIsStationary));
  m_Ui->m_SaveOutputBeforeCalibrationCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::SAVE_OUTPUT_BEFORE_CALIBRATION_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultSaveOutputBeforeCalibration));
  m_Ui->m_ResetCalibrationIfNodeChangesCheckBox->setChecked(m_CameraCalViewPreferencesNode->GetBool(CameraCalViewPreferencePage::RESET_CALIBRATION_IF_NODE_CHANGES_NODE_NAME, niftk::NiftyCalVideoCalibrationManager::DefaultResetCalibrationIfNodeChanges));
}

} // end namespace
