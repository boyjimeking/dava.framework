#include "QtMainWindowHandler.h"

#include "../Commands/CommandCreateNode.h"
#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
#include "../Commands/ToolsCommands.h"
#include "../Commands/SceneGraphCommands.h"
#include "../Commands/CommandReloadTextures.h"
#include "../Commands/ParticleEditorCommands.h"
#include "../Commands/TextureOptionsCommands.h"
#include "../Commands/CustomColorCommands.h"
#include "../Commands/VisibilityCheckToolCommands.h"
#include "../Commands/TilemapEditorCommands.h"
#include "../Commands/HeightmapEditorCommands.h"
#include "../Commands/SetSwitchIndexCommands.h"
#include "../Commands/MaterialViewOptionsCommands.h"
#include "../Constants.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../DockHangingObjects/HangingObjectsHelper.h"
#include "Scene/SceneDataManager.h"
#include "Scene/SceneData.h"
#include "Main/QtUtils.h"
#include "Main/mainwindow.h"
#include "TextureBrowser/TextureBrowser.h"
#include "Project/ProjectManager.h"
#include "ModificationWidget.h"
#include "../Commands/CommandSignals.h"
#include "SceneEditor/EntityOwnerPropertyHelper.h"

#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QStatusBar>
#include <QSpinBox.h>
#include <QFileDialog>

#include "Render/LibDxtHelper.h"

using namespace DAVA;

QtMainWindowHandler::QtMainWindowHandler(QObject *parent)
    :   QObject(parent)
	,	menuResentScenes(NULL)
	,	defaultFocusWidget(NULL)
    ,   statusBar(NULL)
{
    new CommandsManager();
    
    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);
    ClearActions(FILE_FORMAT_COUNT, textureFileFormatActions);
	ClearActions(ResourceEditor::MODIFY_COUNT, modificationActions);

    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(Format("resentSceneActions[%d]", i)));
    }

	SceneDataManager* sceneDataManager = SceneDataManager::Instance();

	connect(sceneDataManager, SIGNAL(SceneActivated(SceneData*)), this, SLOT(OnSceneActivated(SceneData*)));
	connect(sceneDataManager, SIGNAL(SceneReleased(SceneData*)), this, SLOT(OnSceneReleased(SceneData*)));
	connect(sceneDataManager, SIGNAL(SceneCreated(SceneData*)), this, SLOT(OnSceneCreated(SceneData*)));
	connect(QtMainWindow::Instance(), SIGNAL(RepackAndReloadFinished()), this, SLOT(ReloadSceneTextures()));
	connect(CommandSignals::Instance(), SIGNAL(CommandAffectsEntities(DAVA::Scene* , CommandList::eCommandId , const DAVA::Set<DAVA::Entity*>& ) ) ,
			this,SLOT( OnEntityModified(DAVA::Scene* , CommandList::eCommandId , const DAVA::Set<DAVA::Entity*>& ) ));
}

QtMainWindowHandler::~QtMainWindowHandler()
{
	defaultFocusWidget = NULL;
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        SafeDelete(resentSceneActions[i]);
    }

    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);
    ClearActions(FILE_FORMAT_COUNT, textureFileFormatActions);
	ClearActions(ResourceEditor::MODIFY_COUNT, modificationActions);
	ClearActions(ResourceEditor::EDIT_COUNT, editActions);

    CommandsManager::Instance()->Release();
}

void QtMainWindowHandler::ClearActions(int32 count, QAction **actions)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = NULL;
    }
}

void QtMainWindowHandler::NewScene()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        SceneData *levelScene = SceneDataManager::Instance()->SceneGetLevel();
        int32 answer = ShowSaveSceneQuestion(levelScene->GetScene());
        if(answer == MB_FLAG_CANCEL)
        {
            return;
        }
        
        if(answer == MB_FLAG_YES)
        {
            bool saved = SaveScene(levelScene->GetScene());
            if(!saved)
            {
                return;
            }
        }
        
		// Can now create the scene.
		screen->NewScene();
        
        SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
        SceneDataManager::Instance()->SetActiveScene(activeScene->GetScene());
    }
}


void QtMainWindowHandler::OpenScene()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandOpenScene());
}


