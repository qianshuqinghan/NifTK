/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSingleViewerWidgetTest_h
#define niftkSingleViewerWidgetTest_h

#include <QObject>

#include <niftkAtomicStateTransitionTester.cxx>

#include <niftkSingleViewerWidgetState.h>

#include <vector>

namespace mitk
{
class DataNode;
}

class QmitkRenderWindow;


namespace niftk
{
class SingleViewerWidget;
class SingleViewerWidgetTestClassPrivate;


class SingleViewerWidgetTestClass: public QObject
{
  Q_OBJECT

public:

  enum WorldAxes { SagittalAxis, CoronalAxis, AxialAxis };

  typedef AtomicStateTransitionTester<const SingleViewerWidget*, SingleViewerWidgetState> ViewerStateTester;
  typedef SingleViewerWidgetState ViewerState;
  typedef SingleViewerWidgetTestClass Self;

  /// \brief Constructs a niftkSingleViewerWidgetTestClass object.
  explicit SingleViewerWidgetTestClass();

  /// \brief Destructs the niftkSingleViewerWidgetTestClass object.
  virtual ~SingleViewerWidgetTestClass();

  /// \brief Gets the name of the image file to load into the viewer.
  std::string GetFileName() const;

  /// \brief Sets the name of the image file to load into the viewer.
  void SetFileName(const std::string& fileName);

  /// \brief Gets the interactive mode.
  /// In interactive mode the windows are not closed when the test is finished.
  bool GetInteractiveMode() const;

  /// \brief Sets the interactive mode.
  /// In interactive mode the windows are not closed when the test is finished.
  void SetInteractiveMode(bool interactiveMode);

  /// \brief Calculates the world origin from a 3D geometry.
  /// The world origin is the centre of the bottom-left-back voxel for image geometries
  /// and the bottom-left-back corner of the bottom-left-back voxel for non-image geometries.
  static mitk::Point3D GetWorldOrigin(const mitk::BaseGeometry* geometry);

  /// \brief Calculates the up direction of the world axes from a 3D geometry.
  static mitk::Vector3D GetWorldUpDirections(const mitk::BaseGeometry* geometry);

  /// \brief Calculates the world axes from a 3D geometry.
  static mitk::Vector3D GetWorldAxes(const mitk::BaseGeometry* geometry);

  /// \brief Calculates the world origin from a 3D geometry.
  /// The world origin is the centre of the bottom-left-back voxel for image geometries
  /// and the bottom-left-back corner of the bottom-left-back voxel for non-image geometries.
  static mitk::Point3D GetWorldBottomLeftBackCorner(const mitk::BaseGeometry* geometry);

  /// \brief Converts a display position in a render window to a point on the screen.
  /// The display position is a relative position within the render window normalised by the render window size.
  /// The bottom left position is (0.0, 0.0), the top right position is (1.0, 1.0).
  QPoint GetPointAtDisplayPosition(int windowIndex, const mitk::Vector2D& displayPosition);

  /// \brief Converts a point on the screen to a display position in a render window.
  /// The display position is a relative position within the render window normalised by the render window size.
  /// The bottom left position is (0.0, 0.0), the top right position is (1.0, 1.0).
  mitk::Vector2D GetDisplayPositionAtPoint(int windowIndex, const QPoint& point);

  mitk::Point3D GetWorldPositionAtDisplayPosition(int orientation, const mitk::Vector2D& displayPosition);

  mitk::Vector2D GetDisplayPositionAtWorldPosition(int orientation, const mitk::Point3D& worldPosition);

  /// \brief Gets the position of the centre of the displayed region, relative to the render window.
  mitk::Vector2D GetCentrePosition(int windowIndex);

  /// \brief Gets the position of the centre of the displayed regions, relative to their render windows.
  std::vector<mitk::Vector2D> GetCentrePositions();

  /// \brief Gets one coordinate of the centre of the voxel that contains the given coordinate along the axis.
  double GetVoxelCentreCoordinate(int axis, double coordinate);

  /// \brief Gets the position of the centre of the voxel that contains the given position.
  mitk::Point3D GetVoxelCentrePosition(const mitk::Point3D& position);

  /// \brief Determines if two world positions are equal with the certain tolerance.
  /// By default, or if negative value is specified, the tolerance is half voxel for each coordinate.
  /// Converting the positions to voxel space should result equal coordinates.
  bool Equals(const mitk::Point3D& selectedPosition1, const mitk::Point3D& selectedPosition2, double tolerance = -1.0);

  /// \brief Determines if two cursor positions are equal with the given tolerance.
  static bool Equals(const mitk::Vector2D& cursorPosition1, const mitk::Vector2D& cursorPosition2, double tolerance = 0.01);

