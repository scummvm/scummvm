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

#include "scumm.h"
#include "sound.h"
#include "gui.h"

#define SRC_WIDTH 320
#define SRC_HEIGHT 200
#define SRC_PITCH (320)

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
	Scumm *_scumm;
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
Scumm scumm;
ScummDebugger debugger;
Gui gui;
SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;

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
	{ kEventClassCommand, kEventProcessCommand },
	{ kEventClassWindow, kEventWindowClose }
};

pascal OSErr QuitEventHandler(const AppleEvent *theEvent, AppleEvent *theReply, SInt32 refCon)
{
	Quit();
	return(noErr);
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
		case kEventClassWindow:
			WindowRef	theWin;
			GetEventParameter( inEvent, kEventParamDirectObject, typeWindowRef, NULL,
				sizeof( WindowRef ), NULL, &theWin );
			if(theWin == wm->wPtr)
				Quit();
		case kEventClassCommand:
			switch(command.commandID)
			{
				case kNewGameCmd:
					
				break;
				
				case kOpenGameCmd:
					wm->_scumm->_saveLoadSlot = 0;
					wm->_scumm->_saveLoadFlag = 2;
				break;
				
				case kSaveGameCmd:
					wm->_scumm->_saveLoadSlot = 0;
					sprintf(wm->_scumm->_saveLoadName, "Quicksave %d", wm->_scumm->_saveLoadSlot);
					wm->_scumm->_saveLoadFlag = 1;
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
				char key;
				
				GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key);
				wm->_scumm->_keyPressed = (int)key;
			}
		break;
		
		case kEventClassMouse:
		switch(GetEventKind(inEvent))
		{
			
			case kEventMouseDown:
				WindowPtr window;
				int part;
				
				GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &mouse);
				part = FindWindow(mouse, &window);
				switch(part)
				{
					case inContent:
						if((window != FrontWindow() && window == wm->wPtr))
						{
							SelectWindow(window);
							BringToFront(window);
						}
						if(window == wm->wPtr)
						{
							if(CommandKeyDown())
								wm->_scumm->_rightBtnPressed |= msClicked|msDown;
							else
								wm->_scumm->_leftBtnPressed |= msClicked|msDown;
						}
					break;
					
					case inDrag:
						BitMap	qdscreenbits;
						
						GetQDGlobalsScreenBits(&qdscreenbits);
						DragWindow(window, mouse, &qdscreenbits.bounds);
					break;
					
					case inGoAway:
						if(TrackGoAway(window, mouse) && window == wm->wPtr)
							Quit();
					break;
				}
			break;
			
			case kEventMouseUp:
				wm->_scumm->_rightBtnPressed &= ~msDown;
				wm->_scumm->_leftBtnPressed &= ~msDown;
			break;
			
			case kEventMouseMoved:
				Point mouse2;
				
				GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &mouse2);
				
				CGrafPtr oldPort;
				GetPort(&oldPort);
				SetPortWindowPort(wm->wPtr);
				Rect wRect;
				GetPortBounds(GetWindowPort(wm->wPtr), &wRect);
				if(PtInRect(mouse2, &wRect))
				{
					GlobalToLocal(&mouse2);
					
					wm->_scumm->mouse.x = mouse2.h/wm->scale;
					wm->_scumm->mouse.y = mouse2.v/wm->scale+25;
				}
				SetPort(oldPort);
			break;
		}
		break;
	}
	return result;
}

void pc_loop()
{
	int delta;
	int last_time, new_time, old_time = TickCount();
	
	delta = 0;
	do {
		updateScreen(&scumm);
		
		new_time = TickCount();
		waitForTimer(&scumm, delta * 15 + last_time - new_time);
		last_time = TickCount();
		
		sound.on_timer();
		
		if(gui._active) {
			gui.loop();
			delta = 3;
		} else {
			delta = scumm.scummLoop(delta);
		}
	} while(1);
}

pascal void DoGameLoop(EventLoopTimerRef theTimer, void *userData)
{
	pc_loop();
	QuitApplicationEventLoop();
}

void WndMan::init()
{
	Rect			rectWin;
	
	_scumm->_scale = scale;
	
	DEST_WIDTH = 320 * _scumm->_scale;
	DEST_HEIGHT = 200 * _scumm->_scale;
	
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
	UInt32 WinAttrib = (kWindowCloseBoxAttribute | kWindowFullZoomAttribute |
				kWindowCollapseBoxAttribute | kWindowInWindowMenuAttribute);
	
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
	
	//InstallStandardEventHandler(GetWindowEventTarget(wPtr));

	InstallApplicationEventHandler(NewEventHandlerUPP(EventHandler),
				GetEventTypeCount(kCmdEvents), kCmdEvents, 0, NULL);
	
	OSStatus err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitEventHandler), 0L, false);
	
	EventLoopTimerRef theTimer;
	InstallEventLoopTimer(GetCurrentEventLoop(), 0, 0, NewEventLoopTimerUPP(DoGameLoop),
						NULL, &theTimer);
}

