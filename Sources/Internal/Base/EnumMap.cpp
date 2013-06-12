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

#include "Base/EnumMap.h"
#include "Debug/DVAssert.h"

EnumMap::EnumMap()
{}

EnumMap::~EnumMap()
{}

void EnumMap::Register(const int e, const char* s) const
{
	DVASSERT(!map.count(e));
	map[e] = DAVA::String(s);
}

void EnumMap::UnregistelAll() const
{
	map.clear();
}

bool EnumMap::ToValue(const char* s, int &e) const
{
	bool ret = false;
	EnumMapContainer::const_iterator i = map.begin();
	EnumMapContainer::const_iterator end = map.end();
	DAVA::String str(s);

	for(; i != end; ++i)
	{
		if(i->second == str)
		{
			e = i->first;
			ret = true;
			break;
		}
	}

	return ret;
}

const char* EnumMap::ToString(const int e) const
{
	const char* ret = NULL;

	if(map.count(e))
	{
		ret = map.at(e).c_str();
	}

	return ret;
}

size_t EnumMap::GetCount() const
{
	return map.size();
}

bool EnumMap::GetValue(size_t index, int &e) const
{
	bool ret = false;
	EnumMapContainer::const_iterator i = map.begin();

	if(index < map.size())
	{
		while(index > 0)
		{
			index--;
			i++;
		}

		if(i != map.end())
		{
			e = i->first;
			ret = true;
		}
	}

	return ret;
}
