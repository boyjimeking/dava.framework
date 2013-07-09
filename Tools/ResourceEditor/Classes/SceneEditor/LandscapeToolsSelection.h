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

#ifndef __LANDSCAPE_TOOLS_SELECTION_H__
#define __LANDSCAPE_TOOLS_SELECTION_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool;
class LandscapeToolsSelection;
class LandscapeToolsSelectionDelegate
{
public: 
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool) = 0;
};

class LandscapeToolsSelection: public UIControl, public UIListDelegate
{
    
public:
    LandscapeToolsSelection(LandscapeToolsSelectionDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsSelection();

    virtual void WillAppear();
    
    LandscapeTool *Tool();
    
    void SetDelegate(LandscapeToolsSelectionDelegate *newDelegate);

    void SetBodyControl(UIControl *parent);

    void Show();
    void Close();
    
    //UIListDelegate
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);

    
protected:
    
    void OnToolSelected(BaseObject * object, void * userData, void * callerData);
    void OnClose(BaseObject * object, void * userData, void * callerData);
    
    UIControl *GetToolControl(int32 indexAtRow, UIListCell *cell);

    void EnumerateTools();
    void ReleaseTools();

    void UpdateSize();
    
    
    UIControl *parentBodyControl;

    LandscapeTool *selectedTool;
    Vector<LandscapeTool *>tools;
    
    UIList *toolsList;
    UIButton *closeButton;
    
    LandscapeToolsSelectionDelegate *delegate;
};

#endif // __LANDSCAPE_TOOLS_SELECTION_H__