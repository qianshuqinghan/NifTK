/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkINRImageIOFactory_h
#define itkINRImageIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <NifTKConfigure.h>
#include <niftkITKWin32ExportHeader.h>

#include <itkObjectFactoryBase.h>
#include <itkImageIOBase.h>

namespace itk
{
/**
 * \class INRImageIOFactory
 * \brief Create instances of INRImageIO objects using an object factory.
 */
class NIFTKITK_WINEXPORT ITK_EXPORT INRImageIOFactory : public ObjectFactoryBase
{
public:  
  /** Standard class typedefs. */
  typedef INRImageIOFactory   Self;
  typedef ObjectFactoryBase  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;
    
  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(INRImageIOFactory, ObjectFactoryBase);

protected:
  INRImageIOFactory();
  ~INRImageIOFactory();

private:
  INRImageIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};
  
  
} // end namespace itk

#endif
