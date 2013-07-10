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

#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Main/QtUtils.h"
#include "QtPropertyDataDavaVariant.h"
#include "../QtPropertyWidgets/QtColorLineEdit.h"

#include <QColorDialog>
#include <QPushButton>
#include <QPainter>
#include <QLineEdit>

#include "SceneEditor/EditorSettings.h"

QtPropertyDataDavaVariant::QtPropertyDataDavaVariant(const DAVA::VariantType &value)
	: curVariantValue(value)
	, iconCacheIsValid(false)
	, allowedValuesLocked(false)
{
	// set special flags
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		SetFlags(FLAG_IS_CHECKABLE | FLAG_IS_NOT_EDITABLE);
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
	case DAVA::VariantType::TYPE_BYTE_ARRAY:
		SetFlags(FLAG_IS_DISABLED);
		break;
            
    case DAVA::VariantType::TYPE_MATRIX2:
    case DAVA::VariantType::TYPE_MATRIX3:
    case DAVA::VariantType::TYPE_MATRIX4:
	case DAVA::VariantType::TYPE_AABBOX3:
        SetFlags(FLAG_IS_NOT_EDITABLE);
        break;
            
	case DAVA::VariantType::TYPE_FLOAT:
	case DAVA::VariantType::TYPE_INT32:
	case DAVA::VariantType::TYPE_INT64:
	case DAVA::VariantType::TYPE_UINT32:
	case DAVA::VariantType::TYPE_UINT64:
	case DAVA::VariantType::TYPE_STRING:
	case DAVA::VariantType::TYPE_VECTOR2:
	case DAVA::VariantType::TYPE_VECTOR3:
	case DAVA::VariantType::TYPE_VECTOR4:
    case DAVA::VariantType::TYPE_COLOR:
    case DAVA::VariantType::TYPE_FASTNAME:
	case DAVA::VariantType::TYPE_FILEPATH:
	default:
		break;
	}

	ChildsCreate();

	// ensure data is fully initialized (icons set)
	GetValueInternal();
}

QtPropertyDataDavaVariant::~QtPropertyDataDavaVariant()
{ }

const DAVA::VariantType& QtPropertyDataDavaVariant::GetVariantValue() const
{
	return curVariantValue;
}

void QtPropertyDataDavaVariant::SetVariantValue(const DAVA::VariantType& value)
{
	DVASSERT(curVariantValue.type == value.type);
	curVariantValue = value;
}

void QtPropertyDataDavaVariant::AddAllowedValue(const DAVA::VariantType& realValue, const QVariant& visibleValue /*= QVariant()*/)
{
	AllowedValue av;

	av.realValue = realValue;
	av.visibleValue = visibleValue;

	allowedValues.push_back(av);
}

void QtPropertyDataDavaVariant::ClearAllowedValues()
{
	allowedValues.clear();
}

QVariant QtPropertyDataDavaVariant::GetValueInternal()
{
	return FromDavaVariant(curVariantValue);
}

QVariant QtPropertyDataDavaVariant::GetValueAlias()
{
	QVariant ret;

	if(allowedValues.size() > 0)
	{
		for (int i = 0; i < allowedValues.size(); ++i)
		{
			if(allowedValues[i].realValue == curVariantValue)
			{
				ret = allowedValues[i].visibleValue;
				break;
			}
		}

		if(!ret.isValid())
		{
			// if we have allowed value, but current value isn't in set
			// print this value as unknown
			// 
			QString s("Unknown - ");
			s += FromDavaVariant(curVariantValue).toString();

			ret = s;
		}
	}

	return ret;
}

