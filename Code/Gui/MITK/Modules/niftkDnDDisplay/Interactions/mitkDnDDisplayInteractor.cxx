/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkDnDDisplayInteractor.h"

#include <string.h>

#include <mitkBaseRenderer.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLine.h>
#include <mitkSliceNavigationController.h>
#include <mitkGlobalInteraction.h>

mitk::DnDDisplayInteractor::DnDDisplayInteractor(const std::vector<mitk::BaseRenderer*>& renderers)
: mitk::DisplayInteractor()
, m_Renderers(renderers)
{
  m_SliceNavigationControllers[0] = renderers[0]->GetSliceNavigationController();
  m_SliceNavigationControllers[1] = renderers[1]->GetSliceNavigationController();
  m_SliceNavigationControllers[2] = renderers[2]->GetSliceNavigationController();
}

mitk::DnDDisplayInteractor::~DnDDisplayInteractor()
{
}

void mitk::DnDDisplayInteractor::Notify(InteractionEvent* interactionEvent, bool isHandled)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (std::find(m_Renderers.begin(), m_Renderers.end(), renderer) != m_Renderers.end())
  {
    Superclass::Notify(interactionEvent, isHandled);
  }
}

void mitk::DnDDisplayInteractor::ConnectActionsAndFunctions()
{
  mitk::DisplayInteractor::ConnectActionsAndFunctions();
  CONNECT_FUNCTION("selectPosition", SelectPosition);
  CONNECT_FUNCTION("initZoom", InitZoom);
}

bool mitk::DnDDisplayInteractor::SelectPosition(StateMachineAction* /*action*/, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN << "mitk DnDDisplayInteractor cannot process the event: " << interactionEvent->GetNameOfClass();
    return false;
  }

  // First, check if the slice navigation controllers have a valid geometry,
  // i.e. an image is loaded.
  if (!m_SliceNavigationControllers[0]->GetCreatedWorldGeometry())
  {
    return false;
  }

  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (!renderer->GetFocused())
  {
    mitk::GlobalInteraction::GetInstance()->GetFocusManager()->SetFocused(interactionEvent->GetSender());
  }

  // Selects the point under the mouse pointer in the slice navigation controllers.
  // In the niftkMultiWindowWidget this puts the crosshair to the mouse position, and
  // selects the slice in the two other render window.
  const mitk::Point3D& positionInWorld = positionEvent->GetPositionInWorld();
  m_SliceNavigationControllers[0]->SelectSliceByPoint(positionInWorld);
  m_SliceNavigationControllers[1]->SelectSliceByPoint(positionInWorld);
  m_SliceNavigationControllers[2]->SelectSliceByPoint(positionInWorld);

  return true;
}


bool mitk::DnDDisplayInteractor::ScrollOneUp(StateMachineAction* action, InteractionEvent* interactionEvent)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (!renderer->GetFocused())
  {
    mitk::GlobalInteraction::GetInstance()->GetFocusManager()->SetFocused(interactionEvent->GetSender());
  }
  return Superclass::ScrollOneUp(action, interactionEvent);
}


bool mitk::DnDDisplayInteractor::ScrollOneDown(StateMachineAction* action, InteractionEvent* interactionEvent)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (!renderer->GetFocused())
  {
    mitk::GlobalInteraction::GetInstance()->GetFocusManager()->SetFocused(interactionEvent->GetSender());
  }
  return Superclass::ScrollOneDown(action, interactionEvent);
}


bool mitk::DnDDisplayInteractor::InitZoom(StateMachineAction* action, InteractionEvent* interactionEvent)
{
  BaseRenderer* renderer = interactionEvent->GetSender();
  if (!renderer->GetFocused())
  {
    mitk::GlobalInteraction::GetInstance()->GetFocusManager()->SetFocused(interactionEvent->GetSender());
  }

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN << "mitk DnDDisplayInteractor cannot process the event: " << interactionEvent->GetNameOfClass();
    return false;
  }

  // First, check if the slice navigation controllers have a valid geometry,
  // i.e. an image is loaded.
  if (!m_SliceNavigationControllers[0]->GetCreatedWorldGeometry())
  {
    return false;
  }

  // Selects the point under the mouse pointer in the slice navigation controllers.
  // In the niftkMultiWindowWidget this puts the crosshair to the mouse position, and
  // selects the slice in the two other render window.
  const mitk::Point3D& positionInWorld = positionEvent->GetPositionInWorld();
  m_SliceNavigationControllers[0]->SelectSliceByPoint(positionInWorld);
  m_SliceNavigationControllers[1]->SelectSliceByPoint(positionInWorld);
  m_SliceNavigationControllers[2]->SelectSliceByPoint(positionInWorld);

  // Although the code above puts the crosshair to the mouse pointer position,
  // the two positions are not completely equal because the crosshair is always in
  // the middle of the voxel that contains the mouse position. This slight difference
  // causes that in strong zooming the crosshair moves away from the focus point.
  // So that we zoom around the crosshair, we have to calculate the crosshair position
  // (in world coordinates) and then its projection to the displayed region (in pixels).
  // This will be the focus point during the zooming.
  const mitk::PlaneGeometry* plane1 = m_SliceNavigationControllers[0]->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry* plane2 = m_SliceNavigationControllers[1]->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry* plane3 = m_SliceNavigationControllers[2]->GetCurrentPlaneGeometry();

  mitk::Line3D intersectionLine;
  mitk::Point3D focusPoint3DInMm;
  if (!(plane1 && plane2 && plane1->IntersectionLine(plane2, intersectionLine) &&
        plane3 && plane3->IntersectionPoint(intersectionLine, focusPoint3DInMm)))
  {
    focusPoint3DInMm = positionInWorld;
  }

  mitk::Point2D focusPoint2DInMm;
  mitk::Point2D focusPoint2DInPx;
  mitk::Point2D focusPoint2DInPxUL;

  mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
  displayGeometry->Map(focusPoint3DInMm, focusPoint2DInMm);
  displayGeometry->WorldToDisplay(focusPoint2DInMm, focusPoint2DInPx);
  displayGeometry->DisplayToULDisplay(focusPoint2DInPx, focusPoint2DInPxUL);

  // Create a new position event with the "corrected" position.
  mitk::InteractionPositionEvent::Pointer positionEvent2 = InteractionPositionEvent::New(renderer, focusPoint2DInPxUL);

  return this->Init(action, positionEvent2);
}
