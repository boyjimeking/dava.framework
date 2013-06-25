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

#include "npapisdk/headers/npapi.h"
#include "npapisdk/headers/npfunctions.h"
#include "ApplicationCore.h"
#include "UIEvent.h"

#import <Foundation/Foundation.h>
#import "NPAPIOpenGLLayerMacOS.h"

// An interface to handle calls from NPAPI.
@interface NPAPIPluginMacOS : NSObject
{
	NPP npp;
	NPWindow* npWindow;
	NPAPIOpenGLLayerMacOS* openGLLayer;
	
	DAVA::ApplicationCore* appCore;
	
	// Whether browser window has focus?
	BOOL hasFocused;

	// Plugin width and height.
	int pluginWidth;
	int pluginHeight;

	// Bundle Path to the Plugin (passed from the HTML page).
	NSString* bundlePath;

	DAVA::Vector<DAVA::UIEvent> activeTouches;
	DAVA::int32 oldModifiersFlags;
}

// Constructor/destructor.
-(NPAPIPluginMacOS*) initWithNPP:(NPP)instance;
-(void) dealloc;

// Initialize with parameters sent from the plugin.
-(NPError) npNew:(NPMIMEType)pluginType npMode:(uint16_t) mode npArgCount:(int16_t)argCount
	npArgNames:(char*[])argNames npArgValues:(char*[]) argValues npSavedData:(NPSavedData*) savedData;

// Set the NPWindow.
-(NPError) npSetWindow:(NPWindow*) window;

// Handle the Cocoa event came from browser.
-(NPError) npHandleEvent:(NPCocoaEvent*) event;

// Called when the browser requests us to return a value.
-(NPError) npGetValue:(NPPVariable) variable withParam:(void*)param;

// Called by OpenGL Layer to perform initialization on the first draw.
-(void) doInitializationOnFirstDraw;

// Event handlers
-(NPError) handleWindowFocusChanged:(BOOL) newValue;
- (void) onSuspend;
- (void) onResume;

-(void) parseEvent:(NPCocoaEvent*) event;
-(void) processEvent:(int)touchPhase touch:(NPCocoaEvent*)touch;
-(void) moveTouchesToVector:(NPCocoaEvent*)curEvent touchPhase:(int)touchPhase outTouches:(DAVA::Vector<DAVA::UIEvent>*)outTouches;

-(void) keyDown:(NPCocoaEvent*) event;
-(void) keyUp:(NPCocoaEvent*) event;
-(void) flagsChanged:(NPCocoaEvent*) event;

@end
