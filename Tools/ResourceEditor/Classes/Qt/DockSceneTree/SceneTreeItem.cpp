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

#include <QSet>
#include "DockSceneTree/SceneTreeItem.h"

// framework
#include "Scene3d/Components/ComponentHelpers.h"

SceneTreeItem::SceneTreeItem(eItemType _type)
	: type(_type)
{ }

SceneTreeItem::~SceneTreeItem()
{ }

QVariant SceneTreeItem::data(int role) const
{
	QVariant v;

	switch(role)
	{
	case Qt::DecorationRole:
		v = ItemIcon();
		break;
	case Qt::DisplayRole:
		v = ItemName();
		break;
	case EIDR_Type:
		v = ItemData();
		break;
	case EIDR_Data:
		v = ItemType();
		break;
	default:
		break;
	}

	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}


int SceneTreeItem::ItemType() const
{
	return type;
}

QIcon SceneTreeItem::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/node.png");
	return icon;
}

// =========================================================================================
// SceneTreeItemEntity
// =========================================================================================

SceneTreeItemEntity::SceneTreeItemEntity(DAVA::Entity* _entity)
	: SceneTreeItem(SceneTreeItem::EIT_Entity)
	, entity(_entity)
{
	DoSync(this, entity);
}

SceneTreeItemEntity::~SceneTreeItemEntity()
{ }

DAVA::Entity* SceneTreeItemEntity::GetEntity(SceneTreeItem *item)
{
	DAVA::Entity *ret = NULL;

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Entity)
	{
		SceneTreeItemEntity *itemEntity = (SceneTreeItemEntity *) item;
		ret = itemEntity->entity;
	}

	return ret;
}

QString SceneTreeItemEntity::ItemName() const
{
	QString ret; 

	if(NULL != entity)
	{
		ret = entity->GetName().c_str();
	}

	return ret;
}

QVariant SceneTreeItemEntity::ItemData() const
{
	return qVariantFromValue(entity);
}

QIcon SceneTreeItemEntity::ItemIcon() const
{
	static QIcon effectIcon(":/QtIcons/effect.png");
	static QIcon emitterIcon(":/QtIcons/emitter_particle.png");
	static QIcon renderobjIcon(":/QtIcons/render_object.png");
	static QIcon landscapeIcon(":/QtIcons/heightmapeditor.png");

	QIcon ret;

	if(NULL != entity)
	{
		if(NULL != DAVA::GetEmitter(entity))
		{
			ret = emitterIcon;
		}
		else if(NULL != DAVA::GetEffectComponent(entity))
		{
			ret = effectIcon;
		}
		else if(NULL != DAVA::GetLandscape(entity))
		{
			ret = landscapeIcon;
		}
		else if(NULL != GetLodComponent(entity))
		{
			// TODO:
			// ...
		}
		else if(NULL != DAVA::GetRenderObject(entity))
		{
			ret = renderobjIcon;
		}
	}

	if(ret.isNull())
	{
		ret = SceneTreeItem::ItemIcon();
	}

	return ret;
}

