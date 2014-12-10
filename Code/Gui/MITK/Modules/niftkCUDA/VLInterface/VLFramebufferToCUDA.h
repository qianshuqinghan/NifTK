/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef VLFramebufferToCUDA_h
#define VLFramebufferToCUDA_h

#include "niftkCUDAExports.h"
#include <vlGraphics/FramebufferObject.hpp>
#include <driver_types.h>


class NIFTKCUDA_EXPORT VLFramebufferAdaptor
{

public:
  VLFramebufferAdaptor(vl::FramebufferObject* fbo);
  ~VLFramebufferAdaptor();

  /**
   *
   */
  cudaArray_t Map(cudaStream_t stream);

  /**
   * Invalidates the CUDA array returned by Map() and relinquishes the underlying
   * OpenGL image back to its context.
   */
  void Unmap(cudaStream_t stream);


private:
  vl::ref<vl::FramebufferObject>      m_FBO;
  cudaGraphicsResource_t              m_GfxRes;
};



#endif // VLFramebufferToCUDA
