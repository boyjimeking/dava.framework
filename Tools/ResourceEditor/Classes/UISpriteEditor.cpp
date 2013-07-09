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


#include "UISpriteEditor.h"

namespace DAVA 
{
	
REGISTER_CLASS(UISpriteEditor);

UISpriteEditor::UISpriteEditor(const Rect &rect, bool rectInAbsoluteCoordinates)
	: UIControl(rect, rectInAbsoluteCoordinates)
{
	//spritePreview = new UIControl(Rect(5, 5, rect.dx - 10, rect.dy - 80));
	//spritePreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
	//AddControl(spritePreview);
	
	//
	// UIYamlLoader::Load("~res:/Screens/SpriteEditor.yaml");
	// 
	spritePreview = 0;
	frameSlider = 0;
	frameCount = 0;
	currentFrame = 0;
	spriteScrollView = 0;
}

UISpriteEditor::~UISpriteEditor()
{
	SafeRelease(spriteScrollView);
	SafeRelease(frameSlider);
	SafeRelease(spritePreview);
	SafeRelease(frameCount);
	SafeRelease(currentFrame);
}
	
void UISpriteEditor::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	UIYamlLoader::Load(this, "~res:/Screens/SpriteEditor.yaml");
	
}
    
void UISpriteEditor::LoadFromYamlNodeCompleted() 
{
	spriteScrollView = SafeRetain(dynamic_cast<UIScrollView*> (FindByName("spriteScrollView")));
	spritePreview = SafeRetain(dynamic_cast<UIControl*> (FindByName("spritePreview")));
	frameSlider = SafeRetain(dynamic_cast<UISlider*> (FindByName("frameSlider")));
	frameCount = SafeRetain(dynamic_cast<UIStaticText*> (FindByName("frameCount")));
	currentFrame = SafeRetain(dynamic_cast<UIStaticText*> (FindByName("currentFrame")));
	
	frameSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &UISpriteEditor::OnCurrentFrameChanged));
	float32 value = frameSlider->GetValue();
	Logger::Debug("%f", value);
	frameSlider->SetValue(0.0f);
	//frameSlider->SetEventsContinuos(false);
	//Rect rect = GetRect();
	//spritePreview->SetRect(Rect(5, 25, rect.dx - 10, rect.dy - 85));
}; 

void UISpriteEditor::SetPreviewSprite(const FilePath & spriteName)
{
	Sprite * sprite = Sprite::Create(spriteName);

	spriteScrollView->SetContentSize(sprite->GetSize());
	spritePreview->SetSprite(sprite, 0);
	spritePreview->SetDebugDraw(true);
	spritePreview->SetRect(Rect(0, 0, sprite->GetWidth(), sprite->GetHeight()));
	/*Sprite * sprite = spritePreview->GetSprite();
	if ((sprite->GetWidth() >= spritePreview->GetSize().dx) || 
		(sprite->GetHeight() >= spritePreview->GetSize().dy))
	{
		spritePreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
	}else {
		spritePreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
	}*/
	currentFrame->SetText(Format(L"current frame: %d", 0));
	frameCount->SetText(Format(L"frame count: %d", sprite->GetFrameCount()));
	
	SafeRelease(sprite);
}
	
void UISpriteEditor::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
	UIControl::SetRect(rect, rectInAbsoluteCoordinates);
}	


UIControl * UISpriteEditor::GetSpritePreview()
{
	return spritePreview;
}

void UISpriteEditor::OnCurrentFrameChanged(BaseObject * obj, void * userData, void * systemData)
{
	UISlider * slider = dynamic_cast<UISlider*> (obj);
	if (!slider)return;
	if (!spritePreview->GetSprite())return;
	
	int32 frame = Clamp((int32)(slider->GetValue() * spritePreview->GetSprite()->GetFrameCount()), 
						0, 
						spritePreview->GetSprite()->GetFrameCount() - 1);
	spritePreview->GetBackground()->SetFrame(frame);
	currentFrame->SetText(Format(L"current frame: %d", frame));
}

	
};

