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

#include "UI/UIControlBackground.h"
#include "Debug/DVAssert.h"
#include "UI/UIControl.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

namespace DAVA
{

UIControlBackground::UIControlBackground()
:	spr(NULL)
,	frame(0)
,	align(ALIGN_HCENTER|ALIGN_VCENTER)
,	type(DRAW_ALIGNED)
,	color(1.0f, 1.0f, 1.0f, 1.0f)
,	drawColor(1.0f, 1.0f, 1.0f, 1.0f)
,	leftStretchCap(0)
,	topStretchCap(0)
,	spriteModification(0)
,	colorInheritType(COLOR_IGNORE_PARENT)
,	perPixelAccuracyType(PER_PIXEL_ACCURACY_DISABLED)
,	lastDrawPos(0, 0)
{
	rdoObject = new RenderDataObject();
    vertexStream = rdoObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    texCoordStream = rdoObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
	//rdoObject->SetStream()
}
	
UIControlBackground *UIControlBackground::Clone()
{
	UIControlBackground *cb = new UIControlBackground();
	cb->CopyDataFrom(this);
	return cb;
}
	
void UIControlBackground::CopyDataFrom(UIControlBackground *srcBackground)
{
	SafeRelease(spr);
	spr = SafeRetain(srcBackground->spr);
	frame = srcBackground->frame;
	align = srcBackground->align;
	type = srcBackground->type;
	color = srcBackground->color;
	spriteModification = srcBackground->spriteModification;
	colorInheritType = srcBackground->colorInheritType;
	perPixelAccuracyType = srcBackground->perPixelAccuracyType;
	leftStretchCap = srcBackground->leftStretchCap;
	topStretchCap = srcBackground->topStretchCap;
}


UIControlBackground::~UIControlBackground()
{
	SafeRelease(rdoObject);
	SafeRelease(spr);
}
	
Sprite*	UIControlBackground::GetSprite()
{
	return spr;	
}
int32	UIControlBackground::GetFrame() const
{
	return frame;
}
int32	UIControlBackground::GetAlign() const
{
	return align;
}

int32	UIControlBackground::GetModification() const
{
	return spriteModification;
}

UIControlBackground::eColorInheritType UIControlBackground::GetColorInheritType() const
{
	return (eColorInheritType)colorInheritType;
}


UIControlBackground::eDrawType	UIControlBackground::GetDrawType() const
{
	return (UIControlBackground::eDrawType)type;
}
	
	
void UIControlBackground::SetSprite(const FilePath &spriteName, int32 drawFrame)
{
	Sprite *tempSpr = Sprite::Create(spriteName);
	SetSprite(tempSpr, drawFrame);
	SafeRelease(tempSpr);
}

void UIControlBackground::SetSprite(Sprite* drawSprite, int32 drawFrame)
{
	if (drawSprite == this->spr)
	{
		// Sprite is not changed - update frame only.
		frame = drawFrame;
		return;
	}

	SafeRelease(spr);
	spr = SafeRetain(drawSprite);
	frame =  drawFrame;
}
void UIControlBackground::SetFrame(int32 drawFrame)
{
	DVASSERT(spr);
	frame = drawFrame;
}

void UIControlBackground::SetAlign(int32 drawAlign)
{
	align = drawAlign;
}
void UIControlBackground::SetDrawType(UIControlBackground::eDrawType drawType)
{
	type = drawType;
}

void UIControlBackground::SetModification(int32 modification)
{
	spriteModification = modification;	
}
	
void UIControlBackground::SetColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
	DVASSERT(inheritType >= 0 && inheritType < COLOR_INHERIT_TYPES_COUNT);
	colorInheritType = inheritType;
}
    
void UIControlBackground::SetPerPixelAccuracyType(ePerPixelAccuracyType accuracyType)
{
    perPixelAccuracyType = accuracyType;
}
    
UIControlBackground::ePerPixelAccuracyType UIControlBackground::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}
	
const Color &UIControlBackground::GetDrawColor() const
{
	return drawColor;
}

void UIControlBackground::SetDrawColor(const Color &c)
{
	drawColor = c;
}

