/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMITKTrackerDataSourceService.h"
#include "niftkIGITrackerDataType.h"
#include <niftkIGIDataSourceUtils.h>
#include <niftkCoordinateAxesData.h>
#include <niftkFileIOUtils.h>

#include <mitkExceptionMacro.h>

#include <QDir>
#include <QMutexLocker>

namespace niftk
{

//-----------------------------------------------------------------------------
niftk::IGIDataSourceLocker MITKTrackerDataSourceService::s_Lock;

//-----------------------------------------------------------------------------
MITKTrackerDataSourceService::MITKTrackerDataSourceService(
    QString name,
    QString factoryName,
    const IGIDataSourceProperties& properties,
    mitk::DataStorage::Pointer dataStorage,
    niftk::NDITracker::Pointer tracker
    )
: IGIDataSource((name + QString("-") + QString::number(s_Lock.GetNextSourceNumber())).toStdString(),
                factoryName.toStdString(),
                dataStorage)
, m_Lock(QMutex::Recursive)
, m_FrameId(0)
, m_BackgroundDeleteThread(NULL)
, m_DataGrabbingThread(NULL)
, m_Lag(0)
, m_Tracker(tracker)
{
  if (m_Tracker.IsNull())
  {
    mitkThrow() << "Tracker is NULL!";
  }

  this->SetStatus("Initialising");

  QString deviceName = this->GetName();
  m_TrackerNumber = (deviceName.remove(0, name.length() + 1)).toInt();

  // Set the interval based on desired number of frames per second.
  // eg. 25 fps = 40 milliseconds.
  // However: If system slows down (eg. saving images), then Qt will
  // drop clock ticks, so in effect, you will get less than this.
  int defaultFramesPerSecond = m_Tracker->GetPreferredFramesPerSecond();
  int intervalInMilliseconds = 1000 / defaultFramesPerSecond;

  this->SetTimeStampTolerance(intervalInMilliseconds*1000000*5);
  this->SetProperties(properties);
  this->SetShouldUpdate(true);

  m_BackgroundDeleteThread = new niftk::IGIDataSourceBackgroundDeleteThread(NULL, this);
  m_BackgroundDeleteThread->SetInterval(1000); // try deleting data every 1 second.
  m_BackgroundDeleteThread->start();
  if (!m_BackgroundDeleteThread->isRunning())
  {
    mitkThrow() << "Failed to start background deleting thread";
  }

  m_DataGrabbingThread = new niftk::IGIDataSourceGrabbingThread(NULL, this);
  m_DataGrabbingThread->SetInterval(intervalInMilliseconds);
  m_DataGrabbingThread->start();
  if (!m_DataGrabbingThread->isRunning())
  {
    mitkThrow() << "Failed to start data grabbing thread";
  }

  this->SetDescription("MITK Tracker:" + this->GetName());
  this->SetStatus("Initialised");
  this->Modified();
}


//-----------------------------------------------------------------------------
MITKTrackerDataSourceService::~MITKTrackerDataSourceService()
{
  m_DataGrabbingThread->ForciblyStop();
  delete m_DataGrabbingThread;

  m_BackgroundDeleteThread->ForciblyStop();
  delete m_BackgroundDeleteThread;

  s_Lock.RemoveSource(m_TrackerNumber);
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::SetProperties(const IGIDataSourceProperties& properties)
{
  // In contrast say, to the OpenCV source, we don't set the lag
  // directly on the buffer because, there may be no buffers present
  // at the time this method is called. For example, you could
  // have created a tracker, and no tracked objects are placed within
  // the field of view, thereby no tracking matrices would be generated.
  if (properties.contains("lag"))
  {
    int milliseconds = (properties.value("lag")).toInt();
    m_Lag = milliseconds;

    MITK_INFO << "MITKTrackerDataSourceService(" << this->GetName().toStdString()
              << "): set lag to " << milliseconds << " ms.";
  }
}


//-----------------------------------------------------------------------------
IGIDataSourceProperties MITKTrackerDataSourceService::GetProperties() const
{
  IGIDataSourceProperties props;
  props.insert("lag", m_Lag);

  MITK_INFO << "MITKTrackerDataSourceService:(" << this->GetName().toStdString()
            << "): Retrieved current value of lag as " << m_Lag << " ms.";

  return props;
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::CleanBuffer()
{
  QMutexLocker locker(&m_Lock);

  // Buffer itself should be threadsafe. Clean all buffers.
  QMap<QString, niftk::IGIDataSourceBuffer::Pointer>::iterator iter;
  for (iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
  {
    (*iter)->CleanBuffer();
  }
}


//-----------------------------------------------------------------------------
QMap<QString, std::set<niftk::IGIDataType::IGITimeType> >
MITKTrackerDataSourceService::GetPlaybackIndex(QString directory)
{
  QMap<QString, std::set<niftk::IGIDataType::IGITimeType> > bufferToTimeStamp;
  QMap<QString, QHash<niftk::IGIDataType::IGITimeType, QStringList> > bufferToTimeStampToFileNames;

  niftk::GetPlaybackIndex(directory, QString(".txt"), bufferToTimeStamp, bufferToTimeStampToFileNames);
  return bufferToTimeStamp;
}


//-----------------------------------------------------------------------------
bool MITKTrackerDataSourceService::ProbeRecordedData(niftk::IGIDataType::IGITimeType* firstTimeStampInStore,
                                                     niftk::IGIDataType::IGITimeType* lastTimeStampInStore)
{
  QString path = this->GetPlaybackDirectory();
  return niftk::ProbeRecordedData(path, QString(".txt"), firstTimeStampInStore, lastTimeStampInStore);
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::StartPlayback(niftk::IGIDataType::IGITimeType firstTimeStamp,
                                                 niftk::IGIDataType::IGITimeType lastTimeStamp)
{
  QMutexLocker locker(&m_Lock);

  IGIDataSource::StartPlayback(firstTimeStamp, lastTimeStamp);

  m_Buffers.clear();
  m_PlaybackIndex = this->GetPlaybackIndex(this->GetPlaybackDirectory());
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::StopPlayback()
{
  QMutexLocker locker(&m_Lock);

  m_PlaybackIndex.clear();
  m_Buffers.clear();

  IGIDataSource::StopPlayback();
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::PlaybackData(niftk::IGIDataType::IGITimeType requestedTimeStamp)
{
  assert(this->GetIsPlayingBack());
  assert(m_PlaybackIndex.size() > 0); // Should have failed probing if no data.

  // This will find us the timestamp right after the requested one.
  // Remember we have multiple buffers!
  QMap<QString, std::set<niftk::IGIDataType::IGITimeType> >::iterator playbackIter;
  for(playbackIter = m_PlaybackIndex.begin(); playbackIter != m_PlaybackIndex.end(); ++playbackIter)
  {
    QString bufferName = playbackIter.key();

    std::set<niftk::IGIDataType::IGITimeType>::const_iterator i =
      m_PlaybackIndex[bufferName].upper_bound(requestedTimeStamp);

    if (i != m_PlaybackIndex[bufferName].begin())
    {
      --i;
    }
    if (i != m_PlaybackIndex[bufferName].end())
    {
      if (!m_Buffers.contains(bufferName))
      {
        niftk::IGIDataSourceBuffer::Pointer newBuffer =
          niftk::IGIDataSourceBuffer::New(m_Tracker->GetPreferredFramesPerSecond() * 2);

        newBuffer->SetLagInMilliseconds(m_Lag);
        m_Buffers.insert(bufferName, newBuffer);
      }

      if (!m_Buffers[bufferName]->Contains(*i))
      {
        std::ostringstream  filename;
        filename << this->GetPlaybackDirectory().toStdString()
                 << niftk::GetPreferredSlash().toStdString()
                 << bufferName.toStdString()
                 << niftk::GetPreferredSlash().toStdString()
                 << (*i)
                 << ".txt";

        std::ifstream   file(filename.str().c_str());
        if (file)
        {
          vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
          matrix->Identity();

          for (int r = 0; r < 4; ++r)
          {
            for (int c = 0; c < 4; ++c)
            {
              double tmp;
              file >> tmp;
              matrix->SetElement(r,c,tmp);
            }
          }

          niftk::IGITrackerDataType::Pointer wrapper = niftk::IGITrackerDataType::New();
          wrapper->SetTimeStampInNanoSeconds(*i);
          wrapper->SetTrackingData(matrix);
          wrapper->SetFrameId(m_FrameId++);
          wrapper->SetDuration(this->GetTimeStampTolerance()); // nanoseconds
          wrapper->SetShouldBeSaved(false);

          // Buffer itself should be threadsafe, so I'm not locking anything here.
          m_Buffers[bufferName]->AddToBuffer(wrapper.GetPointer());

        } // end if file open
      } // end if item not already in buffer
    } // end: if we found a valid item to playback

    this->SetStatus("Playing back");

  } // end: for each buffer in playback index
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::GrabData()
{
  {
    QMutexLocker locker(&m_Lock);

    if (this->GetIsPlayingBack())
    {
      return;
    }
  }
  if (m_Tracker.IsNull())
  {
    mitkThrow() << "Tracker is null. This should not happen! It's a programming bug.";
  }

  niftk::IGIDataType::IGITimeType timeCreated = this->GetTimeStampInNanoseconds();

  std::map<std::string, vtkSmartPointer<vtkMatrix4x4> > result = m_Tracker->GetTrackingData();
  if (!result.empty())
  {
    std::map<std::string, vtkSmartPointer<vtkMatrix4x4> >::iterator iter;
    for (iter = result.begin(); iter != result.end(); ++iter)
    {
      std::string toolName = (*iter).first;
      QString toolNameAsQString = QString::fromStdString(toolName);

      niftk::IGITrackerDataType::Pointer wrapper = niftk::IGITrackerDataType::New();
      wrapper->SetToolName(toolName);
      wrapper->SetTrackingData((*iter).second);
      wrapper->SetTimeStampInNanoSeconds(timeCreated);
      wrapper->SetFrameId(m_FrameId++);
      wrapper->SetDuration(this->GetTimeStampTolerance()); // nanoseconds
      wrapper->SetShouldBeSaved(this->GetIsRecording());

      if (!m_Buffers.contains(toolNameAsQString))
      {
        niftk::IGIDataSourceBuffer::Pointer newBuffer =
          niftk::IGIDataSourceBuffer::New(m_Tracker->GetPreferredFramesPerSecond() * 2);

        newBuffer->SetLagInMilliseconds(m_Lag);
        m_Buffers.insert(toolNameAsQString, newBuffer);
      }

      // Save synchronously.
      // This has the side effect that if saving is too slow,
      // the QTimers just won't keep up, and start missing pulses.
      if (this->GetIsRecording())
      {
        this->SaveItem(wrapper.GetPointer());
        this->SetStatus("Saving");
      }
      else
      {
        this->SetStatus("Grabbing");
      }

      // Putting this after the save, as we don't want to
      // add to the buffer in this grabbing thread, then the
      // m_BackgroundDeleteThread deletes the object while
      // we are trying to save the data.
      m_Buffers[toolNameAsQString]->AddToBuffer(wrapper.GetPointer());
    }
  }
}


//-----------------------------------------------------------------------------
void MITKTrackerDataSourceService::SaveItem(niftk::IGIDataType::Pointer data)
{

  niftk::IGITrackerDataType::Pointer dataType = static_cast<niftk::IGITrackerDataType*>(data.GetPointer());
  if (dataType.IsNull())
  {
    mitkThrow() << "Failed to save IGITrackerDataType as the data received was the wrong type!";
  }

  vtkSmartPointer<vtkMatrix4x4> matrix = dataType->GetTrackingData();
  if (matrix == NULL)
  {
    mitkThrow() << "Failed to save IGITrackerDataType as the tracking matrix was NULL!";
  }

  QString directoryPath = this->GetRecordingDirectory();

  QString toolPath = directoryPath
      + niftk::GetPreferredSlash()
      + QString::fromStdString(dataType->GetToolName())
      + niftk::GetPreferredSlash();

  QDir directory(toolPath);
  if (directory.mkpath(toolPath))
  {
    QString fileName =  toolPath + QDir::separator() + tr("%1.txt").arg(data->GetTimeStampInNanoSeconds());

    bool success = SaveVtkMatrix4x4ToFile(fileName.toStdString(), *matrix);
    if (!success)
    {
      mitkThrow() << "Failed to save IGITrackerDataType to " << fileName.toStdString();
    }
    data->SetIsSaved(true);
  }
  else
  {
    mitkThrow() << "Failed to save IGITrackerDataType as could not create " << directoryPath.toStdString();
  }
}


//-----------------------------------------------------------------------------
std::vector<IGIDataItemInfo> MITKTrackerDataSourceService::Update(const niftk::IGIDataType::IGITimeType& time)
{
  std::vector<IGIDataItemInfo> infos;

  // This loads playback-data into the buffers, so must
  // come before the check for empty buffer.
  if (this->GetIsPlayingBack())
  {
    this->PlaybackData(time);
  }

  // Early exit if no buffers, which means that
  // the tracker is created, but has not seen anything to track yet.
  if (m_Buffers.isEmpty())
  {
    return infos;
  }

  if (!this->GetShouldUpdate())
  {
    return infos;
  }

  QMap<QString, niftk::IGIDataSourceBuffer::Pointer>::iterator iter;
  for (iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
  {
    QString bufferName = iter.key();

    if (m_Buffers[bufferName]->GetBufferSize() == 0)
    {
      continue;
    }

    if(m_Buffers[bufferName]->GetFirstTimeStamp() > time)
    {
      continue;
    }

    niftk::IGITrackerDataType::Pointer dataType =
      dynamic_cast<niftk::IGITrackerDataType*>(m_Buffers[bufferName]->GetItem(time).GetPointer());

    if (dataType.IsNull())
    {
      MITK_DEBUG << "Failed to find data for time " << time
                 << ", size=" << m_Buffers[bufferName]->GetBufferSize()
                 << ", last=" << m_Buffers[bufferName]->GetLastTimeStamp() << std::endl;
      continue;
    }

    mitk::DataNode::Pointer node = this->GetDataNode(bufferName);
    if (node.IsNull())
    {
      // The above call to GetDataNode should always retrieve one, or create it.
      mitkThrow() << "Can't find mitk::DataNode with name " << bufferName.toStdString();
    }

    CoordinateAxesData::Pointer coords = dynamic_cast<CoordinateAxesData*>(node->GetData());
    if (coords.IsNull())
    {
      coords = CoordinateAxesData::New();
      node->SetData(coords);
    }

    vtkSmartPointer<vtkMatrix4x4> matrix = dataType->GetTrackingData();
    coords->SetVtkMatrix(*matrix);

    // We tell the node that it is modified so the next rendering event
    // will redraw it. Triggering this does not in itself guarantee a re-rendering.
    coords->Modified();
    node->Modified();

    IGIDataItemInfo info;
    info.m_Name = this->GetName();
    info.m_FramesPerSecond = m_Buffers[bufferName]->GetFrameRate();
    info.m_IsLate = this->IsLate(time, dataType->GetTimeStampInNanoSeconds());
    info.m_LagInMilliseconds = this->GetLagInMilliseconds(time, dataType->GetTimeStampInNanoSeconds());
    infos.push_back(info);
  }

  return infos;
}

} // end namespace
