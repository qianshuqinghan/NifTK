/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <niftkFileHelper.h>

#include <itkBreastMaskSegmentationFromMRI.h>
#include <itkHistogramMatchingImageFilter.h>
#include <itkScalarConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>
#include <itkImageMaskSpatialObject.h>
#include <itkForegroundFromBackgroundImageThresholdCalculator.h>
#include <itkWriteImage.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkThinPlateSplineScatteredDataPointSetToImageFilter.h>

#include <boost/filesystem.hpp>

namespace itk
{

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::BreastMaskSegmentationFromMRI() 
{
  flgVerbose = false;
  flgXML = false;
  flgSmooth = false;
  flgLeft = false;
  flgRight = false;
  flgExtInitialPect = false;

  flgRegGrowXcoord = false;
  flgRegGrowYcoord = false;
  flgRegGrowZcoord = false;

  flgCropWithFittedSurface = false;
  flgExcludeAxilla = false;

  regGrowXcoord = 0;
  regGrowYcoord = 0;
  regGrowZcoord = 0;

  maxIntensity = 0;
  minIntensity = 0;

  bgndThresholdProb = 0.;

  finalSegmThreshold = 0.45;

  sigmaInMM = 5;

  fMarchingK1   = 30.0;
  fMarchingK2   = 15.0;
  fMarchingTime = 5.0;
  
  sigmaBIF = 3.0;

  coilCropDistance = 10.0;
  cropDistPosteriorToMidSternum = 40.0;

  pecControlPointSpacing = 31.;

  imStructural = 0;
  imFatSat = 0;
  imBIFs = 0;

  imMaximum = 0;
  imMaxClosed = 0;
  imPectoralVoxels = 0;
  imPectoralSurfaceVoxels = 0;
  imChestSurfaceVoxels = 0;
  imSegmented = 0;

  imSkinElevationMap = 0;
};


// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::~BreastMaskSegmentationFromMRI()
{

};


// --------------------------------------------------------------------------
// Initialise()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::Initialise( void )
{
  niftkitkInfoMacro( << "Initialising the segmentation object.");
  
  // Must have an input image

  if ( ! imStructural )
  {
    itkExceptionMacro( << "ERROR: The input structural image is not set" );
  }


  typename InternalImageType::RegionType region;
  typename InternalImageType::SizeType size;

  region = imStructural->GetLargestPossibleRegion();
  size   = region.GetSize();

  if ( ! flgRegGrowXcoord ) regGrowXcoord = size[ 0 ]/2;
  if ( ! flgRegGrowYcoord ) regGrowYcoord = size[ 1 ]/4;
  if ( ! flgRegGrowZcoord ) regGrowZcoord = size[ 2 ]/2;

  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType  lateralSize;
  
  // Set the left region

  lateralSize    = size;
  lateralSize[0] = lateralSize[0]/2;

  lateralStart = region.GetIndex();

  m_LeftLateralRegion.SetIndex( lateralStart );
  m_LeftLateralRegion.SetSize(  lateralSize );


  // Set the right region

  lateralSize    = size;
  lateralSize[0] = lateralSize[0]/2;

  lateralStart    = region.GetIndex();
  lateralStart[0] += lateralSize[0];

  m_RightLateralRegion.SetIndex( lateralStart );
  m_RightLateralRegion.SetSize(  lateralSize );


  if ( ! imBIFs ) CreateBIFs();
};


// --------------------------------------------------------------------------
// CreateBIFs()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::CreateBIFs( void )
{
  typename BasicImageFeaturesFilterType::Pointer BIFsFilter = BasicImageFeaturesFilterType::New();
  
  BIFsFilter->SetEpsilon( 1.0e-05 );
  BIFsFilter->CalculateOrientatedBIFs();

  BIFsFilter->SetSigma( sigmaBIF );

  typename SliceBySliceImageFilterType::Pointer 
    sliceBySliceFilter = SliceBySliceImageFilterType::New();
  
  sliceBySliceFilter->SetFilter( BIFsFilter );
  sliceBySliceFilter->SetDimension( 2 );

  sliceBySliceFilter->SetInput( imStructural );

  std::cout << "Computing basic image features";
  sliceBySliceFilter->Update();
  
  imBIFs = sliceBySliceFilter->GetOutput();
  imBIFs->DisconnectPipeline();  
  
  WriteImageToFile( fileOutputBIFs, "Basic image features image", imBIFs, flgLeft, flgRight );
  
};


// --------------------------------------------------------------------------
// Smooth the structural and FatSat images
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::SmoothTheInputImages( void )
{
  if ( ! flgSmooth ) 
  {
    // If no smoothing is to be performed, then the input of the speed function will also not be smoothed!
    typename DuplicatorType::Pointer duplicator = DuplicatorType::New();

    duplicator->SetInputImage(imStructural);
    duplicator->Update();

    imSpeedFuncInputImage = duplicator->GetOutput();
    imSpeedFuncInputImage->DisconnectPipeline();

    return;
  }


  typename SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
    
  smoothing->SetTimeStep( 0.046 );
  smoothing->SetNumberOfIterations(  5 );
  smoothing->SetConductanceParameter( 3.0 );
    
  smoothing->SetInput( imStructural );
    
  std::cout << "Smoothing the structural image" << std::endl;
  smoothing->Update(); 
    
  imTmp = smoothing->GetOutput();
  imTmp->DisconnectPipeline();
    
  imStructural = imTmp;

  // Keep a copy of the smoothed structural image for the region growing of the pectoral muscle 
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();

  duplicator->SetInputImage(imStructural);
  duplicator->Update();

  imSpeedFuncInputImage = duplicator->GetOutput();
  imSpeedFuncInputImage->DisconnectPipeline();

  WriteImageToFile( fileOutputSmoothedStructural, "smoothed structural image", 
		    imStructural, flgLeft, flgRight );
  
  if ( imFatSat ) 
  {
    smoothing->SetInput( imFatSat );

    std::cout << "Smoothing the Fat-Sat image" << std::endl;
    smoothing->Update(); 
    
    imTmp = smoothing->GetOutput();
    imTmp->DisconnectPipeline();
    
    imFatSat = imTmp;
    
    WriteImageToFile( fileOutputSmoothedFatSat, "smoothed FatSat image", 
		      imFatSat, flgLeft, flgRight );      
  }
  
};


// --------------------------------------------------------------------------
// ScanLineMaxima()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::Pointer
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::ScanLineMaxima( typename InternalImageType::Pointer inImage,
                  typename InternalImageType::RegionType region,
                  unsigned int direction, bool flgForward )
{
  InputPixelType maxVoxel;

  typename InternalImageType::SizeType size;
  typename InternalImageType::IndexType start;

  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();

  duplicator->SetInputImage( inImage );

  try
  {
    duplicator->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    exit( EXIT_FAILURE );
  }

  typename InternalImageType::Pointer outImage = duplicator->GetOutput();
  outImage->DisconnectPipeline();
 
  start  = region.GetIndex();
  size   = region.GetSize();

  
  LineIteratorType itImage( outImage, region );

  itImage.SetDirection( direction );

  if ( flgForward )
  {
    for ( itImage.GoToBegin(); 
          ! itImage.IsAtEnd(); 
          itImage.NextLine() )
    {
      itImage.GoToBeginOfLine();
      
      maxVoxel = itImage.Get();
      
      while ( ! itImage.IsAtEndOfLine() )
      {
        if ( itImage.Get() > maxVoxel )
        {
          maxVoxel = itImage.Get();
        }
        
        itImage.Set( maxVoxel );
        
        ++itImage;
      }
    }
  }

  else
  {
    for ( itImage.GoToBegin(); 
          ! itImage.IsAtEnd(); 
          itImage.NextLine() )
    {
      itImage.GoToReverseBeginOfLine();
      
      maxVoxel = itImage.Get();
      
      while ( ! itImage.IsAtReverseEndOfLine() )
      {
        if ( itImage.Get() > maxVoxel )
        {
          maxVoxel = itImage.Get();
        }
        
        itImage.Set( maxVoxel );
        
        --itImage;
      }
    }
  }

  return outImage;
};


// --------------------------------------------------------------------------
// GreyScaleCloseImage(typename InternalImageType::Pointer, unsigned int direction)
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::Pointer
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::GreyScaleCloseImage( typename InternalImageType::Pointer inImage,
                       typename InternalImageType::RegionType region,
                       unsigned int direction,
                       const std::string label )
{
  typename InternalImageType::Pointer imScanLineMaxima[ 2 ];

  imScanLineMaxima[0] = ScanLineMaxima( inImage, region, direction, true );
  imScanLineMaxima[1] = ScanLineMaxima( inImage, region, direction, false );

  if ( fileOutputClosedStructural.length() )
  {
    std::string fileStructuralClose;

    fileStructuralClose = ModifySuffix( fileOutputClosedStructural, 
                                        std::string( "_Forward_" ) + label );

    WriteImageToFile( fileStructuralClose, 
                      "forward scan line maximum", 
                      imScanLineMaxima[0], flgLeft, flgRight );      

    fileStructuralClose = ModifySuffix( fileOutputClosedStructural, 
                                        std::string( "_Reverse_" ) + label );

    WriteImageToFile( fileStructuralClose, 
                      "reverse scan line maximum",
                      imScanLineMaxima[1], flgLeft, flgRight );      
  }

  // Compute the voxel-wise minimum intensities
            
  IteratorType fwdIterator( imScanLineMaxima[0],
                            imScanLineMaxima[0]->GetLargestPossibleRegion() );

  IteratorType revIterator( imScanLineMaxima[1],
                            imScanLineMaxima[1]->GetLargestPossibleRegion() );
        
  for ( fwdIterator.GoToBegin(), revIterator.GoToBegin(); 
        ! fwdIterator.IsAtEnd();
        ++fwdIterator, ++revIterator )
  {
    if ( fwdIterator.Get() > revIterator.Get() )
      fwdIterator.Set( revIterator.Get() );
  }
  
  return imScanLineMaxima[0];
};


// --------------------------------------------------------------------------
// GreyScaleClosing(typename InternalImageType::Pointer)
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::Pointer
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::GreyScaleCloseImage( typename InternalImageType::Pointer inImage,
                       typename InternalImageType::RegionType region,
                       const char *strSide )
{
  typename InternalImageType::Pointer inImageCloseInX;
  typename InternalImageType::Pointer inImageCloseInY;
  typename InternalImageType::Pointer inImageCloseInZ;

  inImageCloseInX = GreyScaleCloseImage( inImage, region, 0, std::string( strSide ) + "_InX_" );
  inImageCloseInY = GreyScaleCloseImage( inImage, region, 1, std::string( strSide ) + "_InY_" );
  inImageCloseInZ = GreyScaleCloseImage( inImage, region, 2, std::string( strSide ) + "_InZ_" );

  if ( fileOutputClosedStructural.length() )
  {
    std::string fileStructuralClose;

    fileStructuralClose = ModifySuffix( fileOutputClosedStructural, 
                                        std::string( "_InX_" ) + strSide );

    WriteImageToFile( fileStructuralClose, 
                      "structural image closed in X", 
                      inImageCloseInX, flgLeft, flgRight );      

    fileStructuralClose = ModifySuffix( fileOutputClosedStructural, 
                                        std::string( "_InY_" ) + strSide );

    WriteImageToFile( fileStructuralClose, 
                      "structural image closed in Y", 
                      inImageCloseInY, flgLeft, flgRight );      

    fileStructuralClose = ModifySuffix( fileOutputClosedStructural, 
                                        std::string( "_InZ_" ) + strSide );

    WriteImageToFile( fileStructuralClose, 
                      "structural image closed in Z", 
                      inImageCloseInZ, flgLeft, flgRight );      
  }

  InputPixelType xVoxel, yVoxel, zVoxel;
            
  IteratorType xIterator( inImageCloseInX,
                          inImageCloseInX->GetLargestPossibleRegion() );

  IteratorType yIterator( inImageCloseInY,
                          inImageCloseInY->GetLargestPossibleRegion() );
        
  IteratorType zIterator( inImageCloseInZ,
                          inImageCloseInZ->GetLargestPossibleRegion() );
        
  for ( xIterator.GoToBegin(), yIterator.GoToBegin(), zIterator.GoToBegin(); 
        ! xIterator.IsAtEnd();
        ++xIterator, ++yIterator, ++zIterator )
  {
    xVoxel = xIterator.Get();
    yVoxel = yIterator.Get();
    zVoxel = zIterator.Get();

    
#if 0
    // Calculate the median

    if ( ( ( xVoxel < yVoxel ) && ( yVoxel <= zVoxel ) ) ||
         ( ( zVoxel <= yVoxel ) && ( yVoxel < xVoxel ) ) )
    {
      xIterator.Set( yVoxel );
    }
    else if ( ( ( xVoxel < zVoxel ) && ( zVoxel <= yVoxel ) ) ||
              ( ( yVoxel <= zVoxel ) && ( zVoxel < xVoxel ) ) )
    {
      xIterator.Set( zVoxel );
    }
#elif 1
    // Calculate the minimum

    if ( ( yVoxel < xVoxel ) && ( yVoxel <= zVoxel ) )
    {
      xIterator.Set( yVoxel );
    }
    else if ( ( zVoxel <= yVoxel ) && ( zVoxel < xVoxel ) )
    {
      xIterator.Set( zVoxel );
    }
#else
    // Calculate the mean of the two lower values

    if ( ( xVoxel <= yVoxel ) && ( zVoxel <= yVoxel ) )
    {
      xIterator.Set( (xVoxel + zVoxel)/2 );
    }
    else if ( ( xVoxel <= zVoxel ) && ( yVoxel <= zVoxel ) )
    {
      xIterator.Set( (xVoxel + yVoxel)/2 );
    }
    else
    {
      xIterator.Set( (yVoxel + zVoxel)/2 );
    }
#endif
  }
 
  return inImageCloseInX;
};


// --------------------------------------------------------------------------
// GreyScaleClosing()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::GreyScaleClosing( void )
{
  imStructural = GreyScaleCloseImage( imStructural, m_LeftLateralRegion, "left" );
  imStructural = GreyScaleCloseImage( imStructural, m_RightLateralRegion, "right" );

  WriteImageToFile( fileOutputClosedStructural, 
                    "structural image closed", 
                    imStructural, flgLeft, flgRight );      

#if 0
  if ( imFatSat ) 
  {
    imFatSat = GreyScaleCloseImage( imFatSat );

    std::string fileStructuralClose( "imFatSatClose.nii" );
    WriteImageToFile( fileStructuralClose, "Fat-Sat image closed", 
                      imFatSat, flgLeft, flgRight );      
  }
#endif
};


// --------------------------------------------------------------------------
// CalculateTheMaximumImage()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::CalculateTheMaximumImage( void )
{
  if ( imFatSat ) 
  {

#if 1
    // Histogram match the images

    typedef itk::HistogramMatchingImageFilter< InternalImageType, InternalImageType,
                                               InputPixelType > HistogramMatchingImageFilterType;

    typename HistogramMatchingImageFilterType::Pointer 
      histMatcher = HistogramMatchingImageFilterType::New();

    histMatcher->SetInput( imFatSat );
    histMatcher->SetReferenceImage( imStructural );
    histMatcher->ThresholdAtMeanIntensityOn();
    
    try
    {
      std::cout << "Histogram matching the structural and FatSat images" << std::endl;
      histMatcher->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
      exit( EXIT_FAILURE );
    }
    
    imMaximum = histMatcher->GetOutput();

    // Compute the voxel-wise maximum intensities
            
    IteratorType inputIterator( imStructural, imStructural->GetLargestPossibleRegion() );
    IteratorType outputIterator( imMaximum, imMaximum->GetLargestPossibleRegion() );
        
    for ( inputIterator.GoToBegin(), outputIterator.GoToBegin(); 
	 ! inputIterator.IsAtEnd();
	 ++inputIterator, ++outputIterator )
    {
      if ( inputIterator.Get() > outputIterator.Get() )
	outputIterator.Set( inputIterator.Get() );
    }
#else    

    // Copy the structural image into the maximum image 

    typename DuplicatorType::Pointer duplicator = DuplicatorType::New();

    duplicator->SetInputImage( imStructural );
    duplicator->Update();

    imMaximum = duplicator->GetOutput();

    // Compute the voxel-wise maximum intensities
            
    IteratorType inputIterator( imFatSat, imFatSat->GetLargestPossibleRegion() );
    IteratorType outputIterator( imMaximum, imMaximum->GetLargestPossibleRegion() );
        
    for ( inputIterator.GoToBegin(), outputIterator.GoToBegin(); 
	 ! inputIterator.IsAtEnd();
	 ++inputIterator, ++outputIterator )
    {
      if ( inputIterator.Get() > outputIterator.Get() )
	outputIterator.Set( inputIterator.Get() );
    }
#endif
  }
  else
  {
    imMaximum = imStructural;
  }

  WriteImageToFile( fileOutputMaxImage, "maximum image", imMaximum, flgLeft, flgRight );


  imMaxClosed = GreyScaleCloseImage( imMaximum,   m_LeftLateralRegion,  "MaxLeft" );
  imMaxClosed = GreyScaleCloseImage( imMaxClosed, m_RightLateralRegion, "MaxRight" );     
  
  // Smooth the image to increase separation of the background

#if 0

  typedef itk::CurvatureFlowImageFilter< InternalImageType, 
					 InternalImageType > CurvatureFlowImageFilterType;

  typename CurvatureFlowImageFilterType::Pointer preRegionGrowingSmoothing = 
    CurvatureFlowImageFilterType::New();

  preRegionGrowingSmoothing->SetInput( imMaxClosed );

  unsigned int nItersPreRegionGrowingSmoothing = 5;
  double timeStepPreRegionGrowingSmoothing = 0.125;

  preRegionGrowingSmoothing->SetNumberOfIterations( nItersPreRegionGrowingSmoothing );
  preRegionGrowingSmoothing->SetTimeStep( 0.125 );

  std::cout << "Applying pre-region-growing smoothing, no. iters: "
	    << nItersPreRegionGrowingSmoothing
	    << ", time step: " << timeStepPreRegionGrowingSmoothing << std::endl;
  preRegionGrowingSmoothing->Update();

  imMaxClosed = preRegionGrowingSmoothing->GetOutput();
  imMaxClosed->DisconnectPipeline();

#endif


  // Ensure the maximum image contains only positive intensities

  IteratorType imIterator( imMaxClosed, imMaxClosed->GetLargestPossibleRegion() );
        
  for ( imIterator.GoToBegin(); ! imIterator.IsAtEnd(); ++imIterator )
  {
    if ( imIterator.Get() < 0 )
      imIterator.Set( 0 );
  }


  // Compute the range of the maximum closed image

  typedef itk::MinimumMaximumImageCalculator< InternalImageType > MinimumMaximumImageCalculatorType;

  typename MinimumMaximumImageCalculatorType::Pointer 
    rangeCalculator = MinimumMaximumImageCalculatorType::New();

  rangeCalculator->SetImage( imMaxClosed );
  rangeCalculator->Compute();

  maxIntensity = rangeCalculator->GetMaximum();
  minIntensity = rangeCalculator->GetMinimum();
  
  if (minIntensity < 1.0f)
  {
    minIntensity = 1.0f;
  }

  if ( flgVerbose ) 
  {
    std::cout << "Maximum image intensity range: " 
	      << niftk::ConvertToString( minIntensity ).c_str() << " to "
	      << niftk::ConvertToString( maxIntensity ).c_str() << std::endl;
  }

  WriteImageToFile( fileOutputMaxClosedImage, "maximum closed image", 
                    imMaxClosed, flgLeft, flgRight );
};


// --------------------------------------------------------------------------
// Segment the backgound using itkForegroundFromBackgroundImageThresholdCalculator
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::SegmentForegroundFromBackground( void )
{
  typedef unsigned int LabelPixelType;
  typedef itk::Image< LabelPixelType, ImageDimension> LabelImageType;


  // Find threshold , t, that maximises:
  // ( MaxIntensity - t )*( CDF( t ) - Variance( t )/Max_Variance ) 
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typedef itk::ForegroundFromBackgroundImageThresholdCalculator< InternalImageType > ThresholdCalculatorType;

  typename ThresholdCalculatorType::Pointer 
    thresholdCalculator = ThresholdCalculatorType::New();

  thresholdCalculator->SetImage( imMaxClosed );

  thresholdCalculator->SetVerbose( flgVerbose );

  thresholdCalculator->Compute();

  double intThreshold = thresholdCalculator->GetThreshold();

  if ( flgVerbose )
  {
    std::cout << "Threshold: " << intThreshold << std::endl;
  }


  // Threshold the image
  // ~~~~~~~~~~~~~~~~~~~

  typedef typename itk::BinaryThresholdImageFilter< InternalImageType, LabelImageType > BinaryThresholdFilterType;

  typename BinaryThresholdFilterType::Pointer thresholder = BinaryThresholdFilterType::New();

  thresholder->SetInput( imMaxClosed );

  thresholder->SetOutsideValue( 0 );
  thresholder->SetInsideValue( 1000 );

  thresholder->SetLowerThreshold( intThreshold );

  std::cout << "Thresholding the image" << std::endl;
  thresholder->Update();
  
  typename LabelImageType::Pointer mask = thresholder->GetOutput();
  mask->DisconnectPipeline();

  imMaxClosed = 0;

  if ( 0 )
  {
    typedef itk::ImageFileWriter< LabelImageType > FileWriterType;
    
    typename FileWriterType::Pointer writer = FileWriterType::New();
    
    writer->SetFileName( "ThresholdedImage.nii.gz" );
    writer->SetInput( mask );

    std::cout << "Writing thresholded image to file: ThresholdedImage.nii.gz" << std::endl;

    writer->Update();
  }


  // Label the image
  // ~~~~~~~~~~~~~~~

  LabelPixelType distanceThreshold = 0;
 
  typedef typename itk::ScalarConnectedComponentImageFilter<LabelImageType, LabelImageType, LabelImageType >
    ConnectedComponentImageFilterType;
  
  typename ConnectedComponentImageFilterType::Pointer connected =
    ConnectedComponentImageFilterType::New ();
  
  connected->SetInput( mask );
  connected->SetMaskImage( mask );
  connected->SetDistanceThreshold( distanceThreshold );
  
  typedef itk::RelabelComponentImageFilter< LabelImageType, LabelImageType >
    RelabelFilterType;

  typename RelabelFilterType::Pointer relabel = RelabelFilterType::New();
  typename RelabelFilterType::ObjectSizeType minSize;
  
  unsigned int i;

  relabel->SetMinimumObjectSize( 500 );

  relabel->SetInput( connected->GetOutput() );

  std::cout << "Computing connected labels" << std::endl;
  relabel->Update();

  std::cout << "Number of connected objects: " << relabel->GetNumberOfObjects() 
            << std::endl
            << "Size of smallest object: " 
            << relabel->GetSizeOfObjectsInPixels()[ relabel->GetNumberOfObjects() - 1 ] 
            << std::endl
            << "Size of largest object: " << relabel->GetSizeOfObjectsInPixels()[0] 
            << std::endl << std::endl;

  mask = relabel->GetOutput();
  mask->DisconnectPipeline();

  if ( 0 ) 
  {
    typedef itk::ImageFileWriter< LabelImageType > FileWriterType;

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( "LabelledImage.nii.gz" );
    writer->SetInput( mask );

    std::cout << "Writing labelled image to file: LabelledImage.nii.gz" << std::endl;

    writer->Update();
  }


  // Only keep the largest object

  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();

  duplicator->SetInputImage( imStructural );
  duplicator->Update();

  imSegmented = duplicator->GetModifiableOutput();

  if ( 0 ) 
  {
    typedef itk::ImageFileWriter< InternalImageType > FileWriterType;

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( "ClonedImage.nii.gz" );
    writer->SetInput( imSegmented );

    std::cout << "Writing labelled image to file: ClonedImage.nii.gz" << std::endl;

    writer->Update();
  }

  typedef itk::ImageRegionIterator< LabelImageType > LabelIteratorType;
  
  LabelIteratorType itMask( mask, mask->GetLargestPossibleRegion() );
  IteratorType itImage( imSegmented, imSegmented->GetLargestPossibleRegion() );


  itMask.GoToBegin();
  itImage.GoToBegin();

  while (! itMask.IsAtEnd() ) 
  {
    if ( ( itMask.Get() == 1 ) )
    {
      itImage.Set( 1000.);
    }
    else
    {
      itImage.Set( 0. );
    }
    
    ++itMask;
    ++itImage;
  }

  // Write the background mask to a file?
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  WriteBinaryImageToUCharFile( fileOutputBackground, "background image", 
                               imSegmented, flgLeft, flgRight );

};


// --------------------------------------------------------------------------
// Segment the backgound using the maximum image histogram
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::SegmentBackground( void )
{
  float bgndThreshold = 0.;


  // Compute the histograms of the images
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typedef itk::Statistics::ScalarImageToHistogramGenerator< InternalImageType > HistogramGeneratorType;

  typename HistogramGeneratorType::Pointer histGeneratorT2 = HistogramGeneratorType::New();
  typename HistogramGeneratorType::Pointer histGeneratorFS = 0;

  typedef typename HistogramGeneratorType::HistogramType  HistogramType;

  const HistogramType *histogramT2 = 0;
  const HistogramType *histogramFS = 0;

  unsigned int nBins = (unsigned int) (maxIntensity - minIntensity + 1);

  if ( flgVerbose ) 
    std::cout << "Number of histogram bins: " << nBins << std::endl;

  histGeneratorT2->SetHistogramMin( minIntensity );
  histGeneratorT2->SetHistogramMax( maxIntensity );

  histGeneratorT2->SetNumberOfBins( nBins );
  histGeneratorT2->SetMarginalScale( 10. );

  histGeneratorT2->SetInput( imStructural );
  histGeneratorT2->Compute();

  histogramT2 = histGeneratorT2->GetOutput();
    
  if ( imFatSat ) {
    histGeneratorFS = HistogramGeneratorType::New();

    histGeneratorFS->SetHistogramMin( minIntensity );
    histGeneratorFS->SetHistogramMax( maxIntensity );

    histGeneratorFS->SetNumberOfBins( nBins );
    histGeneratorFS->SetMarginalScale( 10. );

    histGeneratorFS->SetInput( imFatSat );
    histGeneratorFS->Compute();

    histogramFS = histGeneratorFS->GetOutput();    
  }

  // Find the mode of the histogram

  typename HistogramType::ConstIterator itrT2 = histogramT2->Begin();
  typename HistogramType::ConstIterator endT2 = histogramT2->End();

  typename HistogramType::ConstIterator itrFS = 0;

  if ( histogramFS ) itrFS = histogramFS->Begin();

  unsigned int iBin;
  unsigned int modeBin = 0;

  float modeValue = 0.;
  float modeFrequency = 0.;

  float freqT2 = 0.;
  float freqFS = 0.;
  float freqCombined = 0.;

  float nSamples = histogramT2->GetTotalFrequency();

  vnl_vector< double > xHistIntensity( nBins, 0. );
  vnl_vector< double > yHistFrequency( nBins, 0. );

  if ( histogramFS ) nSamples += histogramFS->GetTotalFrequency();

  if ( nSamples == 0. ) {
    itkExceptionMacro( << "ERROR: Total number of samples is zero" );
  }

  if ( flgVerbose ) 
    std::cout << "Total number of voxels is: " << nSamples << std::endl;

  iBin = 0;

  while ( itrT2 != endT2 )
  {
    freqT2 = itrT2.GetFrequency();

    if ( histogramFS )
      freqFS = itrFS.GetFrequency();

    freqCombined = (freqT2 + freqFS)/nSamples;
    
    xHistIntensity[ iBin ] = histogramT2->GetMeasurement(iBin, 0);
    yHistFrequency[ iBin ] = freqCombined;

    if ( freqCombined > modeValue ) {
      modeBin = iBin;
      modeFrequency = histogramT2->GetMeasurement(modeBin, 0);
      modeValue = freqCombined;
    }
      
    ++iBin;
    ++itrT2;
    if ( histogramFS ) ++itrFS;
  }    

  if ( flgVerbose ) 
    std::cout << "Histogram mode = " << modeValue 
	      << " at intensity: " << modeFrequency << std::endl;
 
  // Only use values above the mode for the fit

  unsigned int nBinsForFit = nBins - modeBin;

  vnl_vector< double > xHistIntensityForFit( nBinsForFit, 0. );
  vnl_vector< double > yHistFrequencyForFit( nBinsForFit, 0. );

  for ( iBin=0; iBin<nBinsForFit; iBin++ ) {
    xHistIntensityForFit[ iBin ] = xHistIntensity[ iBin + modeBin ];
    yHistFrequencyForFit[ iBin ] = yHistFrequency[ iBin + modeBin ];
  }

  // Write the histogram to a text file

  if (fileOutputCombinedHistogram.length()) 
    WriteHistogramToFile( fileOutputCombinedHistogram,
			  xHistIntensity, yHistFrequency, nBins );


  // Fit a Rayleigh distribution to the lower 25% of the histogram
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  //RayleighFunction fitFunc( nBins/4, xHistIntensity, yHistFrequency, true );
  ExponentialDecayFunction fitFunc( nBinsForFit/4, 
				    xHistIntensityForFit, 
				    yHistFrequencyForFit, true );

  vnl_levenberg_marquardt lmOptimiser( fitFunc );

  vnl_double_2 aInitial( 1., 1. );
  vnl_vector<double> aFit = aInitial.as_vector();

  if ( fitFunc.has_gradient() )
    lmOptimiser.minimize_using_gradient( aFit );
  else
    lmOptimiser.minimize_without_gradient( aFit );

  lmOptimiser.diagnose_outcome( std::cout );

  vnl_vector< double > yHistFreqFit( nBinsForFit, 0. );

  for ( iBin=0; iBin<nBinsForFit; iBin++ ) 
    yHistFreqFit[ iBin ] = 
      fitFunc.compute( xHistIntensityForFit[ iBin ], aFit[0], aFit[1] );

  // Write the fit to a file

  if ( fileOutputRayleigh.length() )
    WriteHistogramToFile( fileOutputRayleigh,
			  xHistIntensityForFit, yHistFreqFit, nBinsForFit );


  // Subtract the fit from the histogram and calculate the cummulative
  // distribution of the remaining (non-background?) intensities
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  vnl_vector< double > yFreqLessBgndCDF( nBinsForFit, 0. );

  double totalFrequency = 0.;

  for ( iBin=0; iBin<nBinsForFit; iBin++ ) 
  {
    yFreqLessBgndCDF[ iBin ] = yHistFrequency[ iBin ] - 
      fitFunc.compute( xHistIntensity[ iBin ], aFit[0], aFit[1] );

    if ( yFreqLessBgndCDF[ iBin ] < 0. ) 
      yFreqLessBgndCDF[ iBin ] = 0.;
    
    totalFrequency += yFreqLessBgndCDF[ iBin ];

    yFreqLessBgndCDF[ iBin ] = totalFrequency;
  }

  for ( iBin=0; iBin<nBinsForFit; iBin++ ) 
  {
    yFreqLessBgndCDF[ iBin ] /= totalFrequency;
    
    if ( yFreqLessBgndCDF[ iBin ] < bgndThresholdProb )
      bgndThreshold = xHistIntensity[ iBin ];
  }

  if ( flgVerbose )
    std::cout << "Background region growing threshold is: " 
	      << bgndThreshold << std::endl;

  // Write this CDF to a file

  if ( fileOutputFreqLessBgndCDF.length() )
    WriteHistogramToFile( fileOutputFreqLessBgndCDF,
			  xHistIntensity, yFreqLessBgndCDF, nBinsForFit );


  // Region grow the background
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~

  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

  connectedThreshold->SetInput( imMaxClosed );

  connectedThreshold->SetLower( 0  );
  connectedThreshold->SetUpper( bgndThreshold );

  connectedThreshold->SetReplaceValue( 1000 );

  typename InternalImageType::IndexType  index;
  
  index[0] = regGrowXcoord;
  index[1] = regGrowYcoord;
  index[2] = regGrowZcoord;

  connectedThreshold->SetSeed( index );

  std::cout << "Region-growing the image background between: 0 and "
	    << bgndThreshold << std::endl;
  connectedThreshold->Update();
  
  imSegmented = connectedThreshold->GetOutput();
  imSegmented->DisconnectPipeline();

  connectedThreshold = 0;
  imMaxClosed = 0;


  // Invert the segmentation
  // ~~~~~~~~~~~~~~~~~~~~~~~

  IteratorType segIterator( imSegmented, imSegmented->GetLargestPossibleRegion() );
        
  for ( segIterator.GoToBegin(); ! segIterator.IsAtEnd(); ++segIterator )
  {
    if ( segIterator.Get() )
      segIterator.Set( 0 );
    else
      segIterator.Set( 1000 );
  }
}


// --------------------------------------------------------------------------
// Find a point in the surface offset from the nipple
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::IndexType 
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::FindSurfacePoint( typename InternalImageType::IndexType idxNipple,
                    float deltaXinMM, float deltaZinMM )
{
  typename InternalImageType::IndexType idx;
  typename InternalImageType::IndexType idxDelta;

  typename InternalImageType::SpacingType spacing3D;
  spacing3D = imSegmented->GetSpacing();

  idxDelta[0] = static_cast<typename InternalImageType::IndexValueType>( deltaXinMM / spacing3D[0] );
  idxDelta[1] = 0;
  idxDelta[2] = static_cast<typename InternalImageType::IndexValueType>( deltaZinMM / spacing3D[2] );

  typename InternalImageType::RegionType region;
  typename InternalImageType::SizeType size;
  typename InternalImageType::IndexType start;

  region = imSegmented->GetLargestPossibleRegion();
  size = region.GetSize();

  start[0] = idxNipple[0] + idxDelta[0];
  start[1] = idxNipple[1] + idxDelta[1];
  start[2] = idxNipple[2] + idxDelta[2];

  region.SetIndex( start );

  size[0] = 1;
  size[1] = size[1] - start[1];
  size[2] = 1;

  region.SetSize( size );

  if (flgVerbose) 
    std::cout << "Searching for surface point from: " << start << std::endl;

  LineIteratorType itSegLinear( imSegmented, region );
  itSegLinear.SetDirection( 1 );

  while ( ! itSegLinear.IsAtEndOfLine() )
  {
    idx = itSegLinear.GetIndex();

    if ( itSegLinear.Get() ) 
    {
      break;
    }
    ++itSegLinear;
  }

  if (flgVerbose) 
    std::cout << "   surface point found: " << idx << std::endl;
 

  return idx;
}


// --------------------------------------------------------------------------
// Find the nipple and mid-sternum landmarks
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::FindBreastLandmarks( void )
{

  // Find the nipple locations
  // ~~~~~~~~~~~~~~~~~~~~~~~~~

  bool flgFoundNippleSlice;
  int nVoxels;

  typename InternalImageType::IndexType idx;
        
  typename InternalImageType::RegionType lateralRegion;
  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType lateralSize;

  // Start iterating over the left-hand side

  lateralRegion = m_LeftLateralRegion;

  lateralStart = lateralRegion.GetIndex();  
  lateralSize = lateralRegion.GetSize();

  if ( flgVerbose )
    std::cout << "Iterating over left region: " << lateralRegion << std::endl;

  SliceIteratorType itSegLeftRegion( imSegmented, lateralRegion );
  itSegLeftRegion.SetFirstDirection( 0 );
  itSegLeftRegion.SetSecondDirection( 2 );

  itSegLeftRegion.GoToBegin();

  flgFoundNippleSlice = false;
  while ( ( ! flgFoundNippleSlice ) 
	  && ( ! itSegLeftRegion.IsAtEnd() ) )
  {
    while ( ( ! flgFoundNippleSlice ) 
	    && ( ! itSegLeftRegion.IsAtEndOfSlice() )  )
    {
      while ( ( ! flgFoundNippleSlice ) 
	      && ( ! itSegLeftRegion.IsAtEndOfLine() ) )
      {
	if ( itSegLeftRegion.Get() ) {
	  idx = itSegLeftRegion.GetIndex();
	  flgFoundNippleSlice = true;
	}
	++itSegLeftRegion; 
      }
      itSegLeftRegion.NextLine();
    }
    itSegLeftRegion.NextSlice(); 
  }

  if ( ! flgFoundNippleSlice ) 
  {
    itkExceptionMacro( << "ERROR: Could not find left nipple slice" );
  }

  if ( flgVerbose )
    std::cout << "Left nipple is in slice: " << idx << std::endl;

  // Found the slice, now iterate within the slice to find the center of mass

  lateralSize[1] = 1;
  lateralStart[1] = idx[1];

  lateralRegion.SetSize( lateralSize );
  lateralRegion.SetIndex( lateralStart );

  IteratorType itSegLeftNippleSlice( imSegmented, lateralRegion );

  nVoxels = 0;

  idxNippleLeft[0] = 0;
  idxNippleLeft[1] = 0;
  idxNippleLeft[2] = 0;

  for ( itSegLeftNippleSlice.GoToBegin(); 
	! itSegLeftNippleSlice.IsAtEnd(); 
	++itSegLeftNippleSlice )
  {
    if ( itSegLeftNippleSlice.Get() ) {
      idx = itSegLeftNippleSlice.GetIndex();

      idxNippleLeft[0] += idx[0];
      idxNippleLeft[1] += idx[1];
      idxNippleLeft[2] += idx[2];

      nVoxels++;
    }
  }

  if ( ! nVoxels ) 
  {
    itkExceptionMacro( << "ERROR: Could not find the left nipple" );
  }

  idxNippleLeft[0] /= nVoxels;
  idxNippleLeft[1] /= nVoxels;
  idxNippleLeft[2] /= nVoxels;

  if (flgVerbose) 
    std::cout << "Left nipple location: " << idxNippleLeft << std::endl;
    

  // Then iterate over the right side

  lateralRegion = m_RightLateralRegion;

  lateralStart = lateralRegion.GetIndex();  
  lateralSize = lateralRegion.GetSize();

  if ( flgVerbose )
    std::cout << "Iterating over right region: " << lateralRegion << std::endl;

  SliceIteratorType itSegRightRegion( imSegmented, lateralRegion );
  itSegRightRegion.SetFirstDirection( 0 );
  itSegRightRegion.SetSecondDirection( 2 );

  itSegRightRegion.GoToBegin();

  flgFoundNippleSlice = false;
  while ( ( ! flgFoundNippleSlice ) 
	  && ( ! itSegRightRegion.IsAtEnd() ) )
  {
    while ( ( ! flgFoundNippleSlice ) 
	    && ( ! itSegRightRegion.IsAtEndOfSlice() )  )
    {
      while ( ( ! flgFoundNippleSlice ) 
	      && ( ! itSegRightRegion.IsAtEndOfLine() ) )
      {
	if ( itSegRightRegion.Get() ) {
	  idx = itSegRightRegion.GetIndex();
	  flgFoundNippleSlice = true;
	}
	++itSegRightRegion; 
      }
      itSegRightRegion.NextLine();
    }
    itSegRightRegion.NextSlice(); 
  }

  if ( ! flgFoundNippleSlice ) 
  {
    itkExceptionMacro( << "ERROR: Could not find right nipple slice" );
  }

  if ( flgVerbose )
    std::cout << "Right nipple is in slice: " << idx << std::endl;

  // Found the slice, now iterate within the slice to find the center of mass

  lateralSize[1] = 1;
  lateralStart[1] = idx[1];

  lateralRegion.SetSize( lateralSize );
  lateralRegion.SetIndex( lateralStart );

  IteratorType itSegRightNippleSlice( imSegmented, lateralRegion );

  nVoxels = 0;

  idxNippleRight[0] = 0;
  idxNippleRight[1] = 0;
  idxNippleRight[2] = 0;

  for ( itSegRightNippleSlice.GoToBegin(); 
	! itSegRightNippleSlice.IsAtEnd(); 
	++itSegRightNippleSlice )
  {
    if ( itSegRightNippleSlice.Get() ) {
      idx = itSegRightNippleSlice.GetIndex();

      idxNippleRight[0] += idx[0];
      idxNippleRight[1] += idx[1];
      idxNippleRight[2] += idx[2];

      nVoxels++;
    }
  }

  if ( ! nVoxels ) 
  {
    itkExceptionMacro( << "ERROR: Could not find the right nipple" );
  }

  idxNippleRight[0] /= nVoxels;
  idxNippleRight[1] /= nVoxels;
  idxNippleRight[2] /= nVoxels;

  if (flgVerbose) 
    std::cout << "Right nipple location: " << idxNippleRight << std::endl;


  // Find four 'areolar' points around the nipple to use as chest surfaces seeds later
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  idxAreolarLeft[0] = FindSurfacePoint( idxNippleLeft, -10,   0 ); 
  idxAreolarLeft[1] = FindSurfacePoint( idxNippleLeft,   0, -10 ); 
  idxAreolarLeft[2] = FindSurfacePoint( idxNippleLeft,  10,   0 ); 
  idxAreolarLeft[3] = FindSurfacePoint( idxNippleLeft,   0,  10 ); 

  idxAreolarRight[0] = FindSurfacePoint( idxNippleRight, -10,   0 ); 
  idxAreolarRight[1] = FindSurfacePoint( idxNippleRight,   0, -10 ); 
  idxAreolarRight[2] = FindSurfacePoint( idxNippleRight,   5,   0 ); 
  idxAreolarRight[3] = FindSurfacePoint( idxNippleRight,   0,   5 ); 


  // Find the mid-point on the sternum between the nipples
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Iterate towards the sternum from the midpoint between the nipples

  typename InternalImageType::RegionType region;
  typename InternalImageType::SizeType size;
  typename InternalImageType::IndexType start;

  region = imSegmented->GetLargestPossibleRegion();
  size = region.GetSize();

  start[0] = size[0]/2;
  start[1] = (idxNippleLeft[1] + idxNippleRight[1] )/2;
  start[2] = (idxNippleLeft[2] + idxNippleRight[2] )/2;

  region.SetIndex( start );

  size[0] = 1;
  size[1] = size[1] - start[1];
  size[2] = 1;

  region.SetSize( size );

  if (flgVerbose) 
    std::cout << "Searching for sternum starting from: " << start << std::endl;

  LineIteratorType itSegLinear( imSegmented, region );
  itSegLinear.SetDirection( 1 );

  while ( ! itSegLinear.IsAtEndOfLine() )
  {
    idxMidSternum = itSegLinear.GetIndex();

    if ( itSegLinear.Get() ) 
    {
      break;
    }
    ++itSegLinear;
  }

  if (flgVerbose) 
    std::cout << "Mid-sternum location: " << idxMidSternum << std::endl;

  typename InternalImageType::PixelType 
    pixelValueMidSternumT2 = imStructural->GetPixel( idxMidSternum );

  if (flgVerbose) 
    std::cout << "Mid-sternum structural voxel intensity: " << pixelValueMidSternumT2
	      << std::endl;

  
  // Find the furthest posterior point from the nipples 
  // and discard anything below this
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Left breast first

  region = imSegmented->GetLargestPossibleRegion();
  size = region.GetSize();

  region.SetIndex( idxNippleLeft );

  size[0] = 1;
  size[1] = size[1] - idxNippleLeft[1];
  size[2] = 1;

  region.SetSize( size );  

  LineIteratorType itSegLinearLeftPosterior( imSegmented, region );
  itSegLinearLeftPosterior.SetDirection( 1 );

  while ( ! itSegLinearLeftPosterior.IsAtEndOfLine() )
  {
    idxLeftPosterior = itSegLinearLeftPosterior.GetIndex();

    if ( ! itSegLinearLeftPosterior.Get() ) 
    {
      break;
    }

    ++itSegLinearLeftPosterior;
  }

  idxLeftBreastMidPoint[0] = ( idxNippleLeft[0] + idxLeftPosterior[0] )/2;
  idxLeftBreastMidPoint[1] = ( idxNippleLeft[1] + idxLeftPosterior[1] )/2;
  idxLeftBreastMidPoint[2] = ( idxNippleLeft[2] + idxLeftPosterior[2] )/2;

  if (flgVerbose) 
    std::cout << "Left posterior breast location: " << idxLeftPosterior << std::endl
	      << "Left breast center: " << idxLeftBreastMidPoint << std::endl;


  // then the right breast

  typename InternalImageType::IndexType idxRightPosterior;

  region = imSegmented->GetLargestPossibleRegion();
  size = region.GetSize();

  region.SetIndex( idxNippleRight );

  size[0] = 1;
  size[1] = size[1] - idxNippleRight[1];
  size[2] = 1;

  region.SetSize( size );  

  LineIteratorType itSegLinearRightPosterior( imSegmented, region );
  itSegLinearRightPosterior.SetDirection( 1 );

  while ( ! itSegLinearRightPosterior.IsAtEndOfLine() )
  {
    idxRightPosterior = itSegLinearRightPosterior.GetIndex();

    if ( ! itSegLinearRightPosterior.Get() ) 
    {
      break;
    }

    ++itSegLinearRightPosterior;
  }

  idxRightBreastMidPoint[0] = ( idxNippleRight[0] + idxRightPosterior[0] )/2;
  idxRightBreastMidPoint[1] = ( idxNippleRight[1] + idxRightPosterior[1] )/2;
  idxRightBreastMidPoint[2] = ( idxNippleRight[2] + idxRightPosterior[2] )/2;

  if (flgVerbose)  
    std::cout << "Right posterior breast location: " << idxRightPosterior << std::endl
	      << "Left breast center: " << idxLeftBreastMidPoint << std::endl;
}


// --------------------------------------------------------------------------
// Compute a 2D map of the height of the patient's anterior skin surface
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::ComputeElevationOfAnteriorSurface( bool flgCoilCrop )
{
  typename InternalImageType::RegionType    region3D;
  typename InternalImageType::SizeType      size3D;
  typename InternalImageType::IndexType     start3D;
  typename InternalImageType::DirectionType direction3D;

  typename AxialImageType::RegionType region2D;
  typename AxialImageType::SizeType   size2D;
  typename AxialImageType::IndexType  start2D;

  float elevation;
  float maxElevation = 0.;


  region3D    = imSegmented->GetLargestPossibleRegion();
  start3D     = region3D.GetIndex();
  size3D      = region3D.GetSize();
  direction3D = imSegmented->GetDirection();

  size2D[0] = size3D[0];
  size2D[1] = size3D[2];
  
  start2D[0] = start3D[0];
  start2D[1] = start3D[2];

  region2D.SetIndex( start2D );
  region2D.SetSize( size2D );

  typename InternalImageType::PointType origin3D;
  typename AxialImageType::PointType origin2D;

  origin3D = imSegmented->GetOrigin();

  origin2D[0] = origin3D[0];
  origin2D[1] = origin3D[2];

  typename InternalImageType::SpacingType spacing3D;
  typename AxialImageType::SpacingType spacing2D;

  spacing3D = imSegmented->GetSpacing();

  spacing2D[0] = spacing3D[0];
  spacing2D[1] = spacing3D[2];


  imSkinElevationMap = AxialImageType::New();

  imSkinElevationMap->SetRegions  ( region2D );
  imSkinElevationMap->SetOrigin   ( origin2D );
  imSkinElevationMap->SetSpacing  ( spacing2D );

  imSkinElevationMap->Allocate();
  imSkinElevationMap->FillBuffer( 0 );


  LineIteratorType itSegLinear( imSegmented, region3D );

  itSegLinear.SetDirection( 1 );

  typedef itk::ImageRegionIterator< AxialImageType > AxialIteratorType;  
    
  AxialIteratorType itElevation( imSkinElevationMap, region2D );

  bool flgFoundSurface;
  typename InternalImageType::IndexType idx;
        
  for ( itSegLinear.GoToBegin(), itElevation.GoToBegin(); 
	! itSegLinear.IsAtEnd(); 
	itSegLinear.NextLine(), ++itElevation )
  {
    itSegLinear.GoToBeginOfLine();
    flgFoundSurface = false;
    
    while ( ! itSegLinear.IsAtEndOfLine() )
    {
      idx = itSegLinear.GetIndex();

      if ( itSegLinear.Get() ) 
      {
        flgFoundSurface = true;
        break;
      }
      ++itSegLinear;
    }
    
    if ( flgFoundSurface ) 
    {
      elevation = static_cast<RealType>(size3D[1] - 1 - idx[1])*spacing3D[1];
      itElevation.Set( elevation );

      if ( elevation > maxElevation )
      {
        maxElevation = elevation;
      }
    }
  }
  
  if ( flgVerbose )
  {
    std::cout << "Max skin surface anterior elevation: " << maxElevation << std::endl;
  }

  // Subtract 30mm to ensure we avoid local minimum near the nipple ('y' axis = AP)

  maxElevation -= 30.;

  if ( flgVerbose )
  {
    std::cout << "Max skin surface anterior elevation (less 30mm): " << maxElevation << std::endl;
  }



  // Smooth the elevation map

  typedef itk::SmoothingRecursiveGaussianImageFilter< AxialImageType, AxialImageType > SmoothingFilterType;
 
  typename SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();

  smoothing->SetInput( imSkinElevationMap );
  smoothing->SetSigma( 3 );
  smoothing->Update();

  imSkinElevationMap = smoothing->GetOutput();
  imSkinElevationMap->DisconnectPipeline();

  // Write the image to a file

  if ( fileOutputSkinElevationMap.length() )
  {
    typedef itk::ImageFileWriter< AxialImageType > FileWriterType;

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( fileOutputSkinElevationMap );
    writer->SetInput( imSkinElevationMap );

    std::cout << "Writing elevation map to file: "
              << fileOutputSkinElevationMap << std::endl;

    writer->Update();
  }


  // Crop the breast image region by thresholding an edge enhanced version of the elevation map
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if ( flgCoilCrop )
  {
    // Determine the elevation of the nipples and mid-sternum points

    typename AxialImageType::IndexType nippleLeftIndex;
    typename AxialImageType::IndexType nippleRightIndex;
    typename AxialImageType::IndexType midSternumIndex;

    nippleLeftIndex[0] = idxNippleLeft[0];
    nippleLeftIndex[1] = idxNippleLeft[2];

    nippleRightIndex[0] = idxNippleRight[0];
    nippleRightIndex[1] = idxNippleRight[2];

    midSternumIndex[0] = idxMidSternum[0];
    midSternumIndex[1] = idxMidSternum[2];

    RealType nippleLeftElevation  = imSkinElevationMap->GetPixel( nippleLeftIndex );
    RealType nippleRightElevation = imSkinElevationMap->GetPixel( nippleRightIndex );
    RealType midSternumElevation  = imSkinElevationMap->GetPixel( midSternumIndex );

    std::cout << "Left nipple elevation:  " << nippleLeftElevation  << std::endl
              << "Right nipple elevation: " << nippleRightElevation << std::endl
              << "Mid-sternum elevation:  " << midSternumElevation  << std::endl;


    // Compute the gradient magnitude of the elevation map (map has already been smoothed)

    typedef itk::GradientMagnitudeImageFilter<AxialImageType, AxialImageType>  GradMagFilterType;
 
    typename GradMagFilterType::Pointer gradientFilter = GradMagFilterType::New();

    gradientFilter->SetInput( imSkinElevationMap );
    gradientFilter->Update();

    typename AxialImageType::Pointer imSkinElevationMapGradMag = gradientFilter->GetOutput();

    // Write the image to a file

    if ( fileOutputSkinElevationMap.length() )
    {
      std::string fileOutputSkinElevationMapGradMag;

      typedef itk::ImageFileWriter< AxialImageType > FileWriterType;

      typename FileWriterType::Pointer writer = FileWriterType::New();

      fileOutputSkinElevationMapGradMag = ModifySuffix( fileOutputSkinElevationMap,
                                                        std::string( "_GradMag" ) );

      writer->SetFileName( fileOutputSkinElevationMapGradMag );
      writer->SetInput( imSkinElevationMapGradMag );

      std::cout << "Writing gradient magnitude of elevation map to file: "
                << fileOutputSkinElevationMapGradMag << std::endl;

      writer->Update();
    }

    // Rescale the range of the magnitude to the nipple to mid-sternum
    // distance and add to the elevation map

    typedef typename itk::RescaleIntensityImageFilter< AxialImageType, AxialImageType > RescaleFilterType;
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();

    rescaleFilter->SetInput( imSkinElevationMapGradMag );

    rescaleFilter->SetOutputMinimum( 0 );
    rescaleFilter->SetOutputMaximum( ( nippleLeftElevation + nippleRightElevation )/2. 
                                     - midSternumElevation );

    rescaleFilter->Update();

    typedef typename itk::AddImageFilter < AxialImageType > AddImageFilterType;
 
    typename AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();

    addFilter->SetInput1( imSkinElevationMap );
    addFilter->SetInput2( rescaleFilter->GetOutput() );

    addFilter->Update();

    imSkinElevationMap = addFilter->GetOutput();

    // Write the image to a file

    if ( fileOutputSkinElevationMap.length() )
    {
      std::string fileOutput;

      typedef itk::ImageFileWriter< AxialImageType > FileWriterType;

      typename FileWriterType::Pointer writer = FileWriterType::New();

      fileOutput = ModifySuffix( fileOutputSkinElevationMap,
                                 std::string( "_Enhanced" ) );

      writer->SetFileName( fileOutput );
      writer->SetInput( imSkinElevationMap );

      std::cout << "Writing enhanced elevation map to file: "
                << fileOutput << std::endl;

      writer->Update();
    }

    // Region grow from the nipples to generate the mask

    typedef itk::ConnectedThresholdImageFilter< AxialImageType, AxialImageType > AxialConnectedFilterType;
    typename AxialConnectedFilterType::Pointer connectedThreshold = AxialConnectedFilterType::New();

      connectedThreshold->SetInput( imSkinElevationMap );

      RealType lowThreshold;
      RealType highThreshold;

      lowThreshold =  midSternumElevation + 
        ( ( nippleLeftElevation + nippleRightElevation )/2. - midSternumElevation )/3.;

      highThreshold = nippleLeftElevation + nippleRightElevation;

      connectedThreshold->SetLower( lowThreshold  );
      connectedThreshold->SetUpper( highThreshold );

      connectedThreshold->SetReplaceValue( 1000 );
      connectedThreshold->ClearSeeds();

      connectedThreshold->AddSeed( nippleLeftIndex );
      connectedThreshold->AddSeed( nippleRightIndex );

      std::cout << "Region-growing the skin elevation between: " 
                << lowThreshold << " and " << highThreshold << std::endl;

      connectedThreshold->Update();

      typename AxialImageType::Pointer imSkinElevationMask = connectedThreshold->GetOutput();

      // Expand the mask by a certain radius?

      if ( coilCropDistance )
      {
        typedef SignedMaurerDistanceMapImageFilter<AxialImageType, AxialImageType> 
          DistanceMapFilterType;

        typename DistanceMapFilterType::Pointer distFilter = DistanceMapFilterType::New();

        distFilter->SetInput( imSkinElevationMask );
        distFilter->SetUseImageSpacing(true);

        std::cout << "Computing distance transform for skin elevation mask" << std::endl;
        distFilter->Update();


        typedef itk::BinaryThresholdImageFilter< AxialImageType, AxialImageType > 
          ElevationThresholdFilterType;

        typename ElevationThresholdFilterType::Pointer 
          thresholder = ElevationThresholdFilterType::New();
  
        thresholder->SetLowerThreshold( -100000 ); // Include everytging inside the current mask
        thresholder->SetUpperThreshold( coilCropDistance );

        thresholder->SetOutsideValue(  0  );
        thresholder->SetInsideValue( 1000 );

        thresholder->SetInput( distFilter->GetOutput() );

        thresholder->Update();
  
        imSkinElevationMask = thresholder->GetOutput();
        imSkinElevationMask->DisconnectPipeline();  

      }

      // Include the axilla?

      if ( ! flgExcludeAxilla )
      {        
        typename AxialImageType::RegionType lateralRegion;
        typename AxialImageType::IndexType  lateralStart;
        typename AxialImageType::SizeType   lateralSize;

        typedef itk::ImageRegionIteratorWithIndex< AxialImageType > AxialLateralIteratorType;  

        typename AxialImageType::IndexType idx;

        typename AxialImageType::IndexType idxMostLeftLateralAndSuperior;
        typename AxialImageType::IndexType idxMostLeftMedialAndInferior;

        // Start iterating over the left-hand side

        lateralRegion = region2D;

        lateralStart = start2D;  
        lateralSize = size2D;

        std::cout << "Size: " << lateralSize << std::endl;

        lateralSize[0] = lateralSize[0]/2;
        lateralRegion.SetSize(  lateralSize );

        std::cout << "Size: " << lateralSize << std::endl;

        idxMostLeftLateralAndSuperior = nippleLeftIndex;
        idxMostLeftMedialAndInferior  = nippleLeftIndex;

        if ( flgVerbose )
          std::cout << "Iterating over left region: " << lateralRegion << std::endl;

        AxialLateralIteratorType itLeftRegion( imSkinElevationMask, lateralRegion );

        for ( itLeftRegion.GoToBegin(); 
              ! itLeftRegion.IsAtEnd();
              ++itLeftRegion )
        {
          if ( itLeftRegion.Get() )
          {
            idx = itLeftRegion.GetIndex();

            if ( idx[0] < idxMostLeftLateralAndSuperior[0] )
            {
              idxMostLeftLateralAndSuperior[0] = idx[0];
            }

            if ( idx[1] > idxMostLeftLateralAndSuperior[1] )
            {
              idxMostLeftLateralAndSuperior[1] = idx[1];
            }

            if ( idx[0] > idxMostLeftMedialAndInferior[0] )
            {
              idxMostLeftMedialAndInferior[0] = idx[0];
            }

            if ( idx[1] < idxMostLeftMedialAndInferior[1] )
            {
             idxMostLeftMedialAndInferior[1] = idx[1];
            }
          }
        }
        
        lateralSize[0] = nippleLeftIndex[0] - idxMostLeftLateralAndSuperior[0];
        lateralSize[1] = idxMostLeftLateralAndSuperior[1] - nippleLeftIndex[1];
        
        lateralRegion.SetSize( lateralSize );

        idx[0] = idxMostLeftLateralAndSuperior[0];
        idx[1] = nippleLeftIndex[1];

        lateralRegion.SetIndex( idx );
        
        if ( flgVerbose )
          std::cout << "Iterating over left axilla region: " << lateralRegion << std::endl;
        
        AxialLateralIteratorType itLeftAxilla( imSkinElevationMask, lateralRegion );

        for ( itLeftAxilla.GoToBegin(); 
              ! itLeftAxilla.IsAtEnd();
              ++ itLeftAxilla )
        {
          itLeftAxilla.Set( 1000 );
        }

        // Iterate over the right-hand side

        typename AxialImageType::IndexType idxMostRightLateralAndSuperior;
        typename AxialImageType::IndexType idxMostRightMedialAndInferior;

        lateralRegion = region2D;

        lateralStart = start2D;  
        lateralSize = size2D;

        std::cout << "Size: " << lateralSize << std::endl;

        lateralSize[0] = lateralSize[0]/2;
        lateralRegion.SetSize(  lateralSize );

        std::cout << "Size: " << lateralSize << std::endl;

        lateralStart[0] = lateralSize[0];  
        lateralRegion.SetIndex( lateralStart );

        idxMostRightLateralAndSuperior = nippleRightIndex;
        idxMostRightMedialAndInferior  = nippleRightIndex;

        if ( flgVerbose )
          std::cout << "Iterating over right region: " << lateralRegion << std::endl;

        AxialLateralIteratorType itRightRegion( imSkinElevationMask, lateralRegion );

        for ( itRightRegion.GoToBegin(); 
              ! itRightRegion.IsAtEnd();
              ++itRightRegion )
        {
          if ( itRightRegion.Get() )
          {
            idx = itRightRegion.GetIndex();

            if ( idx[0] > idxMostRightLateralAndSuperior[0] )
            {
              idxMostRightLateralAndSuperior[0] = idx[0];
            }

            if ( idx[1] > idxMostRightLateralAndSuperior[1] )
            {
              idxMostRightLateralAndSuperior[1] = idx[1];
            }

            if ( idx[0] < idxMostRightMedialAndInferior[0] )
            {
              idxMostRightMedialAndInferior[0] = idx[0];
            }

            if ( idx[1] < idxMostRightMedialAndInferior[1] )
            {
             idxMostRightMedialAndInferior[1] = idx[1];
            }
          }
        }
        
        lateralSize[0] = idxMostRightLateralAndSuperior[0] - nippleRightIndex[0];
        lateralSize[1] = idxMostRightLateralAndSuperior[1] - nippleRightIndex[1];
        
        lateralRegion.SetSize( lateralSize );

        idx[0] = nippleRightIndex[0];
        idx[1] = nippleRightIndex[1];

        lateralRegion.SetIndex( idx );
        
        if ( flgVerbose )
          std::cout << "Iterating over right axilla region: " << lateralRegion << std::endl;
        
        AxialLateralIteratorType itRightAxilla( imSkinElevationMask, lateralRegion );

        for ( itRightAxilla.GoToBegin(); 
              ! itRightAxilla.IsAtEnd();
              ++itRightAxilla )
        {
          itRightAxilla.Set( 1000 );
        }

        if ( flgVerbose )
          std::cout << "Left bounding box from: " 
                    << idxMostLeftLateralAndSuperior 
                    << " to: "
                    << idxMostLeftMedialAndInferior
                    << std::endl
                    << "Right bounding box from: " 
                    << idxMostRightLateralAndSuperior 
                    << " to: "
                    << idxMostRightMedialAndInferior
                    << std::endl;
      }


      // Write the image to a file

      if ( fileOutputSkinElevationMap.length() )
      {
        std::string fileOutput;

        typedef itk::ImageFileWriter< AxialImageType > FileWriterType;

        typename FileWriterType::Pointer writer = FileWriterType::New();

        fileOutput = ModifySuffix( fileOutputSkinElevationMap,
                                   std::string( "_Mask" ) );

        writer->SetFileName( fileOutput );
        writer->SetInput( imSkinElevationMask );

        std::cout << "Writing elevation map mask to file: "
                  << fileOutput << std::endl;

        writer->Update();
      }

      
      // Fit a thin-plate spline to the skin area outside the breasts

      if ( fileOutputSkinSurfaceNoBreasts.length() )
      {
        typedef itk::PointSet<double, ImageDimension> LandmarkPointSetType;

        typedef typename itk::ThinPlateSplineScatteredDataPointSetToImageFilter< LandmarkPointSetType, InternalImageType > ThinPlateSplineFilterType;

        typedef typename ThinPlateSplineFilterType::LandmarkPointType LandmarkPointType;

        typename LandmarkPointSetType::Pointer PointSet = LandmarkPointSetType::New();

        LandmarkPointType point; 

        typedef itk::ImageRegionIteratorWithIndex< AxialImageType > AxialIteratorType;  

        AxialIteratorType itElevationMap(  imSkinElevationMap,  region2D );
        AxialIteratorType itElevationMask( imSkinElevationMask, region2D );

        typename AxialImageType::IndexType idx2D;

        typename InternalImageType::IndexType idx3D;
        typename InternalImageType::PointType pt3D;

        unsigned int iSkip = 0;
        unsigned int iLandmark = 0;
        unsigned int stride = 0;
        unsigned int nLandmarks = 0;

        // Count the number of points

        itElevationMask.GoToBegin();

        while ( ! itElevationMask.IsAtEnd() )
        {
          if ( ! itElevationMask.Get() )
          {
            nLandmarks++;
          }
          ++itElevationMask;
        }

        if ( nLandmarks > 100 )
        {
          stride = nLandmarks / 100;
        }

        std::cout << "Number of potential points: " << nLandmarks
                  << ", skipping: " << stride << std::endl;

        std::string fileOutput = niftk::ModifyImageFileSuffix( fileOutputSkinSurfaceNoBreasts,
                                                               std::string( ".pts" ) );
     
        std::fstream fout;
        fout.open(fileOutput.c_str(), std::ios::out);
    
        if ((! fout) || fout.bad()) 
        {
          itkExceptionMacro( << "ERROR: Failed to open file: " << fileOutput.c_str() );
        }

        // Create the point set

        itElevationMask.GoToBegin();
        itElevationMap.GoToBegin();

        while ( (! itElevationMask.IsAtEnd()) && (! itElevationMap.IsAtEnd()) )
        {
          if ( ! itElevationMask.Get() )
          {
            idx2D = itElevationMask.GetIndex();

            idx3D[0] = idx2D[0];
            idx3D[2] = idx2D[1];

            elevation = itElevationMap.Get();
            idx3D[1] = static_cast<RealType>(size3D[1] - 1) - elevation/spacing3D[1];

            imSegmented->TransformIndexToPhysicalPoint( idx3D, pt3D );

            std::cout << iLandmark << ": " << idx3D << " " << pt3D << std::endl;
            fout << pt3D[0] << " " << pt3D[1] << " " << pt3D[2] << std::endl;

            PointSet->SetPoint( iLandmark, pt3D );

            iLandmark++;

            ++itElevationMask;
            ++itElevationMap;

            iSkip = 0;
            while ( ( iSkip < stride ) && 
                    (! itElevationMask.IsAtEnd()) && (! itElevationMap.IsAtEnd()) )
            {
              if ( ! itElevationMask.Get() )
              {
                iSkip++;
              }

              ++itElevationMask;
              ++itElevationMap;
            }
          }
          else
          {
            ++itElevationMask;
            ++itElevationMap;
          }
        }

        fout.close();

        // Fit a thin-plate spline
        
        typename ThinPlateSplineFilterType::Pointer filter = ThinPlateSplineFilterType::New();

        filter->SetInput( PointSet );
        
        filter->SetSpacing( spacing3D );
        filter->SetOrigin(  origin3D );
        filter->SetSize(    size3D );

        filter->SetInvert( true );

        filter->SetSplineHeightDimension( 1 );
        filter->SetStiffness( 100 );

        filter->Print( std::cout );
        filter->Update();
        
        // Write the image

        typedef itk::ImageFileWriter< InternalImageType > FileWriterType;

        typename FileWriterType::Pointer writer = FileWriterType::New();

        writer->SetFileName( fileOutputSkinSurfaceNoBreasts );
        writer->SetInput( filter->GetOutput() );

        std::cout << "Writing skin surface mask to file: "
                  << fileOutputSkinSurfaceNoBreasts << std::endl;

        writer->Update();

      }

      imSkinElevationMap = imSkinElevationMask;
  }


  // Default behaviour is simply to crop the left and right sides to remove the arms
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  else
  {

    // Scan from nipple x coordinates and eliminate arms - Left side

    RealType posElevationTolerance = 5./spacing3D[1]; // 5mm
    RealType negElevationTolerance = 3./spacing3D[1]; // 3mm

    typedef itk::ImageLinearIteratorWithIndex< AxialImageType > AxialLineIteratorType;

    typename AxialImageType::RegionType leftAxialRegion2D;
    typename AxialImageType::SizeType   leftAxialSize2D   = size2D;
    typename AxialImageType::IndexType  leftAxialStart2D  = start2D;

    typename AxialImageType::IndexType  leftAxialCutoff  = start2D;

    leftAxialStart2D[1] = idxNippleLeft[2]; // should this be idxNippleLeft[2] ?

    leftAxialSize2D[0] = idxNippleLeft[0] - start2D[0];
    leftAxialSize2D[1] = 1;

    leftAxialRegion2D.SetIndex( leftAxialStart2D );
    leftAxialRegion2D.SetSize(  leftAxialSize2D );
  

    AxialLineIteratorType itLeftNippleElevation( imSkinElevationMap, leftAxialRegion2D );

    itLeftNippleElevation.SetDirection( 0 );

    InputPixelType minElevation;
    InputPixelType startElevation;
   
    for ( itLeftNippleElevation.GoToBegin(); 
          ! itLeftNippleElevation.IsAtEnd(); 
          itLeftNippleElevation.NextLine() )
    {
      // Calculate the minimum elevation of the left side

      itLeftNippleElevation.GoToReverseBeginOfLine();

      startElevation = itLeftNippleElevation.Get();
      minElevation = startElevation;

      while ( ( ! itLeftNippleElevation.IsAtReverseEndOfLine() )
              && ( ( minElevation > startElevation - posElevationTolerance )
                   || ( itLeftNippleElevation.Get() < minElevation + negElevationTolerance ) ) )
      {
        if ( ( itLeftNippleElevation.Get() < minElevation ) &&
             ( itLeftNippleElevation.Get() < maxElevation ) )
        {
          minElevation = itLeftNippleElevation.Get();
          leftAxialCutoff = itLeftNippleElevation.GetIndex();
        }
      
        --itLeftNippleElevation;
      }
    }


    // Set this whole region in the elevation map to zero

    leftAxialStart2D  = start2D;

    leftAxialSize2D[0] = leftAxialCutoff[0] - start2D[0];
    leftAxialSize2D[1] = size2D[1];

    leftAxialRegion2D.SetIndex( leftAxialStart2D );
    leftAxialRegion2D.SetSize(  leftAxialSize2D );
  

    AxialLineIteratorType itLeftLinearElevation( imSkinElevationMap, leftAxialRegion2D );

    itLeftLinearElevation.SetDirection( 0 );

    for ( itLeftLinearElevation.GoToBegin(); 
          ! itLeftLinearElevation.IsAtEnd(); 
          itLeftLinearElevation.NextLine() )
    {
      itLeftLinearElevation.GoToReverseBeginOfLine();

      while ( ! itLeftLinearElevation.IsAtReverseEndOfLine() )
      {
        itLeftLinearElevation.Set( 0 );
        --itLeftLinearElevation;
      }
    }


    // Scan from nipple x coordinates and eliminate arms - Right side

    typename AxialImageType::RegionType rightAxialRegion2D;
    typename AxialImageType::SizeType   rightAxialSize2D   = size2D;
    typename AxialImageType::IndexType  rightAxialStart2D  = start2D;
  
    typename AxialImageType::IndexType  rightAxialCutoff  = start2D;

    rightAxialStart2D[0] = idxNippleRight[0];
    rightAxialStart2D[1] = idxNippleRight[2]; // should this be idxNippleRight[2] ?

    rightAxialSize2D[0] = size2D[0] - idxNippleRight[0];
    rightAxialSize2D[1] = 1;

    rightAxialRegion2D.SetIndex( rightAxialStart2D );
    rightAxialRegion2D.SetSize(  rightAxialSize2D );
  

    AxialLineIteratorType itRightNippleElevation( imSkinElevationMap, rightAxialRegion2D );

    itRightNippleElevation.SetDirection( 0 );

    for ( itRightNippleElevation.GoToBegin(); 
          ! itRightNippleElevation.IsAtEnd(); 
          itRightNippleElevation.NextLine() )
    {
      // Calculate the minimum elevation of the right side

      itRightNippleElevation.GoToBeginOfLine();

      startElevation = itRightNippleElevation.Get();
      minElevation = startElevation;

      while ( ( ! itRightNippleElevation.IsAtEndOfLine() )
              && ( ( minElevation > startElevation - posElevationTolerance )
                   || ( itRightNippleElevation.Get() < minElevation + negElevationTolerance ) ) )
      {
        if ( ( itRightNippleElevation.Get() < minElevation ) &&
             ( itRightNippleElevation.Get() < maxElevation ) )
        {
          minElevation = itRightNippleElevation.Get();
          rightAxialCutoff = itRightNippleElevation.GetIndex();
        }
      
        ++itRightNippleElevation;
      }
    }

    // Set this whole region in the elevation map to zero

    rightAxialStart2D[0] = rightAxialCutoff[0];
    rightAxialStart2D[1] = start2D[1];

    rightAxialSize2D[0] = size2D[0] - rightAxialCutoff[0];
    rightAxialSize2D[1] = size2D[1];

    rightAxialRegion2D.SetIndex( rightAxialStart2D );
    rightAxialRegion2D.SetSize(  rightAxialSize2D );
  

    AxialLineIteratorType itRightLinearElevation( imSkinElevationMap, rightAxialRegion2D );

    itRightLinearElevation.SetDirection( 0 );

    for ( itRightLinearElevation.GoToBegin(); 
          ! itRightLinearElevation.IsAtEnd(); 
          itRightLinearElevation.NextLine() )
    {
      itRightLinearElevation.GoToBeginOfLine();

      while ( ! itRightLinearElevation.IsAtEndOfLine() )
      {
        itRightLinearElevation.Set( 0 );
        ++itRightLinearElevation;
      }

    }

    // Write the image to a file

    if ( fileOutputSkinElevationMap.length() )
    {
      std::string fileOutputSkinElevationMapCropped;

      typedef itk::ImageFileWriter< AxialImageType > FileWriterType;

      typename FileWriterType::Pointer writer = FileWriterType::New();

      fileOutputSkinElevationMapCropped = ModifySuffix( fileOutputSkinElevationMap,
                                                        std::string( "_Cropped" ) );

      writer->SetFileName( fileOutputSkinElevationMapCropped );
      writer->SetInput( imSkinElevationMap );

      std::cout << "Writing elevation map to file: "
                << fileOutputSkinElevationMapCropped << std::endl;

      writer->Update();
    }

    imSkinElevationMap = 0;


    // Hence crop the patient mask

    // Left side

    typename InternalImageType::SizeType leftSize3D = size3D;

    leftSize3D[0] = leftAxialCutoff[0] - start3D[0];

    region3D.SetIndex( start3D );
    region3D.SetSize(  leftSize3D );

    IteratorType segIteratorLeft( imSegmented, region3D );
        
    for ( segIteratorLeft.GoToBegin(); 
          ! segIteratorLeft.IsAtEnd(); 
          ++segIteratorLeft )
    {
      segIteratorLeft.Set( 0 );
    }


    // Right side

    typename InternalImageType::IndexType rightStart3D = start3D;
    typename InternalImageType::SizeType  rightSize3D = size3D;

    rightStart3D[0] = rightAxialCutoff[0];

    rightSize3D[0] = size3D[0] - rightStart3D[0];

    region3D.SetIndex( rightStart3D );
    region3D.SetSize(  rightSize3D );

    IteratorType segIteratorRight( imSegmented, region3D );
        
    for ( segIteratorRight.GoToBegin(); 
          ! segIteratorRight.IsAtEnd(); 
          ++segIteratorRight )
    {
      segIteratorRight.Set( 0 );
    }

  }


  // Write the background mask to a file?
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  WriteBinaryImageToUCharFile( fileOutputBackground, "background image", 
                               imSegmented, flgLeft, flgRight );
}


// --------------------------------------------------------------------------
// Segment the Pectoral Muscle
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::PointSetType::Pointer 
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::SegmentThePectoralMuscle( RealType rYHeightOffset, unsigned long &iPointPec, 
                            bool flgIncludeNippleSeeds )
{
  typename InternalImageType::RegionType region;
  typename InternalImageType::SizeType  size;
  typename InternalImageType::SizeType  sizeSearch;
  typename InternalImageType::IndexType start;
  typename InternalImageType::IndexType startMidSternum[5];

  typename PointSetType::Pointer pecPointSet = PointSetType::New();  
  typename InternalImageType::IndexType idxPectoralSeed[5];
   

  // Iterate from mid sternum posteriorly looking for the first pectoral voxel
  // Also iterate from the nipple x,z positions to obtain four more seed points

  startMidSternum[0][0] = idxMidSternum[0];
  startMidSternum[0][1] = idxMidSternum[1];
  startMidSternum[0][2] = idxMidSternum[2];

  int iSeed, nSeeds;

  if ( flgIncludeNippleSeeds )
  {
    nSeeds = 5;

    startMidSternum[1][0] = (idxNippleRight[0] + 4*idxMidSternum[0])/5;
    startMidSternum[1][1] = idxMidSternum[1];
    startMidSternum[1][2] = idxMidSternum[2];

    startMidSternum[2][0] = (idxNippleLeft[0] + 4*idxMidSternum[0])/5;
    startMidSternum[2][1] = idxMidSternum[1];
    startMidSternum[2][2] = idxMidSternum[2];

    startMidSternum[3][0] = (idxNippleRight[0] + 9*idxMidSternum[0])/10;
    startMidSternum[3][1] = idxMidSternum[1];
    startMidSternum[3][2] = idxMidSternum[2];

    startMidSternum[4][0] = (idxNippleLeft[0] + 9*idxMidSternum[0])/10;
    startMidSternum[4][1] = idxMidSternum[1];
    startMidSternum[4][2] = idxMidSternum[2];
  }
  else
  {
    nSeeds = 1;
  }

  // Iterate posteriorly to make sure we're inside the breast

  region = imSegmented->GetLargestPossibleRegion();
  size = region.GetSize();
    
  for ( iSeed=0; iSeed<nSeeds; iSeed++ )
  {

    region.SetIndex( startMidSternum[ iSeed ] );

    if ( flgVerbose )
    {
      std::cout << "Breast mask search start: " << iSeed << " = " 
                << startMidSternum[ iSeed ] << std::endl;
    }

    sizeSearch[0] = 1;
    sizeSearch[1] = size[1] - idxMidSternum[1];
    sizeSearch[2] = 1;

    region.SetSize( sizeSearch );
    
    LineIteratorType itMaskLinear( imSegmented, region );
    
    itMaskLinear.SetDirection( 1 );
    
    while ( ! itMaskLinear.IsAtEndOfLine() )
    {
      startMidSternum[ iSeed ] = itMaskLinear.GetIndex();
      
      if ( itMaskLinear.Get() ) 
      {
        break;
      }
      ++itMaskLinear;
    }
  }


  // Iterate posteriorly to find the pectoral 'dark line'

  region = imBIFs->GetLargestPossibleRegion();
  size = region.GetSize();
    
  for ( iSeed=0; iSeed<nSeeds; iSeed++ )
  {

    region.SetIndex( startMidSternum[ iSeed ] );

    if ( flgVerbose )
    {
      std::cout << "Pectoral search start: " << iSeed << " = " << startMidSternum[ iSeed ] << std::endl;
    }

    sizeSearch[0] = 1;
    sizeSearch[1] = size[1] - startMidSternum[ iSeed ][1];
    sizeSearch[2] = 1;

    region.SetSize( sizeSearch );
    
    LineIteratorType itBIFsLinear( imBIFs, region );
    
    itBIFsLinear.SetDirection( 1 );
    
    while ( ! itBIFsLinear.IsAtEndOfLine() )
    {
      idxPectoralSeed[ iSeed ] = itBIFsLinear.GetIndex();
      
      if ( itBIFsLinear.Get() == 15 ) 
      {
        break;
      }
      ++itBIFsLinear;
    }


    if ( flgVerbose )
    {
      std::cout << "Pectoral seed: " << iSeed << " = " << idxPectoralSeed[ iSeed ] << std::endl;
    }
  }
    
    
  // And then region grow from this voxel
    
  // Idea: 
  // Create a copy of the BIF image and modify the intensities so that 
  // ascending dark lines (axial view, BIF intensity = 18) in the left 
  // half of the image are changed to 16 (BIF-feature descending dark 
  // lines). Hence the connected thresholder can then be given the 
  // intensity range [15 - 16] for left and right side. 
  //
  //   direction  |  BIF intensity | changed to (for low x)
  //  ------------+----------------+------------------------
  //       /      |        18      |          16
  //       \      |        16      |          18
  //       -      |        15      |
  //       |      |        17      |
  

  typename InternalImageType::Pointer imModBIFs;

  if ( flgExtInitialPect )
  {
    typename DuplicatorType::Pointer duplicatorBIF = DuplicatorType::New();
    duplicatorBIF->SetInputImage( imBIFs ) ;
    duplicatorBIF->Update();
    imModBIFs = duplicatorBIF->GetOutput();
    imModBIFs->DisconnectPipeline();
    duplicatorBIF = 0;
      
    // Set the region for the low x-indices [ 0 -> midSternum[0] ]
    region   = imModBIFs->GetLargestPossibleRegion();
      
    sizeSearch     = region.GetSize();
    sizeSearch[0]  = idxMidSternum[0];
      
    start    = region.GetIndex();
    start[0] = start[1] = start[2] = 0;
      
    region.SetIndex( start );
    region.SetSize ( sizeSearch  );

    IteratorType itModBIFs = IteratorType( imModBIFs, region );
      
    for ( itModBIFs.GoToBegin();  !itModBIFs.IsAtEnd();  ++itModBIFs )
    {
      if ( itModBIFs.Get() == 16.0f )
      {
	itModBIFs.Set(18.0f);
	continue;
      }
      if ( itModBIFs.Get() == 18.0f )
      {
	itModBIFs.Set( 16.0f );
	continue;
      }
    }
  }

  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

  connectedThreshold = ConnectedFilterType::New();

  connectedThreshold->SetLower( 15 );
  connectedThreshold->SetReplaceValue( 1000 );
  connectedThreshold->ClearSeeds();

  for ( iSeed=0; iSeed<nSeeds; iSeed++ )
  {
    std::cout << "Setting seed here: " << iSeed << " = " << idxPectoralSeed[ iSeed ] << std::endl;
    connectedThreshold->AddSeed( idxPectoralSeed[ iSeed ] );
  }

  // Extend initial pectoral muscle region?
  if ( flgExtInitialPect )
  {
    connectedThreshold->SetInput( imModBIFs );
    connectedThreshold->SetUpper( 16 );
  }
  else
  {
    connectedThreshold->SetInput( imBIFs );
    connectedThreshold->SetUpper( 15 );
  }
    
  std::cout << "Region-growing the pectoral muscle" << std::endl;
  connectedThreshold->Update();
  
  imPectoralVoxels = connectedThreshold->GetOutput();
  imPectoralVoxels->DisconnectPipeline();  

  connectedThreshold = 0;
  imModBIFs          = 0;

  if ( fMarchingTime > 0. )
  {

    // Iterate through the mask to extract the voxel locations to be
    // used as seeds for the Fast Marching filter
             
    typedef typename FastMarchingFilterType::NodeContainer           NodeContainer;
    typedef typename FastMarchingFilterType::NodeType                NodeType;
    
    NodeType node;
    node.SetValue( 0 );

    typename NodeContainer::Pointer seeds = NodeContainer::New();
    seeds->Initialize();

    IteratorType pecVoxelIterator( imPectoralVoxels, 
                                   imPectoralVoxels->GetLargestPossibleRegion() );
        
    for ( i=0, pecVoxelIterator.GoToBegin(); 
          ! pecVoxelIterator.IsAtEnd(); 
          i++, ++pecVoxelIterator )
    {
      if ( pecVoxelIterator.Get() ) {
        node.SetIndex( pecVoxelIterator.GetIndex() );
        seeds->InsertElement( i, node );
      }	
    }
   

    // Apply the Fast Marching filter to these seed positions
    
    typename GradientFilterType::Pointer gradientMagnitude = GradientFilterType::New();
    
    gradientMagnitude->SetSigma( 1 );
    gradientMagnitude->SetInput( imSpeedFuncInputImage );
    gradientMagnitude->Update();

    WriteImageToFile( fileOutputGradientMagImage, "gradient magnitude image", 
                      gradientMagnitude->GetOutput(), flgLeft, flgRight );

    typename SigmoidFilterType::Pointer sigmoid = SigmoidFilterType::New();

    sigmoid->SetOutputMinimum(  0.0  );
    sigmoid->SetOutputMaximum(  1.0  );

    //K1: min gradient along contour of structure to be segmented
    //K2: average value of gradient magnitude in middle of structure

    sigmoid->SetAlpha( (fMarchingK2 - fMarchingK1) / 6. );
    sigmoid->SetBeta ( (fMarchingK1 + fMarchingK2) / 2. );

    sigmoid->SetInput( gradientMagnitude->GetOutput() );

    sigmoid->Update();

    WriteImageToFile( fileOutputSpeedImage, "sigmoid speed image", 
                      sigmoid->GetOutput(), flgLeft, flgRight );

    typename FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();

    fastMarching->SetTrialPoints( seeds );
    fastMarching->SetOutputSize( imStructural->GetLargestPossibleRegion().GetSize() );
    fastMarching->SetStoppingValue( fMarchingTime + 2.0 );
    fastMarching->SetInput( sigmoid->GetOutput() );

    fastMarching->Update();

    WriteImageToFile( fileOutputFastMarchingImage, "fast marching image", 
                      fastMarching->GetOutput(), flgLeft, flgRight );

    
    typename ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
    
    thresholder->SetLowerThreshold(           0.0 );
    thresholder->SetUpperThreshold( fMarchingTime );

    thresholder->SetOutsideValue(  0  );
    thresholder->SetInsideValue(  1000 );

    thresholder->SetInput( fastMarching->GetOutput() );

    std::cout << "Segmenting pectoral with fast marching algorithm" << std::endl;
    thresholder->Update();

    imTmp = thresholder->GetOutput();
    imTmp->DisconnectPipeline();
    
    imPectoralVoxels = imTmp;
    
    // Iterate posteriorly again but this time with the smoothed mask
    
    region = imPectoralVoxels->GetLargestPossibleRegion();
    size = region.GetSize();
    
    for ( iSeed=0; iSeed<nSeeds; iSeed++ )
    {
      region.SetIndex( startMidSternum[ iSeed ] );

      if ( flgVerbose )
      {
        std::cout << "Pectoral surface search start: " << iSeed << " = " 
                  << startMidSternum[ iSeed ] << std::endl;
      }

      sizeSearch[0] = 1;
      sizeSearch[1] = size[1] - startMidSternum[ iSeed ][1];
      sizeSearch[2] = 1;

      region.SetSize( sizeSearch );
    
      LineIteratorType itPectoralVoxels( imPectoralVoxels, region );
    
      itPectoralVoxels.SetDirection( 1 );
    
      while ( ! itPectoralVoxels.IsAtEndOfLine() )
      {
        idxPectoralSeed[ iSeed ] = itPectoralVoxels.GetIndex();

        if ( itPectoralVoxels.Get() ) 
        {
          break;
        }
        ++itPectoralVoxels;
      }
    }
  }


  // Write the pectoral mask?
    
  WriteBinaryImageToUCharFile( fileOutputPectoral, "pectoral mask", imPectoralVoxels,
			       flgLeft, flgRight );

    
  // And region-grow the pectoral surface from this point
    
  typename ConnectedSurfaceVoxelFilterType::Pointer 
    connectedSurfacePecPoints = ConnectedSurfaceVoxelFilterType::New();

  connectedSurfacePecPoints->SetInput( imPectoralVoxels );

  connectedSurfacePecPoints->SetLower( 1  );
  connectedSurfacePecPoints->SetUpper( 1000 );

  connectedSurfacePecPoints->SetReplaceValue( 1000 );
  connectedSurfacePecPoints->ClearSeeds();

  std::cout << "Region-growing the pectoral surface" << std::endl;

  for ( iSeed=0; iSeed<nSeeds; iSeed++ )
  {
    connectedSurfacePecPoints->AddSeed( idxPectoralSeed[ iSeed ] );

    if ( flgVerbose )
    {
      std::cout << "   seed: " << iSeed << " = " << idxPectoralSeed[ iSeed ] << std::endl;
    }
  }

  connectedSurfacePecPoints->Update();
  
  imPectoralSurfaceVoxels = connectedSurfacePecPoints->GetOutput();
  imPectoralSurfaceVoxels->DisconnectPipeline();
 
  // Extract the most anterior pectoral voxels to fit the B-Spline surface to

  region = imPectoralSurfaceVoxels->GetLargestPossibleRegion();

  start[0] = start[1] = start[2] = 0;
  region.SetIndex( start );

  LineIteratorType itPecSurfaceVoxelsLinear( imPectoralSurfaceVoxels, region );

  VectorType pecHeight;
  typename InternalImageType::IndexType idx;
  typename PointSetType::PointType point;

  itPecSurfaceVoxelsLinear.SetDirection( 1 );
    
  for ( itPecSurfaceVoxelsLinear.GoToBegin(); 
	! itPecSurfaceVoxelsLinear.IsAtEnd(); 
	itPecSurfaceVoxelsLinear.NextLine() )
  {
    itPecSurfaceVoxelsLinear.GoToBeginOfLine();
      
    while ( ! itPecSurfaceVoxelsLinear.IsAtEndOfLine() )
    {
      if ( itPecSurfaceVoxelsLinear.Get() ) {

	idx = itPecSurfaceVoxelsLinear.GetIndex();

	// The 'height' of the pectoral surface
	pecHeight[0] = static_cast<RealType>( idx[1] ) - rYHeightOffset;
    
	// Location of this surface point
	point[0] = static_cast<RealType>( idx[0] );
	point[1] = static_cast<RealType>( idx[2] );

	pecPointSet->SetPoint( iPointPec, point );
	pecPointSet->SetPointData( iPointPec, pecHeight );

	iPointPec++;
	    
	break;
      }
	  
      ++itPecSurfaceVoxelsLinear;
    }
  }

  WriteImageToFile( fileOutputPectoralSurfaceVoxels, "pectoral surface voxels", 
		    imPectoralSurfaceVoxels, flgLeft, flgRight );

  imPectoralSurfaceVoxels = 0;

  return pecPointSet;
}


// --------------------------------------------------------------------------
// Discard anything not within the skin elevation mask  
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::CropTheMaskAccordingToEstimateOfCoilExtentInCoronalPlane( void )
{
  if ( imSkinElevationMap )
  {

    SliceIteratorType itSegSlices( imSegmented, 
                                   imSegmented->GetLargestPossibleRegion() );
    
    typedef itk::ImageRegionIterator< AxialImageType > AxialIteratorType;  
  
    AxialIteratorType itElevationMask( imSkinElevationMap, 
                                       imSkinElevationMap->GetLargestPossibleRegion() );
  
    itSegSlices.SetFirstDirection( 0 );
    itSegSlices.SetSecondDirection( 2 );
  
    itSegSlices.GoToBegin();
  
    while ( ! itSegSlices.IsAtEnd() )
    {
      itElevationMask.GoToBegin();
    
      while ( ! itSegSlices.IsAtEndOfSlice() )
      {
        while ( ! itSegSlices.IsAtEndOfLine() ) 
        {
          if ( ! itElevationMask.Get() )
          {
            itSegSlices.Set( 0 );
          }
        
          ++itSegSlices; 
          ++itElevationMask;
        }
        itSegSlices.NextLine();
      }
      itSegSlices.NextSlice(); 
    }

  }

  else
  {
    std::cerr << "WARNING: No skin elevation map available for coil crop, so ignored" << std::endl;
  }
}
  

// --------------------------------------------------------------------------
// Discard anything not within a B-Spline fitted to the breast skin surface
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::MaskWithBSplineBreastSurface( RealType rYHeightOffset )
{

  if ( flgVerbose )
  {
    std::cout << "Fitting B-Spline surface to left and right breast for cropping." << std::endl;
  }
  VectorType surfHeight;
    
  typename InternalImageType::SizeType 
    maxSize = imStructural->GetLargestPossibleRegion().GetSize();

  typename InternalImageType::RegionType lateralRegion;
  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType lateralSize;

  lateralRegion = imChestSurfaceVoxels->GetLargestPossibleRegion();

  lateralStart = lateralRegion.GetIndex();
  lateralSize  = lateralRegion.GetSize();

  // left region definition
  lateralStart[0] = 0;
  lateralStart[1] = idxNippleLeft[1] - 1;  
  lateralStart[2] = 0;

  int positionFraction = 95; // 100 = mid-sternum,  0 = nipple

  lateralSize[0] = idxMidSternum[0];
  lateralSize[1] = positionFraction * ( idxMidSternum[1] - idxNippleLeft[1] + 1 ) / 100;
  lateralSize[2] = lateralSize[2];

  lateralRegion.SetSize( lateralSize );
  lateralRegion.SetIndex( lateralStart );

  typename PointSetType::Pointer leftChestPointSet  = PointSetType::New();  
  typename PointSetType::Pointer rightChestPointSet = PointSetType::New();  

  // iterate over left breast
  IteratorWithIndexType itChestSurfLeftRegion = IteratorWithIndexType( imChestSurfaceVoxels, lateralRegion );
  int iPointLeftSurf  = 0;

  typename InternalImageType::IndexType idx;
  typename PointSetType::PointType point;

  for ( itChestSurfLeftRegion.GoToBegin(); 
	! itChestSurfLeftRegion.IsAtEnd();
	++ itChestSurfLeftRegion )
  {
    if ( itChestSurfLeftRegion.Get() )
    {
      idx = itChestSurfLeftRegion.GetIndex();
        
      // The 'height' of the chest surface
      surfHeight[0] = static_cast<RealType>( idx[1] ) - rYHeightOffset;

      // Location of this surface point
      point[0] = static_cast<RealType>( idx[0] );
      point[1] = static_cast<RealType>( idx[2] );

      leftChestPointSet->SetPoint( iPointLeftSurf, point );
      leftChestPointSet->SetPointData( iPointLeftSurf, surfHeight );

      ++iPointLeftSurf;
    } 
  }  

  // Fit the B-Spline...
  typename InternalImageType::Pointer 
    imLeftFittedBreastMask 
    = MaskImageFromBSplineFittedSurface( leftChestPointSet, 
					 imSegmented->GetLargestPossibleRegion(), 
					 imStructural->GetOrigin(), 
					 imStructural->GetSpacing(), 
					 imStructural->GetDirection(),
					 rYHeightOffset, 3, 75, 3, false );

  // and now extract surface points of right breast for surface fitting
  lateralRegion = imChestSurfaceVoxels->GetLargestPossibleRegion();

  lateralStart = lateralRegion.GetIndex();
  lateralSize  = lateralRegion.GetSize();

  lateralStart[0] = idxMidSternum[0];
  lateralStart[1] = idxNippleRight[1];
  lateralStart[2] = 0;
    
  lateralSize[0] = lateralSize[0] - idxMidSternum[0];
  lateralSize[1] = positionFraction * ( idxMidSternum[1] - idxNippleRight[1] + 1 ) / 100;

  lateralRegion.SetIndex( lateralStart );
  lateralRegion.SetSize( lateralSize );

  IteratorWithIndexType itChestSurfRightRegion = IteratorWithIndexType( imChestSurfaceVoxels, lateralRegion );
  int iPointRightSurf = 0;

  for ( itChestSurfRightRegion.GoToBegin(); 
	! itChestSurfRightRegion.IsAtEnd();
	++ itChestSurfRightRegion )
  {
    if ( itChestSurfRightRegion.Get() )
    {
      idx = itChestSurfRightRegion.GetIndex();
        
      // The 'height' of the chest surface
      surfHeight[0] = static_cast<RealType>( idx[1] ) - rYHeightOffset;

      // Location of this surface point
      point[0] = static_cast<RealType>( idx[0] );
      point[1] = static_cast<RealType>( idx[2] );

      rightChestPointSet->SetPoint( iPointRightSurf, point );
      rightChestPointSet->SetPointData( iPointRightSurf, surfHeight );

      ++ iPointRightSurf;
    } 
  }

  // Fit B-Spline...

  typename InternalImageType::Pointer 
    imRightFittedBreastMask 
    = MaskImageFromBSplineFittedSurface( rightChestPointSet, 
					 imSegmented->GetLargestPossibleRegion(), 
					 imStructural->GetOrigin(), 
					 imStructural->GetSpacing(), 
					 imStructural->GetDirection(),
					 rYHeightOffset, 3, 75, 3, false );
    
  // Combine the left and right mask into one

  typename MaxImageFilterType::Pointer maxFilter = MaxImageFilterType::New();
  maxFilter->SetInput1( imRightFittedBreastMask );
  maxFilter->SetInput2( imLeftFittedBreastMask  );

  maxFilter->Update();

  WriteBinaryImageToUCharFile( fileOutputFittedBreastMask, 
			       "fitted breast surface mask", 
			       maxFilter->GetOutput(), 
			       flgLeft, flgRight );

  imChestSurfaceVoxels = NULL;

  // Clip imSegmented outside the fitted surfaces...
  IteratorType itImSeg( imSegmented,            
			imSegmented->GetLargestPossibleRegion() );

  IteratorType itImFit( maxFilter->GetOutput(), 
			imSegmented->GetLargestPossibleRegion() );

  for ( itImSeg.GoToBegin(), itImFit.GoToBegin() ; 
	( (! itImSeg.IsAtEnd()) && (! itImFit.IsAtEnd()) )  ; 
	++itImSeg, ++itImFit )
  {
    if ( itImSeg.Get() )
    {
      if ( ! itImFit.Get() )
      {
	itImSeg.Set( 0 );
      }
    }
  }

}


// --------------------------------------------------------------------------
// Mask with a sphere centered on each breast
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::MaskBreastWithSphere( void )
{
  // Left breast

  double leftRadius = DistanceBetweenVoxels( idxLeftBreastMidPoint, idxMidSternum );
  double leftHeight = vcl_fabs( (double) (idxNippleLeft[1] - idxLeftPosterior[1]) );

  if ( leftRadius < leftHeight/2. )
    leftRadius = leftHeight/2.;

  leftRadius *= 1.05;

  typename InternalImageType::RegionType lateralRegion;
  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType lateralSize;

  // Start iterating over the left-hand side

  lateralRegion = imSegmented->GetLargestPossibleRegion();

  lateralStart = lateralRegion.GetIndex();

  lateralSize = lateralRegion.GetSize();
  lateralSize[0] = lateralSize[0]/2;

  lateralRegion.SetSize( lateralSize );

  if ( flgVerbose )
    std::cout << "Iterating over left region: " << lateralRegion << std::endl;

  SliceIteratorType itSegLeftRegion( imSegmented, lateralRegion );
  itSegLeftRegion.SetFirstDirection( 0 );
  itSegLeftRegion.SetSecondDirection( 2 );

  itSegLeftRegion.GoToBegin();

  typename InternalImageType::IndexType idx;
 
  while ( ! itSegLeftRegion.IsAtEnd() ) 
  {
    while ( ! itSegLeftRegion.IsAtEndOfSlice() ) 
    {
      while ( ! itSegLeftRegion.IsAtEndOfLine() )
      {
	if ( itSegLeftRegion.Get() ) {
	  idx = itSegLeftRegion.GetIndex();

	  if ( DistanceBetweenVoxels( idxLeftBreastMidPoint, idx ) > leftRadius )
	    itSegLeftRegion.Set( 0 );
	}
	++itSegLeftRegion; 
      }
      itSegLeftRegion.NextLine();
    }
    itSegLeftRegion.NextSlice(); 
  }

  // Right breast
    
  double rightRadius = DistanceBetweenVoxels( idxRightBreastMidPoint, idxMidSternum );
  double rightHeight = vcl_fabs( (double) (idxNippleRight[1] - idxRightPosterior[1]) );

  if ( rightRadius < rightHeight/2. )
    rightRadius = rightHeight/2.;

  lateralRegion = imSegmented->GetLargestPossibleRegion();

  lateralSize = lateralRegion.GetSize();
  lateralSize[0] = lateralSize[0]/2;

  lateralStart = lateralRegion.GetIndex();
  lateralStart[0] = lateralSize[0];

  lateralRegion.SetIndex( lateralStart );
  lateralRegion.SetSize( lateralSize );

  if ( flgVerbose )
    std::cout << "Iterating over right region: " << lateralRegion << std::endl;

  SliceIteratorType itSegRightRegion( imSegmented, lateralRegion );
  itSegRightRegion.SetFirstDirection( 0 );
  itSegRightRegion.SetSecondDirection( 2 );

  itSegRightRegion.GoToBegin();

  while ( ! itSegRightRegion.IsAtEnd() ) 
  {
    while ( ! itSegRightRegion.IsAtEndOfSlice() ) 
    {
      while ( ! itSegRightRegion.IsAtEndOfLine() )
      {
	if ( itSegRightRegion.Get() ) {
	  idx = itSegRightRegion.GetIndex();

	  if ( DistanceBetweenVoxels( idxRightBreastMidPoint, idx ) > rightRadius )
	    itSegRightRegion.Set( 0 );
	}
	++itSegRightRegion; 
      }
      itSegRightRegion.NextLine();
    }
    itSegRightRegion.NextSlice(); 
  }
}


// --------------------------------------------------------------------------
// Smooth the mask and threshold to round corners etc.
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::SmoothMask( void )
{
  DerivativeFilterPointer derivativeFilterX = DerivativeFilterType::New();
  DerivativeFilterPointer derivativeFilterY = DerivativeFilterType::New();
  DerivativeFilterPointer derivativeFilterZ = DerivativeFilterType::New();
    
  derivativeFilterX->SetSigma( sigmaInMM );
  derivativeFilterY->SetSigma( sigmaInMM );
  derivativeFilterZ->SetSigma( sigmaInMM );

  derivativeFilterX->SetSingleThreadedExecution();
  derivativeFilterY->SetSingleThreadedExecution();
  derivativeFilterZ->SetSingleThreadedExecution();

  derivativeFilterX->SetInput( imSegmented );
  derivativeFilterY->SetInput( derivativeFilterX->GetOutput() );
  derivativeFilterZ->SetInput( derivativeFilterY->GetOutput() );

  derivativeFilterX->SetDirection( 0 );
  derivativeFilterY->SetDirection( 1 );
  derivativeFilterZ->SetDirection( 2 );

  derivativeFilterX->SetOrder( DerivativeFilterType::ZeroOrder );
  derivativeFilterY->SetOrder( DerivativeFilterType::ZeroOrder );
  derivativeFilterZ->SetOrder( DerivativeFilterType::ZeroOrder );


  typename ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
  
  thresholder->SetLowerThreshold( 1000. * finalSegmThreshold );
  thresholder->SetUpperThreshold( 100000 );

  thresholder->SetOutsideValue(  0  );
  thresholder->SetInsideValue( 1000 );

  thresholder->SetInput( derivativeFilterZ->GetOutput() );

  std::cout << "Smoothing the segmented mask" << std::endl;
  thresholder->Update(); 


  // Use this smoothed image to generate a VTK surface?
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if ( fileOutputVTKSurface.length() ) 
  {

    if ( flgRight )
      WriteImageToVTKSurfaceFile( derivativeFilterZ->GetOutput(), 
				  fileOutputVTKSurface,
				  RIGHT_BREAST, flgVerbose, finalSegmThreshold );
    
    if ( flgLeft )
      WriteImageToVTKSurfaceFile( derivativeFilterZ->GetOutput(), 
				  fileOutputVTKSurface,
				  LEFT_BREAST, flgVerbose, finalSegmThreshold );
        
    if ( ! ( flgLeft || flgRight ) )
      WriteImageToVTKSurfaceFile( derivativeFilterZ->GetOutput(), 
				  fileOutputVTKSurface,
				  BOTH_BREASTS, flgVerbose, finalSegmThreshold );
 }


  // Do a final region growing to eliminate any voids inside the breast
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

  connectedThreshold->SetInput( thresholder->GetOutput() );

  connectedThreshold->SetLower( 0  );
  connectedThreshold->SetUpper( 500 );

  connectedThreshold->SetReplaceValue( 1000 );

  typename InternalImageType::IndexType  index;

  index[0] = regGrowXcoord;
  index[1] = regGrowYcoord;
  index[2] = regGrowZcoord;

  connectedThreshold->SetSeed( index );

  std::cout << "Region-growing to eliminate voids in the breast" << std::endl;
  connectedThreshold->Update();
  
  imSegmented = connectedThreshold->GetOutput();
  imSegmented->DisconnectPipeline();

  connectedThreshold = 0;

  // Invert the segmentation

  IteratorType segIterator2( imSegmented, imSegmented->GetLargestPossibleRegion() );
        
  for ( segIterator2.GoToBegin(); ! segIterator2.IsAtEnd(); ++segIterator2 )
    if ( segIterator2.Get() )
      segIterator2.Set( 0 );
    else
      segIterator2.Set( 1000 );

}


// --------------------------------------------------------------------------
// Extract the largest object
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::ExtractLargestObject( enumBreastSideType breastSide )
{
  typename InternalImageType::RegionType lateralRegion;
  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType lateralSize;

  typedef unsigned int LabelPixelType;
  typedef itk::Image<LabelPixelType, ImageDimension > LabelImageType;

  typename InternalImageType::Pointer imLateral;

  // Extract the left breast region

  if ( breastSide == LEFT_BREAST ) 
  {
    typedef itk::RegionOfInterestImageFilter< InternalImageType, InternalImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    lateralRegion = imSegmented->GetLargestPossibleRegion();

    lateralSize = lateralRegion.GetSize();
    lateralSize[0] = lateralSize[0]/2;

    lateralStart = lateralRegion.GetIndex();
    lateralRegion.SetSize( lateralSize );

    filter->SetRegionOfInterest( lateralRegion );
    filter->SetInput( imSegmented );

    filter->Update();

    imLateral = filter->GetOutput();
  }

  // Extract the right breast region

  else if ( breastSide == RIGHT_BREAST ) 
  {
    typedef itk::RegionOfInterestImageFilter< InternalImageType, InternalImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    lateralRegion = imSegmented->GetLargestPossibleRegion();

    lateralSize = lateralRegion.GetSize();
    lateralSize[0] = lateralSize[0]/2;

    lateralStart = lateralRegion.GetIndex();
    lateralStart[0] = lateralSize[0];

    lateralRegion.SetIndex( lateralStart );
    lateralRegion.SetSize( lateralSize );

    filter->SetRegionOfInterest( lateralRegion );
    filter->SetInput( imSegmented );

    filter->Update();

    imLateral = filter->GetOutput();
  }

  std::cout << lateralRegion << std::endl;

  // Detect connected components

  typedef itk::ScalarConnectedComponentImageFilter < InternalImageType, LabelImageType >
    ConnectedComponentImageFilterType;

  typename ConnectedComponentImageFilterType::Pointer connected =
    ConnectedComponentImageFilterType::New ();

  connected->SetInput( imLateral );
  connected->SetDistanceThreshold( 0 );

  // Relabel the object by size

  typedef itk::RelabelComponentImageFilter < LabelImageType, LabelImageType >
    RelabelFilterType;

  typename RelabelFilterType::Pointer relabel = RelabelFilterType::New();

  relabel->SetMinimumObjectSize( 500 );

  relabel->SetInput( connected->GetOutput() );

  try
  {
    relabel->Update(); 

    std::cout << "Number of connected objects: " 
              << relabel->GetNumberOfObjects() << std::endl << std::endl
              << "Size of largest object: " 
              << relabel->GetSizeOfObjectsInPixels()[0] << std::endl << std::endl;
  }
  catch( itk::ExceptionObject & err ) 
  { 
    std::cerr << "Failed: " << err << std::endl; 
    exit( EXIT_FAILURE );
  }                

  // The largest object should be the background: 0, the breasts next: 1 and 2

  typename LabelImageType::IndexType idx;

  typedef itk::ImageRegionIteratorWithIndex< LabelImageType > LabelIteratorType;
  
  LabelIteratorType labIterator( relabel->GetOutput(), 
                                 relabel->GetOutput()->GetLargestPossibleRegion() );
        
  for ( labIterator.GoToBegin(); 
        ! labIterator.IsAtEnd(); 
        ++labIterator )
  {
    idx = labIterator.GetIndex();
    idx[0] += lateralStart[0];

    if ( ( labIterator.Get() == 2 ) )
    {
      imSegmented->SetPixel( idx, 1000 );
    }
    else
    {
      imSegmented->SetPixel( idx, 0 );
    }
  }
}


// --------------------------------------------------------------------------
// DistanceBetweenVoxels()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
double
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::DistanceBetweenVoxels( typename InternalImageType::IndexType p, 
			 typename InternalImageType::IndexType q )
{
  double dx = p[0] - q[0];
  double dy = p[1] - q[1];
  double dz = p[2] - q[2];

  return vcl_sqrt( dx*dx + dy*dy + dz*dz );
}


// --------------------------------------------------------------------------
// ModifySuffix()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
std::string
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::ModifySuffix( std::string filename, std::string strInsertBeforeSuffix ) 
{
  boost::filesystem::path pathname( filename );
  boost::filesystem::path ofilename;

  std::string extension = pathname.extension().string();
  std::string stem = pathname.stem().string();

  if ( extension == std::string( ".gz" ) ) {
    
    extension = pathname.stem().extension().string() + extension;
    stem = pathname.stem().stem().string();
  }
  
  ofilename = pathname.parent_path() /
    boost::filesystem::path( stem + strInsertBeforeSuffix + extension );
  
  return ofilename.string();
}



// --------------------------------------------------------------------------
// GetBreastSide()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
std::string
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::GetBreastSide( std::string &fileOutput, enumBreastSideType breastSide )
{
  std::string fileModifiedOutput;

  // The left breast

  if ( breastSide == LEFT_BREAST ) 
  {
    fileModifiedOutput = ModifySuffix( fileOutput, std::string( "_left" ) );
  }

  // The right breast

  else if ( breastSide == RIGHT_BREAST ) 
  {
    fileModifiedOutput = ModifySuffix( fileOutput, std::string( "_right" ) );
  }

  // Both breasts

  else
  {
    fileModifiedOutput = fileOutput;
  }

  return fileModifiedOutput;
}


// --------------------------------------------------------------------------
// GetBreastSide()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::Pointer 
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::GetBreastSide( typename InternalImageType::Pointer inImage, 
		 enumBreastSideType breastSide )
{
  typename InternalImageType::RegionType lateralRegion;
  typename InternalImageType::IndexType lateralStart;
  typename InternalImageType::SizeType lateralSize;

  typename InternalImageType::Pointer imLateral;
      
  typename InternalImageType::Pointer outImage; 

  // Extract the left breast region

  if ( breastSide == LEFT_BREAST ) 
  {
    typedef itk::RegionOfInterestImageFilter< InternalImageType, InternalImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    lateralRegion = inImage->GetLargestPossibleRegion();

    lateralSize = lateralRegion.GetSize();
    lateralSize[0] = lateralSize[0]/2;

    lateralRegion.SetSize( lateralSize );

    filter->SetRegionOfInterest( lateralRegion );
    filter->SetInput( inImage );

    filter->Update();

    outImage = filter->GetOutput();
  }

  // Extract the right breast region

  else if ( breastSide == RIGHT_BREAST ) 
  {
    typedef itk::RegionOfInterestImageFilter< InternalImageType, InternalImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    lateralRegion = inImage->GetLargestPossibleRegion();

    lateralSize = lateralRegion.GetSize();
    lateralSize[0] = lateralSize[0]/2;

    lateralStart = lateralRegion.GetIndex();
    lateralStart[0] = lateralSize[0];

    lateralRegion.SetIndex( lateralStart );
    lateralRegion.SetSize( lateralSize );


    filter->SetRegionOfInterest( lateralRegion );
    filter->SetInput( inImage );

    filter->Update();

    outImage = filter->GetOutput();
  }

  // Output both breasts

  else
  {
    outImage = inImage;
  }

  return outImage;
}



// --------------------------------------------------------------------------
// WriteImageToFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
bool
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteImageToFile( std::string &fileOutput, const char *description,
		    typename InternalImageType::Pointer image, enumBreastSideType breastSide )
{
  if ( fileOutput.length() ) {

    std::string fileModifiedOutput;
    typename InternalImageType::Pointer pipeITKImageDataConnector;

    pipeITKImageDataConnector = GetBreastSide( image, breastSide );
    fileModifiedOutput = GetBreastSide( fileOutput, breastSide );

    // Write the image

    typedef itk::ImageFileWriter< InternalImageType > FileWriterType;

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( fileModifiedOutput.c_str() );
    writer->SetInput( pipeITKImageDataConnector );

    try
    {
      std::cout << "Writing " << description << " to file: "
                << fileModifiedOutput.c_str() << std::endl;
      writer->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
      exit( EXIT_FAILURE );
    }


    return true;
  }
  else
    return false;
}


// --------------------------------------------------------------------------
// WriteImageToFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
bool
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteImageToFile( std::string &fileOutput,
		    const char *description,
		    typename InternalImageType::Pointer image, 
		    bool flgLeft, bool flgRight )
{
  if ( flgLeft && flgRight )
    return 
      WriteImageToFile( fileOutput, description, image, LEFT_BREAST ) &&
      WriteImageToFile( fileOutput, description, image, RIGHT_BREAST );

  else if ( flgRight )
    return WriteImageToFile( fileOutput, description, image, RIGHT_BREAST );

  else if ( flgLeft )
    return WriteImageToFile( fileOutput, description, image, LEFT_BREAST );

  else
    return WriteImageToFile( fileOutput, description, image, BOTH_BREASTS );
}



// --------------------------------------------------------------------------
// WriteBinaryImageToUCharFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
bool
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteBinaryImageToUCharFile( std::string &fileOutput, 
			       const char *description,
			       typename InternalImageType::Pointer image, 
			       enumBreastSideType breastSide )
{
  if ( fileOutput.length() ) {

    typedef unsigned char OutputPixelType;
    typedef itk::Image< OutputPixelType, ImageDimension> OutputImageType;

    typedef itk::RescaleIntensityImageFilter< InternalImageType, OutputImageType > CastFilterType;
    typedef itk::ImageFileWriter< OutputImageType > FileWriterType;

    std::string fileModifiedOutput;
    typename InternalImageType::Pointer pipeITKImageDataConnector;

    pipeITKImageDataConnector = GetBreastSide( image, breastSide );
    fileModifiedOutput = GetBreastSide( fileOutput, breastSide );


    typename CastFilterType::Pointer caster = CastFilterType::New();

    caster->SetInput( pipeITKImageDataConnector );
    caster->SetOutputMinimum(   0 );
    caster->SetOutputMaximum( 255 );

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( fileModifiedOutput.c_str() );
    writer->SetInput( caster->GetOutput() );

    std::cout << "Writing " << description << " to file: "
	      << fileModifiedOutput.c_str() << std::endl;
    writer->Update();

    return true;
  }
  else
    return false;
}



// --------------------------------------------------------------------------
// WriteBinaryImageToUCharFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
bool
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteBinaryImageToUCharFile( std::string &fileOutput,
			       const char *description,
			       typename InternalImageType::Pointer image, 
			       bool flgLeft, 
			       bool flgRight )
{
  if ( flgLeft && flgRight )
    return 
      WriteBinaryImageToUCharFile( fileOutput, description, image, LEFT_BREAST ) &&
      WriteBinaryImageToUCharFile( fileOutput, description, image, RIGHT_BREAST );

  else if ( flgRight )
    return WriteBinaryImageToUCharFile( fileOutput, description, image, RIGHT_BREAST );

  else if ( flgLeft )
    return WriteBinaryImageToUCharFile( fileOutput, description, image, LEFT_BREAST );

  else
    return WriteBinaryImageToUCharFile( fileOutput, description, image, BOTH_BREASTS );
}



// --------------------------------------------------------------------------
// WriteBinaryImageToUCharFileOrVTKSurfaceFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
bool
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteBinaryImageToUCharFileOrVTKSurfaceFile( std::string &fileOutput,
			       const char *description,
			       typename InternalImageType::Pointer image, 
			       bool flgLeft, 
			       bool flgRight )
{

  boost::filesystem::path pathname( fileOutput );
  std::string extension = pathname.extension().string();

  if ( extension == std::string( ".vtk" ) ) 
  {
    // Save mesh
    if ( flgLeft && flgRight )
    {
      WriteImageToVTKSurfaceFile( image, fileOutput, LEFT_BREAST,  flgVerbose, finalSegmThreshold );
      WriteImageToVTKSurfaceFile( image, fileOutput, RIGHT_BREAST, flgVerbose, finalSegmThreshold );
      return true;
    }
    else if ( flgRight )
    {
      WriteImageToVTKSurfaceFile( image, fileOutput, RIGHT_BREAST, flgVerbose, finalSegmThreshold );
      return true;
    }
    else if ( flgLeft )
    {
      WriteImageToVTKSurfaceFile( image, fileOutput, LEFT_BREAST, flgVerbose, finalSegmThreshold );
      return true;
    }
    else
    {
      WriteImageToVTKSurfaceFile( image, fileOutput, BOTH_BREASTS, flgVerbose, finalSegmThreshold );
      return true;
    }
  }
  else
  {
    // Save as binary image...
    if ( flgLeft && flgRight )
      return 
        WriteBinaryImageToUCharFile( fileOutput, description, image, LEFT_BREAST ) &&
        WriteBinaryImageToUCharFile( fileOutput, description, image, RIGHT_BREAST );
    else if ( flgRight )
      return WriteBinaryImageToUCharFile( fileOutput, description, image, RIGHT_BREAST );

    else if ( flgLeft )
      return WriteBinaryImageToUCharFile( fileOutput, description, image, LEFT_BREAST );

    else
      return WriteBinaryImageToUCharFile( fileOutput, description, image, BOTH_BREASTS );
  }
}




// --------------------------------------------------------------------------
// WriteHistogramToFile()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteHistogramToFile( std::string fileOutput,
			vnl_vector< double > &xHistIntensity, 
			vnl_vector< double > &yHistFrequency, 
			unsigned int nBins )
{
  unsigned int iBin;
  std::fstream fout;

  fout.open(fileOutput.c_str(), std::ios::out);
    
  if ((! fout) || fout.bad()) 
  {
    itkExceptionMacro( << "ERROR: Failed to open file: " << fileOutput.c_str() );
  }
  
  std::cout << "Writing histogram to file: "
	    << fileOutput.c_str() << std::endl;

  for ( iBin=0; iBin<nBins; iBin++ ) 
    fout << xHistIntensity[ iBin ] << " "
	 << yHistFrequency[ iBin ] << std::endl;     

  fout.close();
}  