void UIControlBackground::SetParentColor(const Color &parentColor)
{
	switch (colorInheritType) 
	{
		case COLOR_MULTIPLY_ON_PARENT:
		{
			drawColor.r = color.r * parentColor.r;
			drawColor.g = color.g * parentColor.g;
			drawColor.b = color.b * parentColor.b;
			drawColor.a = color.a * parentColor.a;
		}
			break;
		case COLOR_ADD_TO_PARENT:
		{
			drawColor.r = Min(color.r + parentColor.r, 1.0f);
			drawColor.g = Min(color.g + parentColor.g, 1.0f);
			drawColor.b = Min(color.b + parentColor.b, 1.0f);
			drawColor.a = Min(color.a + parentColor.a, 1.0f);
		}
			break;
		case COLOR_REPLACE_TO_PARENT:
		{
			drawColor = parentColor;
		}
			break;
		case COLOR_IGNORE_PARENT:
		{
			drawColor = color;
		}
			break;
		case COLOR_MULTIPLY_ALPHA_ONLY:
		{
			drawColor = color;
			drawColor.a = color.a * parentColor.a;
		}
			break;
		case COLOR_REPLACE_ALPHA_ONLY:
		{
			drawColor = color;
			drawColor.a = parentColor.a;
		}
			break;
	}	
}
	



