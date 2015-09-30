/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSpectraTracker_h
#define niftkSpectraTracker_h

#include <niftkIGITrackersExports.h>
#include "niftkNDIBaseTracker.h"

namespace niftk
{

/**
 * \class SpectraTracker
 * \brief RAII object to connect to Polaris Spectra tracker.
 */
class NIFTKIGITRACKERS_EXPORT SpectraTracker : public niftk::NDIBaseTracker
{
public:

  mitkClassMacroItkParent(SpectraTracker, niftk::NDIBaseTracker);
  mitkNewMacro3Param(SpectraTracker, mitk::DataStorage::Pointer, mitk::SerialCommunication::PortNumber, std::string);

protected:

  SpectraTracker(mitk::DataStorage::Pointer dataStorage,
                 mitk::SerialCommunication::PortNumber portNumber,
                 std::string toolConfigFileName); // Purposefully hidden.

  virtual ~SpectraTracker(); // Purposefully hidden.

  SpectraTracker(const SpectraTracker&); // Purposefully not implemented.
  SpectraTracker& operator=(const SpectraTracker&); // Purposefully not implemented.

private:

}; // end class

} // end namespace

#endif
