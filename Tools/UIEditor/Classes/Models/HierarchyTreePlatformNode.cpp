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

#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeAggregatorNode.h"
#include "ResourcesManageHelper.h"
#include <QDir>

#define WIDTH_NODE "width"
#define HEIGHT_NODE "height"
#define SCREENS_NODE "screens"
#define AGGREGATORS_NODE "aggregators"

#define LOCALIZATION_PATH_NODE "LocalizationPath"
#define LOCALIZATION_LOCALE_NODE "Locale"

#define SCREEN_PATH "%1/%2.yaml"

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const QString& name) :
	HierarchyTreeNode(name)
{
	this->rootNode = rootNode;
	width = height = 0;
}

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const HierarchyTreePlatformNode* base) :
	HierarchyTreeNode(base)
{
	this->rootNode = rootNode;
	this->width = base->GetWidth();
	this->height = base->GetHeight();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& chilren = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = chilren.begin();
		 iter != chilren.end();
		 ++iter)
	{
		const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode* >((*iter));
		const HierarchyTreeAggregatorNode* aggregator = dynamic_cast<const HierarchyTreeAggregatorNode* >((*iter));

		if (!screen && !aggregator)
			continue;

		HierarchyTreeNode* control;
		if (aggregator)
		{
			control = new HierarchyTreeAggregatorNode(this, aggregator);
		}
		else
		{
			control = new HierarchyTreeScreenNode(this, screen);
		}
		AddTreeNode(control);
	}
}

HierarchyTreePlatformNode::~HierarchyTreePlatformNode()
{
	// Remove screens before removing aggregators
	HierarchyTreeNode::HIERARCHYTREENODESITER it;
	for (it = childNodes.begin(); it != childNodes.end();)
	{
		if (!dynamic_cast<HierarchyTreeAggregatorNode*>(*it))
		{
			delete (*it);
			it = childNodes.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void HierarchyTreePlatformNode::SetSize(int width, int height)
{
	this->width = width;
	this->height = height;
	
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregator)
			continue;
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (screenNode)
		{
			screenNode->GetScreen()->SetRect(Rect(0, 0, width, height));
		}
	}
}
	
int HierarchyTreePlatformNode::GetWidth() const
{
	return width;
}

int HierarchyTreePlatformNode::GetHeight() const
{
	return height;
}

HierarchyTreeNode* HierarchyTreePlatformNode::GetParent()
{
	// Root Node is the parent for the Platform.
	return this->rootNode;
}

FilePath HierarchyTreePlatformNode::GetPlatformFolder() const
{
	QString path;
	if (rootNode)
	{
		path = ResourcesManageHelper::GetPlatformRootPath(rootNode->GetProjectDir());
	}
	path += GetName();

    FilePath folder(path.toStdString());
    folder.MakeDirectoryPathname();
    
	return folder;
}

QString HierarchyTreePlatformNode::GetScreenPath(QString screenName) const
{
	return QString(SCREEN_PATH).arg(QString::fromStdString(GetPlatformFolder().GetAbsolutePathname())).arg(screenName);
}

QString HierarchyTreePlatformNode::GetScreenPath(String screenName) const
{
	return GetScreenPath(QString::fromStdString(screenName));
}

void HierarchyTreePlatformNode::ActivatePlatform()
{
	if (rootNode)
	{
		String bundleName = rootNode->GetProjectDir().toStdString();
        //after platform ativation only one item(projDir) should be placed to it
		FilePath::CleanResourcesFolders();
		FilePath::SetBundleName(bundleName);
	}
}

bool HierarchyTreePlatformNode::Load(YamlNode* platform)
{
	YamlNode* width = platform->Get(WIDTH_NODE);
	YamlNode* height = platform->Get(HEIGHT_NODE);
	if (!width || !height)
		return false;
	
	bool result = true;
	SetSize(width->AsInt(), height->AsInt());
	ActivatePlatform();
	
	YamlNode* screens = platform->Get(SCREENS_NODE);
	if (screens)
	{
		for (int i = 0; i < screens->GetCount(); i++)
		{
			YamlNode* screen = screens->Get(i);
			if (!screen)
				continue;
			String screenName = screen->AsString();
			
			QString screenPath = GetScreenPath(screenName);
			HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(this, QString::fromStdString(screenName));
			result &= screenNode->Load(screenPath);
			AddTreeNode(screenNode);
		}
	}
	
	YamlNode* aggregators = platform->Get(AGGREGATORS_NODE);
	if (aggregators)
	{
		for (int i = 0; i < aggregators->GetCount(); i++)
		{
			YamlNode* aggregator = aggregators->Get(i);
			if (!aggregator)
				continue;
			String aggregatorName = aggregator->AsString();

			QString aggregatorPath = GetScreenPath(aggregatorName);
			HierarchyTreeAggregatorNode* aggregatorNode = new HierarchyTreeAggregatorNode(this, QString::fromStdString(aggregatorName), Rect());

			YamlNode* aggregatorWidth = aggregator->Get(WIDTH_NODE);
			YamlNode* aggregatorHeight = aggregator->Get(HEIGHT_NODE);
			if (!aggregatorWidth || !aggregatorHeight)
			{
				result = false;
			}
			else
			{
				Rect r = Rect(0, 0, aggregatorWidth->AsInt(), aggregatorHeight->AsInt());
				result &= aggregatorNode->Load(r, aggregatorPath);
			}

			AddTreeNode(aggregatorNode);			
		}
	}
	
	return result;
}

bool HierarchyTreePlatformNode::LoadLocalization(YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }
    
    YamlNode* pathNode = platform->Get(LOCALIZATION_PATH_NODE);
    YamlNode* localeNode = platform->Get(LOCALIZATION_LOCALE_NODE);

    if (pathNode && localeNode &&
        !pathNode->AsString().empty() &&
        !localeNode->AsString().empty())
    {
        localizationPath = pathNode->AsString();
		// YuriCoder, 2013/04/23. Localization path must be absolute - do the conversion
		// for previous projects.
		localizationPath.MakeDirectoryPathname();

        locale = localeNode->AsString();
    }

    return true;
}