void QtMainWindowHandler::OpenProject()
{
	/*
    CommandsManager::Instance()->ExecuteAndRelease(new CommandOpenProject());
	emit ProjectChanged();
	*/

	QString newPath = ProjectManager::Instance()->ProjectOpenDialog();
	ProjectManager::Instance()->ProjectOpen(newPath);
}

void QtMainWindowHandler::OpenResentScene(int32 index)
{
	DAVA::String path = EditorSettings::Instance()->GetLastOpenedFile(index);
    CommandsManager::Instance()->ExecuteAndRelease(new CommandOpenScene(path));
}

bool QtMainWindowHandler::SaveScene()
{
    return SaveScene(SceneDataManager::Instance()->SceneGetActive()->GetScene());
}

bool QtMainWindowHandler::SaveScene(Scene *scene)
{
    bool sceneWasSaved = false;
    
    SceneData *activeScene = SceneDataManager::Instance()->SceneGet(scene);
    if(activeScene->CanSaveScene())
    {
		FilePath currentPath = activeScene->GetScenePathname();
        if(currentPath.IsEmpty())
        {
			currentPath = EditorSettings::Instance()->GetDataSourcePath();
        }
        
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Scene File"), QString(currentPath.GetAbsolutePathname().c_str()),
                                                        QString("Scene File (*.sc2)")
                                                        );
        if(0 < filePath.size())
        {
			FilePath normalizedPathname = PathnameToDAVAStyle(filePath);
			sceneWasSaved = SaveScene(scene, normalizedPathname);

			EditorSettings::Instance()->AddLastOpenedFile(normalizedPathname);
			UpdateRecentScenesList();
        }
    }
    
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();
    return sceneWasSaved;
}

bool QtMainWindowHandler::SaveScene(Scene *scene, const FilePath &pathname)
{
	SaveParticleEmitterNodes(scene);

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	screen->SaveSceneToFile(pathname);

	return true;
}


void QtMainWindowHandler::SaveParticleEmitterNodes(Scene* scene)
{
	if (!scene) return;
    
	int32 childrenCount = scene->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SaveParticleEmitterNodeRecursive(scene->GetChild(i));
	}
}

void QtMainWindowHandler::SaveParticleEmitterNodeRecursive(Entity* parentNode)
{
	bool needSaveThisLevelNode = true;
	ParticleEmitter * emitter = GetEmitter(parentNode);
	if (!emitter)
	{
		needSaveThisLevelNode = false;
	}
    
	if (needSaveThisLevelNode)
	{
		// Do we have file name? Ask for it, if not.
		FilePath yamlPath = emitter->GetConfigPath();
		if (yamlPath.IsEmpty())
		{
			QString saveDialogCaption = QString("Save Particle Emitter \"%1\"").arg(QString::fromStdString(parentNode->GetName()));
			QString saveDialogYamlPath = QFileDialog::getSaveFileName(NULL, saveDialogCaption, "", QString("Yaml File (*.yaml)"));
            
			if (!saveDialogYamlPath.isEmpty())
			{
				yamlPath = PathnameToDAVAStyle(saveDialogYamlPath);
			}
		}
        
		if (!yamlPath.IsEmpty())
		{
			emitter->SaveToYaml(yamlPath);
		}
	}
    
	// Repeat for all children.
	int32 childrenCount = parentNode->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		SaveParticleEmitterNodeRecursive(parentNode->GetChild(i));
	}
}



void QtMainWindowHandler::ExportAsPNG()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandExport(PNG_FILE));
}

void QtMainWindowHandler::ExportAsPVR()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandExport(PVR_FILE));
}

void QtMainWindowHandler::ExportAsDXT()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandExport(DXT_FILE));
}

void QtMainWindowHandler::SaveToFolderWithChilds()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandSaveToFolderWithChilds());
}

void QtMainWindowHandler::CreateNode(ResourceEditor::eNodeType type)
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandCreateNode(type));
}

void QtMainWindowHandler::Materials()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->MaterialsTriggered();
	}

	/*
	MaterialBrowser *materialBrowser = new MaterialBrowser((QWidget *) parent());
	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(NULL != screen)
	{
		SceneEditorScreenMain::BodyItem *body = screen->FindCurrentBody();

		if(NULL != body && NULL != body->bodyControl)
		{
			DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();
			materialBrowser->SetScene(mainScreenScene);
		}
	}

	materialBrowser->show();
	*/
}

