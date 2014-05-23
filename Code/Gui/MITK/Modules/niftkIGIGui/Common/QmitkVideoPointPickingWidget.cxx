/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkVideoPointPickingWidget.h"
#include <stdexcept>
#include <mitkOpenCVFileIOUtils.h>
#include <mitkFileIOUtils.h>
#include <niftkFileHelper.h>
#include <stdexcept>
#include <iostream>
#include <QApplication>
#include <QMessageBox>
#include <boost/regex.hpp>

//-----------------------------------------------------------------------------
QmitkVideoPointPickingWidget::QmitkVideoPointPickingWidget(
  const QString& inputTrackerDirectory,
  const QString& inputVideoDirectory,
  const QString& outputPointDirectory,  
  const unsigned long long timingToleranceInMilliseconds,
  const bool& skipForward,
  const bool& multiPointMode,
  const bool& skipExistingFrames,
  const unsigned int& samplingFrequency,
  QWidget *parent)
: QVTKWidget(parent)
, m_InputTrackerDirectory(inputTrackerDirectory)
, m_InputVideoDirectory(inputVideoDirectory)
, m_OutputPointDirectory(outputPointDirectory)
, m_TimingToleranceInMilliseconds(timingToleranceInMilliseconds)
, m_SkipForward(skipForward)
, m_MultiPointMode(multiPointMode)
, m_SkipExistingFrames(skipExistingFrames)
, m_SamplingFrequency(samplingFrequency)
{
  m_ImageViewer = vtkImageViewer::New();
  this->SetRenderWindow(m_ImageViewer->GetRenderWindow());
  m_ImageViewer->SetupInteractor(this->GetRenderWindow()->GetInteractor());
  m_ImageViewer->SetColorLevel(127.5);
  m_ImageViewer->SetColorWindow(255);  
  
  m_Matcher = mitk::VideoTrackerMatching::New();

  // Load all data, and set up the PNG reader to the first image.
  bool canFindTrackingData = mitk::CheckIfDirectoryContainsTrackingMatrices(m_InputTrackerDirectory.toStdString());
  if (!canFindTrackingData)
  {
    throw std::runtime_error("Could not find tracker matrices\n");
  }

  m_TrackingTimeStamps = mitk::FindTrackingTimeStamps(m_InputTrackerDirectory.toStdString());

  std::cout << "Found " << m_TrackingTimeStamps.m_TimeStamps.size() << " tracking matrices..." << std::endl;

  m_ImageFileCounter = 0;
  m_PointsOutputCounter = 0;

  /*
  m_ImageViewer->SetInputConnection(m_PNGReader->GetOutputPort());
  m_PNGReader->Update();

  int extent[6];
  m_PNGReader->GetDataExtent(extent);
  m_ImageWidth = extent[1] + 1;
  m_ImageHeight = extent[3] + 1;
  */
}


