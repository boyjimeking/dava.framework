#include "Scene/System/HoodSystem/MoveHood.h"
#include "Scene/System/ModifSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

MoveHood::MoveHood() : HoodObject(4.0f)
{
	DAVA::float32 b = baseSize / 5;
	DAVA::float32 c = baseSize / 3;

	axisX = CreateLine(DAVA::Vector3(b, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, b, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, b), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;

	axisXY1 = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(c, c, 0));
	axisXY1->axis = ST_AXIS_XY;

	axisXY2 = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(c, c, 0));
	axisXY2->axis = ST_AXIS_XY;

	axisXZ1 = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(c, 0, c));
	axisXZ1->axis = ST_AXIS_XZ;

	axisXZ2 = CreateLine(DAVA::Vector3(0, 0, c), DAVA::Vector3(c, 0, c));
	axisXZ2->axis = ST_AXIS_XZ;

	axisYZ1 = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(0, c, c));
	axisYZ1->axis = ST_AXIS_YZ;
	
	axisYZ2 = CreateLine(DAVA::Vector3(0, 0, c), DAVA::Vector3(0, c, c));
	axisYZ2->axis = ST_AXIS_YZ;
}

MoveHood::~MoveHood()
{

}

void MoveHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis)
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);
	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

	DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
	DAVA::Vector3 curPos = axisX->curPos;

	// x
	if(selectedAxis & ST_AXIS_X) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorX);

	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo);
	
	// y
	if(selectedAxis & ST_AXIS_Y) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorY);

	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo);

	// z
	if(selectedAxis & ST_AXIS_Z) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorZ);

	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo);

	// arrow length
	DAVA::float32 arrowLen = axisX->curScale * baseSize / 4;

	// arrow x
	DAVA::RenderManager::Instance()->SetColor(colorX);
	DAVA::RenderHelper::Instance()->FillArrow(axisX->curFrom, axisX->curTo, arrowLen, 0);

	// arrow y
	DAVA::RenderManager::Instance()->SetColor(colorY);
	DAVA::RenderHelper::Instance()->FillArrow(axisY->curFrom, axisY->curTo, arrowLen, 0);

	// arrow z
	DAVA::RenderManager::Instance()->SetColor(colorZ);
	DAVA::RenderHelper::Instance()->FillArrow(axisZ->curFrom, axisZ->curTo, arrowLen, 0);


	// xy
	if(selectedAxis == ST_AXIS_XY) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY1->curFrom, axisXY1->curTo);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY2->curFrom, axisXY2->curTo);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisXY1->curFrom);
		poly.AddPoint(axisXY1->curTo);
		poly.AddPoint(axisXY2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorX);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY1->curFrom, axisXY1->curTo);
		DAVA::RenderManager::Instance()->SetColor(colorY);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY2->curFrom, axisXY2->curTo);
	}

	// xz
	if(selectedAxis == ST_AXIS_XZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ1->curFrom, axisXZ1->curTo);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ2->curFrom, axisXZ2->curTo);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisXZ1->curFrom);
		poly.AddPoint(axisXZ1->curTo);
		poly.AddPoint(axisXZ2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorX);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ1->curFrom, axisXZ1->curTo);
		DAVA::RenderManager::Instance()->SetColor(colorZ);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ2->curFrom, axisXZ2->curTo);
	}

	// yz
	if(selectedAxis == ST_AXIS_YZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ1->curFrom, axisYZ1->curTo);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ2->curFrom, axisYZ2->curTo);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisYZ1->curFrom);
		poly.AddPoint(axisYZ1->curTo);
		poly.AddPoint(axisYZ2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorY);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ1->curFrom, axisYZ1->curTo);
		DAVA::RenderManager::Instance()->SetColor(colorZ);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ2->curFrom, axisYZ2->curTo);
	}

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->SetState(oldState);
}
