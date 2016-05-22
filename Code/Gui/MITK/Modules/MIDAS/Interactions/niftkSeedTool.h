/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSeedTool_h
#define niftkSeedTool_h

#include "niftkMIDASExports.h"

//#include "niftkPointSetDataInteractor.h"
#include "niftkPointSetInteractor.h"
#include "niftkTool.h"

namespace niftk
{

/**
 * \class SeedTool
 * \brief Seed tool for adding / removing / moving seeds.
 *
 * Interestingly, ANY of PolyTool, DrawTool or SeedTool can add seeds.
 * but only SeedTool can remove them.
 *
 * Provides
 * <pre>
 * 1. Right mouse button = place seed
 * 2. Left mouse button = select seed
 * 3. Move with left mouse button down = move selected seed
 * 4. Middle mouse button = select a seed if it is within a given distance and remove it.
 * </pre>
 * and includes Undo/Redo functionality. Given the above list, to remove seeds most people
 * hold the middle mouse button down, and drag it around, sucking up the seed points like a hoover.
 */
class NIFTKMIDAS_EXPORT SeedTool : public MIDASTool {

public:

  mitkClassMacro(SeedTool, MIDASTool);
  itkNewMacro(SeedTool);

  virtual void InitializeStateMachine();

  /// \see mitk::Tool::GetName()
  virtual const char* GetName() const;

  /// \see mitk::Tool::GetXPM()
  virtual const char** GetXPM() const;

  /// \brief When called, we create and register a point set interactor.
  virtual void Activated();

  /// \brief When called, we unregister the point set interactor.
  virtual void Deactivated();

  /// \brief Adds an event filter that can reject a state machine event or let it pass through.
  /// Overrides niftkMIDASStateMachine::InstallEventFilter() so that it adds every filter also to the
  /// internal point set interactor.
  virtual void InstallEventFilter(MIDASEventFilter* eventFilter);

  /// \brief Removes an event filter that can reject a state machine event or let it pass through.
  /// Overrides niftkMIDASStateMachine::InstallEventFilter() to that it removes every filter also from the
  /// internal point set interactor.
  virtual void RemoveEventFilter(MIDASEventFilter* eventFilter);

protected:

  SeedTool();
  virtual ~SeedTool();

private:

  MIDASPointSetInteractor::Pointer m_PointSetInteractor;
//  MIDASPointSetDataInteractor::Pointer m_PointSetInteractor;

};

}

#endif
