/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/

#include "UI/UISlider.h"
#include "UI/UIButton.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"
#include "Core/Core.h"

namespace DAVA 
{
	
	
REGISTER_CLASS(UISlider);

// Use these names for children buttons to define UISlider in .yaml
static const String UISLIDER_THUMB_SPRITE_CONTROL_NAME = "thumbSpriteControl";
static const String UISLIDER_MIN_SPRITE_CONTROL_NAME = "minSpriteControl";
static const String UISLIDER_MAX_SPRITE_CONTROL_NAME = "maxSpriteControl";

UISlider::UISlider(const Rect & rect)
:	UIControl(rect)
,	bgMin(0)
,	bgMax(0)
,	thumbButton(0)
,	minDrawType(UIControlBackground::DRAW_ALIGNED)
,	maxDrawType(UIControlBackground::DRAW_ALIGNED)
{
	inputEnabled = true;
	isEventsContinuos = true;
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;

	InitSubcontrols();
}
	

UISlider::UISlider() :
	bgMin(0),
	bgMax(0),
	thumbButton(0),
	minDrawType(UIControlBackground::DRAW_ALIGNED),
	maxDrawType(UIControlBackground::DRAW_ALIGNED)
{
	inputEnabled = true;
	isEventsContinuos = true;
	
	InitSubcontrols();
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;
}
	
void UISlider::InitThumb()
{
	if (!thumbButton)
	{
		thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
		thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
		UIControl::AddControl(thumbButton);
	}
	
	thumbButton->SetInputEnabled(false);
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	SetValue(currentValue);
}

void UISlider::AddControl(DAVA::UIControl *control)
{
	// Synchronize the pointers to the buttons each time new control is added.
	UIControl::AddControl(control);

	if (control->GetName() == UISLIDER_THUMB_SPRITE_CONTROL_NAME)
	{
		thumbButton = control;
	}
	else if (control->GetName() == UISLIDER_MIN_SPRITE_CONTROL_NAME)
	{
		bgMin = control;
		PostInitBackground(bgMin);
	}
	else if (control->GetName() == UISLIDER_MAX_SPRITE_CONTROL_NAME)
	{
		bgMax = control;
		PostInitBackground(bgMax);
	}
}
		
void UISlider::InitMinBackground()
{
	if (!bgMin)
	{
		bgMin = new UIControl(this->GetRect());
		bgMin->SetName(UISLIDER_MIN_SPRITE_CONTROL_NAME);
		UIControl::AddControl(bgMin);
		
		PostInitBackground(bgMin);
	}
}

void UISlider::InitMaxBackground()
{
	if (!bgMax)
	{
		bgMax = new UIControl(this->GetRect());
		bgMax->SetName(UISLIDER_MAX_SPRITE_CONTROL_NAME);
		UIControl::AddControl(bgMax);
		
		PostInitBackground(bgMin);
	}
}

void UISlider::ReleaseAllSubcontrols()
{
	if (thumbButton)
	{
		RemoveControl(thumbButton);
		SafeRelease(thumbButton);
	}
	
	if (bgMin)
	{
		RemoveControl(bgMin);
		SafeRelease(bgMin);
	}
	
	if (bgMax)
	{
		RemoveControl(bgMax);
		SafeRelease(bgMax);
	}
}

void UISlider::InitInactiveParts(Sprite* spr)
{
	if(NULL == spr)
	{
		return;
	}

	leftInactivePart = rightInactivePart = (int32)((spr->GetWidth() / 2.0f) + 1.0f); /* 1 px added to align it and make touches easier, with default setup */
}

void UISlider::SetThumb(UIControl *newThumb)
{
    RemoveControl(thumbButton);
    SafeRelease(thumbButton);
    
    thumbButton = SafeRetain(newThumb);
	thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	AddControl(thumbButton);
	
	SetValue(currentValue);
}
    
    
UISlider::~UISlider()
{
	SafeRelease(bgMin);
	SafeRelease(bgMax);
	SafeRelease(thumbButton);
}

void UISlider::SetThumbSprite(Sprite * sprite, int32 frame)
{
	thumbButton->SetSprite(sprite, frame);
	InitInactiveParts(sprite);
}

void UISlider::SetThumbSprite(const FilePath & spriteName, int32 frame)
{
	thumbButton->SetSprite(spriteName, frame);
	InitInactiveParts(thumbButton->GetBackground()->GetSprite());
}

void UISlider::SetMinSprite(Sprite * sprite, int32 frame)
{
	InitMinBackground();
	bgMin->SetSprite(sprite, frame);
}
void UISlider::SetMinSprite(const FilePath & spriteName, int32 frame)
{
	InitMinBackground();
	bgMin->SetSprite(spriteName, frame);
}
	
void UISlider::SetMinDrawType(UIControlBackground::eDrawType drawType)
{
	InitMinBackground();
    bgMin->GetBackground()->SetDrawType(drawType);
}
    
void UISlider::SetMinLeftRightStretchCap(float32 stretchCap)
{
	InitMinBackground();
    bgMin->GetBackground()->SetLeftRightStretchCap(stretchCap);
}
    
void UISlider::SetMaxSprite(Sprite * sprite, int32 frame)
{
	InitMaxBackground();
	bgMax->SetSprite(sprite, frame);
}
	
void UISlider::SetMaxSprite(const FilePath & spriteName, int32 frame)
{
	InitMaxBackground();
	bgMax->SetSprite(spriteName, frame);
}
    
void UISlider::SetMaxDrawType(UIControlBackground::eDrawType drawType)
{
	InitMaxBackground();
    bgMax->GetBackground()->SetDrawType(drawType);
}

void UISlider::SetMaxLeftRightStretchCap(float32 stretchCap)
{
	InitMaxBackground();
    bgMax->GetBackground()->SetLeftRightStretchCap(stretchCap);
}

void UISlider::RecalcButtonPos()
{
	if (thumbButton)
	{
		thumbButton->relativePosition.x = Interpolation::Linear((float32)leftInactivePart, size.x - rightInactivePart, minValue, currentValue, maxValue);
		clipPointRelative = thumbButton->relativePosition.x;
	}
}

void UISlider::SetValue(float32 value)
{
	currentValue = value;
	RecalcButtonPos();
}

void UISlider::SetMinValue(float32 value)
{
    minValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMaxValue(float32 value)
{
    maxValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMinMaxValue(float32 _minValue, float32 _maxValue)
{
	minValue = _minValue;
	maxValue = _maxValue;
	SetValue((minValue + maxValue) / 2.0f);
}

void UISlider::Input(UIEvent *currentInput)
{
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)                                        
	if (currentInput->phase == UIEvent::PHASE_MOVE || currentInput->phase == UIEvent::PHASE_KEYCHAR)
		return;
#endif
	
	const Rect & absRect = GetGeometricData().GetUnrotatedRect();
	//absTouchPoint = currentInput->point;
	
	relTouchPoint = currentInput->point;
	relTouchPoint -= absRect.GetPosition();
	
	
	float oldVal = currentValue;
	currentValue = Interpolation::Linear(minValue, maxValue, (float32)leftInactivePart, relTouchPoint.x, size.x - (float32)rightInactivePart);
	
	if(currentValue < minValue)
	{
		currentValue = minValue;
	}
	if(currentValue > maxValue)
	{
		currentValue = maxValue;
	}

	if (isEventsContinuos) // if continuos events
	{
		if(oldVal != currentValue)
		{
			PerformEvent(EVENT_VALUE_CHANGED);
		}
	}else if (currentInput->phase == UIEvent::PHASE_ENDED) 
	{
		/* if not continuos always perform event because last move position almost always the same as end pos */
		PerformEvent(EVENT_VALUE_CHANGED);
	}
			  
			  
	RecalcButtonPos();
}

void UISlider::Draw(const UIGeometricData &geometricData)
{
	const Rect & aRect =  thumbButton->GetGeometricData().GetUnrotatedRect();
	float32 clipPointAbsolute = aRect.x + aRect.dx * 0.5f;
	if (bgMin && bgMin->GetVisible())
	{
		bgMin->GetBackground()->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(Rect(Core::Instance()->GetVirtualScreenXMin(), 0, clipPointAbsolute - Core::Instance()->GetVirtualScreenXMin(), (float32)GetScreenHeight()));
		bgMin->Draw(geometricData);
		RenderManager::Instance()->ClipPop();
	}
	if (bgMax && bgMax->GetVisible())
	{
		bgMax->GetBackground()->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(Rect(clipPointAbsolute, 0, Core::Instance()->GetVirtualScreenXMax() - clipPointAbsolute, (float32)GetScreenHeight()));
		bgMax->Draw(geometricData);
		RenderManager::Instance()->ClipPop();
	}
	
	if (!bgMax && !bgMin)
	{
		UIControl::Draw(geometricData);
	}
}
	
void UISlider::SystemDraw(const UIGeometricData &geometricData)
{
	UIGeometricData drawData;
	drawData.position = relativePosition;
	drawData.size = size;
	drawData.pivotPoint = pivotPoint;
	drawData.scale = scale;
	drawData.angle = angle;
	drawData.AddToGeometricData(geometricData);
	
	if(parent)
	{
		GetBackground()->SetParentColor(parent->GetBackground()->GetDrawColor());
	}
	else
	{
		GetBackground()->SetParentColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	}
	
	if(clipContents)
	{
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(drawData.GetUnrotatedRect());
	}

	// Draw us.
	if(visible)
	{
		Draw(drawData);
	}
	
	// Draw all the child controls BUT the backgrounds.
	List<UIControl*>::iterator it = childs.begin();
	List<UIControl*>::iterator itEnd = childs.end();
	for(; it != itEnd; ++it)
	{
		if ((*it) == bgMin || (*it) == bgMax)
		{
			continue;
		}
		
		(*it)->SystemDraw(drawData);
	}
	
	if(visible)
	{
		DrawAfterChilds(drawData);
	}
	if(clipContents)
	{
		RenderManager::Instance()->ClipPop();
	}

	if (debugDrawEnabled)
	{
		Color oldColor = RenderManager::Instance()->GetColor();
		RenderManager::Instance()->SetColor(debugDrawColor);
		RenderHelper::Instance()->DrawRect(drawData.GetUnrotatedRect());
		RenderManager::Instance()->SetColor(oldColor);
	}
}

List<UIControl* >& UISlider::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(FindByName(UISLIDER_THUMB_SPRITE_CONTROL_NAME));
	realChildren.remove(FindByName(UISLIDER_MIN_SPRITE_CONTROL_NAME));
	realChildren.remove(FindByName(UISLIDER_MAX_SPRITE_CONTROL_NAME));

	return realChildren;
}
	
void UISlider::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	ReleaseAllSubcontrols();
	YamlNode * thumbSpriteNode = node->Get("thumbSprite");