void QtPropertyDataDavaVariant::SetValueInternal(const QVariant &value)
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		curVariantValue.SetBool(value.toBool());
		break;
	case DAVA::VariantType::TYPE_FLOAT:
		curVariantValue.SetFloat(value.toFloat());
		break;
	case DAVA::VariantType::TYPE_INT32:
		curVariantValue.SetInt32(value.toInt());
		break;
	case DAVA::VariantType::TYPE_INT64:
		curVariantValue.SetInt64(value.toLongLong());
		break;
	case DAVA::VariantType::TYPE_UINT32:
		curVariantValue.SetUInt32(value.toUInt());
		break;
	case DAVA::VariantType::TYPE_UINT64:
		curVariantValue.SetUInt64(value.toULongLong());
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		ToKeyedArchive(value);
		break;
	case DAVA::VariantType::TYPE_STRING:
		curVariantValue.SetString(value.toString().toStdString());
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		ToMatrix2(value);
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		ToMatrix3(value);
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		ToMatrix4(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		ToVector2(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		ToVector3(value);
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		ToVector4(value);
		break;
    case DAVA::VariantType::TYPE_COLOR:
        ToColor(value);
        break;
    case DAVA::VariantType::TYPE_FASTNAME:
        curVariantValue.SetFastName(DAVA::FastName(value.toString().toStdString().c_str()));
        break;
	case DAVA::VariantType::TYPE_AABBOX3:
		ToAABBox3(value);
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		curVariantValue.SetFilePath(value.toString().toStdString());
		break;

	case DAVA::VariantType::TYPE_BYTE_ARRAY:
	default:
		break;
	}
}

void QtPropertyDataDavaVariant::ChildChanged(const QString &key, QtPropertyData *data)
{
	MeSetFromChilds(key, data);
}

void QtPropertyDataDavaVariant::ChildNeedUpdate()
{
	ChildsSetFromMe();
}

void QtPropertyDataDavaVariant::ChildsCreate()
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec = curVariantValue.AsVector2();
			ChildAdd("X", vec.x);
			ChildAdd("Y", vec.y);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec = curVariantValue.AsVector3();
			ChildAdd("X", vec.x);
			ChildAdd("Y", vec.y);
			ChildAdd("Z", vec.z);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec = curVariantValue.AsVector4();
			ChildAdd("X", vec.x);
			ChildAdd("Y", vec.y);
			ChildAdd("Z", vec.z);
			ChildAdd("W", vec.w);
		}
		break;
    case DAVA::VariantType::TYPE_COLOR:
        {
			QPushButton *colorBtn = new QPushButton(QIcon(":/QtIcons/color.png"), "");
			colorBtn->setIconSize(QSize(12, 12));
			colorBtn->setFlat(true);
			AddOW(QtPropertyOW(colorBtn));
			QObject::connect(colorBtn, SIGNAL(pressed()), this, SLOT(ColorOWPressed()));
        }
        break;
	case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box = curVariantValue.AsAABBox3();
            
            ChildAdd("min", FromVector3(box.min));
            ChildAdd("max", FromVector3(box.max));
            
            QtPropertyData* min = ChildGet("min");
            min->SetFlags(FLAG_IS_NOT_EDITABLE);
            min->ChildAdd("X", box.min.x);
            min->ChildAdd("Y", box.min.y);
            min->ChildAdd("Z", box.min.z);
            
            QtPropertyData* max = ChildGet("max");
            max->SetFlags(FLAG_IS_NOT_EDITABLE);
            max->ChildAdd("X", box.max.x);
            max->ChildAdd("Y", box.max.y);
            max->ChildAdd("Z", box.max.z);
        }
		break;
	}
}

void QtPropertyDataDavaVariant::ChildsSetFromMe()
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		{
			// No way to change whole archive
			// so don't need to re-set childs
		}
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec = curVariantValue.AsVector2();
			ChildGet("X")->SetValue(vec.x);
			ChildGet("Y")->SetValue(vec.y);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec = curVariantValue.AsVector3();
			ChildGet("X")->SetValue(vec.x);
			ChildGet("Y")->SetValue(vec.y);
			ChildGet("Z")->SetValue(vec.z);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec = curVariantValue.AsVector4();
			ChildGet("X")->SetValue(vec.x);
			ChildGet("Y")->SetValue(vec.y);
			ChildGet("Z")->SetValue(vec.z);
			ChildGet("W")->SetValue(vec.w);
		}
		break;
    case DAVA::VariantType::TYPE_COLOR:
		{
//			DAVA::Color color = curVariantValue.AsColor();
//			ChildGet("R")->SetValue(color.r);
//			ChildGet("G")->SetValue(color.g);
//			ChildGet("B")->SetValue(color.b);
//			ChildGet("A")->SetValue(color.a);
		}
        break;
	case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box = curVariantValue.AsAABBox3();
            
            QtPropertyData* min = ChildGet("min");
            min->SetValue(FromVector3(box.min));
            min->ChildGet("X")->SetValue(box.min.x);
            min->ChildGet("Y")->SetValue(box.min.y);
            min->ChildGet("Z")->SetValue(box.min.z);
            
            QtPropertyData* max = ChildGet("max");
            max->SetValue(FromVector3(box.max));
            max->ChildGet("X")->SetValue(box.max.x);
            max->ChildGet("Y")->SetValue(box.max.y);
            max->ChildGet("Z")->SetValue(box.max.z);
        }
            break;
	}
}

