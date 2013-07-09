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

#include "UIListMetadata.h"
#include "EditorListDelegate.h"

namespace DAVA {

UIListMetadata::UIListMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIList* UIListMetadata::GetActiveUIList() const
{
	return dynamic_cast<UIList*>(GetActiveUIControl());
}

void UIListMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	BaseMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIList* list = dynamic_cast<UIList*>(this->treeNodeParams[i].GetUIControl());
		if (list)
		{
			EditorListDelegate *editorList = new EditorListDelegate(list->GetRect());
			list->SetDelegate(editorList);
			list->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
		}
    }	
}

void UIListMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

int UIListMetadata::GetAggregatorID()
{
    if (!VerifyActiveParamID())
    	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();	
	if (!editorList)
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	return editorList->GetAggregatorID();
}

void UIListMetadata::SetAggregatorID(int value)
{
	if (!VerifyActiveParamID())
    	return;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();
	if (!editorList)
		return;

	editorList->SetAggregatorID(value);
	GetActiveUIList()->Refresh();
}

void UIListMetadata::SetActiveControlRect(const Rect& rect)
{
	UIControlMetadata::SetActiveControlRect(rect);
	// Get delegate for current list
	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();	
	if (!editorList)
		return;
	// Update cell width
	float32 width = rect.dx;
	// Calculate new height of list cell by dividing list height on elements count
	float32 height = rect.dy / editorList->ElementsCount(GetActiveUIList());
	// Update cell size with new height and width
	editorList->SetCellSize(Vector2(width, height));
	// Refresh list - and recreate cells
	GetActiveUIList()->Refresh();
}

};