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

#include "Commands2/CustomColorsCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/CustomColorsProxy.h"

ModifyCustomColorsCommand::ModifyCustomColorsCommand(Image* originalImage,
													 CustomColorsProxy* customColorsProxy,
													 const Rect& updatedRect)
:	Command2(CMDID_MODIFY_CUSTOM_COLORS, "Custom Colors Modification")
{
	this->updatedRect = updatedRect;
	this->customColorsProxy = SafeRetain(customColorsProxy);
	
	Image* currentImage = customColorsProxy->GetSprite()->GetTexture()->CreateImageFromMemory();
	
	undoImage = Image::CopyImageRegion(originalImage, updatedRect);
	redoImage = Image::CopyImageRegion(currentImage, updatedRect);
	
	SafeRelease(currentImage);
}

ModifyCustomColorsCommand::~ModifyCustomColorsCommand()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
	SafeRelease(customColorsProxy);
}

void ModifyCustomColorsCommand::Undo()
{
	ApplyImage(undoImage);
	customColorsProxy->DecrementChanges();
}

void ModifyCustomColorsCommand::Redo()
{
	ApplyImage(redoImage);
	customColorsProxy->IncrementChanges();
}

void ModifyCustomColorsCommand::ApplyImage(DAVA::Image *image)
{
	Sprite* customColorsSprite = customColorsProxy->GetSprite();
	
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());
	
	RenderManager::Instance()->SetRenderTarget(customColorsSprite);
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->ClipRect(updatedRect);
	
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	sprite->SetPosition(updatedRect.x, updatedRect.y);
	sprite->Draw();
	
	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->RestoreRenderTarget();
	
	customColorsProxy->UpdateRect(updatedRect);
	
	SafeRelease(sprite);
	SafeRelease(texture);
}

Entity* ModifyCustomColorsCommand::GetEntity() const
{
	return NULL;
}
