/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __niftkSingleViewerWidgetTest_h
#define __niftkSingleViewerWidgetTest_h

#include <QObject>

#include <mitkAtomicStateTransitionTester.cxx>

#include <niftkSingleViewerWidgetState.h>

#include <vector>

class niftkSingleViewerWidgetTestClassPrivate;

class niftkSingleViewerWidget;

namespace mitk
{
class DataNode;
}

class niftkSingleViewerWidgetTestClass: public QObject
{
  Q_OBJECT

public:

  typedef mitk::AtomicStateTransitionTester<const niftkSingleViewerWidget*, niftkSingleViewerWidgetState> ViewerStateTester;
  typedef niftkSingleViewerWidgetState ViewerState;
  typedef niftkSingleViewerWidgetTestClass Self;

  /// \brief Constructs a niftkSingleViewerWidgetTestClass object.
  explicit niftkSingleViewerWidgetTestClass();

  /// \brief Destructs the niftkSingleViewerWidgetTestClass object.
  virtual ~niftkSingleViewerWidgetTestClass();

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

  /// \brief Converts a cursor position in a render window to a point on the screen.
  /// The cursor position is a relative position within the render window normalised to the render window size.
  /// The bottom left position is (0.0, 0.0), the top right position is (1.0, 1.0).
  static QPoint GetPointAtCursorPosition(QmitkRenderWindow *renderWindow, const mitk::Vector2D& cursorPosition);

  /// \brief Converts a point on the screen to a cursor position in a render window.
  /// The cursor position is a relative position within the render window normalised to the render window size.
  /// The bottom left position is (0.0, 0.0), the top right position is (1.0, 1.0).
  static mitk::Vector2D GetCursorPositionAtPoint(QmitkRenderWindow *renderWindow, const QPoint& point);

  /// \brief Determines if two world positions are equal with the tolerance of half spacing.
  /// Converting the positions to voxel space should result equal coordinates.
  bool Equals(const mitk::Point3D& selectedPosition1, const mitk::Point3D& selectedPosition2);

  /// \brief Determines if two cursor positions are equal with the given tolerance.
  static bool Equals(const mitk::Vector2D& cursorPosition1, const mitk::Vector2D& cursorPosition2, double tolerance = 0.001);

  /// \brief Determines if two vectors of cursor positions are equal with the given tolerance.
  /// The function assumes that the vectors contain three elements.
  static bool Equals(const std::vector<mitk::Vector2D>& cursorPositions1, const std::vector<mitk::Vector2D>& cursorPositions2, double tolerance = 0.001);

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

  /// \brief Tests if the selected orientation is correct after the image is loaded.
  void testGetOrientation();

  /// \brief Tests if the selected position is in the centre after the image is loaded.
  void testGetSelectedPosition();

  /// \brief Tests the SetSelectedPosition function.
  void testSetSelectedPosition();

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

  /// \brief Tests if the correct renderer is focused after the image is loaded.
  void testFocusedRenderer();

  /// \brief Tests the window layout change.
  void testSetWindowLayout();

  /// \brief Tests remembering selected position when changing window layout.
  void testRememberSelectedPosition();

  /// \brief Tests selecting a position by interaction (left mouse button click).
  void testSelectPositionByInteraction();

  /// \brief Tests selecting a position by interaction (left mouse button click).
  void testSelectRenderWindowByInteraction();

private:

  void DropNodes(QWidget* window, const std::vector<mitk::DataNode*>& nodes);

  QScopedPointer<niftkSingleViewerWidgetTestClassPrivate> d_ptr;

  Q_DECLARE_PRIVATE(niftkSingleViewerWidgetTestClass)
  Q_DISABLE_COPY(niftkSingleViewerWidgetTestClass)
};


int niftkSingleViewerWidgetTest(int argc, char* argv[]);

#endif