void UIControlBackground::Draw(const UIGeometricData &geometricData)
{
	

	Rect drawRect = geometricData.GetUnrotatedRect();
//	if(drawRect.x > RenderManager::Instance()->GetScreenWidth() || drawRect.y > RenderManager::Instance()->GetScreenHeight() || drawRect.x + drawRect.dx < 0 || drawRect.y + drawRect.dy < 0)
//	{//TODO: change to screen size from control system and sizes from sprite
//		return;
//	}
	
	RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	
	Sprite::DrawState drawState;
	if (spr)
	{
		drawState.frame = frame;
		if (spriteModification) 
		{
			drawState.flags = spriteModification;
		}
//		spr->Reset();
//		spr->SetFrame(frame);
//		spr->SetModification(spriteModification);
	}
	switch (type) 
	{
		case DRAW_ALIGNED:
		{
			if (!spr)break;
			if(align & ALIGN_LEFT)
			{
				drawState.position.x = drawRect.x;
			}
			else if(align & ALIGN_RIGHT)
			{
				drawState.position.x = drawRect.x + drawRect.dx - spr->GetWidth() * geometricData.scale.x;
			}
			else
			{
				drawState.position.x = drawRect.x + ((drawRect.dx - spr->GetWidth() * geometricData.scale.x) * 0.5f) ;
			}
			if(align & ALIGN_TOP)
			{
				drawState.position.y = drawRect.y;
			}
			else if(align & ALIGN_BOTTOM)
			{
				drawState.position.y = drawRect.y + drawRect.dy - spr->GetHeight() * geometricData.scale.y;
			}
			else
			{
				drawState.position.y = drawRect.y + ((drawRect.dy - spr->GetHeight() * geometricData.scale.y + spr->GetDefaultPivotPoint().y * geometricData.scale.y) * 0.5f) ;
			}
			if(geometricData.angle != 0)
			{
				float tmpX = drawState.position.x;
				drawState.position.x = (tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x;
				drawState.position.y = (tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y;
//				spr->SetAngle(geometricData.angle);
				drawState.angle = geometricData.angle;
			}
//			spr->SetPosition(x, y);
			drawState.scale = geometricData.scale;
			drawState.pivotPoint = spr->GetDefaultPivotPoint();
//			spr->SetScale(geometricData.scale);
            //if (drawState.scale.x == 1.0 && drawState.scale.y == 1.0)
            {
                switch(perPixelAccuracyType)
                {
                    case PER_PIXEL_ACCURACY_ENABLED:
                        if(lastDrawPos == drawState.position)
                        {
                            drawState.usePerPixelAccuracy = true;
                        }
                        break;
                    case PER_PIXEL_ACCURACY_FORCED:
                        drawState.usePerPixelAccuracy = true;
                        break;
                    default:
                        break;
                }
            }
			
			lastDrawPos = drawState.position;

			spr->Draw(&drawState);
		}
		break;

		case DRAW_SCALE_TO_RECT:
		{
			if (!spr)break;

			drawState.position = geometricData.position;
			drawState.flags = spriteModification;
			drawState.scale.x = drawRect.dx / spr->GetSize().dx;
			drawState.scale.y = drawRect.dy / spr->GetSize().dy;
			drawState.pivotPoint.x = geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx);
			drawState.pivotPoint.y = geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy);
			drawState.angle = geometricData.angle;
			{
				switch(perPixelAccuracyType)
				{
				case PER_PIXEL_ACCURACY_ENABLED:
					if(lastDrawPos == drawState.position)
					{
						drawState.usePerPixelAccuracy = true;
					}
					break;
				case PER_PIXEL_ACCURACY_FORCED:
					drawState.usePerPixelAccuracy = true;
					break;
				default:
					break;
				}
			}

			lastDrawPos = drawState.position;

//			spr->SetPosition(geometricData.position);
//			spr->SetScale(drawRect.dx / spr->GetSize().dx, drawRect.dy / spr->GetSize().dy);
//			spr->SetPivotPoint(geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx), geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy));
//			spr->SetAngle(geometricData.angle);
			
			spr->Draw(&drawState);
		}
		break;
		
		case DRAW_SCALE_PROPORTIONAL:
        case DRAW_SCALE_PROPORTIONAL_ONE:
		{
			if (!spr)break;
			float32 w, h;
			w = drawRect.dx / (spr->GetWidth() * geometricData.scale.x);
			h = drawRect.dy / (spr->GetHeight() * geometricData.scale.y);
			float ph = spr->GetDefaultPivotPoint().y;
            
            if(w < h)
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
                else
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
            }
            else
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
                else
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
            }
            
			if(align & ALIGN_LEFT)
			{
				drawState.position.x = drawRect.x;
			}
			else if(align & ALIGN_RIGHT)
			{
				drawState.position.x = (drawRect.x + drawRect.dx - w);
			}
			else
			{
				drawState.position.x = drawRect.x + (int32)((drawRect.dx - w) * 0.5) ;
			}
			if(align & ALIGN_TOP)
			{
				drawState.position.y = drawRect.y;
			}
			else if(align & ALIGN_BOTTOM)
			{
				drawState.position.y = (drawRect.y + drawRect.dy - h);
			}
			else
			{
				drawState.position.y = (drawRect.y) + (int32)((drawRect.dy - h + ph) * 0.5) ;
			}
			drawState.scale.x = w / spr->GetWidth();
			drawState.scale.y = h / spr->GetHeight();
//			spr->SetScaleSize(w, h);
			if(geometricData.angle != 0)
			{
				float32 tmpX = drawState.position.x;
				drawState.position.x = ((tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x);
				drawState.position.y = ((tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y);
				drawState.angle = geometricData.angle;
//				spr->SetAngle(geometricData.angle);
			}
//			spr->SetPosition((float32)x, (float32)y);
			{
				switch(perPixelAccuracyType)
				{
				case PER_PIXEL_ACCURACY_ENABLED:
					if(lastDrawPos == drawState.position)
					{
						drawState.usePerPixelAccuracy = true;
					}
					break;
				case PER_PIXEL_ACCURACY_FORCED:
					drawState.usePerPixelAccuracy = true;
					break;
				default:
					break;
				}
			}

			lastDrawPos = drawState.position;
			
			spr->Draw(&drawState);
		}
		break;
		
		case DRAW_FILL:
		{//TODO: add rotation
			RenderHelper::Instance()->FillRect(drawRect);
		}	
		break;
			
		case DRAW_STRETCH_BOTH:
		case DRAW_STRETCH_HORIZONTAL:
		case DRAW_STRETCH_VERTICAL:
			DrawStretched(drawRect);
		break;
		
		case DRAW_TILED:
			DrawTiled(drawRect);
		break;
	}
	
	RenderManager::Instance()->ResetColor();
	
}

	
void UIControlBackground::DrawStretched(const Rect &drawRect)
{
	if (!spr)return;
	Texture *texture = spr->GetTexture(frame);
	
	float32 texX = spr->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE);
	float32 texY = spr->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE);
	float32 texDx = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH);
	float32 texDy = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT);
    float32 texOffX = spr->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE);
    float32 texOffY = spr->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE);

    const float32 spriteWidth = spr->GetWidth();
    const float32 spriteHeight = spr->GetHeight();

    const float32 leftOffset  = leftStretchCap - texOffX;
    const float32 rightOffset = leftStretchCap - ( spriteWidth - texDx - texOffX );
    const float32 topOffset   = topStretchCap  - texOffY;
    const float32 bottomOffset= topStretchCap  - ( spriteHeight - texDy - texOffY );

    const float32 realLeftStretchCap  = Max( 0.0f, leftOffset );
    const float32 realRightStretchCap = Max( 0.0f, rightOffset );
    const float32 realTopStretchCap   = Max( 0.0f, topOffset );
    const float32 realBottomStretchCap= Max( 0.0f, bottomOffset );

    const float32 scaleFactorX = drawRect.dx / spriteWidth;
    const float32 scaleFactorY = drawRect.dy / spriteHeight;
    const float32 x = drawRect.x + Max( 0.0f, -leftOffset ) * scaleFactorX;
    const float32 y = drawRect.y + Max( 0.0f, -topOffset  ) * scaleFactorY;

    const float32 dx = drawRect.dx - ( Max( 0.0f, -leftOffset ) + Max( 0.0f, -rightOffset  ) ) * scaleFactorX;
    const float32 dy = drawRect.dy - ( Max( 0.0f, -topOffset  ) + Max( 0.0f, -bottomOffset ) ) * scaleFactorY;

    const float32 resMulFactor = 1.0f / Core::Instance()->GetResourceToVirtualFactor(spr->GetResourceSizeIndex());