void QtMainWindowHandler::HeightmapEditor()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->HeightmapTriggered();
	}
}

void QtMainWindowHandler::TilemapEditor()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->TilemapTriggered();
	}
}

void QtMainWindowHandler::ConvertTextures()
{
	TextureBrowser *textureBrowser = new TextureBrowser((QWidget *) parent());
	SceneData *activeScene =  SceneDataManager::Instance()->SceneGetActive();
	
	textureBrowser->sceneActivated(activeScene);
	textureBrowser->sceneNodeSelected(activeScene, SceneDataManager::Instance()->SceneGetSelectedNode(activeScene));
	textureBrowser->show();
}

void QtMainWindowHandler::SetViewport(ResourceEditor::eViewportType type)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->SetViewport(type);
	}
}


void QtMainWindowHandler::CreateNodeTriggered(QAction *nodeAction)
{
    for(int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        if(nodeAction == nodeActions[i])
        {
            CreateNode((ResourceEditor::eNodeType)i);
            break;
        }
    }
}

void QtMainWindowHandler::ViewportTriggered(QAction *viewportAction)
{
    for(int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        if(viewportAction == viewportActions[i])
        {
            SetViewport((ResourceEditor::eViewportType)i);
            break;
        }
    }
}

void QtMainWindowHandler::SetResentMenu(QMenu *menu)
{
    menuResentScenes = menu;
}

void QtMainWindowHandler::UpdateRecentScenesList()
{
    //TODO: what a bug?
    DVASSERT(menuResentScenes && "Call SetResentMenu() to setup resent menu");

    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentSceneActions[i]->parentWidget())
        {
            menuResentScenes->removeAction(resentSceneActions[i]);
        }
    }

    
    int32 sceneCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(0 < sceneCount)
    {
        QList<QAction *> resentActions;
        for(int32 i = 0; i < sceneCount; ++i)
        {
            resentSceneActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            resentActions.push_back(resentSceneActions[i]);
        }
        
		menuResentScenes->addSeparator();
        menuResentScenes->addActions(resentActions);
    }
}

void QtMainWindowHandler::FileMenuTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentScene == resentSceneActions[i])
        {
            OpenResentScene(i);
        }
    }
}


void QtMainWindowHandler::RegisterNodeActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::NODE_COUNT == count) && "Wrong count of actions");

    va_list vl;
    va_start(vl, count);
    
    RegisterActions(nodeActions, count, vl);    
    
    va_end(vl);
}


void QtMainWindowHandler::RegisterViewportActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::VIEWPORT_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(viewportActions, count, vl);    
  
    va_end(vl);
}

void QtMainWindowHandler::RegisterDockActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::HIDABLEWIDGET_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(hidablewidgetActions, count, vl);
    
    va_end(vl);
}

void QtMainWindowHandler::RegisterTextureFormatActions(DAVA::int32 count, ...)
{
    DVASSERT((FILE_FORMAT_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(textureFileFormatActions, count, vl);
    
    va_end(vl);
}

void QtMainWindowHandler::RegisterModificationActions(DAVA::int32 count, ...)
{
	DVASSERT((count == ResourceEditor::MODIFY_COUNT) && "Wrong count of actions");
	va_list vl;
	va_start(vl, count);

	RegisterActions(modificationActions, count, vl);

	va_end(vl);
}

void QtMainWindowHandler::RegisterEditActions(DAVA::int32 count, ...)
{
	DVASSERT((count == ResourceEditor::EDIT_COUNT) && "Wrong count of actions");
	va_list vl;
	va_start(vl, count);

	RegisterActions(editActions, count, vl);

	va_end(vl);
}



void QtMainWindowHandler::RegisterActions(QAction **actions, int32 count, va_list &vl)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = va_arg(vl, QAction *);
    }
}


void QtMainWindowHandler::RestoreViews()
{
    for(int32 i = 0; i < ResourceEditor::HIDABLEWIDGET_COUNT; ++i)
    {
        if(hidablewidgetActions[i] && !hidablewidgetActions[i]->isChecked())
        {
            hidablewidgetActions[i]->trigger();
        }
    }
}

void QtMainWindowHandler::RefreshSceneGraph()
{
	SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();
}

void QtMainWindowHandler::ShowSettings()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->ShowSettings();
	}
}


void QtMainWindowHandler::Beast()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandBeast());
}

