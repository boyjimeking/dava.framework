/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "SceneSaverTool.h"
#include "SceneSaver.h"

#include "CommandLine/EditorCommandLineParser.h"

using namespace DAVA;

void SceneSaverTool::PrintUsage()
{
    printf("\n");
    printf("-scenesaver -save [-indir [directory]] [-outdir [directory]] [-processfile [directory]]\n");
    printf("-scenesaver -resave [-indir [directory]] [-processfile [directory]] [-forceclose]\n");
    printf("\twill save scene file from DataSource/3d to any Data or DataSource folder\n");
    printf("\t-save - will save level to selected Data/3d/\n");
    printf("\t-resave - will open and save level\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processfile - filename from DataSource/3d/ for saving\n");

    printf("\n");
    printf("Samples:\n");
    printf("-scenesaver -save -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");
    printf("-scenesaver -resave -indir /Users/User/Project/DataSource/3d -processfile Maps/level.sc2 -forceclose\n");
}

DAVA::String SceneSaverTool::GetCommandLineKey()
{
    return "-scenesaver";
}

bool SceneSaverTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    inFolder = EditorCommandLineParser::GetCommandParam(String("-indir"));
    if(inFolder.IsEmpty())
    {
        errors.insert("Incorrect indir parameter");
        return false;
    }
    inFolder.MakeDirectoryPathname();
    
    filename = EditorCommandLineParser::GetCommandParam(String("-processfile"));
    if(filename.empty())
    {
        errors.insert("Filename is not set");
        return false;
    }
    
    
    if(EditorCommandLineParser::CommandIsFound(String("-save")))
    {
        commandAction = ACTION_SAVE;
        outFolder = EditorCommandLineParser::GetCommandParam(String("-outdir"));
        if(outFolder.IsEmpty())
        {
            errors.insert("Incorrect outdir parameter");
            return false;
        }
        outFolder.MakeDirectoryPathname();
    }
    else if(EditorCommandLineParser::CommandIsFound(String("-resave")))
    {
        commandAction = ACTION_RESAVE;
    }
    else
    {
        errors.insert("Incorrect action");
        return false;
    }
    
    return true;
}

void SceneSaverTool::Process()
{
    SceneSaver saver;

    saver.SetInFolder(inFolder);
    if(commandAction == ACTION_SAVE)
    {
        saver.SetOutFolder(outFolder);
        saver.SaveFile(filename, errors);
    }
    else if(commandAction == ACTION_RESAVE)
    {
        saver.ResaveFile(filename, errors);
    }
}