void SceneTreeItemEntity::DoSync(QStandardItem *rootItem, DAVA::Entity *entity)
{
	if(NULL != rootItem && NULL != entity)
	{
		DAVA::int32 i;
		QSet<DAVA::Entity *> entitiesSet;
		QSet<DAVA::ParticleLayer *> layersSet;

		DAVA::ParticleEmitter *emitter = DAVA::GetEmitter(entity);
		DAVA::Vector<DAVA::ParticleLayer*> *emitterLayers = NULL;

		// remember all entity childs
		for(i = 0; i < entity->GetChildrenCount(); ++i)
		{
			entitiesSet.insert(entity->GetChild(i));
		}

		// remember all particle layers
		if(NULL != emitter)
		{
			emitterLayers = &emitter->GetLayers();
			for(size_t i = 0; i < emitterLayers->size(); ++i)
			{
				layersSet.insert(emitterLayers->operator[](i));
			}
		}

		// remove items that are not in set
		for(int i = 0; i < rootItem->rowCount(); ++i)
		{
			bool doRemove = true;
			SceneTreeItem *childItem = (SceneTreeItem *) rootItem->child(i);

			if(childItem->ItemType() == SceneTreeItem::EIT_Entity)
			{
				SceneTreeItemEntity *entityItem = (SceneTreeItemEntity *) childItem;
				if(entitiesSet.contains(entityItem->entity))
				{
					doRemove = false;
				}
			}
			else if(childItem->ItemType() == SceneTreeItem::EIT_Layer)
			{
				SceneTreeItemParticleLayer *layerItem = (SceneTreeItemParticleLayer *) childItem;
				if(layersSet.contains(layerItem->layer))
				{
					doRemove = false;
				}
			}

			if(doRemove)
			{
				rootItem->removeRow(i);
				i--;
			}
		}

		entitiesSet.clear();
		layersSet.clear();

		// add entities
		int row = 0;

		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			bool repeatStep;
			DAVA::Entity *childEntity = entity->GetChild(i);

			do
			{
				SceneTreeItem *item = (SceneTreeItem *) rootItem->child(row);
				DAVA::Entity *itemEntity = SceneTreeItemEntity::GetEntity(item);

				repeatStep = false;

				// remove items that we already add
				while(entitiesSet.contains(itemEntity))
				{
					rootItem->removeRow(row);

					item = (SceneTreeItem *) rootItem->child(row);
					itemEntity = SceneTreeItemEntity::GetEntity(item);
				}

				// append entity that isn't in child items list
				if(NULL == item)
				{
					rootItem->appendRow(new SceneTreeItemEntity(childEntity));
				}
				else if(childEntity != itemEntity)
				{
					// now we should decide what to do: remove item or insert it

					// calc len until itemEntity will be found in real entity childs
					int lenUntilRealEntity = 0;
					for(int j = i; j < entity->GetChildrenCount(); ++j)
					{
						if(entity->GetChild(j) == itemEntity)
						{
							lenUntilRealEntity = j - i;
							break;
						}
					}

					// calc len until current real entity child will be found in current item childs
					int lenUntilItem = 0;
					for(int j = i; j < rootItem->rowCount(); ++j)
					{
						SceneTreeItem *itm = (SceneTreeItem *) rootItem->child(j);
						DAVA::Entity *itmEn = SceneTreeItemEntity::GetEntity(itm);

						if(childEntity == itmEn)
						{
							lenUntilItem = j - i;
							break;
						}
					}

					if(lenUntilRealEntity >= lenUntilItem)
					{
						rootItem->removeRow(row);
						repeatStep = true;
					}
					else
					{
						rootItem->insertRow(row, new SceneTreeItemEntity(childEntity));
					}
				}
				else
				{
					DoSync(item, itemEntity);
				}
			}
			while(repeatStep);

			// remember that we add that entity
			entitiesSet.insert(childEntity);

			row++;
		}

		// add layers
		if(NULL != emitterLayers)
		{
			for(size_t i = 0; i < emitterLayers->size(); ++i)
			{
				bool repeatStep;
				DAVA::ParticleLayer* childLayer = emitterLayers->operator[](i);

				do 
				{
					SceneTreeItem *item = (SceneTreeItem *) rootItem->child(row);
					DAVA::ParticleLayer *itemLayer = SceneTreeItemParticleLayer::GetLayer(item);

					repeatStep = false;

					// remove items that we already add
					while(layersSet.contains(itemLayer))
					{
						rootItem->removeRow(row);

						SceneTreeItem *item = (SceneTreeItem *) rootItem->child(row);
						DAVA::ParticleLayer *itemLayer = SceneTreeItemParticleLayer::GetLayer(item);
					}

					if(NULL == item)
					{
						rootItem->appendRow(new SceneTreeItemParticleLayer(entity, childLayer));
					}
					else if(childLayer != itemLayer)
					{
						// now we should decide what to do: remove layer or insert it

						// calc len until itemEntity will be found in real
						int lenUntilRealLayer = 0;
						for(int j = i; j < (int) emitterLayers->size(); ++j)
						{
							if(emitterLayers->operator[](j) == itemLayer)
							{
								lenUntilRealLayer = j - i;
								break;
							}
						}

						// calc len until current real entity child will be found in current item childs
						int lenUntilItem = 0;
						for(int j = i; j < rootItem->rowCount(); ++j)
						{
							SceneTreeItem *itm = (SceneTreeItem *) rootItem->child(j);
							DAVA::ParticleLayer *itmLay = SceneTreeItemParticleLayer::GetLayer(itm);

							if(childLayer == itmLay)
							{
								lenUntilItem = j - i;
								break;
							}
						}

						if(lenUntilRealLayer >= lenUntilItem)
						{
							rootItem->removeRow(row);
							repeatStep = true;
						}
						else
						{
							rootItem->insertRow(row, new SceneTreeItemParticleLayer(entity, childLayer));
						}
					}
					else
					{
						SceneTreeItemParticleLayer::DoSync(item, itemLayer);
					}
				} while (repeatStep);

				row++;
				layersSet.insert(childLayer);
			}
		}

		// remove all other rows
		if(row < rootItem->rowCount())
		{
			rootItem->removeRows(row, rootItem->rowCount() - row);
		}
	}
}

