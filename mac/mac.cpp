/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2002  Mutwin Kraus (Mac Port)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <Carbon.h>
#include <CarbonEvents.h>

#include <sioux.h>

#include <string.h>
#include <assert.h>

#include "stdafx.h"
#include "scumm.h"
#include "gui.h"
#include "sound.h"
#include "cdmusic.h"
#include "mp3_cd.h"
#include "gameDetector.h"

#define SRC_WIDTH 320
#define SRC_HEIGHT 200
#define SRC_PITCH (320)

#define MS_PER_TICK	(1000.0/60.0)

int Time()
{
	UnsignedWide ms;
	
	Microseconds(&ms);
	//return(ms.lo * MS_PER_TICK);
	return(ms.lo / 1000);
}

int DEST_WIDTH, DEST_HEIGHT;
static bool shutdown;

KeyMap fKeyMap;

Rect srcRect, dstRect;

enum
{
	kNewGameCmd		= 'newG',
	kQuitCmd		= kHICommandQuit,
	kOpenGameCmd	= 'opnG',
	kSaveGameCmd	= 'savG',
	kPrefsCmd		= kHICommandPreferences,
	kAboutCmd		= 'abtG'
};

static unsigned char *CToPascal(char *str);

Boolean OptionKeyDown()
{
	GetKeys(fKeyMap);							// get info
	if (fKeyMap[1] & 4)
		return true;
	else
		return false;
}

Boolean CommandKeyDown()
{
	GetKeys(fKeyMap);							// get info
	if (fKeyMap[1] & 0x8000)
		return true;
	else
		return false;
}


Boolean ShiftKeyDown()
{
	GetKeys(fKeyMap);							// get info
	if (fKeyMap[1] & 1)
		return true;
	else
		return false;
}


// Fast macro for testing key codes.
#define KeyCode(x,y) (BitTst(&(x), (y) ^ 0x07))

Boolean IsKeyDown(unsigned short key)
{
	GetKeys(fKeyMap);							// get info
	return (KeyCode(fKeyMap, key));
}


class WndMan
{
	bool terminated;
public:
	byte *_vgabuf;
	GWorldPtr	screenBuf;
	WindowRef wPtr;
	CTabHandle	pal;
	bool newPal;
	CCrsrHandle theCrsr;
	bool fullscreen;
	StringPtr gameName;
	int scale;
	GWorldPtr 					workSrcMap, backSrcMap;
	GDHandle					thisGDevice;
	
	void init();
	
	void ChangeScaling(short scaling);
	bool handleMessage();
	void run();
	void writeToScreen();
	void setPalette(byte *ctab, int first, int num);
};

int sel;
Scumm *scumm;
ScummDebugger debugger;
Gui gui;
SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;
OSystem _system;
GameDetector detector;

WndMan wm[1];
byte veryFastMode;

void About();
void Preferences();

void Quit()
{
	QuitApplicationEventLoop();
	ExitToShell();
}

void Error(const char* msg)
{
	
	//DebugStr((const unsigned char*)msg);
	//ExitToShell();
}

const EventTypeSpec kCmdEvents[] = 
{
	{ kEventClassMouse, kEventMouseDown },
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassKeyboard, kEventRawKeyDown },
	{ kEventClassCommand, kEventProcessCommand }
};

const EventTypeSpec kWindowEvents[] =
{
	{ kEventClassWindow, kEventWindowDrawContent },
	{ kEventClassWindow, kEventWindowHandleContentClick },
	{ kEventClassWindow, kEventWindowClose }
};

pascal OSErr QuitEventHandler(const AppleEvent *theEvent, AppleEvent *theReply, SInt32 refCon)
{
	Quit();
	return(noErr);
}

