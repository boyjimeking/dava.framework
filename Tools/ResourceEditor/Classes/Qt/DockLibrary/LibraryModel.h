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

#ifndef __LIBRARY_MODEL_H__
#define __LIBRARY_MODEL_H__

#include <QFileSystemModel>
#include <QString>
#include "DAVAEngine.h"

class QTreeView;
class FileSelectionModel;

struct ExtensionToColorMap
{
	QString extension;
	QColor color;
};

class LibraryModel : public QFileSystemModel
{
    Q_OBJECT
    
public:
    LibraryModel(QObject *parent = 0);
    virtual ~LibraryModel();

	void SetFileNameFilters(bool showDAEFiles, bool showSC2Files);

    void SetLibraryPath(const QString &path);
	// bool SelectFile(const QString &path);
    
    virtual QVariant data(const QModelIndex &index, int role) const;
	
protected:
	QVariant GetColorForExtension(const QString& extension, const ExtensionToColorMap* colorMap,
								const QModelIndex &index, int role) const;

private:
	static ExtensionToColorMap extensionToBackgroundColorMap[];
};

#endif // __GRAPH_MODEL_H__
