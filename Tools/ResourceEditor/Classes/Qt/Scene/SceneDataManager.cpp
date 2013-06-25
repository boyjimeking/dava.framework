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

#include "SceneDataManager.h"

#include "DockSceneGraph/SceneGraphModel.h"

#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "TextureCompression/PVRConverter.h"

#include "./Qt/SpritesPacker/SpritePackerHelper.h"
#include "../Main/QtUtils.h"
#include "../SceneEditor/EntityOwnerPropertyHelper.h"
#include "../StringConstants.h"

using namespace DAVA;

SceneDataManager::SceneDataManager()
    :   currentScene(NULL)
{
}

SceneDataManager::~SceneDataManager()
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SafeDelete(*it);
    }
    scenes.clear();
}

SceneData* SceneDataManager::CreateNewScene()
{
	SceneData *levelScene = SceneGetLevel();
    
	levelScene->CreateScene(true);
	
	UpdateParticleSprites();
	return levelScene;	
}

Entity* SceneDataManager::AddScene(const FilePath &scenePathname)
{
    DVASSERT(scenePathname.IsEqualToExtension(".sc2"));

	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to add the scene when SceneGetActive() returns NULL!");
		return NULL;
	}
	
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return NULL;
	}

    Entity * rootNode = scene->GetRootNode(scenePathname)->Clone();

    KeyedArchive * customProperties = rootNode->GetCustomProperties();
    customProperties->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, scenePathname.GetAbsolutePathname());
    
    rootNode->SetSolid(true);
    scene->AddNode(rootNode);
    
    Camera *currCamera = scene->GetCurrentCamera();
    if(currCamera)
    {
        Vector3 pos = currCamera->GetPosition();
        Vector3 direction  = currCamera->GetDirection();
        
        Vector3 nodePos = pos + 10 * direction;
        nodePos.z = 0;
        
        Landscape * ls = scene->GetLandscape(scene);
        if(ls)
        {
            Vector3 result;
            bool res = ls->PlacePoint(nodePos, result);
            if(res)
            {
                nodePos = result;
            }
        }
        
        Matrix4 mod;
        mod.CreateTranslation(nodePos);
        rootNode->SetLocalTransform(rootNode->GetLocalTransform() * mod);
    }

	
    Landscape *landscape = scene->GetLandscape(scene);
    bool needUpdateLandscapeController = (landscape != NULL);

    //TODO: need save scene automatically?
    bool changesWereMade = SceneValidator::Instance()->ValidateSceneAndShowErrors(scene);
//    SceneValidator::Instance()->EnumerateSceneTextures();
	
	if(needUpdateLandscapeController)
	{
		sceneData->SetLandscapesControllerScene(scene);
	}

    sceneData->GetScene()->UpdateCameraLightOnScene();
	UpdateParticleSprites();
	emit SceneGraphNeedRebuild();

	return rootNode;
}

void SceneDataManager::EditLevelScene(const FilePath &scenePathname)
{
	SceneData* sceneData = SceneGetLevel();
	if (!sceneData)
	{
		DVASSERT(false && "No way to edit the scene when SceneGetLevel() returns NULL!");
		return;
	}

	EditScene(sceneData, scenePathname);
}

void SceneDataManager::EditActiveScene(const FilePath &scenePathname)
{
	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to edit the scene when SceneGetActive() returns NULL!");
		return;
	}
	
	EditScene(sceneData, scenePathname);
}

void SceneDataManager::EditScene(SceneData* sceneData, const FilePath &scenePathname)
{
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}

    DVASSERT(scenePathname.IsEqualToExtension(".sc2"));
	
    Entity * rootNode = scene->GetRootNode(scenePathname);
    if(rootNode)
    {
        sceneData->SetScenePathname(scenePathname);
		Vector<Entity*> tempV;
		tempV.reserve(rootNode->GetChildrenCount());
		
		for (int32 ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
		{
			tempV.push_back(rootNode->GetChild(ci));
		}
        for (int32 ci = 0; ci < (int32)tempV.size(); ++ci)
        {
            //рут нода это сама сцена в данном случае
            scene->AddNode(tempV[ci]);
        }
    }

    //TODO: need save scene automatically?
    bool changesWereMade = SceneValidator::Instance()->ValidateSceneAndShowErrors(scene);
//    SceneValidator::Instance()->EnumerateSceneTextures();

    sceneData->SetLandscapesControllerScene(scene);
	
	scene->Update(0);
	sceneData->EmitSceneChanged();


    scene->UpdateCameraLightOnScene();
	UpdateParticleSprites();
    emit SceneGraphNeedRebuild();

	emit SceneCreated(sceneData);
}

