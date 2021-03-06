#include "HeightmapEditorCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../LandscapeEditor/LandscapesController.h"

#include "Utils/Random.h"

HeightmapModificationCommand::HeightmapModificationCommand(Command::eCommandType type,
														   const Rect& updatedRect,
														   CommandList::eCommandId id)
:	Command(type, id)
{
	this->updatedRect = updatedRect;
}

String HeightmapModificationCommand::TimeString()
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);
	
    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);
	
    return timeString;
}

FilePath HeightmapModificationCommand::SaveHeightmap(Heightmap* heightmap)
{
	FilePath documentsPath("~doc:/");

	FilePath folderPathname("~doc:/History/");
	FileSystem::Instance()->CreateDirectory(folderPathname);

	folderPathname = folderPathname + "Heightmap/";
	FileSystem::Instance()->CreateDirectory(folderPathname);
	
	FileList* fileList = new FileList(folderPathname);
	
	bool validFileName = false;
	String filename;
	String time = TimeString();
	uint32 num = 0;
	do
	{
		filename = time;
		if (num)
		{
			filename += Format(" (%d)", num);
		}
		filename += Heightmap::FileExtension();
		
		int32 i = 0;
		for (; i < fileList->GetCount(); ++i)
		{
			if (fileList->GetFilename(i) == filename)
			{
				++num;
				break;
			}
		}
		if (i >= fileList->GetCount())
			validFileName = true;
	} while (!validFileName);

	FilePath saveFileName = folderPathname + filename;
	heightmap->Save(saveFileName);
	
	SafeRelease(fileList);
	
	return saveFileName;
}

LandscapeEditorHeightmap* HeightmapModificationCommand::GetEditor()
{
	LandscapeEditorHeightmap* editor = NULL;
	
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorHeightmap*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_HEIGHTMAP));
	}
	
	return editor;
}

void HeightmapModificationCommand::UpdateLandscapeHeightmap(const FilePath & filename)
{
	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	LandscapesController* landscapesController = activeScene->GetLandscapesController();
	
	Landscape* landscapeNode = landscapesController->GetCurrentLandscape();
	
	Heightmap* heightmap = new Heightmap();
	heightmap->Load(filename);
	
	landscapeNode->SetHeightmap(heightmap);
	heightmap->Save(landscapeNode->GetHeightmapPathname());

	SafeRelease(heightmap);
}


CommandDrawHeightmap::CommandDrawHeightmap(Heightmap* originalHeightmap, Heightmap* newHeightmap, const Rect& updatedRect)
:	HeightmapModificationCommand(COMMAND_UNDO_REDO, updatedRect, CommandList::ID_COMMAND_DRAW_HEIGHTMAP)
{
	commandName = "Heightmap Change";

	if (originalHeightmap && newHeightmap)
	{
		undoFilename = SaveHeightmap(originalHeightmap);
		redoFilename = SaveHeightmap(newHeightmap);
	}
}

CommandDrawHeightmap::~CommandDrawHeightmap()
{
	FileSystem::Instance()->DeleteFile(undoFilename);
	FileSystem::Instance()->DeleteFile(redoFilename);
}

void CommandDrawHeightmap::Execute()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	if (editor->IsActive())
	{
		Heightmap* heightmap = editor->GetHeightmap();
		heightmap->Load(redoFilename);
		editor->UpdateHeightmap(heightmap, updatedRect);
	}
	else
	{
		UpdateLandscapeHeightmap(redoFilename);
	}
}

void CommandDrawHeightmap::Cancel()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (!editor)
		return;

	if (editor->IsActive())
	{
		Heightmap* heightmap = editor->GetHeightmap();
		heightmap->Load(undoFilename);
		editor->UpdateHeightmap(heightmap, updatedRect);
	}
	else
	{
		UpdateLandscapeHeightmap(undoFilename);
	}
}


