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

#ifndef __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__
#define __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../SceneEditor/LandscapeEditorVisibilityCheckTool.h"

using namespace DAVA;

class CommandSaveTextureVisibilityTool: public Command
{
public:
	DAVA_DEPRECATED(CommandSaveTextureVisibilityTool());// DEPRECATED : use QString
	
protected:
	virtual void Execute();
};

class CommandPlacePointVisibilityTool: public Command
{
public:
	DAVA_DEPRECATED(CommandPlacePointVisibilityTool(const Vector2& newVisibilityPoint, const Vector2& oldVisibilityPoint, bool oldPointIsSet, Image* oldImage));// DEPRECATED: usage of SceneEditorScreenMain
	virtual ~CommandPlacePointVisibilityTool();

protected:
	Vector2 point;
	LandscapeEditorVisibilityCheckTool* editor;

	Vector2 oldPoint;
	bool oldPointIsSet;
	Image* oldImage;

	LandscapeEditorVisibilityCheckTool* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};

class CommandPlaceAreaVisibilityTool: public Command
{
public:
	DAVA_DEPRECATED(CommandPlaceAreaVisibilityTool(const Vector2& areaPoint, uint32 areaSize, Image* oldImage));// DEPRECATED: usage of SceneEditorScreenMain
	virtual ~CommandPlaceAreaVisibilityTool();

protected:
	Vector2 point;
	uint32 size;

	Image* oldImage;
	Image* redoImage;

	LandscapeEditorVisibilityCheckTool* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};

#endif