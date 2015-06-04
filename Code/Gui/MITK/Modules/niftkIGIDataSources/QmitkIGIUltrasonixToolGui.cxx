/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGIUltrasonixToolGui.h"
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QFileDialog>
#include <Common/NiftyLinkXMLBuilder.h>
#include "QmitkIGIUltrasonixTool.h"
#include "QmitkIGIDataSourceMacro.h"
#include <mitkRenderingManager.h>

NIFTK_IGISOURCE_GUI_MACRO(NIFTKIGIDATASOURCES_EXPORT, QmitkIGIUltrasonixToolGui, "IGI Ultrasonix Tool Gui")

//-----------------------------------------------------------------------------
QmitkIGIUltrasonixToolGui::QmitkIGIUltrasonixToolGui()
: m_UltrasonixTool(NULL)
, m_Image(NULL)
{

}


//-----------------------------------------------------------------------------
QmitkIGIUltrasonixToolGui::~QmitkIGIUltrasonixToolGui()
{
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixToolGui::OnFlippingChanged()
{
  if (m_UltrasonixTool != NULL)
  {
    m_UltrasonixTool->SetFlipHorizontally(this->m_FlipHorizontallyCheckBox->isChecked());
    m_UltrasonixTool->SetFlipVertically(this->m_FlipVerticallyCheckBox->isChecked());
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixToolGui::InitializeImage()
{
  if (m_UltrasonixTool != NULL)
  {
    mitk::DataStorage* dataStorage = m_UltrasonixTool->GetDataStorage();
    assert(dataStorage);

    mitk::DataNode* node = dataStorage->GetNamedNode(QmitkIGIUltrasonixTool::ULTRASONIX_IMAGE_NAME);
    if (node != NULL)
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image != NULL)
      {
        mitk::RenderingManager::GetInstance()->InitializeView(this->m_RenderWindow->GetRenderWindow(), image->GetGeometry());
      }
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixToolGui::Initialize(QWidget* /*parent*/, ClientDescriptorXMLBuilder* /*config*/)
{
  setupUi(this);

  connect(this->m_FlipHorizontallyCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnFlippingChanged()));
  connect(this->m_FlipVerticallyCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnFlippingChanged()));

  if (this->GetSource() != NULL)
  {
    m_UltrasonixTool = dynamic_cast<QmitkIGIUltrasonixTool*>(this->GetSource());
    assert(m_UltrasonixTool);

    if (m_UltrasonixTool != NULL)
    {
      this->m_FlipHorizontallyCheckBox->setChecked(m_UltrasonixTool->GetFlipHorizontally());
      this->m_FlipVerticallyCheckBox->setChecked(m_UltrasonixTool->GetFlipVertically());

      mitk::DataStorage* dataStorage = m_UltrasonixTool->GetDataStorage();
      assert(dataStorage);

      this->m_RenderWindow->GetRenderer()->SetDataStorage(dataStorage);
      mitk::BaseRenderer::GetInstance(this->m_RenderWindow->GetRenderWindow())->SetMapperID(mitk::BaseRenderer::Standard2D);

      this->InitializeImage();
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIUltrasonixToolGui::Update()
{
  if (m_UltrasonixTool != NULL)
  {
    float motorPosition = m_UltrasonixTool->GetCurrentMotorPosition();
    m_MotorPositionLCDLabel->display(motorPosition);
    m_MotorPositionLCDLabel->repaint();
  }
  this->InitializeImage();
}
