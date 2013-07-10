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
 =====================================================================================*/

#ifndef __RESOURCEEDITORQT__LANDSCAPEPROXY__
#define __RESOURCEEDITORQT__LANDSCAPEPROXY__

#include "DAVAEngine.h"
#include "LandscapeEditor/EditorLandscape.h"

using namespace DAVA;

class CustomLandscape;

class LandscapeProxy: public BaseObject
{
public:
	enum eLandscapeMode
	{
		MODE_CUSTOM_LANDSCAPE = 0,
		MODE_ORIGINAL_LANDSCAPE,

		MODES_COUNT
	};

	LandscapeProxy(Landscape* landscape);
	virtual ~LandscapeProxy();

	void SetMode(LandscapeProxy::eLandscapeMode mode);
	void SetDisplayingTexture(Texture* texture);
	
	void SetRenderer(LandscapeRenderer* renderer);
	LandscapeRenderer* GetRenderer();

	AABBox3 GetLandscapeBoundingBox();
	Texture* GetLandscapeTexture(Landscape::eTextureLevel level);
	
	void SetTilemaskTexture(Texture* texture);
	void SetTilemaskTextureEnabled(bool enabled);

	void SetNotPassableTexture(Texture* texture);
	void SetNotPassableTextureEnabled(bool enabled);
	
	void SetCustomColorsTexture(Texture* texture);
	void SetCustomColorsTextureEnabled(bool enabled);
	
	void SetVisibilityCheckToolTexture(Texture* texture);
	void SetVisibilityCheckToolTextureEnabled(bool enabled);

	RenderObject* GetRenderObject();
	void SetHeightmap(Heightmap* heightmap);

	void CursorEnable();
	void CursorDisable();
	void SetCursorTexture(Texture* texture);
	void SetBigTextureSize(float32 size);
	void SetCursorScale(float32 scale);
	void SetCursorPosition(const Vector2& position);

	void ApplyTilemask();
	void UpdateFullTiledTexture(bool force = false);

protected:
	enum eTextureType
	{
		TEXTURE_TYPE_TILEMASK = 0,
		TEXTURE_TYPE_NOT_PASSABLE,
		TEXTURE_TYPE_CUSTOM_COLORS,
		TEXTURE_TYPE_VISIBILITY_CHECK_TOOL,
		
		TEXTURE_TYPES_COUNT
	};
	
	Texture* texturesToBlend[TEXTURE_TYPES_COUNT];
	bool texturesEnabled[TEXTURE_TYPES_COUNT];
	
	Texture* displayingTexture;
	Landscape* baseLandscape;
	CustomLandscape* customLandscape;

	eLandscapeMode mode;
	
	void UpdateDisplayedTexture();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEPROXY__) */
