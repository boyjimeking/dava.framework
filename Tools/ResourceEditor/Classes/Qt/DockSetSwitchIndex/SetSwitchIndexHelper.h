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

#ifndef __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__
#define __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class SwitchComponent;

class SetSwitchIndexHelper
{
public:
	enum eSET_SWITCH_INDEX
	{
		FOR_SELECTED = 0,
		FOR_SCENE
	};

	static void ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state, Set<Entity*>& affectedEntities, Map<SwitchComponent *, int32>& originalIndexes);
	
	static void RestoreOriginalIndexes(Map<SwitchComponent *, int32>& originalIndexes, Set<Entity*>& affectedEntities);
};
};

#endif /* defined(__RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__) */
