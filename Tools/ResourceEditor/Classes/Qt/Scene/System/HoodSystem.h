#ifndef __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
#define __ENTITY_MODIFICATION_SYSTEM_HOOD_H__

#include "Scene/SceneTypes.h"
#include "Scene/System/HoodSystem/NormalHood.h"
#include "Scene/System/HoodSystem/MoveHood.h"
#include "Scene/System/HoodSystem/ScaleHood.h"
#include "Scene/System/HoodSystem/RotateHood.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

class SceneCameraSystem;

class HoodSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys);
	~HoodSystem();

	void SetModifMode(ST_ModifMode mode);
	ST_ModifMode GetModifMode() const;

	DAVA::Vector3 GetPosition() const;
	void SetPosition(const DAVA::Vector3 &pos);
	void MovePosition(const DAVA::Vector3 &offset);

	void SetModifAxis(ST_Axis axis);
	ST_Axis GetModifAxis() const;
	ST_Axis GetPassingAxis() const;

	void SetScale(DAVA::float32 scale);
	DAVA::float32 GetScale() const;

	void Lock();
	void Unlock();

	void Show();
	void Hide();

protected:
	bool locked;
	bool visible;

	ST_ModifMode curMode;
	ST_Axis curAxis;
	ST_Axis moseOverAxis;
	DAVA::Vector3 curPos;
	DAVA::Vector3 curOffset;
	DAVA::float32 curScale;

	SceneCameraSystem *cameraSystem;

	virtual void Update(float timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	void AddCollObjects(const DAVA::Vector<HoodCollObject*>* objects);
	void RemCollObjects(const DAVA::Vector<HoodCollObject*>* objects);

private:
	btCollisionWorld* collWorld;
	btAxisSweep3* collBroadphase;
	btDefaultCollisionConfiguration* collConfiguration;
	btCollisionDispatcher* collDispatcher;
	btIDebugDraw* collDebugDraw;

	HoodObject *curHood;

	NormalHood normalHood;
	MoveHood moveHood;
	RotateHood rotateHood;
	ScaleHood scaleHood;
};

#endif // __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
