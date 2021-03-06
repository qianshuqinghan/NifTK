/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <mitkImageReadAccessor.h>
#include <mitkImageCast.h>
#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>

namespace niftk
{

typedef itk::RGBPixel<unsigned char>  UCRGBPixelType;
typedef itk::RGBAPixel<unsigned char> UCRGBAPixelType;

//-----------------------------------------------------------------------------
// Note: Does not do pixel conversions (e.g. BGR to RGB).
template <typename ITKPixelType>
mitk::Image::Pointer CreateMitkImageInternal(const char* imageData,
                                             unsigned int numberOfChannels,
                                             unsigned int width,
                                             unsigned int widthStep,
                                             unsigned int height
                                            )
{
  typedef itk::Image<ITKPixelType, 3>             ItkImageType;
  typedef itk::ImportImageFilter<ITKPixelType, 3> ImportFilterType;

  // we do not do pixel type conversions!
  if (numberOfChannels != sizeof(ITKPixelType))
  {
    throw std::runtime_error("Source and target image type differ");
  }

  typename ImportFilterType::SizeType size;
  size.Fill(1);
  size[0] = width;
  size[1] = height;

  typename ImportFilterType::IndexType start;
  start.Fill( 0 );

  typename ImportFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );

  double origin[ 3 ];
  origin[0] = 0.0;    // X coordinate
  origin[1] = 0.0;    // Y coordinate
  origin[2] = 0.0;    // Z coordinate

  double spacing[ 3 ];
  spacing[0] = 1.0;    // along X direction
  spacing[1] = 1.0;    // along Y direction
  spacing[2] = 1.0;    // along Y direction

  const unsigned int numberOfPixels = size[0] * size[1] * size[2];

  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
  importFilter->SetRegion(region);
  importFilter->SetOrigin(origin);
  importFilter->SetSpacing(spacing);

  // This creates a new buffer, and copies this image into it.
  // Without this, if you use the OpenCV buffer directly, you can see
  // artefacts as the framebuffer is written to.
  const unsigned int numberOfBytes = numberOfPixels * sizeof(ITKPixelType);
  ITKPixelType* localBuffer = new ITKPixelType[numberOfPixels];

  // if the image pitch is the same as its width then everything is peachy
  // but if not we need to take care of that
  const unsigned int numberOfBytesPerLine = width * numberOfChannels;
  if (numberOfBytesPerLine == widthStep)
  {
    std::memcpy(localBuffer, imageData, numberOfBytes);
  }
  else
  {
    // if that is not true then something is seriously borked
    assert(widthStep >= static_cast<int>(numberOfBytesPerLine));

    // "slow" path: copy line by line
    for (int y = 0; y < height; ++y)
    {
      // widthStep is in bytes while width is in pixels
      std::memcpy(&(((char*) localBuffer)[y * numberOfBytesPerLine]), &(imageData[y * widthStep]), numberOfBytesPerLine);
    }
  }

  // This will tell the importFilter to use the supplied buffer,
  // and the output of this filter, references the same buffer.
  // We don't let the import filter take control of the buffer
  // because if you do, the importFilter destructor will destroy the
  // buffer when it goes out of scope.  We want the output image to
  // remain after the filter is out of scope.
  bool importFilterWillTakeControlOfBuffer = false;
  importFilter->SetImportPointer( localBuffer, numberOfPixels, importFilterWillTakeControlOfBuffer);
  importFilter->Update();

  // We then need a stand-alone ITK image, that survives after a pipeline.
  typename ItkImageType::Pointer itkOutput = importFilter->GetOutput();
  itkOutput->DisconnectPipeline();

  // We then convert it to MITK, and this conversion takes responsibility for the memory.
  mitk::Image::Pointer mitkImage;
  mitk::CastToMitkImage(itkOutput, mitkImage);

  // Delete local buffer, as even though the output object is disconnected, it does not manage data.
  delete [] localBuffer;

  return mitkImage;
}

} // namespace
