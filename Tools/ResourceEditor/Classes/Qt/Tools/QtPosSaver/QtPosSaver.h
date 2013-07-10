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

#ifndef __QT_POS_SAVER_H__
#define __QT_POS_SAVER_H__

#include <QMainWindow>
#include <QDialog>
#include <QSplitter>
#include <QMainWindow>
#include <QTreeView>
#include "DAVAEngine.h"

class QtPosSaver
{
public:
	QtPosSaver();
	virtual ~QtPosSaver();

	void Attach(QWidget *widget, const QString &name = QString());

	void SaveGeometry(QWidget *widget);
	void LoadGeometry(QWidget *widget);

	void SaveState(QSplitter *splitter);
	void LoadState(QSplitter *splitter);

	void SaveState(QMainWindow *mainwindow);
	void LoadState(QMainWindow *mainwindow);

	void SaveValue(const QString &key, const DAVA::VariantType &value);
	DAVA::VariantType LoadValue(const QString &key);

protected:
	QWidget *attachedWidget;
	QString attachedWidgetName;

	void Save(const QString &key, const QByteArray &data);
	QByteArray Load(const QString &key);

private:
	static bool settingsArchiveIsLoaded;
	static DAVA::KeyedArchive settingsArchive;
};

#endif // __QT_POS_SAVER_H__