void QtMainWindowHandler::SetDefaultFocusWidget(QWidget *widget)
{
	defaultFocusWidget = widget;
}

void QtMainWindowHandler::RestoreDefaultFocus()
{
	if(defaultFocusWidget)
	{
		defaultFocusWidget->setEnabled(false);
		defaultFocusWidget->setEnabled(true);
        defaultFocusWidget->setFocus();
	}
}


void QtMainWindowHandler::RepackAndReloadTextures()
{
	QtMainWindow::Instance()->RepackAndReloadScene();
}

void QtMainWindowHandler::CreateParticleEmitterNode()
{
    CreateNode(ResourceEditor::NODE_PARTICLE_EMITTER);
}

void QtMainWindowHandler::ToggleNotPassableTerrain()
{
	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	if (activeScene)
	{
		activeScene->ToggleNotPassableLandscape();
	}
}

void QtMainWindowHandler::RegisterStatusBar(QStatusBar *registeredSatusBar)
{
    statusBar = registeredSatusBar;
}

void QtMainWindowHandler::ShowStatusBarMessage(const DAVA::String &message, DAVA::int32 displayTime)
{
    if(statusBar)
    {
        statusBar->showMessage(QString(message.c_str()), displayTime);
    }
}


void QtMainWindowHandler::SetWaitingCursorEnabled(bool enabled)
{
    QMainWindow *window = dynamic_cast<QMainWindow *>(parent());
    
    if(window)
    {
        if(enabled)
        {
            window->setCursor(QCursor(Qt::WaitCursor));
        }
        else
        {
            window->setCursor(QCursor(Qt::ArrowCursor));
        }
    }
}

void QtMainWindowHandler::MenuViewOptionsWillShow()
{
    uint8 textureFileFormat = (uint8)EditorSettings::Instance()->GetTextureViewFileFormat();
    
    for(int32 i = 0; i < FILE_FORMAT_COUNT; ++i)
    {
        textureFileFormatActions[i]->setCheckable(true);
        textureFileFormatActions[i]->setChecked(i == textureFileFormat);
    }
}

void QtMainWindowHandler::RulerTool()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->RulerToolTriggered();

		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		if (activeScene)
		{
			ShowStatusBarMessage(activeScene->GetScenePathname().GetAbsolutePathname());
		}
	}
}

void QtMainWindowHandler::ReloadAsPNG()
{
    CommandsManager::Instance()->ExecuteAndRelease(new ReloadTexturesAsCommand(PNG_FILE));
	MenuViewOptionsWillShow();
}

void QtMainWindowHandler::ReloadAsPVR()
{
    CommandsManager::Instance()->ExecuteAndRelease(new ReloadTexturesAsCommand(PVR_FILE));
	MenuViewOptionsWillShow();
}

void QtMainWindowHandler::ReloadAsDXT()
{
    CommandsManager::Instance()->ExecuteAndRelease(new ReloadTexturesAsCommand(DXT_FILE));
	MenuViewOptionsWillShow();
}

void QtMainWindowHandler::ReloadSceneTextures()
{
	CommandsManager::Instance()->ExecuteAndRelease(new CommandReloadTextures());
}

void QtMainWindowHandler::OnEntityModified(DAVA::Scene* scene, CommandList::eCommandId id, const DAVA::Set<DAVA::Entity*>& affectedEntities)
{
	for(DAVA::Set<DAVA::Entity*>::iterator it = affectedEntities.begin(); it != affectedEntities.end(); ++it)
	{
		EntityOwnerPropertyHelper::Instance()->UpdateEntityOwner((*it)->GetCustomProperties());
	}
}

void QtMainWindowHandler::ToggleSetSwitchIndex(DAVA::uint32  value, SetSwitchIndexHelper::eSET_SWITCH_INDEX state)
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandToggleSetSwitchIndex(value,state));
}

void QtMainWindowHandler::MaterialViewOptionChanged(int index)
{
	CommandsManager::Instance()->ExecuteAndRelease(new CommandChangeMaterialViewOption((Material::eViewOptions)index));
}

void QtMainWindowHandler::ToggleHangingObjects(float value, bool isEnabled)
{
	HangingObjectsHelper::ProcessHangingObjectsUpdate(value, isEnabled);
}

void QtMainWindowHandler::ToggleCustomColors()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->CustomColorsTriggered();
	}
}