void SceneDataManager::ReloadScene(const FilePath &scenePathname, const FilePath &fromScenePathname)
{
	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to add reference scene when SceneGetActive() returns NULL!");
		return;
	}
	
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}
    
	sceneData->SelectNode(NULL);
    scene->ReleaseRootNode(scenePathname);
    
	nodesToAdd.clear();
    
    Set<String> errors;
    ReloadNode(scene, scene, scenePathname, fromScenePathname, errors);
    if(!errors.empty())
    {
        ShowErrorDialog(errors);
        
        nodesToAdd.clear();
        return;
    }

    Landscape* landscape = EditorScene::GetLandscape(currentScene->GetScene());
    for (int32 i = 0; i < (int32)nodesToAdd.size(); i++)
    {
        scene->ReleaseUserData(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->RemoveNode(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->AddNode(nodesToAdd[i].nodeToAdd);
		ApplyDefaultFogSettings(landscape, nodesToAdd[i].nodeToAdd);

        SafeRelease(nodesToAdd[i].nodeToAdd);
    }
    nodesToAdd.clear();
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->OnReloadRootNodesQt();
    }
    
    
    scene->UpdateCameraLightOnScene();
	UpdateParticleSprites();
    emit SceneGraphNeedRebuild();
	sceneData->SetLandscapesControllerScene(scene);
}

void SceneDataManager::ReloadNode(EditorScene* scene, Entity *node, const FilePath &nodePathname, const FilePath &fromPathname, Set<String> &errors)
{
	//если в рут ноды сложить такие же рут ноды то на релоаде все накроет пиздой
    KeyedArchive *customProperties = node->GetCustomProperties();
	EntityOwnerPropertyHelper::Instance()->UpdateEntityOwner(customProperties);
    if (customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, "") == nodePathname.GetAbsolutePathname())
    {
        Entity *loadedNode = scene->GetRootNode(fromPathname);
        if(loadedNode)
        {
            Entity *newNode = loadedNode->Clone();
            newNode->SetLocalTransform(node->GetLocalTransform());
            newNode->GetCustomProperties()->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER,
															fromPathname.GetAbsolutePathname());
            newNode->SetSolid(true);
            
            Entity *parent = node->GetParent();
            AddedNode addN;
            addN.nodeToAdd = newNode;
            addN.nodeToRemove = node;
            addN.parent = parent;
            
            nodesToAdd.push_back(addN);
        }
        else
        {
            errors.insert(Format("Cannot load object: %s", fromPathname.GetAbsolutePathname().c_str()));
        }
        
        scene->UpdateCameraLightOnScene();
        return;
    }
    
    int32 csz = node->GetChildrenCount();
    for (int ci = 0; ci < csz; ++ci)
    {
        Entity * child = node->GetChild(ci);
        ReloadNode(scene, child, nodePathname, fromPathname, errors);
    }
}

void SceneDataManager::SetActiveScene(EditorScene *scene)
{
    if(currentScene)
    {
		emit SceneDeactivated(currentScene);
    }
    
    currentScene = FindDataForScene(scene);
    DVASSERT(currentScene && "There is no current scene. Something wrong.");
    currentScene->RebuildSceneGraph();

	emit SceneActivated(currentScene);
}

SceneData * SceneDataManager::FindDataForScene(EditorScene *scene)
{
    SceneData *foundData = NULL;

    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        if((*it)->GetScene() == scene)
        {
            foundData = *it;
            break;
        }
    }
    
    return foundData;
}


SceneData * SceneDataManager::SceneGetActive()
{
	return currentScene;
}

SceneData *SceneDataManager::SceneGetLevel()
{
    if(0 < scenes.size())
    {
        return scenes.front();
    }
    
    return NULL;
}

DAVA::Entity* SceneDataManager::SceneGetSelectedNode(SceneData *scene)
{
	DAVA::Entity *node = NULL;

	if(NULL != scene)
	{
		node = scene->GetSelectedNode();
	}

	return node;
}

