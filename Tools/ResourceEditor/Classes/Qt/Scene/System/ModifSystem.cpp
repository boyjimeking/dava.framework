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

#include "Qt/Scene/System/ModifSystem.h"
#include "Qt/Scene/System/HoodSystem.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/SceneSignals.h"

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"

#include "Commands/CommandsManager.h"
#include "Commands/EditorBodyControlCommands.h"

#include "Commands2/TransformCommand.h"

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(colSys)
	, cameraSystem(camSys)
	, hoodSystem(hoodSys)
	, inModifState(false)
	, modified(false)
{
	SetModifMode(ST_MODIF_MOVE);
	SetModifAxis(ST_AXIS_Z);
}

EntityModificationSystem::~EntityModificationSystem()
{ }

void EntityModificationSystem::SetModifAxis(ST_Axis axis)
{
	if(axis != ST_AXIS_NONE)
	{
		curAxis = axis;
		hoodSystem->SetModifAxis(axis);
	}
}

ST_Axis EntityModificationSystem::GetModifAxis() const
{
	return curAxis;
}

void EntityModificationSystem::SetModifMode(ST_ModifMode mode)
{
	curMode = mode;
	hoodSystem->SetModifMode(mode);
}

ST_ModifMode EntityModificationSystem::GetModifMode() const
{
	return curMode;
}

void EntityModificationSystem::Update(DAVA::float32 timeElapsed)
{ }

void EntityModificationSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(NULL != collisionSystem)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
		const EntityGroup *selectedEntities = selectionSystem->GetSelection();

		// remember current cursor point, when looking from current camera
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!inModifState)
		{
			bool modifCanStart = false;

			// we can start modification only if mouse is over hood
			// on mouse is over one of currently selected items
			ST_Axis mouseOverAxis = hoodSystem->GetPassingAxis();
			if(mouseOverAxis != ST_AXIS_NONE)
			{
				// allow starting modification
				modifCanStart = true;
			}
			else if(selectedEntities->Size() > 0)
			{
				// send this ray to collision system and get collision objects
				const EntityGroup *collisionEntities = collisionSystem->ObjectsRayTestFromCamera();

				// check if one of got collision objects is intersected with selected items
				// if so - we can start modification
				if(NULL != selectedEntities->IntersectedEntity(collisionEntities))
				{
					modifCanStart = true;
				}
			}

			// can we start modification???
			if(modifCanStart)
			{
				SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2 *) GetScene(), selectedEntities);

				if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
				{
					if(event->tid == DAVA::UIEvent::BUTTON_1)
					{
						// go to modification state
						inModifState = true;

						// select current hood axis as active
						if(curMode == ST_MODIF_MOVE || curMode == ST_MODIF_ROTATE)
						{
							SetModifAxis(mouseOverAxis);
						}

						// set entities to be modified
						BeginModification(selectedEntities);

						// init some values, needed for modifications
						modifStartPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						modifStartPos2d = event->point;
					}
				}
			}
			else
			{
				SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2 *) GetScene(), NULL);
			}
		}
		// or we are already in modification state
		else
		{
			// phase still continue
			if(event->phase == DAVA::UIEvent::PHASE_DRAG)
			{
				DAVA::Vector3 moveOffset;
				DAVA::float32 rotateAngle;
				DAVA::float32 scaleForce;

				switch (curMode)
				{
				case ST_MODIF_MOVE:
					{
						DAVA::Vector3 newPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						moveOffset = Move(newPos3d);
						modified = true;
					}
					break;
				case ST_MODIF_ROTATE:
					{
						rotateAngle = Rotate(event->point);
						modified = true;
					}
					break;
				case ST_MODIF_SCALE:
					{
						scaleForce = Scale(event->point);
						modified = true;
					}
					break;
				default:
					break;
				}

				if(modified)
				{
					// say to selection system, that selected items were modified
					selectionSystem->SelectedItemsWereModified();

					// lock hood, so it wont process ui events, wont calc. scale depending on it current position
					hoodSystem->Lock();
					hoodSystem->MovePosition(moveOffset);

					// TODO:
					// emit move/rotate/scale offset/angle/force
					// ...
				}
			}
			// phase ended
			else if(event->phase == DAVA::UIEvent::PHASE_ENDED)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					if(modified)
					{
						ApplyModification();
					}

					hoodSystem->Unlock();

					EndModification();
					inModifState = false;
					modified = false;
				}
			}
		}
	}
}

void EntityModificationSystem::Draw()
{ }

