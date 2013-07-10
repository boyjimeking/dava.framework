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

#include "VisibilityToolActions.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/VisibilityToolProxy.h"

ActionSetVisibilityPoint::ActionSetVisibilityPoint(Image* originalImage,
												   Sprite* cursorSprite,
												   VisibilityToolProxy* visibilityToolProxy,
												   const Vector2& visibilityPoint)
:	CommandAction(CMDID_SET_VISIBILITY_POINT, "Set Visibility Point")
{
//	this->undoImage = SafeRetain(originalImage);
	this->cursorSprite = SafeRetain(cursorSprite);
	this->visibilityToolProxy = SafeRetain(visibilityToolProxy);
//	this->undoVisibilityPoint = visibilityToolProxy->GetVisibilityPoint();
	this->redoVisibilityPoint = visibilityPoint;
//	this->undoVisibilityPointSet = visibilityToolProxy->IsVisibilityPointSet();
}

ActionSetVisibilityPoint::~ActionSetVisibilityPoint()
{
//	SafeRelease(undoImage);
	SafeRelease(cursorSprite);
	SafeRelease(visibilityToolProxy);
}

void ActionSetVisibilityPoint::Redo()
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();
	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	cursorSprite->SetPosition(redoVisibilityPoint - cursorSprite->GetSize() / 2.f);
	cursorSprite->Draw();

	RenderManager::Instance()->RestoreRenderTarget();

	visibilityToolProxy->UpdateVisibilityPointSet(true);
	visibilityToolProxy->UpdateRect(Rect(0.f, 0.f, visibilityToolSprite->GetWidth(), visibilityToolSprite->GetHeight()));
	visibilityToolProxy->SetVisibilityPoint(redoVisibilityPoint);
}

//void ActionSetVisibilityPoint::Undo()
//{
//	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();
//	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
//	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
//	
//	if (undoImage)
//	{
//		Texture* drawTexture = Texture::CreateFromData(undoImage->GetPixelFormat(),
//													   undoImage->GetData(),
//													   undoImage->GetWidth(),
//													   undoImage->GetHeight(),
//													   false);
//		Sprite* drawSprite = Sprite::CreateFromTexture(drawTexture, 0, 0, undoImage->GetWidth(), undoImage->GetHeight());
//		
//		drawSprite->SetPosition(0.f, 0.f);
//		drawSprite->Draw();
//		
//		SafeRelease(drawSprite);
//		SafeRelease(drawTexture);
//		
//		visibilityToolProxy->UpdateRect(Rect(0.f, 0.f, undoImage->GetWidth(), undoImage->GetHeight()));
//	}
//	
//	RenderManager::Instance()->RestoreRenderTarget();
//	
//	visibilityToolProxy->SetVisibilityPoint(undoVisibilityPoint);
//	visibilityToolProxy->UpdateVisibilityPointSet(undoVisibilityPointSet);
//}


ActionSetVisibilityArea::ActionSetVisibilityArea(Image* originalImage,
												 VisibilityToolProxy* visibilityToolProxy,
												 const Rect& updatedRect)
:	CommandAction(CMDID_SET_VISIBILITY_AREA, "Set Visibility Area")
{
	Image* currentImage = visibilityToolProxy->GetSprite()->GetTexture()->CreateImageFromMemory();

//	undoImage = Image::CopyImageRegion(originalImage, updatedRect);
	redoImage = Image::CopyImageRegion(currentImage, updatedRect);

	SafeRelease(currentImage);

	this->visibilityToolProxy = SafeRetain(visibilityToolProxy);
	this->updatedRect = updatedRect;
}

ActionSetVisibilityArea::~ActionSetVisibilityArea()
{
//	SafeRelease(undoImage);
	SafeRelease(redoImage);
	SafeRelease(visibilityToolProxy);
}

void ActionSetVisibilityArea::Redo()
{
	ApplyImage(redoImage);
}

//void ActionSetVisibilityArea::Undo()
//{
//	ApplyImage(undoImage);
//}

void ActionSetVisibilityArea::ApplyImage(DAVA::Image *image)
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();

	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	texture->GeneratePixelesation();
	Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, image->GetWidth(), image->GetHeight());

	RenderManager::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->ClipRect(updatedRect);

	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	sprite->SetPosition(updatedRect.x, updatedRect.y);
	sprite->Draw();

	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->RestoreRenderTarget();

	visibilityToolProxy->UpdateRect(updatedRect);

	SafeRelease(texture);
	SafeRelease(sprite);
}
