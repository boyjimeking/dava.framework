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

#ifndef __DAVAENGINE_UISCROLLVIEW_H__
#define __DAVAENGINE_UISCROLLVIEW_H__

#include "DAVAEngine.h"

namespace DAVA
{

class UIScrollViewContainer;

class UIScrollView : public UIControl
{
public:
	UIScrollView(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~UIScrollView();
	
	virtual void AddControl(UIControl *control);
	virtual List<UIControl* >& GetRealChildren();
    virtual List<UIControl* > GetSubcontrols();
	
	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);
	
	virtual void SetRect(const Rect &rect, bool rectInAbsoluteCoordinates = FALSE);
	virtual void SetSize(const Vector2 &newSize);
	
	void SetPadding(const Vector2 & padding);
	const Vector2 GetPadding() const;
	
	const Vector2 GetContentSize() const;
	
	void RecalculateContentSize();
protected:
	virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

	Vector2 GetMaxSize(UIControl *control, Vector2 currentMaxSize, Vector2 parentShift);

	UIScrollViewContainer *scrollContainer;

private:
	void FindRequiredControls();

};
};

#endif //__DAVAENGINE_UISCROLLVIEW__