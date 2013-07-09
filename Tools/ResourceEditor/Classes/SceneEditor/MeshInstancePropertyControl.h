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

#ifndef __MESH_INSTANCE_PROPERTY_CONTROL_H__
#define __MESH_INSTANCE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class MeshInstancePropertyControl : public NodesPropertyControl
{
public:
	MeshInstancePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~MeshInstancePropertyControl();

	virtual void ReadFrom(Entity * sceneNode);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
	virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);

	void OnConvertToShadowVolume(BaseObject * object, void * userData, void * callerData);
    
protected:

	int32 GetFlagValue(const String &keyName, int32 flagValue);



	int32 GetIndexFromKey(const String &forKey);
    void OnGo2Materials(BaseObject * object, void * userData, void * callerData);
    void OnShowTexture(BaseObject * object, void * userData, void * callerData);

    Vector<Material*> materials;
    Vector<String> materialNames;
};

#endif //__MESH_INSTANCE_PROPERTY_CONTROL_H__
