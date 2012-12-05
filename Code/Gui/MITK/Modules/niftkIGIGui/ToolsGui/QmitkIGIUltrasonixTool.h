/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2012-07-25 07:31:59 +0100 (Wed, 25 Jul 2012) $
 Revision          : $Revision: 9401 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKIGIULTRASONIXTOOL_H
#define QMITKIGIULTRASONIXTOOL_H

#include "niftkIGIGuiExports.h"
#include "QmitkQImageToMitkImageFilter.h"
#include "QmitkIGINiftyLinkDataSource.h"
#include "mitkImage.h"
#include "mitkDataNode.h"

/**
 * \class QmitkIGIUltrasonixTool
 * \brief Implements a QmitkIGINiftyLinkDataSource interface to receive and process messages from the Ultrasonix scanner.
 */
class NIFTKIGIGUI_EXPORT QmitkIGIUltrasonixTool : public QmitkIGINiftyLinkDataSource
{
  Q_OBJECT

public:

  mitkClassMacro(QmitkIGIUltrasonixTool, QmitkIGINiftyLinkDataSource);
  itkNewMacro(QmitkIGIUltrasonixTool);

  /**
   * \brief We store the node name here so other classes can refer to it.
   */
  static const std::string ULTRASONIX_TOOL_2D_IMAGE_NAME;

  /**
   * \brief Retrieves the motor position from the most recent data available.
   */
  float GetMotorPos();

  /**
   * \brief Copies out the internal matrix.
   */
  void GetImageMatrix(igtl::Matrix4x4& matrix);

  /**
   * \brief Defined in base class, so we check that the data is in fact a OIGTLMessageType containing tracking data.
   * \see mitk::IGIDataSource::CanHandleData()
   */
  virtual bool CanHandleData(mitk::IGIDataType* data) const;

  /**
   * \brief Defined in base class, this is the method where we do the update based on the available data.
   * \see mitk::IGIDataSource::Update()
   */
  virtual bool Update(mitk::IGIDataType* data);

public slots:

  /**
   * \brief Main message handler routine for this tool.
   */
  virtual void InterpretMessage(OIGTLMessage::Pointer msg);

signals:

  void StatusUpdate(QString statusUpdateMessage);
  void UpdatePreviewImage(OIGTLMessage::Pointer msg);

protected:

  QmitkIGIUltrasonixTool(); // Purposefully hidden.
  virtual ~QmitkIGIUltrasonixTool(); // Purposefully hidden.

  QmitkIGIUltrasonixTool(const QmitkIGIUltrasonixTool&); // Purposefully not implemented.
  QmitkIGIUltrasonixTool& operator=(const QmitkIGIUltrasonixTool&); // Purposefully not implemented.

private:

  /**
   * \brief Called by the base class Update message, which processes the message
   * by extracting an image, and converting it appropriate to the associated image
   * in the data storage.
   */
  void HandleImageData(OIGTLMessage* msg);

  /**
   * \brief Utility method to save both the matrix and the image.
   */
  void SaveImageMessage (OIGTLImageMessage::Pointer imageMsg);

  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  QmitkQImageToMitkImageFilter::Pointer m_Filter;
  igtl::Matrix4x4 m_ImageMatrix;
  float m_RadToDeg;

}; // end class

#endif

