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

#ifndef __SCENE_CAMERA_SYSTEM_H__
#define __SCENE_CAMERA_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Camera.h"
#include "UI/UIEvent.h"

#include "Commands2/Command2.h"

class SceneCameraSystem : public DAVA::SceneSystem
{
	friend class SceneEditor2;

public:
	SceneCameraSystem(DAVA::Scene * scene);
	~SceneCameraSystem();

	DAVA::Vector3 GetPointDirection(const DAVA::Vector2 &point);
	DAVA::Vector3 GetCameraPosition();

	void SetMoveSeep(DAVA::float32 speed);
	DAVA::float32 GetMoveSpeed();

	void SetViewportRect(const DAVA::Rect &rect);
	const DAVA::Rect GetViewportRect();

	void LookAt(const DAVA::AABBox3 &box);
	void MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &direction = DAVA::Vector3());

	DAVA::Vector2 GetScreenPos(const DAVA::Vector3 &pos3);
	DAVA::Vector3 GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z);

protected:
	void Update(DAVA::float32 timeElapsed);
	void Draw();

	void ProcessUIEvent(DAVA::UIEvent *event);
	void ProcessCommand(const Command2 *command, bool redo);

protected:
	DAVA::Rect viewportRect;

	DAVA::float32 curSpeed;
	DAVA::Camera* curSceneCamera;

	DAVA::Vector2 rotateStartPoint;
	DAVA::Vector2 rotateStopPoint;

	bool animateToNewPos;
	DAVA::float32 animateToNewPosTime;
	DAVA::Vector3 newPos;
	DAVA::Vector3 newDir;

	DAVA::float32 curViewAngleZ, curViewAngleY;
	const DAVA::float32 maxViewAngle;

	void ProcessKeyboardMove(float timeElapsed);

	void RecalcCameraViewAngles();
	void MouseMoveCameraPosition();
	void MouseMoveCameraDirection();
	void MouseMoveCameraPosAroundPoint(const DAVA::Vector3 &point);

	void MoveAnimate(DAVA::float32 timeElapsed);
};

#endif
