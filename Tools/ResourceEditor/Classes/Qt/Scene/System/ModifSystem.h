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

#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Commands2/Command2.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/SceneTypes.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class EntityGroup;
class HoodSystem;

class EntityModificationSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys);
	~EntityModificationSystem();

	ST_Axis GetModifAxis() const;
	void SetModifAxis(ST_Axis axis);

	ST_ModifMode GetModifMode() const;
	void SetModifMode(ST_ModifMode mode);

protected:
	SceneCollisionSystem *collisionSystem;
	SceneCameraSystem *cameraSystem;
	HoodSystem* hoodSystem;

	void Update(DAVA::float32 timeElapsed);
	void Draw();

	void ProcessUIEvent(DAVA::UIEvent *event);
	void PropeccCommand(const Command2 *command, bool redo);

protected:
	struct EntityToModify
	{
		DAVA::Entity* entity;
		DAVA::Matrix4 originalTransform;
		DAVA::Vector3 originalCenter;
		DAVA::Matrix4 moveToZeroPos;
		DAVA::Matrix4 moveFromZeroPos;
	};

	bool inModifState;
	bool modified;

	ST_ModifMode curMode;
	ST_Axis curAxis;

	// starting modification pos
	DAVA::Vector3 modifStartPos3d;
	DAVA::Vector2 modifStartPos2d;

	// entities to modify
	DAVA::Vector<EntityToModify> modifEntities;

	// values calculated, when starting modification
	ST_PivotPoint modifPivotPoint;
	DAVA::Vector3 modifEntitiesCenter;
	DAVA::Matrix4 moveToZeroPosRelativeCenter;
	DAVA::Matrix4 moveFromZeroPosRelativeCenter;
	DAVA::Vector2 rotateNormal;
	DAVA::Vector3 rotateAround;
	DAVA::float32 crossXY;
	DAVA::float32 crossXZ;
	DAVA::float32 crossYZ;

	void BeginModification(const EntityGroup *entities);
	void EndModification();

	void ApplyModification();

	DAVA::Vector3 CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint);
	DAVA::Vector2 Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

	DAVA::Vector3 Move(const DAVA::Vector3 &newPos3d);
	DAVA::float32 Rotate(const DAVA::Vector2 &newPos2d);
	DAVA::float32 Scale(const DAVA::Vector2 &newPos2d);

	void MoveDone(const DAVA::Vector2 &newPos3d);
	void RotateDone(const DAVA::Vector2 &newPos2d);
	void ScaleDone(const DAVA::Vector2 &newPos2d);
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
