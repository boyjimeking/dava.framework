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


#ifndef __UIEditor__HierarchyTreeNode__
#define __UIEditor__HierarchyTreeNode__

#include <set>
#include "DAVAEngine.h"
#include <QString>
#include "HierarchyTreeNodeExtraData.h"

using namespace DAVA;

// Base class for all Hierarchy Tree Nodes.
class HierarchyTreeNode
{
public:
    // Type definitions for the Tree Node.
	typedef std::list<HierarchyTreeNode*> HIERARCHYTREENODESLIST;
    typedef HIERARCHYTREENODESLIST::iterator HIERARCHYTREENODESITER;
    typedef HIERARCHYTREENODESLIST::const_iterator HIERARCHYTREENODESCONSTITER;
    
	typedef int HIERARCHYTREENODEID;
	typedef std::list<HIERARCHYTREENODEID> HIERARCHYTREENODESIDLIST;
  	static const HIERARCHYTREENODEID HIERARCHYTREENODEID_EMPTY = -1;

    HierarchyTreeNode(const QString& name);
	HierarchyTreeNode(const HierarchyTreeNode* node);
    virtual ~HierarchyTreeNode();
    
    // Add the node to the list.
    void AddTreeNode(HierarchyTreeNode* treeNode);
    void AddTreeNode(HierarchyTreeNode* treeNode, HierarchyTreeNode* nodeToAddAfter);

    // Remove the node from the list, return TRUE if succeeded.
    bool RemoveTreeNode(HierarchyTreeNode* treeNode, bool needDelete, bool needRemoveFromScene);
    
    // Access to the nodes list.
    const HIERARCHYTREENODESLIST& GetChildNodes() const;
    
	virtual void SetName(const QString& name) {this->name = name;};
    const QString& GetName() const {return name;};
	
	HIERARCHYTREENODEID GetId() const {return id;};
	void UpdateId(HIERARCHYTREENODEID newID) { this->id = newID; };

    // Access to the node extra data.
    HierarchyTreeNodeExtraData& GetExtraData() {return extraData;};
	
	virtual void SetParent(HierarchyTreeNode* /*node*/, HierarchyTreeNode* /*insertAfter*/){};
	virtual HierarchyTreeNode* GetParent() {return NULL;};

	bool IsHasChild(const HierarchyTreeNode* node) const;

	// Remove the tree node from scene, but keep it in memory.
	virtual void RemoveTreeNodeFromScene() {};
	
	// Return it back to the scene.
	virtual void ReturnTreeNodeToScene() {};

	// Prepare the Undo/Redo information.
	void PrepareRemoveFromSceneInformation();

	virtual bool IsMarked() const;
	virtual bool IsNeedSave() const;
	bool HasUnsavedChilder() const;
	void SetMarked(bool marked);
	void SetChildrenMarked(bool marked, bool recursive = false);

	// Modifiers for the unsaved changes counter.
	void IncrementUnsavedChanges();
	void DecrementUnsavedChanges();
	void ResetUnsavedChanges();

protected:
	HIERARCHYTREENODEID id;
	
	QString name;
	
    // Cleanup the list of tree nodes.
    void Cleanup();
    
    // List of child nodes.
    HIERARCHYTREENODESLIST childNodes;
    
    // Tree node extra data.
    HierarchyTreeNodeExtraData extraData;
	
	static HIERARCHYTREENODEID nextId;
	
	// Undo/Redo information.
	HierarchyTreeNode* redoParentNode;
	HierarchyTreeNode* redoPreviousNode;

	bool marked;
	int32 unsavedChangesCounter;
};


#endif /* defined(__UIEditor__HierarchyTreeNode__) */