static pascal OSStatus WindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData )
{
	OSStatus		result = eventNotHandledErr;
	
	if(GetEventClass(inEvent) == kEventClassWindow)
	{
		switch(GetEventKind(inEvent))
		{
			case kEventWindowDrawContent:
				wm->writeToScreen();
			break;
			
			case kEventWindowHandleContentClick:
				if(CommandKeyDown())
					scumm->_rightBtnPressed |= msClicked|msDown;
				else
					scumm->_leftBtnPressed |= msClicked|msDown;
				
				if(wm->wPtr != FrontWindow())
				{
					ActivateWindow(wm->wPtr, true);
					BringToFront(wm->wPtr);
				}
			break;
			
			case kEventWindowClose:
				Quit();
			break;
		}
	}
	return result;
}

char mapKey(char key, char code, byte mod)
{
	switch(code)
	{
		case 0x35:
			key = 27;
		break;
		
		case 0x31:
			key = 32;
		break;
		
		case 0x60:
			key = 601;
	}
	return key;
}

static pascal OSStatus EventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData )
{
	OSStatus		result = eventNotHandledErr;
	HICommand		command;
	Point			mouse;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
		sizeof( HICommand ), NULL, &command );
	
	switch(GetEventClass(inEvent))
	{
		case kEventClassCommand:
			switch(command.commandID)
			{
				case kNewGameCmd:
					
				break;
				
				case kOpenGameCmd:
					scumm->_saveLoadSlot = 0;
					scumm->_saveLoadFlag = 2;
				break;
				
				case kSaveGameCmd:
					scumm->_saveLoadSlot = 0;
					sprintf(scumm->_saveLoadName, "Quicksave %d", scumm->_saveLoadSlot);
					scumm->_saveLoadFlag = 1;
				break;
				
				case kQuitCmd:
					Quit();
				break;
				
				case kPrefsCmd:
					Preferences();
				break;
				
				case kAboutCmd:
					About();
				break;
			}
			break;
		break;
		
		case kEventClassKeyboard:
			if(GetEventKind(inEvent) == kEventRawKeyDown)
			{
				char	key;
				UInt32	mod, code;
				
				GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &code);
				GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key);
				GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &mod);
				scumm->_keyPressed = (int)mapKey(key, code, mod);
			}
		break;
		
		case kEventClassMouse:
		switch(GetEventKind(inEvent))
		{			
			case kEventMouseDown:
				WindowRef theWin;
				
				GetEventParameter(inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef),
					NULL, &theWin);
				if(theWin != FrontWindow())
				{
					ActivateWindow(theWin, true);
					BringToFront(theWin);
				}
			break;
			
			case kEventMouseUp:
				scumm->_rightBtnPressed &= ~msDown;
				scumm->_leftBtnPressed &= ~msDown;
			break;
			
			case kEventMouseMoved:				
				GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &mouse);
				Rect winRect;
				
				GetWindowBounds(wm->wPtr, kWindowContentRgn, &winRect);
				if(PtInRect(mouse, &winRect))
				{
					CGrafPtr oldPort;
					
					GetPort(&oldPort);
					SetPortWindowPort(wm->wPtr);
					GlobalToLocal(&mouse);
					scumm->mouse.x = mouse.h/wm->scale;
					scumm->mouse.y = mouse.v/wm->scale;
					
					//debug(1, "Mouse X:%i Y:%i", scumm.mouse.x, scumm.mouse.y);
				}
			break;
		}
		break;
	}
	return result;
}

pascal void DoGameLoop(EventLoopTimerRef theTimer, void *userData)
{
	scumm->mainRun();
	QuitApplicationEventLoop();
}

