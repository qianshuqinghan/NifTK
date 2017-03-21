/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
#ifndef niftkBKMedicalDataSourceWorker_h
#define niftkBKMedicalDataSourceWorker_h

#include <QObject>
#include <QImage>
#include <QTcpSocket>
#include <QByteArray>
#include <QMutex>

namespace niftk
{

/**
* \class BKMedicalDataSourceWorker
* \brief Main Worker object for BKMedicalDataSourceService that runs in a separate QThread.
*/
class BKMedicalDataSourceWorker : public QObject
{

  Q_OBJECT

public:
  BKMedicalDataSourceWorker(const int& timeOut, const int& framesPerSecond);
  ~BKMedicalDataSourceWorker();

  void ConnectToHost(QString address, int port);
  void RequestStopStreaming();

public slots:

  /** Called by QThread, and it the main processing method. */
  void ReceiveImages();

signals:

  void ImageReceived(QImage);
  void ErrorGenerated(QString);

private:

  size_t GenerateCommandMessage(const std::string& message);
  bool SendCommandMessage(const std::string& message);
  std::string ReceiveResponseMessage(const size_t& expectedSize);
  void StopStreaming();
  void ReceiveImage(QImage& image);
  int FindFirstANotPreceededByB(const QByteArray& buf,
                                const char& a,
                                const char& b);

  QMutex        m_Lock;
  int           m_Timeout;
  int           m_FramesPerSecond;
  QTcpSocket    m_Socket;
  QByteArray    m_IntermediateBuffer;
  char          m_OutgoingMessageBuffer[256];
  char          m_ImageBuffer[1024*1024*4];
  int           m_ImageSize[2];
  QVector<QRgb> m_DefaultLUT;
  bool          m_RequestStopStreaming;
  bool          m_IsStreaming;
};

} // end namespace

#endif
