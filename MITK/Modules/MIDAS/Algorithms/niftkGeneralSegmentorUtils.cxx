/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkGeneralSegmentorUtils.h"

#include <mitkImageAccessByItk.h>

namespace niftk
{

//-----------------------------------------------------------------------------
void GenerateOutlineFromBinaryImage(mitk::Image::ConstPointer image,
    int sliceAxis,
    int sliceIndex,
    int projectedSliceIndex,
    mitk::ContourModelSet* outputContourSet
    )
{
  try
  {
    AccessFixedTypeByItk_n(image,
        ITKGenerateOutlineFromBinaryImage,
        (unsigned char),
        (3),
        (sliceAxis,
         sliceIndex,
         projectedSliceIndex,
         outputContourSet
        )
      );
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "Failed in ITKGenerateOutlineFromBinaryImage due to:" << e.what();
    outputContourSet->Clear();
  }
  catch(const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Failed in ITKGenerateOutlineFromBinaryImage due to:" << e.what();
    outputContourSet->Clear();
  }
}

}
