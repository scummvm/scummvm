#include "StdAfx.h"
#include "ConsoleWindow.h"
#include "mfc\PopupMenu.h"
#include "Serialization\Serialization.h"

#include <string.h>
#include <commctrl.h>
#include <winuser.h>
#include <richedit.h>

ConsoleWindow::ConsoleWindow()
: wnd_(0)
, edit_(0)
, defaultFont_(0)
, boldFont_(0)
, richEdModule_(0)
, popupMenu_(new PopupMenu(100))
, console_(0)
, numMessagesInEdit_(0)
{
	if(instances().empty()){
		registerWindowClass(0);
        richEdModule_ = LoadLibrary("Riched20.dll");
	}
    instances().push_back(this);
}

ConsoleWindow::~ConsoleWindow()
{
    Instances::iterator it = std::find(instances().begin(), instances().end(), this);
    xassert(it != instances().end());
    instances().erase(it);

	if(console_){
		console_->unregisterListener(this);
		console_ = 0;
	}

    if(instances().empty())
        FreeLibrary(richEdModule_);
}

void ConsoleWindow::createFonts()
{
	LOGFONT lf; 
	strcpy(lf.lfFaceName,"Lucida Console");
	lf.lfHeight	= -12;
	lf.lfWidth = 0;
	lf.lfEscapement	= 0;
	lf.lfOrientation = 0;
	lf.lfWeight	= FW_NORMAL;
	lf.lfItalic	= 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = RUSSIAN_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	defaultFont_ = CreateFontIndirect(&lf); 

	lf.lfWeight = FW_BOLD;
	boldFont_ = CreateFontIndirect(&lf); 
}

void ConsoleWindow::freeFonts()
{
    DeleteObject(defaultFont_);
    DeleteObject(boldFont_);
}

int ConsoleWindow::onCreate()
{
    createFonts();
    createEdit();
    return 0;
}

bool ConsoleWindow::create(HINSTANCE instance)
{
    xassert(wnd_ == 0);
	std::string title = console_->moduleName();
	title += " Console";


	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int consoleWidth = screenWidth * 2/ 3;
	int consoleHeight = screenHeight / 3;

	HWND wnd = ::CreateWindowEx(0,
		className(),
		title.c_str(),
		WS_OVERLAPPEDWINDOW,
		screenWidth - consoleWidth,
		screenHeight - consoleHeight,
		consoleWidth,
		consoleHeight,
		0,
		0,
		instance,
		0);
	xassert(wnd == wnd_);
	return bool(wnd_); 
}

void ConsoleWindow::onPaint(HDC dc)
{
    InvalidateRect(edit_, 0, FALSE);
}


void ConsoleWindow::onDestroy()
{
    if(::IsWindow(edit_))
        DestroyWindow(edit_);
    freeFonts();
}

void ConsoleWindow::onNotify(int id, NMHDR* nmhdr)
{
	if(id == 0){
		MSGFILTER* msgFilter = (MSGFILTER*)(nmhdr);
		if(msgFilter->nmhdr.hwndFrom == edit_){
			if(msgFilter->msg == WM_RBUTTONDOWN){
				LPARAM lParam = msgFilter->lParam;
				WPARAM wParam = msgFilter->wParam;
				onRButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);
				//onRButtonDown(LOWORD(msgFilter->wParam
			}
		}
	}
}

bool ConsoleWindow::onClose()
{
//    console().Show(false);
	show(false);
	return false;
}

void ConsoleWindow::onSize()
{
    RECT clientRect;
    GetClientRect(wnd_, &clientRect);
    MoveWindow(edit_, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom, TRUE);
}

int ConsoleWindow::onCommand(WPARAM wParam, LPARAM lParam)
{
    if(HIWORD(wParam) == 0){ // menu
        popupMenu_->onCommand(wParam, lParam);
    }
	return 0;
}

LRESULT CALLBACK ConsoleWindow::WindowProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	xassert(!instances().empty());

    ConsoleWindow* window = 0;
			
    for(Instances::iterator it = instances().begin(); it != instances().end(); ++it)
        if((*it)->wnd_ == wnd || ((*it)->wnd_ == 0 && message == WM_CREATE)){
            window = *it;
            break;
        }

	if(window)
	switch(message){
	case WM_CREATE:
		window->wnd_ = wnd;
        return window->onCreate();
	case WM_COMMAND:
		return window->onCommand(wParam, lParam);
	case WM_NOTIFY:
		window->onNotify((int)(wParam), (LPNMHDR)(lParam));
        return 0;
	case WM_SIZE:
		window->onSize();
        return 0;
	case WM_RBUTTONDOWN:
		window->onRButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);
		return 0;
	case WM_DESTROY:
		if(window)
			window->onDestroy();
        return 0;
	case WM_CLOSE:
		if(window && window->onClose())
			return DefWindowProc(wnd, message, wParam, lParam);
		else
			return 0;
	}
	return DefWindowProc(wnd, message, wParam, lParam);
}

