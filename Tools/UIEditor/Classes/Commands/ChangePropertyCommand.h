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

#ifndef __UIEditor__ChangePropertyCommand__
#define __UIEditor__ChangePropertyCommand__

#include <QString>

#include "UI/UIControl.h"

#include "CommandsController.h"

#include "BaseCommand.h"
#include "MetadataFactory.h"
#include "HierarchyTreeNode.h"
#include "PropertyGridWidgetData.h"
#include "PropertiesHelper.h"
#include "UIControlStateHelper.h"

using namespace DAVA;

// Templated Command Data to store the list of "Tree Node ID - Tree Node Values"
// for different value types.
template<typename T> class ChangePropertyCommandData
{
public:
    ChangePropertyCommandData(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID,
                              const T& treeNodePropertyValue)
    {
        this->treeNodeID = treeNodeID;
        this->treeNodePropertyValue = treeNodePropertyValue;
    }

    HierarchyTreeNode::HIERARCHYTREENODEID GetTreeNodeID() const
    {
        return this->treeNodeID;
    }
    
    const T& GetTreeNodePropertyValue() const
    {
        return treeNodePropertyValue;
    }

private:
    HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID;
    T treeNodePropertyValue;
};

// Template ChangePropertyCommand - common for all types.
template <typename Type>
class ChangePropertyCommand : public BaseCommand
{
public:
    ChangePropertyCommand(BaseMetadata* baseMetadata,
                          const PropertyGridWidgetData& propertyGridWidgetData,
                          Type value);
	virtual ~ChangePropertyCommand();

    virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();

protected:
	// Command data.
    typedef Vector<ChangePropertyCommandData<Type> > COMMANDDATAVECT;
    typedef typename COMMANDDATAVECT::iterator COMMANDDATAVECTITER;

    // Build the Command Data based on the type.
    Vector<ChangePropertyCommandData<Type> > BuildCommandData(BaseMetadata* baseMetadata);

    // Get the Metadata with Params attached for the Tree Node passed.
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID);

    // Get the property name.
    QString GetPropertyName();

	// Actually apply the property value.
	bool ApplyPropertyValue(const COMMANDDATAVECTITER& iter, Type newValue);

    // Property Grid Widget data.
    PropertyGridWidgetData propertyGridWidgetData;
    
    // Current value.
    Type curValue;

    // Current state.
	Vector<UIControl::eControlState> curStates;

	// The vector of Command Data with the initial values.
    COMMANDDATAVECT commandData;
};

// Change Property Command Helper class is needed to retain/release the pointers to the data stored when needed.
template <typename Type>
	class ChangePropertyCommandHelper
{
public:
	static void RetainBeforeStore(Type /*value*/) {};
	static void ReleaseBeforeDelete(Type /*value*/) {};
};

// Partial specialization for Font class - it has to be retained/released.
template <>
	class ChangePropertyCommandHelper<Font*>
{
public:
	static void RetainBeforeStore(Font* value) { SafeRetain(value);};
	static void ReleaseBeforeDelete(Font* value) { SafeRelease(value);};
};

template<typename Type>
    ChangePropertyCommand<Type>::ChangePropertyCommand(BaseMetadata* baseMetadata,
                                                       const PropertyGridWidgetData& propertyGridWidgetData,
                                                       Type value) :
    propertyGridWidgetData(propertyGridWidgetData)
{
    this->curValue = value;
    this->curStates = baseMetadata->GetUIControlStates();
    this->commandData = BuildCommandData(baseMetadata);
}

template<typename Type>
	ChangePropertyCommand<Type>::~ChangePropertyCommand()
{
	// Cleanup the Command Data items previously retained, if any.
	for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != this->commandData.end(); iter ++)
	{
		ChangePropertyCommandHelper<Type>::ReleaseBeforeDelete((*iter).GetTreeNodePropertyValue());
	}
}

template<typename Type>
    inline Vector<ChangePropertyCommandData<Type> > ChangePropertyCommand<Type>::BuildCommandData(BaseMetadata* baseMetadata)
{
    // Take the params and use them for building Command Data.
    const METADATAPARAMSVECT& paramsVect = baseMetadata->GetParams();
    Vector< ChangePropertyCommandData<Type> > commandData;

    int paramsCount = baseMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        const BaseMetadataParams& params = paramsVect[i] ;
        HierarchyTreeNode::HIERARCHYTREENODEID nodeID = params.GetTreeNodeID();

        Type nodeValue = PropertiesHelper::GetPropertyValue<Type>(baseMetadata, GetPropertyName(), i);
        ChangePropertyCommandHelper<Type>::RetainBeforeStore(nodeValue);

        commandData.push_back(ChangePropertyCommandData<Type>(nodeID, nodeValue));
    }
    
    return commandData;
}