DAVA::Entity* SceneDataManager::SceneGetRootNode(SceneData *scene)
{
	DAVA::Entity *node = NULL;

	if(NULL != scene)
	{
		node = scene->GetScene();
	}

	return node;
}


EditorScene * SceneDataManager::RegisterNewScene()
{
    SceneData *data = new SceneData();
    data->CreateScene(true);

    scenes.push_back(data);

	emit SceneCreated(data);

	connect(data, SIGNAL(SceneChanged(EditorScene *)), this, SLOT(InSceneData_SceneChanged(EditorScene *)));
	connect(data, SIGNAL(SceneNodeSelected(DAVA::Entity *)), this, SLOT(InSceneData_SceneNodeSelected(DAVA::Entity *)));

	connect(data, SIGNAL(SceneGraphModelNeedsRebuildNode(DAVA::Entity *)), this, SLOT(InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::Entity *)));
	connect(data, SIGNAL(SceneGraphModelNeedsRebuild()), this, SLOT(InSceneData_SceneGraphModelNeedsRebuild()));
	
	connect(data, SIGNAL(SceneGraphModelNeedSetScene(EditorScene *)), this, SLOT(InSceneData_SceneGraphModelNeedSetScene(EditorScene *)));
	connect(data, SIGNAL(SceneGraphModelNeedsSelectNode(DAVA::Entity*)), this, SLOT(InSceneData_SceneGraphModelNeedsSelectNode(DAVA::Entity*)));

    return data->GetScene();
}

void SceneDataManager::ReleaseScene(EditorScene *scene)
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SceneData *sceneData = *it;
        if(sceneData->GetScene() == scene)
        {
			emit SceneReleased(sceneData);

			if(currentScene == sceneData)
            {
                DVASSERT((0 < scenes.size()) && "There is no main level scene.")
                currentScene = *scenes.begin(); // maybe we need to activate next or prev tab?
            }
                
            SafeDelete(sceneData);

            scenes.erase(it);
            break;
        }
    }
}

DAVA::int32 SceneDataManager::SceneCount()
{
    return (int32)scenes.size();
}

SceneData *SceneDataManager::SceneGet(DAVA::int32 index)
{
    DVASSERT((0 <= index) && (index < (int32)scenes.size()));
    
    DAVA::List<SceneData *>::iterator it = scenes.begin();
    std::advance(it, index);
    
    return *it;
}

SceneData *SceneDataManager::SceneGet(DAVA::Scene *scene)
{
    DVASSERT(scene);
    
    auto endIt = scenes.end();
    for(auto it = scenes.begin(); it != endIt; ++it)
    {
        if((*it)->GetScene() == scene)
            return *it;
    }
    
    DVASSERT(false);
    
    return NULL;
}


void SceneDataManager::InSceneData_SceneChanged(EditorScene *scene)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneChanged(sceneData);
}

void SceneDataManager::InSceneData_SceneNodeSelected(Entity *node)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneNodeSelected(sceneData, node);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::Entity *node)
{
	// Re-emit the signal from the "inner" Scene Data to all SceneDataManager subscribers.
	emit SceneGraphNeedRebuildNode(node);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsRebuild()
{
	// Re-emit the signal from the "inner" Scene Data to all SceneDataManager subscribers.
	emit SceneGraphNeedRebuild();
}

void SceneDataManager::InSceneData_SceneGraphModelNeedSetScene(EditorScene *scene)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneGraphNeedSetScene(sceneData, scene);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsSelectNode(DAVA::Entity* node)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneGraphNeedSelectNode(sceneData, node);
}

void SceneDataManager::EnumerateTextures(DAVA::Entity *forNode, Map<String, Texture *> &textures)
{
	if(!forNode)  return;

    Vector<Entity *> nodes;
    forNode->GetChildNodes(nodes);
    
    nodes.push_back(forNode);
    
    for(int32 n = 0; n < (int32)nodes.size(); ++n)
    {
        RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
        if(!rc) continue;
            
        RenderObject *ro = rc->GetRenderObject();
        if(!ro) continue;
        
        uint32 count = ro->GetRenderBatchCount();
        for(uint32 b = 0; b < count; ++b)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(b);
            
            Material *material = renderBatch->GetMaterial();
            if(material)
            {
                for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
                {
                    CollectTexture(textures, material->GetTextureName((DAVA::Material::eTextureLevel)t).GetAbsolutePathname(), material->GetTexture((DAVA::Material::eTextureLevel)t));
                }
            }

            InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
            if(instanceMaterial)
            {
                CollectTexture(textures, instanceMaterial->GetLightmapName().GetAbsolutePathname(), instanceMaterial->GetLightmap());
            }
        }

        Landscape *land = dynamic_cast<Landscape *>(ro);
        if(land)
        {
            CollectLandscapeTextures(textures, land);
        }
    }
}