void QtPropertyDataDavaVariant::MeSetFromChilds(const QString &lastChangedChildKey, QtPropertyData *lastChangedChildData)
{
	switch(curVariantValue.type)
	{
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		{
			DAVA::Vector2 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();
			curVariantValue.SetVector2(vec);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		{
			DAVA::Vector3 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();
			vec.z = ChildGet("Z")->GetValue().toFloat();
			curVariantValue.SetVector3(vec);
		}
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		{
			DAVA::Vector4 vec;
			vec.x = ChildGet("X")->GetValue().toFloat();
			vec.y = ChildGet("Y")->GetValue().toFloat();
			vec.z = ChildGet("Z")->GetValue().toFloat();
			vec.w = ChildGet("W")->GetValue().toFloat();
			curVariantValue.SetVector4(vec);
		}
		break;
    case DAVA::VariantType::TYPE_COLOR:
		{
//			DAVA::Color color;
//			color.r = ChildGet("R")->GetValue().toFloat();
//			color.g = ChildGet("G")->GetValue().toFloat();
//			color.b = ChildGet("B")->GetValue().toFloat();
//			color.a = ChildGet("A")->GetValue().toFloat();
//			curVariantValue.SetColor(color);
		}
        break;
        case DAVA::VariantType::TYPE_AABBOX3:
        {
            DAVA::AABBox3 box;
            
            QtPropertyData* min = ChildGet("min");
            box.min.x = min->ChildGet("X")->GetValue().toFloat();
            box.min.y = min->ChildGet("Y")->GetValue().toFloat();
            box.min.z = min->ChildGet("Z")->GetValue().toFloat();
            
            QtPropertyData* max = ChildGet("max");
            box.max.x = max->ChildGet("X")->GetValue().toFloat();
            box.max.y = max->ChildGet("Y")->GetValue().toFloat();
            box.max.z = max->ChildGet("Z")->GetValue().toFloat();
            
            curVariantValue.SetAABBox3(box);
        }
            break;
	}
}

QVariant QtPropertyDataDavaVariant::FromDavaVariant(const DAVA::VariantType &variant)
{
	QVariant v;

	switch(variant.type)
	{
	case DAVA::VariantType::TYPE_BOOLEAN:
		v = variant.AsBool();
		break;
	case DAVA::VariantType::TYPE_FLOAT:
		v = variant.AsFloat();
		break;
	case DAVA::VariantType::TYPE_INT32:
		v = variant.AsInt32();
		break;
	case DAVA::VariantType::TYPE_INT64:
		v = variant.AsInt64();
		break;
	case DAVA::VariantType::TYPE_UINT32:
		v = variant.AsUInt32();
		break;
	case DAVA::VariantType::TYPE_UINT64:
		v = variant.AsUInt64();
		break;
	case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
		v = FromKeyedArchive(variant.AsKeyedArchive());
		break;
	case DAVA::VariantType::TYPE_STRING:
		v = variant.AsString().c_str();
		break;
	case DAVA::VariantType::TYPE_MATRIX2:
		v = FromMatrix2(variant.AsMatrix2());
		break;
	case DAVA::VariantType::TYPE_MATRIX3:
		v = FromMatrix3(variant.AsMatrix3());
		break;
	case DAVA::VariantType::TYPE_MATRIX4:
		v = FromMatrix4(variant.AsMatrix4());
		break;
	case DAVA::VariantType::TYPE_VECTOR2:
		v = FromVector2(variant.AsVector2());
		break;
	case DAVA::VariantType::TYPE_VECTOR3:
		v = FromVector3(variant.AsVector3());
		break;
	case DAVA::VariantType::TYPE_VECTOR4:
		v = FromVector4(variant.AsVector4());
		break;
	case DAVA::VariantType::TYPE_COLOR:
		v = FromColor(variant.AsColor());
		break;
	case DAVA::VariantType::TYPE_FASTNAME:
		v = QString(variant.AsFastName().c_str());
		break;
	case DAVA::VariantType::TYPE_AABBOX3:
		v = FromAABBox3(variant.AsAABBox3());
		break;
	case DAVA::VariantType::TYPE_FILEPATH:
		v = variant.AsFilePath().GetAbsolutePathname().c_str();
		break;

	case DAVA::VariantType::TYPE_BYTE_ARRAY:
	default:
		break;
	}

	return v;
}

QVariant QtPropertyDataDavaVariant::FromKeyedArchive(DAVA::KeyedArchive *archive)
{
	QVariant v;

	if(NULL != archive)
	{
		v = QString("KeyedArchive");
	}

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector4(const DAVA::Vector4 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f, %8.2f]", vector.x, vector.y, vector.z, vector.w);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector3(const DAVA::Vector3 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]", vector.x, vector.y, vector.z);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromVector2(const DAVA::Vector2 &vector)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f]", vector.x, vector.y);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix4(const DAVA::Matrix4 &matrix)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f, %8.2f]",
                          matrix._00, matrix._10, matrix._20, matrix._30,
                          matrix._01, matrix._11, matrix._21, matrix._31,
                          matrix._02, matrix._12, matrix._22, matrix._32,
                          matrix._03, matrix._13, matrix._23, matrix._33
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix3(const DAVA::Matrix3 &matrix)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]",
                          matrix._00, matrix._10, matrix._20,
                          matrix._01, matrix._11, matrix._21,
                          matrix._02, matrix._12, matrix._22
                          );

	return v;
}

