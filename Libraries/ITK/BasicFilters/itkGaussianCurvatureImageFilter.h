/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkGaussianCurvatureImageFilter_h
#define itkGaussianCurvatureImageFilter_h
#include "itkBasicFiniteDifferenceBaseClassImageFilter.h"

namespace itk
{

/**
 * \class GaussianCurvatureImageFilter
 * \brief Class to calculate Gaussian curvature of a scalar image.
 *
 * The output datatype should really be float or double.
 * Implements the formula as written on page 70 of [1] (in 2006 printing of the book).
 *
 * \par REFERENCES
 * \par
 * [1] Sethian, J.A. Level Set Methods and Fast Marching Methods Cambridge University Press. 1996.
 *
 * \sa itkGaussianCurvatureImageFilter
 * \sa itkBasicFiniteDifferenceBaseClassImageFilter
 * \sa itkMinimumCurvatureImageFilter
 * \sa itkMaximumCurvatureImageFilter
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT GaussianCurvatureImageFilter
  : public BasicFiniteDifferenceBaseClassImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef GaussianCurvatureImageFilter                                             Self;
  typedef BasicFiniteDifferenceBaseClassImageFilter<TInputImage, TOutputImage>     Superclass;
  typedef SmartPointer<Self>                                                       Pointer;
  typedef SmartPointer<const Self>                                                 ConstPointer;
  typedef typename TInputImage::RegionType                                         ImageRegionType;
  typedef typename TInputImage::PixelType                                          PixelType;
  typedef typename TInputImage::IndexType                                          IndexType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GaussianCurvatureImageFilter, BasicFiniteDifferenceBaseClassImageFilter);

  /** Print internal ivars */
  void PrintSelf(std::ostream& os, Indent indent) const;

protected:

  GaussianCurvatureImageFilter();
  virtual ~GaussianCurvatureImageFilter();

  // The main method to implement in derived classes, note, its threaded.
  virtual void ThreadedGenerateData(const ImageRegionType &outputRegionForThread, ThreadIdType threadId);

private:

  GaussianCurvatureImageFilter(const Self&); // purposely not implemented
  void operator=(const Self&); // purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGaussianCurvatureImageFilter.txx"
#endif

#endif
