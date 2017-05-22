/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkIGITracker.h"
#include <niftkMITKMathsUtils.h>

#include <mitkException.h>
#include <mitkNavigationToolStorageDeserializer.h>

namespace niftk
{

//-----------------------------------------------------------------------------
IGITracker::IGITracker(mitk::DataStorage::Pointer dataStorage,
                       std::string toolConfigFileName,
                       int preferredFramesPerSecond
                       )
: m_DataStorage(dataStorage)
, m_ToolConfigFileName(toolConfigFileName)
, m_PreferredFramesPerSecond(preferredFramesPerSecond)
, m_TrackingVolumeNode(nullptr)
{
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "DataStorage is NULL";
  }
  if (m_ToolConfigFileName.size() == 0)
  {
    mitkThrow() << "Empty file name for tracker tool configuration";
  }

  // Load configuration for tracker tools (e.g. pointer, laparoscope etc) from external file.
  mitk::NavigationToolStorageDeserializer::Pointer deserializer = mitk::NavigationToolStorageDeserializer::New(m_DataStorage);
  m_NavigationToolStorage = deserializer->Deserialize(m_ToolConfigFileName);
  if(m_NavigationToolStorage->isEmpty())
  {
    std::string errorMessage = std::string("Failed to load tracker tool configuration:") + deserializer->GetErrorMessage();
    mitkThrow() << errorMessage;
  }
  if (m_NavigationToolStorage->GetToolCount() < 1)
  {
    mitkThrow() << "No tracker tools available";
  }

  // For polaris at least, the expected number of frames per second depends on number of tools.
  // For Aurora, Im not sure. But this is only used for the late/not-late indicator really.
  m_PreferredFramesPerSecond = m_PreferredFramesPerSecond / m_NavigationToolStorage->GetToolCount();
  MITK_INFO << "Setting tracker to expect " << m_PreferredFramesPerSecond << " frames per second.";
}


//-----------------------------------------------------------------------------
IGITracker::~IGITracker()
{
  if (m_TrackingVolumeNode.IsNotNull() && m_DataStorage->Exists(m_TrackingVolumeNode))
  {
    m_DataStorage->Remove(m_TrackingVolumeNode);
  }
}


//-----------------------------------------------------------------------------
void IGITracker::SetVisibilityOfTrackingVolume(bool isVisible)
{
  if (m_TrackingVolumeNode.IsNotNull())
  {
    m_TrackingVolumeNode->SetBoolProperty("visible", isVisible);
  }
}


//-----------------------------------------------------------------------------
bool IGITracker::GetVisibilityOfTrackingVolume() const
{
  bool result = false;
  if (m_TrackingVolumeNode.IsNotNull())
  {
    m_TrackingVolumeNode->GetBoolProperty("visible", result);
  }
  return result;
}


//-----------------------------------------------------------------------------
std::map<std::string, vtkSmartPointer<vtkMatrix4x4> > IGITracker::GetTrackingDataAsMatrix()
{
  std::map<std::string, vtkSmartPointer<vtkMatrix4x4> > result;
  std::map<std::string, std::pair<mitk::Point4D, mitk::Vector3D> > data = this->GetTrackingData();
  if (data.size() > 0)
  {

    std::map<std::string, std::pair<mitk::Point4D, mitk::Vector3D> >::const_iterator iter;
    for (iter = data.begin(); iter != data.end(); iter++)
    {
      vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
      niftk::ConvertRotationAndTranslationToMatrix((*iter).second.first, (*iter).second.second, *mat);
      result.insert(std::pair<std::string, vtkSmartPointer<vtkMatrix4x4> >((*iter).first, mat));
    }
  }
  return result;
}

} // end namespace
