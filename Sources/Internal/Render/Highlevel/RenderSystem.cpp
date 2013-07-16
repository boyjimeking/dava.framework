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
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/ShadowRect.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Render/Highlevel/RenderFastNames.h"

namespace DAVA
{

RenderSystem::RenderSystem()
{
    // Register available passes & layers
    renderPassesMap.Insert(PASS_FORWARD, new RenderPass(PASS_FORWARD));
    renderPassesMap.Insert(PASS_SHADOW_VOLUME, new ShadowVolumeRenderPass(PASS_SHADOW_VOLUME));

    renderLayersMap.Insert(LAYER_OPAQUE, new RenderLayer(LAYER_OPAQUE));
    renderLayersMap.Insert(LAYER_ALPHA_TEST_LAYER, new RenderLayer(LAYER_ALPHA_TEST_LAYER));
    
    renderLayersMap.Insert(LAYER_TRANSLUCENT, new RenderLayer(LAYER_TRANSLUCENT));
    renderLayersMap.Insert(LAYER_AFTER_TRANSLUCENT, new RenderLayer(LAYER_AFTER_TRANSLUCENT));
    
    renderLayersMap.Insert(LAYER_SHADOW_VOLUME, new RenderLayer(LAYER_SHADOW_VOLUME));
    
    
    RenderPass * forwardPass = renderPassesMap[PASS_FORWARD];
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_OPAQUE], LAST_LAYER);
    forwardPass->AddRenderLayer(renderLayersMap[LAYER_TRANSLUCENT], LAST_LAYER);

    ShadowVolumeRenderPass * shadowVolumePass = (ShadowVolumeRenderPass*)renderPassesMap[PASS_SHADOW_VOLUME];
    shadowVolumePass->AddRenderLayer(renderLayersMap[LAYER_SHADOW_VOLUME], LAST_LAYER);

    renderPassOrder.push_back(renderPassesMap[PASS_FORWARD]);
    renderPassOrder.push_back(renderPassesMap[PASS_SHADOW_VOLUME]);

	particleEmitterSystem = new ParticleEmitterSystem();
}

RenderSystem::~RenderSystem()
{
	SafeDelete(particleEmitterSystem);
    
    FastNameMap<RenderPass*>::Iterator endPasses = renderPassesMap.End();
    for(FastNameMap<RenderPass*>::Iterator it = renderPassesMap.Begin(); it != endPasses; ++it)
    {
        RenderPass *pass = it.GetValue();
        SafeDelete(pass);
    }
    renderPassesMap.Clear();
    
    FastNameMap<RenderLayer*>::Iterator endLayers = renderLayersMap.End();
    for(FastNameMap<RenderLayer*>::Iterator it = renderLayersMap.Begin(); it != endLayers; ++it)
    {
        RenderLayer *layer = it.GetValue();
        SafeDelete(layer);
    }
    renderLayersMap.Clear();
}
    

void RenderSystem::RenderPermanent(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() == -1);
    
    renderObject->Retain();
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        AddRenderBatch(batch);
    }
}

void RenderSystem::RemoveFromRender(RenderObject * renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() != -1);

	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
	for (uint32 k = 0; k < renderBatchCount; ++k)
	{
		RenderBatch * batch = renderObject->GetRenderBatch(k);
		RemoveRenderBatch(batch);
	}

    List<RenderObject*>::iterator end = markedObjects.end();
    for (List<RenderObject*>::iterator it = markedObjects.begin(); it != end; ++it)
    {
        if(*it == renderObject)
        {
            markedObjects.erase(it);
            break;
        }
    }

    
	RenderObject * lastRenderObject = renderObjectArray[renderObjectArray.size() - 1];
    renderObjectArray[renderObject->GetRemoveIndex()] = lastRenderObject;
    renderObjectArray.pop_back();
	lastRenderObject->SetRemoveIndex(renderObject->GetRemoveIndex());
    renderObject->SetRemoveIndex(-1);
    
    RemoveRenderObject(renderObject);

	renderObject->Release();
}

void RenderSystem::AddRenderObject(RenderObject * renderObject)
{
	particleEmitterSystem->AddIfEmitter(renderObject);
	renderObject->SetRenderSystem(this);
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
    particleEmitterSystem->RemoveIfEmitter(renderObject);
	renderObject->SetRenderSystem(0);
}

void RenderSystem::AddRenderBatch(RenderBatch * renderBatch)
{
    // Get Layer Name
    const FastName & name = renderBatch->GetOwnerLayerName();

    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
    RenderLayer * layer = renderLayersMap[name];
    layer->AddRenderBatch(renderBatch);
}
    
void RenderSystem::RemoveRenderBatch(RenderBatch * renderBatch)
{
    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
}

RenderPass * RenderSystem::GetRenderPass(const FastName & passName)
{
	return renderPassesMap[passName];
}

void RenderSystem::ImmediateUpdateRenderBatch(RenderBatch * renderBatch)
{
    AddRenderBatch(renderBatch);
}
    
void RenderSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

Camera * RenderSystem::GetCamera()
{
	return camera;
}
    
