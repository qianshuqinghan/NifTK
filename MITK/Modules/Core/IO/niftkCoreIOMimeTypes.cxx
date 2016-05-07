/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoreIOMimeTypes.h"
#include <mitkIOMimeTypes.h>

//-----------------------------------------------------------------------------
std::vector<mitk::CustomMimeType*> niftk::CoreIOMimeTypes::Get()
{
  std::vector<mitk::CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)
  mimeTypes.push_back(LABELMAP_MIMETYPE().Clone());

  return mimeTypes;
}


// ----------------------------------------------------------------
// LabelMap Mime type
mitk::CustomMimeType niftk::CoreIOMimeTypes::LABELMAP_MIMETYPE()
{
  mitk::CustomMimeType mimeType(LABELMAP_MIMETYPE_NAME());
  std::string category = "LabelMap File";
  mimeType.SetComment("Label Map format");
  mimeType.SetCategory(category);
  mimeType.AddExtension("lmap");
    
  return mimeType;
}

std::string niftk::CoreIOMimeTypes::LABELMAP_MIMETYPE_NAME()
{
  static std::string name = mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".lmap";
  return name;
}

std::string niftk::CoreIOMimeTypes::LABELMAP_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Label map that defines the mapping between color and pixel value.";
  return description;
}