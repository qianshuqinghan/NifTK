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

#include "QmitkIGIUltrasonixTool.h"
#include <QImage>
#include "mitkRenderingManager.h"

NIFTK_IGITOOL_MACRO(NIFTKIGIGUI_EXPORT, QmitkIGIUltrasonixTool, "IGI Ultrasonix Tool");

const std::string QmitkIGIUltrasonixTool::ULTRASONIX_TOOL_2D_IMAGE_NAME = std::string("QmitkIGIUltrasonixTool image");

//-----------------------------------------------------------------------------
QmitkIGIUltrasonixTool::QmitkIGIUltrasonixTool()
: m_Image(NULL)
, m_ImageNode(NULL)
, m_Filter(NULL)
{
  m_Filter = QmitkQImageToMitkImageFilter::New();

  m_ImageNode = mitk::DataNode::New();
  m_ImageNode->SetName(ULTRASONIX_TOOL_2D_IMAGE_NAME);
  m_ImageNode->SetVisibility(true);
  m_ImageNode->SetOpacity(1);
}


//-----------------------------------------------------------------------------
QmitkIGIUltrasonixTool::~QmitkIGIUltrasonixTool()
{
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixTool::InterpretMessage(OIGTLMessage::Pointer msg)
{
  if (msg.data() != NULL &&
      (msg->getMessageType() == QString("IMAGE"))
     )
  {
    this->HandleImageData(msg);
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixTool::HandleImageData(OIGTLMessage::Pointer msg)
{
  OIGTLImageMessage::Pointer imageMsg;
  imageMsg = static_cast<OIGTLImageMessage::Pointer>(msg);

  if (imageMsg.data() != NULL)
  {
    emit UpdatePreviewImage(imageMsg);

    QImage image = imageMsg->getQImage();
    m_Filter->SetQImage(&image);
    m_Filter->Update();
    m_Image = m_Filter->GetOutput();
    m_ImageNode->SetData(m_Image);

    if (!this->GetDataStorage()->Exists(m_ImageNode))
    {
      this->GetDataStorage()->Add(m_ImageNode);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}