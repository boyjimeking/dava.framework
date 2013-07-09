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

#ifndef __FILE_COMMANDS_H__
#define __FILE_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"
#include "EditorScene.h"

class CommandOpenScene: public Command
{
public:	
	DAVA_DEPRECATED(CommandOpenScene(const DAVA::FilePath &scenePathname = DAVA::FilePath())); // DEPRECATED: using QFileDialog
    
protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::FilePath selectedScenePathname;
};


class CommandSaveSpecifiedScene: public Command
{
public:	
	DAVA_DEPRECATED(CommandSaveSpecifiedScene(Entity *activeScene, FilePath& filePath)); // DEPRECATED: using QFileDialog
    
protected:	
    
    virtual void Execute();

	Entity*		activeScene;
	FilePath	filePath;
};

class CommandExport: public Command
{
    
public:	
	DAVA_DEPRECATED(CommandExport(DAVA::eGPUFamily gpu)); // DEPRECATED: using of SceneEditorScreenMain, Cancel absent
    
protected:	
    
    virtual void Execute();
    
protected:
    DAVA::eGPUFamily gpuFamily;
    
};

class CommandSaveToFolderWithChilds: public Command
{
public:
	DAVA_DEPRECATED(CommandSaveToFolderWithChilds()); // DEPRECATED: using QFileDialog
protected:
        virtual void Execute();
};



#endif // #ifndef __FILE_COMMANDS_H__