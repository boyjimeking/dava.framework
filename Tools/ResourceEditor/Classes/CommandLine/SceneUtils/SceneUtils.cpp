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

#include "SceneUtils.h"

using namespace DAVA;

SceneUtils::SceneUtils()
{
}

SceneUtils::~SceneUtils()
{
}

void SceneUtils::CleanFolder(const FilePath &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str())));
        }
    }
}

void SceneUtils::SetInFolder(const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataSourceFolder = folderPathname;
}

void SceneUtils::SetOutFolder(const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataFolder = folderPathname;
}


bool SceneUtils::CopyFile(const FilePath &filePathname, Set<String> &errorLog)
{
    String workingPathname = RemoveFolderFromPath(filePathname, dataSourceFolder);
    PrepareFolderForCopyFile(workingPathname, errorLog);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s",
                                      workingPathname.c_str(),
                                      dataSourceFolder.GetAbsolutePathname().c_str(),
                                      dataFolder.GetAbsolutePathname().c_str())));
    }
    
    return retCopy;
}

void SceneUtils::PrepareFolderForCopyFile(const String &filename, Set<String> &errorLog)
{
    FilePath newFolderPath = (dataFolder + filename).GetDirectory();
    
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s", newFolderPath.GetAbsolutePathname().c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + filename);
}

String SceneUtils::RemoveFolderFromPath(const FilePath &pathname, const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());

    String workingPathname = pathname.GetAbsolutePathname();
    String::size_type pos = workingPathname.find(folderPathname.GetAbsolutePathname());
    if(String::npos != pos)
    {
        workingPathname = workingPathname.replace(pos, folderPathname.GetAbsolutePathname().length(), "");
    }

    return workingPathname;
}