void WndMan::init()
{
	Rect			rectWin;
	
	scumm->_scale = scale;
	
	DEST_WIDTH = 320 * scumm->_scale;
	DEST_HEIGHT = 200 * scumm->_scale;
	
	MenuRef AppleMenu = GetMenu(1000);
	InsertMenu(AppleMenu, 0);
	SetMenuItemCommandID(AppleMenu, 1, kAboutCmd);
	MenuRef FileMenu = GetMenu(1001);
	SetMenuItemCommandID(FileMenu, 1, kNewGameCmd);
	SetMenuItemCommandID(FileMenu, 2, kOpenGameCmd);
	SetMenuItemCommandID(FileMenu, 3, kSaveGameCmd);
	SetMenuItemCommandID(FileMenu, 5, kQuitCmd);
	DeleteMenuItems(FileMenu, CountMenuItems(FileMenu)-1, 2);
	InsertMenu(FileMenu, 0);
	MenuRef windMenu;
	CreateStandardWindowMenu(0, &windMenu);
	InsertMenu(windMenu, 0);
	EnableMenuCommand(NULL, kPrefsCmd);
	DrawMenuBar();
	
	SetRect(&rectWin, 0, 0, DEST_WIDTH, DEST_HEIGHT);
	UInt32 WinAttrib = (kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute |
				kWindowInWindowMenuAttribute | kWindowStandardHandlerAttribute);
	
	if(noErr != CreateNewWindow(kDocumentWindowClass, WinAttrib, &rectWin, &wPtr))
	{
		Error("Couldn't create Window!");
	}
	
	RepositionWindow(wPtr, NULL, kWindowCenterOnMainScreen);
	
	Str255 WindowTitle = "\pScummVM";
	SetWTitle(wPtr, WindowTitle);
	
	SetPortWindowPort(wPtr);
	ShowWindow(wPtr);
	
	SetRect(&dstRect, 0, 0, DEST_WIDTH, DEST_HEIGHT);
	SetRect(&srcRect, 0, 0, SRC_WIDTH, SRC_HEIGHT);	

	InstallApplicationEventHandler(NewEventHandlerUPP(EventHandler),
				GetEventTypeCount(kCmdEvents), kCmdEvents, 0, NULL);
	InstallStandardEventHandler(GetWindowEventTarget(wPtr));
	InstallWindowEventHandler(wPtr, NewEventHandlerUPP(WindowEventHandler),
				GetEventTypeCount(kWindowEvents), kWindowEvents, 0, NULL);
	
	OSStatus err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitEventHandler), 0L, false);
	
	EventLoopTimerRef theTimer;
	InstallEventLoopTimer(GetCurrentEventLoop(), 0, 0, NewEventLoopTimerUPP(DoGameLoop),
						NULL, &theTimer);
	
	NewGWorldFromPtr(&screenBuf, 8, &srcRect, pal, nil, 0, (char *)_vgabuf, SRC_WIDTH);
}

void WndMan::ChangeScaling(short scaling)
{
	scumm->_scale = scaling;
	scale = scaling;
	
	Rect rectWin;
	
	DEST_WIDTH = 320 * scumm->_scale;
	DEST_HEIGHT = 200 * scumm->_scale;
	
	SetRect(&rectWin, 0, 0, DEST_WIDTH, DEST_HEIGHT);
	
	SetWindowBounds(wPtr, kWindowContentRgn, &rectWin);
	RepositionWindow(wPtr, NULL, kWindowCenterOnMainScreen);
	dstRect = rectWin;
}

bool WndMan::handleMessage()
{
	EventRef theEvent;
	EventTargetRef theTarget;
	OSStatus	theErr;
	
	theTarget = GetEventDispatcherTarget();
    theErr = ReceiveNextEvent(GetEventTypeCount(kCmdEvents), kCmdEvents, kEventDurationNoWait,true, &theEvent);
    if(theErr == noErr && theEvent != NULL)
	{
		SendEventToEventTarget (theEvent, theTarget);
		ReleaseEvent(theEvent);
	}
}

void WndMan::run()
{
}

void WndMan::writeToScreen()
{
	//NewGWorldFromPtr(&screenBuf, 8, &srcRect, pal, nil, 0, (char *)_vgabuf, SRC_WIDTH);
	CopyBits(GetPortBitMapForCopyBits(screenBuf),
			GetPortBitMapForCopyBits(GetWindowPort(wPtr)), 
			&srcRect, &dstRect, srcCopy, 0L);
}

void waitForTimer(Scumm *s, int delay)
{
	uint32 start_time;
	
	if(s->_fastMode&2)
		delay = 0;
	else if(s->_fastMode&1)
		delay = 10;
	
	start_time = Time();
	do {
		wm->handleMessage();
		if(Time() >= start_time + delay)
			break;
	} while (1);
}

