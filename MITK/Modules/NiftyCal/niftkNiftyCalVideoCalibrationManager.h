/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkNiftyCalVideoCalibrationManager_h
#define niftkNiftyCalVideoCalibrationManager_h

#include "niftkNiftyCalExports.h"
#include <itkObject.h>
#include <itkObjectFactoryBase.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <niftkPointUtilities.h>
#include <niftkIPoint2DDetector.h>
#include <cv.h>
#include <list>

namespace niftk
{

/**
 * \class NiftyCalVideoCalibrationManager
 * \brief Manager class to perform video calibration as provided by NiftyCal.
 *
 * Note: This class is stateful and not thread safe.
 *
 * This class was originally intended to work with Zhang's method, which requires
 * N (typically 5-10) views of a calibration pattern. However, it was modified to
 * work with Tsai's method (1 view, coplanar or non-coplanar). So, there may
 * be some residual technical debt to tidy up and simplify.
 */
class NIFTKNIFTYCAL_EXPORT NiftyCalVideoCalibrationManager : public itk::Object
{
public:

  enum CalibrationPatterns
  {
    // Order must match that in niftk::CameraCalViewPreferencePage
    CHESS_BOARD,
    CIRCLE_GRID,
    APRIL_TAGS,
    TEMPLATE_MATCHING_CIRCLES,
    TEMPLATE_MATCHING_RINGS,
    TEMPLATE_MATCHING_NON_COPLANAR_CIRCLES,
    TEMPLATE_MATCHING_NON_COPLANAR_RINGS
  };

  enum HandEyeMethod
  {
    // Order must match that in niftk::CameraCalViewPreferencePage
    TSAI_1989,
    SHAHIDI_2002,
    MALTI_2013,
    NON_LINEAR_EXTRINSIC
  };

  const static bool                DefaultDoIterative;
  const static bool                DefaultDo3DOptimisation;
  const static bool                DefaultDoClustering;
  const static unsigned int        DefaultNumberOfSnapshotsForCalibrating;
  const static double              DefaultScaleFactorX;
  const static double              DefaultScaleFactorY;
  const static unsigned int        DefaultGridSizeX;
  const static unsigned int        DefaultGridSizeY;
  const static CalibrationPatterns DefaultCalibrationPattern;
  const static HandEyeMethod       DefaultHandEyeMethod;
  const static std::string         DefaultTagFamily;
  const static unsigned int        DefaultMinimumNumberOfPoints;
  const static bool                DefaultUpdateNodes;
  const static bool                DefaultModelIsStationary;
  const static bool                DefaultCameraIsStationary;
  const static bool                DefaultSaveOutputBeforeCalibration;
  const static bool                DefaultResetCalibrationIfNodeChanges;

  mitkClassMacroItkParent(NiftyCalVideoCalibrationManager, itk::Object);
  itkNewMacro(NiftyCalVideoCalibrationManager);

  void SetDataStorage(const mitk::DataStorage::Pointer storage);

  void SetLeftImageNode(mitk::DataNode::Pointer node);
  mitk::DataNode::Pointer GetLeftImageNode() const;

  void SetRightImageNode(mitk::DataNode::Pointer node);
  mitk::DataNode::Pointer GetRightImageNode() const;

  void SetTrackingTransformNode(mitk::DataNode::Pointer node);
  itkGetMacro(TrackingTransformNode, mitk::DataNode::Pointer);

  itkSetMacro(ModelTransformNode, mitk::DataNode::Pointer);
  itkGetMacro(ModelTransformNode, mitk::DataNode::Pointer);

  itkSetMacro(NumberOfSnapshotsForCalibrating, unsigned int);
  itkGetMacro(NumberOfSnapshotsForCalibrating, unsigned int);

  itkSetMacro(DoIterative, bool);
  itkGetMacro(DoIterative, bool);

  itkSetMacro(Do3DOptimisation, bool);
  itkGetMacro(Do3DOptimisation, bool);

  itkSetMacro(ModelIsStationary, bool);
  itkGetMacro(ModelIsStationary, bool);

