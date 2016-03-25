/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __niftkMorphologicalSegmentorControls_h
#define __niftkMorphologicalSegmentorControls_h

#include "ui_niftkMorphologicalSegmentorWidget.h"
#include "niftkBaseSegmentorControls.h"

#include <MorphologicalSegmentorPipelineParams.h>

#include <niftkMIDASGuiExports.h>

class QAbstractButton;

/**
 * \class niftkMorphologicalSegmentorControls
 * \brief Implements a few Qt specific things that are of no interest to the MITK view class.
 * \ingroup uk_ac_ucl_cmic_midasmorphologicalsegmentor_internal
 */
class NIFTKMIDASGUI_EXPORT niftkMorphologicalSegmentorControls
  : public niftkBaseSegmentorControls,
    private Ui_niftkMorphologicalSegmentorWidget
{
  Q_OBJECT

public:

  /// \brief Constructor.
  niftkMorphologicalSegmentorControls(QWidget* parent = nullptr);

  /// \brief Destructor.
  virtual ~niftkMorphologicalSegmentorControls();

  /// \brief Get the current tab index.
  int GetTabIndex() const;

  /// \brief Set the current tab index.
  /// No TabChanged(int) signal is emitted.
  void SetTabIndex(int tabIndex);

  /// \brief Enables/disables all controls.
  void SetEnabled(bool enabled);

  /// \brief Set the dialog according to relevant image data.
  void SetControlsByReferenceImage(double lowestValue, double highestValue, int numberOfAxialSlices, int upDirection);

  /// \brief Set the dialog according to current parameter values
  void SetControlsByPipelineParams(MorphologicalSegmentorPipelineParams& params);

signals:

  void ThresholdingValuesChanged(double lowerThreshold, double upperThreshold, int axialSlicerNumber);
  void ErosionsValuesChanged(double upperThreshold, int numberOfErosions);
  void DilationsValuesChanged(double lowerPercentage, double upperPercentage, int numberOfDilations);
  void RethresholdingValuesChanged(int boxSize);
  void TabChanged(int tabIndex);
  void OKButtonClicked();
  void CancelButtonClicked();
  void RestartButtonClicked();

protected slots:

  void OnThresholdLowerValueChanged();
  void OnThresholdUpperValueChanged();
  void OnAxialCutOffSliderChanged();
  void OnBackButtonClicked();
  void OnNextButtonClicked();
  void OnErosionsUpperThresholdChanged();
  void OnErosionsIterationsChanged();
  void OnDilationsLowerThresholdChanged();
  void OnDilationsUpperThresholdChanged();
  void OnDilationsIterationsChanged();
  void OnRethresholdingSliderChanged();
  void OnRestartButtonClicked();

private:

  void EmitThresholdingValues();
  void EmitErosionValues();
  void EmitDilationValues();
  void EmitRethresholdingValues();

};

#endif
