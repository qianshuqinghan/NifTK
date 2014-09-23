/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef CUDAManager_h
#define CUDAManager_h

#include "niftkCUDAExports.h"
#include <CUDAImage/CUDAImage.h>
#include <CUDAImage/LightweightCUDAImage.h>
#include <QThread>
#include <QMutex>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <vector>
#include <list>
#include <set>
#include <string>


struct ScopedCUDADevice
{
  ScopedCUDADevice(int dev);
  ~ScopedCUDADevice();
};


// FIXME: better class name
struct ReadAccessor
{
  unsigned int    m_Id;
  cudaEvent_t     m_ReadyEvent;
  const void*     m_DevicePointer;
  std::size_t     m_SizeInBytes;
};


// FIXME: better class name
struct WriteAccessor
{
  void*           m_DevicePointer;
  std::size_t     m_SizeInBytes;

  unsigned int    m_Id;

  cudaEvent_t     m_ReadyEvent;
};


class NIFTKCUDA_EXPORT CUDAManager : public QThread
{
  friend class LightweightCUDAImage;

public:
  /**
   *
   * @throws std::runtime_error if CUDA is not available on the system.
   */
  static CUDAManager* GetInstance();


  ScopedCUDADevice ActivateDevice(int dev);

  cudaStream_t GetStream(const std::string& name);

  ReadAccessor RequestReadAccess(const LightweightCUDAImage& lwci);

  WriteAccessor RequestOutputImage(unsigned int width, unsigned int height, int FIXME_pixeltype);

  // when done with queueing commands to fill output image, call this.
  // it will give you a LightweightCUDAImage that can be stuffed in CUDAImage,
  // which in turn can go to a DataNode.
  LightweightCUDAImage Finalise(WriteAccessor& writeAccessor, cudaStream_t stream);

  LightweightCUDAImage FinaliseAndAutorelease(WriteAccessor& writeAccessor, ReadAccessor& readAccessor, cudaStream_t stream);


protected:
  CUDAManager();
  virtual ~CUDAManager();

  /**
   * Used by LightweightCUDAImage to notify us that all references to it have been dropped,
   * and that it can be placed back onto m_AvailableImagePool for later re-use.
   */
  void AllRefsDropped(LightweightCUDAImage& lwci);


private:
  std::size_t TierToSize(unsigned int tier) const;
  unsigned int SizeToTier(std::size_t size) const;


  /**
   * Called by the CUDA driver when work has finished.
   * The callback is queued by FinaliseAndAutorelease() to release an image.
   * Note: this callback will block work on the stream, therefore the image-ready-events are
   * triggered before the callback so that work on other streams can proceed in parallel.
   */
  static void CUDART_CB StreamCallback(cudaStream_t stream, cudaError_t status, void* userData);


  /**
   * Called by StreamCallback (which in turn is triggered by FinaliseAndAutorelease()) to "release"
   * a previously requested image. 
   */
  void ReleaseReadAccess(unsigned int id);


  static CUDAManager*           s_Instance;
  // there's only one instance of our class (singleton), so a single mutex is ok too.
  static QMutex                 s_Lock;

  unsigned int                  m_LastIssuedId;

  // vector is a size tier, followed by linked list for that tier.
  std::vector<std::list<LightweightCUDAImage> >     m_AvailableImagePool;

  // images currently in use via WriteAccessor, i.e. work is being queued.
  std::map<unsigned int, LightweightCUDAImage>    m_InFlightOutputImages;

  // images that can be requested with RequestReadAccess.
  std::map<unsigned int, LightweightCUDAImage>    m_ValidImages;

  std::map<std::string, cudaStream_t>     m_Streams;
};


#endif // CUDAManager_h
