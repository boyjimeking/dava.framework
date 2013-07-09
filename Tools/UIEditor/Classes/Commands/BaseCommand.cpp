/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseCommand.h"
#include "MetadataFactory.h"

using namespace DAVA;

BaseCommand::BaseCommand()
{
 	activePlatform = activeScreen = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* platform = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platform)
		activePlatform = platform->GetId();
	HierarchyTreeNode* screen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (screen)
		activeScreen = screen->GetId();
}

BaseCommand::~BaseCommand()
{
}

BaseMetadata* BaseCommand::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID)
{
    const HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Tree Node is NULL for Tree Node ID %i", treeNodeID);
        return NULL;
    }
    
    UIControl* uiControl = NULL;
    const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (controlNode)
    {
        uiControl = controlNode->GetUIObject();
    }
    
    BaseMetadata* baseMetadata = MetadataFactory::Instance()->GetMetadataForTreeNode(treeNode);
    if (baseMetadata == NULL)
    {
        Logger::Error("Unable to found Hierarchy Tree Node Metadata for node %i while executing Command!",
                      treeNodeID);
        return NULL;
    }
    
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(treeNodeID, uiControl));
    baseMetadata->SetupParams(params);
    
    return baseMetadata;
}

void BaseCommand::ActivateCommandScreen()
{
	HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activePlatform));
	HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));

	HierarchyTreeController::Instance()->UpdateSelection(platform, screen);
}

void BaseCommand::IncrementUnsavedChanges()
{
	HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
	if (!screen)
	{
		return;
	}
	
	screen->IncrementUnsavedChanges();
}

void BaseCommand::DecrementUnsavedChanges()
{
	HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
	if (!screen)
	{
		return;
	}
	
	screen->DecrementUnsavedChanges();
	
}

void BaseCommand::ResetUnsavedChanges()
{
	HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));
	if (!screen)
	{
		return;
	}
	
	screen->ResetUnsavedChanges();
}
