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

#import "NPAPIPluginMacOS.h"

#include "NPAPICorePlatformMacOS.h"
#include "RenderManager.h"

#include <pwd.h>

// Relative path to NPAPI Internet Plugins for MacOS.
#define PATH_TO_INTERNET_PLUGINS @"Library/Internet Plug-Ins"

// Arguments passed to the NPAPI plugin instance.
#define NPAPI_PLUGIN_ARGUMENT_WIDTH @"width"
#define NPAPI_PLUGIN_ARGUMENT_HEIGHT @"height"
#define NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME @"bundlename"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

@implementation NPAPIPluginMacOS

-(NPAPIPluginMacOS*) initWithNPP:(NPP)instance
{
	if (self = [super init])
	{
		self->npp = instance;
		self->npWindow = NULL;
		self->openGLLayer = NULL;
		self->appCore = NULL;

		self->pluginWidth = 0;
		self->pluginHeight = 0;
		self->bundlePath = NULL;
		
		self->hasFocused = NO;

		oldModifiersFlags = 0;
	}
	
	return self;
}

-(void) dealloc
{
	[self terminateFramework];

	[openGLLayer release];
	openGLLayer = NULL;
	
	[bundlePath release];
	bundlePath = NULL;
	
	[super dealloc];
}

-(NPError) npNew:(NPMIMEType)pluginType npMode:(uint16_t) mode npArgCount:(int16_t)argCount
	  npArgNames:(char*[])argNames npArgValues:(char*[]) argValues npSavedData:(NPSavedData*) savedData
{
	// Lookup for the parameters needed for the initialization.
	NSLog(@"npNew: Params count: %i", argCount);
	
	// Lookup for the params needed to initialize the plugin properly.
	for (int16_t i = 0; i < argCount; i ++)
	{
		NSString* argName = [NSString stringWithCString:argNames[i] encoding:NSASCIIStringEncoding];
		NSString* argValue = [NSString stringWithCString:argValues[i] encoding:NSASCIIStringEncoding];
		NSLog(@"npNew: Param #%i name is %@, value is %@", i, argName, argValue);
		
		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_WIDTH] == NSOrderedSame)
		{
			self->pluginWidth = [argValue integerValue];
		}

		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_HEIGHT] == NSOrderedSame)
		{
			self->pluginHeight = [argValue integerValue];
		}

		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME] == NSOrderedSame)
		{
			[self setBundlePath: argValue];
		}
	}

	// Verify all the required parameters are specified.
	bool allParametersSpecified = true;
	if (self->pluginHeight == 0)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_HEIGHT];
		allParametersSpecified = false;
	}
	
	if (self->pluginWidth == 0)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_WIDTH];
		allParametersSpecified = false;
	}
	
	if (self->bundlePath == NULL)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME];
		allParametersSpecified = false;
	}

	// Yuri Coder, 2013/05/22. Do we need to pass these parameters to some other location?
	return allParametersSpecified ? NPERR_NO_ERROR : NPERR_INVALID_PARAM;
}

-(void) logRequiredParameterNotSpecified:(NSString*) paramName
{
	NSLog(@"npNew: The %@ parameter for NPAPI DAVA Framework plugin is required is not specified", paramName);
}

// Set the NPWindow.
-(NPError) npSetWindow:(NPWindow*) window
{
	self->npWindow = window;
	return NPERR_NO_ERROR;
}

// Handle the Cocoa event came from browser.
-(NPError) npHandleEvent:(NPCocoaEvent*) event
{
	switch (event->type)
	{
		case NPCocoaEventWindowFocusChanged:
		{
			return [self handleWindowFocusChanged: event->data.focus.hasFocus];
		}

		default:
		{
			// Key/mouse tracking is implemented here.
			[self parseEvent:event];
			return NPERR_NO_ERROR;
		}
	}
}

-(void) parseEvent:(NPCocoaEvent*)event
{
	switch (event->type)
	{
		case NPCocoaEventDrawRect:
			break;

		case NPCocoaEventMouseDown:
			[self processEvent:DAVA::UIEvent::PHASE_BEGAN touch:event];
			break;

		case NPCocoaEventMouseUp:
			[self processEvent:DAVA::UIEvent::PHASE_ENDED touch:event];
			break;

		case NPCocoaEventMouseMoved:
			[self processEvent:DAVA::UIEvent::PHASE_MOVE touch:event];
			break;

		case NPCocoaEventMouseEntered:
			break;

		case NPCocoaEventMouseExited:
			break;

		case NPCocoaEventMouseDragged:
			[self processEvent:DAVA::UIEvent::PHASE_DRAG touch:event];
			break;

		case NPCocoaEventKeyDown:
			[self keyDown:event];
			break;

		case NPCocoaEventKeyUp:
			[self keyUp:event];
			break;

		case NPCocoaEventFlagsChanged:
			[self flagsChanged:event];
			break;

		case NPCocoaEventScrollWheel:
			break;

		case NPCocoaEventTextInput:
			break;

		default:
			break;
	}
}