bool ConsoleWindow::createEdit()
{
	RECT clientRect; 
	GetClientRect(wnd_, &clientRect); 
	edit_ = CreateWindowEx(0, RICHEDIT_CLASS, 0,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |  ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 
		0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,  wnd_, 0, 0, 0);
	SendMessage(edit_, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS);
	DWORD eventMask = SendMessage(edit_, EM_GETEVENTMASK, 0, 0);
    SendMessage(edit_, WM_SETFONT, (WPARAM)defaultFont_, 0);
	return bool(edit_);
}

bool ConsoleWindow::isVisible() const
{
	return ::IsWindowVisible(wnd_);
}

void ConsoleWindow::show(bool show)
{
    if(show)
        ShowWindow(wnd_, SW_SHOWNOACTIVATE);
    else
        ShowWindow(wnd_, SW_HIDE);

	options_.visible_ = show;

	UpdateWindow(wnd_); 
}

void ConsoleWindow::printMessageToEdit(const Console::Message& message)
{
	XBuffer str;
	str < ">>> " < message.message();
    if(!message.author())
        str < " (group: " < message.author() < ")";
    str < "\n";
	GETTEXTLENGTHEX tex;
	tex.codepage = 0;
	tex.flags = 0;
	SendMessage(edit_, EM_SETSEL, -1, -1);
	int s = (int)SendMessage(edit_, EM_GETTEXTLENGTHEX, (WPARAM)(&tex), 0);
	SendMessage(edit_, EM_REPLACESEL, 0, (LPARAM) str.buffer()); 
	SendMessage(edit_, EM_SETSEL, s, s+str.size());
	COLORREF color;
	if (message.level() == Console::LEVEL_WARNING)
		color = RGB(255,102,0);
	else if (message.level() == Console::LEVEL_ERROR)
		color = RGB(255,0,0);
	else
		color = RGB(0,0,0);
	CHARFORMAT format;
	memset(&format,0,sizeof(CHARFORMAT));
	format.cbSize = sizeof(CHARFORMAT);
	format.dwMask = CFM_COLOR;
	format.crTextColor = color;
	SendMessage(edit_, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)(&format)); 
	SendMessage(edit_, EM_SETSEL, -1, -1);
}

void ConsoleWindow::addGroup(const char* groupName)
{
    Groups::iterator it = std::find(groups_.begin(), groups_.end(), groupName);
    if(it == groups_.end())
        groups_.push_back(groupName);
}

void ConsoleWindow::syncEditToConsole()
{
    assert(console_);
    const Console::Messages& messages = console_->messages();
    if(messages.size() > numMessagesInEdit_){
        Console::Messages::const_iterator it = messages.begin();
        std::advance(it, numMessagesInEdit_);
        for(; it != messages.end(); ++it){
            const Console::Message& message = *it;

			addGroup(message.author());

            if(isMessageVisible(message))
                printMessageToEdit(message);
        }
    }
	numMessagesInEdit_ = messages.size();
}

bool ConsoleWindow::isMessageVisible(const Console::Message& msg) const
{
	return !isGroupHidden(msg.author()) && showLevel() <= msg.level();
}

void ConsoleWindow::writeMessage(const Console::Message& msg)
{
	if(msg.level() >= Console::LEVEL_ERROR)
		show(true);
    assert(console_);

	//LockWindowUpdate(edit_);
    syncEditToConsole();
	//LockWindowUpdate(0);
}

void ConsoleWindow::detach()
{
	console_ = 0;
	DestroyWindow(wnd_);
}

void ConsoleWindow::init(Console* console)
{
    console_ = console;
	if(!create(0))
		xassert(0);
	syncEditToConsole();
	show(true);
}