void SceneDataManager::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, forNode->GetTextureName((Landscape::eTextureLevel)t).GetAbsolutePathname(), forNode->GetTexture((Landscape::eTextureLevel)t));
	}
}



void SceneDataManager::CollectTexture(Map<String, Texture *> &textures, const String &name, Texture *tex)
{
	if(!name.empty() && SceneValidator::Instance()->IsPathCorrectForProject(name))
	{
		textures[name] = tex;
	}
}


void SceneDataManager::EnumerateDescriptors(DAVA::Entity *forNode, DAVA::Set<DAVA::FilePath> &descriptors)
{
    if(!forNode)  return;
    
    Vector<Entity *> nodes;
    forNode->GetChildNodes(nodes);
    
    nodes.push_back(forNode);
    
    for(int32 n = 0; n < (int32)nodes.size(); ++n)
    {
        RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
        if(!rc) continue;
        
        RenderObject *ro = rc->GetRenderObject();
        if(!ro) continue;
        
        uint32 count = ro->GetRenderBatchCount();
        for(uint32 b = 0; b < count; ++b)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(b);
            
            Material *material = renderBatch->GetMaterial();
            if(material)
            {
                for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
                {
                    CollectDescriptors(descriptors, material->GetTextureName((DAVA::Material::eTextureLevel)t));
                }
            }
            
            InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
            if(instanceMaterial)
            {
                CollectDescriptors(descriptors, instanceMaterial->GetLightmapName());
            }
        }
        
        Landscape *land = dynamic_cast<Landscape *>(ro);
        if(land)
        {
            CollectLandscapeDescriptors(descriptors, land);
        }
    }
}


void SceneDataManager::CollectLandscapeDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, DAVA::Landscape *forNode)
{
    for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectDescriptors(descriptors, forNode->GetTextureName((Landscape::eTextureLevel)t));
	}
}

void SceneDataManager::CollectDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, const DAVA::FilePath &pathname)
{
    if(pathname.GetType() == FilePath::PATH_EMPTY)
        return;

    DVASSERT(pathname.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()));
    
    if(!pathname.IsEmpty() && SceneValidator::Instance()->IsPathCorrectForProject(pathname))
	{
        descriptors.insert(pathname);
	}
}


void SceneDataManager::TextureCompressAllNotCompressed()
{
	Map<String, Texture *> textures;
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		EnumerateTextures((*it)->GetScene(), textures);
	}

	List<Texture *>texturesForPVRCompression;
	List<Texture *>texturesForDXTCompression;

	Map<String, Texture *>::const_iterator endItTextures = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endItTextures; ++it)
	{
        for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
        {
            eGPUFamily gpu = (eGPUFamily)i;
            if(SceneValidator::Instance()->IsTextureChanged(it->first, gpu))
            {
                //TODO: need correct code to create compression threads
                DVASSERT(false);
            }
        }
        
//		if(SceneValidator::Instance()->IsTextureChanged(it->first, PVR_FILE))
//		{
//			texturesForPVRCompression.push_back(SafeRetain(it->second));
//		}
//		else if(SceneValidator::Instance()->IsTextureChanged(it->first, DXT_FILE))
//		{
//			texturesForDXTCompression.push_back(SafeRetain(it->second));
//		}
	}

//	CompressTextures(texturesForPVRCompression, PVR_FILE);
//	CompressTextures(texturesForDXTCompression, DXT_FILE);

	for_each(texturesForPVRCompression.begin(), texturesForPVRCompression.end(),  SafeRelease<Texture>);
	for_each(texturesForDXTCompression.begin(), texturesForDXTCompression.end(),  SafeRelease<Texture>);
}