QVariant QtPropertyDataDavaVariant::FromMatrix2(const DAVA::Matrix2 &matrix)
{
	QVariant v;

	v = QString().sprintf("([%8.2f, %8.2f]\n[%8.2f, %8.2f])",
		matrix._00, matrix._10,
		matrix._01, matrix._11);

	return v;
}

QVariant QtPropertyDataDavaVariant::FromColor(const DAVA::Color &color)
{
	QVariant v;
	QColor c = ColorToQColor(color);

	v = QString().sprintf("#%02x%02x%02x%02x", c.red(), c.green(), c.blue(), c.alpha());

	return v;
}


QVariant QtPropertyDataDavaVariant::FromAABBox3(const DAVA::AABBox3 &aabbox)
{
	QVariant v;

	v = QString().sprintf("[%8.2f, %8.2f, %8.2f]\n[%8.2f, %8.2f, %8.2f]",
		aabbox.min.x, aabbox.min.y, aabbox.min.z,
		aabbox.max.x, aabbox.max.y, aabbox.max.z
		);

	return v;
}

void QtPropertyDataDavaVariant::ToKeyedArchive(const QVariant &value)
{
	// No way to set whole archive
}

void QtPropertyDataDavaVariant::ToVector4(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToVector3(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToVector2(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix4(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix3(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToMatrix2(const QVariant &value)
{
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToColor(const QVariant &value)
{
	//curVariantValue.SetColor(QColorToColor(c));
	// TODO:
	// ...
}

void QtPropertyDataDavaVariant::ToAABBox3(const QVariant &value)
{
	// TODO:
	// ...
}

QWidget* QtPropertyDataDavaVariant::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option)
{
	QWidget* ret = NULL;

	// shouldn't add allowedValues during edit process
	allowedValuesLocked = true;

	// if there is allowedValues we should show them in combobox
	// user will only be able to select values from combobox
	if(allowedValues.size() > 0)
	{
		ret = CreateAllowedValuesComboBox(parent);
	}
	// check types and create our own widgets for edit
	// if we don't create - Qt will create standard editing widget for QVariant
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			ret = new QtColorLineEdit(parent);
		}
	}
    
    return ret;
}

bool QtPropertyDataDavaVariant::SetEditorDataInternal(QWidget *editor)
{
	bool ret = false;

	// if we have valueItems, so that means combobox was created
	if(allowedValues.size())
	{
		QComboBox *comboBox = dynamic_cast<QComboBox *>(editor);
		if(NULL != comboBox)
		{
			int index = -1;

			// we should set combobox current index,
			// that matches current value
			for(int i = 0; i < allowedValues.size(); ++i)
			{
				if(allowedValues[i].realValue == curVariantValue)
				{
					index = i;
					break;
				}
			}

			comboBox->setCurrentIndex(index);
			ret = true;
		}
	}
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QtColorLineEdit *colorLineEdit = (QtColorLineEdit *) editor;

			colorLineEdit->SetColor(ColorToQColor(curVariantValue.AsColor()));
			ret = true;
		}
	}

	return ret;
}

bool QtPropertyDataDavaVariant::EditorDoneInternal(QWidget *editor)
{
	bool ret = false;

	// if we have allowedValues - that means combobox was created
	if(allowedValues.size())
	{
		SetAllowedValueFromComboBox(dynamic_cast<QComboBox *>(editor));
		ret = true;
	}
	else
	{
		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QtColorLineEdit *colorLineEdit = (QtColorLineEdit *) editor;

			curVariantValue.SetColor(QColorToColor(colorLineEdit->GetColor()));
			ret = true;
		}
	}

	// reset icon cache. icon will be recreated on next icon request
	iconCacheIsValid = false;

	// allow modify valueItems list
	allowedValuesLocked = false;

	return ret;
}

