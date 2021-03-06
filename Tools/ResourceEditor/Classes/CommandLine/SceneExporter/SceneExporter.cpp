#include "SceneExporter.h"
#include "SceneEditor/SceneValidator.h"

#include "TextureCompression/PVRConverter.h"
#include "TextureCompression/DXTConverter.h"

#include "Render/TextureDescriptor.h"
#include "Qt/Scene/SceneDataManager.h"


using namespace DAVA;

SceneExporter::SceneExporter()
{
    SetExportingFormat(String("png"));
}

SceneExporter::~SceneExporter()
{
    ReleaseTextures();
}


void SceneExporter::CleanFolder(const FilePath &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str())));
        }
    }
}

void SceneExporter::SetInFolder(const FilePath &folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneExporter::SetOutFolder(const FilePath &folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}

void SceneExporter::SetExportingFormat(const String &newFormat)
{
    String format = newFormat;
    if(!format.empty() && format.at(0) != '.')
    {
        format = "." + format;
    }
    
    exportFormat = NOT_FILE;
    if(0 == CompareCaseInsensitive(format, ".png"))
    {
        exportFormat = PNG_FILE;
    }
    else if(0 == CompareCaseInsensitive(format, ".pvr"))
    {
        exportFormat = PVR_FILE;
    }
    else if(0 == CompareCaseInsensitive(format, ".dds"))
    {
        exportFormat = DXT_FILE;
    }
}

void SceneExporter::ExportFile(const String &fileName, Set<String> &errorLog)
{
    Logger::Info("[SceneExporter::ExportFile] %s", fileName.c_str());
    
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;
    
    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(filePath);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<Entity*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
		
		ExportScene(scene, filePath, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneExporter::ExportFile] Can't open file %s", filePath.GetAbsolutePathname().c_str()));
	}

    SafeRelease(scene);
}

void SceneExporter::ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog)
{
    DVASSERT(0 == texturesForExport.size())
    
    //Create destination folder
    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);
    
    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true); 
    
    scene->Update(0.1f);
    //Export scene data
    RemoveEditorNodes(scene);

    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);
	//SceneValidator::Instance()->ValidateScales(scene, errorLog);

    texturesForExport.clear();
    SceneDataManager::EnumerateTextures(scene, texturesForExport);

    ExportTextures(scene, errorLog);
    ExportLandscape(scene, errorLog);
    
    ReleaseTextures();
    
    //save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(sceneUtils.dataSourceFolder + relativeFilename, ".exported.sc2");
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    
    outFile->SaveScene(tempSceneName, scene);
    SafeRelease(outFile);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
	if(!moved)
	{
		errorLog.insert(Format("Can't move file %s", fileName.GetAbsolutePathname().c_str()));
	}
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}

void SceneExporter::RemoveEditorNodes(DAVA::Entity *rootNode)
{
    //Remove scene nodes
    Vector<Entity *> scenenodes;
    rootNode->GetChildNodes(scenenodes);
        
    //remove nodes from hierarhy
    Vector<Entity *>::reverse_iterator endItDeletion = scenenodes.rend();
    for (Vector<Entity *>::reverse_iterator it = scenenodes.rbegin(); it != endItDeletion; ++it)
    {
        Entity * node = *it;
		String::size_type pos = node->GetName().find(String("editor."));
        if(String::npos != pos)
        {
            node->GetParent()->RemoveNode(node);
        }
		else
		{
			DAVA::RenderComponent *renderComponent = static_cast<DAVA::RenderComponent *>(node->GetComponent(DAVA::Component::RENDER_COMPONENT));
			if(renderComponent)
			{
				DAVA::RenderObject *ro = renderComponent->GetRenderObject();
				if(ro && ro->GetType() != RenderObject::TYPE_LANDSCAPE)
				{
					DAVA::uint32 count = ro->GetRenderBatchCount();
					for(DAVA::uint32 ri = 0; ri < count; ++ri)
					{
						DAVA::Material *material = ro->GetRenderBatch(ri)->GetMaterial();
						if(material)
							material->SetStaticLightingParams(0);
					}
				}

			}
		}
    }
}