void QtMainWindowHandler::SaveTextureCustomColors()
{
    CommandsManager::Instance()->ExecuteAndRelease(new CommandSaveTextureCustomColors());
}

void QtMainWindowHandler::LoadTextureCustomColors()
{
	CommandsManager::Instance()->ExecuteAndRelease(new CommandLoadTextureCustomColors());
}

void QtMainWindowHandler::ChangeBrushSizeCustomColors(int newSize)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->CustomColorsSetRadius(newSize);
	}
}

void QtMainWindowHandler::ChangeColorCustomColors(int newColorIndex)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->CustomColorsSetColor(newColorIndex);
	}
}

void QtMainWindowHandler::RegisterCustomColorsWidgets(QPushButton* toggleButton, QPushButton* saveTextureButton, QSlider* brushSizeSlider, QComboBox* colorComboBox, QPushButton* loadTextureButton)
{
	this->customColorsToggleButton = toggleButton;
	this->customColorsSaveTextureButton = saveTextureButton;
	this->customColorsBrushSizeSlider = brushSizeSlider;
	this->customColorsColorComboBox = colorComboBox;
	this->customColorsLoadTextureButton = loadTextureButton;
}

void QtMainWindowHandler::SetCustomColorsWidgetsState(bool state)
{
	DVASSERT(customColorsToggleButton &&
			 customColorsSaveTextureButton &&
			 customColorsBrushSizeSlider &&
			 customColorsColorComboBox &&
			 customColorsLoadTextureButton);

	customColorsToggleButton->blockSignals(true);
	customColorsToggleButton->setCheckable(state);
	customColorsToggleButton->setChecked(state);
	customColorsToggleButton->blockSignals(false);

	QString buttonText = state ? tr("Disable Custom Colors") : tr("Enable Custom Colors");
	customColorsToggleButton->setText(buttonText);

	customColorsSaveTextureButton->setEnabled(state);
	customColorsBrushSizeSlider->setEnabled(state);
	customColorsColorComboBox->setEnabled(state);
	customColorsLoadTextureButton->setEnabled(state);
	customColorsSaveTextureButton->blockSignals(!state);
	customColorsBrushSizeSlider->blockSignals(!state);
	customColorsColorComboBox->blockSignals(!state);
	customColorsLoadTextureButton->blockSignals(!state);

	if(state == true)
	{
		ChangeBrushSizeCustomColors(customColorsBrushSizeSlider->value());
		ChangeColorCustomColors(customColorsColorComboBox->currentIndex());
	}
}

void QtMainWindowHandler::RegisterMaterialViewOptionsWidgets(QComboBox* combo)
{
	this->comboMaterialViewOption = combo;
}

void QtMainWindowHandler::SetMaterialViewOptionsWidgetsState(bool state)
{
	comboMaterialViewOption->blockSignals(true);
	comboMaterialViewOption->setEnabled(state);
	comboMaterialViewOption->blockSignals(false);
}


void QtMainWindowHandler::SelectMaterialViewOption(Material::eViewOptions value)
{
	comboMaterialViewOption->setCurrentIndex((int)value);
}

void QtMainWindowHandler::RegisterSetSwitchIndexWidgets(QSpinBox* spinBox, QRadioButton* rBtnSelection, QRadioButton* rBtnScene, QPushButton* btnOK)
{
	this->setSwitchIndexToggleButton = btnOK;
	this->editSwitchIndexValue = spinBox;
	this->rBtnSelection = rBtnSelection;
	this->rBtnScene = rBtnScene;
}

void QtMainWindowHandler::SetSwitchIndexWidgetsState(bool state)
{
	DVASSERT(setSwitchIndexToggleButton &&
		 editSwitchIndexValue &&
		 rBtnSelection &&
		 rBtnScene );

	setSwitchIndexToggleButton->blockSignals(true);
	setSwitchIndexToggleButton->setEnabled(state);
	setSwitchIndexToggleButton->blockSignals(false);

	editSwitchIndexValue->setEnabled(state);
	rBtnSelection->setEnabled(state);
	rBtnScene->setEnabled(state);
}

void QtMainWindowHandler::RegisterHangingObjectsWidgets(QCheckBox* chBox, QDoubleSpinBox* dSpinBox, QPushButton* btnUpdate)
{
	this->hangingObjectsToggleButton= btnUpdate;
	this->editHangingObjectsValue	= dSpinBox;
	this->checkBoxHangingObjects	= chBox;
}

