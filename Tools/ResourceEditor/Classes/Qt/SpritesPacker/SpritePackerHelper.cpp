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

#include "SpritePackerHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"

#include "../SpritesPacker.h"
#include "../SceneEditor/EditorSettings.h"
#include "./Scene/SceneDataManager.h"

#include <QtConcurrentRun>

#include "TexturePacker/ResourcePacker2D.h"
#include "Platform/Qt/QtLayer.h"

using namespace DAVA;

SpritePackerHelper::SpritePackerHelper()
{
	QObject::connect(&watcher, SIGNAL(finished()), this, SLOT(threadRepackAllFinished()), Qt::QueuedConnection);
}

void SpritePackerHelper::UpdateParticleSprites()
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.IsEmpty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	Pack();
	
	Reload();
}

void SpritePackerHelper::Pack()
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	FilePath inputDir = projectPath + "DataSource/Gfx/Particles/";
	FilePath outputDir = projectPath + "Data/Gfx/Particles/";

	if(!FileSystem::Instance()->IsDirectory(inputDir))
	{
		Logger::Error("[SpritePackerHelper::Pack] inputDir is not directory (%s)", inputDir.GetAbsolutePathname().c_str());
		return;
	}

	ResourcePacker2D * resourcePacker = new ResourcePacker2D();
	
	bool isChanged = resourcePacker->IsMD5ChangedDir(projectPath+"DataSource/Gfx/",inputDir,"particles.md5",true);
	
	SafeDelete(resourcePacker);
	if(!isChanged)
	{
		return;
	}
	
	SpritesPacker packer;
	packer.SetInputDir(inputDir);
	packer.SetOutputDir(outputDir);
	packer.Pack(false);
	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
}

void SpritePackerHelper::Reload()
{
    Map<String, Sprite *>spritesForReloading;
    
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		if(NULL != sceneData)
		{
			// All the Particle Effects must be re-started after sprites are reloaded to avoid
			// issue like DF-545.
			EnumerateSpritesForReloading(sceneData, spritesForReloading);
		}
    }

    Map<String, Sprite *>::const_iterator endIt = spritesForReloading.end();
    for(Map<String, Sprite *>::const_iterator it = spritesForReloading.begin(); it != endIt; ++it)
    {
        it->second->Reload();
    }
    
    if(ParticlesEditorController::Instance())
        ParticlesEditorController::Instance()->RefreshSelectedNode();
}

void SpritePackerHelper::EnumerateSpritesForReloading(SceneData* sceneData, Map<String, Sprite *> &sprites)
{
    List<Entity*> particleEffects;
	sceneData->GetAllParticleEffects(particleEffects);
    
	for (auto it = particleEffects.begin(); it != particleEffects.end(); ++it)
	{
		Entity* curNode = (*it);
	    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(curNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		
		if (!effectComponent)
		{
			continue;
		}
        
		bool isStopped = effectComponent->IsStopped();
		if (!isStopped)
		{
			effectComponent->Stop();
		}
        
		// All the children of this Scene Node must have Emitter components.
		int32 emittersCount = curNode->GetChildrenCount();
		for (int32 i = 0; i < emittersCount; i ++)
		{
			Entity* childNode = curNode->GetChild(i);
			ParticleEmitter * emitter = GetEmitter(childNode);
			
			EnumerateSpritesForParticleEmitter(emitter, sprites);
		}
        
		if (!isStopped)
		{
			effectComponent->Start();
		}
	}
}

void SpritePackerHelper::EnumerateSpritesForParticleEmitter(ParticleEmitter* emitter, Map<String, Sprite *> &sprites)
{
	if (!emitter)
	{
		return;
	}
	
	Vector<ParticleLayer*> & layers = emitter->GetLayers();
	int32 layersCount = layers.size();
	for (int il = 0; il < layersCount; ++il)
	{
		ParticleLayer* curLayer = layers[il];
		Sprite *sprite = curLayer->GetSprite();
		if (sprite)
		{
			sprites[sprite->GetRelativePathname().GetAbsolutePathname()] = sprite;
		}
		
		// Superemitter layers might have inner emitter with its own sprites.
		if (curLayer->GetInnerEmitter())
		{
			EnumerateSpritesForParticleEmitter(curLayer->GetInnerEmitter(), sprites);
		}
	}
}

void SpritePackerHelper::UpdateParticleSpritesAsync()
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.IsEmpty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	if(NULL == future)
	{
		future = new QFuture<void>;
		*future = QtConcurrent::run(this, &SpritePackerHelper::Pack);
		watcher.setFuture(*future);
	}
}

void SpritePackerHelper::threadRepackAllFinished()
{
	future = NULL;
	
	Reload();

	emit readyAll();
}
