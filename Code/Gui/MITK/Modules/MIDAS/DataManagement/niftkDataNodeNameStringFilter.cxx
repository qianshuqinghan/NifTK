/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkDataNodeNameStringFilter.h"

#include <niftkMorphologicalSegmentorPipelineManager.h>
#include <niftkPaintbrushTool.h>
#include <niftkPolyTool.h>
#include <niftkTool.h>

namespace niftk
{

//-----------------------------------------------------------------------------
MIDASDataNodeNameStringFilter::MIDASDataNodeNameStringFilter()
{
  this->AddToList("One of FeedbackContourTool's feedback nodes");
  this->AddToList("niftkContourTool");
  this->AddToList(MIDASTool::SEEDS_NAME);
  this->AddToList(MIDASTool::CONTOURS_NAME);
  this->AddToList(MIDASTool::REGION_GROWING_NAME);
  this->AddToList(MIDASTool::PRIOR_CONTOURS_NAME);
  this->AddToList(MIDASTool::NEXT_CONTOURS_NAME);
  this->AddToList(MIDASTool::DRAW_CONTOURS_NAME);
  this->AddToList(PaintbrushTool::EROSIONS_SUBTRACTIONS_NAME);
  this->AddToList(PaintbrushTool::EROSIONS_ADDITIONS_NAME);
  this->AddToList(PaintbrushTool::DILATIONS_SUBTRACTIONS_NAME);
  this->AddToList(PaintbrushTool::DILATIONS_ADDITIONS_NAME);
  this->AddToList(MorphologicalSegmentorPipelineManager::SEGMENTATION_OF_LAST_STAGE_NAME);
  this->AddToList(PolyTool::MIDAS_POLY_TOOL_ANCHOR_POINTS);
  this->AddToList(PolyTool::MIDAS_POLY_TOOL_PREVIOUS_CONTOUR);
  this->AddToList("Paintbrush_Node");
  this->AddToList("widget1Plane");
  this->AddToList("widget2Plane");
  this->AddToList("widget3Plane");
  this->SetPropertyName("name");
}


//-----------------------------------------------------------------------------
MIDASDataNodeNameStringFilter::~MIDASDataNodeNameStringFilter()
{

}

}
