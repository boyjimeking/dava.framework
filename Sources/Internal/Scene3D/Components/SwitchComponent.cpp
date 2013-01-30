#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{

SwitchComponent::SwitchComponent()
:	oldSwitchIndex(-1),
	newSwitchIndex(0)
{

}

Component * SwitchComponent::Clone(SceneNode * toEntity)
{
	SwitchComponent * newComponent = new SwitchComponent();
	newComponent->SetEntity(toEntity);
	Scene::GetActiveScene()->ImmediateEvent(toEntity, GetType(), EventSystem::SWITCH_CHANGED);
	return newComponent;
}

void SwitchComponent::SetSwitchIndex(const int32 & _switchIndex)
{
	newSwitchIndex = _switchIndex;

	Scene::GetActiveScene()->ImmediateEvent(entity, GetType(), EventSystem::SWITCH_CHANGED);
}

int32 SwitchComponent::GetSwitchIndex() const
{
	return newSwitchIndex;
}

}