void WndMan::setPalette(byte *ctab, int first, int num)
{
	pal = (CTabHandle)NewHandleClear(sizeof(ColorTable) + 255 * sizeof(ColorSpec));
	(*pal)->ctSeed = TickCount();
	(*pal)->ctFlags = 0;
	(*pal)->ctSize = 256;
	for(int i = 0; i < 256; i++, ctab +=3)
	{
		(*pal)->ctTable[i].value = i;
		(*pal)->ctTable[i].rgb.red = ctab[0]<<8;
		(*pal)->ctTable[i].rgb.green = ctab[1]<<8;
		(*pal)->ctTable[i].rgb.blue = ctab[2]<<8;
	}
	NewGWorldFromPtr(&screenBuf, 8, &srcRect, pal, nil, 0, (char *)_vgabuf, SRC_WIDTH);
}

void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h)
{
	byte *dst;
	int i;

	dst = (byte*)wm->_vgabuf + y*320 + x;

	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);
}

void updateScreen(Scumm *s)
{
	if (s->_palDirtyMax != -1) {
		wm->setPalette(s->_currentPalette, 0, 256);	
		s->_palDirtyMax = -1;
	}

	wm->writeToScreen();
}


void initGraphics(Scumm *s, bool fullScreen, unsigned int scaleFactor) {
	s->_scale = scaleFactor;
	wm->init();
}

static unsigned char *CToPascal(char *str)
{
	register char *p,*q;
	register long len;

	len = strlen(str);
	if (len > 255) len = 255;
	p = str + len;
	q = p-1;
	while (p != str) *p-- = *q--;
	*str = len;
	return((unsigned char *)str);
}

static char *PascalToC(unsigned char *str)
{
	register unsigned char *p,*q,*end;

	end = str + *str;
	q = (p=str) + 1;
	while (p < end) *p++ = *q++;
	*p = '\0';
	return((char *)str);
}

ControlRef    popUpControlRef, checkBoxControlRef;
char 			*gameTitle;

void LaunchGame(int id)
{
	switch(id)
	{
		case 6:
			gameTitle = "indy3";
		break;
		
		case 7:
			gameTitle = "zak256";
		break;
		
		case 8:
			gameTitle = "loom";
		break;
		
		case 9:
			gameTitle = "monkeyEGA";
		break;
		
		case 10:
			gameTitle = "loomcd";
		break;
		
		case 11:
			gameTitle = "monkey";
		break;
		
		case 12:
			gameTitle = "monkey2";
		break;
		
		case 13:
			gameTitle = "atlantis";
		break;
		
		case 14:
			gameTitle = "playfate";
		break;
		
		case 15:
			gameTitle = "tentacle";
		break;
		
		case 16:
			gameTitle = "dottdemo";
		break;
		
		case 17:
			gameTitle = "samnmax";
		break;
		
		case 18:
			gameTitle = "snmdemo";
		break;
		
		case 19:
			gameTitle = "ft";
		break;
		
		case 20:
			gameTitle = "dig";
	}
}

OSStatus dialogEventHandler(EventHandlerCallRef eventHandlerCallRef,EventRef eventRef,
                             void *userData)
{
  OSStatus   result = eventNotHandledErr;
  UInt32     eventClass;
  UInt32     eventKind;
  ControlRef controlRef;
  ControlID  controlID;

  eventClass = GetEventClass(eventRef);
  eventKind  = GetEventKind(eventRef);

  if(eventClass == kEventClassControl)
  {
    if(eventKind == kEventControlHit)
    {
      GetEventParameter(eventRef,kEventParamDirectObject,typeControlRef,NULL,
                        sizeof(ControlRef),NULL,&controlRef);

      GetControlID(controlRef,&controlID);
      if(controlID.id == 'okay')
      {
        QuitAppModalLoopForWindow((WindowRef)userData);
        LaunchGame(GetControlValue(popUpControlRef));
        if((Boolean)GetControlValue(checkBoxControlRef))
        	wm->scale = 2;
        else
        	wm->scale = 3;
        
        DisposeWindow((WindowRef)userData);
        result = noErr;
      }
      else if(controlID.id == 'cncl')
      {
      	QuitAppModalLoopForWindow((WindowRef)userData);
      	DisposeWindow((WindowRef)userData);
      	ExitToShell();
      }
    }
  }
  return result;
}