  itkSetMacro(CameraIsStationary, bool);
  itkGetMacro(CameraIsStationary, bool);

  itkSetMacro(SaveOutputBeforeCalibration, bool);
  itkGetMacro(SaveOutputBeforeCalibration, bool);

  itkSetMacro(ResetCalibrationIfNodeChanges, bool);
  itkGetMacro(ResetCalibrationIfNodeChanges, bool);

  itkSetMacro(DoClustering, bool);
  itkGetMacro(DoClustering, bool);

  itkSetMacro(UpdateNodes, bool);
  itkGetMacro(UpdateNodes, bool);

  itkSetMacro(ScaleFactorX, double);
  itkGetMacro(ScaleFactorX, double);

  itkSetMacro(ScaleFactorY, double);
  itkGetMacro(ScaleFactorY, double);

  itkSetMacro(GridSizeX, unsigned int);
  itkGetMacro(GridSizeX, unsigned int);

  itkSetMacro(GridSizeY, unsigned int);
  itkGetMacro(GridSizeY, unsigned int);

  itkSetMacro(MinimumNumberOfPoints, unsigned int);
  itkGetMacro(MinimumNumberOfPoints, unsigned int);

  itkSetMacro(CalibrationPattern, CalibrationPatterns);
  itkGetMacro(CalibrationPattern, CalibrationPatterns);

  itkSetMacro(HandeyeMethod, HandEyeMethod);
  itkGetMacro(HandeyeMethod, HandEyeMethod);

  itkSetMacro(TagFamily, std::string);
  itkGetMacro(TagFamily, std::string);

  void SetModelFileName(const std::string& fileName);
  itkGetMacro(ModelFileName, std::string);

  void SetOutputPrefixName(const std::string& dirName);
  itkGetMacro(OutputPrefixName, std::string);

  void SetReferenceDataFileNames(const std::string& imageFileName,
                                 const std::string& pointsFileName);
  itkGetMacro(ReferenceImageFileName, std::string);
  itkGetMacro(ReferencePointsFileName, std::string);

  void SetTemplateImageFileName(const std::string& fileName);
  itkGetMacro(TemplateImageFileName, std::string);

  void SetModelTransformFileName(const std::string& fileName);
  itkGetMacro(ModelTransformFileName, std::string);

  bool isStereo() const;

  unsigned int GetNumberOfSnapshots() const;

  /**
   * \brief Clears down the internal points and image arrays,
   * so calibration will restart with zero data.
   */
  void Restart();

  /**
   * \brief Grabs images and tracking, and runs the point extraction.
   * \return Returns true if successful and false otherwise.
   */
  bool Grab();

  /**
   * \brief Removes the last grabbed snapshot.
   */
  void UnGrab();

  /**
   * \brief Performs the actual calibration.
   *
   * This can be mono, stereo, iterative and include hand-eye,
   * depending on the configuration parameters stored in this class.
   *
   * \return calibration message with all results in.
   */
  std::string Calibrate();

  /**
   * \brief Saves a bunch of standard (from a NifTK perspective)
   * calibration files to the output dir, overwriting existing files.
   */
  void Save();

  /**
   * \brief To update the camera to world in mitk::DataStorage so
   * we can watch the current calibration live in real-time.
   */
  void UpdateCameraToWorldPosition();

  /**
   * \brief Transforms m_ModelPoints into m_ModelPointsToVisualise;
   */
  void UpdateVisualisedPoints();

  /**
   * \brief Loads our NifTK standard named calibration files from disk,
   * overwriting all the existing, intrinsic, distortion, hand-eye etc.
   */
  void LoadCalibrationFromDirectory(const std::string& dirName);

protected:

  NiftyCalVideoCalibrationManager(); // Purposefully hidden.
  virtual ~NiftyCalVideoCalibrationManager(); // Purposefully hidden.

  NiftyCalVideoCalibrationManager(const NiftyCalVideoCalibrationManager&); // Purposefully not implemented.
  NiftyCalVideoCalibrationManager& operator=(const NiftyCalVideoCalibrationManager&); // Purposefully not implemented.

private:

