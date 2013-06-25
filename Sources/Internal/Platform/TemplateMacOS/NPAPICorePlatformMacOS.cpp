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

#include "NPAPICorePlatformMacOS.h"

namespace DAVA {
	
NPAPICoreMacOSPlatform::NPAPICoreMacOSPlatform()
{
}

NPAPICoreMacOSPlatform::~NPAPICoreMacOSPlatform()
{
}

Core::eScreenMode NPAPICoreMacOSPlatform::GetScreenMode()
{
    return Core::MODE_WINDOWED;
}

void NPAPICoreMacOSPlatform::ToggleFullscreen()
{
	Logger::Error("Toggle Full Screen is not supported on NPAPI Plugin");
}

void NPAPICoreMacOSPlatform::SwitchScreenToMode(eScreenMode screenMode)
{
    Logger::Error("[NPAPICoreMacOSPlatform::SwitchScreenToMode] is not supported on NPAPI Plugin.");
}

void* NPAPICoreMacOSPlatform::GetOpenGLView()
{
	// No OpenGL View exists on the NPAPI Plugin - layer only.
	return NULL;
}


// some macros to make code more readable.
#define GetModeWidth(mode) GetDictionaryLong((mode), kCGDisplayWidth)
#define GetModeHeight(mode) GetDictionaryLong((mode), kCGDisplayHeight)
#define GetModeRefreshRate(mode) GetDictionaryLong((mode), kCGDisplayRefreshRate)
#define GetModeBitsPerPixel(mode) GetDictionaryLong((mode), kCGDisplayBitsPerPixel)

//------------------------------------------------------------------------------------------
long NPAPICoreMacOSPlatform::GetDictionaryLong(CFDictionaryRef theDict, const void* key)
{
	// get a long from the dictionary
	long value = 0;
	CFNumberRef numRef;
	numRef = (CFNumberRef)CFDictionaryGetValue(theDict, key);
	if (numRef != NULL)
		CFNumberGetValue(numRef, kCFNumberLongType, &value);
	return value;
}

DisplayMode NPAPICoreMacOSPlatform::GetCurrentDisplayMode()
{
	CFDictionaryRef currentMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	
	DisplayMode mode;
	mode.width = GetModeWidth(currentMode);
	mode.height = GetModeHeight(currentMode);
	mode.refreshRate = GetModeRefreshRate(currentMode);
	mode.bpp = GetModeBitsPerPixel(currentMode);
	
	return mode;
}

void NPAPICoreMacOSPlatform::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{
	// One and only Display Mode is available on NPAPI plugin.
	availableModes.push_back(GetCurrentDisplayMode());
}

};