CommandCopyPasteHeightmap::CommandCopyPasteHeightmap(bool copyHeightmap, bool copyTilemap, Heightmap* originalHeightmap, Heightmap* newHeightmap, Image* originalTilemap, Image* newTilemap, const FilePath& tilemapSavedPath, const Rect& updatedRect)
:	HeightmapModificationCommand(COMMAND_UNDO_REDO, updatedRect, CommandList::ID_COMMAND_COPY_PASTE_HEIGHTMAP)
,	heightmap(copyHeightmap)
,	tilemap(copyTilemap)
{
	commandName = "Heightmap Copy/Paste";

	tilemapUndoImage = NULL;
	tilemapRedoImage = NULL;

	if (copyHeightmap && originalHeightmap && newHeightmap)
	{
		heightmapUndoFilename = SaveHeightmap(originalHeightmap);
		heightmapRedoFilename = SaveHeightmap(newHeightmap);
	}

	if (copyTilemap && originalTilemap && newTilemap)
	{
        tilemapSavedPathname = FilePath::CreateWithNewExtension(tilemapSavedPath, ".png");
        
		tilemapUndoImage = SafeRetain(originalTilemap);
		tilemapRedoImage = SafeRetain(newTilemap);
	}
}

CommandCopyPasteHeightmap::~CommandCopyPasteHeightmap()
{
	FileSystem::Instance()->DeleteFile(heightmapUndoFilename);
	FileSystem::Instance()->DeleteFile(heightmapRedoFilename);
	SafeRelease(tilemapUndoImage);
	SafeRelease(tilemapRedoImage);
}

void CommandCopyPasteHeightmap::Execute()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (!editor)
	{
		SetState(STATE_INVALID);
		return;
	}

	// Apply new heightmap
	if (heightmap)
	{
		if (editor->IsActive())
		{
			Heightmap* heightmap = editor->GetHeightmap();
			heightmap->Load(heightmapRedoFilename);
			editor->UpdateHeightmap(heightmap, updatedRect);
		}
		else
		{
			UpdateLandscapeHeightmap(heightmapRedoFilename);
		}
	}

	// Apply new tilemap
	if (tilemap)
	{
		UpdateLandscapeTilemap(tilemapRedoImage);
	}
}

void CommandCopyPasteHeightmap::Cancel()
{
	// Restore old heightmap
	if (heightmap)
	{
		LandscapeEditorHeightmap* editor = GetEditor();
		if (!editor)
			return;
		
		if (editor->IsActive())
		{
			Heightmap* heightmap = editor->GetHeightmap();
			heightmap->Load(heightmapUndoFilename);
			editor->UpdateHeightmap(heightmap, updatedRect);
		}
		else
		{
			UpdateLandscapeHeightmap(heightmapUndoFilename);
		}
	}

	// Restore old tilemap
	if (tilemap)
	{
		UpdateLandscapeTilemap(tilemapUndoImage);
	}
}

void CommandCopyPasteHeightmap::UpdateLandscapeTilemap(DAVA::Image *image)
{
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);
	texture->relativePathname = tilemapSavedPathname;
	texture->GenerateMipmaps();
	texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);

	LandscapeEditorBase* editor = GetActiveEditor();
	if (editor)
	{
		editor->UpdateLandscapeTilemap(texture);
	}
	else
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		EditorScene* scene = screen->FindCurrentBody()->bodyControl->GetScene();
		Landscape* landscape = scene->GetLandscape(scene);

		landscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
		landscape->UpdateFullTiledTexture();
		ImageLoader::Save(image, tilemapSavedPathname);
	}

	SafeRelease(texture);
}

LandscapeEditorBase* CommandCopyPasteHeightmap::GetActiveEditor()
{
	LandscapeEditorBase* editor = NULL;
	
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = screen->FindCurrentBody()->bodyControl->GetCurrentLandscapeEditor();
	}
	
	return editor;
}