  /**
   * \brief Extracts mitk::Image from imageNode, converts to OpenCV and makes grey-scale.
   */
  void ConvertImage(mitk::DataNode::Pointer imageNode, cv::Mat& outputImage);

  /**
   * \brief Runs the niftk::IPoint2DDetector on the image.
   *
   * The preferences page will determine the current preferred method of extraction.
   * e.g. Chessboard, grid of circles, AprilTags.
   */
  bool ExtractPoints(int imageIndex, const cv::Mat& image);

  /**
   * \brief Converts OpenCV rotation vectors and translation vectors to matrices.
   */
  std::list<cv::Matx44d> ExtractCameraMatrices(int imageIndex);

  /**
   * \brief Extracts a set of tracking matrices.
   */
  std::list<cv::Matx44d> ExtractTrackingMatrices(bool shahidiOverride = false);

  /**
   * \brief Extracts a set of model (chessboard to tracker) matrices.
   */
  std::list<cv::Matx44d> ExtractModelMatrices();

  /**
   * \brief Converts a list of matrices to a vector.
   */
  std::vector<cv::Mat> ConvertMatrices(const std::list<cv::Matx44d>& list);

  /**
   * \brief Actually does Tsai's 1989 hand-eye calibration for imageIndex=0=left, imageIndex=1=right camera.
   */
  cv::Matx44d DoTsaiHandEye(int imageIndex);

  /**
   * \brief Actually does Shahidi 2002 hand-eye calibration for imageIndex=0=left, imageIndex=1=right camera.
   */
  cv::Matx44d DoShahidiHandEye(int imageIndex);

  /**
   * \brief Actually does Malti's 2013 hand-eye calibration for imageIndex=0=left, imageIndex=1=right camera.
   */
  cv::Matx44d DoMaltiHandEye(int imageIndex);

  /**
   * \brief Actually does a full non-linear calibration of all extrinsic parameters.
   */
  cv::Matx44d DoFullExtrinsicHandEye(int imageIndex);

  /**
   * \brief Bespoke method to calculate independent leftHandEye and rightHandEye,
   * by optimising all parameters in stereo, simultaneously.
   */
  void DoFullExtrinsicHandEyeInStereo(cv::Matx44d& leftHandEye, cv::Matx44d& rightHandEye);

  /**
   * \brief Saves list of images that were used for calibration.
   */
  void SaveImages(const std::string& prefix,
                  const std::list<std::pair<std::shared_ptr<niftk::IPoint2DDetector>, cv::Mat> >&
                  );

  /**
   * \brief Saves list of points that were used for calibration.
   */
  void SavePoints(const std::string& prefix, const std::list<niftk::PointSet>& points);

  /**
   * \brief Utility method to apply intrinsic and distortion parameters
   * to an image by setting a mitk::CameraIntrinsics property.
   */
  void SetIntrinsicsOnImage(const cv::Mat& intrinsics,
                            const cv::Mat& distortion,
                            const std::string& propertyName,
                            mitk::DataNode::Pointer image);

  /**
   * \brief Utility method to apply a stereo (left-to-right) transformation
   * to an image node, by setting a property containing the matrix.
   *
   * Note: NifTK standard is Right-to-Left, whereas NiftyCal and OpenCV
   * by default use Left-To-Right. So, here, this method is private
   * as we specifically handle data in this class, which by NiftyCal and OpenCV
   * convention use Left-to-Right.
   */
  void SetStereoExtrinsicsOnImage(const cv::Mat& leftToRightRotationMatrix,
                                  const cv::Mat& leftToRightTranslationVector,
                                  const std::string& propertyName,
                                  mitk::DataNode::Pointer image
                                  );

  /**
   * \brief Combined method to update properties on image nodes, and to
   * move the model points to the correct location in space.
   */
  void UpdateDisplayNodes();