//-----------------------------------------------------------------------------
QmitkVideoPointPickingWidget::~QmitkVideoPointPickingWidget()
{
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::enterEvent(QEvent* event)
{
  this->resize(m_ImageWidth, m_ImageHeight);
  QVTKWidget::enterEvent(event);
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::mousePressEvent(QMouseEvent* event)
{
  try 
  {
    this->StorePoint(event);
    event->accept();
  }
  catch (std::exception& e)
  {
    std::cerr << "Caught std::exception:" << e.what();
    event->ignore();
  }
  catch (...)
  {
    std::cerr << "Caught unknown exception:";
    event->ignore();
  }
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_N)
  {
    this->NextImage();
    event->accept();    
  }
  else if (event->key() == Qt::Key_Q)
  {
    this->QuitApplication();
    event->accept();
  }
  else
  {
    event->ignore();
  }
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::QuitApplication()
{
  QApplication::exit(0);
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::ShowImage(const unsigned long int& imageNumber)
{
 /* m_PNGReader->SetFileName(m_ImageFiles[imageNumber].c_str());
  m_ImageViewer->Render();
  std::cout << "Displaying image[" << imageNumber << "/" << m_ImageFiles.size() << ", " << imageNumber*100/m_ImageFiles.size() << "%]=" << m_ImageFiles[imageNumber] << ", stored " << m_PointsOutputCounter << " so far." << std::endl; */
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::NextImage()
{
/*  if (m_ImageFileCounter < m_ImageFiles.size() - 1)
  {
    unsigned int offset = 1;

    if (m_SkipForward)
    {
      unsigned int testCounter = m_ImageFileCounter + offset;

      while (testCounter < m_ImageFiles.size() - 1)
      {
        QString imageFileName = QString::fromStdString(m_ImageFiles[testCounter]);
        QRegExp rx("([0-9]{19})");

        int matchIndex = imageFileName.indexOf(rx);
        if (matchIndex != -1)
        {
          QString imageTimeStampString = imageFileName.mid(matchIndex,19);

          long long delta = 0;
          unsigned long long imageTimeStamp = imageTimeStampString.toULongLong();
          unsigned long long matrixTimeStamp = m_TrackingTimeStamps.GetNearestTimeStamp(imageTimeStamp, &delta);

          delta /= 1000000; // convert nanoseconds to milliseconds.

          if (fabs(static_cast<double>(delta)) < m_TimingToleranceInMilliseconds)
          {
            break;
          }
        }
        std::cerr << "For image=" << m_ImageFileCounter << ", skipping offset=" << offset << std::endl;
        offset++;
        testCounter = m_ImageFileCounter + offset;
      }
    }

    m_ImageFileCounter += offset;
    this->ShowImage(m_ImageFileCounter);
  }*/
}

//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::CreateDir(const std::string& dir)
{
  if (!niftk::DirectoryExists(dir))
  {
    if (!niftk::CreateDirAndParents(dir))
    {
      QMessageBox::critical(this, tr("QmitkVideoPointPickingWidget"),
                                  tr("Can't write to\n%1").arg(QString::fromStdString(dir)),
                                  QMessageBox::Ok);
      QApplication::exit(-1);
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkVideoPointPickingWidget::StorePoint(QMouseEvent* event)
{
/*  if (event != NULL)
  {
    this->CreateDir(m_OutputPointDirectory.toStdString());
    this->CreateDir(m_OutputMatrixDirectory.toStdString());

    QString imageFileName = QString::fromStdString(m_ImageFiles[m_ImageFileCounter]);
    QRegExp rx("([0-9]{19})");

    int matchIndex = imageFileName.indexOf(rx);
    if (matchIndex != -1)
    {

      // Check if we have the right data.
      //
      // For a given image, where the filename is the timestamp,
      // we find the closest tracking matrix within tolerance.
      // If such a tracking matrix exists, we output both
      // matrix and point in separate files, named after the timestamp.
      QString imageTimeStampString = imageFileName.mid(matchIndex,19);

      long long delta = 0;
      unsigned long long imageTimeStamp = imageTimeStampString.toULongLong();
      unsigned long long matrixTimeStamp = m_TrackingTimeStamps.GetNearestTimeStamp(imageTimeStamp, &delta);

      delta /= 1000000; // convert nanoseconds to milliseconds.

      if (fabs(static_cast<double>(delta)) < m_TimingToleranceInMilliseconds)
      {
        // Output point.
        int xPixel = event->x();
        int yPixel = event->y();
        Qt::MouseButton button = event->button();
        int pinNumber = 0;
        if (button == Qt::LeftButton)
        {
          pinNumber = 0;
        }
        else if (button == Qt::MidButton)
        {
          pinNumber = 1;
        }
        else if (button == Qt::RightButton)
        {
          pinNumber = 2;
        }
        QString baseNameForPoint = imageTimeStampString + QString(".txt");
        std::string pointFileFullPath = niftk::ConvertToFullNativePath((m_OutputPointDirectory + QString("/") + baseNameForPoint).toStdString());

        ofstream myfile(pointFileFullPath.c_str(), std::ofstream::out | std::ofstream::trunc);
        if (myfile.is_open())
        {
          if (m_MultiPointMode)
          {
            myfile << pinNumber << " " << xPixel << " " << yPixel << std::endl;
          }
          else
          {
            myfile << xPixel << " " << yPixel << std::endl;
          }
          myfile.close();
        }
        else
        {
          QMessageBox::warning(this, tr("niftkUltrasoundPinCalibrationSorter"),
                                      tr("Failed to write point to file\n%1").arg(QString::fromStdString(pointFileFullPath)),
                                      QMessageBox::Ok);
        }

        // Output matrix
        QString baseNameForMatrix = QString::number(matrixTimeStamp) + QString(".txt");
        QString baseNameForImage = imageTimeStampString + QString(".txt");

        std::string inputMatrixFileFullPath = niftk::ConvertToFullNativePath((m_InputTrackerDirectory + QString("/") + baseNameForMatrix).toStdString());
        std::string outputMatrixFileFullPath = niftk::ConvertToFullNativePath((m_OutputMatrixDirectory + QString("/") + baseNameForImage).toStdString());

        vtkSmartPointer<vtkMatrix4x4> trackingMatrix = vtkMatrix4x4::New();
        trackingMatrix = mitk::LoadVtkMatrix4x4FromFile(inputMatrixFileFullPath);

        m_PointsOutputCounter++;

        if (!mitk::SaveVtkMatrix4x4ToFile(outputMatrixFileFullPath, *trackingMatrix))
        {
          QMessageBox::warning(this, tr("niftkUltrasoundPinCalibrationSorter"),
                                      tr("Failed to write matrix to file\n%1").arg(QString::fromStdString(outputMatrixFileFullPath)),
                                      QMessageBox::Ok);
        }
      }
      else
      {
        MITK_ERROR << "niftkUltrasoundPinCalibrationSorter: No tracking data for this image";
      }
    }
    else
    {
      QMessageBox::warning(this, tr("niftkUltrasoundPinCalibrationSorter"),
                                  tr("Invalid image file name\n%1").arg(imageFileName),
                                  QMessageBox::Ok);
    }
  } */
}

