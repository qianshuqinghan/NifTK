/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <cstdlib>
#include <limits>
#include <fstream>

#include <mitkMakeMaskImagesFromStereoVideo.h>
#include <niftkMakeMaskImagesFromStereoVideoCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;

  if ( trackingInputDirectory.length() == 0 )
  {
    std::cout << trackingInputDirectory.length() << std::endl;
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {
    mitk::MakeMaskImagesFromStereoVideo::Pointer projector = mitk::MakeMaskImagesFromStereoVideo::New();
    projector->SetAllowableTimingError(maxTimingError * 1e6);
    projector->SetFrequency(frequency*2);
    projector->SetAskOverWrite(queryOverWrite);
    projector->SetTimeStampsFile(timeStamps);
    projector->Initialise(trackingInputDirectory);

    mitk::VideoTrackerMatching::Pointer matcher = mitk::VideoTrackerMatching::New();
    matcher->Initialise(trackingInputDirectory);
    if ( videoLag != 0 ) 
    {
      if ( videoLag < 0 )
      {
        matcher->SetVideoLagMilliseconds(videoLag,true);
      }
      else 
      {
        matcher->SetVideoLagMilliseconds(videoLag,false);
      }
    }

    if ( ! projector->GetInitOK() ) 
    {
      MITK_ERROR << "Projector failed to initialise, halting.";
      return -1;
    }
    matcher->SetWriteTimingErrors(WriteTimingErrors);

    projector->SetTrackerIndex(trackerIndex);
    projector->SetReferenceIndex(referenceIndex);
    projector->Project(matcher);

    returnStatus = EXIT_SUCCESS;
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Caught std::exception:" << e.what();
    returnStatus = -1;
  }
  catch (...)
  {
    MITK_ERROR << "Caught unknown exception:";
    returnStatus = -2;
  }

  return returnStatus;
}