void ConsoleWindow::registerWindowClass(HINSTANCE instance)
{
	WNDCLASSEX wcx; 
	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW |CS_VREDRAW;
	wcx.lpfnWndProc = &ConsoleWindow::WindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = 0;
	wcx.hIcon = LoadIcon(0,IDI_APPLICATION);
	wcx.hCursor = LoadCursor(0,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)GetStockObject(COLOR_BTNFACE);
	wcx.lpszMenuName =  "MainMenu";
	wcx.lpszClassName = className();
	wcx.hIconSm = 0;
	if(!RegisterClassEx(&wcx))
		xassert(0 && "Unable to register class");
}

void ConsoleWindow::clearMessages()
{
	SendMessage(edit_, EM_SETSEL, 0, -1);
	SendMessage(edit_, EM_REPLACESEL, 0, (LPARAM)("")); 
	SendMessage(edit_, EM_SETSEL, -1, -1);
	numMessagesInEdit_ = 0;
}

void ConsoleWindow::refreshEdit()
{
	if(console_){
		LockWindowUpdate(edit_);
		clearMessages();
		syncEditToConsole();
		LockWindowUpdate(0);
	}
}

void ConsoleWindow::hideGroup(const char* groupName)
{
	Groups::iterator hit = std::find(hiddenGroups().begin(), hiddenGroups().end(), groupName);
	if(hit != hiddenGroups().end())
		hiddenGroups().erase(hit);
}

void ConsoleWindow::unhideGroup(const char* groupName)
{
	hiddenGroups().push_back(groupName);
}

bool ConsoleWindow::isGroupHidden(const char* groupName) const
{
    Groups::const_iterator it = std::find(hiddenGroups().begin(), hiddenGroups().end(), groupName);
    return it != hiddenGroups().end();
}

bool ConsoleWindow::allGroupsVisible() const
{
	Groups::const_iterator it;
	for(it = groups_.begin(); it != groups_.end(); ++it){
		if(isGroupHidden(it->c_str()))
			return false;
	}
	return true;
}

void ConsoleWindow::onFilterGroupAll()
{
    if(allGroupsVisible()){
        for(Groups::iterator it = groups_.begin(); it != groups_.end(); ++it)
            unhideGroup(it->c_str());
    }
    else{
        for(Groups::iterator it = groups_.begin(); it != groups_.end(); ++it)
            hideGroup(it->c_str());
    }
	refreshEdit();
}

void ConsoleWindow::onFilterGroup(std::string groupName)
{
    if(isGroupHidden(groupName.c_str()))
        hideGroup(groupName.c_str());
    else
        unhideGroup(groupName.c_str());
	refreshEdit();
}

void ConsoleWindow::setShowLevel(Console::Level level)
{
	options_.showLevel_ = int(level);

	refreshEdit();
}

Console::Level ConsoleWindow::showLevel() const
{
	return static_cast<Console::Level>(options_.showLevel_);
}

void ConsoleWindow::onRButtonDown(int x, int y, UINT button)
{
	popupMenu_->clear();
	PopupMenuItem& root = popupMenu_->root();

	PopupMenuItem& groups = root.add("Filter Groups");
    Groups::iterator it;
	if(!groups_.empty()){
		groups.add(allGroupsVisible() ? "Hide All" : "Show All")
			.connect(bindMethod(*this, &ConsoleWindow::onFilterGroupAll));
		groups.addSeparator();
		for(it = groups_.begin(); it != groups_.end(); ++it){
			const std::string& groupName = *it;
			groups.add(groupName.empty() ? "[no group]" : groupName.c_str())
				.connect(bindArgument(bindMethod(*this, &ConsoleWindow::onFilterGroup), groupName))
                .check(!isGroupHidden(groupName.c_str()));
		}
	}
	else
		groups.add("empty").enable(false);	

    PopupMenuItem& levels = root.add("Filter by Level");
	for(int i = Console::NUM_LEVELS - 1; i >= 0; --i){
		Console::Level level(static_cast<Console::Level>(i));
		const char* levelName = Console::levelText(level);
		levels.add(levelName)
			.connect(bindArgument(bindMethod(*this, &ConsoleWindow::setShowLevel), level))
			.check(showLevel() == level);
	}

	root.addSeparator();
	root.add("Hide").connect(bindArgument(bindMethod(*this, &ConsoleWindow::show), false));
	
	POINT pt;
	::GetCursorPos(&pt);
	popupMenu_->spawn(pt, wnd_);
}

void ConsoleWindow::Options::serialize(Archive& ar)
{
	ar.serialize(hiddenGroups_, "hiddenGroups", 0);
	ar.serialize(showLevel_, "showLevel", 0);
}