void SelectGame()
{
	WindowRef 	aboutWin;
	OSStatus      osError = noErr;
	Rect          rect = { 0,0,120,350 };
	Rect          pushButtonRect = { 75,250,96,330 };
	Rect		popupRect = { 10, 10, 26, 330 };
	ControlID	controlID;
	ControlRef	controlRef;
	Rect		checkboxRect = { 36, 10, 50, 80 };
	EventTypeSpec dialogEvents[] = { kEventClassControl, kEventControlHit };
  	
	osError = CreateNewWindow(kMovableModalWindowClass,kWindowStandardHandlerAttribute,&rect, &aboutWin);
	RepositionWindow(aboutWin,FrontWindow(),kWindowAlertPositionOnMainScreen);
	SetThemeWindowBackground(aboutWin,kThemeBrushDialogBackgroundActive,false);
	CreateRootControl(aboutWin,&controlRef);

    CreatePushButtonControl(aboutWin,&pushButtonRect,CFSTR("OK"),&controlRef);
    SetWindowDefaultButton(aboutWin,controlRef);
    controlID.id = 'okay';
    SetControlID(controlRef,&controlID);
    AutoEmbedControl(controlRef,aboutWin);
    
    OffsetRect(&pushButtonRect, -100, 0);
    CreatePushButtonControl(aboutWin,&pushButtonRect,CFSTR("Cancel"),&controlRef);
    SetWindowCancelButton(aboutWin,controlRef);
    controlID.id = 'cncl';
    SetControlID(controlRef,&controlID);
    AutoEmbedControl(controlRef,aboutWin);
    
    CreateCheckBoxControl(aboutWin,&checkboxRect, CFSTR("Scaling"), 1,
    				true, &checkBoxControlRef);
    AutoEmbedControl(checkBoxControlRef,aboutWin);
    
    CreatePopupButtonControl(aboutWin, &popupRect, CFSTR("Game: "), 999, false, -1, 0, NULL, &popUpControlRef);
    SetWindowDefaultButton(aboutWin,popUpControlRef);
    controlID.id = 'game';
    SetControlID(popUpControlRef,&controlID);
    
	AutoEmbedControl(controlRef,aboutWin);
	
    InstallWindowEventHandler(aboutWin, NewEventHandlerUPP((EventHandlerProcPtr) dialogEventHandler),
                              GetEventTypeCount(dialogEvents),dialogEvents,aboutWin,NULL);
    ShowWindow(aboutWin);
    osError = RunAppModalLoopForWindow(aboutWin);
}

static void DrawCenteredStringAt(Str255 theString, short yLocation)
{
	Rect	portRect;
	CGrafPtr thePort = GetQDGlobalsThePort();

	GetPortBounds(thePort, &portRect);
	
	MoveTo(portRect.left + ((portRect.right-portRect.left) >> 1) -
							(StringWidth(theString) >> 1), yLocation);
	DrawString(theString);
	
}

void DrawAboutText(WindowRef win)
{
	CGrafPtr	oldPort;
	
	GetPort(&oldPort);
	SetPortWindowPort(win);
	
	TextFont(systemFont);
	TextSize(12);
	
	DrawCenteredStringAt("\pAbout ScummVMÉ", 32);
	
	TextFont(applFont);
	TextSize(9);
	
	DrawCenteredStringAt("\pScummVM", 50);
	DrawCenteredStringAt("\pLet's you run all your favourite Scumm Games", 65);
	DrawCenteredStringAt("\pon MacOS 9 and X", 80);
	
	SetPort(oldPort);
}

