/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include <itkImage.h>
#include <itkForwardDifferenceDisplacementFieldJacobianDeterminantFilter.h>
#include <itkVector.h>

//
// Taken from ITK and modified. 
// 
static bool TestDisplacementJacobianDeterminantValue(void)
{
  std::cout.precision(9);
  bool testPassed = true;
  const unsigned int ImageDimension = 2;

  typedef itk::Vector<float,ImageDimension> VectorType;
  typedef itk::Image<VectorType,ImageDimension> FieldType;

  // In this case, the image to be warped is also a vector field.
  typedef FieldType VectorImageType;
  typedef VectorImageType::PixelType  PixelType;
  typedef VectorImageType::IndexType  IndexType;

  //=============================================================

  std::cout << "Create the dispacementfield image pattern." << std::endl;
  VectorImageType::RegionType region;
  //NOTE:  Making the image size much larger than necessary in order to get
  //       some meaningful time measurements.  Simulate a 256x256x256 image.
  VectorImageType::SizeType size = {{4096, 4096}};
  region.SetSize( size );

  VectorImageType::Pointer dispacementfield = VectorImageType::New();
  dispacementfield->SetLargestPossibleRegion( region );
  dispacementfield->SetBufferedRegion( region );
  dispacementfield->Allocate();

  VectorType values;
  values[0]=0;
  values[1]=0;
  typedef itk::ImageRegionIteratorWithIndex<VectorImageType> Iterator;
  Iterator inIter( dispacementfield, region );
  for( ; !inIter.IsAtEnd(); ++inIter )
    {
    const unsigned int i=inIter.GetIndex()[0];
    const unsigned int j=inIter.GetIndex()[1];
    values[0]=0.125*i*i+0.125*j;
    values[1]=0.125*i*j+0.25*j;
    inIter.Set( values );
    //std::cout << "Setting: " << values << " at " << inIter.GetIndex() << std::endl;
    }

  //displacementfield:
  //|-------------------------------------------|
  //| [0.25;0.5]   | [0.375;0.75] | [0.75;1]    |
  //|-------------------------------------------|
  //| [0.125;0.25] | [0.25;0.375] | [0.625;0.5] |
  //|-------------------------------------------|
  //| [0.0;0.0]    | [0.125;0.0]  | [0.5;0]     |
  //|-------------------------------------------|
  //
  // centered difference. 
  //J(1,1) = [ (.625-.125)/2 (.5-.25)/2; (.375-.125)/2 (.75-0.0)/2] =[ .25  .125; .125 .375]
  //det((J+I)(1,1))=((.25+1.0)*(.375+1.0))-(.125*.125) = 1.703125;
  //const float KNOWN_ANSWER=(((.25+1.0)*(.375+1.0))-(.125*.125));
  // 
  // forward difference. 
  //J(1,1) = [ (.625-.25) (.5-.375); (.375-.25) (.75-.375)] =[ .375  .125; .125 .375]
  const float KNOWN_ANSWER=(((.375+1.0)*(.375+1.0))-(.125*.125));
  typedef itk::Image<float, ImageDimension> OutputImageType; 
  typedef itk::ForwardDifferenceDisplacementFieldJacobianDeterminantFilter<VectorImageType,float,OutputImageType> FilterType; 
  FilterType::Pointer filter = FilterType::New();

  filter->SetInput(dispacementfield);
  filter->Update();
  itk::Image<float,2>::Pointer output=filter->GetOutput();

  VectorImageType::IndexType index;
  index[0]=1;
  index[1]=1;
  std::cout << "Output="  << output->GetPixel(index) << std::endl;
  std::cout << "KNOWN_ANSWER=" << KNOWN_ANSWER << std::endl; 
  if(vcl_abs(output->GetPixel(index) - KNOWN_ANSWER) > 1e-13)
  {
    std::cout << "Test failed." << KNOWN_ANSWER << "!=" << output->GetPixel(index)  << std::endl;
    testPassed=false;
  }
  else
  {
    std::cout << "Test passed." << std::endl;
  }
  return testPassed;
}

int ForwardDifferenceDisplacementFieldJacobianDeterminantFilterTest(int , char * [] )
{
  bool ValueTestPassed = TestDisplacementJacobianDeterminantValue();
  
  if (ValueTestPassed == false)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

