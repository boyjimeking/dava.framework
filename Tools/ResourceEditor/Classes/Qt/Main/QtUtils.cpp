#include "QtUtils.h"
#include "../../SceneEditor/SceneValidator.h"
#include "CommandLine/EditorCommandLineParser.h"

#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "QtMainWindowHandler.h"

#include "../../Commands/FileCommands.h"
#include "../../Commands/CommandsManager.h"

#include "../../SceneEditor/SceneEditorScreenMain.h"

#include "DAVAEngine.h"
using namespace DAVA;


DAVA::FilePath PathnameToDAVAStyle(const QString &convertedPathname)
{
    return FilePath((const String &)QSTRING_TO_DAVASTRING(convertedPathname));
}


DAVA::FilePath GetOpenFileName(const DAVA::String &title, const DAVA::FilePath &pathname, const DAVA::String &filter)
{
    QString filePath = QFileDialog::getOpenFileName(NULL, QString(title.c_str()), QString(pathname.GetAbsolutePathname().c_str()),
                                                    QString(filter.c_str()));
    
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();

    FilePath openedPathname = PathnameToDAVAStyle(filePath);
    if(!openedPathname.IsEmpty() && !SceneValidator::Instance()->IsPathCorrectForProject(openedPathname))
    {
        //Need to Show Error
		ShowErrorDialog(String(Format("File(%s) was selected from incorect project.", openedPathname.GetAbsolutePathname().c_str())));
        openedPathname = FilePath();
    }
    
    return openedPathname;
}


DAVA::String SizeInBytesToString(DAVA::float32 size)
{
    DAVA::String retString = "";
    
    if(1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024) );
    }
    else if(1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else
    {
        retString = Format("%d B", (int32)size);
    }
    
    return  retString;
}

DAVA::WideString SizeInBytesToWideString(DAVA::float32 size)
{
    return StringToWString(SizeInBytesToString(size));
}


DAVA::Image * CreateTopLevelImage(const DAVA::FilePath &imagePathname)
{
    Image *image = NULL;
    Vector<Image *> imageSet = ImageLoader::CreateFromFile(imagePathname);
    if(0 != imageSet.size())
    {
        image = SafeRetain(imageSet[0]);
		for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
    }
    
    return image;
}

void ShowErrorDialog(const DAVA::Set<DAVA::String> &errors)
{
    if(errors.empty())
        return;
    
    String errorMessage = String("");
    Set<String>::const_iterator endIt = errors.end();
    for(Set<String>::const_iterator it = errors.begin(); it != endIt; ++it)
    {
        errorMessage += *it + String("\n");
    }
    
    ShowErrorDialog(errorMessage);
}

void ShowErrorDialog(const DAVA::String &errorMessage)
{
	bool forceClose =    EditorCommandLineParser::CommandIsFound(String("-force"))
					||  EditorCommandLineParser::CommandIsFound(String("-forceclose"));
	if(!forceClose)
	{
		QMessageBox::critical(QtMainWindow::Instance(), "Error", errorMessage.c_str());
	}
}

bool IsKeyModificatorPressed(int32 key)
{
	return InputSystem::Instance()->GetKeyboard()->IsKeyPressed(key);
}

bool IsKeyModificatorsPressed()
{
	return (IsKeyModificatorPressed(DVKEY_SHIFT) || IsKeyModificatorPressed(DVKEY_CTRL) || IsKeyModificatorPressed(DVKEY_ALT));
}

QColor ColorToQColor(const DAVA::Color& color)
{
	return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

DAVA::Color QColorToColor(const QColor &qcolor)
{
	return Color(qcolor.redF(), qcolor.greenF(), qcolor.blueF(), qcolor.alphaF());
}

int ShowQuestion(const DAVA::String &header, const DAVA::String &question, int buttons, int defaultButton)
{
    int answer = QMessageBox::question(NULL, QString::fromStdString(header), QString::fromStdString(question),
                                       (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
    return answer;
}

int ShowSaveSceneQuestion(DAVA::Scene *scene)
{
    int answer = MB_FLAG_NO;
    
    int32 changesCount = CommandsManager::Instance()->GetUndoQueueLength(scene);
    if(changesCount)
    {
        answer = ShowQuestion("Scene was changed", "Do you want to save changes in the current scene prior to creating new one?",
                                    MB_FLAG_YES | MB_FLAG_NO | MB_FLAG_CANCEL, MB_FLAG_CANCEL);
    }
    
    return answer;
}

