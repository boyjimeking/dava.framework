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

#ifndef __QT_SCENE_TREE_MODEL_H__
#define __QT_SCENE_TREE_MODEL_H__

#include <QPair>
#include <QStandardItemModel>

#include "Scene/SceneEditor2.h"
#include "Qt/DockSceneTree/SceneTreeItem.h"

// framework
#include "Scene3D/Scene.h"

class SceneTreeModel : public QStandardItemModel
{
	Q_OBJECT

public:
	SceneTreeModel(QObject* parent = 0);
	~SceneTreeModel();

	// virtual QVariant data(const QModelIndex &index, int role) const;

	void SetScene(SceneEditor2 *scene);
	SceneEditor2* GetScene() const;

	QModelIndex GetEntityIndex(DAVA::Entity *entity) const;
	DAVA::Entity* GetEntity(const QModelIndex &index) const;

	// this workaround for Qt bug
	// see https://bugreports.qt-project.org/browse/QTBUG-26229 
	// for more information
	bool DropIsAccepted();

	// drag and drop support
	Qt::DropActions supportedDropActions() const;
	QMimeData *	mimeData(const QModelIndexList & indexes) const;
	QStringList	mimeTypes() const;
	bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

protected:
	bool dropAccepted;
	SceneEditor2 * curScene;

protected slots:
	void EntityRemoved(SceneEditor2 *scene, DAVA::Entity *entity);
};

#endif // __QT_SCENE_TREE_MODEL_H__
