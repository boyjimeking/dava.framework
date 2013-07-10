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

#include "HeightmapEditorCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"

ModifyHeightmapCommand::ModifyHeightmapCommand(HeightmapProxy* heightmapProxy,
											   Heightmap* originalHeightmap,
											   const Rect& updatedRect)
:	Command2(CMDID_DRAW_HEIGHTMAP, "Heightmap Change")
,	heightmapProxy(heightmapProxy)
{
	if (originalHeightmap && heightmapProxy)
	{
		this->updatedRect = updatedRect;
		undoRegion = GetHeightmapRegion(originalHeightmap);
		redoRegion = GetHeightmapRegion(heightmapProxy);
	}
}

ModifyHeightmapCommand::~ModifyHeightmapCommand()
{
	SafeDeleteArray(undoRegion);
	SafeDeleteArray(redoRegion);
}

Entity* ModifyHeightmapCommand::GetEntity() const
{
	return NULL;
}

void ModifyHeightmapCommand::Redo()
{
	ApplyHeightmapRegion(redoRegion);
}

void ModifyHeightmapCommand::Undo()
{
	ApplyHeightmapRegion(undoRegion);
}

uint16* ModifyHeightmapCommand::GetHeightmapRegion(Heightmap* heightmap)
{
	int32 size = heightmap->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;
	
	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);
	
	uint16* newData = new uint16[width * height];
	uint16* oldData = heightmap->Data();
	
	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = oldData + (yOffset + i) * size + xOffset;
		uint16* dst = newData + i * width;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	return newData;
}

void ModifyHeightmapCommand::ApplyHeightmapRegion(uint16* region)
{
	int32 size = heightmapProxy->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;
	
	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);
	
	uint16* data = heightmapProxy->Data();
	
	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = region + i * width;
		uint16* dst = data + (yOffset + i) * size + xOffset;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	heightmapProxy->UpdateRect(updatedRect);
}