void EntityModificationSystem::PropeccCommand(const Command2 *command, bool redo)
{

}


void EntityModificationSystem::BeginModification(const EntityGroup *entities)
{
	// clear any priv. selection
	EndModification();

	if(NULL != entities)
	{
		for(size_t i = 0; i < entities->Size(); ++i)
		{
			DAVA::Entity *en = entities->GetSolidEntity(i);

			if(NULL == en)
			{
				en = entities->GetEntity(i);
			}

			if(NULL != en)
			{
				EntityToModify etm;
				etm.entity = en;
				etm.originalCenter = en->GetWorldTransform().GetTranslationVector();
				etm.originalTransform = en->GetLocalTransform();
				etm.moveToZeroPos.CreateTranslation(-etm.originalCenter);
				etm.moveFromZeroPos.CreateTranslation(etm.originalCenter);
				modifEntities.push_back(etm);
			}
		}

		// center of this bbox will modification center, common for all entities
		modifEntitiesCenter = entities->GetCommonBbox().GetCenter();

		// prepare translation matrix's, used before and after rotation
		moveToZeroPosRelativeCenter.CreateTranslation(-modifEntitiesCenter);
		moveFromZeroPosRelativeCenter.CreateTranslation(modifEntitiesCenter);

		// remember axis vector we are rotating around
		switch(curAxis)
		{
		case ST_AXIS_X:
		case ST_AXIS_YZ:
			rotateAround = DAVA::Vector3(1, 0, 0);
			break;
		case ST_AXIS_Y:
		case ST_AXIS_XZ:
			rotateAround = DAVA::Vector3(0, 1, 0);
			break;
		case ST_AXIS_XY:
		case ST_AXIS_Z:
			rotateAround = DAVA::Vector3(0, 0, 1);
			break;
		}

		// 2d axis projection we are rotating around
		DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

		// axis dot products
		DAVA::Vector2 zeroPos = cameraSystem->GetScreenPos(modifEntitiesCenter);
		DAVA::Vector2 xPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(1, 0, 0));
		DAVA::Vector2 yPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 1, 0));
		DAVA::Vector2 zPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 0, 1));

		DAVA::Vector2 vx = xPos - zeroPos;
		DAVA::Vector2 vy = yPos - zeroPos;
		DAVA::Vector2 vz = zPos - zeroPos;

		crossXY = Abs(vx.CrossProduct(vy));
		crossXZ = Abs(vx.CrossProduct(vz));
		crossYZ = Abs(vy.CrossProduct(vz));

		// real rotate should be done in direction of 2dAxis normal,
		// so calculate this normal
		rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
		rotateNormal.Normalize();

		// remember current selection pivot point
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
		modifPivotPoint = selectionSystem->GetPivotPoint();
	}
}

void EntityModificationSystem::EndModification()
{
	modifEntitiesCenter.Set(0, 0, 0);
	modifEntities.clear();
}

void EntityModificationSystem::ApplyModification()
{
	SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());

	if(NULL != sceneEditor)
	{
		bool isMultiple = (modifEntities.size() > 1);

		if(isMultiple)
		{
			sceneEditor->BeginBatch("Multiple transform");
		}

		for (size_t i = 0; i < modifEntities.size(); ++i)
		{
			sceneEditor->Exec(new TransformCommand(modifEntities[i].entity,	modifEntities[i].originalTransform, modifEntities[i].entity->GetLocalTransform()));
		}

		if(isMultiple)
		{
			sceneEditor->EndBatch();
		}
	}
}

DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint)
{
	DAVA::Vector3 ret;
	DAVA::Vector3 planeNormal;

	switch(curAxis)
	{
	case ST_AXIS_X:
		{
			if(crossXY > crossXZ) planeNormal = DAVA::Vector3(0, 0, 1);
			else planeNormal = DAVA::Vector3(0, 1, 0);
		}
		break;
	case ST_AXIS_Y:
		{
			if(crossXY > crossYZ) planeNormal = DAVA::Vector3(0, 0, 1);
			else planeNormal = DAVA::Vector3(1, 0, 0);
		}
		break;
	case ST_AXIS_Z:
		{
			if(crossXZ > crossYZ) planeNormal = DAVA::Vector3(0, 1, 0);
			else planeNormal = DAVA::Vector3(1, 0, 0);
		}
		break;
	case ST_AXIS_XZ:
		planeNormal = DAVA::Vector3(0, 1, 0);
		break;
	case ST_AXIS_YZ:
		planeNormal = DAVA::Vector3(1, 0, 0);
		break;
	case ST_AXIS_XY:
	default:
		planeNormal = DAVA::Vector3(0, 0, 1);
		break;
	}

	DAVA::Plane plane(planeNormal, planePoint);
	DAVA::float32 distance = FLT_MAX;

	plane.IntersectByRay(camPosition, camPointDirection, distance);
	ret = camPosition + (camPointDirection * distance);
	
	return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Vector2 axisBegin = cameraSystem->GetScreenPos(from);
	DAVA::Vector2 axisEnd = cameraSystem->GetScreenPos(to);
	DAVA::Vector2 ret = axisEnd - axisBegin;
	ret.Normalize();

	return ret;
}

