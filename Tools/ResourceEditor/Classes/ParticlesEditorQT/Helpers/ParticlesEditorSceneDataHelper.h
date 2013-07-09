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

#ifndef __ResourceEditorQt__ParticlesEditorSceneDataHelper__
#define __ResourceEditorQt__ParticlesEditorSceneDataHelper__

#include "DAVAEngine.h"
#include "Scene3D/Entity.h"

namespace DAVA {

// Scene Data Helper for Particles Editor. Contains no Qt-related code.
class ParticlesEditorSceneDataHelper
{
public:
	// Add the new node, if it is related to Particles Editor. Returns TRUE if
	// no further processing needed.
	bool AddSceneNode(Entity* node) const;
		
	// Remove the Scene Node, if it is related to Particles Editor.
	void RemoveSceneNode(Entity* node) const;
	
	// Validate the Particle Emitter, generate the error message, if needed.
	static bool ValidateParticleEmitter(ParticleEmitter* emitter, String& validationMsg);
};

};

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneDataHelper__) */