OSStatus aboutEventHandler(EventHandlerCallRef eventHandlerCallRef,EventRef eventRef,
                             void *userData)
{
  OSStatus   result = eventNotHandledErr;
  UInt32     eventClass;
  UInt32     eventKind;
  ControlRef controlRef;
  ControlID  controlID;

  eventClass = GetEventClass(eventRef);
  eventKind  = GetEventKind(eventRef);

  if(eventClass == kEventClassControl)
  {
    if(eventKind == kEventControlHit)
    {
      GetEventParameter(eventRef,kEventParamDirectObject,typeControlRef,NULL,
                        sizeof(ControlRef),NULL,&controlRef);

      GetControlID(controlRef,&controlID);
      if(controlID.id == 'okay')
      {
        QuitAppModalLoopForWindow((WindowRef)userData);
        DisposeWindow((WindowRef)userData);
        result = noErr;
      }
    }
  }
  else if(eventClass == kEventClassWindow)
  {
  	if(eventKind == kEventWindowUpdate)
  	{
  		DrawAboutText((WindowRef)userData);
  	}
  }
  return result;
}

void About()
{
	WindowRef	aboutWin;
	OSStatus      osError = noErr;
	Rect          rect = { 0,0,150,300 };
	Rect          pushButtonRect = { 125,125,145,175 };
	ControlID	controlID;
	ControlRef	controlRef;
	EventTypeSpec dialogEvents[] = { { kEventClassControl, kEventControlHit },
									 { kEventClassWindow, kEventWindowUpdate} };
  	
	osError = CreateNewWindow(kMovableModalWindowClass,kWindowStandardHandlerAttribute,&rect, &aboutWin);
	SetWTitle(aboutWin, "\pAbout ScummVMÉ");
	RepositionWindow(aboutWin,FrontWindow(),kWindowAlertPositionOnMainScreen);
	SetThemeWindowBackground(aboutWin,kThemeBrushDialogBackgroundActive,false);
	CreateRootControl(aboutWin,&controlRef);
	
	DrawAboutText(aboutWin);
	
    CreatePushButtonControl(aboutWin,&pushButtonRect,CFSTR("OK"),&controlRef);
    SetWindowDefaultButton(aboutWin,controlRef);
    controlID.id = 'okay';
    SetControlID(controlRef,&controlID);
    AutoEmbedControl(controlRef,aboutWin);
    
    InstallWindowEventHandler(aboutWin, NewEventHandlerUPP((EventHandlerProcPtr) aboutEventHandler),
                              GetEventTypeCount(dialogEvents),dialogEvents,aboutWin,NULL);
    ShowWindow(aboutWin);
    osError = RunAppModalLoopForWindow(aboutWin);
}

ControlRef radioGroupRef, musicVolumeSlider, masterVolumeSlider;

OSStatus prefsEventHandler(EventHandlerCallRef eventHandlerCallRef,EventRef eventRef,
                             void *userData)
{
  OSStatus   result = eventNotHandledErr;
  UInt32     eventClass;
  UInt32     eventKind;
  ControlRef controlRef;
  ControlID  controlID;

  eventClass = GetEventClass(eventRef);
  eventKind  = GetEventKind(eventRef);

  if(eventClass == kEventClassControl)
  {
    if(eventKind == kEventControlHit)
    {
      GetEventParameter(eventRef,kEventParamDirectObject,typeControlRef,NULL,
                        sizeof(ControlRef),NULL,&controlRef);

      GetControlID(controlRef,&controlID);
      if(controlID.id == 'okay')
      {
        scumm->_noSubtitles = (Boolean)!GetControlValue(checkBoxControlRef);
        short scale = GetControlValue(radioGroupRef);
        if(scale != scumm->_scale)
        	wm->ChangeScaling(scale);
        short music_vol = GetControlValue(musicVolumeSlider);
        if(music_vol != sound.get_music_volume())
        	sound.set_music_volume(music_vol);
        short master_vol = GetControlValue(masterVolumeSlider);
        if(master_vol != sound.get_master_volume())
        	sound.set_master_volume(master_vol);
        QuitAppModalLoopForWindow((WindowRef)userData);
        DisposeWindow((WindowRef)userData);
        result = noErr;
      }
    }
  }
}