void QtMainWindowHandler::SetHangingObjectsWidgetsState(bool state)
{
		DVASSERT(hangingObjectsToggleButton &&
		 editHangingObjectsValue &&
		 checkBoxHangingObjects );

	hangingObjectsToggleButton->blockSignals(true);
	hangingObjectsToggleButton->setEnabled(state);
	hangingObjectsToggleButton->blockSignals(false);

	editHangingObjectsValue->setEnabled(state);
	checkBoxHangingObjects->setEnabled(state);
}

void QtMainWindowHandler::ToggleVisibilityTool()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->VisibilityToolTriggered();
	}
}

void QtMainWindowHandler::SaveTextureVisibilityTool()
{
	CommandsManager::Instance()->ExecuteAndRelease(new CommandSaveTextureVisibilityTool());
}

void QtMainWindowHandler::ChangleAreaSizeVisibilityTool(int newSize)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->VisibilityToolSetAreaSize(newSize);
	}
}

void QtMainWindowHandler::SetVisibilityPointVisibilityTool()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->VisibilityToolSetPoint();
	}
}

void QtMainWindowHandler::SetVisibilityAreaVisibilityTool()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->VisibilityToolSetArea();
	}
}

void QtMainWindowHandler::RegisterWidgetsVisibilityTool(QPushButton* toggleButton, QPushButton* saveTextureButton, QPushButton* setPointButton, QPushButton* setAreaButton, QSlider* areaSizeSlider)
{
	visibilityToolToggleButton = toggleButton;
	visibilityToolSaveTextureButton = saveTextureButton;
	visibilityToolSetPointButton = setPointButton;
	visibilityToolSetAreaButton = setAreaButton;
	visibilityToolAreaSizeSlider = areaSizeSlider;
}

void QtMainWindowHandler::SetWidgetsStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolToggleButton &&
			 visibilityToolSetPointButton &&
			 visibilityToolSetAreaButton &&
			 visibilityToolSaveTextureButton &&
			 visibilityToolAreaSizeSlider);
	
	visibilityToolToggleButton->blockSignals(true);
	visibilityToolToggleButton->setCheckable(state);
	visibilityToolToggleButton->setChecked(state);
	visibilityToolToggleButton->blockSignals(false);
	
	QString toggleButtonText = state ? tr("Disable Visibility Check Tool"): tr("Enable Visibility Check Tool");
	visibilityToolToggleButton->setText(toggleButtonText);

	visibilityToolSaveTextureButton->setEnabled(state);
	visibilityToolSetPointButton->setEnabled(state);
	visibilityToolSetAreaButton->setEnabled(state);
	visibilityToolAreaSizeSlider->setEnabled(state);
	visibilityToolSaveTextureButton->blockSignals(!state);
	visibilityToolSetPointButton->blockSignals(!state);
	visibilityToolSetAreaButton->blockSignals(!state);
	visibilityToolAreaSizeSlider->blockSignals(!state);
	
	if(state == true)
	{
		ChangleAreaSizeVisibilityTool(visibilityToolAreaSizeSlider->value());
	}
}

void QtMainWindowHandler::SetPointButtonStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolSetPointButton);

	bool b = visibilityToolSetPointButton->signalsBlocked();
	visibilityToolSetPointButton->blockSignals(true);
	visibilityToolSetPointButton->setChecked(state);
	visibilityToolSetPointButton->blockSignals(b);
}

void QtMainWindowHandler::SetAreaButtonStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolSetAreaButton);

	bool b = visibilityToolSetAreaButton->signalsBlocked();
	visibilityToolSetAreaButton->blockSignals(true);
	visibilityToolSetAreaButton->setChecked(state);
	visibilityToolSetAreaButton->blockSignals(b);
}

