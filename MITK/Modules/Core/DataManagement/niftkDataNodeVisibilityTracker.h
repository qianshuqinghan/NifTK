/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkDataStorageVisibilityTracker_h
#define niftkDataStorageVisibilityTracker_h

#include "niftkCoreExports.h"

#include <itkObject.h>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>

#include "niftkDataNodePropertyListener.h"

namespace mitk
{
class BaseRenderer;
}


namespace niftk
{

/**
 * \class DataStorageVisibilityTracker
 * \brief Observes the visibility changes of data nodes in a 'tracked' renderer and updates their visibility in the 'managed' renderers accordingly.
 *
 * If the tracked renderer is set to 0, only the global visibility is observed.
 *
 * This finds use in the Thumbnail viewer plugin, which tracks visibility properties, and applies
 * them to a single render window, and also in the Side Viewer which tracks visibility properties,
 * and applies them to another viewer.
 */
class NIFTKCORE_EXPORT DataNodeVisibilityTracker : public DataNodePropertyListener
{

public:

  mitkClassMacro(DataNodeVisibilityTracker, DataNodePropertyListener)
  mitkNewMacro1Param(DataNodeVisibilityTracker, const mitk::DataStorage::Pointer)

  /// \brief Sets the renderer we are tracking.
  void SetTrackedRenderer(const mitk::BaseRenderer* trackedRenderer);

  /// \brief Sets the list of renderers to propagate visibility properties onto.
  void SetManagedRenderers(const std::vector<const mitk::BaseRenderer*>& managedRenderers);

  /// \brief Sets the list of nodes that whose visibility should not be tracked.
  /// We provide facility to ignore nodes, and not adjust their visibility, which is useful for cross hairs.
  void SetNodesToIgnore(const std::vector<mitk::DataNode*>& nodesToIgnore);

  /// \brief Tells if a node is ignored, i.e its visibility is not tracked.
  bool IsIgnored(mitk::DataNode* node);

protected:

  DataNodeVisibilityTracker(const mitk::DataStorage::Pointer dataStorage);
  virtual ~DataNodeVisibilityTracker();

  DataNodeVisibilityTracker(const DataNodeVisibilityTracker&); // Purposefully not implemented.
  DataNodeVisibilityTracker& operator=(const DataNodeVisibilityTracker&); // Purposefully not implemented.

  /// \see DataStorageListener::NodeAdded
  virtual void OnNodeAdded(mitk::DataNode* node) override;

  /// \brief Called when the property value has changed globally or for the given renderer.
  /// If the global property has changed, renderer is NULL.
  virtual void OnPropertyChanged(mitk::DataNode* node, const mitk::BaseRenderer* renderer) override;

private:

  const mitk::BaseRenderer* m_TrackedRenderer;
  std::vector<const mitk::BaseRenderer*> m_ManagedRenderers;
  std::vector<mitk::DataNode*> m_NodesToIgnore;
};

}

#endif
