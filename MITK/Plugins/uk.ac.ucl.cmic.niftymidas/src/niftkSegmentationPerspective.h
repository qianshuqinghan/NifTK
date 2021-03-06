/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSegmentationPerspective_h
#define niftkSegmentationPerspective_h

#include <uk_ac_ucl_cmic_niftymidas_Export.h>
#include <berryIPerspectiveFactory.h>

namespace niftk
{

/// \class niftkMIDASSegmentationPerspective
/// \brief Perspective to arrange widgets as would be suitable for MIDAS applications,
/// where the standard view has up to 5x5 independent windows.
/// \ingroup uk_ac_ucl_cmic_common
///
/// Note: We have to load at least one view component, to get an editor created.
class NIFTYMIDAS_EXPORT SegmentationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  SegmentationPerspective();
  SegmentationPerspective(const SegmentationPerspective& other);

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

}

#endif
