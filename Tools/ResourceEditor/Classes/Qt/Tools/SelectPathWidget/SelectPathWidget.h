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

#ifndef __RESOURCEEDITORQT__SELECTPATHWIDGET__
#define __RESOURCEEDITORQT__SELECTPATHWIDGET__

#include <QWidget>
#include <QMimeData>

#include "DAVAEngine.h"
class SceneEditor2;

namespace Ui
{
	class SelectPathWidget;
}

class SelectPathWidget: public QWidget
{
	Q_OBJECT

public:
	explicit SelectPathWidget( QWidget* parent = 0, DAVA::String openDialoDefualtPath = "", DAVA::String relativPath = "");
	
	~SelectPathWidget();
	
	void SetDiscriptionText(const DAVA::String &);

	DAVA::String GetDiscriptionText();

	void SetPathText(const DAVA::String &);

	DAVA::String GetPathText();

	void EraseWidget();
	
	DAVA::FilePath GetRelativPath()
	{
		return relativePath;
	}

	void SetRelativePath(const DAVA::String& );

	DAVA::Entity* GetOutputEntity(SceneEditor2*);

	DAVA::String GetOpenDialogDefaultPath()
	{
		return openDialogDefaultPath;
	}

	void SetOpenDialogDefaultPath(const DAVA::String& newPath)
	{
		openDialogDefaultPath = newPath;
	}
	
	

protected:

	void dragEnterEvent(QDragEnterEvent* event);

	void dropEvent(QDropEvent * event);
	
	void Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath);
	
	DAVA::String ConvertToRelativPath(const DAVA::String& path);
	
	Ui::SelectPathWidget*	ui;
	
	DAVA::FilePath			relativePath;
	
	DAVA::String			openDialogDefaultPath;
	
	QMimeData				mimeData;
	

protected slots:

	void EraseClicked();

	void OpenClicked();

	
};

#endif /* defined(__RESOURCEEDITORQT__SELECTPATHWIDGET__) */