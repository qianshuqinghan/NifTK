/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef NiftyRegParameters_h
#define NiftyRegParameters_h

// broken
#undef _USE_CUDA

#include "RegAladinParameters.h"
#include "RegF3dParameters.h"

#include <QString>
#include <mitkImage.h>

#include <_reg_aladin.h>
#include <_reg_aladin_sym.h>
#include <_reg_tools.h>

#include <_reg_f3d2.h>

#ifdef _USE_CUDA
#include <_reg_f3d_gpu.h>
#endif


/**
 * \class NiftyRegParameters
 * \brief Class to store and initialise the parameters of the affine Aladin registration.
 * \ingroup uk.ac.ucl.cmic.niftyreg
*/


template <class PRECISION_TYPE>
class NiftyRegParameters
{  

  public:  

    NiftyRegParameters();
    virtual ~NiftyRegParameters();

    /// \brief Set the default parameters
    void SetDefaultParameters();

    /// \brief Print the object
    void PrintSelf( std::ostream& os );

    /// Assignment operator
    NiftyRegParameters<PRECISION_TYPE> &operator=(const NiftyRegParameters<PRECISION_TYPE> &p);

    /// \brief Create the Aladin registration object
    reg_aladin<PRECISION_TYPE> 
      *CreateAladinRegistrationObject( mitk::Image *mitkSourceImage, 
				       mitk::Image *mitkTargetImage, 
				       mitk::Image *mitkTargetMaskImage );

    /// \brief Create the non-rigid registration object
    reg_f3d<PRECISION_TYPE> 
      *CreateNonRigidRegistrationObject( mitk::Image *mitkSourceImage, 
					 mitk::Image *mitkTargetImage, 
					 mitk::Image *mitkTargetMaskImage );
    

   /// Deallocate the nifti images used in the registration
    void DeallocateImages( void );


    /// \brief The number of multi-resolution levels
    int m_LevelNumber;
    /// \brief The number of (coarse to fine) multi-resolution levels to use 
    int m_Level2Perform;    

    // Smooth the target image using the specified sigma (mm) 
    float m_TargetSigmaValue;
    // Smooth the source image using the specified sigma (mm)
    float m_SourceSigmaValue;

    /// Flag indicating whether to do an initial rigid registration
    bool m_FlagDoInitialRigidReg;
    /// Flag indicating whether to do the non-rigid registration
    bool m_FlagDoNonRigidReg;

    /// The filename of the initial affine transformation
    QString m_InputAffineName;  // -inaff
    /// Flag indicating whether an initial affine transformation is specified
    bool m_FlagInputAffine;
    /// Flag indicating whether the initial affine transformation is FLIRT
    bool m_FlagFlirtAffine;   // -affFlirt


    /// The 'reg_aladin' parameters
    RegAladinParameters m_AladinParameters;

    /// The 'reg_f3d' parameters
    RegF3dParameters<PRECISION_TYPE> m_F3dParameters;


    /// The reference/target image
    nifti_image *m_ReferenceImage;
    /// The floating/source image
    nifti_image *m_FloatingImage;
    /// The reference/target mask image
    nifti_image *m_ReferenceMaskImage;
    /// The input control grid image
    nifti_image *m_ControlPointGridImage;


};

#ifndef ITK_MANUAL_INSTANTIATION
#include "NiftyRegParameters.txx"
#endif

#endif // NiftyRegParameters_h

