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

#include "BeastCommandLineTool.h"

#include "CommandLine/EditorCommandLineParser.h"

using namespace DAVA;

BeastCommandLineTool::BeastCommandLineTool()
	:	CommandLineTool()
{
	oneFrameCommand = false;
}

void BeastCommandLineTool::PrintUsage()
{
    printf("\n");
    printf("-beast [-file [file]]\n");
    printf("\twill beast scene file\n");
    printf("\t-file - full pathname of scene for beasting \n");
    printf("\t-format - png, pvr, dxt\n");
    
    printf("\n");
    printf("Samples:\n");
    printf("-beast -file /Projects/WOT/wot.blitz/DataSource/3d/Maps/karelia/karelia.sc2\n");

}

DAVA::String BeastCommandLineTool::GetCommandLineKey()
{
    return "-beast";
}

bool BeastCommandLineTool::InitializeFromCommandLine()
{
    scenePathname = EditorCommandLineParser::GetCommandParam(String("-file"));
    if(scenePathname.IsEmpty())
    {
        errors.insert(String("Incorrect params for beasting of the scene"));
        return false;
    }
    
    if(!scenePathname.IsEqualToExtension(".sc2"))
    {
        errors.insert(String("Wrong pathname. Need path ot *.sc2"));
        return false;
    }
    
    return true;
}

void BeastCommandLineTool::Process()
{
    //Do nothing
}

const DAVA::FilePath & BeastCommandLineTool::GetScenePathname() const
{
    return scenePathname;
}


