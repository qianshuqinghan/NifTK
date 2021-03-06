/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/


#ifndef itkBSplineCurveFitMetric_h
#define itkBSplineCurveFitMetric_h

#include <vector>

#include <itkConceptChecking.h>
#include <itkMultipleValuedCostFunction.h>
#include <itkPoint.h>


namespace itk
{
  
/** \class BSplineCurveFitMetric
 * \brief Class to compute the goodness of fit of a BSpline to a set of data values.
 */

template < class IntensityType = int >
class ITK_EXPORT BSplineCurveFitMetric : public MultipleValuedCostFunction
{
  public:

  /** Standard class typedefs. */
  typedef BSplineCurveFitMetric        Self;
  typedef MultipleValuedCostFunction   Superclass;
  typedef SmartPointer<Self>           Pointer;
  typedef SmartPointer<const Self>     ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(BSplineCurveFitMetric, MultipleValuedCostFunction);
  
  /** Type used for representing point components  */
  typedef typename Superclass::ParametersValueType CoordinateRepresentationType;

  /**  Type of the parameters. */
  typedef typename MultipleValuedCostFunction::ParametersType ParametersType;
  typedef typename MultipleValuedCostFunction::MeasureType    MeasureType;
  typedef typename MultipleValuedCostFunction::DerivativeType DerivativeType;
  
  /**  Type of the Transform Base class */
  typedef Transform<CoordinateRepresentationType, 4, 4> TransformType;

  typedef typename TransformType::Pointer            TransformPointer;
  typedef typename TransformType::InputPointType     InputPointType;
  typedef typename TransformType::OutputPointType    OutputPointType;
  typedef typename TransformType::ParametersType     TransformParametersType;
  typedef typename TransformType::JacobianType       TransformJacobianType;

  /** Initialise the regularly spaced B-Spline control points */
  void Initialise( void );

  /** Return the number of parameters required by the Transform */
  unsigned int GetNumberOfParameters(void) const override;
  
  /** Return the number of values that are computed by the metric. */
  unsigned int GetNumberOfValues( void ) const override; 

  /** This method returns the value and derivative of the cost function corresponding
    * to the specified parameters    */ 
  virtual void GetValueAndDerivative( const ParametersType & parameters,
                                      MeasureType & value,
                                      DerivativeType & derivative ) const;

  /** This method returns the value of the cost function corresponding
    * to the specified parameters. */ 
  virtual MeasureType GetValue( const ParametersType & parameters ) const override;

  /** This method returns the derivative of the cost function corresponding
    * to the specified parameters. */ 
  virtual void GetDerivative( const ParametersType & parameters,
                              DerivativeType & derivative ) const override;


protected:

  BSplineCurveFitMetric();
  virtual ~BSplineCurveFitMetric() {};

  void PrintSelf(std::ostream& os, Indent indent) const override;


private:

  BSplineCurveFitMetric(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented


};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineCurveFitMetric.txx"
#endif

#endif
