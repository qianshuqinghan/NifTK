/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
#ifndef niftkNVidiaSDIDataSourceService_h
#define niftkNVidiaSDIDataSourceService_h

#include <niftkIGIDataSource.h>
#include <niftkIGIDataSourceLocker.h>
#include <niftkIGILocalDataSourceI.h>
#include <niftkIGIDataSourceGrabbingThread.h>
#include <niftkIGIDataSourceLinearBuffer.h>
#include <niftkIGICleanableDataSourceI.h>
#include <niftkIGIDataSourceBackgroundDeleteThread.h>

#include <QObject>
#include <QSet>
#include <QMutex>
#include <QString>
#include <cv.h>
#include <memory>

namespace niftk
{

// some forward decls to avoid header pollution
class NVidiaSDIDataSourceImpl;

/**
* \class NVidiaSDIDataSourceService
* \brief Provides an NVidia SDI video feed, as an IGIDataSourceServiceI.
*
* Note: All errors should thrown as mitk::Exception or sub-classes thereof.
*/
class NVidiaSDIDataSourceService
    : public QObject
    , public IGIDataSource
    , public IGILocalDataSourceI
    , public IGICleanableDataSourceI
{

  Q_OBJECT

public:

  mitkClassMacroItkParent(NVidiaSDIDataSourceService, IGIDataSource)
  mitkNewMacro3Param(NVidiaSDIDataSourceService, QString, const IGIDataSourceProperties&, mitk::DataStorage::Pointer)

    // This should match libvideo/SDIInput::InterlacedBehaviour
  enum InterlacedBehaviour
  {
    DO_NOTHING_SPECIAL    = 0,
    DROP_ONE_FIELD        = 1,
    STACK_FIELDS          = 2, /** No longer supported! */
    SPLIT_LINE_INTERLEAVED_STEREO = 3
  };

  /**
  * \see niftk::IGILocalDataSourceI::GrabData()
  */
  virtual void GrabData() override;

  /**
  * \see niftk::IGIDataSource::CleanBuffer()
  */
  virtual void CleanBuffer() override;

  /**
  * \see  IGIDataSourceI::StartPlayback()
  */
  virtual void StartPlayback(niftk::IGIDataSourceI::IGITimeType firstTimeStamp,
                             niftk::IGIDataSourceI::IGITimeType lastTimeStamp) override;

  /**
  * \see IGIDataSourceI::PlaybackData()
  */
  void PlaybackData(niftk::IGIDataSourceI::IGITimeType requestedTimeStamp) override;

  /**
  * \see IGIDataSourceI::StopPlayback()
  */
  virtual void StopPlayback() override;

  /**
  * \see IGIDataSourceI::Update()
  */
  virtual std::vector<IGIDataItemInfo> Update(const niftk::IGIDataSourceI::IGITimeType& time) override;

  /**
  * \see IGIDataSourceI::ProbeRecordedData()
  */
  bool ProbeRecordedData(niftk::IGIDataSourceI::IGITimeType* firstTimeStampInStore,
                         niftk::IGIDataSourceI::IGITimeType* lastTimeStampInStore) override;

  /**
  * \brief IGIDataSourceI::SetProperties()
  */
  virtual void SetProperties(const IGIDataSourceProperties& properties) override;

  /**
  * \brief IGIDataSourceI::GetProperties()
  */
  virtual IGIDataSourceProperties GetProperties() const override;

  /** From here down until 'protected:', importing Johannes's QmitkIGINVidiaDataSource API. */

  // used to capture a lower-resolution image
  // can only be changed when no capture is running! see IsCapturing() etc 
  void SetMipmapLevel(unsigned int l);
  void SetFieldMode(InterlacedBehaviour b);
  InterlacedBehaviour GetFieldMode() const;
  int GetNumberOfStreams();
  int GetCaptureWidth();
  int GetCaptureHeight();
  int GetRefreshRate();
  int GetTextureId(int stream);
  const char* GetWireFormatString();

protected slots:

  // to be used by NVidiaSDIDataSourceImpl to make us show a message box.
  void ShowFatalErrorMessage(QString msg);

  void OnGrabbingThreadFailed(QString errorMsg);

protected:

  NVidiaSDIDataSourceService(QString factoryName,
                               const IGIDataSourceProperties& properties,
                               mitk::DataStorage::Pointer dataStorage
                               );
  virtual ~NVidiaSDIDataSourceService();

private:

  NVidiaSDIDataSourceService(const NVidiaSDIDataSourceService&); // deliberately not implemented
  NVidiaSDIDataSourceService& operator=(const NVidiaSDIDataSourceService&); // deliberately not implemented

  void SaveItem(niftk::IGIDataType& item);

  /** For pattern required by base class */
  static niftk::IGIDataSourceLocker                 s_Lock;
  QMutex                                            m_Lock;
  int                                               m_ChannelNumber;
  niftk::IGIDataSourceI::IGIIndexType               m_FrameId;
  niftk::IGIDataSourceLinearBuffer                  m_Buffer;
  niftk::IGIDataSourceGrabbingThread*               m_DataGrabbingThread;
  niftk::IGIDataSourceBackgroundDeleteThread*       m_BackgroundDeleteThread;

  /** From here down, importing Johannes's QmitkIGINVidiaDataSource */

  // holds internals to prevent header pollution
  NVidiaSDIDataSourceImpl*                          m_Pimpl;
  unsigned int                                      m_MostRecentSequenceNumber;
  unsigned int                                      m_MipmapLevel;
  
  // used to correlate clock, frame numbers and other events
  std::ofstream                                     m_FrameMapLogFile;

  // used to detect whether record has stopped or not.
  // there's no notification when the user clicked stop-record.
  bool                                              m_WasSavingMessagesPreviously;

  // because the sdi thread is running separately to the data-source-interface
  // we can end up in a situation where sdi bits get recreated with new sequence numbers
  // but these parts here still expect the old sdi instance.
  unsigned int                                      m_ExpectedCookie;
  static const char*                                s_NODE_NAME;
    
  // Nested private type
  struct PlaybackPerFrameInfo
  {
    unsigned int m_SequenceNumber;
    // we have max 4 channels via sdi.
    unsigned int m_frameNumber[4];
    PlaybackPerFrameInfo();
  };
  std::map<niftk::IGIDataSourceI::IGITimeType, 
    PlaybackPerFrameInfo>                           m_PlaybackIndex;

  // used to prevent replaying the same thing over and over again.
  // because decompression in its current implementation is quite heavy-weight,
  // the repeated calls to PlaybackData() and similarly Update() slow down the machine
  // quite significantly.
  niftk::IGIDataSourceI::IGITimeType                m_MostRecentlyPlayedbackTimeStamp;
  niftk::IGIDataSourceI::IGITimeType                m_MostRecentlyUpdatedTimeStamp;
  std::pair<IplImage*, int>                         m_CachedUpdate;

private:

  /**
  * \brief Starts the framegrabbing.
  *
  * Should only be called once from constructor.
  */
  void StartCapturing();

  /**
  * \brief Stops the framegrabbing.
  *
  * Should only be called once from destructor.
  */
  void StopCapturing();

  /**
  * \brief Returns true if capturing and false otherwise.
  */
  bool IsCapturing();

  bool InitWithRecordedData(
    std::map<niftk::IGIDataSourceI::IGITimeType, PlaybackPerFrameInfo>& index, 
    const std::string& path, 
    niftk::IGIDataSourceI::IGITimeType* firstTimeStampInStore, 
    niftk::IGIDataSourceI::IGITimeType* lastTimeStampInStore, 
    bool forReal);

}; // end class

} // end namespace

#endif