void Preferences()
{
	WindowRef	prefsWin;
	OSStatus    osError = noErr;
	Rect        rect = { 0,0,210,300 };
	Rect        okButtonRect;
	ControlID	controlID;
	ControlRef	controlRef;
	EventTypeSpec dialogEvents[] = { kEventClassControl, kEventControlHit };
  	
	osError = CreateNewWindow(kMovableModalWindowClass,kWindowStandardHandlerAttribute,&rect, &prefsWin);
	SetWTitle(prefsWin, "\pPreferences");
	RepositionWindow(prefsWin,FrontWindow(),kWindowAlertPositionOnMainScreen);
	SetThemeWindowBackground(prefsWin,kThemeBrushDialogBackgroundActive,false);
	CreateRootControl(prefsWin,&controlRef);
	
	SetRect(&rect, 5, 5, 150, 21);
	
	CreateStaticTextControl(prefsWin, &rect, CFSTR("ScummVM Preferences"), NULL, &controlRef);
	AutoEmbedControl(controlRef, prefsWin);
	
	SetRect(&okButtonRect, 225, 180, 295, 200);
	
    CreatePushButtonControl(prefsWin,&okButtonRect,CFSTR("OK"),&controlRef);
    SetWindowDefaultButton(prefsWin,controlRef);
    controlID.id = 'okay';
    SetControlID(controlRef,&controlID);
    AutoEmbedControl(controlRef,prefsWin);
    
    SetRect(&rect, 150, 35, 260, 51);
    
    CreateCheckBoxControl(prefsWin,&rect, CFSTR("Subtitles"), 1, true, &checkBoxControlRef);
    AutoEmbedControl(checkBoxControlRef, prefsWin);
    
    if(scumm->_noSubtitles)
    	SetControlValue(checkBoxControlRef, false);
    
    OffsetRect(&rect, 0, 20);
    
    CreateCheckBoxControl(prefsWin,&rect, CFSTR("Fullscreen"), 0, true, &controlRef);
    AutoEmbedControl(controlRef, prefsWin);
    DeactivateControl(controlRef);
    
    Rect RadioGroupRect;
    SetRect(&RadioGroupRect, 5, 35, 120, 100);
    CreateRadioGroupControl(prefsWin, &RadioGroupRect, &radioGroupRef);
    AutoEmbedControl(radioGroupRef, prefsWin);
    
    ControlRef radioButton;
    
    Rect RadioButtonRect;	
    SetRect(&RadioButtonRect, 5, 35, 120, 51);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 1x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    	
    OffsetRect(&RadioButtonRect, 0, 20);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 2x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    
    OffsetRect(&RadioButtonRect, 0, 20);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 3x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    
    SetControlValue(radioGroupRef, scumm->_scale);
    
    SetRect(&rect, 5, 110, 175, 146);
    
    CreateSliderControl(prefsWin, &rect, sound.get_music_volume(), 1, 100,
    		kControlSliderPointsDownOrRight, 10, false, NULL, &musicVolumeSlider);
    AutoEmbedControl(musicVolumeSlider, prefsWin);
    
    OffsetRect(&rect, 0, 36);
    
    CreateSliderControl(prefsWin, &rect, sound.get_master_volume(), 1, 100,
    		kControlSliderPointsDownOrRight, 10, false, NULL, &masterVolumeSlider);
    AutoEmbedControl(masterVolumeSlider, prefsWin);
    
    OffsetRect(&rect, 180, -36);
    
    CreateStaticTextControl(prefsWin, &rect, CFSTR("Music Volume"), NULL, &controlRef);
	AutoEmbedControl(controlRef, prefsWin);
	
	OffsetRect(&rect, 0, 36);
    
    CreateStaticTextControl(prefsWin, &rect, CFSTR("Master Volume"), NULL, &controlRef);
	AutoEmbedControl(controlRef, prefsWin);
    
    InstallWindowEventHandler(prefsWin, NewEventHandlerUPP((EventHandlerProcPtr) prefsEventHandler),
                              GetEventTypeCount(dialogEvents),dialogEvents,prefsWin,NULL);
    ShowWindow(prefsWin);
    osError = RunAppModalLoopForWindow(prefsWin);
}

