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

#ifndef UIEditor_UISliderMetadata_h
#define UIEditor_UISliderMetadata_h

#include "UIControlMetadata.h"
#include "UI/UISlider.h"

namespace DAVA {

// Metadata class for DAVA text-aware controls, responsible for the localization.
class UISliderMetadata : public UIControlMetadata
{
	Q_OBJECT

    // Slider Properties
    Q_PROPERTY(float SliderValue READ GetSliderValue WRITE SetSliderValue);
	Q_PROPERTY(float SliderMinValue READ GetSliderMinValue WRITE SetSliderMinValue);
	Q_PROPERTY(float SliderMaxValue READ GetSliderMaxValue WRITE SetSliderMaxValue);
	
	// Thumb sprite and frame
	Q_PROPERTY(QString SliderThumbSprite READ GetSliderThumbSprite WRITE SetSliderThumbSprite);
    Q_PROPERTY(int SliderThumbSpriteFrame READ GetSliderThumbSpriteFrame WRITE SetSliderThumbSpriteFrame);
	
	// Min sprite, frame and drawtype
	Q_PROPERTY(QString SliderMinSprite READ GetSliderMinSprite WRITE SetSliderMinSprite);
	Q_PROPERTY(int SliderMinSpriteFrame READ GetSliderMinSpriteFrame WRITE SetSliderMinSpriteFrame);
	Q_PROPERTY(int SliderMinDrawType READ GetSliderMinDrawType WRITE SetSliderMinDrawType);
	
	// Max sprite, frame and drawtype and leftrigth stratch cap
	Q_PROPERTY(QString SliderMaxSprite READ GetSliderMaxSprite WRITE SetSliderMaxSprite);
	Q_PROPERTY(int SliderMaxSpriteFrame READ GetSliderMaxSpriteFrame WRITE SetSliderMaxSpriteFrame);
	Q_PROPERTY(int SliderMaxDrawType READ GetSliderMaxDrawType WRITE SetSliderMaxDrawType);
	  
public:
    UISliderMetadata(QObject* parent = 0);
    
protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);   
    virtual QString GetUIControlClassName() { return "UISlider"; };
	// Override Resize function
	virtual void ApplyResize(const Rect& originalRect, const Rect& newRect);
		
    // Helper to access active UI Slider.
    UISlider* GetActiveUISlider() const;
	
    // Getters/setters.
    float GetSliderValue() const;
	void SetSliderValue(float value);
	
	// Max/Min value
    float GetSliderMinValue() const;
    void SetSliderMinValue(float value);
    float GetSliderMaxValue() const;
    void SetSliderMaxValue(float value);
	
	// Slider thumb sprite and frame
	QString GetSliderThumbSprite() const;
	void SetSliderThumbSprite(QString value);
	int GetSliderThumbSpriteFrame() const;
	void SetSliderThumbSpriteFrame(int value);
	
	// Slider Min sprite, frame and drawtype
	QString GetSliderMinSprite() const;
	void SetSliderMinSprite(QString value);
	int GetSliderMinSpriteFrame() const;
	void SetSliderMinSpriteFrame(int value);
	int GetSliderMinDrawType() const;
	void SetSliderMinDrawType(int value);
	
	// Slider Max sprite, frame and drawtype
	QString GetSliderMaxSprite() const;
	void SetSliderMaxSprite(QString value);
	int GetSliderMaxSpriteFrame() const;
	void SetSliderMaxSpriteFrame(int value);
	int GetSliderMaxDrawType() const;
	void SetSliderMaxDrawType(int value);
};

};

#endif