// =========================================================================================
// SceneTreeItemParticleLayer
// =========================================================================================

SceneTreeItemParticleLayer::SceneTreeItemParticleLayer(DAVA::Entity* _parent, DAVA::ParticleLayer *_layer)
	: SceneTreeItem(SceneTreeItem::EIT_Layer)
	, parent(_parent)
	, layer(_layer)
{
	if(NULL != layer)
	{
		setCheckable(true);

		if(layer->GetDisabled())
		{
			setCheckState(Qt::Unchecked);
		}
		else
		{
			setCheckState(Qt::Checked);
		}
	}

	DoSync(this, layer);
}

SceneTreeItemParticleLayer::~SceneTreeItemParticleLayer()
{ }

DAVA::ParticleLayer* SceneTreeItemParticleLayer::GetLayer(SceneTreeItem *item)
{
	DAVA::ParticleLayer *ret = NULL;

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Layer)
	{
		SceneTreeItemParticleLayer *itemLayer = (SceneTreeItemParticleLayer *) item;
		ret = itemLayer->layer;
	}

	return ret;
}

QString SceneTreeItemParticleLayer::ItemName() const
{
	QString ret; 

	if(NULL != layer)
	{
		ret = layer->layerName.c_str();
	}

	return ret;
}

QVariant SceneTreeItemParticleLayer::ItemData() const
{
	return qVariantFromValue(layer);
}

void SceneTreeItemParticleLayer::DoSync(QStandardItem *rootItem, DAVA::ParticleLayer *layer)
{
	if(NULL != rootItem && NULL != layer)
	{
		size_t itemsCount = layer->forces.size();
		QList<QStandardItem*> items;

		// add all items to the head
		for(size_t i = 0; i < itemsCount; ++i)
		{
			items.push_back(new SceneTreeItemParticleForce(layer, layer->forces[i]));
		}

		if(items.size() > 0)
		{
			rootItem->insertRows(0, items);
		}

		// remove old items from the tail
		rootItem->removeRows(itemsCount, rootItem->rowCount() - itemsCount);
	}
}

QIcon SceneTreeItemParticleLayer::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/layer_particle.png");
	return icon;
}

// =========================================================================================
// SceneTreeItemParticleForce
// =========================================================================================

SceneTreeItemParticleForce::SceneTreeItemParticleForce(DAVA::ParticleLayer *_parent, DAVA::ParticleForce *_force)
	: SceneTreeItem(SceneTreeItem::EIT_Force)
	, parent(_parent)
	, force(_force)
{ }

DAVA::ParticleForce* SceneTreeItemParticleForce::GetForce(SceneTreeItem *item)
{
	DAVA::ParticleForce *ret = NULL;

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Force)
	{
		SceneTreeItemParticleForce *itemForce = (SceneTreeItemParticleForce *) item;
		ret = itemForce->force;
	}

	return ret;
}

SceneTreeItemParticleForce::~SceneTreeItemParticleForce()
{ }

QString SceneTreeItemParticleForce::ItemName() const
{
	return "force";
}

QVariant SceneTreeItemParticleForce::ItemData() const
{
	return qVariantFromValue(force);
}

QIcon SceneTreeItemParticleForce::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/force.png");
	return icon;
}
