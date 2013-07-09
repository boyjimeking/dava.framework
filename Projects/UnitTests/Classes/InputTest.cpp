//
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "InputTest.h"

using namespace DAVA;

static const float INPUT_TEST_AUTO_CLOSE_TIME = 30.0f;


class UIWebViewDelegate: public IUIWebViewDelegate
{
	virtual eAction URLChanged(UIWebView* webview, const String& newURL, bool isRedirectedByMouseClick);

	virtual void PageLoaded(UIWebView* webview);
};

IUIWebViewDelegate::eAction UIWebViewDelegate::URLChanged(UIWebView* webview, const String& newURL, bool isInitiatedByUser)
{
	if(isInitiatedByUser)
	{
		DAVA::Logger::Debug("Link %s from browser", newURL.c_str());
	}
	else
	{
		DAVA::Logger::Debug("Link %s from source code", newURL.c_str());
	}

	if (newURL.find("google.com.ua") != String::npos)
	{
		return IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER;
	}
	else if (newURL.find("bash.im") != String::npos)
	{
		return IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER;
	}
	else if (newURL.find("microsoft.com") != String::npos)
	{
		return IUIWebViewDelegate::NO_PROCESS;
	}

	return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
}

void UIWebViewDelegate::PageLoaded(UIWebView* webview)
{
	webview->SetVisible(true);
}


InputTest::InputTest() :
 TestTemplate<InputTest>("InputTest")
{
	textField = NULL;
	staticText = NULL;
	testButton = NULL;
	
	onScreenTime = 0.0f;
	testFinished = false;
	
	RegisterFunction(this, &InputTest::TestFunction, Format("InputTest"), NULL);
}

void InputTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Texture* texture = Texture::CreateFromFile("~res:/TestData/InputTest/rect2.png");
	Sprite* spr = Sprite::CreateFromTexture(texture,0,0,texture->width,texture->height);

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
    font->SetColor(Color::White());
	
	textField = new UITextField(Rect(0, 0, 512, 100));
#ifdef __DAVAENGINE_IPHONE__
	Color color(1.f, 1.f, 1.f, 1.f);
	textField->SetFontColor(color);
#else
	textField->SetFont(font);
#endif
	textField->SetSprite(spr,0);
    textField->SetSpriteAlign(ALIGN_RIGHT);
	textField->SetTextAlign(ALIGN_LEFT | ALIGN_BOTTOM);
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UITextFieldDelegate());
	AddControl(textField);
	
	textField = new UITextField(Rect(600, 10, 100, 100));
#ifdef __DAVAENGINE_IPHONE__
	textField->SetFontColor(color);
#else
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UITextFieldDelegate());
	AddControl(textField);

	textField = new UITextField(Rect(750, 10, 100, 500));
#ifdef __DAVAENGINE_IPHONE__
	textField->SetFontColor(color);
#else
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UITextFieldDelegate());
	AddControl(textField);

	testButton = new UIButton(Rect(0, 300, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::ButtonPressed));

	staticText = new UIStaticText(Rect(500, 500, 100, 50));
	font->SetSize(10);
	staticText->SetFont(font);
	staticText->SetTextAlign(12);// 12 - Rtop
	staticText->SetText(L"StaticText");
	staticText->SetDebugDraw(true);
	AddControl(staticText);

	webView1 = new UIWebView(Rect(5, 105, 500, 190));
	webView1->SetVisible(false);
	delegate = new UIWebViewDelegate();
	webView1->SetDelegate((UIWebViewDelegate*)delegate);
	webView1->OpenURL("http://www.linux.org.ru");
	AddControl(webView1);

	webView2 = new UIWebView(Rect(305, 300, 440, 190));
    webView2->SetVisible(false);
    webView2->SetDelegate((UIWebViewDelegate*)delegate);
	webView2->OpenURL("http://www.apple.com");
	AddControl(webView2);

	FilePath srcDir("~res:/TestData/InputTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "InputTest/";
	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
	FilePath srcFile = srcDir + "test.html";
	FilePath cpyFile = cpyDir + "test.html";
	FileSystem::Instance()->CopyFile(srcFile, cpyFile);
	String url = "file:///" + cpyFile.GetAbsolutePathname();

	//delegate = new UIWebViewDelegate();
	webView3 = new UIWebView(Rect(520, 130, 215, 135));
	webView3->SetDelegate((UIWebViewDelegate*)delegate);
	webView3->OpenURL(url);

	AddControl(webView3);

	AddControl(testButton);
    
    SafeRelease(spr);
    SafeRelease(texture);
	
	staticText->SetShadowColor(DAVA::Color(0xFF/255.f, 0xC4/255.f, 0xC3/255.f, 1.f));
	staticText->SetShadowOffset(DAVA::Vector2(4.0f, 4.0f));
	Color faded = staticText->GetBackground()->color;
	faded.a = 0.1f;
	staticText->ColorAnimation(faded, 2.0f, Interpolation::LINEAR);
	staticText->ShadowColorAnimation(faded, 2.0f, Interpolation::LINEAR);
}

void InputTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(textField);
	SafeRelease(staticText);
	
	SafeRelease(webView1);
	SafeRelease(webView2);
	SafeRelease(webView3);
	
	UIWebViewDelegate* d = (UIWebViewDelegate*)delegate;
	delete d;
}

void InputTest::TestFunction(PerfFuncData * data)
{
	return;
}

void InputTest::DidAppear()
{
    onScreenTime = 0.f;
}

void InputTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > INPUT_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }

    TestTemplate<InputTest>::Update(timeElapsed);
}

bool InputTest::RunTest(int32 testNum)
{
	TestTemplate<InputTest>::RunTest(testNum);
	return testFinished;
}


void InputTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}

void InputTest::OnPageLoaded(DAVA::BaseObject * caller, void * param, void *callerData)
{
	UIWebView* webView = dynamic_cast<UIWebView*>(caller);
	if(NULL == webView)
	{
		return;
	}
	
	webView->SetVisible(true);
}