/* FIXME: CD Music Stubs */
void cd_playtrack(int track, int offset, int delay) {;}
void cd_play(Scumm *s, int track, int num_loops, int start_frame, int end_frame) {;}
void cd_stop() {;}
int cd_is_running() {return 0;}

void launcherLoop() {
  /* No launcher an Mac yet, probably there won't ever be one, as */
  /* there is a nice Mac-like Launcher already. */
}

void BoxTest(int num) {
}

void setShakePos(Scumm *s, int shake_pos) {}

void InitMacStuff()
{
	InitCursor();
	
	SIOUXSettings.autocloseonquit    = true;
    SIOUXSettings.asktosaveonclose    = false;
    SIOUXSettings.showstatusline    = false;
    SIOUXSettings.fontsize            = 9;
    GetFNum("\pMonaco",&SIOUXSettings.fontid);
   	SIOUXSettings.standalone        = false;
   	SIOUXSettings.setupmenus		= false;
   	SIOUXSettings.toppixel			= 40;
   	SIOUXSettings.leftpixel			= 5;
}

void InitScummStuff()
{
	detector.detectMain(2, &gameTitle);
	
	if(detector._features & GF_OLD256)
		scumm = new Scumm_v3;
	else
	if(detector._features & GF_SMALL_HEADER) // this force loomCD as v4
		scumm = new Scumm_v4;
	else
	if(detector._features & GF_AFTER_V7)
		scumm = new Scumm_v7;
	else
	if(detector._features & GF_AFTER_V6) // this force SamnmaxCD as v6
		scumm = new Scumm_v6;
	else
		scumm = new Scumm_v5;
	
	scumm->_fullScreen = detector._fullScreen;
	scumm->_debugMode = detector._debugMode;
	scumm->_bootParam = detector._bootParam;
	scumm->_scale = detector._scale;
	scumm->_gameDataPath = detector._gameDataPath;
	scumm->_gameTempo = detector._gameTempo;
	scumm->_soundEngine = detector._soundEngine;
	scumm->_videoMode = detector._videoMode;
	scumm->_exe_name = detector._exe_name;
	scumm->_gameId = detector._gameId;
	scumm->_gameText = detector._gameText;
	scumm->_features = detector._features;
	scumm->_soundCardType = detector._soundCardType;
	scumm->_noSubtitles = detector._noSubtitles;
	scumm->_midi_driver = detector._midi_driver;
	scumm->_cdrom = detector._cdrom;
	
	scumm->delta=6;
	scumm->_gui = &gui;	
	sound.initialize(scumm,&snd_driv);
	
	scumm->delta=0;
	scumm->_system = &_system;	
}

void setWindowName(Scumm *scumm)
{
	char buf[512], *gameName;
	
	sprintf(buf, "ScummVM - %s", gameName = detector.getGameName());
	free(gameName);
	StringPtr gameText = CToPascal(buf);
	SetWTitle(wm->wPtr, gameText);
}

void main(void)
{
	InitMacStuff();
	SelectGame();
	
//	initGraphics(&scumm, wm->fullscreen, wm->scale);
	
	wm->_vgabuf = (byte*)calloc(320,200);
	
	InitScummStuff();
	
	scumm->launch();
	
	gui.init(scumm);
	
	setWindowName(scumm);
	
	RunApplicationEventLoop();
	
	return;	
}

OSystem::OSystem()
{
	last_time = Time();
}

int OSystem::waitTick(int delta)
{
	do
	{
		updateScreen(scumm);
		new_time = Time();
		waitForTimer(scumm, delta * 15 + last_time - new_time);
		last_time = Time();
		if(gui._active)
		{
			gui.loop(scumm);
			delta = 5;
		}
	} while(gui._active);
	
	return(delta);
}