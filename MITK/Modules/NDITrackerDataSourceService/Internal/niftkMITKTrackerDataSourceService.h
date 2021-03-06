/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
#ifndef niftkMITKTrackerDataSourceService_h
#define niftkMITKTrackerDataSourceService_h

#include <niftkIGITrackerDataSourceService.h>
#include <niftkIGIDataSourceGrabbingThread.h>
#include <niftkNDITracker.h>

namespace niftk
{

/**
* \class MITKTrackerDataSourceService
* \brief Provides a local MITK implementation of a tracker interface,
* as an IGIDataSourceServiceI. This class coordinates between
* niftk::NDITracker subclasses, niftk::IGITrackerBackend subclasses and
* an niftk::IGIDataSourceGrabbingThread and also enables this class to
* be registered as an MITK MicroService.
*
* Note: All errors should thrown as mitk::Exception or sub-classes thereof.
*/
class MITKTrackerDataSourceService : public IGITrackerDataSourceService
{

  Q_OBJECT

public:

  mitkClassMacroItkParent(MITKTrackerDataSourceService, IGITrackerDataSourceService)
  mitkNewMacro5Param(MITKTrackerDataSourceService, QString, QString,
                     const IGIDataSourceProperties&, mitk::DataStorage::Pointer, niftk::NDITracker::Pointer)

protected:

  MITKTrackerDataSourceService(QString name,
                               QString factoryName,
                               const IGIDataSourceProperties& properties,
                               mitk::DataStorage::Pointer dataStorage,
                               niftk::NDITracker::Pointer tracker
                              );
  virtual ~MITKTrackerDataSourceService();

private slots:

  void OnErrorFromThread(QString);

private:

  MITKTrackerDataSourceService(const MITKTrackerDataSourceService&); // deliberately not implemented
  MITKTrackerDataSourceService& operator=(const MITKTrackerDataSourceService&); // deliberately not implemented

  niftk::IGIDataSourceGrabbingThread* m_DataGrabbingThread;

}; // end class

} // end namespace

#endif