template<typename Type>
    BaseMetadata* ChangePropertyCommand<Type>::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID)
{
    const HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Tree Node is NULL for Tree Node ID %i", treeNodeID);
        return NULL;
    }
    
    UIControl* uiControl = NULL;
    const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (controlNode)
    {
        uiControl = controlNode->GetUIObject();
    }
    
    BaseMetadata* baseMetadata = MetadataFactory::Instance()->GetMetadataForTreeNode(treeNode);
    if (baseMetadata == NULL)
    {
        Logger::Error("Unable to found Hierarchy Tree Node Metadata for node %i while executing Command!",
                      treeNodeID);
        return NULL;
    }
    
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(treeNodeID, uiControl));
    baseMetadata->SetupParams(params);
    
    // Restore the Metadata UI Control state.
    baseMetadata->SetUIControlStates(curStates);
    
    return baseMetadata;
}

template<typename Type>
    void ChangePropertyCommand<Type>::Execute()
{
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		bool propertySetOK = ApplyPropertyValue(iter, curValue);
        if (propertySetOK)
        {
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
            CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

template<typename Type>
	void ChangePropertyCommand<Type>::Rollback()
{
	// The previous values are stored in Command Data.
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		Type previousValue = (*iter).GetTreeNodePropertyValue();
		bool propertySetOK = ApplyPropertyValue(iter, previousValue);

        if (propertySetOK)
        {
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
            CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

template<typename Type>
	bool ChangePropertyCommand<Type>::ApplyPropertyValue(const COMMANDDATAVECTITER& iter, Type newValue)
{
	QString propertyName = GetPropertyName();
	BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter).GetTreeNodeID());

	baseMetadata->SetUIControlStates(this->curStates);
	PropertiesHelper::SetAllPropertyValues<Type>(baseMetadata, propertyName, newValue);

	// Verify whether the properties were indeed changed.
	bool isPropertyValueDiffers = false;
	Type realValue = PropertiesHelper::GetAllPropertyValues<Type>(baseMetadata, propertyName,
																  isPropertyValueDiffers);
	
	bool propertySetOK = (realValue == curValue);

	SAFE_DELETE(baseMetadata);

	return propertySetOK;
}

template<typename Type>
    QString ChangePropertyCommand<Type>::GetPropertyName()
{
    return this->propertyGridWidgetData.getProperty().name();
}

template<typename Type>
    void ChangePropertyCommand<Type>::IncrementUnsavedChanges()
{
	QString propertyName = GetPropertyName();

	COMMANDDATAVECTITER it;
	for (it = commandData.begin(); it != commandData.end(); ++it)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID = (*it).GetTreeNodeID();

		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
		if (dynamic_cast<HierarchyTreePlatformNode*>(node))
		{
			node->IncrementUnsavedChanges();
		}
		else
		{
			BaseCommand::IncrementUnsavedChanges();
		}

		if (propertyName == "Name")
		{
			HierarchyTreeNode* nodeId = HierarchyTreeController::Instance()->GetTree().GetNode((*it).GetTreeNodeID());
			HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(nodeId);
			if (aggregator)
			{
				HierarchyTreeAggregatorNode::CHILDS controls = aggregator->GetChilds();
				HierarchyTreeAggregatorNode::CHILDS::iterator iter;
				for (iter = controls.begin(); iter != controls.end(); ++iter)
				{
					HierarchyTreeScreenNode* screen = HierarchyTreeController::Instance()->GetScreenNodeForNode(*iter);
					if (screen)
					{
						screen->IncrementUnsavedChanges();
					}
				}
			}
		}
	}
}

template<typename Type>
    void ChangePropertyCommand<Type>::DecrementUnsavedChanges()
{
	QString propertyName = GetPropertyName();

	COMMANDDATAVECTITER it;
	for (it = commandData.begin(); it != commandData.end(); ++it)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID = (*it).GetTreeNodeID();

		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
		if (dynamic_cast<HierarchyTreePlatformNode*>(node))
		{
			node->DecrementUnsavedChanges();
		}
		else
		{
			BaseCommand::DecrementUnsavedChanges();
		}

		if (propertyName == "Name")
		{
			HierarchyTreeNode* nodeId = HierarchyTreeController::Instance()->GetTree().GetNode((*it).GetTreeNodeID());
			HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(nodeId);
			if (aggregator)
			{
				HierarchyTreeAggregatorNode::CHILDS controls = aggregator->GetChilds();
				HierarchyTreeAggregatorNode::CHILDS::iterator iter;
				for (iter = controls.begin(); iter != controls.end(); ++iter)
				{
					HierarchyTreeScreenNode* screen = HierarchyTreeController::Instance()->GetScreenNodeForNode(*iter);
					if (screen)
					{
						screen->DecrementUnsavedChanges();
					}
				}
			}
		}
	}
}

#endif /* defined(__UIEditor__ChangePropertyCommand__) */
