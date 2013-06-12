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

#include "LayerForceWidget.h"
#include "TimeLineWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include "../Scene/SceneDataManager.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QSizePolicy>

LayerForceWidget::LayerForceWidget(QWidget *parent):
	QWidget(parent)
{
	mainBox = new QVBoxLayout;
	this->setLayout(mainBox);
	
	forceTimeLine = new TimeLineWidget(this);
	InitWidget(forceTimeLine);
	forceVariationTimeLine = new TimeLineWidget(this);
	InitWidget(forceVariationTimeLine);
	forceOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(forceOverLifeTimeLine);
	
	blockSignals = false;
}

LayerForceWidget::~LayerForceWidget()
{
	
}


void LayerForceWidget::InitWidget(QWidget* widget)
{
	mainBox->addWidget(widget);
	connect(widget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void LayerForceWidget::Init(ParticleEmitter* emitter, ParticleLayer* layer, uint32 forceIndex, bool updateMinimized)
{	
	if (!layer || layer->forces.size() <= forceIndex)
	{
		return;
	}
	
	this->emitter = emitter;
	this->layer = layer;
	this->forceIndex = forceIndex;
	
	blockSignals = true;
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);
	ParticleForce* curForce = layer->forces[forceIndex];

	Vector<QColor> colors;
	colors.push_back(Qt::red); colors.push_back(Qt::darkGreen); colors.push_back(Qt::blue);
	Vector<QString> legends;
	legends.push_back("force x"); legends.push_back("force y"); legends.push_back("force z");
	forceTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceTimeLine->AddLines(PropLineWrapper<Vector3>(curForce->GetForce()).GetProps(), colors, legends);
	forceTimeLine->EnableLock(true);

	legends.clear();
	legends.push_back("force variable x"); legends.push_back("force variable y"); legends.push_back("force variable z");
	forceVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceVariationTimeLine->AddLines(PropLineWrapper<Vector3>(curForce->GetForceVariation()).GetProps(), colors, legends);
	forceVariationTimeLine->EnableLock(true);

	forceOverLifeTimeLine->Init(0, 1, updateMinimized, true, false);
	forceOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(curForce->GetForceOverlife()).GetProps(), Qt::blue, "forces over life");

	blockSignals = false;
}

void LayerForceWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	forceTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_PROPS"));
	forceVariationTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_VARIATION_PROPS"));
	forceOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_OVER_LIFE_PROPS"));
}

void LayerForceWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	forceTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_PROPS", props);

	props->DeleteAllKeys();
	forceVariationTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_VARIATION_PROPS", props);

	props->DeleteAllKeys();
	forceOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_OVER_LIFE_PROPS", props);

	SafeRelease(props);
}

void LayerForceWidget::OnValueChanged()
{
	if (blockSignals)
		return;
	
	PropLineWrapper<Vector3> propForce;
	forceTimeLine->GetValues(propForce.GetPropsPtr());
	PropLineWrapper<Vector3> propForceVariable;
	forceVariationTimeLine->GetValues(propForceVariable.GetPropsPtr());
	PropLineWrapper<float32> propForceOverLife;
	forceOverLifeTimeLine->GetValue(0, propForceOverLife.GetPropsPtr());

	CommandUpdateParticleForce* updateForceCmd = new CommandUpdateParticleForce(layer, forceIndex);
	updateForceCmd->Init(propForce.GetPropLine(),
						 propForceVariable.GetPropLine(),
						 propForceOverLife.GetPropLine());
	
	CommandsManager::Instance()->ExecuteAndRelease(updateForceCmd,
												   SceneDataManager::Instance()->SceneGetActive()->GetScene());

	Init(emitter, layer, forceIndex, false);
	emit ValueChanged();
}

void LayerForceWidget::Update()
{
	Init(emitter, layer, forceIndex, false);
}