	if (thumbSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create sprite subcontrol.
		InitThumb();
		YamlNode * spriteNode = thumbSpriteNode->Get(0);
		YamlNode * frameNode = thumbSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetThumbSprite(spriteNode->AsString(), frameNode->AsInt());
		}
	}
	
	YamlNode * minSpriteNode = node->Get("minSprite");
	
	if (minSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create min background subcontrol.
		InitMinBackground();
		YamlNode * spriteNode = minSpriteNode->Get(0);
		YamlNode * frameNode = minSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetMinSprite(spriteNode->AsString(), frameNode->AsInt());
		}
	}
	
	YamlNode * maxSpriteNode = node->Get("maxSprite");
	
	if (maxSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create max background subcontrol.
		InitMaxBackground();
		YamlNode * spriteNode = maxSpriteNode->Get(0);
		YamlNode * frameNode = maxSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetMaxSprite(spriteNode->AsString(), frameNode->AsInt());
		}
	}
	
	// Values
	YamlNode * valueNode = node->Get("value");
	
	if (valueNode)
		SetValue(valueNode->AsFloat());
		
	YamlNode * minValueNode= node->Get("minValue");
	
	if (minValueNode)
		SetMinValue(minValueNode->AsFloat());
		
	YamlNode * maxValueNode= node->Get("maxValue");
	
	if (maxValueNode)
		SetMaxValue(maxValueNode->AsFloat());
	
	
	// Load the Min/Max draw types to apply them when the loading will be completed.
	YamlNode * minDrawTypeNode = node->Get("minDrawType");

	if(minDrawTypeNode)
	{
		this->minDrawType =(UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(minDrawTypeNode);
	}
	
	YamlNode * maxDrawTypeNode = node->Get("maxDrawType");
	if(maxDrawTypeNode)
	{
		this->maxDrawType= (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(maxDrawTypeNode);
	}
}

void UISlider::LoadFromYamlNodeCompleted()
{
	// All the UIControls should exist at this moment - just attach to them.
	AttachToSubcontrols();
	bgMin->GetBackground()->SetDrawType(minDrawType);
	bgMax->GetBackground()->SetDrawType(maxDrawType);

	RecalcButtonPos();
}
	
YamlNode * UISlider::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    // Control Type
	SetPreferredNodeType(node, "UISlider");

	// Sprite value
	float32 value = this->GetValue();
	node->Set("value", value);
	
	// Sprite min value
	value = this->GetMinValue();
	node->Set("minValue", value);
	
	// Sprite max value
	value = this->GetMaxValue();
	node->Set("maxValue", value);

	// Yuri Coder, 2013/04/24. Thumb Button and all the backgrounds are child controls now
	// so save them under appropriate names.
	YamlNode* thumbButtonNode = this->thumbButton->SaveToYamlNode(loader);
	node->AddNodeToMap(UISLIDER_THUMB_SPRITE_CONTROL_NAME, thumbButtonNode);

	YamlNode* minBackgroundNode = this->bgMin->SaveToYamlNode(loader);
	node->AddNodeToMap(UISLIDER_MIN_SPRITE_CONTROL_NAME, minBackgroundNode);

	YamlNode* maxBackgroundNode = this->bgMax->SaveToYamlNode(loader);
	node->AddNodeToMap(UISLIDER_MAX_SPRITE_CONTROL_NAME, maxBackgroundNode);

    return node;
}
	
