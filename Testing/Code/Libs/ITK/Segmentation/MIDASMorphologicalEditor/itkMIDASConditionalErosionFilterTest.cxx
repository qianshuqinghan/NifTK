/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-10-06 10:55:39 +0100 (Thu, 06 Oct 2011) $
 Revision          : $Revision: 7447 $
 Last modified by  : $Author: mjc $

 Original author   : a.duttaroy@cs.ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include <iostream>
#include <memory>
#include <math.h>
#include "itkImage.h"
#include "itkMIDASConditionalErosionFilter.h"
#include "itkImageRegionConstIterator.h"

/**
 * Basic tests for MIDASConditionalErosionFilterTest
 */
int itkMIDASConditionalErosionFilterTest(int argc, char * argv[])
{
  // Declare the types of the images
  const unsigned int Dimension = 2;
  typedef int PixelType;
  typedef itk::Image<PixelType, Dimension>                         ImageType;
  typedef itk::MIDASConditionalErosionFilter<ImageType, ImageType, ImageType> ConditionalErosionFilterType;

  // Create the first image.
  ImageType::Pointer inputImage  = ImageType::New();
  typedef ImageType::IndexType     IndexType;
  typedef ImageType::SizeType      SizeType;
  typedef ImageType::RegionType    RegionType;
 
  //Create a 5x5 image
  SizeType imageSize;
  imageSize[0] = 5;
  imageSize[1] = 5;

  IndexType startIndex;
  startIndex[0] = 0;
  startIndex[1] = 0;

  RegionType imageRegion;
  imageRegion.SetIndex(startIndex);
  imageRegion.SetSize(imageSize);
  
  inputImage->SetLargestPossibleRegion(imageRegion);
  inputImage->SetBufferedRegion(imageRegion);
  inputImage->SetRequestedRegion(imageRegion);
  inputImage->Allocate();

  //filling the rows 
  //rows
  for(unsigned int i = 0; i < 5; i++)
  {
    //columns
    for(unsigned int j = 0; j < 5; j++)
    {
      IndexType imageIndex;
      imageIndex[0] = i;
      imageIndex[1] = j;
      inputImage->SetPixel(imageIndex, j);
    }
  }

  
/****************************************************************/

  // Create the second image.
  ImageType::Pointer inputMask  = ImageType::New();

  //Create a 5x5 region
  SizeType regionSizeMask;
  regionSizeMask[0] = 5;
  regionSizeMask[1] = 5;

  IndexType startRegionIndexMask;
  startRegionIndexMask[0] = 0;
  startRegionIndexMask[1] = 0;

  RegionType regionToProcess;
  regionToProcess.SetIndex(startRegionIndexMask);
  regionToProcess.SetSize(regionSizeMask);

  inputMask->SetLargestPossibleRegion(regionToProcess);
  inputMask->SetBufferedRegion(regionToProcess);
  inputMask->SetRequestedRegion(regionToProcess);
  inputMask->Allocate();

  int intensityValue = 0;
  //filling the rows 
  //rows
  for(unsigned int i = 0; i < 5; i++)
  {
    //columns
    for(unsigned int j = 0; j < 5; j++)
    {
      if( ((i == 1) || (i == 2) || (i == 3))
         && ((j == 1) || (j == 2) || (j == 3)) )
      {
        intensityValue = 1;
      }
      else
      {
        intensityValue = 0;
      }

      IndexType imageIndex;
      imageIndex[0] = i;
      imageIndex[1] = j;
      inputMask->SetPixel(imageIndex, intensityValue);
    }
  }


  ConditionalErosionFilterType::Pointer ConditionalErosionFilter = ConditionalErosionFilterType::New();
  ConditionalErosionFilter->SetGreyScaleImageInput(inputImage);
  ConditionalErosionFilter->SetBinaryImageInput(inputMask);

  int numberOfErosions     = 2;
  PixelType upperThreshold = 4;

  ConditionalErosionFilter->SetUpperThreshold(upperThreshold);
  ConditionalErosionFilter->SetNumberOfIterations(numberOfErosions);
  ConditionalErosionFilter->Update();

  /**Check if the filter gives the correct output */
  ImageType::Pointer outputImagePtr  = ConditionalErosionFilter->GetOutput();
  typedef itk::ImageRegionConstIterator<ImageType> OutputImageIterator;
  OutputImageIterator outputImageIter(outputImagePtr, outputImagePtr->GetLargestPossibleRegion());

  std::vector<int> outValueVector;
  outValueVector.reserve(25);
  for(unsigned int i = 0; i < 25; i++)
  {
    outValueVector.push_back(0);
  }

  //TEST FOR DIFFERENT NUMBER OF EROSIONS
  if(numberOfErosions == 1)
  {
    outValueVector[12] = 1;
  }

  unsigned int index   = 0;
  bool bFilterStatus   = true;

  outputImageIter.GoToBegin();
  while(!outputImageIter.IsAtEnd())
  {
    if(outputImageIter.Get() != outValueVector[index])
    {
      bFilterStatus = false;
      break;
    }
    ++outputImageIter;
    ++index;
  }

  if(bFilterStatus)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;

  return EXIT_FAILURE;

}