-(void) moveTouchesToVector:(NPCocoaEvent*)curEvent touchPhase:(int)touchPhase outTouches:(DAVA::Vector<DAVA::UIEvent>*)outTouches
{
	int button = 0;
	button = curEvent->data.mouse.buttonNumber + 1;
	time_t timestamp = time(NULL);

	if (touchPhase == DAVA::UIEvent::PHASE_DRAG)
	{
		for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			NSPoint p;
			p.x = curEvent->data.mouse.pluginX;
			p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;

			if(DAVA::InputSystem::Instance()->IsCursorPining())
			{
				it->physPoint.x = curEvent->data.mouse.deltaX;
				it->physPoint.y = curEvent->data.mouse.deltaY;
			}

			it->tapCount = DAVA::Max(curEvent->data.mouse.clickCount, 1);
			it->timestamp = timestamp;
			it->phase = touchPhase;
		}
	}

	bool isFind = false;
	for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		if(it->tid == button)
		{
			isFind = true;

			NSPoint p;
			p.x = curEvent->data.mouse.pluginX;
			p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;
			
			if(DAVA::InputSystem::Instance()->IsCursorPining())
			{
				it->physPoint.x = curEvent->data.mouse.deltaX;
				it->physPoint.y = curEvent->data.mouse.deltaY;
			}

			it->tapCount = curEvent->data.mouse.clickCount;
			it->timestamp = timestamp;
			it->phase = touchPhase;

			break;
		}
	}

	if(!isFind)
	{
		DAVA::UIEvent newTouch;
		newTouch.tid = button;
		NSPoint p;
		p.x = curEvent->data.mouse.pluginX;
		p.y = curEvent->data.mouse.pluginY;

		newTouch.physPoint.x = p.x;
		newTouch.physPoint.y = p.y;

		if(DAVA::InputSystem::Instance()->IsCursorPining())
		{
			newTouch.physPoint.x = curEvent->data.mouse.deltaX;
			newTouch.physPoint.y = curEvent->data.mouse.deltaY;
		}

		newTouch.tapCount = curEvent->data.mouse.clickCount;
		newTouch.timestamp = timestamp;
		newTouch.phase = touchPhase;
		activeTouches.push_back(newTouch);
	}

	for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		outTouches->push_back(*it);
	}

	if(touchPhase == DAVA::UIEvent::PHASE_ENDED || touchPhase == DAVA::UIEvent::PHASE_MOVE)
	{
		for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				activeTouches.erase(it);
				break;
			}
		}
	}
}

-(void) processEvent:(int)touchPhase touch:(NPCocoaEvent*)touch
{
	DAVA::Vector<DAVA::UIEvent> touches;
	DAVA::Vector<DAVA::UIEvent> emptyTouches;
	[self moveTouchesToVector:touch touchPhase:touchPhase outTouches:&touches];

	DAVA::UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);
	touches.clear();
	
	if ((touchPhase == DAVA::UIEvent::PHASE_MOVE || touchPhase == DAVA::UIEvent::PHASE_DRAG)
		&& DAVA::InputSystem::Instance()->IsCursorPining())
	{
		DAVA::Cursor::MoveToCenterOfWindow();
	}
}

-(void) keyDown:(NPCocoaEvent*)event
{
	NSString* s = (NSString*)event->data.key.characters;
	unichar c = [s characterAtIndex:0];

	DAVA::Vector<DAVA::UIEvent> touches;
	DAVA::Vector<DAVA::UIEvent> emptyTouches;

	for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		touches.push_back(*it);
	}

	time_t timestamp = time(NULL);

	DAVA::UIEvent ev;
	ev.keyChar = c;
	ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
	ev.timestamp = timestamp;
	ev.tapCount = 1;
	ev.tid = DAVA::InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey(event->data.key.keyCode);

	touches.push_back(ev);

	DAVA::UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
	touches.pop_back();
	DAVA::UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

	DAVA::InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(event->data.key.keyCode);
	if (event->data.key.modifierFlags & NSCommandKeyMask)
	{
		DAVA::InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(event->data.key.keyCode);
	}
}

-(void) keyUp:(NPCocoaEvent*) event
{
	DAVA::InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(event->data.key.keyCode);
}