DAVA::Vector3 EntityModificationSystem::Move(const DAVA::Vector3 &newPos3d)
{
	DAVA::Vector3 moveOffset;
	DAVA::Vector3 modifPosWithLocedAxis = modifStartPos3d;
	DAVA::Vector3 deltaPos3d = newPos3d - modifStartPos3d;

	switch(curAxis)
	{
	case ST_AXIS_X:
		modifPosWithLocedAxis.x += DAVA::Vector3(1, 0, 0).DotProduct(deltaPos3d);
		break;
	case ST_AXIS_Y:
		modifPosWithLocedAxis.y += DAVA::Vector3(0, 1, 0).DotProduct(deltaPos3d);
		break;
	case ST_AXIS_Z:
		modifPosWithLocedAxis.z += DAVA::Vector3(0, 0, 1).DotProduct(deltaPos3d);
		break;
	case ST_AXIS_XY:
		modifPosWithLocedAxis.x = newPos3d.x;
		modifPosWithLocedAxis.y = newPos3d.y;
		break;
	case ST_AXIS_XZ:
		modifPosWithLocedAxis.x = newPos3d.x;
		modifPosWithLocedAxis.z = newPos3d.z;
		break;
	case ST_AXIS_YZ:
		modifPosWithLocedAxis.z = newPos3d.z;
		modifPosWithLocedAxis.y = newPos3d.y;
		break;
	}

	moveOffset = modifPosWithLocedAxis - modifStartPos3d;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 moveModification;
		moveModification.Identity();
		moveModification.CreateTranslation(moveOffset);

		DAVA::Matrix4 newLocalTransform = modifEntities[i].originalTransform * moveModification;
		modifEntities[i].entity->SetLocalTransform(newLocalTransform);
	}

	return moveOffset;
}

DAVA::float32 EntityModificationSystem::Rotate(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
	DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength)) / 70.0f;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 rotateModification;
		rotateModification.Identity();
		rotateModification.CreateRotation(rotateAround, rotateForce);

		switch(modifPivotPoint)
		{
		case ST_PIVOT_ENTITY_CENTER:
			// move to zero, rotate, move back to original center point
			rotateModification = (modifEntities[i].moveToZeroPos * rotateModification) * modifEntities[i].moveFromZeroPos;
			break;
		case ST_PIVOT_COMMON_CENTER:
			// move to zero relative selection center, rotate, move back to original center point
			rotateModification = (moveToZeroPosRelativeCenter * rotateModification) * moveFromZeroPosRelativeCenter;
			break;
		default:
			rotateModification.Identity();
			break;
		}

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * rotateModification);
	}

	return rotateForce;
}

DAVA::float32 EntityModificationSystem::Scale(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 scaleDir = (newPos2d - modifStartPos2d);
	DAVA::float32 scaleForce;

	scaleForce = 1.0f - (scaleDir.y / 70.0f);

	if(scaleForce >= 0)
	{
		for (size_t i = 0; i < modifEntities.size(); ++i)
		{
			DAVA::Matrix4 scaleModification;
			scaleModification.Identity();
			scaleModification.CreateScale(DAVA::Vector3(scaleForce, scaleForce, scaleForce));

			switch(modifPivotPoint)
			{
			case ST_PIVOT_ENTITY_CENTER:
				// move to zero, rotate, move back to original center point
				scaleModification = (modifEntities[i].moveToZeroPos * scaleModification) * modifEntities[i].moveFromZeroPos;
				break;
			case ST_PIVOT_COMMON_CENTER:
				// move to zero relative selection center, rotate, move back to original center point
				scaleModification = (moveToZeroPosRelativeCenter * scaleModification) * moveFromZeroPosRelativeCenter;
				break;
			default:
				scaleModification.Identity();
				break;
			}
			
			modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * scaleModification);
		}
	}

	return scaleForce;
}
