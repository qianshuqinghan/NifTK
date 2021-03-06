/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkPaintbrushTool_h
#define niftkPaintbrushTool_h

#include "niftkMIDASExports.h"

#include <itkImage.h>

#include <mitkGeometry3D.h>
#include <mitkImage.h>
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkPlanarEllipse.h>
#include <mitkSegTool2D.h>

#include <usServiceReference.h>

#include <itkMIDASImageUpdatePixelWiseSingleValueProcessor.h>

#include "niftkPaintbrushToolOpEditImage.h"
#include "niftkPaintbrushToolEventInterface.h"
#include "niftkFilteringStateMachine.h"

namespace niftk
{

 /**
  * \class PaintbrushTool
  * \brief Paint brush tool used during editing on the morphological editor screen (a.k.a connection breaker).
  *
  * Note the following:
  * <pre>
  * 1.) Writes into 4 images, so ToolManager must have 4 working volume to edit into.
  *     We define Working Image[0] = "additions image for erosions", which is added to the main segmentation to add stuff back into the volume.
  *     We define Working Image[1] = "subtractions image for erosions", which is subtracted from the main segmentation to do connection breaking.
  *     We define Working Image[2] = "additions image for dilations", which is added to the main segmentation to add stuff back into the volume.
  *     We define Working Image[3] = "subtractions image for dilations", which is subtracted from the main segmentation to do connection breaking.
  * 2.) Then:
  *     Left mouse = paint into the "additions image".
  *     Middle mouse = paint into the "subtractions image".
  *     Right mouse = subtract from the "subtractions image".
  * 3.) We derive from SegTool2D to keep things simple, as we just need to convert from mm world points to voxel points, and paint.
  * 4.) Derives from mitk::OperationActor, so this tool supports undo/redo.
  * </pre>
  *
  * This class is a MITK tool with a GUI defined in niftk::PaintbrushToolGUI, and instantiated
  * using the object factory described in Maleike et. al. doi:10.1016/j.cmpb.2009.04.004.
  *
  * To effectively use this tool, you need a 3 button mouse.
  *
  * Trac 1695, 1700, 1701, 1706: Fixing up dilations: We change pipeline so that WorkingData 0,1 are
  * applied during erosions phase, and WorkingData 2,3 are applied during dilations phase.
  */
class NIFTKMIDAS_EXPORT PaintbrushTool : public mitk::SegTool2D, public FilteringStateMachine
{

public:

  /// \brief Constants that identify the data needed for the morphological edit tools.
  /// They should be used for indexing the vector of working data.
  enum
  {
    SEGMENTATION,
    EROSIONS_ADDITIONS,
    EROSIONS_SUBTRACTIONS,
    DILATIONS_ADDITIONS,
    DILATIONS_SUBTRACTIONS,
    AXIAL_CUT_OFF_PLANE
  };

  /// \brief Stores the name of the MIDAS additions image, used in Morphological Editor.
  static const std::string EROSIONS_ADDITIONS_NAME;

  /// \brief Stores the name of the MIDAS connection breaker image, used in Morphological Editor.
  static const std::string EROSIONS_SUBTRACTIONS_NAME;

  /// \brief Stores the name of the MIDAS additions image, used in Morphological Editor.
  static const std::string DILATIONS_ADDITIONS_NAME;

  /// \brief Stores the name of the MIDAS connection breaker image, used in Morphological Editor.
  static const std::string DILATIONS_SUBTRACTIONS_NAME;

  /// \brief Stores the name of the axial cut-off slice data node, used in Morphological Editor.
  static const std::string AXIAL_CUT_OFF_PLANE_NAME;


  mitkClassMacro(PaintbrushTool, mitk::SegTool2D)
  itkNewMacro(PaintbrushTool)

  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, 3> ImageType;
  typedef itk::MIDASImageUpdatePixelWiseSingleValueProcessor<mitk::Tool::DefaultSegmentationDataType, 3> ProcessorType;

  virtual void InitializeStateMachine() override;

  /// Strings to help the tool identify itself in GUI.
  virtual const char* GetName() const override;
  virtual const char** GetXPM() const override;

  /// We store the name of a property that stores the image region.
  static const std::string REGION_PROPERTY_NAME;

  /// \brief Gets the position of the eraser cursor.
  mitk::Point2D GetEraserPosition() const;

  /// \brief Sets the position of the eraser cursor.
  void SetEraserPosition(const mitk::Point2D& eraserPosition);

  /// \brief Gets the eraser size.
  /// Default size is 1 voxel.
  int GetEraserSize() const;

  /// \brief Sets the eraser size.
  void SetEraserSize(int eraserSize);

  /// \brief Shows or hides the eraser cursor.
  void SetEraserVisible(bool visible, mitk::BaseRenderer* renderer = nullptr);

  /// \brief Gets the erosion mode.
  /// If true, we are editing image 0,1, and if false, we are editing image 2,3. Default true.
  bool GetErosionMode() const;

  /// \brief Sets the erosion mode.
  /// If true, we are editing image 0,1, and if false, we are editing image 2,3. Default true.
  void SetErosionMode(bool erosionMode);