//	if (spr->IsUseContentScale()) 
//	{
		texDx *= resMulFactor;
		texDy *= resMulFactor;
//	}

    const float32 leftCap  = realLeftStretchCap   * resMulFactor;
    const float32 rightCap = realRightStretchCap  * resMulFactor;
    const float32 topCap   = realTopStretchCap    * resMulFactor;
    const float32 bottomCap= realBottomStretchCap * resMulFactor;

	float32 vertices[16 * 2];
	float32 texCoords[16 * 2];
	
    float32 textureWidth = (float32)texture->GetWidth();
    float32 textureHeight = (float32)texture->GetHeight();
	
	int32 vertInTriCount = 18;
	int32 vertCount = 16;

	switch (type) 
	{
		case DRAW_STRETCH_HORIZONTAL:
		{
            vertices[0] = vertices[8]  = x;
            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[4] = vertices[12] = x + dx - realRightStretchCap;

            vertices[2] = vertices[10] = x + realLeftStretchCap;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + dy;
            vertices[6] = vertices[14] = x + dx;

            texCoords[0] = texCoords[8]  = texX / textureWidth;
            texCoords[1] = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[4] = texCoords[12] = (texX + texDx - rightCap) / textureWidth;

            texCoords[2] = texCoords[10] = (texX + leftCap) / textureWidth;
            texCoords[9] = texCoords[11] = texCoords[13] = texCoords[15] = (texY + texDy) / textureHeight;
            texCoords[6] = texCoords[14] = (texX + texDx) / textureWidth;
		}
		break;
		case DRAW_STRETCH_VERTICAL:
		{
            vertices[0] = vertices[2]  = vertices[4]  = vertices[6]  = x;
            vertices[8] = vertices[10] = vertices[12] = vertices[14] = x + dx;

            vertices[1] = vertices[9]  = y;
            vertices[3] = vertices[11] = y + realTopStretchCap;
            vertices[5] = vertices[13] = y + dy - realBottomStretchCap;
            vertices[7] = vertices[15] = y + dy;

            texCoords[0] = texCoords[2]  = texCoords[4]  = texCoords[6]  = texX / textureWidth;
            texCoords[8] = texCoords[10] = texCoords[12] = texCoords[14] = (texX + texDx) / textureWidth;

            texCoords[1] = texCoords[9]  = texY / textureHeight;
            texCoords[3] = texCoords[11] = (texY + topCap) / textureHeight;
            texCoords[5] = texCoords[13] = (texY + texDy - bottomCap) / textureHeight;
            texCoords[7] = texCoords[15] = (texY + texDy) / textureHeight;
		}
		break;
		case DRAW_STRETCH_BOTH:
		{
            vertInTriCount = 18 * 3;
            vertCount = 32;

            vertices[0] = vertices[8]  = vertices[16] = vertices[24] = x;
            vertices[2] = vertices[10] = vertices[18] = vertices[26] = x + realLeftStretchCap;
            vertices[4] = vertices[12] = vertices[20] = vertices[28] = x + dx - realRightStretchCap;
            vertices[6] = vertices[14] = vertices[22] = vertices[30] = x + dx;

            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + realTopStretchCap;
            vertices[17]= vertices[19] = vertices[21] = vertices[23] = y + dy - realBottomStretchCap;
            vertices[25]= vertices[27] = vertices[29] = vertices[31] = y + dy;

            texCoords[0] = texCoords[8]  = texCoords[16] = texCoords[24] = texX / textureWidth;
            texCoords[2] = texCoords[10] = texCoords[18] = texCoords[26] = (texX + leftCap) / textureWidth;
            texCoords[4] = texCoords[12] = texCoords[20] = texCoords[28] = (texX + texDx - rightCap) / textureWidth;
            texCoords[6] = texCoords[14] = texCoords[22] = texCoords[30] = (texX + texDx) / textureWidth;

            texCoords[1]  = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[9]  = texCoords[11] = texCoords[13] = texCoords[15] = (texY + topCap) / textureHeight;
            texCoords[17] = texCoords[19] = texCoords[21] = texCoords[23] = (texY + texDy - bottomCap)  / textureHeight;
            texCoords[25] = texCoords[27] = texCoords[29] = texCoords[31] = (texY + texDy) / textureHeight;
		}
		break;
	}
	
