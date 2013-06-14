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

#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/SceneSignals.h"

#include <QApplication>
#include "Scene/SceneEditor2.h"

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(collSys)
	, hoodSystem(hoodSys)
	, drawMode(ST_SELDRAW_FILL_SHAPE | ST_SELDRAW_DRAW_CORNERS)
	, curPivotPoint(ST_PIVOT_COMMON_CENTER)
	, applyOnPhaseEnd(false)
{

}

SceneSelectionSystem::~SceneSelectionSystem()
{

}

void SceneSelectionSystem::Update(DAVA::float32 timeElapsed)
{
	UpdateHoodPos();
}

void SceneSelectionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
	{
		// we can select only if mouse isn't over hood axis
		if(ST_AXIS_NONE == hoodSystem->GetPassingAxis())
		{
			if(event->tid == DAVA::UIEvent::BUTTON_1)
			{
				const EntityGroup* collisionEntities = collisionSystem->ObjectsRayTestFromCamera();
				EntityGroup selectableItems = GetSelecetableFromCollision(collisionEntities);

				DAVA::Entity *firstEntity = selectableItems.GetEntity(0);
				DAVA::Entity *nextEntity = selectableItems.GetEntity(0);

				// search possible next item only if now there is no selection or is only single selection
				if(curSelections.Size() <= 1)
				{
					// find first after currently selected items
					for(size_t i = 0; i < selectableItems.Size(); i++)
					{
						DAVA::Entity *entity = selectableItems.GetEntity(i);
						if(curSelections.HasEntity(entity))
						{
							if((i + 1) < selectableItems.Size())
							{
								nextEntity = selectableItems.GetEntity(i + 1);
								break;
							}
						}
					}
				}

				int curKeyModifiers = QApplication::keyboardModifiers();
				if(curKeyModifiers & Qt::ControlModifier)
				{
					AddSelection(firstEntity);
				}
				else if(curKeyModifiers & Qt::AltModifier)
				{
					RemSelection(firstEntity);
				}
				else
				{
					// if new selection is NULL or is one of already selected items
					// we should change current selection only on phase end
					if(nextEntity == NULL || NULL != curSelections.IntersectedEntity(collisionEntities))
					{
						applyOnPhaseEnd = true;
						lastSelection = nextEntity;
					}
					else
					{
						SetSelection(nextEntity);
					}
				}
			}
		}
	}
	else if(DAVA::UIEvent::PHASE_ENDED == event->phase)
	{
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			if(applyOnPhaseEnd)
			{
				applyOnPhaseEnd = false;
				SetSelection(lastSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if(curSelections.Size() > 0)
	{
		int oldState = DAVA::RenderManager::Instance()->GetState();
		DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
		DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

		int newState = DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL;

		if(!(drawMode & ST_SELDRAW_NO_DEEP_TEST))
		{
			newState |= DAVA::RenderState::STATE_DEPTH_TEST;
		}

		DAVA::RenderManager::Instance()->SetState(newState);
		DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

		for (DAVA::uint32 i = 0; i < curSelections.Size(); i++)
		{
			DAVA::AABBox3 selectionBox = curSelections.GetBbox(i);

			// draw selection share
			if(drawMode & ST_SELDRAW_DRAW_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawBox(selectionBox);
			}
			// draw selection share
			else if(drawMode & ST_SELDRAW_DRAW_CORNERS)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawCornerBox(selectionBox);
			}

			// fill selection shape
			if(drawMode & ST_SELDRAW_FILL_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				DAVA::RenderHelper::Instance()->FillBox(selectionBox);
			}
		}

		DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void SceneSelectionSystem::PropeccCommand(const Command2 *command, bool redo)
{

}

void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	// emit deselection for current selected items
	for(size_t i = 0; i < curSelections.Size(); ++i)
	{
		EntityGroupItem selectedItem = curSelections.GetItem(i);
		SceneSignals::Instance()->EmitDeselected((SceneEditor2 *) GetScene(), selectedItem.solidEntity);
	}

	// clear current selection
	curSelections.Clear();

	// add new selection
	if(NULL != entity)
	{
		EntityGroupItem selectableItem = GetSelectableEntity(entity);
		curSelections.Add(selectableItem);

		SceneSignals::Instance()->EmitSelected((SceneEditor2 *) GetScene(), selectableItem.solidEntity);
	}
	else
	{
		SceneSignals::Instance()->EmitSelected((SceneEditor2 *) GetScene(), NULL);
	}

	UpdateHoodPos();
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		EntityGroupItem selectableItem = GetSelectableEntity(entity);
		curSelections.Add(selectableItem);

		SceneSignals::Instance()->EmitSelected((SceneEditor2 *) GetScene(), selectableItem.solidEntity);
	}

	UpdateHoodPos();
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	curSelections.Rem(entity);

	EntityGroupItem selectableItem = GetSelectableEntity(entity);
	SceneSignals::Instance()->EmitDeselected((SceneEditor2 *) GetScene(), selectableItem.solidEntity);

	UpdateHoodPos();
}

const EntityGroup* SceneSelectionSystem::GetSelection() const
{
	return &curSelections;
}

void SceneSelectionSystem::SelectedItemsWereModified()
{
	// don't change selection on phase end
	applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(ST_PivotPoint pp)
{
	curPivotPoint = pp;
}

ST_PivotPoint SceneSelectionSystem::GetPivotPoint() const
{
	return curPivotPoint;
}

void SceneSelectionSystem::UpdateHoodPos() const
{
	if(curSelections.Size() > 0)
	{
		DAVA::Vector3 p;
		switch (curPivotPoint)
		{
		case ST_PIVOT_ENTITY_CENTER:
			p = curSelections.GetBbox(0).GetCenter();
			break;
		default:
			p = curSelections.GetCommonBbox().GetCenter();
			break;
		}

		hoodSystem->SetPosition(p);
		hoodSystem->Show();
	}
	else
	{
		hoodSystem->SetPosition(DAVA::Vector3(0, 0, 0));
		hoodSystem->Hide();
	}
}

EntityGroup SceneSelectionSystem::GetSelecetableFromCollision(const EntityGroup *collisionEntities)
{
	EntityGroup ret;

	if(NULL != collisionEntities)
	{
		for(size_t i = 0; i < collisionEntities->Size(); ++i)
		{
			DAVA::Entity *entity = collisionEntities->GetEntity(i);
			EntityGroupItem item = GetSelectableEntity(entity);

			ret.Add(item);
		}
	}

	return ret;
}

EntityGroupItem SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
	EntityGroupItem ret;
	DAVA::Entity *solidEntity = entity;
	
	if(NULL != entity)
	{
		// find real solid entity
		solidEntity = entity;
		while(NULL != solidEntity && !solidEntity->GetSolid())
		{
			solidEntity = solidEntity->GetParent();
		}

		// if there is no solid entity, try to find lod parent entity
		if(NULL == solidEntity)
		{
			// find entity that has lod component
			solidEntity = entity;
			while(NULL != solidEntity && NULL == solidEntity->GetComponent(DAVA::Component::LOD_COMPONENT))
			{
				solidEntity = solidEntity->GetParent();
			}
		}

		// still not found?
		if(NULL == solidEntity)
		{
			// let it current entity to be tread as solid
			solidEntity = entity;
		}

		ret.entity = entity;
		ret.solidEntity = solidEntity;
		ret.bbox = CalcAABox(solidEntity);
	}

	return ret;
}

DAVA::AABBox3 SceneSelectionSystem::CalcAABox(DAVA::Entity *entity) const
{
	DAVA::AABBox3 ret;

	if(NULL != entity)
	{
		// we will get selection bbox from collision system
		ret = collisionSystem->GetBoundingBox(entity);

		if(ret.IsEmpty())
		{
			for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
			{
				ret.AddAABBox(collisionSystem->GetBoundingBox(entity->GetChild(i)));
			}
		}
	}

	return ret;
}

void SceneSelectionSystem::SetDrawMode(int mode)
{
	drawMode = mode;
}

int SceneSelectionSystem::GetDrawMode() const
{
	return drawMode;
}