bool HierarchyTreePlatformNode::Save(YamlNode* node, bool saveAll)
{
	YamlNode* platform = new YamlNode(YamlNode::TYPE_MAP);
	platform->Set(WIDTH_NODE, GetWidth());
	platform->Set(HEIGHT_NODE, GetHeight());

	MultiMap<String, YamlNode*> &platformsMap = node->AsMap();
	platformsMap.erase(GetName().toStdString());
	platformsMap.insert(std::pair<String, YamlNode*>(GetName().toStdString(), platform));
	ActivatePlatform();
	
	MultiMap<String, YamlNode*> &platformMap = platform->AsMap();
	YamlNode* screens = new YamlNode(YamlNode::TYPE_ARRAY);
	platformMap.erase(SCREENS_NODE);
	platformMap.insert(std::pair<String, YamlNode*>(SCREENS_NODE, screens));
	
	YamlNode* aggregators = new YamlNode(YamlNode::TYPE_MAP);
	platformMap.erase(AGGREGATORS_NODE);
	platformMap.insert(std::pair<String, YamlNode*>(AGGREGATORS_NODE, aggregators));

    // Add the Localization info - specific for each Platform.
    SaveLocalization(platform);

	FilePath platformFolder = GetPlatformFolder();

	QDir dir;
	dir.mkpath(QString::fromStdString(platformFolder.GetAbsolutePathname()));
	
	bool result = true;
	
	//save aggregators node before save screens
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (!node)
			continue;

		QString path = GetScreenPath(node->GetName());
		MultiMap<String, YamlNode*> &aggregatorsMap = aggregators->AsMap();
		
		YamlNode* aggregator = new YamlNode(YamlNode::TYPE_MAP);
		result &= node->Save(aggregator, path, saveAll);
		
		aggregatorsMap.erase(node->GetName().toStdString());
		aggregatorsMap.insert(std::pair<String, YamlNode*>(node->GetName().toStdString(), aggregator));
	}
		
	
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (node)
			continue;	//skip aggregators
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*iter);
		if (!screenNode)
			continue;
		
		QString screenPath = GetScreenPath(screenNode->GetName());
		result &= screenNode->Save(screenPath, saveAll);
		
		screens->AddValueToArray(screenNode->GetName().toStdString());
	}
	return result;
}

bool HierarchyTreePlatformNode::SaveLocalization(YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }

	// YuriCoder, 2013/04/23. Localization path must be absolute.
	if(this->localizationPath.IsEmpty())
	{
		return false;
	}
	this->localizationPath.MakeDirectoryPathname();

	//TODO VK: Fix FilePath::GetFrameworkPath()
	String pathname = this->localizationPath.GetRelativePathname(FilePath::GetBundleName());
	pathname.replace(0, 4, "~res:");

    platform->Set(LOCALIZATION_PATH_NODE, pathname);
    platform->Set(LOCALIZATION_LOCALE_NODE, locale);

    return true;
}

void HierarchyTreePlatformNode::SetLocalizationPath(const FilePath & localizationPath)
{
    this->localizationPath = localizationPath;
}

void HierarchyTreePlatformNode::SetLocale(const String& locale)
{
    this->locale = locale;
}

void HierarchyTreePlatformNode::ReturnTreeNodeToScene()
{
	if (!this->redoParentNode)
	{
		return;
	}
	
	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);
}

void HierarchyTreePlatformNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	if (!node)
		return;
	
	node->RemoveTreeNode(this, false, false);
	node->AddTreeNode(this, insertAfter);
}

bool HierarchyTreePlatformNode::IsAggregatorOrScreenNamePresent(const QString& candidatName)
{
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (NULL == aggregator && NULL == screen)
		{
			continue;
		}
		if(node->GetName().compare(candidatName) == 0)
		{
			return true;
		}
	}
	return false;
}