void SceneExporter::ExportTextures(DAVA::Scene *scene, Set<String> &errorLog)
{
    Map<String, Texture *>::const_iterator endIt = texturesForExport.end();
    Map<String, Texture *>::iterator it = texturesForExport.begin();
    for(; it != endIt; ++it)
    {
        ExportTexture( it->first, errorLog);
    }
}

void SceneExporter::ReleaseTextures()
{
    texturesForExport.clear();
}

void SceneExporter::ExportFolder(const String &folderName, Set<String> &errorLog)
{
    FilePath folderPathname = sceneUtils.dataSourceFolder + folderName;
    folderPathname.MakeDirectoryPathname();
    
	FileList * fileList = new FileList(folderPathname);
    for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
        FilePath pathname = fileList->GetPathname(i);
		if(fileList->IsDirectory(i))
		{
            if(!fileList->IsNavigationDirectory(i))
            {
                String workingPathname = sceneUtils.RemoveFolderFromPath(pathname, sceneUtils.dataSourceFolder);
                ExportFolder(workingPathname, errorLog);
            }
        }
        else 
        {
            if(pathname.IsEqualToExtension(".sc2"))
            {
                String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
                if(exportedPos != String::npos)
                {
                    //Skip temporary files, created during export
                    continue;
                }
                
                String workingPathname = sceneUtils.RemoveFolderFromPath(pathname, sceneUtils.dataSourceFolder);
                ExportFile(workingPathname, errorLog);
            }
        }
    }
    
    SafeRelease(fileList);
}



void SceneExporter::ExportLandscape(Scene *scene, Set<String> &errorLog)
{
    DVASSERT(scene);

    Landscape *landscape = EditorScene::GetLandscape(scene);
    if (landscape)
    {
        sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
        
        Landscape::eTiledShaderMode mode = landscape->GetTiledShaderMode();
        if(mode == Landscape::TILED_MODE_MIXED || mode == Landscape::TILED_MODE_TEXTURE)
        {
            ExportLandscapeFullTiledTexture(landscape, errorLog);
        }
    }
}

void SceneExporter::ExportLandscapeFullTiledTexture(Landscape *landscape, Set<String> &errorLog)
{
    if(landscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILEMASK)
    {
        return;
    }
    
    FilePath textureName = landscape->GetTextureName(Landscape::TEXTURE_TILE_FULL);
    if(textureName.IsEmpty())
    {
        FilePath fullTiledPathname = landscape->GetTextureName(Landscape::TEXTURE_COLOR);
        fullTiledPathname.ReplaceExtension(".thumbnail_exported.png");
        
        String workingPathname = sceneUtils.RemoveFolderFromPath(fullTiledPathname, sceneUtils.dataSourceFolder);
        sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

        Texture *fullTiledTexture = Texture::GetPinkPlaceholder();
        Image *image = fullTiledTexture->CreateImageFromMemory();
        if(image)
        {
            ImageLoader::Save(image, sceneUtils.dataFolder + workingPathname);
            ImageLoader::Save(image, sceneUtils.dataSourceFolder + workingPathname);
            SafeRelease(image);
            
            TextureDescriptor *descriptor = new TextureDescriptor();
            if(exportFormat == PVR_FILE)
            {
                descriptor->pvrCompression.format = FORMAT_PVR4;
            }
            
            FilePath descriptorPathnameInData = TextureDescriptor::GetDescriptorPathname(sceneUtils.dataFolder + workingPathname);
            descriptor->Save(descriptorPathnameInData);
            
            FilePath descriptorPathnameInDataSource = TextureDescriptor::GetDescriptorPathname(sceneUtils.dataSourceFolder + workingPathname);
            bool needToDeleteDescriptorFile = !FileSystem::Instance()->IsFile(descriptorPathnameInDataSource);

            descriptor->Save(descriptorPathnameInDataSource);
            SafeRelease(descriptor);
            
            landscape->SetTexture(Landscape::TEXTURE_TILE_FULL, sceneUtils.dataSourceFolder + workingPathname);
            
            if(needToDeleteDescriptorFile)
            {
                FileSystem::Instance()->DeleteFile(descriptorPathnameInDataSource);
            }
            FileSystem::Instance()->DeleteFile(sceneUtils.dataSourceFolder + workingPathname);

            errorLog.insert(String(Format("Full tiled texture is autogenerated png-image.", workingPathname.c_str())));
        }
        else
        {
            errorLog.insert(String(Format("Can't create image for fullTiled Texture for file %s", workingPathname.c_str())));
            landscape->SetTextureName(Landscape::TEXTURE_TILE_FULL, String(""));
        }
        
        landscape->SetTextureName(Landscape::TEXTURE_TILE_FULL, sceneUtils.dataSourceFolder + workingPathname);
    }
}