  /** Used to send messages when the eraser size is changed or should be updated in a GUI. */
  mitk::Message1<int> EraserSizeChanged;

  /** Method to enable this class to interact with the Undo/Redo framework. */
  virtual void ExecuteOperation(mitk::Operation* operation);

  /** Process all mouse events. */
  virtual bool StartAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StartAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StartRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);

  /** Message sent when the user modified the segmentation using this tool */
  mitk::Message1<int> SegmentationEdited;

protected:

  PaintbrushTool();          // purposely hidden
  virtual ~PaintbrushTool(); // purposely hidden

  /// \brief Connects state machine actions to functions.
  virtual void ConnectActionsAndFunctions() override;

  /// \brief Tells if this tool can handle the given event.
  ///
  /// This implementation delegates the call to FilteringStateMachine::CanHandleEvent(),
  /// that checks if the event is filtered by one of the installed event filters and if not,
  /// calls CanHandle() and returns with its result.
  ///
  /// Note that this function is purposefully not virtual. Eventual subclasses should
  /// override the CanHandle function.
  bool FilterEvents(mitk::InteractionEvent* event, mitk::DataNode* dataNode) override;

  /**
  \brief Called when the tool gets activated (registered to mitk::GlobalInteraction).

  Derived tools should call their parents implementation.
  */
  virtual void Activated() override;

  /**
  \brief Called when the tool gets deactivated (unregistered from mitk::GlobalInteraction).

  Derived tools should call their parents implementation.
  */
  virtual void Deactivated() override;

private:

  // Operation constant, used in Undo/Redo framework.
  static const mitk::OperationType MIDAS_PAINTBRUSH_TOOL_OP_EDIT_IMAGE;

  ///
  /// \brief Used for working out which voxels to edit.
  ///
  /// Essentially, we take two points, currentPoint and previousPoint in millimetre space
  /// and step along a line between them. At each step we convert from millimetres to voxels,
  /// and that list of voxels is the affected region.
  void GetListOfAffectedVoxels(
      const mitk::PlaneGeometry& planeGeometry,
      mitk::Point3D& currentPoint,
      mitk::Point3D& previousPoint,
      ProcessorType &processor);

  /// \brief Marks the initial mouse position when any of the left/middle/right mouse buttons are pressed.
  bool MarkInitialPosition(unsigned int dataIndex, mitk::StateMachineAction* action, mitk::InteractionEvent* event);

  /// \brief Sets an invalid region (indicating that we are not editing) on the chosen image number data node.
  void SetInvalidRegion(unsigned int dataIndex);

  /// \brief Sets a valid region property, taken from the bounding box of edited voxels, indicating that we are editing the given image number.
  void SetValidRegion(unsigned int dataIndex, const std::vector<int>& boundingBox);

  /// \brief Method that actually sets the region property on a working image.
  void SetRegion(unsigned int dataIndex, bool valid, const std::vector<int>& boundingBox = std::vector<int>());

  /// \brief Does the main functionality when the mouse moves.
  bool DoMouseMoved(mitk::StateMachineAction* action, mitk::InteractionEvent* event,
      int dataIndex,
      unsigned char valueForRedo,
      unsigned char valueForUndo
      );

  /// \brief Using the MITK to ITK access functions to run the ITK processor object.
  template<typename TPixel, unsigned int VImageDimension>
  void RunITKProcessor(
      itk::Image<TPixel, VImageDimension>* itkImage,
      ProcessorType::Pointer processor,
      bool redo,
      unsigned char valueToWrite
      );

  // Pointer to interface object, used as callback in Undo/Redo framework
  PaintbrushToolEventInterface::Pointer m_Interface;

  /// \brief Calculates the current image number.
  int GetDataIndex(bool isLeftMouseButton);

  /// \brief Updates coordinates of the eraser cursor.
  void UpdateEraserCursor();

  /// The position of the eraser on the 2D plane, in mm.
  mitk::Point2D m_EraserPosition;

  // Eraser size for editing, and cursor type is currently always a cross. Size is in voxels.
  int m_EraserSize;

  // This is the 3D geometry associated with the m_WorkingImage, where we assume both working images have same size and geometry.
  mitk::BaseGeometry* m_WorkingImageGeometry;

  // This points to the current working image, assuming that we are only ever processing, left, middle or right mouse button at any one time.
  mitk::Image* m_WorkingImage;

  // Used between MouseDown and MouseMoved events to track movement.
  mitk::Point3D m_MostRecentPointInMillimetres;

  // If m_ErosionMode is true, we update WorkingData 0 and 1, if m_ErosionMode is false, we update WorkingData 2 and 3.
  bool m_ErosionMode;

  // Stores the current display interactor configurations when this tool is activated.
  // The configurations are restored when the tool is deactivated.
  std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;

  bool m_AddingAdditionInProgress;
  bool m_AddingSubtractionInProgress;
  bool m_RemovingSubtractionInProgress;

  mitk::PlanarEllipse::Pointer m_EraserCursor;
  mitk::DataNode::Pointer m_EraserCursorNode;
  bool m_EraserVisible;

};

}

#endif