void SceneDataManager::CompressTextures(const List<DAVA::Texture *> texturesForCompression, DAVA::eGPUFamily forGPU)
{
	//TODO: need to run compression at thread

// 	List<Texture *>::const_iterator endIt = texturesForCompression.end();
// 	for(List<Texture *>::const_iterator it = texturesForCompression.begin(); it != endIt; ++it)
// 	{
// 		Texture *texture = *it;
// 		//TODO: compress texture
// 		TextureDescriptor *descriptor = texture->CreateDescriptor();
// 		if(descriptor)
// 		{
// 			if(fileFormat == PVR_FILE)
// 			{
// 				PVRConverter::Instance()->ConvertPngToPvr(descriptor->GetSourceTexturePathname(), *descriptor);
// 			}
// 			else if(fileFormat == DXT_FILE)
// 			{
// 
// 			}
// 
// 			bool wasUpdated = descriptor->UpdateDateAndCrcForFormat(fileFormat);
// 			if(wasUpdated) descriptor->Save();
// 			SafeRelease(descriptor);
// 		}
// 	}
}

void SceneDataManager::TextureReloadAll(DAVA::eGPUFamily forGPU)
{
	Map<String, Texture *> textures;
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		EnumerateTextures((*it)->GetScene(), textures);
	}

	Map<String, Texture *>::const_iterator endItTextures = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endItTextures; ++it)
	{
		TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(it->first);
		if(descriptor)
		{
			Texture *newTexture = TextureReload(descriptor, it->second, forGPU);
			SafeRelease(descriptor);
		}
	}
}

DAVA::Texture * SceneDataManager::TextureReload(const TextureDescriptor *descriptor, DAVA::Texture *prevTexture, DAVA::eGPUFamily forGPU)
{
	if(!descriptor)
		return NULL;
	
	Texture *workingTexture = prevTexture;
	if(workingTexture == Texture::GetPinkPlaceholder())
	{
		//Create texture from descriptor pathname and real image file
        FilePath pathname = descriptor->pathname;
		workingTexture = Texture::CreateFromFile(pathname);
		RestoreTexture(pathname, workingTexture);

		DVASSERT_MSG(1 < workingTexture->GetRetainCount(), "Can be more than 1");
		workingTexture->Release();

		if(workingTexture == Texture::GetPinkPlaceholder())
		{
			return workingTexture;
		}
	}

	//apply descriptor parameters
	workingTexture->ReloadAs(forGPU, descriptor);
	return workingTexture;
}

void SceneDataManager::RestoreTexture( const DAVA::FilePath &descriptorPathname, DAVA::Texture *texture )
{
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		(*it)->RestoreTexture(descriptorPathname, texture);
	}
}

void SceneDataManager::EnumerateMaterials(DAVA::Entity *forNode, Vector<Material *> &materials)
{
	if(forNode)
	{
		forNode->GetDataNodes(materials);
	}
}

void SceneDataManager::SceneNodeSelectedInSceneGraph(Entity* node)
{
	SceneData *activeScene = SceneGetActive();
	
	
	activeScene->SceneNodeSelectedInGraph(node);
}

void SceneDataManager::RefreshParticlesLayer(DAVA::ParticleLayer* layer)
{
	emit SceneGraphNeedRefreshLayer(layer);
}

void SceneDataManager::UpdateParticleSprites()
{
	SpritePackerHelper::Instance()->UpdateParticleSprites();
}

void SceneDataManager::ApplyDefaultFogSettings(Landscape* landscape, DAVA::Entity *entity)
{
	if (!entity || !landscape)
	{
		return;
	}
	
	// Yuri Coder, 2013/05/13. The default fog settings are taken from Landscape.
	Vector<Material *> materials;
	entity->GetDataNodes(materials);
	for (Vector<Material*>::iterator iter = materials.begin(); iter != materials.end();
		 iter ++)
	{
		Material* material = (*iter);

		material->SetFog(landscape->IsFogEnabled());
		material->SetFogColor(landscape->GetFogColor());
		material->SetFogDensity(landscape->GetFogDensity());
	}
}

void SceneDataManager::UpdateCameraLightOnScene(bool show)
{
 	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
        (*it)->GetScene()->UpdateCameraLightOnScene(show);
	}
    
    emit SceneGraphNeedRebuild();
}

