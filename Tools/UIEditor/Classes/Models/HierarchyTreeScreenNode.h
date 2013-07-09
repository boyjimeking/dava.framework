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


#ifndef __UIEditor__HierarchyTreeScreenNode__
#define __UIEditor__HierarchyTreeScreenNode__

#include "HierarchyTreeNode.h"
#include "ScreenControl.h"

using namespace DAVA;

class HierarchyTreeControlNode;
class HierarchyTreePlatformNode;

// "Screen" node for the Hierarchy Tree.
class HierarchyTreeScreenNode: public HierarchyTreeNode
{
public:
	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const QString& name);
	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base);
	~HierarchyTreeScreenNode();
	
	HierarchyTreePlatformNode* GetPlatform() const {return parent;};
	ScreenControl* GetScreen() const {return screen;};
	
	void SetScale(float scale);
	float GetScale() const;
	
	void SetPosX(int x);
	int GetPosX() const;
	void SetPosY(int y);
	int GetPosY() const;
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual HierarchyTreeNode* GetParent();
	
	String GetNewControlName(const String& baseName) const;
	
	bool IsNameExist(const QString& name, const HierarchyTreeNode* parent) const;
	
	bool Load(const QString& path);
	bool Save(const QString& path, bool saveAll);
	
	virtual void ReturnTreeNodeToScene();
	virtual Rect GetRect() const;
	
	virtual void RemoveSelection() {};

	// Access to the screen unsaved changes counter.
	int32 GetUnsavedChanges() const {return unsavedChangesCounter;};

	virtual bool IsNeedSave() const;

protected:
	void CombineRectWithChild(Rect& rect) const;

private:
	void BuildHierarchyTree(HierarchyTreeNode* parent, List<UIControl*> child);
	
protected:
	HierarchyTreePlatformNode* parent;
	ScreenControl* screen;
	
	float scale;
	int posX;
	int posY;
};

#endif /* defined(__UIEditor__HierarchyTreeScreenNode__) */
