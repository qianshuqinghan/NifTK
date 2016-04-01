/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __niftkGeneralSegmentorController_h
#define __niftkGeneralSegmentorController_h

#include <mitkOperationActor.h>

#include <niftkBaseSegmentorController.h>
#include <niftkMIDASToolKeyPressResponder.h>
#include <niftkMIDASToolKeyPressStateMachine.h>

#include "niftkGeneralSegmentorEventInterface.h"

namespace mitk
{
class PointSet;
}

class niftkGeneralSegmentorGUI;
class niftkGeneralSegmentorView;

/**
 * \class niftkGeneralSegmentorController
 * \brief Provides the MIDAS general purpose, Irregular Volume Editor functionality originally developed
 * at the Dementia Research Centre UCL (http://dementia.ion.ucl.ac.uk/).
 *
 * This class uses the mitk::ToolManager and associated framework described in this paper on the
 * <a href="http://www.sciencedirect.com/science/article/pii/S0169260709001229">MITK Segmentation framework</a>.
 *
 * The mitk::ToolManager has the following data sets registered in this order.
 * <pre>
 *   0. mitk::Image = the image being segmented, i.e. The Output.
 *   1. mitk::PointSet = the seeds for region growing, noting that the seeds are in 3D, spreading throughout the volume.
 *   2. mitk::ContourModelSet = a set of contours for the current slice being edited - representing the current segmentation, i.e. green lines in MIDAS, but drawn here in orange.
 *   3. mitk::ContourModelSet = a set of contours specifically for the draw tool, i.e. also green lines in MIDAS, and also drawn here in orange.
 *   4. mitk::ContourModelSet = a set of contours for the prior slice, i.e. whiteish lines in MIDAS.
 *   5. mitk::ContourModelSet = a set of contours for the next slice, i.e. turquoise blue lines in MIDAS.
 *   6. mitk::Image = binary image, same size as item 0, to represent the current region growing, i.e. blue lines in MIDAS.
 * </pre>
 * Useful notes towards helping the understanding of this class
 * <ul>
 *   <li>Items 1-6 are set up in the mitk::DataManager as hidden children of item 0.</li>
 *   <li>The segmentation is very specific to a given view, as for example the ContourModelSet in WorkingData items 2,3,4,5 are only generated for a single slice, corresponding to the currently selected render window.</li>
 *   <li>Region growing is 2D on the currently selected slice, except when doing propagate up or propagate down.</li>
 *   <li>Apologies that this is rather a large monolithic class.</li>
 * </ul>
 * Additionally, significant bits of functionality include:
 *
 * <h2>Recalculation of Seed Position</h2>
 *
 * The number of seeds for a slice often needs re-computing.  This is often because a slice
 * has been automatically propagated, and hence we need new seeds for each slice because
 * as you scroll through slices, regions without a seed would be wiped. For a given slice, the seeds
 * are set so that each disjoint (i.e. not 4-connected) region will have its own seed at the
 * largest minimum distance from the edge, scanning only in a vertical or horizontal direction.
 * In other words, for an image containing a single region:
 * <pre>
 * Find the first voxel in the image, best voxel location = current voxel location,
 * and best distance = maximum number of voxels along an image axis.
 * For each voxel
 *   Scan +x, -x, +y, -y and measure the minimum distance to the boundary
 *   If minimum distance > best distance
 *     best voxel location = current voxel location
 *     best distance = minimum distance
 * </pre>
 * The result is the largest minimum distance, or the largest minimum distance to an edge, noting
 * that we are not scanning diagonally.
 *
 * <h2>Propagate Up/Down/3D</h2>
 *
 * Propagate runs a 3D region propagation from and including the current slice up/down, writing the
 * output to the current segmentation volume, overwriting anything already there.
 * The current slice is always affected. So, you can leave the threshold tick box either on or off.
 * For each subsequent slice in the up/down direction, the number of seeds is recomputed (as above).
 * 3D propagation is exactly equivalent to clicking "prop up" followed by "prop down".
 * Here, note that in 3D, you would normally do region growing in a 6-connected neighbourhood.
 * Here, we are doing a 5D connected neighbourhood, as you always propagate forwards in one
 * direction. i.e. in a coronal slice, and selecting "propagate up", which means propagate anterior,
 * then you cannot do region growing in the posterior direction. So its a 5D region growing.
 *
 * <h2>Threshold Apply</h2>
 *
 * The threshold "apply" button is only enabled when the threshold check-box is enabled,
 * and disabled otherwise. The current segmentation, draw tool contours and poly tool contours
 * (eg. WorkingData items 2 and 3, plus temporary data in the niftk::MIDASPolyTool) all limit the
 * region growing.
 *
 * When we hit "apply":
 * <pre>
 * 1. Takes the current region growing image, and writes it to the current image.
 * 2. Recalculate the number of seeds for that slice, 1 per disjoint region, as above.
 * 3. Turn off thresholding, leaving sliders at current value.
 * </pre>
 *
 * <h2>Wipe, Wipe+, Wipe-</h2>
 *
 * All three pieces of functionality appear similar, wiping the whole slice, whole anterior
 * region, or whole posterior region, including all segmentation and seeds. The threshold controls
 * are not changed. So, if it was on before, it will be on afterwards.
 *
 * <h2>Retain Marks</h2>
 *
 * The "retain marks" functionality only has an impact if we change slices. When the "retain marks"
 * checkbox is ticked, and we change slices we:
 * <pre>
 * 1. Check if the new slice is empty.
 * 2. If not empty we warn.
 * 3. If the user elects to overwrite the new slice, we simply copy all seeds and all image data to the new slice.
 * </pre>
 *
 * \sa niftkBaseSegmentorController
 * \sa MIDASMorphologicalSegmentorController
 */