  cv::Matx44d GetInitialHandEye(int imageIndex);
  cv::Matx44d GetInitialModelToWorld();
  double      GetStereoRMSReconstructionError(const cv::Matx44d& handEye);
  double      GetMonoRMSReconstructionError(const cv::Matx44d& handEye);
  cv::Matx44d GetModelToWorld(const cv::Matx44d& handEye);

  typedef mitk::GenericProperty<itk::Matrix<float, 4, 4> > MatrixProperty;

  // Data from Plugin/DataStorage.
  mitk::DataStorage::Pointer                     m_DataStorage;
  mitk::DataNode::Pointer                        m_ImageNode[2];
  mitk::DataNode::Pointer                        m_TrackingTransformNode;
  mitk::DataNode::Pointer                        m_ModelTransformNode;

  // Data from preferences.
  bool                                           m_DoIterative;
  bool                                           m_Do3DOptimisation;
  bool                                           m_DoClustering;
  bool                                           m_ModelIsStationary;
  bool                                           m_CameraIsStationary;
  bool                                           m_SaveOutputBeforeCalibration;
  bool                                           m_ResetCalibrationIfNodeChanges;
  unsigned int                                   m_NumberOfSnapshotsForCalibrating;
  std::string                                    m_ModelFileName;
  double                                         m_ScaleFactorX;
  double                                         m_ScaleFactorY;
  unsigned int                                   m_GridSizeX;
  unsigned int                                   m_GridSizeY;
  CalibrationPatterns                            m_CalibrationPattern;
  HandEyeMethod                                  m_HandeyeMethod;
  std::string                                    m_TagFamily;
  std::string                                    m_OutputPrefixName;
  std::string                                    m_OutputDirName;
  std::string                                    m_ModelTransformFileName;
  std::string                                    m_ReferenceImageFileName;
  std::string                                    m_ReferencePointsFileName;
  bool                                           m_UpdateNodes;
  unsigned int                                   m_MinimumNumberOfPoints;
  std::string                                    m_TemplateImageFileName;
  std::string                                    m_CalibrationDirName;

  // Data used for temporary storage
  cv::Mat                                        m_TmpImage[2];

  // Data used for calibration.
  cv::Size2i                                     m_ImageSize;
  std::pair< cv::Mat, niftk::PointSet>           m_ReferenceDataForIterativeCalib;
  cv::Mat                                        m_TemplateImage;
  cv::Matx44d                                    m_StaticModelTransform;
  cv::Matx44d                                    m_ModelTransform;
  niftk::Model3D                                 m_ModelPoints;
  mitk::PointSet::Pointer                        m_ModelPointsToVisualise;
  mitk::DataNode::Pointer                        m_ModelPointsToVisualiseDataNode;
  std::list<niftk::PointSet>                     m_Points[2];
  std::list<
    std::pair<
      std::shared_ptr<niftk::IPoint2DDetector>,
      cv::Mat>
    >                                            m_OriginalImages[2];
  std::list<
    std::pair<
      std::shared_ptr<niftk::IPoint2DDetector>,
      cv::Mat>
    >                                            m_ImagesForWarping[2];
  std::list<cv::Matx44d>                         m_TrackingMatrices;
  std::vector<mitk::DataNode::Pointer>           m_TrackingMatricesDataNodes;
  std::list<cv::Matx44d>                         m_ModelTrackingMatrices;

  // Calibration result
  cv::Mat                                        m_Intrinsic[2];
  cv::Mat                                        m_Distortion[2];
  std::vector<cv::Mat>                           m_Rvecs[2];
  std::vector<cv::Mat>                           m_Tvecs[2];
  cv::Mat                                        m_EssentialMatrix;
  cv::Mat                                        m_FundamentalMatrix;
  cv::Mat                                        m_LeftToRightRotationMatrix;
  cv::Mat                                        m_LeftToRightTranslationVector;
  std::vector<cv::Matx44d>                       m_HandEyeMatrices[2];
  cv::Matx44d                                    m_ModelToWorld;
  std::string                                    m_CalibrationResult;

}; // end class

} // end namespace

#endif