UIControl* UISlider::Clone()
{
	UISlider *t = new UISlider(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UISlider::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	UISlider* t = (UISlider*) srcControl;

	isEventsContinuos = t->isEventsContinuos;
	
	leftInactivePart = t->leftInactivePart;
	rightInactivePart = t->rightInactivePart;
	
	minValue = t->minValue;
	maxValue = t->maxValue;
	
	currentValue = t->currentValue;
	
	if (t->thumbButton)
	{
		thumbButton = t->thumbButton->Clone();
		AddControl(thumbButton);
	}
	if (t->bgMin)
	{
		bgMin = t->bgMin->Clone();
		AddControl(bgMin);
		PostInitBackground(bgMin);
	}
	if (t->bgMax)
	{
		bgMax = t->bgMax->Clone();
		AddControl(bgMax);
		PostInitBackground(bgMax);
	}
	
	clipPointRelative = t->clipPointRelative;
	relTouchPoint = t->relTouchPoint;
}

void UISlider::InitSubcontrols()
{
	InitThumb();
	InitMinBackground();
	InitMaxBackground();
}
	
void UISlider::AttachToSubcontrols()
{
	if (!thumbButton)
	{
		thumbButton = FindByName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
		DVASSERT(thumbButton);
		InitInactiveParts(thumbButton->GetBackground()->GetSprite());
	}
	
	if (!bgMin)
	{
		bgMin = FindByName(UISLIDER_MIN_SPRITE_CONTROL_NAME);
		DVASSERT(bgMin);
	}
	
	if (!bgMax)
	{
		bgMax = FindByName(UISLIDER_MAX_SPRITE_CONTROL_NAME);
		DVASSERT(bgMax);
	}

	PostInitBackground(bgMin);
	PostInitBackground(bgMax);

	// All the controls will be released in the destructor, so need to addref.
	SafeRetain(thumbButton);
	SafeRetain(bgMin);
	SafeRetain(bgMax);
}

List<UIControl*> UISlider::GetSubcontrols()
{
	List<UIControl*> subControls;
	AddControlToList(subControls, UISLIDER_THUMB_SPRITE_CONTROL_NAME);
	AddControlToList(subControls, UISLIDER_MIN_SPRITE_CONTROL_NAME);
	AddControlToList(subControls, UISLIDER_MAX_SPRITE_CONTROL_NAME);

	return subControls;
}

void UISlider::PostInitBackground(UIControl* backgroundControl)
{
	if (!backgroundControl)
	{
		return;
	}
	
	// UISlider's background are drawn in specific way, so they have to be
	// positioned to (0.0) coordinates to avoid input interception. See pls
	// DF-1379 for details.
	backgroundControl->SetInputEnabled(false);
	backgroundControl->SetPosition(Vector2(0.0f, 0.0f));
}
	
} // ns
