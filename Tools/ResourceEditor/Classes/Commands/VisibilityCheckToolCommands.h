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