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

#ifndef __STRING_CONSTANTS_H__
#define __STRING_CONSTANTS_H__

using namespace DAVA;

namespace ResourceEditor 
{
	// Node names
	static const String LANDSCAPE_NODE_NAME = "Landscape";
	static const String LIGHT_NODE_NAME = "Light";
	static const String SERVICE_NODE_NAME = "Servicenode";
	static const String CAMERA_NODE_NAME = "Camera";
	static const String IMPOSTER_NODE_NAME = "Imposter";
	static const String PARTICLE_EMITTER_NODE_NAME = "Particle Emitter";
	static const String USER_NODE_NAME = "UserNode";
	static const String SWITCH_NODE_NAME = "SwitchNode";
	static const String PARTICLE_EFFECT_NODE_NAME = "Particle Effect";
	static const String LAYER_NODE_NAME = "Layer";

	// Base node names
	static const String EDITOR_BASE = "editor.";
	static const String EDITOR_MAIN_CAMERA = "editor.main-camera";
	static const String EDITOR_DEBUG_CAMERA = "editor.debug-camera";
	static const String EDITOR_ARROWS_NODE = "editor.arrows-node";
	static const String EDITOR_CAMERA_LIGHT = "editor.camera-light";
	
	// Headers
	static const WideString CREATE_NODE_LANDSCAPE = L"createnode.landscape";
	static const WideString CREATE_NODE_LIGHT = L"createnode.light";
	static const WideString CREATE_NODE_SERVICE = L"createnode.servicenode";
	static const WideString CREATE_NODE_CAMERA = L"createnode.camera";
	static const WideString CREATE_NODE_IMPOSTER = L"createnode.imposter";
	static const WideString CREATE_NODE_PARTICLE_EMITTER = L"createnode.particleemitter";
	static const WideString CREATE_NODE_USER = L"createnode.usernode";
	static const WideString CREATE_NODE_SWITCH = L"createnode.switchnode";
	static const WideString CREATE_NODE_PARTICLE_EFFECT = L"Particle Effect";
	
	// Properties
	static const String EDITOR_REFERENCE_TO_OWNER = "editor.referenceToOwner";
	static const String EDITOR_IS_LOCKED = "editor.isLocked";
	static const String EDITOR_DO_NOT_REMOVE = "editor.donotremove";

    //Documentation
    static const String DOCUMENTATION_PATH = "~doc:/ResourceEditorHelp/";
}

#endif //#ifndef __STRING_CONSTANTS_H__
