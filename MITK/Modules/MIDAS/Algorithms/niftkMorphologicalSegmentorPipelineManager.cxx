/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMorphologicalSegmentorPipelineManager.h"

#include <itkImageDuplicator.h>

#include <mitkDataNode.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkITKImageImport.h>
#include <mitkSegmentationObjectFactory.h>

#include <niftkDataStorageUtils.h>
#include <niftkImageUtils.h>
#include <niftkImageOrientationUtils.h>
#include <niftkITKRegionParametersDataNodeProperty.h>
#include <niftkPaintbrushTool.h>
#include <niftkTool.h>

namespace niftk
{

const std::string MorphologicalSegmentorPipelineManager::PROPERTY_MIDAS_MORPH_SEGMENTATION_FINISHED = "midas.morph.finished";

const std::string MorphologicalSegmentorPipelineManager::SEGMENTATION_OF_LAST_STAGE_NAME = "MORPHO_SEGMENTATION_OF_LAST_STAGE";

//-----------------------------------------------------------------------------
MorphologicalSegmentorPipelineManager::MorphologicalSegmentorPipelineManager()
{
}


//-----------------------------------------------------------------------------
MorphologicalSegmentorPipelineManager::~MorphologicalSegmentorPipelineManager()
{
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (dataStorage != m_DataStorage)
  {
    m_DataStorage = dataStorage;
  }
}


//-----------------------------------------------------------------------------
mitk::DataStorage::Pointer MorphologicalSegmentorPipelineManager::GetDataStorage() const
{
  return m_DataStorage;
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::SetToolManager(mitk::ToolManager::Pointer toolManager)
{
  if (toolManager != m_ToolManager)
  {
    m_ToolManager = toolManager;
  }
}


//-----------------------------------------------------------------------------
mitk::ToolManager::Pointer MorphologicalSegmentorPipelineManager::GetToolManager() const
{
  return m_ToolManager;
}


//-----------------------------------------------------------------------------
mitk::Image::ConstPointer MorphologicalSegmentorPipelineManager::GetReferenceImage() const
{
  mitk::Image::ConstPointer referenceImage = NULL;

  std::vector<mitk::DataNode*> referenceData = this->GetToolManager()->GetReferenceData();
  if (referenceData.size() > 0)
  {
    referenceImage = dynamic_cast<mitk::Image*>(referenceData[0]->GetData());
  }
  return referenceImage;
}


//-----------------------------------------------------------------------------
mitk::Image::Pointer MorphologicalSegmentorPipelineManager::GetWorkingImage(unsigned int dataIndex) const
{
  mitk::Image::Pointer result = NULL;

  std::vector<mitk::DataNode*> workingData = this->GetToolManager()->GetWorkingData();
  if (workingData.size() > dataIndex)
  {
    result = dynamic_cast<mitk::Image*>(workingData[dataIndex]->GetData());
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer MorphologicalSegmentorPipelineManager::GetSegmentationNode() const
{
  return this->GetToolManager()->GetWorkingData(PaintbrushTool::SEGMENTATION);
}


//-----------------------------------------------------------------------------
bool MorphologicalSegmentorPipelineManager::HasSegmentationNode() const
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  return segmentationNode.IsNotNull();
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::OnThresholdingValuesChanged(double lowerThreshold, double upperThreshold, int axialSlicerNumber)
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->SetFloatProperty("midas.morph.thresholding.lower", lowerThreshold);
    segmentationNode->SetFloatProperty("midas.morph.thresholding.upper", upperThreshold);
    segmentationNode->SetIntProperty("midas.morph.thresholding.slice", axialSlicerNumber);
    this->UpdateSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::OnErosionsValuesChanged(double upperThreshold, int numberOfErosions)
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->SetFloatProperty("midas.morph.erosion.threshold", upperThreshold);
    segmentationNode->SetIntProperty("midas.morph.erosion.iterations", numberOfErosions);
    this->UpdateSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::OnDilationsValuesChanged(double lowerPercentage, double upperPercentage, int numberOfDilations)
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->SetFloatProperty("midas.morph.dilation.lower", lowerPercentage);
    segmentationNode->SetFloatProperty("midas.morph.dilation.upper", upperPercentage);
    segmentationNode->SetIntProperty("midas.morph.dilation.iterations", numberOfDilations);
    this->UpdateSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::OnRethresholdingValuesChanged(int boxSize)
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->SetIntProperty("midas.morph.rethresholding.box", boxSize);
    this->UpdateSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::OnTabChanged(int tabIndex)
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->SetIntProperty("midas.morph.stage", tabIndex);
    this->UpdateSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::GetPipelineParamsFromSegmentationNode(MorphologicalSegmentorPipelineParams& params) const
{
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    segmentationNode->GetIntProperty("midas.morph.stage", params.m_Stage);
    segmentationNode->GetFloatProperty("midas.morph.thresholding.lower", params.m_LowerIntensityThreshold);
    segmentationNode->GetFloatProperty("midas.morph.thresholding.upper", params.m_UpperIntensityThreshold);
    segmentationNode->GetIntProperty("midas.morph.thresholding.slice", params.m_AxialCutOffSlice);
    segmentationNode->GetFloatProperty("midas.morph.erosion.threshold", params.m_UpperErosionsThreshold);
    segmentationNode->GetIntProperty("midas.morph.erosion.iterations", params.m_NumberOfErosions);
    segmentationNode->GetFloatProperty("midas.morph.dilation.lower", params.m_LowerPercentageThresholdForDilations);
    segmentationNode->GetFloatProperty("midas.morph.dilation.upper", params.m_UpperPercentageThresholdForDilations);
    segmentationNode->GetIntProperty("midas.morph.dilation.iterations", params.m_NumberOfDilations);
    segmentationNode->GetIntProperty("midas.morph.rethresholding.box", params.m_BoxSize);
  }
}


//-----------------------------------------------------------------------------
mitk::Image::Pointer MorphologicalSegmentorPipelineManager::GetSegmentationImage() const
{
  mitk::Image::Pointer result = NULL;

  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  if (segmentationNode.IsNotNull())
  {
    result = dynamic_cast<mitk::Image*>(segmentationNode->GetData());
  }
  return result;

}


//-----------------------------------------------------------------------------
bool MorphologicalSegmentorPipelineManager::IsNodeASegmentationImage(const mitk::DataNode::Pointer node) const
{
  assert(node);
  std::set<std::string> set;

  bool result = false;

  if (niftk::IsNodeAnUcharBinaryImage(node))
  {
    mitk::DataNode::Pointer parent = niftk::FindFirstParentImage(this->GetDataStorage(), node, false);
    if (parent.IsNotNull())
    {
      // Should also have at least 4 children (see PaintbrushTool)
      mitk::DataStorage::SetOfObjects::Pointer children = niftk::FindDerivedImages(this->GetDataStorage(), node, true);
      for (std::size_t i = 0; i < children->size(); ++i)
      {
        set.insert(children->at(i)->GetName());
      }
      if (set.find(PaintbrushTool::EROSIONS_SUBTRACTIONS_NAME) != set.end()
          && set.find(PaintbrushTool::EROSIONS_ADDITIONS_NAME) != set.end()
          && set.find(PaintbrushTool::DILATIONS_SUBTRACTIONS_NAME) != set.end()
          && set.find(PaintbrushTool::DILATIONS_ADDITIONS_NAME) != set.end())
      {
        result = true;
      }
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
bool MorphologicalSegmentorPipelineManager::IsNodeAWorkingImage(const mitk::DataNode::Pointer node) const
{
  assert(node);
  bool result = false;

  if (niftk::IsNodeAnUcharBinaryImage(node))
  {
    mitk::DataNode::Pointer parent = niftk::FindFirstParentImage(this->GetDataStorage(), node, true);
    if (parent.IsNotNull())
    {
      std::string name;
      if (node->GetStringProperty("name", name))
      {
        if (   name == PaintbrushTool::EROSIONS_SUBTRACTIONS_NAME
            || name == PaintbrushTool::EROSIONS_ADDITIONS_NAME
            || name == PaintbrushTool::DILATIONS_SUBTRACTIONS_NAME
            || name == PaintbrushTool::DILATIONS_ADDITIONS_NAME
            )
        {
          result = true;
        }
      }
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
std::vector<mitk::DataNode*> MorphologicalSegmentorPipelineManager::GetWorkingDataFromSegmentationNode(const mitk::DataNode::Pointer segmentationNode) const
{
  assert(segmentationNode.IsNotNull());

  std::vector<mitk::DataNode*> workingData(5);
  std::fill(workingData.begin(), workingData.end(), (mitk::DataNode*) 0);

  workingData[PaintbrushTool::SEGMENTATION] = segmentationNode;

  mitk::DataStorage::SetOfObjects::Pointer derivedNodes = niftk::FindDerivedImages(this->GetDataStorage(), segmentationNode, true );

  for (std::size_t i = 0; i < derivedNodes->size(); i++)
  {
    mitk::DataNode::Pointer derivedNode = derivedNodes->at(i);
    std::string name = derivedNode->GetName();
    if (name == PaintbrushTool::EROSIONS_ADDITIONS_NAME)
    {
      workingData[PaintbrushTool::EROSIONS_ADDITIONS] = derivedNode;
    }
    else if (name == PaintbrushTool::EROSIONS_SUBTRACTIONS_NAME)
    {
      workingData[PaintbrushTool::EROSIONS_SUBTRACTIONS] = derivedNode;
    }
    else if (name == PaintbrushTool::DILATIONS_ADDITIONS_NAME)
    {
      workingData[PaintbrushTool::DILATIONS_ADDITIONS] = derivedNode;
    }
    else if (name == PaintbrushTool::DILATIONS_SUBTRACTIONS_NAME)
    {
      workingData[PaintbrushTool::DILATIONS_SUBTRACTIONS] = derivedNode;
    }
  }

  if (std::count(workingData.begin(), workingData.end(), (mitk::DataNode*) 0) != 0)
  {
    MITK_INFO << "Working data nodes missing for the morphological segmentation pipeline.";
    workingData.clear();
  }

  return workingData;
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::SetSegmentationNodePropsFromReferenceImage()
{
  mitk::Image::ConstPointer referenceImage = this->GetReferenceImage();

  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();

  if(referenceImage.IsNotNull() && segmentationNode.IsNotNull())
  {
    int thresholdingSlice = 0;
    int upDirection = GetUpDirection(referenceImage, IMAGE_ORIENTATION_AXIAL);
    if (upDirection == -1)
    {
      int axialAxis = GetThroughPlaneAxis(referenceImage, IMAGE_ORIENTATION_AXIAL);
      thresholdingSlice = referenceImage->GetDimension(axialAxis) - 1;
    }

    segmentationNode->SetIntProperty("midas.morph.stage", 0);
    segmentationNode->SetFloatProperty("midas.morph.thresholding.lower", referenceImage->GetStatistics()->GetScalarValueMin());
    segmentationNode->SetFloatProperty("midas.morph.thresholding.upper", referenceImage->GetStatistics()->GetScalarValueMin());
    segmentationNode->SetIntProperty("midas.morph.thresholding.slice", thresholdingSlice);
    segmentationNode->SetFloatProperty("midas.morph.erosion.threshold", referenceImage->GetStatistics()->GetScalarValueMax());
    segmentationNode->SetIntProperty("midas.morph.erosion.iterations", 0);
    segmentationNode->SetFloatProperty("midas.morph.dilation.lower", 60);
    segmentationNode->SetFloatProperty("midas.morph.dilation.upper", 160);
    segmentationNode->SetIntProperty("midas.morph.dilation.iterations", 0);
    segmentationNode->SetIntProperty("midas.morph.rethresholding.box", 0);
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::UpdateSegmentation()
{
  mitk::DataNode::Pointer referenceNode = this->GetToolManager()->GetReferenceData(0);
  mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
  // The grey scale image:
  mitk::Image::ConstPointer referenceImage = this->GetReferenceImage();
  // The working images:
  mitk::Image::ConstPointer erosionsAdditions = this->GetWorkingImage(PaintbrushTool::EROSIONS_ADDITIONS).GetPointer();
  mitk::Image::ConstPointer erosionsSubtractions = this->GetWorkingImage(PaintbrushTool::EROSIONS_SUBTRACTIONS).GetPointer();
  mitk::Image::ConstPointer dilationsAdditions = this->GetWorkingImage(PaintbrushTool::DILATIONS_ADDITIONS).GetPointer();
  mitk::Image::ConstPointer dilationsSubtractions = this->GetWorkingImage(PaintbrushTool::DILATIONS_SUBTRACTIONS).GetPointer();
  // The output image:
  mitk::Image::Pointer segmentationImage = this->GetSegmentationImage();

  if (referenceNode.IsNotNull()
      && segmentationNode.IsNotNull()
      && referenceImage.IsNotNull()
      && segmentationImage.IsNotNull()
      && erosionsAdditions.IsNotNull()
      && erosionsSubtractions.IsNotNull()
      && dilationsAdditions.IsNotNull()
      && dilationsSubtractions.IsNotNull()
      )
  {
    MorphologicalSegmentorPipelineParams params;
    this->GetPipelineParamsFromSegmentationNode(params);

    typedef itk::Image<unsigned char, 3> SegmentationImageType;
    typedef mitk::ImageToItk<SegmentationImageType> ImageToItkType;

    /// Note:
    /// We pass NULLs to the pipeline for the inputs that have not been changed.
    /// This is to avoid unnecessary conversions.

    std::vector<SegmentationImageType::ConstPointer> workingImages(5);
    std::fill(workingImages.begin(), workingImages.end(), (SegmentationImageType*)0);

    if (!m_Pipelines[segmentationImage])
    {
      /// Note:
      ///
      /// We have to set the IgnoreLock option on the read accessors here, so that the
      /// paintbrush tool can apply its write lock on the same images. Here, we keep
      /// smart pointers to these images throughout the life of the pipeline. Releasing
      /// the smart pointers would result in releasing the read lock (and therefore no
      /// conflict with the paintbrush tool) but it would also mean that we need to
      /// re-connect the pipeline over and over again and we would need to re-execute
      /// filters even if their input images have not changed. (The whole pipeline
      /// approach would become senseless.)
      ///
      /// As we are bypassing the locking mechanism, we must ensure it by the GUI logic
      /// that the paintbrush interactor and the pipeline update are not working at the
      /// same time.

      ImageToItkType::Pointer mitkToItk;

      mitkToItk = ImageToItkType::New();
      mitkToItk->SetOptions(mitk::ImageAccessorBase::IgnoreLock);
      mitkToItk->SetInput(erosionsAdditions);
      mitkToItk->Update();
      workingImages[PaintbrushTool::EROSIONS_ADDITIONS] = mitkToItk->GetOutput();

      mitkToItk = ImageToItkType::New();
      mitkToItk->SetOptions(mitk::ImageAccessorBase::IgnoreLock);
      mitkToItk->SetInput(erosionsSubtractions);
      mitkToItk->Update();
      workingImages[PaintbrushTool::EROSIONS_SUBTRACTIONS] = mitkToItk->GetOutput();

      mitkToItk = ImageToItkType::New();
      mitkToItk->SetOptions(mitk::ImageAccessorBase::IgnoreLock);
      mitkToItk->SetInput(dilationsAdditions);
      mitkToItk->Update();
      workingImages[PaintbrushTool::DILATIONS_ADDITIONS] = mitkToItk->GetOutput();

      mitkToItk = ImageToItkType::New();
      mitkToItk->SetOptions(mitk::ImageAccessorBase::IgnoreLock);
      mitkToItk->SetInput(dilationsSubtractions);
      mitkToItk->Update();
      workingImages[PaintbrushTool::DILATIONS_SUBTRACTIONS] = mitkToItk->GetOutput();
    }

    std::vector<int> region(6);
    std::vector<bool> editingFlags(4);

    std::vector<mitk::DataNode*> workingData = this->GetToolManager()->GetWorkingData();

    /// This assumes that the working nodes with the additions and subtractions are from index 1 to 4.
    /// The pipeline assumes that the editing flags are indexed from 0 to 3. In the past the two indices
    /// used to match, but for consistency with the irregular editor and compatibility with the image
    /// selector widget, the segmentation image also needs to be stored in the working data vector with
    /// index 0. Hence, there is 1 difference between the two indices.
    for (unsigned i = 0; i < 4; ++i)
    {
      auto editingProperty = dynamic_cast<ITKRegionParametersDataNodeProperty*>(workingData[i + 1]->GetProperty(PaintbrushTool::REGION_PROPERTY_NAME.c_str()));

      bool isEditing = editingProperty && editingProperty->IsValid();
      if (isEditing)
      {
        region = editingProperty->GetITKRegionParameters();
      }

      editingFlags[i] = isEditing;
    }

    bool isRestarting = false;
    bool foundRestartingFlag = segmentationNode->GetBoolProperty("midas.morph.restarting", isRestarting);

    try
    {
      AccessFixedDimensionByItk_n(
          referenceImage.GetPointer(),
          InvokeITKPipeline,
          3,
          (workingImages, params, region, editingFlags, isRestarting, segmentationImage));
    }
    catch (const mitk::AccessByItkException& e)
    {
      MITK_ERROR << "Caught exception, so abandoning pipeline update:" << e.what();
    }
    catch (itk::ExceptionObject& e)
    {
      MITK_ERROR << "Caught exception, so abandoning pipeline update:" << e.what();
    }

    if (foundRestartingFlag)
    {
      referenceNode->SetBoolProperty("midas.morph.restarting", false);
    }

    segmentationImage->Modified();
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::FinalizeSegmentation()
{
    mitk::DataNode::Pointer segmentationNode = this->GetSegmentationNode();
    if (segmentationNode.IsNotNull())
    {
      mitk::Image::Pointer segmentationImage = this->GetSegmentationImage();
      mitk::Image::ConstPointer referenceImage = this->GetReferenceImage();

      try
      {
        AccessFixedDimensionByItk_n(referenceImage, FinalizeITKPipeline, 3, (segmentationImage));
      }
      catch (const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "Caught exception, so finalize pipeline" << e.what();
      }
      this->RemoveWorkingNodes();
      this->DestroyPipeline(segmentationImage);

      segmentationNode->SetBoolProperty(MorphologicalSegmentorPipelineManager::PROPERTY_MIDAS_MORPH_SEGMENTATION_FINISHED.c_str(), true);

      UpdateVolumeProperty(segmentationImage, segmentationNode);
    }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::ClearWorkingData()
{
  for (unsigned int i = 1; i < 5; i++)
  {
    mitk::Image::Pointer image = this->GetWorkingImage(i);
    mitk::DataNode::Pointer node = this->GetToolManager()->GetWorkingData(i);

    if (image.IsNotNull() && node.IsNotNull())
    {
      try
      {
        AccessFixedDimensionByItk(image, ClearITKImage, 3);

        image->Modified();
        node->Modified();
      }
      catch (const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "MorphologicalSegmentorPipelineManager::ClearWorkingData: i=" << i << ", caught exception, so abandoning clearing the segmentation image:" << e.what();
      }
    }
  }
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::RemoveWorkingNodes()
{
  mitk::ToolManager* toolManager = this->GetToolManager();

  std::vector<mitk::DataNode*> workingData = toolManager->GetWorkingData();

  std::vector<mitk::DataNode*> emptyWorkingDataArray(0);
  toolManager->SetWorkingData(emptyWorkingDataArray);

  for (unsigned i = 1; i < workingData.size(); ++i)
  {
    this->GetDataStorage()->Remove(workingData[i]);
  }

  toolManager->ActivateTool(-1);
}


//-----------------------------------------------------------------------------
void MorphologicalSegmentorPipelineManager::DestroyPipeline(mitk::Image::Pointer segmentation)
{
  std::map<mitk::Image::Pointer, MorphologicalSegmentorPipelineInterface*>::iterator iter = m_Pipelines.find(segmentation);

  // By the time this method is called, the pipeline MUST exist.
  assert(iter != m_Pipelines.end());
  assert(iter->second);

  delete iter->second;

  m_Pipelines.erase(iter);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipelineManager
::InvokeITKPipeline(
    const itk::Image<TPixel, VImageDimension>* referenceImage,
    const std::vector<typename itk::Image<unsigned char, VImageDimension>::ConstPointer>& workingImages,
    MorphologicalSegmentorPipelineParams& params,
    const std::vector<int>& editingRegion,
    const std::vector<bool>& editingFlags,
    bool isRestarting,
    mitk::Image::Pointer segmentation
    )
{
  typedef itk::Image<TPixel, VImageDimension> GreyScaleImageType;
  typedef itk::Image<unsigned char, VImageDimension> SegmentationImageType;

  const SegmentationImageType* erosionsAdditions = workingImages[PaintbrushTool::EROSIONS_ADDITIONS];
  const SegmentationImageType* erosionsSubtractions = workingImages[PaintbrushTool::EROSIONS_SUBTRACTIONS];
  const SegmentationImageType* dilationsAdditions = workingImages[PaintbrushTool::DILATIONS_ADDITIONS];
  const SegmentationImageType* dilationsSubtractions = workingImages[PaintbrushTool::DILATIONS_SUBTRACTIONS];

  typedef MorphologicalSegmentorPipeline<TPixel, VImageDimension> Pipeline;
  Pipeline* pipeline = dynamic_cast<Pipeline*>(m_Pipelines[segmentation]);

  if (!pipeline)
  {
    pipeline = new Pipeline();
    m_Pipelines[segmentation] = pipeline;

    // Set most of the parameters on the pipeline.
    pipeline->SetInputs(referenceImage,
                        erosionsAdditions,
                        erosionsSubtractions,
                        dilationsAdditions,
                        dilationsSubtractions);
  }

  // Set most of the parameters on the pipeline.
  pipeline->SetParams(params);

  // Do the update.
  pipeline->Update(editingFlags, editingRegion);

  if (isRestarting)
  {
    for (int i = 0; i <= params.m_Stage; i++)
    {
      params.m_Stage = i;

      pipeline->SetParams(params);

      pipeline->Update(editingFlags, editingRegion);
    }
  }
  else
  {
    pipeline->Update(editingFlags, editingRegion);
  }

  // Get hold of the output, and make sure we don't re-allocate memory.
  segmentation->InitializeByItk<SegmentationImageType>(pipeline->GetOutput().GetPointer());
  segmentation->SetImportChannel(pipeline->GetOutput()->GetBufferPointer(), 0, mitk::Image::ReferenceMemory);
}


//-----------------------------------------------------------------------------
//template<typename TPixel, unsigned int VImageDimension>
//void
//MorphologicalSegmentorPipelineManager
//::SetErosionSubtractionsInput(mitk::Image::ConstPointer erosionSubtractions,
//    mitk::Image::Pointer segmentation
//    )
//{
//  typedef MorphologicalSegmentorPipeline<TPixel, VImageDimension> Pipeline;
//  Pipeline* pipeline = dynamic_cast<Pipeline*>(m_Pipelines[segmentation]);
//  assert(pipeline);

//  // Set most of the parameters on the pipeline.
//  pipeline->SetErosionSubtractionsInput(erosionSubtractions);
//}


//-----------------------------------------------------------------------------
//template<typename TPixel, unsigned int VImageDimension>
//void
//MorphologicalSegmentorPipelineManager
//::SetDilationSubtractionsInput(const SegmentationImageType* dilationSubtractions,
//    mitk::Image::Pointer segmentation
//    )
//{
//  typedef MorphologicalSegmentorPipeline<TPixel, VImageDimension> Pipeline;
//  Pipeline* pipeline = dynamic_cast<Pipeline*>(m_Pipelines[segmentation]);
//  assert(pipeline);

//  // Set most of the parameters on the pipeline.
//  pipeline->SetDilationSubtractionsInput(dilationSubtractions);
//}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipelineManager
::FinalizeITKPipeline(
    const itk::Image<TPixel, VImageDimension>* referenceImage,
    mitk::Image::Pointer segmentation
    )
{
  typedef itk::Image<unsigned char, VImageDimension> ImageType;
  typedef MorphologicalSegmentorPipeline<TPixel, VImageDimension> Pipeline;

  Pipeline* pipeline = dynamic_cast<Pipeline*>(m_Pipelines[segmentation]);

  // By the time this method is called, the pipeline MUST exist.
  assert(pipeline);

  // This deliberately re-allocates the memory
  mitk::CastToMitkImage(pipeline->GetOutput(), segmentation);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipelineManager
::ClearITKImage(itk::Image<TPixel, VImageDimension>* itkImage)
{
  itkImage->FillBuffer(0);
}


}
