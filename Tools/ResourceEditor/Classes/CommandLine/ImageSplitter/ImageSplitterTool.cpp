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

#include "ImageSplitterTool.h"
#include "ImageSplitter.h"

#include "CommandLine/EditorCommandLineParser.h"

using namespace DAVA;


void ImageSplitterTool::PrintUsage()
{
    printf("\n");
    printf("-imagesplitter -split [-file [file]]\n");
    printf("-imagesplitter -merge [-folder [directory]]\n");
    printf("\twill split one image at four channels or merge four channels to one image\n");
    printf("\t-file - filename of the splitting file\n");
    printf("\t-folder - path for folder with four channels\n");

    printf("\n");
    printf("Samples:\n");
    printf("-imagesplitter -split -file /Users/User/Project/Data/3d/image.png\n");
    printf("-imagesplitter -merge -folder /Users/User/Project/Data/3d/\n");
}

DAVA::String ImageSplitterTool::GetCommandLineKey()
{
    return "-imagesplitter";
}

bool ImageSplitterTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    if(EditorCommandLineParser::CommandIsFound(String("-split")))
    {
        commandAction = ACTION_SPLIT;
        filename = EditorCommandLineParser::GetCommandParam(String("-file"));
        if(filename.IsEmpty())
        {
            errors.insert(String("Incorrect params for splitting of the file"));
            return false;
        }
    }
    else if(EditorCommandLineParser::CommandIsFound(String("-merge")))
    {
        commandAction = ACTION_MERGE;
  
        foldername = EditorCommandLineParser::GetCommandParam(String("-folder"));
        if(foldername.IsEmpty())
        {
            errors.insert(String("Incorrect params for merging of the files"));
            return false;
        }
        foldername.MakeDirectoryPathname();
    }
    else
    {
        errors.insert(String("Incorrect params for merging of the files"));
        return false;
    }
    
    return true;
}

void ImageSplitterTool::Process()
{
    if(commandAction == ACTION_SPLIT)
    {
        ImageSplitter::SplitImage(filename, errors);
    }
    else if(commandAction == ACTION_MERGE)
    {
        ImageSplitter::MergeImages(foldername, errors);
    }
}