  /// \brief Determines if two vectors of cursor positions are equal with the given tolerance.
  /// The function assumes that the vectors contain three elements.
  bool Equals(const std::vector<mitk::Vector2D>& cursorPositions1, const std::vector<mitk::Vector2D>& cursorPositions2, double tolerance = 0.01);

  /// \brief Gives a random position from the image volume, in mm coordinates.
  mitk::Point3D GetRandomWorldPosition() const;

  /// \brief Gives a random position in the render window, normalised with the render window size.
  /// The measurement is unit (i.e. px/px).
  static mitk::Vector2D GetRandomDisplayPosition();

  /// \brief Gives a vector of random diplay positions, normalised with the render window size.
  static std::vector<mitk::Vector2D> GetRandomDisplayPositions(std::size_t size = std::size_t(3));

  /// \brief Gives a random scale factor (mm/px) within the range (0.0, 2.0).
  static double GetRandomScaleFactor();

  /// \brief Gives a vector of random scale factors (mm/px) within the range (0.0, 2.0).
  static std::vector<double> GetRandomScaleFactors(std::size_t size = std::size_t(3));

  /// \brief Sets random selected position, cursor positions and scale factors for the viewer.
  void SetRandomPositions();

private slots:

  /// \brief Initialisation before the first test function.
  void initTestCase();

  /// \brief Clean up after the last test function.
  void cleanupTestCase();

  /// \brief Initialisation before each test function.
  void init();

  /// \brief Clean up after each test function.
  void cleanup();

  /// \brief Creates a viewer and and loads an image.
  void testViewer();

  /// \brief Tests the SetSelectedSlice function.
  void testSetSelectedSlice2();

  /// \brief Tests if the geometry is correctly initialised.
  void testGetTimeGeometry();

  /// \brief Tests if the viewer correctly initialises the renderer geometries.
  void testSetTimeGeometry();

  /// \brief Tests if the selected orientation is correct after the image is loaded.
  void testGetOrientation();

  /// \brief Tests if the selected position is in the centre after the image is loaded.
  void testGetSelectedPosition();

  /// \brief Tests the SetSelectedPosition function.
  void testSetSelectedPosition();

  /// \brief Tests if the centre slice is selected after the image is loaded.
  void testGetSelectedSlice();

  /// \brief Tests the SetSetSelectedSlice function.
  void testSetSelectedSlice();

  /// \brief Tests if the cursor position is in the centre of the windoow after the image is loaded.
  void testGetCursorPosition();

  /// \brief Tests the SetCursorPosition function.
  void testSetCursorPosition();

  /// \brief Tests if the cursor position is in the centre of the windoow after the image is loaded.
  void testGetCursorPositions();

  /// \brief Tests the SetCursorPosition function.
  void testSetCursorPositions();

  /// \brief Tests if the selected position is correct after the image is loaded.
  void testGetWindowLayout();

  /// \brief Tests if the correct render window is selected after the image is loaded.
  void testGetSelectedRenderWindow();

  /// \brief Tests the SetSelectedRenderWindow function.
  void testSetSelectedRenderWindow();

  /// \brief Tests the window layout change.
  void testSetWindowLayout();

  /// \brief Tests remembering selected position when changing window layout.
  void testRememberPositionsPerWindowLayout();

  /// \brief Tests selecting a position by interaction (left mouse button click).
  void testSelectPositionByInteraction();

  /// \brief Tests changing slice by mouse interaction (wheel up or down).
  void testChangeSliceByMouseInteraction();

  /// \brief Tests changing slice by key interaction ('a' or 'z').
  void testChangeSliceByKeyInteraction();

  /// \brief Tests selecting a position by interaction (left mouse button click).
  void testSelectRenderWindowByInteraction();

  /// \brief Test select slice through SNC.
  void testSelectSliceThroughSliceNavigationController();

  /// \brief Test select position through SNC.
  void testSelectPositionThroughSliceNavigationController();

  /// \brief Test the cursor position binding.
  void testCursorPositionBinding();

  /// \brief Test the scale factor binding.
  void testScaleFactorBinding();

  /// \brief Test the cursor position and scale factor binding together.
  void testCursorPositionAndScaleFactorBinding();

private:

  void DropNodes(QmitkRenderWindow* renderWindow, const std::vector<mitk::DataNode*>& nodes);

  static void MouseWheel(QWidget* window, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                         QPoint point, int delta, Qt::Orientation orientation = Qt::Vertical);

  static mitk::Vector3D s_MITKRendererUpDirections;

  QScopedPointer<SingleViewerWidgetTestClassPrivate> d_ptr;

  Q_DECLARE_PRIVATE(SingleViewerWidgetTestClass)
  Q_DISABLE_COPY(SingleViewerWidgetTestClass)
};

}

int niftkSingleViewerWidgetTest(int argc, char* argv[]);

#endif