//	if (Core::GetContentScaleFactor() != 1.0 && RenderManager::IsRenderTarget()) 
//	{
//		for (int i = 0; i < vertCount; i++) 
//		{
//			vertices[i] *= Core::GetVirtualToPhysicalFactor();
//	}
//	}


	uint16 indeces[18 * 3] = 
	{
		0, 1, 4, 
		1, 5, 4,
		1, 2, 5,
		2, 6, 5, 
		2, 3, 6,
		3, 7, 6,

		4, 5, 8,
		5, 9, 8,
		5, 6, 9,
		6, 10, 9,
		6, 7, 10,
		7, 11, 10,

		8, 9, 12,
		9, 12, 13,
		9, 10, 13,
		10, 14, 13,
		10, 11, 14,
		11, 15, 14
	};

	vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
	texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords);

	RenderManager::Instance()->SetTexture(texture);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(rdoObject);
	RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, vertInTriCount, EIF_16, indeces);

	/*GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		Logger::Debug("GLError: 0x%x", glErr);
	}*/
}

void UIControlBackground::GenerateCell(float32* vertices, int32 offset, float32 leftStretchCap, float32 topStretchCap, float32 x, float32 y, float32 tWidth, float32 tHeight)
{
	for (int32 i = 0; i < 8; i+=2)
	{
		vertices[offset + i] = (x + (((i % 4) > 0) ? leftStretchCap : 0)) / tWidth;
		vertices[offset + i + 1] = (y + ((i >= 4) ? topStretchCap : 0)) / tHeight;
	}
}

