/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkDataStoragePropertyListener_h
#define niftkDataStoragePropertyListener_h

#include "niftkCoreExports.h"

#include <itkObject.h>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkMessage.h>

#include "niftkDataStorageListener.h"

namespace niftk
{

class PropertyChangedCommand;

/**
 * \class DataStoragePropertyListener
 * \brief Class for listening to changes of data node properties with a specific name.
 *
 * To use this class, you need to create an instance, and assign a handler to the
 * 'NodePropertyChanged' data member.
 *
 * An alternative way of using this class is deriving a new class and overriding the
 * 'OnPropertyChanged' member function. Note that if you do so, you need to call
 * the superclass implementation at the beginning of your function.
 *
 * We recommend to follow the first way.
 *
 * The event listening can be restricted to a set of renderers. It is the resposibility of the user of this
 * class to remove the renderer from the DataStoragePropertyListener objects when the renderer is deleted.
 */
class NIFTKCORE_EXPORT DataNodePropertyListener : public DataStorageListener
{

public:

  mitkClassMacro(DataNodePropertyListener, DataStorageListener)
  mitkNewMacro2Param(DataNodePropertyListener, const mitk::DataStorage::Pointer, const std::string&)
  mitkNewMacro3Param(DataNodePropertyListener, const mitk::DataStorage::Pointer, const std::string&, bool)
  mitkNewMacro3Param(DataNodePropertyListener, const mitk::DataStorage::Pointer, const std::string&, int)
  mitkNewMacro3Param(DataNodePropertyListener, const mitk::DataStorage::Pointer, const std::string&, float)
  mitkNewMacro3Param(DataNodePropertyListener, const mitk::DataStorage::Pointer, const std::string&, const std::string&)

  DataNodePropertyListener(const mitk::DataStorage::Pointer dataStorage, const std::string& propertyName);
  DataNodePropertyListener(const mitk::DataStorage::Pointer dataStorage, const std::string& propertyName, bool defaultValue);
  DataNodePropertyListener(const mitk::DataStorage::Pointer dataStorage, const std::string& propertyName, int defaultValue);
  DataNodePropertyListener(const mitk::DataStorage::Pointer dataStorage, const std::string& propertyName, float defaultValue);
  DataNodePropertyListener(const mitk::DataStorage::Pointer dataStorage, const std::string& propertyName, const std::string& defaultValue);

  virtual ~DataNodePropertyListener();

  /// \brief Sets the list of renderers to check.
  void SetRenderers(const std::vector<const mitk::BaseRenderer*>& renderers);

  /// \brief GUI independent message callback.
  mitk::Message2<mitk::DataNode*, const mitk::BaseRenderer*> NodePropertyChanged;

protected:

  DataNodePropertyListener(const DataNodePropertyListener&); // Purposefully not implemented.
  DataNodePropertyListener& operator=(const DataNodePropertyListener&); // Purposefully not implemented.

  /// \brief Called when a node is added to the data storage.
  /// Adds the observers for the node then notifies them.
  /// \see DataStoragePropertyListener::NodeAdded
  virtual void OnNodeAdded(mitk::DataNode* node) override;

  /// \brief Called when a node is removed from the data storage.
  /// Notifies the observers for the node then removes them.
  /// \see DataStoragePropertyListener::NodeRemoved
  virtual void OnNodeRemoved(mitk::DataNode* node) override;

  /// \brief Called when a node is deleted.
  /// Notifies the observers for the node then removes them.
  /// \see DataStoragePropertyListener::NodeDeleted
  virtual void OnNodeDeleted(mitk::DataNode* node) override;

  /// \brief Called when the global or a renderer specific property of the node has changed or removed.
  virtual void OnPropertyChanged(mitk::DataNode* node, const mitk::BaseRenderer* renderer);

private:

  /// \brief Add the property observers.
  void AddObservers(mitk::DataNode* node);

  /// \brief Removes the property observers.
  void RemoveObservers(mitk::DataNode* node);

  /// \brief Add the property observers for all node in the data storage.
  /// One observer is added for the global property and one for each renderer.
  virtual void AddAllObservers();

  /// \brief Removes the property observers from all node in the data storage.
  virtual void RemoveAllObservers();

  /// \brief The name of the property we are tracking.
  std::string m_PropertyName;

  /// \brief We store an optional list of renderers for watching renderer specific changes.
  std::vector<const mitk::BaseRenderer*> m_Renderers;

  typedef std::map<mitk::DataNode*, std::vector<unsigned long> > NodePropertyObserverTags;

  /// \brief We observe all the properties with a given name for each registered node.
  /// The first element of the vector is the "global" property, the rest are the renderer
  /// specific properties in the same order as in m_Renderers.
  /// The observers are notified when a property of a node is changed or removed.
  NodePropertyObserverTags m_PropertyObserverTagsPerNode;

  enum DefaultValueType
  {
    NoType, BoolType, IntType, FloatType, StringType
  };

  DefaultValueType m_DefaultValueType;
  bool             m_BoolDefaultValue;
  int              m_IntDefaultValue;
  float            m_FloatDefaultValue;
  std::string      m_StringDefaultValue;

friend class PropertyChangedCommand;

};

}

#endif