class niftkGeneralSegmentorController
  : public niftkBaseSegmentorController,
    public mitk::OperationActor,
    public niftk::MIDASToolKeyPressResponder
{

  Q_OBJECT

public:

  niftkGeneralSegmentorController(niftkGeneralSegmentorView* segmentorView);
  virtual ~niftkGeneralSegmentorController();

  /// \brief \see niftk::MIDASToolKeyPressResponder::SelectSeedTool()
  virtual bool SelectSeedTool() override;

  /// \brief \see niftk::MIDASToolKeyPressResponder::SelectDrawTool()
  virtual bool SelectDrawTool() override;

  /// \brief \see niftk::MIDASToolKeyPressResponder::UnselectTools()
  virtual bool UnselectTools() override;

  /// \brief \see niftk::MIDASToolKeyPressResponder::SelectPolyTool()
  virtual bool SelectPolyTool() override;

  /// \brief \see niftk::MIDASToolKeyPressResponder::SelectViewMode()
  virtual bool SelectViewMode() override;

  /// \brief \see niftk::MIDASToolKeyPressResponder::CleanSlice()
  virtual bool CleanSlice() override;

  /// \brief Method to enable this class to interact with the Undo/Redo framework.
  virtual void ExecuteOperation(mitk::Operation* operation) override;

protected:

  /// \brief For Irregular Volume Editing, a Segmentation image should have a grey
  /// scale parent, and several children as described in the class introduction.
  virtual bool IsNodeASegmentationImage(const mitk::DataNode::Pointer node) override;

  /// \brief Assumes input is a valid segmentation node, then searches for the derived
  /// children of the node, looking for the seeds and contours  as described in the class introduction.
  virtual mitk::ToolManager::DataVectorType GetWorkingDataFromSegmentationNode(const mitk::DataNode::Pointer node) override;

  /// \brief We return true if the segmentation can be "re-started", i.e. you switch between binary images
  /// in the DataManager, and if the binary image has the correct hidden child nodes, then
  /// this returns true, indicating that it's a valid "in-progress" segmentation.
  virtual bool CanStartSegmentationForBinaryNode(const mitk::DataNode::Pointer node) override;

    /// \brief Creates the general segmentor widget that holds the GUI components of the view.
  virtual niftkBaseSegmentorGUI* CreateSegmentorGUI(QWidget* parent) override;

  void OnNodeVisibilityChanged(const mitk::DataNode* node);

protected slots:

  /// \brief Qt slot called from "see prior" checkbox to show the contour from the previous slice.
  void OnSeePriorCheckBoxToggled(bool checked);

  /// \brief Qt slot called from "see next" checkbox to show the contour from the next slice.
  void OnSeeNextCheckBoxToggled(bool checked);

  /// \brief Qt slot called when the Clean button is pressed, indicating the
  /// current contours on the current slice should be cleaned, see additional spec,
  /// currently at:  https://cmiclab.cs.ucl.ac.uk/CMIC/NifTK/issues/1096
  void OnCleanButtonClicked();

  /// \brief Qt slot called when the Wipe button is pressed and will erase the current
  /// slice and seeds on the current slice.
  void OnWipeButtonClicked();

  /// \brief Qt slot called when the Wipe+ button is pressed and will erase the
  /// whole region Anterior/Superior/Right from the current slice, including seeds.
  void OnWipePlusButtonClicked();

  /// \brief Qt slot called when the Wipe- button is pressed and will erase the
  /// whole region Posterior/Inferior/Left from the current slice, including seeds.
  void OnWipeMinusButtonClicked();

  /// \brief Qt slot called when the Propagate Up button is pressed to take the
  /// current seeds and threshold values, and propagate Anterior/Superior/Right.
  void OnPropagateUpButtonClicked();

  /// \brief Qt slot called when the Propagate Down button is pressed to take the current
  /// seeds and threshold values, and propagate Posterior/Inferor/Left.
  void OnPropagateDownButtonClicked();

  /// \brief Qt slot called when the Propagate 3D button is pressed that is effectively
  /// equivalent to calling OnPropagateUpButtonPressed and OnPropagateDownButtonPressed.
  void OnPropagate3DButtonClicked();

  /// \brief Qt slot called when the Apply button is pressed and used to accept the
  /// current region growing segmentation, and recalculates seed positions as per MIDAS spec
  /// described in this class intro.
  void OnThresholdApplyButtonClicked();

  /// \brief Qt slot called when the "threshold" checkbox is checked, and toggles
  /// the thresholding widget section on and calls niftkGeneralSegmentorView::UpdateRegionGrowing.
  void OnThresholdingCheckBoxToggled(bool checked);

  /// \brief Qt slot called when the lower or upper threshold slider is moved, calls
  /// niftkGeneralSegmentorView::UpdateRegionGrowing as thresholds have changed.
  void OnThresholdValueChanged();

  /// \brief Qt slot called when the any button is pressed on this widget.
  ///
  /// It transfers the focus back to the main window so that the key interactions keep working.
  void OnAnyButtonClicked();

  /// \brief Callback for when the window focus changes, where we update this view
  /// to be listening to the right window, and make sure ITK pipelines know we have
  /// changed orientation.
  void OnFocusChanged();

  /// \brief Qt slot called when the OK button is pressed and accepts the current
  /// segmentation, destroying the working data (seeds, contours, region growing image),
  /// leaving you with a finished segmentation.
  void OnOKButtonClicked();

  /// \brief Qt slot called when the Reset button is pressed and resets to the start
  /// of the segmentation, so wipes the current segmentation (no undo), but leaves the
  /// reference data so you can continue segmenting.
  void OnResetButtonClicked();

  /// \brief Qt slot called when the Cancel button is pressed and destroys all working
  /// data (seeds, contours, region growing image), and also destroys the current segmentation
  /// if it was created by this volume editor. Otherwise, it restores the original segmentation.
  void OnCancelButtonClicked();

  /// \brief Qt slot called when the Restart button is pressed and restores the initial
  /// state of the segmentation.
  void OnRestartButtonClicked();

private:

  void OnViewGetsVisible();

  void OnViewGetsHidden();

  /// \brief If the user hits the close icon, it is equivalent to a Cancel,
  /// and the segmentation is destroyed without warning.
  void OnViewGetsClosed();

  /// \brief Called from the slice navigation controller to indicate a different slice,
  /// which in MIDAS terms means automatically accepting the currently segmented slice
  /// and moving to the next one.
  virtual void OnSliceChanged(const itk::EventObject& geometrySliceEvent);

  /// \brief Qt slot called to effect a change of slice, which means accepting
  /// the current segmentation, and moving to the prior/next slice, see class intro.
  void OnSliceNumberChanged(int before, int after);

  /// \brief This view registers with the mitk::DataStorage and listens for changing
  /// data, so this method is called when any node is changed, but only performs an update,
  /// if the nodes changed are those registered with the ToolManager as WorkingData,
  /// see class introduction.
  void OnNodeChanged(const mitk::DataNode* node);

  /// \brief This view registers with the mitk::DataStorage and listens for removing
  /// data, so this method cancels the operation and frees the resources if the
  /// segmentation node is removed.
  void OnNodeRemoved(const mitk::DataNode* node);

  /// \brief Called from the registered Poly tool and Draw tool to indicate that contours have changed.
  virtual void OnContoursChanged();

  /// \brief Used to create an image used for the region growing, see class intro.
  mitk::DataNode::Pointer CreateHelperImage(mitk::Image::Pointer referenceImage, mitk::DataNode::Pointer segmentationNode,  float r, float g, float b, std::string name, bool visible, int layer);

  /// \brief Used to create a contour set, used for the current, prior and next contours, see class intro.
  mitk::DataNode::Pointer CreateContourSet(mitk::DataNode::Pointer segmentationNode, float r, float g, float b, std::string name, bool visible, int layer);

  /// \brief Utility method to check that we have initialised all the working data such as contours, region growing images etc.
  bool HasInitialisedWorkingData();

  /// \brief Stores the initial state of the segmentation so that the Restart button can restore it.
  void StoreInitialSegmentation();

  /// \brief Looks for the Seeds registered as WorkingData[1] with the ToolManager.
  mitk::PointSet* GetSeeds();

  /// \brief Used when restarting a volume, to initialize all seeds for an existing segmentation.
  void InitialiseSeedsForWholeVolume();

  /// \brief Retrieves the min and max of the image (cached), and sets the thresholding
  /// intensity sliders range accordingly.
  void RecalculateMinAndMaxOfImage();

  /// \brief For each seed in the list of seeds and current slice, converts to millimetre position,
  /// and looks up the pixel value in the reference image (grey scale image being segmented)
  /// at that location, and updates the min and max labels on the GUI thresholding panel.
  void RecalculateMinAndMaxOfSeedValues();

  /// \brief Simply returns true if slice has any unenclosed seeds, and false otherwise.
  bool DoesSliceHaveUnenclosedSeeds(bool thresholdOn, int sliceNumber);

  /// \brief Simply returns true if slice has any unenclosed seeds, and false otherwise.
  bool DoesSliceHaveUnenclosedSeeds(bool thresholdOn, int sliceNumber, mitk::PointSet& seeds);

  /// \brief Filters seeds to current slice
  void FilterSeedsToCurrentSlice(
      mitk::PointSet& inputPoints,
      int& axis,
      int& sliceNumber,
      mitk::PointSet& outputPoints
      );

  /// \brief Filters seeds to current slice, and selects seeds that are enclosed.
  void FilterSeedsToEnclosedSeedsOnCurrentSlice(
      mitk::PointSet& inputPoints,
      bool& thresholdOn,
      int& sliceNumber,
      mitk::PointSet& outputPoints
      );

  /// \brief Retrieves the lower and upper threshold from widgets and calls UpdateRegionGrowing.
  void UpdateRegionGrowing(bool updateRendering = true);

  /// \brief Given the two thresholds, and all seeds and contours, will recalculate the thresholded region in the current slice.
  /// \param isVisible whether the region growing volume should be visible.
  void UpdateRegionGrowing(bool isVisible, int sliceNumber, double lowerThreshold, double upperThreshold, bool skipUpdate);

  /// \brief Takes the current slice, and refreshes the current slice contour set (WorkingData[2]).
  void UpdateCurrentSliceContours(bool updateRendering = true);

  /// \brief Takes the current slice, and updates the prior (WorkingData[4]) and next (WorkingData[5]) contour sets.
  void UpdatePriorAndNext(bool updateRendering = true);

  /// \brief Does propagate up/down/3D.
  void DoPropagate(bool isUp, bool is3D);

  /// \brief Does wipe, where if direction=0, wipes current slice, if direction=1, wipes anterior,
  /// and if direction=-1, wipes posterior.
  bool DoWipe(int direction);

  /// \brief Method that actually does the threshold apply, so we can call it from the
  /// threshold apply button and not change slice, or when we change slice.
  bool DoThresholdApply(int oldSliceNumber, int newSliceNumber, bool optimiseSeeds, bool newSliceEmpty, bool newCheckboxStatus);

  /// \brief Used to toggle tools on/off.
  void ToggleTool(int toolId);

  /// \brief Completely removes the current pipeline.
  void DestroyPipeline();

  /// \brief Removes the images we are using for editing during segmentation.
  void RemoveWorkingData();

  /// \brief Restores the initial state of the segmentation after the Restart button was pressed.
  void RestoreInitialSegmentation();

  /// \brief Called when the view is closed or the segmentation node is removed from the data
  /// manager and destroys all working data (seeds, contours, region growing image), and also
  /// destroys the current segmentation.
  void DiscardSegmentation();

  /// \brief Clears both images of the working data.
  void ClearWorkingData();

  /// \brief All the GUI controls for the main view part.
  niftkGeneralSegmentorGUI* m_GeneralSegmentorGUI;

  niftkGeneralSegmentorView* m_GeneralSegmentorView;

  /// \brief Pointer to interface object, used as callback in Undo/Redo framework
  niftkGeneralSegmentorEventInterface::Pointer m_Interface;

  /// \brief Flag to stop re-entering code, while updating.
  bool m_IsUpdating;

  /// \brief Flag to stop re-entering code, while trying to delete/clear the pipeline.
  bool m_IsDeleting;

  /// \brief Additional flag to stop re-entering code, specifically to block
  /// slice change commands from the slice navigation controller.
  bool m_IsChangingSlice;

  bool m_IsRestarting;

  /// \brief Keep track of this to SliceNavigationController register and unregister event listeners.
  mitk::SliceNavigationController::Pointer m_SliceNavigationController;

  /// \brief Each time the window changes, we register to the current slice navigation controller.
  unsigned long m_SliceNavigationControllerObserverTag;

  /// \brief Used for the mitk::FocusManager to register callbacks to track the currently focus window.
  unsigned long m_FocusManagerObserverTag;

  /// \brief Keep track of the previous slice number and reset to -1 when the window focus changes.
  int m_PreviousSliceNumber;

  /// \brief We track the current and previous focus point, as it is used in calculations of which slice we are on,
  /// as under certain conditions, you can't just take the slice number from the slice navigation controller.
  mitk::Point3D m_CurrentFocusPoint;

  /// \brief We track the current and previous focus point, as it is used in calculations of which slice we are on,
  /// as under certain conditions, you can't just take the slice number from the slice navigation controller.
  mitk::Point3D m_PreviousFocusPoint;

  /// \brief This class hooks into the Global Interaction system to respond to Key press events.
  niftk::MIDASToolKeyPressStateMachine::Pointer m_ToolKeyPressStateMachine;

friend class niftkGeneralSegmentorView;

};

#endif