void UIControlBackground::DrawTiled(const Rect &drawRect)
{
	if (!spr)return;
	Texture *texture = spr->GetTexture(frame);
	
	float32 texX = spr->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE);
	float32 texY = spr->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE);
	float32 texDx = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH);
	float32 texDy = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT);

	float32 textureWidth = (float32)texture->GetWidth();
    float32 textureHeight = (float32)texture->GetHeight();
	
	// Don't calculate tiles when texture is damaged
	DVASSERT((textureWidth > 0) && (textureHeight > 0));
	if ((textureWidth <= 0) || (textureHeight <= 0))
		return;
	
	float32 x = drawRect.x;
	float32 y = drawRect.y;
	float32 dx = drawRect.dx;
	float32 dy = drawRect.dy;	
	
	// We have to avoid division on zero
	if ((texDx - leftStretchCap * 2) <= 0 ||  (texDy - topStretchCap * 2) <= 0)
		return;
	// The number of horizontal and vertical "tiles"
	const float32 cellsH =  Max( 0.0f, (dx - leftStretchCap * 2) / (texDx - leftStretchCap * 2) );
	const float32 cellsV =  Max( 0.0f, (dy - topStretchCap * 2) / (texDy - topStretchCap * 2) );
	// The size of a single "tile"
	float32 horizontalStretchCap = texDx - leftStretchCap * 2;
	float32 verticalStretchCap = texDy - topStretchCap * 2;
	
	// The overall horizontal and vertical cells number
	int32 cellsHCount = ceilf(cellsH) + 2;
	int32 cellsVCount = ceilf(cellsV) + 2;
	
	// The number of vertices for cells - 8 coordinates for each cells - 4 "X" and 4 "Y"
	int32 vertexCount = cellsHCount * cellsVCount * 4 * 2;
	// Arrays of vertices and texture coordinates
	float32* vertices = new float32[vertexCount];
	float32* texCoords = new float32[vertexCount];
	 
	memset(vertices, 0,  vertexCount * sizeof(float32));
	memset(texCoords, 0, vertexCount * sizeof(float32));
	
	// Generate coorinates for corner cells
	// Top left corner cell
	GenerateCell(vertices, 0, leftStretchCap, topStretchCap, x, y);
	// Top right corner cell
	GenerateCell(vertices, (cellsHCount - 1) * 8, leftStretchCap, topStretchCap, x + dx - leftStretchCap, y);
	// Bottom left corner cell
	GenerateCell(vertices, (cellsHCount * (cellsVCount - 1)) * 8, leftStretchCap, topStretchCap, x, y + dy - topStretchCap);
	// Bottom right corner cell
	GenerateCell(vertices, (cellsHCount * cellsVCount - 1) * 8, leftStretchCap, topStretchCap, x + dx - leftStretchCap, y + dy - topStretchCap);	
	
	// Generate texture coordinates for cells
	// Top left
	GenerateCell(texCoords, 0, leftStretchCap, topStretchCap,
					texX, texY, textureWidth, textureHeight);
	// Top right
	GenerateCell(texCoords, (cellsHCount - 1) * 8, leftStretchCap, topStretchCap,
					texX + texDx - leftStretchCap, texY, textureWidth, textureHeight);
	// Bottom left
	GenerateCell(texCoords, (cellsHCount * (cellsVCount - 1)) * 8, leftStretchCap, topStretchCap,
					texX, texY + texDy - topStretchCap, textureWidth, textureHeight);
	// Bottom right
	GenerateCell(texCoords, (cellsHCount * cellsVCount - 1) * 8, leftStretchCap, topStretchCap,
					texX + texDx - leftStretchCap, texY + texDy - topStretchCap, textureWidth, textureHeight);
				
	// Horizontal border cells
	for (int32 i = 1; i <= cellsHCount - 2; ++i)
	{	
		float32 lastCellOffset = 0;
		// For the last cell in row - we should calculate offset
		if (i == cellsHCount - 2)
		{		
			lastCellOffset = ((cellsHCount - 2) * horizontalStretchCap + 2 * leftStretchCap) - dx;
			if (lastCellOffset < 0) lastCellOffset = 0;
		}		
		// Left border cell
		GenerateCell(vertices, i * 8, horizontalStretchCap - lastCellOffset, topStretchCap,
					x + leftStretchCap + horizontalStretchCap * (i - 1), y);
		// Right border cell
		GenerateCell(vertices, (i + cellsHCount * (cellsVCount - 1)) * 8, horizontalStretchCap - lastCellOffset, topStretchCap,
					x + leftStretchCap + horizontalStretchCap * (i - 1), y + dy - topStretchCap);
		// Left border cell texture
		GenerateCell(texCoords, i * 8, horizontalStretchCap - lastCellOffset, topStretchCap,
						texX + leftStretchCap, texY, textureWidth, textureHeight);
		// Right border cell texture
		GenerateCell(texCoords, (i + cellsHCount * (cellsVCount - 1)) * 8, horizontalStretchCap - lastCellOffset, topStretchCap,
						texX + leftStretchCap, texY + texDy - topStretchCap, textureWidth, textureHeight);
	}
	
	// Vertical border cells
	for (int32 i = 1; i <= cellsVCount - 2; ++i)
	{
		float32 lastCellOffset = 0;
		// For the last cell in row - we should calculate offset
		if (i == cellsVCount - 2)
		{		
			lastCellOffset = ((cellsVCount - 2) * verticalStretchCap + 2 * topStretchCap) - dy;
			if (lastCellOffset < 0) lastCellOffset = 0;
		}		
		// Top border cell
		GenerateCell(vertices, (i * cellsHCount) * 8, leftStretchCap, verticalStretchCap - lastCellOffset,
					x, y + topStretchCap + verticalStretchCap * (i - 1));
		// Bottom border cell
		GenerateCell(vertices, (i * cellsHCount + cellsHCount - 1) * 8, leftStretchCap, verticalStretchCap - lastCellOffset,
					x + dx - leftStretchCap, y + topStretchCap + verticalStretchCap * (i - 1));
		// Top border cell texture part
		GenerateCell(texCoords, (i * cellsHCount) * 8, leftStretchCap, verticalStretchCap - lastCellOffset,
						texX, texY + topStretchCap, textureWidth, textureHeight);
		// Bottom border cell texture part
		GenerateCell(texCoords, (i * cellsHCount + cellsHCount - 1) * 8, leftStretchCap, verticalStretchCap - lastCellOffset,
						texX + texDx - leftStretchCap, texY + topStretchCap, textureWidth, textureHeight);
	}
	
	// Central cells - tiles
	for (int32 iy = 1; iy <= cellsVCount - 2; ++iy)
	{
		float32 lastCellVOffset = 0;
		// For the last cell in row - we should calculate offset
		if (iy == cellsVCount - 2)
		{		
			lastCellVOffset = ((cellsVCount - 2) * verticalStretchCap + 2 * topStretchCap) - dy;
			if (lastCellVOffset < 0) lastCellVOffset = 0;
		}		
	
		for (int32 ix = 1; ix <= cellsHCount - 2; ++ix)
		{
			float32 lastCellHOffset = 0;
			// For the last cell in row - we should calculate offset
			if (ix == cellsHCount - 2)
			{		
				lastCellHOffset = ((cellsHCount - 2) * horizontalStretchCap + 2 * leftStretchCap) - dx;
				if (lastCellHOffset < 0) lastCellHOffset = 0;
			}		
			// Central cell used for "tiling"
			GenerateCell(vertices, (ix + cellsHCount * iy) * 8, horizontalStretchCap - lastCellHOffset, verticalStretchCap - lastCellVOffset,
						x + leftStretchCap + horizontalStretchCap * (ix - 1), y +  topStretchCap + verticalStretchCap * (iy - 1));
			// Central cell texture part
			GenerateCell(texCoords, (ix + cellsHCount * iy) * 8, horizontalStretchCap - lastCellHOffset, verticalStretchCap - lastCellVOffset,
						texX + leftStretchCap, texY + topStretchCap,
						textureWidth, textureHeight);
		}
	}
	
	// The number of triangles and array of triangle points
	int32 vertInTriCount = cellsHCount * cellsVCount * 6;
	uint32 *indeces = new uint32[vertInTriCount];

	memset(indeces, 0, vertInTriCount * sizeof(uint32));

	// Generate triangles points - first 2 triangles -  0,1,2, 1,3,2
	int32 a = 0;
	for (int32 i1 = 0; i1 < cellsVCount; ++i1)
	{
		for (int32 i2 = 0; i2 < cellsHCount; ++i2)
		{
			indeces[(i2 + i1 * cellsHCount) * 6 + 0] = a;
			indeces[(i2 + i1 * cellsHCount) * 6 + 1] = a + 1;
			indeces[(i2 + i1 * cellsHCount) * 6 + 2] = a + 2;
		
			indeces[(i2 + i1 * cellsHCount) * 6 + 3] = a + 1;
			indeces[(i2 + i1 * cellsHCount) * 6 + 4] = a + 3;
			indeces[(i2 + i1 * cellsHCount) * 6 + 5] = a + 2;
			a += 4;
		}
	}
	
	vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
	texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords);

	RenderManager::Instance()->SetTexture(texture);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
	RenderManager::Instance()->SetRenderData(rdoObject);
	RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, vertInTriCount, EIF_32, indeces);
	
	SafeDeleteArray(indeces);
	SafeDeleteArray(texCoords);
	SafeDeleteArray(vertices);	
}

void UIControlBackground::SetLeftRightStretchCap(float32 _leftStretchCap)
{
	leftStretchCap = _leftStretchCap;
}

void UIControlBackground::SetTopBottomStretchCap(float32 _topStretchCap)
{
	topStretchCap = _topStretchCap;
}
	
float32 UIControlBackground::GetLeftRightStretchCap() const
{
    return leftStretchCap;
}
	
float32 UIControlBackground::GetTopBottomStretchCap() const
{
    return topStretchCap;
}	

};