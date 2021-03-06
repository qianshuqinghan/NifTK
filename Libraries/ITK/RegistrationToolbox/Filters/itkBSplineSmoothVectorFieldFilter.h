/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkBSplineSmoothVectorFieldFilter_h
#define itkBSplineSmoothVectorFieldFilter_h
#include "itkSmoothVectorFieldFilter.h"
#include <itkVectorNeighborhoodOperatorImageFilter.h>
#include "itkBSplineOperator.h"

namespace itk {
/** 
 * \class BSplineSmoothVectorFieldFilter
 * \brief Class that takes a vector field, and applies BSpline smoothing.
 * 
 * As of 16/01/2010, I have extended this to cope with time varying vector fields.
 * This means that you have an image dimension and a vector dimension, which can be
 * different. For example, you could have images of 2D vectors (x,y), but the image
 * could be 2D, or for example 3D (time varying field of 2D vectors).
 */
template <
  class TScalarType,                     // Data type for the vectors
  unsigned int NumberImageDimensions=3,  // Number of image dimensions
  unsigned int NumberVectorDimensions=NumberImageDimensions>   // Number of dimensions in the vectors
class ITK_EXPORT BSplineSmoothVectorFieldFilter :
  public SmoothVectorFieldFilter<TScalarType, NumberImageDimensions, NumberVectorDimensions>
{
public:

  /** Standard "Self" typedef. */
  typedef BSplineSmoothVectorFieldFilter                       Self;
  typedef SmoothVectorFieldFilter<TScalarType,
                                 NumberImageDimensions,
                                 NumberVectorDimensions>       Superclass;
  typedef SmartPointer<Self>                                   Pointer;
  typedef SmartPointer<const Self>                             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(BSplineSmoothVectorFieldFilter, SmoothVectorFieldFilter);

  /** Standard typedefs. */
  typedef typename Superclass::NeighborhoodOperatorType         NeighborhoodOperatorType;
  typedef BSplineOperator<TScalarType, NumberImageDimensions>   BSplineOperatorType;
  typedef FixedArray<TScalarType, NumberImageDimensions>        GridSpacingType;
  
  /** Set/Get the grid spacing to apply. */
  itkGetMacro(GridSpacing, GridSpacingType);
  itkSetMacro(GridSpacing, GridSpacingType);
  
protected:

  BSplineSmoothVectorFieldFilter();
  ~BSplineSmoothVectorFieldFilter() {};
  
  /** Get the BSpline operator. Called by baseclass. */
  NeighborhoodOperatorType* CreateOperator(int dimension);

private:

  /**
   * Prohibited copy and assingment. 
   */
  BSplineSmoothVectorFieldFilter(const Self&); 
  void operator=(const Self&); 

  /** To specify the grid spacing, to affect the kernel. */
  GridSpacingType m_GridSpacing;
  
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineSmoothVectorFieldFilter.txx"
#endif

#endif
