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

#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIScrollViewContainer);


UIScrollViewContainer::UIScrollViewContainer(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	scrollOrigin(0, 0)
{
	this->SetInputEnabled(true);
	this->SetMultiInput(true);
	this->SetDebugDraw(true);
}

UIScrollViewContainer::~UIScrollViewContainer()
{
}

UIControl* UIScrollViewContainer::Clone()
{
	UIScrollViewContainer *t = new UIScrollViewContainer(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollViewContainer::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	
	UIScrollViewContainer* t = (UIScrollViewContainer*) srcControl;
		
	scrollOrigin = t->scrollOrigin;
}

void UIScrollViewContainer::StartScroll(Vector2 _startScrollPosition)
{
	scrollStartInitialPosition = _startScrollPosition;
	scrollStartPosition = _startScrollPosition;
	scrollCurrentPosition = _startScrollPosition;
	scrollStartMovement = false;
}

void UIScrollViewContainer::ProcessScroll(Vector2 _currentScrollPosition)
{
	scrollCurrentPosition = _currentScrollPosition;

	//	This check is required on iPhone, to avoid bugs in movement.	
#if defined(__DAVAENGINE_IPHONE__)
	Vector2 lineLenght = scrollCurrentPosition - scrollStartInitialPosition;

	if (lineLenght.Length() >= SCROLL_BEGIN_PIXELS)
#endif
	{
		//touchStartTime = 0;
		if (!scrollStartMovement) scrollStartPosition = scrollCurrentPosition;
		scrollCurrentShift = Vector2((scrollCurrentPosition.x - scrollStartPosition.x),  (scrollCurrentPosition.y - scrollStartPosition.y));
		scrollStartMovement = true;
	}

	ScrollToPosition(scrollCurrentShift);
}

void UIScrollViewContainer::EndScroll()
{
	if (scrollStartMovement)
	{
		scrollOrigin.x = scrollCurrentShift.x;
		scrollOrigin.y = scrollCurrentShift.y;
	}

	scrollCurrentShift.x = 0;
	scrollCurrentShift.y = 0;
}

void UIScrollViewContainer::ScrollToPosition(const DAVA::Vector2 &position)
{
	Rect rect = this->GetRect();	
	rect.x = scrollOrigin.x + position.x;
	rect.y = scrollOrigin.y + position.y;	
	this->SetRect(rect);
}

void UIScrollViewContainer::Input(UIEvent *currentTouch)
{
	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();
	
	if(1 == touches.size())
	{
		switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				
				Vector2 start = currentTouch->point;
				StartScroll(start);
				state = STATE_SCROLL;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						// perform scrolling
						ProcessScroll(currentTouch->point);
					}
				}
			}
				break;
			case UIEvent::PHASE_ENDED:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						EndScroll();
						state = STATE_DECCELERATION;
					}
				}
			}
				break;
		}
	}
}

void UIScrollViewContainer::Update(float32 timeElapsed)
{
	if (state == STATE_NONE)
	{
		return;
	}

	if(state == STATE_DECCELERATION)
	{
		Rect contentRect = this->GetRect();
		scrollOrigin = Vector2(contentRect.x, contentRect.y);
	}
	
	if(state != STATE_ZOOM && state != STATE_SCROLL) 
	{
		Rect contentRect = this->GetRect();
		Rect parentRect = this->GetParent()->GetRect();
	
		float32 shiftSizeX = abs(contentRect.x) + parentRect.dx;
		float32 shiftSizeY = abs(contentRect.y) + parentRect.dy;
	
		if (contentRect.x > 0)
		{
			contentRect.x -= 1;
			this->SetRect(contentRect);
		}
		else if (shiftSizeX > contentRect.dx)
		{
			contentRect.x += 1;
			this->SetRect(contentRect);
		}
	
		if (contentRect.y > 0)
		{
			contentRect.y -=  1;
			this->SetRect(contentRect);
		}
		else if (shiftSizeY > contentRect.dy)
		{
			contentRect.y += 1;
			this->SetRect(contentRect);
		}
	}
}

YamlNode * UIScrollViewContainer::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
	
    // Control Type
	SetPreferredNodeType(node, "UIScrollViewContainer");
	// Save scroll view container childs including all sub-childs
	SaveChilds(this, loader, node);
    
    return node;
}

void UIScrollViewContainer::SaveChilds(UIControl *parent, UIYamlLoader * loader, YamlNode * parentNode)
{
	List<UIControl*> childslist = parent->GetRealChildren();
	for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
    {
       	UIControl *childControl = (UIControl*)(*it);
	   	if (!childControl)
	   		continue;

		YamlNode* childNode = childControl->SaveToYamlNode(loader);		
		parentNode->AddNodeToMap(childControl->GetName(), childNode);
		// Save sub-childs
		SaveChilds(childControl, loader, childNode);
	}
}

};