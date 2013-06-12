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

#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA
{

ParticleEffectComponent::ParticleEffectComponent()
{
	stopAfterNRepeats = -1;
	stopWhenEmpty = false;
	effectDuration = 0.0f;
	emittersCurrentlyStopped = 0;
	stopOnLoad = false;
}

Component * ParticleEffectComponent::Clone(Entity * toEntity)
{
	ParticleEffectComponent * newComponent = new ParticleEffectComponent();
	newComponent->SetEntity(toEntity);

	newComponent->stopAfterNRepeats = stopAfterNRepeats;
	newComponent->stopWhenEmpty = stopWhenEmpty;
	newComponent->needEmitPlaybackComplete = needEmitPlaybackComplete;
	newComponent->playbackComplete = playbackComplete;
	newComponent->effectDuration = effectDuration;
	newComponent->emittersCurrentlyStopped = emittersCurrentlyStopped;
	newComponent->stopOnLoad = stopOnLoad;

	return newComponent;
}

void ParticleEffectComponent::Start()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Play();
		}
	}

	this->emittersCurrentlyStopped = 0;
}

void ParticleEffectComponent::Stop()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Stop();
		}
		emittersCurrentlyStopped++;
	}
}

bool ParticleEffectComponent::IsStopped()
{
	// Effect is stopped if all its emitters are stopped.
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			if (!emitter->IsStopped())
			{
				return false;
			}
		}
	}
	
	return true;
}
	
void ParticleEffectComponent::Restart()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Restart();
		}
	}
}

void ParticleEffectComponent::StopAfterNRepeats(int32 numberOfRepeats)
{
    stopAfterNRepeats = numberOfRepeats;
}

void ParticleEffectComponent::StopWhenEmpty(bool value /*= true*/)
{
	stopWhenEmpty = value;
}

void ParticleEffectComponent::EffectUpdate(float32 timeElapsed)
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			if (IsStopEmitter(emitter))
			{
				emitter->Stop();
				this->emittersCurrentlyStopped++;

				// Are all the emitters finished playback? Notify user if yes.
				CheckPlaybackComplete();
			}
		}
	}
}

void ParticleEffectComponent::SetPlaybackCompleteMessage(const Message & msg)
{
	playbackComplete = msg;
}

void ParticleEffectComponent::UpdateDurationForChildNodes(float32 newEmitterLifeTime)
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->SetLifeTime(newEmitterLifeTime);
		}
	}
}

bool ParticleEffectComponent::IsStopEmitter(ParticleEmitter * emitter) const
{
	if (!emitter)
	{
		return true;
	}

	if (emitter->IsStopped())
	{
		// Already stopped - no need to stop again.
		return false;
	}

	// Check whether emitter is about to be stopped because of repeats count.
	if ((stopAfterNRepeats > 0) && (emitter->GetRepeatCount() >= stopAfterNRepeats))
	{
		return true;
	}

	// Check whether emitter is about to be stopped because of it is empty.
	if (stopWhenEmpty && emitter->GetParticleCount() == 0)
	{
		return true;
	}

	// No rules to stop emitter - continue its playback.
	return false;
}

void ParticleEffectComponent::CheckPlaybackComplete()
{
	if(entity->GetChildrenCount() == this->emittersCurrentlyStopped)
	{
		// Playback is finished!
		emittersCurrentlyStopped = 0;
		playbackComplete(entity, 0);
	}
}

float32 ParticleEffectComponent::GetPlaybackSpeed()
{
	// Ask the first emitter available.
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			return emitter->GetPlaybackSpeed();
		}
	}

	return 1.0f;
}

void ParticleEffectComponent::SetPlaybackSpeed(float32 value)
{
	// Update all emitters.
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->SetPlaybackSpeed(value);
		}
	}
}
	
int32 ParticleEffectComponent::GetActiveParticlesCount()
{
	int32 totalActiveParticles = 0;
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			totalActiveParticles += emitter->GetActiveParticlesCount();
		}
	}

	return totalActiveParticles;
}

void ParticleEffectComponent::SetStopOnLoad(bool value)
{
	this->stopOnLoad = value;
}

bool ParticleEffectComponent::IsStopOnLoad() const
{
	return this->stopOnLoad;
}

void ParticleEffectComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);
	if(archive)
	{
		archive->SetBool("pec.stoponload", this->stopOnLoad);
	}
}
	
void ParticleEffectComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(archive)
	{
		this->stopOnLoad = archive->GetBool("pec.stoponload", false);
	}
		
	Component::Deserialize(archive, sceneFile);
}

}