// ----------------------------------------------------------
// polyDataInfo(vtkPolyData *polyData) 
// ----------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::polyDataInfo(vtkPolyData *polyData) 
{
  if (polyData) {
    std::cout << "   Number of vertices: " 
	 << polyData->GetNumberOfVerts() << std::endl;

    std::cout << "   Number of lines:    " 
	 << polyData->GetNumberOfLines() << std::endl;
    
    std::cout << "   Number of cells:    " 
	 << polyData->GetNumberOfCells() << std::endl;
    
    std::cout << "   Number of polygons: " 
	 << polyData->GetNumberOfPolys() << std::endl;
    
    std::cout << "   Number of strips:   " 
	 << polyData->GetNumberOfStrips() << std::endl;
  }
}  




// ----------------------------------------------------------
// WriteImageToVTKSurfaceFile()
// ----------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::WriteImageToVTKSurfaceFile( typename InternalImageType::Pointer image, 
			      std::string &fileOutput, 
			      enumBreastSideType breastSide,
			      bool flgVerbose, 
			      float finalSegmThreshold ) 
{
  typename InternalImageType::Pointer pipeITKImageDataConnector;
  vtkPolyData *pipeVTKPolyDataConnector;	// The link between objects in the pipeline


  pipeITKImageDataConnector = GetBreastSide( image, breastSide );
  std::string fileModifiedOutput = GetBreastSide( fileOutput, breastSide );

  // Get input image properties
  // Spacing
  const typename InternalImageType::SpacingType 
    &sp = pipeITKImageDataConnector->GetSpacing();
  
  // Origin
  const typename InternalImageType::PointType 
    & originInput = pipeITKImageDataConnector->GetOrigin();

  // Size
  const typename InternalImageType::SizeType 
    &sz = pipeITKImageDataConnector->GetLargestPossibleRegion().GetSize();

  // Print these properties
  std::cout << "Input image origin: " 
    << originInput[0] << ","  << originInput[1] << "," << originInput[2] << std::endl;

  std::cout << "Input image resolution: "
	    << sp[0] << "," << sp[1] << "," << sp[2] << std::endl;

  std::cout << "Input image dimensions: "
	    << sz[0] << "," << sz[1] << "," << sz[2] << std::endl;
  
  // Estimate the image extent as an AABB in physical coordinates
  typename InternalImageType::IndexType idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8;
  idx1[0] = sz[0];    idx1[1] = sz[1];    idx1[2] = sz[2];  // x+ y+ z+
  idx2[0] =     0;    idx2[1] = sz[1];    idx2[2] = sz[2];  // x- y+ z+
  idx3[0] = sz[0];    idx3[1] =     0;    idx3[2] = sz[2];  // x+ y- z+
  idx4[0] =     0;    idx4[1] =     0;    idx4[2] = sz[2];  // x- y- z+
  idx5[0] = sz[0];    idx5[1] = sz[1];    idx5[2] =     0;  // x+ y+ z-
  idx6[0] =     0;    idx6[1] = sz[1];    idx6[2] =     0;  // x- y+ z-
  idx7[0] = sz[0];    idx7[1] =     0;    idx7[2] =     0;  // x+ y- z-
  idx8[0] =     0;    idx8[1] =     0;    idx8[2] =     0;  // x- y- z-

  typename InternalImageType::PointType p1, p2, p3, p4, p5, p6, p7, p8;
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx1, p1 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx2, p2 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx3, p3 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx4, p4 ); 
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx5, p5 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx6, p6 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx7, p7 );
  pipeITKImageDataConnector->TransformIndexToPhysicalPoint( idx8, p8);

  std::cout << " --> coordinate (1): " << p1[0] << ", " << p1[1] << ", "<< p1[2] << std::endl;
  std::cout << " --> coordinate (2): " << p2[0] << ", " << p2[1] << ", "<< p2[2] << std::endl;
  std::cout << " --> coordinate (3): " << p3[0] << ", " << p3[1] << ", "<< p3[2] << std::endl;
  std::cout << " --> coordinate (4): " << p4[0] << ", " << p4[1] << ", "<< p4[2] << std::endl;
  std::cout << " --> coordinate (5): " << p5[0] << ", " << p5[1] << ", "<< p5[2] << std::endl;
  std::cout << " --> coordinate (6): " << p6[0] << ", " << p6[1] << ", "<< p6[2] << std::endl;
  std::cout << " --> coordinate (7): " << p7[0] << ", " << p7[1] << ", "<< p7[2] << std::endl;
  std::cout << " --> coordinate (8): " << p8[0] << ", " << p8[1] << ", "<< p8[2] << std::endl;

  // min x
  double dXMin = p1[0];
  dXMin = dXMin < p2[0] ? dXMin : p2[0];
  dXMin = dXMin < p3[0] ? dXMin : p3[0];
  dXMin = dXMin < p4[0] ? dXMin : p4[0];
  dXMin = dXMin < p5[0] ? dXMin : p5[0];
  dXMin = dXMin < p6[0] ? dXMin : p6[0];
  dXMin = dXMin < p7[0] ? dXMin : p7[0];
  dXMin = dXMin < p8[0] ? dXMin : p8[0];

  // max x
  double dXMax = p1[0];
  dXMax = dXMax > p2[0] ? dXMax : p2[0];
  dXMax = dXMax > p3[0] ? dXMax : p3[0];
  dXMax = dXMax > p4[0] ? dXMax : p4[0];
  dXMax = dXMax > p5[0] ? dXMax : p5[0];
  dXMax = dXMax > p6[0] ? dXMax : p6[0];
  dXMax = dXMax > p7[0] ? dXMax : p7[0];
  dXMax = dXMax > p8[0] ? dXMax : p8[0];

  //min y
  double dYMin = p1[1];
  dYMin = dYMin < p2[1] ? dYMin : p2[1];
  dYMin = dYMin < p3[1] ? dYMin : p3[1];
  dYMin = dYMin < p4[1] ? dYMin : p4[1];
  dYMin = dYMin < p5[1] ? dYMin : p5[1];
  dYMin = dYMin < p6[1] ? dYMin : p6[1];
  dYMin = dYMin < p7[1] ? dYMin : p7[1];
  dYMin = dYMin < p8[1] ? dYMin : p8[1];

  // max y
  double dYMax = p1[1];
  dYMax = dYMax > p2[1] ? dYMax : p2[1];
  dYMax = dYMax > p3[1] ? dYMax : p3[1];
  dYMax = dYMax > p4[1] ? dYMax : p4[1];
  dYMax = dYMax > p5[1] ? dYMax : p5[1];
  dYMax = dYMax > p6[1] ? dYMax : p6[1];
  dYMax = dYMax > p7[1] ? dYMax : p7[1];
  dYMax = dYMax > p8[1] ? dYMax : p8[1];

  //min z
  double dZMin = p1[2];
  dZMin = dZMin < p2[2] ? dZMin : p2[2];
  dZMin = dZMin < p3[2] ? dZMin : p3[2];
  dZMin = dZMin < p4[2] ? dZMin : p4[2];
  dZMin = dZMin < p5[2] ? dZMin : p5[2];
  dZMin = dZMin < p6[2] ? dZMin : p6[2];
  dZMin = dZMin < p7[2] ? dZMin : p7[2];
  dZMin = dZMin < p8[2] ? dZMin : p8[2];

  // max z
  double dZMax = p1[2];
  dZMax = dZMax > p2[2] ? dZMax : p2[2];
  dZMax = dZMax > p3[2] ? dZMax : p3[2];
  dZMax = dZMax > p4[2] ? dZMax : p4[2];
  dZMax = dZMax > p5[2] ? dZMax : p5[2];
  dZMax = dZMax > p6[2] ? dZMax : p6[2];
  dZMax = dZMax > p7[2] ? dZMax : p7[2];
  dZMax = dZMax > p8[2] ? dZMax : p8[2];

  // Think about: Use itk alternative, maybe using itkImageMaskSpatialObject
  std::cout << "Bounding box of image: " << std::endl
    << " x:  " << dXMin << " --> " << dXMax << std::endl
    << " y:  " << dYMin << " --> " << dYMax << std::endl
    << " z:  " << dZMin << " --> " << dZMax << std::endl;


  // Set the border around the image to zero to prevent holes in the mesh
 
  typedef itk::SetBoundaryVoxelsToValueFilter< InternalImageType, 
    InternalImageType > SetBoundaryVoxelsToValueFilterType;
    
  typename SetBoundaryVoxelsToValueFilterType::Pointer 
    setBoundary = SetBoundaryVoxelsToValueFilterType::New();
    
  setBoundary->SetInput( pipeITKImageDataConnector );
    
  setBoundary->SetValue( 0 );
    
  std::cout << "Sealing the image boundary..."<< std::endl;
  setBoundary->Update();

  pipeITKImageDataConnector = setBoundary->GetOutput();


  // Downsample the image to istropic voxels with dimensions

  double subsamplingResolution = 3.0; //The isotropic volume resolution in mm for sub-sampling
  typedef itk::ResampleImageFilter< InternalImageType, InternalImageType > ResampleImageFilterType;
  typename ResampleImageFilterType::Pointer subsampleFilter = ResampleImageFilterType::New();
  
  subsampleFilter->SetInput( pipeITKImageDataConnector );

  double spacing[ ImageDimension ];
  spacing[0] = subsamplingResolution; // pixel spacing in millimeters along X
  spacing[1] = subsamplingResolution; // pixel spacing in millimeters along Y
  spacing[2] = subsamplingResolution; // pixel spacing in millimeters along Z
    
  subsampleFilter->SetOutputSpacing( spacing );

  double origin[ ImageDimension ];
  origin[0] = dXMin-spacing[0]; // X space coordinate of origin (including some safety margin for sealed boundary!)
  origin[1] = dYMin-spacing[1]; // Y space coordinate of origin
  origin[2] = dZMin-spacing[2]; // Y space coordinate of origin

  subsampleFilter->SetOutputOrigin( origin );

  typename InternalImageType::DirectionType direction;
  direction.SetIdentity();
  subsampleFilter->SetOutputDirection( direction );

  typename InternalImageType::SizeType size;

  size[0] = (int) ceil( (dXMax - dXMin + 2 * subsamplingResolution) / spacing[0] ); //(int) ceil( sz[0]*sp[0]/spacing[0] );  // number of pixels along X
  size[1] = (int) ceil( (dYMax - dYMin + 2 * subsamplingResolution) / spacing[1] ); //(int) ceil( sz[1]*sp[1]/spacing[1] );  // number of pixels along X
  size[2] = (int) ceil( (dZMax - dZMin + 2 * subsamplingResolution) / spacing[2] ); //(int) ceil( sz[2]*sp[2]/spacing[2] );  // number of pixels along X

  subsampleFilter->SetSize( size );

  typedef itk::AffineTransform< double, ImageDimension >  TransformType;
  typename TransformType::Pointer transform = TransformType::New();

  subsampleFilter->SetTransform( transform );

  typedef itk::LinearInterpolateImageFunction< InternalImageType, double >  InterpolatorType;
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
 
  subsampleFilter->SetInterpolator( interpolator );
    
  subsampleFilter->SetDefaultPixelValue( 0 );

  std::cout << "Resampling image to dimensions: "
	    << size[0] << ", " << size[1] << ", "<< size[2]
	    << "voxels, with resolution : "
	    << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << "mm..."<< std::endl;
  
  subsampleFilter->Update();

  pipeITKImageDataConnector = subsampleFilter->GetOutput();


  // Create the ITK to VTK filter

  typedef itk::ImageToVTKImageFilter< InternalImageType > ImageToVTKImageFilterType;

  typename ImageToVTKImageFilterType::Pointer convertITKtoVTK = ImageToVTKImageFilterType::New();

  convertITKtoVTK->SetInput( pipeITKImageDataConnector );
    
  if (flgVerbose) 
    std::cout << "Converting the image to VTK format." << std::endl;

  convertITKtoVTK->Update();
    

  // Apply the Marching Cubes algorithm
  
  vtkSmartPointer<vtkMarchingCubes> surfaceExtractor = vtkSmartPointer<vtkMarchingCubes>::New();

  surfaceExtractor->SetValue(0, 1000.*finalSegmThreshold);

  surfaceExtractor->SetInputData( convertITKtoVTK->GetOutput() );
  surfaceExtractor->Update();
  pipeVTKPolyDataConnector = surfaceExtractor->GetOutput();

  if (flgVerbose) {
    std::cout << std::endl << "Extracted surface data:" << std::endl;
    polyDataInfo(pipeVTKPolyDataConnector);
  }

  // Post-extraction smoothing

  int niterations = 5;		// The number of smoothing iterations
  float bandwidth = 0.05;	// The band width of the smoothing filter

  vtkSmartPointer<vtkWindowedSincPolyDataFilter> postSmoothingFilter = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    
  postSmoothingFilter->BoundarySmoothingOff();
    
  postSmoothingFilter->SetNumberOfIterations(niterations);
  postSmoothingFilter->SetPassBand(bandwidth);
    
  postSmoothingFilter->SetInputData( pipeVTKPolyDataConnector );
  postSmoothingFilter->Update();

  pipeVTKPolyDataConnector = postSmoothingFilter->GetOutput();

  // Write the created vtk surface to a file

  vtkSmartPointer<vtkPolyDataWriter> writer3D = vtkSmartPointer<vtkPolyDataWriter>::New();

  writer3D->SetFileName( fileModifiedOutput.c_str() );
  writer3D->SetInputDataObject(pipeVTKPolyDataConnector);

  writer3D->SetFileType(VTK_BINARY);

  writer3D->Write();

  if (flgVerbose) 
    std::cout << "Polydata written to VTK file: " << fileModifiedOutput.c_str() << std::endl;
}