void WndMan::ChangeScaling(short scaling)
{
	_scumm->_scale = scaling;
	scale = scaling;
	
	Rect rectWin;
	
	DEST_WIDTH = 320 * _scumm->_scale;
	DEST_HEIGHT = 200 * _scumm->_scale;
	
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
	NewGWorldFromPtr(&screenBuf, 8, &srcRect, pal, nil, 0, (char *)_vgabuf, SRC_WIDTH);
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
	
	start_time = TickCount();
	do {
		wm->handleMessage();
		if(TickCount() >= start_time + delay)
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

ControlRef radioGroupRef;

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
        wm->_scumm->_noSubtitles = (Boolean)!GetControlValue(checkBoxControlRef);
        short scale = GetControlValue(radioGroupRef);
        if(scale != wm->_scumm->_scale)
        	wm->ChangeScaling(scale);
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
	OSStatus      osError = noErr;
	Rect          rect = { 0,0,150,300 };
	Rect          pushButtonRect = { 125,125,145,175 };
	Rect		checkboxRect = { 30, 10, 45, 90 };
	ControlID	controlID;
	ControlRef	controlRef;
	EventTypeSpec dialogEvents[] = { kEventClassControl, kEventControlHit };
  	
	osError = CreateNewWindow(kMovableModalWindowClass,kWindowStandardHandlerAttribute,&rect, &prefsWin);
	SetWTitle(prefsWin, "\pPreferences");
	RepositionWindow(prefsWin,FrontWindow(),kWindowAlertPositionOnMainScreen);
	SetThemeWindowBackground(prefsWin,kThemeBrushDialogBackgroundActive,false);
	CreateRootControl(prefsWin,&controlRef);
	
    CreatePushButtonControl(prefsWin,&pushButtonRect,CFSTR("OK"),&controlRef);
    SetWindowDefaultButton(prefsWin,controlRef);
    controlID.id = 'okay';
    SetControlID(controlRef,&controlID);
    AutoEmbedControl(controlRef,prefsWin);
    
    CreateCheckBoxControl(prefsWin,&checkboxRect, CFSTR("Subtitles"), 1,
    				true, &checkBoxControlRef);
    AutoEmbedControl(checkBoxControlRef,prefsWin);
    
    if(wm->_scumm->_noSubtitles)
    	SetControlValue(checkBoxControlRef, false);
    
    Rect RadioGroupRect;
    SetRect(&RadioGroupRect, 120, 10, 290, 120);
    CreateRadioGroupControl(prefsWin, &RadioGroupRect, &radioGroupRef);
    AutoEmbedControl(radioGroupRef, prefsWin);
    
    ControlRef radioButton;
    
    Rect RadioButtonRect;	
    SetRect(&RadioButtonRect, 125, 30, 285, 45);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 1x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    	
    OffsetRect(&RadioButtonRect, 0, 20);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 2x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    
    OffsetRect(&RadioButtonRect, 0, 20);
    CreateRadioButtonControl(prefsWin, &RadioButtonRect, CFSTR("Scaling 3x"), 0, true, &radioButton);
    AutoEmbedControl(radioButton, prefsWin);
    
    SetControlValue(radioGroupRef, wm->_scumm->_scale);
    InstallWindowEventHandler(prefsWin, NewEventHandlerUPP((EventHandlerProcPtr) prefsEventHandler),
                              GetEventTypeCount(dialogEvents),dialogEvents,prefsWin,NULL);
    ShowWindow(prefsWin);
    osError = RunAppModalLoopForWindow(prefsWin);
}

/* FIXME: CD Music Stubs */
void cd_playtrack(int track, int offset, int delay) {;}
void cd_play(int track, int num_loops, int start_frame) {;}
void cd_stop() {;}
int cd_is_running() {return 0;}

void launcherLoop() {
  /* No launcher on mac yet => stub function */
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
}

void main(void)
{
	InitMacStuff();
	SelectGame();
	
//	initGraphics(&scumm, wm->fullscreen, wm->scale);
	
	wm->_vgabuf = (byte*)calloc(320,200);
	wm->_scumm = &scumm;
	
	sound.initialize(&scumm,&snd_driv);
	
	scumm._gui = &gui;
	
	scumm.scummMain(2, &gameTitle);
	
	StringPtr gameText = CToPascal(wm->_scumm->getGameName());
	SetWTitle(wm->wPtr, gameText);
	
	if (!(scumm._features & GF_SMALL_HEADER))
    	gui.init(&scumm);
    
	scumm._fastMode = true;
	
	veryFastMode = true;
	
	RunApplicationEventLoop();
	
	return;	
}