void QtMainWindowHandler::ModificationSelect()
{
	SetModificationMode(ResourceEditor::MODIFY_NONE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationMove()
{
	SetModificationMode(ResourceEditor::MODIFY_MOVE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationRotate()
{
	SetModificationMode(ResourceEditor::MODIFY_ROTATE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationScale()
{
	SetModificationMode(ResourceEditor::MODIFY_SCALE);
	UpdateModificationActions();
}

void QtMainWindowHandler::SetModificationMode(ResourceEditor::eModificationActions mode)
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
		if (bodyControl)
		{
			ResourceEditor::eModificationActions curModificationMode = bodyControl->GetModificationMode();

			if (mode != curModificationMode)
				bodyControl->SetModificationMode(mode);
		}
	}
}

void QtMainWindowHandler::ModificationPlaceOnLand()
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->FindCurrentBody()->bodyControl->OnPlaceOnLandscape();
	}
}

void QtMainWindowHandler::ModificationSnapToLand()
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;

		if (bodyControl)
		{
			bool curSnapToLand = bodyControl->IsLandscapeRelative();
			bodyControl->SetLandscapeRelative(!curSnapToLand);
		}
	}
}

void QtMainWindowHandler::UpdateModificationActions()
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (!screen || !screen->FindCurrentBody())
		return;

	EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
	ResourceEditor::eModificationActions modificationMode = bodyControl->GetModificationMode();

	modificationActions[ResourceEditor::MODIFY_NONE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_MOVE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_ROTATE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_SCALE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_NONE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_MOVE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_ROTATE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_SCALE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setCheckable(true);

	switch (modificationMode)
	{
		case ResourceEditor::MODIFY_MOVE:
		case ResourceEditor::MODIFY_ROTATE:
		case ResourceEditor::MODIFY_SCALE:
			modificationActions[modificationMode]->setChecked(true);
			modificationActions[ResourceEditor::MODIFY_PLACE_ON_LAND]->setEnabled(true);
			modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setEnabled(true);

			if (bodyControl->IsLandscapeRelative())
			{
				modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setChecked(true);
			}
			break;

		case ResourceEditor::MODIFY_NONE:
			modificationActions[modificationMode]->setChecked(true);

		default:
			modificationActions[ResourceEditor::MODIFY_PLACE_ON_LAND]->setEnabled(false);
			modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setEnabled(false);
			break;
	}
}

void QtMainWindowHandler::OnApplyModification(double x, double y, double z)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->FindCurrentBody()->bodyControl->ApplyTransform(x, y, z);
	}
}

void QtMainWindowHandler::OnResetModification()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if (screen)
	{
		screen->FindCurrentBody()->bodyControl->RestoreOriginalTransform();
	}
}

void QtMainWindowHandler::UndoAction()
{
	CommandsManager::Instance()->Undo();
	UpdateUndoActionsState();
}

void QtMainWindowHandler::RedoAction()
{
	CommandsManager::Instance()->Redo();
	UpdateUndoActionsState();
}

void QtMainWindowHandler::UpdateUndoActionsState()
{
	CommandsManager* commandsManager = CommandsManager::Instance();

	bool isEnabled;
	String commandName;

	isEnabled = false;
	commandName = "";
	if (commandsManager->GetUndoQueueLength())
	{
		isEnabled = true;
		commandName = commandsManager->GetUndoCommandName();
	}
	QString str = tr("Undo");
	if (isEnabled)
		str += " " + tr(commandName.c_str());
	editActions[ResourceEditor::EDIT_UNDO]->setText(str);
	editActions[ResourceEditor::EDIT_UNDO]->setEnabled(isEnabled);

	isEnabled = false;
	commandName = "";
	if (commandsManager->GetRedoQueueLength())
	{
		isEnabled = true;
		commandName = commandsManager->GetRedoCommandName();
	}
	str = tr("Redo");
	if (isEnabled)
		str += " " + tr(commandName.c_str());
	editActions[ResourceEditor::EDIT_REDO]->setText(str);
	editActions[ResourceEditor::EDIT_REDO]->setEnabled(isEnabled);
}

void QtMainWindowHandler::OnSceneActivated(SceneData *scene)
{
	UpdateUndoActionsState();
	UpdateModificationActions();
}

void QtMainWindowHandler::OnSceneCreated(SceneData *scene)
{
	UpdateRecentScenesList();
}

void QtMainWindowHandler::OnSceneReleased(SceneData *scene)
{
	CommandsManager::Instance()->SceneReleased(scene->GetScene());

	UpdateRecentScenesList();
}


void QtMainWindowHandler::ConvertToShadow()
{
	CommandsManager::Instance()->ExecuteAndRelease(new CommandConvertToShadow());
}
