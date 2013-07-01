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

#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{

SwitchComponent::SwitchComponent()
:	oldSwitchIndex(-1),
	newSwitchIndex(0)
{

}

Component * SwitchComponent::Clone(Entity * toEntity)
{
	SwitchComponent * newComponent = new SwitchComponent();
	newComponent->SetEntity(toEntity);
	GlobalEventSystem::Instance()->Event(toEntity, this, EventSystem::SWITCH_CHANGED);
	return newComponent;
}

void SwitchComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(NULL != archive)
	{
		archive->SetInt32("sc.switchindex", newSwitchIndex);
	}
}

void SwitchComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		SetSwitchIndex(archive->GetInt32("sc.switchindex"));
	}

	Component::Deserialize(archive, sceneFile);
}

void SwitchComponent::SetSwitchIndex(const int32 & _switchIndex)
{
	newSwitchIndex = _switchIndex;

	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::SWITCH_CHANGED);
}

int32 SwitchComponent::GetSwitchIndex() const
{
	return newSwitchIndex;
}

}