-(void) flagsChanged:(NPCocoaEvent*) event
{
	DAVA::int32 newModifiers = event->data.key.modifierFlags;

	static DAVA::int32 masks[] = {
		NSAlphaShiftKeyMask,
		NSShiftKeyMask,
		NSControlKeyMask,
		NSAlternateKeyMask,
		NSCommandKeyMask};

	static DAVA::int32 keyCodes[] = {
		DAVA::DVMACOS_CAPS_LOCK,
		DAVA::DVMACOS_SHIFT,
		DAVA::DVMACOS_CONTROL,
		DAVA::DVMACOS_OPTION,
		DAVA::DVMACOS_COMMAND};

	for (int i = 0; i < 5; i++)
	{
		if ((oldModifiersFlags & masks[i]) != (newModifiers & masks[i]))
		{
			if (newModifiers & masks[i])
			{
				DAVA::InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(keyCodes[i]);
			}
			else
			{
				DAVA::InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(keyCodes[i]);
			}
		}
	}

	oldModifiersFlags = newModifiers;
}


// Called when the browser requests us to return a value.
-(NPError) npGetValue:(NPPVariable) variable withParam:(void*)param
{
	switch (variable)
	{
		case NPPVpluginCoreAnimationLayer:
		{
			[self createOpenGLLayer];

			*((CALayer **)param) = openGLLayer;
			return NPERR_NO_ERROR;
		}
			
		default:
		{
			return NPERR_GENERIC_ERROR;
		}
	}
}

-(void) setBundlePath:(NSString*) bundleName
{
	NSString* homeDir = NULL;
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
	{
		homeDir = [NSString stringWithCString:pwd->pw_dir encoding:NSASCIIStringEncoding];
	}

	if (homeDir)
	{
		self->bundlePath = [[[homeDir stringByAppendingPathComponent: PATH_TO_INTERNET_PLUGINS]
							 stringByAppendingPathComponent: bundleName] retain];
	}

	NSLog(@"Bundle Path is set to %@", self->bundlePath);
}

-(void) createOpenGLLayer
{
	if (self->openGLLayer)
	{
		// Already created.
		return;
	}

	NSLog(@"Creating NPAPI OpenGL Layer");
	openGLLayer = [[NPAPIOpenGLLayerMacOS alloc] initWithPluginInstance:self];
	openGLLayer.opaque = YES;
	
	// Initialize the layer with the height and width of the plugin.
	[openGLLayer setFrame:CGRectMake(0.0f,  0.0f, self->pluginWidth, self->pluginHeight)];


	// The OpenGL layer is async and will be responsible for updates by itself.
	openGLLayer.asynchronous = YES;
	[openGLLayer retain];   // should be balanced by a -release in NPP_Destroy
	
	// Ready to initialize the framework.
	[self initializeFramework];
}

-(void) initializeFramework
{
	//sleep(15);
	DAVA::NPAPICoreMacOSPlatform * core = new DAVA::NPAPICoreMacOSPlatform();
	core->CreateSingletons();

	#if defined (__DAVAENGINE_NPAPI__)
	// A Bundle Path have to be updated for the NPAPI Plugin.
	// Yuri Coder, 2013/05/22. TODO: looks like temporary patch.
	DAVA::FilePath::InitializeBundleNameNPAPI([bundlePath UTF8String]);
	#endif // #if defined (__DAVAENGINE_NPAPI__)

	FrameworkDidLaunched();
    DAVA::RenderManager::Create(DAVA::Core::RENDERER_OPENGL);

	appCore = DAVA::Core::GetApplicationCore();
}

-(void) doInitializationOnFirstDraw
{
    DAVA::RenderManager::Instance()->DetectRenderingCapabilities();

	NSRect rect = [openGLLayer frame];
	DAVA::RenderManager::Instance()->Init(rect.size.width, rect.size.height);
	DAVA::UIControlSystem::Instance()->SetInputScreenAreaSize(rect.size.width, rect.size.height);
	DAVA::Core::Instance()->SetPhysicalScreenSize(rect.size.width, rect.size.height);
    DAVA::Core::Instance()->SetVirtualScreenSize(rect.size.width, rect.size.height);
	
	NSLog(@"[NPAPICoreMacOSPlatform] SystemAppStarted");
	DAVA::Core::Instance()->SystemAppStarted();
}

-(void) terminateFramework
{
	FrameworkWillTerminate();	
}

#pragma mark Event Handlers

-(NPError) handleWindowFocusChanged:(BOOL)newValue
{
	NSLog(@"NPAPI Plugin Focus Changed, new value is %i", newValue);
	
	if (newValue == hasFocused)
	{
		// No change actually happened.
		return NPERR_NO_ERROR;
	}

	hasFocused = newValue;
	if (newValue)
	{
		[self onResume];
	}
	else
	{
		[self onSuspend];
	}

	return NPERR_NO_ERROR;
}

- (void) onSuspend
{
    if(appCore)
    {
        appCore->OnSuspend();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
}

- (void) onResume
{
    if(appCore)
    {
        appCore->OnResume();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }
}

@end
