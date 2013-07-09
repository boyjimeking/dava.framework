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

#include "SceneExporterTool.h"
#include "SceneExporter.h"

#include "TexturePacker/CommandLineParser.h"

using namespace DAVA;


void SceneExporterTool::PrintUsage()
{
    printf("\n");
    printf("-sceneexporter [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\twill export scene file from DataSource/3d to Data/3d\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n");
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n");
    printf("\t-gpu - PoverVR_iOS, PoverVR_Android, tegra, mali, adreno\n");
    
    printf("\n");
    printf("Samples:\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");

}

DAVA::String SceneExporterTool::GetCommandLineKey()
{
    return "-sceneexporter";
}

bool SceneExporterTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    inFolder = CommandLineParser::GetCommandParam(String("-indir"));
    outFolder = CommandLineParser::GetCommandParam(String("-outdir"));
    if(inFolder.IsEmpty() && outFolder.IsEmpty())
    {
        errors.insert(Format("Incorrect indir (%s) or outdir (%s) parameter",inFolder.GetAbsolutePathname().c_str(), outFolder.GetAbsolutePathname().c_str()));
        return false;
    }
    
    inFolder.MakeDirectoryPathname();
    outFolder.MakeDirectoryPathname();
    
    gpu = CommandLineParser::GetCommandParam(String("-gpu"));
    if(gpu.empty())
    {
        errors.insert("Format for export is not set");
        return false;
    }
    
    filename = CommandLineParser::GetCommandParam(String("-processfile"));
    foldername = CommandLineParser::GetCommandParam(String("-processdir"));

    if(!filename.empty())
    {
        commandAction = ACTION_EXPORT_FILE;
    }
    else if(!foldername.empty())
    {
        commandAction = ACTION_EXPORT_FOLDER;
    }
    else
    {
        errors.insert("File or folder for export is not set");
        return false;
    }
    
    
    return true;
}

void SceneExporterTool::Process()
{
    SceneExporter exporter;

    exporter.SetOutFolder(outFolder);
    exporter.SetInFolder(inFolder);
    exporter.SetGPUForExporting(gpu);
    
    if(commandAction == ACTION_EXPORT_FILE)
    {
        exporter.ExportFile(filename, errors);
    }
    else if(commandAction == ACTION_EXPORT_FOLDER)
    {
        exporter.ExportFolder(foldername, errors);
    }
}


