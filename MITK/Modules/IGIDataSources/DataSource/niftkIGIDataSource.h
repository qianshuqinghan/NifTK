/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkIGIDataSource_h
#define niftkIGIDataSource_h

#include <niftkIGIDataSourcesExports.h>
#include <niftkIGIDataSourceI.h>

#include <mitkDataStorage.h>
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

#include <igtlTimeStamp.h>

#include <QDir>
#include <QString>
#include <QMutex>

namespace niftk
{

/**
* \class IGIDataSource
* \brief Abstract base class for IGI DataSources, such as objects
* that produce tracking data, video frames or ultrasound frames.
*
* Each source registers as a service when it is instantiated. You
* MUST allow for multiple instances of each service. Each service
* should have a different name, and unique Id.
*
* Uses RAII pattern to register/de-register as MITK Micro-Service.
*
* Note: All errors should thrown as mitk::Exception or sub-classes thereof.
*/
class NIFTKIGIDATASOURCES_EXPORT IGIDataSource : public niftk::IGIDataSourceI
{
public:

  mitkClassMacroItkParent(IGIDataSource, niftk::IGIDataSourceI)

  /**
  * \see IGIDataSourceI::GetName()
  */
  virtual QString GetName() const override;

  /**
  * \see IGIDataSourceI::GetFactoryName()
  */
  virtual QString GetFactoryName() const override;

  /**
  * \see IGIDataSourceI::GetStatus()
   */
  virtual QString GetStatus() const override;

  /**
  * \see IGIDataSourceI::GetDescription()
  */
  QString GetDescription() const override;

  /**
  * \see IGIDataSourceI::GetShouldUpdate()
  */
  virtual bool GetShouldUpdate() const override;

  /**
  * \see IGIDataSourceI::SetShouldUpdate()
  */
  virtual void SetShouldUpdate(bool shouldUpdate) override;

  /**
  * \see IGIDataSourceI::StartPlayback()
  */
  virtual void StartPlayback(niftk::IGIDataSourceI::IGITimeType firstTimeStamp,
                             niftk::IGIDataSourceI::IGITimeType lastTimeStamp) override;

  /**
  * \see IGIDataSourceI::StopPlayback()
  */
  virtual void StopPlayback() override;

  /**
  * \see IGIDataSourceI::StartRecording()
  */
  virtual void StartRecording() override;

  /**
  * \see IGIDataSourceI::StopRecording()
  */
  virtual void StopRecording() override;

  /**
  * \see IGIDataSourceI::SetRecordingLocation()
  */
  virtual void SetRecordingLocation(const QString& pathName) override;

  /**
  * \see IGIDataSourceI::GetRecordingLocation()
  */
  virtual QString GetRecordingLocation() const override;

  /**
  * \see IGIDataSourceI::SetPlaybackSourceName()
  */
  virtual void SetPlaybackSourceName(const QString& sourceName) override;

  /**
  * \see IGIDataSourceI::GetPlaybackSourceName()
  */
  virtual QString GetPlaybackSourceName() const override;

  /**
  * \brief Returns this->GetRecordingLocation() slash this->GetPlaybackSourceName().
  */
  QString GetPlaybackDirectory() const;

  /**
  * \brief Returns this->GetRecordingLocation() slash this->GetName().
  */
  QString GetRecordingDirectory() const;

  itkGetConstMacro(IsRecording, bool);
  itkGetConstMacro(IsPlayingBack, bool);

  itkSetMacro(TimeStampTolerance, niftk::IGIDataSourceI::IGITimeType);
  itkGetConstMacro(TimeStampTolerance, niftk::IGIDataSourceI::IGITimeType);

protected:

  IGIDataSource(const std::string& name,
                const std::string& factoryName,
                mitk::DataStorage::Pointer dataStorage); // Purposefully hidden.
  virtual ~IGIDataSource(); // Purposefully hidden.

  IGIDataSource(const IGIDataSource&); // Purposefully not implemented.
  IGIDataSource& operator=(const IGIDataSource&); // Purposefully not implemented.

  /**
   * \brief Derived classes request a node for a given name. If the node does
   * not exist, it will be created with some default properties.
   * \param name if supplied the node will be assigned that name,
   * and if empty, the node will be given the name this->GetMicroServiceDeviceName().
   * \param addToDataStorage if true, will be added to data storage,
   * if false, the caller can determine when to do it.
   */
  mitk::DataNode::Pointer GetDataNode(const QString& name=QString(), const bool& addToDataStorage=true);

  /**
  * \brief Returns true if the delay between requested and actual is
  * greater than the TimeStampTolerance, and false otherwise.
  */
  bool IsLate(const niftk::IGIDataSourceI::IGITimeType& requested,
              const niftk::IGIDataSourceI::IGITimeType& actual
              ) const;

  /**
  * \brief Simply checks the difference in time, and converts to milliseconds.
  */
  unsigned int GetLagInMilliseconds(const niftk::IGIDataSourceI::IGITimeType& requested,
                                    const niftk::IGIDataSourceI::IGITimeType& actual
                                   ) const;

  /**
  * \brief Returns the pointer to the internal data storage.
  */
  mitk::DataStorage::Pointer GetDataStorage() const;

  /**
  * \brief Queries the internal igtl::TimeStamp to get an up-to-date timestamp.
  */
  niftk::IGIDataSourceI::IGITimeType GetTimeStampInNanoseconds();

  itkSetMacro(IsRecording, bool);
  itkSetMacro(IsPlayingBack, bool);

  void SetStatus(const QString& status);
  void SetDescription(const QString& description);

private:

  mutable QMutex                     m_Mutex;
  igtl::TimeStamp::Pointer           m_TimeStamp;
  mitk::DataStorage::Pointer         m_DataStorage;
  std::set<mitk::DataNode::Pointer>  m_DataNodes;
  us::ServiceRegistration<Self>      m_MicroServiceRegistration;
  QString                            m_Name;
  QString                            m_FactoryName;
  QString                            m_Status;
  QString                            m_Description;
  QString                            m_RecordingLocation;
  QString                            m_PlaybackSourceName;
  niftk::IGIDataSourceI::IGITimeType m_TimeStampTolerance; // nanoseconds.
  bool                               m_ShouldUpdate;
  bool                               m_IsRecording;
  bool                               m_IsPlayingBack;
};

} // end namespace

MITK_DECLARE_SERVICE_INTERFACE(niftk::IGIDataSource, "uk.ac.ucl.cmic.IGIDataSource")

#endif
