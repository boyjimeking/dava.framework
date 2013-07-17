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

#ifndef __Framework__UIScrollViewContainer__
#define __Framework__UIScrollViewContainer__

#include "DAVAEngine.h"

namespace DAVA 
{

class UIScrollViewContainer : public UIControl
{
public:
	UIScrollViewContainer(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~UIScrollViewContainer();
	
	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);
	
public:
	virtual void Update(float32 timeElapsed);
	virtual void Input(UIEvent *currentTouch);
	
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

	Vector2		scrollOrigin;

protected:

	void		StartScroll(Vector2 startScrollPosition);
	void		ProcessScroll(Vector2 currentScrollPosition);
	void		EndScroll();
	void		ScrollToPosition(const Vector2& position);
	void   		SaveChilds(UIControl *parent, UIYamlLoader * loader, YamlNode * parentNode);

	enum 
	{
		STATE_NONE = 0,
		STATE_SCROLL,
		STATE_ZOOM,
		STATE_DECCELERATION,
		STATE_SCROLL_TO_SPECIAL,
	};

	int32		state;
	Vector2		scrollCurrentShift;
	// Scroll information
	Vector2		scrollStartInitialPosition;	// position of click
	Vector2		scrollStartPosition;		// position related to current scroll start pos, can be different from scrollStartInitialPosition
	Vector2		scrollCurrentPosition;	// scroll current position
	bool		scrollStartMovement;
	UIEvent		scrollTouch;
};
};

#endif /* defined(__Framework__UIScrollViewContainer__) */