// ------------------------------------------------------------
// Mask image from B-Spline surface
// ------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
typename BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >::InternalImageType::Pointer 
BreastMaskSegmentationFromMRI< ImageDimension, InputPixelType >
::MaskImageFromBSplineFittedSurface( const typename PointSetType::Pointer            & pointSet, 
				     const typename InternalImageType::RegionType    & region,
				     const typename InternalImageType::PointType     & origin, 
				     const typename InternalImageType::SpacingType   & spacing,
				     const typename InternalImageType::DirectionType & direction,
				     const RealType rYHeightOffset, 
				     const int splineOrder, 
				     const RealType controlPointSpacingInMM,
				     const int numOfLevels,
                                     bool correctSurfaceOffest )
{
  
  // Fit the B-Spline surface
  // ~~~~~~~~~~~~~~~~~~~~~~~~
  typedef itk::BSplineScatteredDataPointSetToImageFilter < PointSetType, 
                                                           VectorImageType > FilterType;

  typename FilterType::Pointer filter = FilterType::New();

  filter->SetSplineOrder( splineOrder );  

  typename InternalImageType::SizeType size = region.GetSize();
  RealType numOfControlPoints = ceil( ( static_cast<RealType>( size[0] )*spacing[0] )
                                       /controlPointSpacingInMM );

  if (flgVerbose) 
    std::cout << "Region: " << region << std::endl
              << "PointSet: " << pointSet << std::endl
              << "Origin: " << origin << std::endl
              << "Spacing: " << spacing << std::endl
              << "Number of control points: " << numOfControlPoints 
              << " ( spacing: " << controlPointSpacingInMM << " mm )" << std::endl;

  typename FilterType::ArrayType ncps;  
  ncps.Fill( static_cast<unsigned int>( numOfControlPoints ) );  
  filter->SetNumberOfControlPoints( ncps );

  filter->SetNumberOfLevels( numOfLevels );

  // Define the parametric domain.

  typename FilterType::PointType   bsDomainOrigin;
  typename FilterType::SpacingType bsDomainSpacing;
  typename FilterType::SizeType    bsDomainSize;

  for (int i=0; i<2; i++) 
  {
    bsDomainOrigin[i]  = 0;
    bsDomainSpacing[i] = 1;
  }

  bsDomainSize[0] = size[0];
  bsDomainSize[1] = size[2];

  filter->SetOrigin ( bsDomainOrigin  );
  filter->SetSpacing( bsDomainSpacing );
  filter->SetSize   ( bsDomainSize    );
  filter->SetInput  ( pointSet        );

  filter->Print( std::cout );

  filter->Update();

  // The B-Spline surface heights are the intensities of the 2D output image

  typename VectorImageType::Pointer bSplineSurface = filter->GetOutput();
  bSplineSurface->DisconnectPipeline();

  typename VectorImageType::IndexType bSplineCoord;
  RealType surfaceHeight;


  // Construct the mask
  // ~~~~~~~~~~~~~~~~~~

  // Check if there is a bias between the point set and the fitted surface... if so, correct for it?
  PointsIterator    pointIt     = pointSet->GetPoints()->Begin(); 
  PointDataIterator pointDataIt = pointSet->GetPointData()->Begin();

  PointsIterator    end       = pointSet->GetPoints()->End();
  PointDataIterator ptDataEnd = pointSet->GetPointData()->End();

  RealType rBias = 0.0;

  while( (pointIt != end) && (pointDataIt != ptDataEnd) ) 
  {    
    typename PointSetType::PointType p = pointIt.Value();   // access the point
    
    bSplineCoord[0] = static_cast<typename VectorImageType::IndexValueType>(p[0]);
    bSplineCoord[1] = static_cast<typename VectorImageType::IndexValueType>(p[1]);

    rBias += pointDataIt.Value()[0] - bSplineSurface->GetPixel( bSplineCoord )[0] ;
    
    ++pointIt;
    ++pointDataIt;
  }

  rBias /= pointSet->GetNumberOfPoints();
  
  if ( this->flgVerbose )
  {
    std::cout << "Mean offset between pointset and fitted surface: " << rBias << " [vox]" << std::endl;
    
    if ( correctSurfaceOffest )
      std::cout << "Surface offset will be corrected." << std::endl;
    else
      std::cout << "Surface offset will be ignored." << std::endl;
  }

  // Do not consider the offset 
  if ( ! correctSurfaceOffest )
  {
    rBias = 0.0;
  }

  typename InternalImageType::Pointer imSurfaceMask = InternalImageType::New();
  imSurfaceMask->SetRegions  ( region    );
  imSurfaceMask->SetOrigin   ( origin    );
  imSurfaceMask->SetSpacing  ( spacing   );
  imSurfaceMask->SetDirection( direction );
  imSurfaceMask->Allocate();
  imSurfaceMask->FillBuffer( 0 );

  LineIteratorType itSurfaceMaskLinear( imSurfaceMask, region );

  itSurfaceMaskLinear.SetDirection( 1 );

  for ( itSurfaceMaskLinear.GoToBegin(); 
        ! itSurfaceMaskLinear.IsAtEnd(); 
        itSurfaceMaskLinear.NextLine() )
  {
    itSurfaceMaskLinear.GoToBeginOfLine();

    // Get the coordinate of this column of AP voxels
    
    typename InternalImageType::IndexType idx = itSurfaceMaskLinear.GetIndex();

    bSplineCoord[0] = idx[0];
    bSplineCoord[1] = idx[2];

    // Hence the height (or y coordinate) of the PecSurface surface

    surfaceHeight = bSplineSurface->GetPixel( bSplineCoord )[0];

    while ( ! itSurfaceMaskLinear.IsAtEndOfLine() )
    {
      idx = itSurfaceMaskLinear.GetIndex();

      if ( static_cast<RealType>( idx[1] ) < surfaceHeight + rYHeightOffset + rBias )
        itSurfaceMaskLinear.Set( 0 );
      else
        itSurfaceMaskLinear.Set( 1000 );

      ++itSurfaceMaskLinear;
    }
  }

  return imSurfaceMask;
};
 

} // namespace itk
