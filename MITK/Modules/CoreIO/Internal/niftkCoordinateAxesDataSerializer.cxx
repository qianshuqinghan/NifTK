/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoordinateAxesDataSerializer.h"

#include <itksys/SystemTools.hxx>

#include <niftkCoordinateAxesData.h>

#include "niftkCoordinateAxesDataWriterService.h"
#include "niftkSerializerMacros.h"

NIFTK_REGISTER_SERIALIZER(CoordinateAxesDataSerializer)

namespace niftk
{

CoordinateAxesDataSerializer::CoordinateAxesDataSerializer()
{
}


CoordinateAxesDataSerializer::~CoordinateAxesDataSerializer()
{
}


std::string CoordinateAxesDataSerializer::Serialize()
{
  const CoordinateAxesData* image = dynamic_cast<const CoordinateAxesData*>( m_Data.GetPointer() );
  if (image == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an CoordinateAxesData. Cannot serialize as CoordinateAxesData.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".4x4";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    niftk::CoordinateAxesDataWriterService writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(const_cast<CoordinateAxesData*>(image));
    writer.Write();
  }
  catch (std::exception& e)
  {
    MITK_ERROR << " Error serializing object at " << (const void*) this->m_Data
              << " to "
              << fullname
              << ": "
              << e.what();
    return "";
  }

  return filename;
}

}
