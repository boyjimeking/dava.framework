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

#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__

#include "Entity/SceneSystem.h"
#include "EditorScene.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;
class LandscapeEditorDrawSystem;

class HeightmapEditorSystem: public DAVA::SceneSystem
{
public:
	static const float32 MAX_STRENGTH;
	
	enum eHeightmapDrawType
	{
		HEIGHTMAP_DRAW_RELATIVE = 0,
		HEIGHTMAP_DRAW_AVERAGE,
		HEIGHTMAP_DRAW_ABSOLUTE_DROPPER,
		HEIGHTMAP_DROPPER,
		HEIGHTMAP_COPY_PASTE,
		
		HEIGHTMAP_DRAW_TYPES_COUNT
	};
	
	HeightmapEditorSystem(Scene* scene);
	virtual ~HeightmapEditorSystem();
	
	bool EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	bool IsLandscapeEditingEnabled() const;
	
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	
	void SetBrushSize(int32 brushSize);
	void SetStrength(float32 strength);
	void SetAverageStrength(float32 averageStrength);
	void SetToolImage(const FilePath& toolImagePath);
	void SetDrawingType(eHeightmapDrawType type);

protected:
	bool enabled;
	
	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;
	
	int32 landscapeSize;
	Texture* cursorTexture;
	Texture* squareTexture;
	uint32 cursorSize;
	uint32 curToolSize;
	Image* toolImage;
	
	eHeightmapDrawType drawingType;
	float32 strength;
	float32 averageStrength;
	bool inverseDrawingEnabled;
	FilePath toolImagePath;
	
	float32 curHeight;
	bool isIntersectsLandscape;
	Vector2 cursorPosition;
	Vector2 copyPasteFrom;
	Vector2 copyPasteTo;
	
	Rect updatedRectAccumulator;
	
	bool editingIsEnabled;
	
	Heightmap* originalHeightmap;
	
	void UpdateCursorPosition();
	void UpdateToolImage(bool force = false);
	void UpdateBrushTool(float32 timeElapsed);
	Image* CreateToolImage(int32 sideSize, const FilePath& filePath);
	
	void AddRectToAccumulator(const Rect& rect);
	void ResetAccumulatorRect();
	Rect GetUpdatedRect();
	
	void StoreOriginalHeightmap();
	void CreateUndoPoint();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORSYSTEM__) */
