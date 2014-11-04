/**************************************************************************************/
/*                                                                                    */
/*  Visualization Library                                                             */
/*  http://www.visualizationlibrary.org                                               */
/*                                                                                    */
/*  Copyright (c) 2005-2010, Michele Bosi                                             */
/*  All rights reserved.                                                              */
/*                                                                                    */
/*  Redistribution and use in source and binary forms, with or without modification,  */
/*  are permitted provided that the following conditions are met:                     */
/*                                                                                    */
/*  - Redistributions of source code must retain the above copyright notice, this     */
/*  list of conditions and the following disclaimer.                                  */
/*                                                                                    */
/*  - Redistributions in binary form must reproduce the above copyright notice, this  */
/*  list of conditions and the following disclaimer in the documentation and/or       */
/*  other materials provided with the distribution.                                   */
/*                                                                                    */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND   */
/*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE            */
/*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR  */
/*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    */
/*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      */
/*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    */
/*  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           */
/*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     */
/*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                      */
/*                                                                                    */
/**************************************************************************************/

#ifndef VLRenderingApplet_INCLUDE_ONCE
#define VLRenderingApplet_INCLUDE_ONCE

// VL
#include <vlCore/ResourceDatabase.hpp>

#include <vlCore/VisualizationLibrary.hpp>

#include <vlGraphics/Applet.hpp>
#include <vlGraphics/Rendering.hpp>

#include <vlGraphics/Text.hpp>
#include <vlGraphics/FontManager.hpp>
#include <vlGraphics/GLSL.hpp>



#include <vlVolume/RaycastVolume.hpp>
#include <vlVolume/VolumeUtils.hpp>


// MITK
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkBaseData.h>
#include <mitkProperties.h>
#include <mitkOclResourceService.h>

// Microservices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// VTK
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>

//using namespace vl;

// forward decl
class vl::OpenGLContext;

class VLRenderingApplet : public vl::Applet
{
public:
  VLRenderingApplet(vl::OpenGLContext* context);
  virtual ~VLRenderingApplet();

  void initEvent();
  void updateEvent();
  virtual vl::String appletInfo();

  void updateScene();

  void UpdateThresholdVal( int val );

protected:
  //typedef std::vector<const char*> CStringList;
  //typedef std::vector<size_t> ClSizeList;

private:
  vl::ref<vl::Actor> AddImageActor(mitk::Image::Pointer mitkImg);
  
  void LoadGLSLSourceFromResources(const char* filename, vl::String &source);

  

private:
  //vl::Time           m_FPSTimer;
  vl::ref<vl::Uniform>   m_ThresholdVal;
  vl::ref<vl::Text>      mText;
  vl::ref<vl::Transform> mTransform1;
  vl::ref<vl::Transform> mTransform2;



  

  
  

  
};

#endif