void RenderSystem::ProcessClipping()
{
    int32 objectBoxesUpdated = 0;
    List<RenderObject*>::iterator end = markedObjects.end();
    for (List<RenderObject*>::iterator it = markedObjects.begin(); it != end; ++it)
    {
        RenderObject * obj = *it;
        obj->GetBoundingBox().GetTransformedBox(*obj->GetWorldTransformPtr(), obj->GetWorldBoundingBox());
        FindNearestLights(obj);
        objectBoxesUpdated++;
    }
    markedObjects.clear();
    
//    List<RenderObject*>::iterator endLights = movedLights.end();
//    for (List<LightNode*>::iterator it = movedLights.begin(); it != endLights; ++it)
//    {
//        FindNearestLights(*it);
//    }
    if (movedLights.size() > 0)
    {
        FindNearestLights();
    }
    movedLights.clear();
    
    
    int32 objectsCulled = 0;
    
    Frustum * frustum = camera->GetFrustum();

    uint32 size = renderObjectArray.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject * node = renderObjectArray[pos];
		
		uint32 flags = node->GetFlags();
		flags = (flags | RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME) & RenderObject::VISIBILITY_CRITERIA;
		if (flags != RenderObject::VISIBILITY_CRITERIA)
		{
			continue;
		}

        node->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
        //Logger::Debug("Cull Node: %s rc: %d", node->GetFullName().c_str(), node->GetRetainCount());
        if (!frustum->IsInside(node->GetWorldBoundingBox()))
        {
            node->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
            objectsCulled++;
        }
    }
}

void RenderSystem::MarkForUpdate(RenderObject * renderObject)
{
    markedObjects.push_back(renderObject);
}
  
void RenderSystem::MarkForUpdate(Light * lightNode)
{
    movedLights.push_back(lightNode);
}
    
void RenderSystem::RegisterForUpdate(IRenderUpdatable * updatable)
{
    objectsForUpdate.push_back(updatable);
}
    
void RenderSystem::UnregisterFromUpdate(IRenderUpdatable * updatable)
{
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(objectsForUpdate[i] == updatable)
		{
			objectsForUpdate[i] = objectsForUpdate[size - 1];
			objectsForUpdate.pop_back();
			return;
		}
	}
}
    
//void RenderSystem::MarkForMaterialSort(Material * material)
//{
//    //for (FastNameMap<RenderLayer*>::Iterator it = renderLayersMap.Begin(); it != )
//}

    
void RenderSystem::FindNearestLights(RenderObject * renderObject)
{
    Light * nearestLight = 0;
    float32 squareMinDistance = 10000000.0f;
    Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
    
    uint32 size = lights.size();
    for (uint32 k = 0; k < size; ++k)
    {
        Light * light = lights[k];
        
        if (!light->IsDynamic())continue;
        
        const Vector3 & lightPosition = light->GetPosition();
        
        float32 squareDistanceToLight = (position - lightPosition).SquareLength();
        if (squareDistanceToLight < squareMinDistance)
        {
            squareMinDistance = squareDistanceToLight;
            nearestLight = light;
        }
    }
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        batch->GetMaterialInstance()->SetLight(0, nearestLight);
    }
}

void RenderSystem::FindNearestLights()
{
    uint32 size = (uint32)renderObjectArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        FindNearestLights(renderObjectArray[k]);
    }
}
    
void RenderSystem::AddLight(Light * light)
{
    lights.push_back(SafeRetain(light));
    FindNearestLights();
}
    
void RenderSystem::RemoveLight(Light * light)
{
    lights.erase(std::remove(lights.begin(), lights.end(), light), lights.end());
}

Vector<Light*> & RenderSystem::GetLights()
{
    return lights;
}

void RenderSystem::Update(float32 timeElapsed)
{
    ProcessClipping();
    
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
        objectsForUpdate[i]->RenderUpdate(camera, timeElapsed);
    }

	particleEmitterSystem->Update(timeElapsed);
}

void RenderSystem::Render()
{
    uint32 size = (uint32)renderPassOrder.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPassOrder[k]->Draw(camera);
    }
}

RenderLayer * RenderSystem::AddRenderLayer(const FastName & layerName, const FastName & passName, const FastName & afterLayer)
{
	DVASSERT(false == renderLayersMap.IsKey(layerName));

	RenderLayer * newLayer = new RenderLayer(layerName);
	renderLayersMap.Insert(layerName, newLayer);

	RenderPass * inPass = renderPassesMap[passName];
	inPass->AddRenderLayer(newLayer, afterLayer);

	return newLayer;
}
    
void RenderSystem::SetShadowRectColor(const Color &color)
{
    ShadowVolumeRenderPass *shadowVolume = static_cast<ShadowVolumeRenderPass *>(renderPassesMap[PASS_SHADOW_VOLUME]);
    DVASSERT(shadowVolume);

    ShadowRect *shadowRect = shadowVolume->GetShadowRect();
    DVASSERT(shadowRect);

    shadowRect->SetColor(color);
}
    
const Color & RenderSystem::GetShadowRectColor()
{
    ShadowVolumeRenderPass *shadowVolume = static_cast<ShadowVolumeRenderPass *>(renderPassesMap[PASS_SHADOW_VOLUME]);
    DVASSERT(shadowVolume);
    
    ShadowRect *shadowRect = shadowVolume->GetShadowRect();
    DVASSERT(shadowRect);
    
    return shadowRect->GetColor();
}


};