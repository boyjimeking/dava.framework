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

#include "DockLibrary/LibraryView.h"
#include "Project/ProjectManager.h"
#include <QFileSystemModel>
#include <QMenu>

#include "../Scene/SceneDataManager.h"
#include "../Scene/SceneData.h"
#include "../../LandscapeEditor/LandscapesController.h"

#include "../../SceneEditor/SceneEditorScreenMain.h"
#include "../../SceneEditor/EditorBodyControl.h"
#include "../../AppScreens.h"


LibraryView::LibraryView(QWidget *parent /* = 0 */)
	: QTreeView(parent)
{
	libModel = new LibraryModel();

	QObject::connect(libModel, SIGNAL(rootPathChanged(const QString &)), this, SLOT(ModelRootPathChanged(const QString &)));

	setModel(libModel);

	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed(const QString &)), this, SLOT(ProjectClosed(const QString &)));

	QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
	QObject::connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(FileSelectionChanged(const QItemSelection &, const QItemSelection &)));

	// hide columns with size/modif date etc.
	for(int32 i = 1; i < libModel->columnCount(); ++i)
	{
		setColumnHidden(i, true);
	}
}

LibraryView::~LibraryView()
{
	delete libModel;
}

void LibraryView::ProjectOpened(const QString &path)
{
	libModel->SetLibraryPath(ProjectManager::Instance()->CurProjectDataSourcePath());
}

void LibraryView::ProjectClosed(const QString &path)
{
	libModel->SetLibraryPath("");
}

void LibraryView::ShowContextMenu(const QPoint &point)
{
	QModelIndex index = indexAt(point);

	if(index.isValid())
	{
		QFileInfo fileInfo = libModel->fileInfo(index);
		QString fileExtension = fileInfo.suffix();
		if(fileInfo.isFile())
		{
			QMenu contextMenu(this);

			if(0 == fileExtension.compare("sc2", Qt::CaseInsensitive))
			{
				SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
				LandscapesController *landsacpesController = activeScene->GetLandscapesController();

				SceneEditorScreenMain *screen = static_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen(SCREEN_MAIN_OLD));
				EditorBodyControl *c = screen->FindCurrentBody()->bodyControl;

				bool canChangeScene = !landsacpesController->EditorLandscapeIsActive() && !c->LandscapeEditorActive();
				if(canChangeScene)
				{
					contextMenu.addAction(new ContextMenuAction(QString("Add Model"), new CommandAddScene(fileInfo.filePath().toStdString())));
				}

				contextMenu.addAction(new ContextMenuAction(QString("Edit Model"), new CommandEditScene(fileInfo.filePath().toStdString())));

				if(canChangeScene)
				{
					contextMenu.addAction(new ContextMenuAction(QString("Reload Model"), new CommandReloadScene(fileInfo.filePath().toStdString())));
				}

			}
			else if(0 == fileExtension.compare("dae", Qt::CaseInsensitive))
			{
				contextMenu.addAction(new ContextMenuAction(QString("Convert"), new CommandConvertScene(fileInfo.filePath().toStdString())));
			}

			ContextMenuAction* action = (ContextMenuAction *) contextMenu.exec(mapToGlobal(point));
			if(NULL != action)
			{
				action->Exec();
			}
		}
	}
}

void LibraryView::ModelRootPathChanged(const QString & newPath)
{
	setRootIndex(libModel->index(newPath));
}

void LibraryView::FileSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	DAVA::String previewPath;
	const QModelIndex index = selected.indexes().first();

	if(index.isValid())
	{
		QFileInfo fileInfo = libModel->fileInfo(index);
		if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
		{
			previewPath = fileInfo.filePath().toStdString();
		}
	}
    
    SceneDataManager::Instance()->SceneShowPreview(previewPath);
}

void LibraryView::LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files)
{
	libModel->SetFileNameFilters(showDAEFiles, showSC2Files);
}