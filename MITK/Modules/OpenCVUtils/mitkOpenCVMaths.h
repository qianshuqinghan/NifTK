/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkOpenCVMaths_h
#define mitkOpenCVMaths_h

#include "niftkOpenCVUtilsExports.h"
#include "mitkOpenCVPointTypes.h"
#include <cv.h>
#include <mitkPointSet.h>
#include <vtkMatrix4x4.h>

/**
 * \file mitkOpenCVMaths.h
 * \brief Various simple mathematically based functions using OpenCV data types.
 */
namespace mitk {

/**
 * \brief Subtracts a point (e.g. the centroid) from a list of points.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<cv::Point3d> SubtractPointFromPoints(const std::vector<cv::Point3d> listOfPoints, const cv::Point3d& point);


/**
 * \brief Converts mitk::PointSet to vector of cv::Point3d, but you lose the point ID contained within the mitk::PointSet.
 * Optionally you can fill missing indices with NaN values, to preserve point to point correspondence.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<cv::Point3d> PointSetToVector(const mitk::PointSet::Pointer& pointSet,
    bool fillMissingIndecesWithNaN = false);


/**
 * \brief Haven't found a direct method to do this yet.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void MakeIdentity(cv::Matx44d& outputMatrix);


/**
 * \brief Calculates 1/N Sum (q_i * qPrime_i^t) where q_i and qPrime_i are column vectors, so the product is a 3x3 matrix.
 * \see Least-Squares Fitting of two, 3-D Point Sets, Arun, 1987, DOI=10.1109/TPAMI.1987.4767965, where this calculates matrix H.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx33d CalculateCrossCovarianceH(const std::vector<cv::Point3d>& q, const std::vector<cv::Point3d>& qPrime);


/**
 * \brief Helper method to do the main SVD bit of the point based registration, and handle the degenerate conditions mentioned in Aruns paper.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool DoSVDPointBasedRegistration(const std::vector<cv::Point3d>& fixedPoints,
  const std::vector<cv::Point3d>& movingPoints,
  cv::Matx33d& H,
  cv::Point3d &p,
  cv::Point3d& pPrime,
  cv::Matx44d& outputMatrix,
  double &fiducialRegistrationError
  );


/**
 * \brief Calculates Fiducial Registration Error by multiplying the movingPoints by the matrix, and comparing with fixedPoints.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double CalculateFiducialRegistrationError(const std::vector<cv::Point3d>& fixedPoints,
  const std::vector<cv::Point3d>& movingPoints,
  const cv::Matx44d& matrix
  );


/**
 * \brief Converts format of input to call the other CalculateFiducialRegistrationError method.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double CalculateFiducialRegistrationError(const mitk::PointSet::Pointer& fixedPointSet,
  const mitk::PointSet::Pointer& movingPointSet,
  vtkMatrix4x4& vtkMatrix
  );


/**
 * \brief Simply copies the translation vector and rotation matrix into the 4x4 matrix.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void ConstructAffineMatrix(const cv::Matx31d& translation, const cv::Matx33d& rotation, cv::Matx44d& matrix);


/**
 * \brief Copies matrix to vtkMatrix.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void CopyToVTK4x4Matrix(const cv::Matx44d& matrix, vtkMatrix4x4& vtkMatrix);


/**
 * \brief Copies matrix to openCVMatrix.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void CopyToOpenCVMatrix(const vtkMatrix4x4& matrix, cv::Matx44d& openCVMatrix);


/**
 * \brief Copies to VTK matrix, throwing exceptions if input is not 4x4.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void CopyToVTK4x4Matrix(const cv::Mat& input, vtkMatrix4x4& output);


/**
 * \brief Copies to OpenCV matrix, throwing exceptions if output is not 4x4.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void CopyToOpenCVMatrix(const vtkMatrix4x4& input, cv::Mat& output);


/**
 * \brief Copies to OpenCV matrix, throwing exceptions if output is not 4x4.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void CopyToOpenCVMatrix(const cv::Matx44d& matrix, cv::Mat& output);

/**
 * \brief Generates a rotation about X-axis, given a Euler angle in radians.
 * \param rx angle in radians
 * \return a new [3x3] rotation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx33d ConstructEulerRxMatrix(const double& rx);


/**
 * \brief Generates a rotation about Y-axis, given a Euler angle in radians.
 * \param ry angle in radians
 * \return a new [3x3] rotation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx33d ConstructEulerRyMatrix(const double& ry);


/**
 * \brief Generates a rotation about Z-axis, given a Euler angle in radians.
 * \param rz angle in radians
 * \return a new [3x3] rotation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx33d ConstructEulerRzMatrix(const double& rz);


/**
 * \brief Generates a rotation matrix, given Euler angles in radians.
 * \param rx angle in radians
 * \param ry angle in radians
 * \param rz angle in radians
 * \return a new [3x3] rotation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx33d ConstructEulerRotationMatrix(const double& rx, const double& ry, const double& rz);


/**
 * \brief Converts Euler angles in radians to the Rodrigues rotation vector (axis-angle convention) mentioned in OpenCV.
 * \param rx Euler angle rotation about x-axis in radians
 * \param ry Euler angle rotation about y-axis in radians
 * \param rz Euler angle rotation about z-axis in radians
 * \return A new [1x3] matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx13d ConvertEulerToRodrigues(
  const double& rx,
  const double& ry,
  const double& rz
  );


/**
 * \brief multiplies a set of points by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector <cv::Point3d> operator*(const cv::Mat& M, const std::vector<cv::Point3d>& p);

/**
 * \brief multiplies a set of points by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<cv::Point3d> operator*(const cv::Matx44d& M, const std::vector<cv::Point3d>& p);


/**
 * \brief multiplies a set of points and corresponding scalar values by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector < mitk::WorldPoint > operator*(const cv::Mat& M,
    const std::vector< mitk::WorldPoint >& p);

/**
 * \brief multiplies a set of points and corresponding scalar values by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<mitk::WorldPoint> operator*(const cv::Matx44d& M, const std::vector<mitk::WorldPoint>& p);


/**
 * \brief multiplies a point and corresponding scalar value by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT mitk::WorldPoint  operator*(const cv::Mat& M,
    const mitk::WorldPoint & p);

/**
 * \brief multiplies a point and corresponding scalar value by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT mitk::WorldPoint operator*(const cv::Matx44d& M, const mitk::WorldPoint& p);

/**
 * \brief multiplies a  point by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point3d operator*(const cv::Mat& M, const cv::Point3d& p);

/**
 * \brief multiplies a  point by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point3d operator*(const cv::Matx44d& M, const cv::Point3d& p);

/**
 * \brief multiplies a  point pair by a 4x4 transformation matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::pair < cv::Point3d, cv::Point3d > TransformPointPair (const cv::Matx44d& M, 
    const std::pair < cv::Point3d, cv::Point3d>& p);

/**
 * \brief Tests equality of 2 2d points. The openCV == operator struggles on floating points, 
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool NearlyEqual(const cv::Point2d& p1, const cv::Point2d& p2, const double& tolerance );

/**
 * \brief Tests equality of 2 3d points. The openCV == operator struggles on floating points, 
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool NearlyEqual(const cv::Point3d& p1, const cv::Point3d& p2, const double& tolerance );

/**
 * \brief Tests whether two cv::Mat have the same header info (dimension and data type), 
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool ImageHeadersEqual(const cv::Mat& m1, const cv::Mat& m2);

/**
 * \brief Tests whether two cv::Mat have the same data 
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool ImageDataEqual(const cv::Mat& m1, const cv::Mat& m2, const double& tolerance );


/**
 * \brief Divides a 2d point by an integer (x=x1/n, y=y1/2)
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point2d operator/(const cv::Point2d& p, const int& n);


/**
 * \brief Multiplies the components of a 2d point by an integer (x=x1*x2, y=y1*y2)
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point2d operator*(const cv::Point2d& p1, const cv::Point2d& p2);


/**
 * \ brief Finds the intersection point of two 2D lines defined as cv::Vec4i
 * x0,y1 = line1[0], line1[1], x1,y1 = line1[2],line1[3]
 * x0,y1 = line2[0], line2[1], x1,y1 = line2[2],line2[3]
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point2d FindIntersect(const cv::Vec4i& line1 , const cv::Vec4i& line2);

/**
 * \ brief Finds all the intersection points of a vector of  2D lines defined as cv::Vec4i
 * \ param reject any points that don't fall on both lines
 * \ param reject any points formed by non perpendicular lines
 * \ param the tolerance to use for perpendicularity test
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector <cv::Point2d> FindIntersects (const std::vector <cv::Vec4i>&,
    const bool& RejectIfNotOnBothLines = false , const bool& RejectIfNotPerpendicular = false, 
    const double& angleTolerance = 45.0);

/**
 * \ brief Returns true if the passed point falls within the limits defined by the passed interval
 * x0,y0 = interval[0], interval[1], x1,y1 = interval[2], interval[3]
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool PointInInterval (const cv::Point2d& point , const cv::Vec4i& interval);

/**
 * \ brief Finds the angle, in radians between two line segments
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double AngleBetweenLines(cv::Vec4i , cv::Vec4i);

/**
 * \ brief Checks if two line segments are perpendicular, within a tolerance set in degrees
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool CheckIfLinesArePerpendicular(cv::Vec4i , cv::Vec4i, double toleranceInDegrees);

/**
 * \brief Calculates the centroid of a vector of points.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point2d GetCentroid(const std::vector<cv::Point2d>& points, bool RefineForOutliers = false, cv::Point2d* StandardDeviation = NULL);


/**
 * \brief Calculates the centroid of a vector of points.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point3d GetCentroid(const std::vector<cv::Point3d>& points, bool RefineForOutliers = false, cv::Point3d* StandardDeviation = NULL);


/**
 * \brief From rotations in radians and translations in millimetres, constructs a 4x4 transformation matrix.
 * \param rx Euler rotation about x-axis in radians
 * \param ry Euler rotation about y-axis in radians
 * \param rz Euler rotation about z-axis in radians
 * \param tx translation in millimetres along x-axis
 * \param ty translation in millimetres along y-axis
 * \param tz translation in millimetres along z-axis
 * \return a new [4x4] matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx44d ConstructRigidTransformationMatrix(
  const double& rx,
  const double& ry,
  const double& rz,
  const double& tx,
  const double& ty,
  const double& tz
  );


/**
 * \brief From Rodrigues rotation parameters and translations in millimetres, constructs a 4x4 transformation matrix.
 * \param r1 Rodrigues rotation
 * \param r2 Rodrigues rotation
 * \param r3 Rodrigues rotation
 * \param tx translation in millimetres along x-axis
 * \param ty translation in millimetres along y-axis
 * \param tz translation in millimetres along z-axis
 * \return a new [4x4] matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx44d ConstructRodriguesTransformationMatrix(
  const double& r1,
  const double& r2,
  const double& r3,
  const double& tx,
  const double& ty,
  const double& tz
  );


/**
 * \brief Constructs a scaling matrix from sx, sy, sz where the scale factors simply appear on the diagonal.
 * \param sx scale factor in x direction
 * \param sy scale factor in y direction
 * \param sz scale factor in z direction
 * \return a new [4x4] matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx44d ConstructScalingTransformation(const double& sx, const double& sy, const double& sz = 1);


/**
 * \brief Constructs an affine transformation, without skew using the specified parameters, where rotations are in degrees.
 * \param rx Euler rotation about x-axis in radians
 * \param ry Euler rotation about y-axis in radians
 * \param rz Euler rotation about z-axis in radians
 * \param tx translation in millimetres along x-axis
 * \param ty translation in millimetres along y-axis
 * \param tz translation in millimetres along z-axis
 * \param sx scale factor in x direction
 * \param sy scale factor in y direction
 * \param sz scale factor in z direction
 * \return a new [4x4] matrix
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Matx44d ConstructSimilarityTransformationMatrix(
    const double& rx,
    const double& ry,
    const double& rz,
    const double& tx,
    const double& ty,
    const double& tz,
    const double& sx,
    const double& sy,
    const double& sz
    );


/**
 * \brief Takes a point vector and finds the minimum value in each dimension. Returns the 
 * minimum values. Optionally returns the indexes of the minium values.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point3d FindMinimumValues ( std::vector < cv::Point3d > inputValues, cv::Point3i * indexes = NULL );


/**
 * \brief Returns the mean pixel errors for the right and left sets of projected points
 * \param the measured projected points
 * \param the actual projected points
 * \param optional pointer to return standard deviations
 * \param optionally constrain calculation for only one projected point pair in each vector,
 * if -1 all projected point pairs are used
 * \param discard point pairs with timing errors in excess of allowableTimingError
 * \param if duplicateLines true, only every second entry in measured and actual is used, 
 * this is useful when running from stereo video and tracking data.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT mitk::ProjectedPointPair MeanError (
    std::vector < mitk::ProjectedPointPairsWithTimingError > measured , 
    std::vector < mitk::ProjectedPointPairsWithTimingError > actual, 
    mitk::ProjectedPointPair * StandardDeviations = NULL , int index = -1,
    long long allowableTimingError = 30e6, bool duplicateLines = true );


/** 
 * \brief Returns the RMS error between two projected point vectors
 * \param the measured projected points
 * \param the actual projected points
 * \param optionally constrain calculation for only one projected point pair in each vector,
 * if -1 all projected point pairs are used
 * \param discard point pairs where the error is above the mean error +/- n standard deviations.
 * \param discard point pairs with timing errors in excess of allowableTimingError
 * \param if duplicateLines true, only every second entry in measured and actual is used, 
 * this is useful when running from stereo video and tracking data.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::pair <double,double> RMSError
  (std::vector < mitk::ProjectedPointPairsWithTimingError > measured , 
    std::vector < mitk::ProjectedPointPairsWithTimingError > actual, int index = -1 ,
    cv::Point2d outlierSD = cv::Point2d (2.0,2.0) , long long allowableTimingError = 30e6,
    bool duplicateLines = true);


/**
 * \brief perturbs a 4x4 matrix with a 6 dof rigid transform. The transform is
 * defined by three translations and 3 rotations, in Degrees
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat PerturbTransform (
    const cv::Mat transformIn,
    const double tx, const double ty, const double tz, 
    const double rx, const double ry, const double rz );

/** 
 * \brief Searches through vector of 2D points to find the one closest (by distance)
 * to the passed point, and returns the index of that point
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point2d FindNearestPoint ( const cv::Point2d& point,
    const std::vector < cv::Point2d >& matchingPonints , 
    double* minRatio = NULL , unsigned int * index = NULL );

/** 
 * \brief Searches through vector of 3D points and lines to find the one closest (by distance)
 * to the passed point. Returns an empty PickedPoint if no point found 
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT mitk::PickedObject FindNearestPickedObject ( const mitk::PickedObject& point,
    const std::vector < mitk::PickedObject >& matchingPoints , 
    double* minRatio = NULL );

/**
 * \brief Compare two cv point based on their distance from 0,0
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool DistanceCompare ( const cv::Point2d& p1,
    const cv::Point2d& p2 );

/**
 * \brief works out the rigid rotation correspondence between two sets of corresponding 
 * rigid body transforms
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat Tracker2ToTracker1Rotation (
    const std::vector<cv::Mat>& Tracker1ToWorld1, const std::vector<cv::Mat>& World2ToTracker2,
    double& Residual);


/**
 * \brief works out the rigid translation correspondence between two sets of corresponding 
 * rigid body transforms
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat Tracker2ToTracker1Translation (
    const std::vector<cv::Mat>& Tracker1ToWorld1, const std::vector<cv::Mat>& World2ToTracker2,
    double& Residual, const cv::Mat & rcg);


/**
 * \brief works out the rigid rotation and translation correspondence between two sets of corresponding 
 * rigid body transforms
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat Tracker2ToTracker1RotationAndTranslation (
    const std::vector<cv::Mat>& Tracker1ToWorld1, const std::vector<cv::Mat>& World2ToTracker2,
    std::vector<double>& Residuals, cv::Mat* World2ToWorld1 = NULL );


/**
 * \brief Flips the matrices in the vector from left handed coordinate
 * system to right handed and vice versa
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<cv::Mat> FlipMatrices (const std::vector<cv::Mat> matrices);


/**
 * \brief find the average of a vector of 4x4 matrices
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat AverageMatrices(const std::vector<cv::Mat>& matrices);


 /**
  * \brief Sorts the matrices based on the translations , and returns the order
  */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<int> SortMatricesByDistance (const std::vector<cv::Mat> matrices);


/**
 * \brief Sorts the matrices based on the rotations, and returns the order
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::vector<int> SortMatricesByAngle (const std::vector<cv::Mat> matrices);


/**
 * \brief Returns the angular distance between two rotation matrices
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double AngleBetweenMatrices(cv::Mat Mat1 , cv::Mat Mat2);


/**
 * \brief Returns the distance between two 4x4 matrices
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceBetweenMatrices(cv::Mat Mat1 , cv::Mat Mat2);


/**
 * \brief Converts a 3x3 rotation matrix to a quaternion
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Mat DirectionCosineToQuaternion(cv::Mat dc_Matrix);


/**
 * \brief Specific method that inverts a matrix without SVD or decomposition,
 * because the input is known to be orthonormal.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void InvertRigid4x4Matrix(const CvMat& input, CvMat& output);


/**
 * \brief Overloaded invert method that calls the C-looking one.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void InvertRigid4x4Matrix(const cv::Matx44d& input, cv::Matx44d& output);


/**
 * \brief Overloaded invert method that calls the C-looking one.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void InvertRigid4x4Matrix(const cv::Mat& input, cv::Mat& output);


/**
 * \brief Interpolates between two matrices.
 * \param proportion is defined as between [0 and 1], where 0 gives exactly the before matrix,
 * 1 gives exactly the after matrix, and the proportion is a linear proportion between them over which to interpolate.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void InterpolateTransformationMatrix(const cv::Mat& before, const cv::Mat& after, const double& proportion, cv::Mat& output);


/**
 * \see InterpolateTransformationMatrix(const cv::Mat& before, const cv::Mat& after, const double& proportion, cv::Mat& output)
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void InterpolateTransformationMatrix(const cv::Matx44d& before, const cv::Matx44d& after, const double& proportion, cv::Matx44d& output);

/**
 * \brief returns the matrix type as a string
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::string MatrixType(const cv::Mat& matrix);

/** 
 * \brief check if point has a NaN value
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool IsNaN(const cv::Point2d&);

/** 
 * \brief check if point has a NaN value
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool IsNaN(const cv::Point3d&);

/** 
 * \brief check if 2D point has a NaN  or inf value
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool IsNotNaNorInf(const cv::Point2d&);

/** 
 * \brief check if 3D point has a NaN  or inf value
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT bool IsNotNaNorInf(const cv::Point3d&);

/**
 * \brief calculates the distance between a line and a point
 * \param the line defined by two points on the line
 * \param the point
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceToLine ( const std::pair<cv::Point3d, cv::Point3d>& line, const cv::Point3d& point);

/**
 * \brief calculates the distance between two points
 * \param point 1
 * \param point 2
 * \param optional the signed distance vector between the two points
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceBetweenTwoPoints ( const cv::Point3d& p1 , const cv::Point3d& p2, cv::Point3d* delta = NULL );

/**
 * \brief calculates the average shortest distance between two splines.  
 * \param control points for spline 1, each of which is used for a measurement
 * \param control points for spline 2, which form the line segments for the measurement
 * \param order the polynomial order of the spline (only 1 is implemented)
 * \param optional the mean signed distance vector between the two points
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceBetweenTwoSplines ( const std::vector <cv::Point3d>& s0,
    const std::vector <cv::Point3d>& s1, unsigned int splineOrder , cv::Point3d* delta = NULL );

/**
 * \brief calculates the distance between a line segment and a point
 * \param the line defined by the two end points of the line segment
 * \param the point
 * \param optional the signed distance vector between the two points
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceToLineSegment ( const std::pair<cv::Point3d, cv::Point3d>& line, const cv::Point3d& point, cv::Point3d* delta = NULL);

/**
 * \brief calculates the shortest distance between two lines  
 * \param line 1 define as x = P0 + lambda u
 * \param line 2 defined as x = Q0 + lambda v
 * \param holder to return the midpoint
 * \param optionally return the closest point on the second line
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceBetweenLines ( const cv::Point3d& P0, const cv::Point3d& u, 
    const cv::Point3d& Q0, const cv::Point3d& v , cv::Point3d& midpoint );

/**
 * \brief calculates the shortest distance between a line and a segment 
 * the point must lie on the segment
 * \param line 1 define as x = P0 + lambda u
 * \param line 2 defined as the start and end points x0, x1
 * \param holder for the closest point on the second line
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DistanceBetweenLineAndSegment ( const cv::Point3d& P0, const cv::Point3d& u, 
    const cv::Point3d& x0, const cv::Point3d& x1 , cv::Point3d& closestPointOnSecondLine );


/**
 * \brief converts a line defined by two points on the line to the same line defined by a single point and it's unit vector
 * \param line 1 define as x1 x2
 * \return lin defined as x = Q0 + lambda v
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT std::pair < cv::Point3d , cv::Point3d > TwoPointsToPLambda ( const std::pair < cv::Point3d , cv::Point3d >& twoPointLine );

/**
 * \brief Calculates the cross product of two vectors (cv::Point3D)
 * \param the vectors as points
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT cv::Point3d CrossProduct ( const cv::Point3d& p1, const cv::Point3d& p2);

/**
 * \brief Calculates the dot product of two vectors (cv::Point3D)
 * \param the vectors as points
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double DotProduct ( const cv::Point3d& p1, const cv::Point3d& p2);

/**
 * \brief Calculates the norm product of a vectors (cv::Point3D)
 * \param the vector as cv::Point3D
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT double Norm ( const cv::Point3d& p1);

/** 
 * \brief Removes points outside the passed limits
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT unsigned int RemoveOutliers ( std::vector <cv::Point3d>& points,
  const double& xLow, const double& xHigh,
  const double& yLow, const double& yHigh,
  const double& zLow, const double& zHigh);

/** 
 * \brief Removes points outside the passed limits
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT unsigned int RemoveOutliers ( std::vector <std::pair < cv::Point3d, double > >& points,
  const double& xLow, const double& xHigh,
  const double& yLow, const double& yHigh,
  const double& zLow, const double& zHigh);

/**
 * \brief Decomposes a rigid body matrix into Rodrigues Rotations and Translations.
 */
extern "C++" NIFTKOPENCVUTILS_EXPORT void ExtractRigidBodyParameters(const vtkMatrix4x4& matrix, mitk::Point3D& outputRodriguesRotationParameters, mitk::Point3D& outputTranslationParameters);

} // end namespace

#endif