bool SceneExporter::ExportTexture(const FilePath &texturePathname, Set<String> &errorLog)
{
    bool wasDescriptorExported = ExportTextureDescriptor(texturePathname, errorLog);
    if(!wasDescriptorExported)
        return false;
    
    CompressTextureIfNeed(texturePathname, errorLog);
    
    return sceneUtils.CopyFile(TextureDescriptor::GetPathnameForFormat(texturePathname, exportFormat), errorLog);;
}

bool SceneExporter::ExportTextureDescriptor(const FilePath &texturePathname, Set<String> &errorLog)
{
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(texturePathname);
    if(!descriptor)
    {
        errorLog.insert(Format("Can't create descriptor for pathname %s", texturePathname.GetAbsolutePathname().c_str()));
        return false;
    }

    if(     (exportFormat == PVR_FILE && descriptor->pvrCompression.format == FORMAT_INVALID)
       ||   (exportFormat == DXT_FILE && descriptor->dxtCompression.format == FORMAT_INVALID))
    {
        errorLog.insert(Format("Not selected export format for pathname %s", texturePathname.GetAbsolutePathname().c_str()));
        
        SafeRelease(descriptor);
        return false;
    }
    
    descriptor->textureFileFormat = exportFormat;
    
    String workingPathname = sceneUtils.RemoveFolderFromPath(descriptor->pathname, sceneUtils.dataSourceFolder);
    sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

#if defined TEXTURE_SPLICING_ENABLED
    descriptor->ExportAndSlice(sceneUtils.dataFolder + workingPathname, GetExportedTextureName(texturePathname));
#else //#if defined TEXTURE_SPLICING_ENABLED
    descriptor->Export(sceneUtils.dataFolder + workingPathname);
#endif //#if defined TEXTURE_SPLICING_ENABLED

    SafeRelease(descriptor);
    
    return true;
}


void SceneExporter::CompressTextureIfNeed(const FilePath &texturePathname, Set<String> &errorLog)
{
    String modificationDate = File::GetModificationDate(TextureDescriptor::GetPathnameForFormat(texturePathname, exportFormat));
    
    FilePath sourceTexturePathname(texturePathname);
    sourceTexturePathname.ReplaceExtension(".png");
    
    if(PNG_FILE != exportFormat)
    {
        bool needToConvert = SceneValidator::IsTextureChanged(sourceTexturePathname, exportFormat);
        if(needToConvert || modificationDate.empty())
        {
            //TODO: convert to pvr/dxt
            //TODO: do we need to convert to pvr if needToConvert is false, but *.pvr file isn't at filesystem
            
            TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(texturePathname);
			if(descriptor)
			{
				if(exportFormat == PVR_FILE)
				{
					PVRConverter::Instance()->ConvertPngToPvr(sourceTexturePathname, *descriptor);
					bool wasUpdated = descriptor->UpdateCrcForFormat(PVR_FILE);
                    if(wasUpdated)
                    {
                        descriptor->Save();
                    }
				}
				else if(exportFormat == DXT_FILE)
				{
					DXTConverter::ConvertPngToDxt(sourceTexturePathname, *descriptor);
					bool wasUpdated = descriptor->UpdateCrcForFormat(DXT_FILE);
                    if(wasUpdated)
                    {
                        descriptor->Save();
                    }
				}

				SafeRelease(descriptor);
			}
			else
			{
				errorLog.insert(String(Format("Can't load descriptor from path: %s", texturePathname.GetAbsolutePathname().c_str())));
			}
        }
    }
}

