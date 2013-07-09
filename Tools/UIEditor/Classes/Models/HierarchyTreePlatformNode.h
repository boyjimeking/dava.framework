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

#ifndef __UIEditor__HierarchyTreePlatformNode__
#define __UIEditor__HierarchyTreePlatformNode__

#include "HierarchyTreeNode.h"
#include "HierarchyTreeRootNode.h"

using namespace DAVA;
    
// "Platform" node for the Hierarchy Tree.
class HierarchyTreePlatformNode: public HierarchyTreeNode
{
public:
	HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const QString& name);
	HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const HierarchyTreePlatformNode* base);

	virtual ~HierarchyTreePlatformNode();

	void SetSize(int width, int height);
	int GetWidth() const;
	int GetHeight() const;

	virtual HierarchyTreeNode* GetParent();

	HierarchyTreeNode* GetRoot() {return rootNode;};
	
	FilePath GetPlatformFolder() const;
	void ActivatePlatform();

	QString GetScreenPath(QString screenName) const;
	QString GetScreenPath(String screenName) const;
	
	bool Load(YamlNode* node);
	bool Save(YamlNode* node, bool saveAll);

    // Separate method to load/save localization.
    bool LoadLocalization(YamlNode* platform);
    bool SaveLocalization(YamlNode* platform);

    // Accessors to the current localization info.
    const FilePath & GetLocalizationPath() const {return localizationPath;};
    const String& GetLocale() const {return locale;};
    
    void SetLocalizationPath(const FilePath & localizationPath);
    void SetLocale(const String& locale);
    
	// Return the Platform Node back to scene after deletion when performing Undo.
	virtual void ReturnTreeNodeToScene();
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);

	bool IsAggregatorOrScreenNamePresent(const QString& candidatName);

private:
	int width;
	int height;

    FilePath localizationPath;
    String locale;

	HierarchyTreeRootNode* rootNode;
};

#endif /* defined(__UIEditor__HierarchyTreePlatformNode__) */
