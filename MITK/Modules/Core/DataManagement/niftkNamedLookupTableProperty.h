/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkNamedLookupTableProperty_h
#define niftkNamedLookupTableProperty_h

#include "niftkCoreExports.h"

#include <mitkLookupTableProperty.h>

namespace niftk
{

/**
 * \class NamedLookupTableProperty
 * \brief Provides a property so we can see the lookup table name in the property window.
 * \ingroup uk_ac_ucl_cmic_imagelookuptables_internal
 */
class NIFTKCORE_EXPORT NamedLookupTableProperty : public mitk::LookupTableProperty
{
public:

  mitkClassMacro(NamedLookupTableProperty, mitk::LookupTableProperty)
  itkNewMacro(NamedLookupTableProperty)
  mitkNewMacro2Param(NamedLookupTableProperty, const std::string&, const mitk::LookupTable::Pointer)
  mitkNewMacro3Param(NamedLookupTableProperty, const std::string&, const mitk::LookupTable::Pointer, bool)

  virtual std::string GetValueAsString() const override;

  itkSetStringMacro(Name);
  itkGetStringMacro(Name);

  itkSetMacro(IsScaled,bool);
  itkGetConstMacro(IsScaled,bool);

  itkBooleanMacro(IsScaled);

protected:

  virtual ~NamedLookupTableProperty();
  NamedLookupTableProperty();
  NamedLookupTableProperty(const NamedLookupTableProperty& other);
  NamedLookupTableProperty(const std::string& name, const mitk::LookupTable::Pointer lut);
  NamedLookupTableProperty(const std::string& name, const mitk::LookupTable::Pointer lut, bool scale);

private:

  NamedLookupTableProperty& operator=(const NamedLookupTableProperty&); // Purposefully not implemented
  itk::LightObject::Pointer InternalClone() const override;

  virtual bool IsEqual(const mitk::BaseProperty& property) const override;
  virtual bool Assign(const mitk::BaseProperty& property) override;

  std::string m_Name;
  bool        m_IsScaled;
};

}

#endif