void QtPropertyDataDavaVariant::ColorOWPressed()
{
	QColor c = QColorDialog::getColor(ColorToQColor(curVariantValue.AsColor()), NULL, "Select color", QColorDialog::ShowAlphaChannel);
	if(c.isValid())
	{
		curVariantValue.SetColor(QColorToColor(c));
		ParentUpdate();

		iconCacheIsValid = false;
	}
}

QIcon QtPropertyDataDavaVariant::GetIcon()
{
	if(!iconCacheIsValid)
	{
		iconCacheIsValid = true;

		if(curVariantValue.type == DAVA::VariantType::TYPE_COLOR)
		{
			QPixmap pix(16,16);
			QPainter p(&pix);
			QColor c = ColorToQColor(curVariantValue.AsColor());

			if(c.alpha() < 255)
			{
				p.setBrush(QColor(250, 250, 250));
				p.drawRect(QRect(0, 0, 15, 15));
				p.setPen(QColor(200, 200, 200));
				p.setBrush(QColor(150, 150, 150));
				p.drawRect(QRect(0, 0, 7, 7));
				p.drawRect(QRect(8, 8, 15, 15));
			}

			p.setPen(QColor(0, 0, 0));
			p.setBrush(QBrush(c));
			p.drawRect(QRect(0,0,15,15));

			iconCache = QIcon(pix);
		}
		else
		{
			iconCache = QtPropertyData::GetIcon();
		}
	}

	return iconCache;
}

void QtPropertyDataDavaVariant::SetIcon(const QIcon &icon)
{
	QtPropertyData::SetIcon(icon);
}

QComboBox* QtPropertyDataDavaVariant::CreateAllowedValuesComboBox(QWidget *parent)
{
	QComboBox *comboBox = NULL;

	if(allowedValues.size() > 0)
	{
		comboBox = new QComboBox(parent);

		for(int i = 0; i < allowedValues.size(); ++i)
		{
			QString text;

			// if we have valid representation for "visible" items,
			// we should add it into combobox
			if(allowedValues[i].visibleValue.isValid())
			{
				text = allowedValues[i].visibleValue.toString();
			}
			// if not - we will create it from dava::varianttype
			else
			{
				text = FromDavaVariant(curVariantValue).toString();
			}

			comboBox->addItem(text);
		}
	}

	return comboBox;
}

void QtPropertyDataDavaVariant::SetAllowedValueFromComboBox(QComboBox *comboBox)
{
	if(NULL != comboBox)
	{
		int index = comboBox->currentIndex();
		if(index >= 0 && index < allowedValues.size())
		{
			if(curVariantValue != allowedValues[index].realValue)
			{
				SetValue(FromDavaVariant(allowedValues[index].realValue));
			}
		}
	}
}
