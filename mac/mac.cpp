/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002  Mutwin Kraus (Mac Port) and The ScummVM Project
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
#include <sioux.h>

#include "stdafx.h"
#include "scumm.h"
#include "mididrv.h"
#include "gameDetector.h"
//#include "mp3_cd.h"
#include "gui.h"
//#include "gameDetector.h"

#define MAX(a,b) (((a)<(b)) ? (b) : (a))
#define MIN(a,b) (((a)>(b)) ? (b) : (a))

class OSystem_MAC : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void update_screen();

	// Either show or hide the mouse cursor
	bool show_mouse(bool visible);
	
	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);
	
	// Set the bitmap that's used when drawing the cursor.
	void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);
	
	// Shaking is used in SCUMM. Set current shake position.
	void set_shake_pos(int shake_pos);
		
	// Get the number of milliseconds since the program was started.
	uint32 get_msecs();
	
	// Delay for a specified amount of milliseconds
	void delay_msecs(uint msecs);
	
	// Create a thread
	void *create_thread(ThreadProc *proc, void *param);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	// Set function that generates samples 
	bool set_sound_proc(void *param, SoundProc *proc, byte sound);
		
	// Poll cdrom status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play cdrom audio track
	void play_cdrom(int track, int num_loops, int start_frame, int end_frame);

	// Stop cdrom audio track
	void stop_cdrom();

	// Update cdrom audio status
	void update_cdrom();

	// Add a new callback timer
	void set_timer(int timer, int (*callback)(int))		{ /* FIXME - TODO */ }

	// Mutex handling
	void *create_mutex(void)			{ return NULL; /* FIXME - TODO */ }
	void lock_mutex(void *mutex)		{ /* FIXME - TODO */ }
	void unlock_mutex(void *mutex)		{ /* FIXME - TODO */ }
	void delete_mutex(void *mutex)		{ /* FIXME - TODO */ }

	// Quit
	void quit();

	// Set a parameter
	uint32 property(int param, Property *value);

	static OSystem *create(int gfx_mode, bool full_screen);
	
	void sound_callback(SndChannel *chan, SndCommand *cmd_passed);
private:
	typedef void TwoXSaiProc(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
	
	GWorldPtr screenBuf;
	WindowRef wref;
	CTabHandle pal;	
	Rect blit_rect;

	enum {
		DF_WANT_RECT_OPTIM			= 1 << 0,
		DF_REAL_8BIT				= 1 << 1,
		DF_SEPARATE_TEMPSCREEN		= 1 << 2,
		DF_UPDATE_EXPAND_1_PIXEL	= 1 << 3
	};

	int _mode;
	bool _full_screen;
	bool _mouse_visible;
	bool _mouse_drawn;
	uint32 _mode_flags;
	byte _internal_scaling;

	bool force_full; //Force full redraw on next update_screen
	bool cksum_valid;

	enum {
		NUM_DIRTY_RECT = 100,

		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40,
		MAX_SCALING = 3
	};
	
	int SCREEN_WIDTH, SCREEN_HEIGHT, CKSUM_NUM;
	Rect *dirty_rect_list;
	int num_dirty_rects;
	uint32 *dirty_checksums;

	int scaling;

	/* CD Audio */
	int cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
	uint32 cd_end_time, cd_stop_time, cd_next_second;

	struct MousePos {
		int16 x,y,w,h;
	};

	byte *_ms_buf;
	byte *_ms_backup;
	MousePos _ms_cur;
	MousePos _ms_old;
	int16 _ms_hotspot_x;
	int16 _ms_hotspot_y;
	int _current_shake_pos;

	byte*	_gfx_buf;	/* Graphics memory */
	int16 *_sai_buf, *_tmp_buf;
	uint _palette_changed_first, _palette_changed_last;
	
	TwoXSaiProc *_sai_func;

	void add_dirty_rgn_auto(const byte *buf);
	void mk_checksums(const byte *buf);

	static void fill_sound(void *userdata, uint8 * stream, int len);
	
	void add_dirty_rect(int x, int y, int w, int h);

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();

	void hotswap_gfx_mode();

	void get_320x200_image(byte *buf);
	
	void init_mac_stuff();
	void set_scaling();
	void blit_to_screen();
	void update_rects();
	
	static uint32 autosave(uint32);
	
	UInt8 *buffer[2];
	CmpSoundHeader header;
	SndChannelPtr channel;
	int size;
	SoundProc *sndProc;
	void * parameter;
};

int Init_2xSaI (uint32 BitFormat);
void _2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr,
						uint32 dstPitch, int width, int height);
void Super2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void SuperEagle(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void AdvMame2x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void Normal1x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void Normal2x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void Normal3x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);

KeyMap fKeyMap;

Boolean CommandKeyDown()
{
	GetKeys(fKeyMap);							// get info
	if (fKeyMap[1] & 0x8000)
		return true;
	else
		return false;
}

static unsigned char *CToPascal(char *str) {
	register char *p,*q;
	register long len;

	len = strlen(str);
	if (len > 255) len = 255;
	p = str + len;
	q = p-1;
	while (p != str) 
	 *p-- = *q--;
	*str = len;
	return((unsigned char *)str);
}

static char *PascalToC(unsigned char *str) {
	register unsigned char *p,*q,*end;

	end = str + *str;
	q = (p=str) + 1;

	while (p < end) 
		*p++ = *q++;
	*p = '\0';

	return((char *)str);
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
	//OSystem_MAC::quit();
	return(noErr);
}

enum
{
	kNewGameCmd		= 'newG',
	kQuitCmd		= kHICommandQuit,
	kOpenGameCmd	= 'opnG',
	kSaveGameCmd	= 'savG',
	kPrefsCmd		= kHICommandPreferences,
	kAboutCmd		= 'abtG'
};

ControlRef radioGroupRef, musicVolumeSlider, masterVolumeSlider;
char *gameTitle;
ControlRef    popUpControlRef, checkBoxControlRef;

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
        /*scumm->_noSubtitles = (Boolean)!GetControlValue(checkBoxControlRef);
        short scale = GetControlValue(radioGroupRef);
        if(scale != scumm->_scale)
        	wm->ChangeScaling(scale);
        short music_vol = GetControlValue(musicVolumeSlider);
        if(music_vol != sound.get_music_volume())
        	sound.set_music_volume(music_vol);
        short master_vol = GetControlValue(masterVolumeSlider);
        if(master_vol != sound.get_master_volume())
        	sound.set_master_volume(master_vol);*/
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
    
    //if(scumm->_noSubtitles)
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
    
    //SetControlValue(radioGroupRef, scumm->_scale);
    
    SetRect(&rect, 5, 110, 175, 146);
    
    CreateSliderControl(prefsWin, &rect, 100, 1, 100,
    		kControlSliderPointsDownOrRight, 10, false, NULL, &musicVolumeSlider);
    AutoEmbedControl(musicVolumeSlider, prefsWin);
    
    OffsetRect(&rect, 0, 36);
    
    CreateSliderControl(prefsWin, &rect, 100, 1, 100,
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
			gameTitle = "monkeyVGA";
		break;
		
		case 11:
			gameTitle = "loomcd";
		break;
		
		case 12:
			gameTitle = "monkey";
		break;
		
		case 13:
			gameTitle = "monkey1";
		break;
		
		case 14:
			gameTitle = "monkey2";
		break;
		
		case 15:
			gameTitle = "atlantis";
		break;
		
		case 16:
			gameTitle = "playfate";
		break;
		
		case 17:
			gameTitle = "tentacle";
		break;
		
		case 18:
			gameTitle = "dottdemo";
		break;
		
		case 19:
			gameTitle = "samnmax";
		break;
		
		case 20:
			gameTitle = "snmdemo";
		break;
		
		case 21:
			gameTitle = "ft";
		break;
		
		case 22:
			gameTitle = "dig";
		break;
		
		case 25:
			gameTitle = "simon1dos";
		break;
		
		case 26:
			gameTitle = "simon1win";
		break;
		
		case 27:
			gameTitle = "simon2win";
		break;
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

char* SelectGame()
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
  	
	osError = CreateNewWindow(kMovableModalWindowClass,kWindowStandardHandlerAttribute,&rect, &aboutWin);
	SetWTitle(aboutWin, "\pPlease Select a GameÉ");
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
            
    CreatePopupButtonControl(aboutWin, &popupRect, CFSTR("Game: "), 999, false, -1, 0, NULL, &popUpControlRef);
    SetWindowDefaultButton(aboutWin,popUpControlRef);
    controlID.id = 'game';

    SetControlID(popUpControlRef,&controlID);
    
	AutoEmbedControl(controlRef,aboutWin);

    InstallWindowEventHandler(aboutWin, NewEventHandlerUPP((EventHandlerProcPtr) dialogEventHandler),
                              GetEventTypeCount(dialogEvents),dialogEvents,aboutWin,NULL);
    ShowWindow(aboutWin);
    osError = RunAppModalLoopForWindow(aboutWin);
    return gameTitle;
}

OSystem *OSystem_MAC::create(int gfx_mode, bool full_screen) {
	Rect rectWin;
	OSystem_MAC *syst = new OSystem_MAC();
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;

	/* Macintosh init */
	syst->init_mac_stuff();
	
	return syst;
}

uint32 OSystem_MAC::autosave(uint32 interval)
{
	g_scumm->_doAutosave = true;

	return interval;
}

OSystem *OSystem_MAC_create(int gfx_mode, bool full_screen) {
	return OSystem_MAC::create(gfx_mode, full_screen);
}

void OSystem_MAC::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	
	(*pal)->ctSeed = TickCount();
	for(int i = start; i < num; i++, b += 4) {
		(*pal)->ctTable[i].value = i;
		(*pal)->ctTable[i].rgb.red = b[0]<<8;
		(*pal)->ctTable[i].rgb.green = b[1]<<8;
		(*pal)->ctTable[i].rgb.blue = b[2]<<8;
	}
	
	CTabChanged(pal);
	
	if(_sai_func)
		UpdateGWorld(&screenBuf, 16, &blit_rect, NULL, NULL, 0);
	else
		UpdateGWorld(&screenBuf, 8, &blit_rect, pal, NULL, 0);
	
	if(start < _palette_changed_first)
		_palette_changed_first = start;
	
	if(start + num > _palette_changed_last)
		_palette_changed_last = start + num;
}

void OSystem_MAC::load_gfx_mode() {
	force_full = true;
	scaling = 1;
	_internal_scaling = 1;
	_mode_flags = 0;
	_sai_func = NULL;
	
	switch(_mode) {
		case GFX_2XSAI:
			_sai_func = _2xSaI;
		break;
		
		case GFX_SUPER2XSAI:
			_sai_func = Super2xSaI;
		break;
		
		case GFX_SUPEREAGLE:
			_sai_func = SuperEagle;
		break;
			
		case GFX_ADVMAME2X:
			_sai_func = AdvMame2x;
		break;
		
		case GFX_DOUBLESIZE:
			scaling = 2;
			_internal_scaling = 2;
			_mode_flags = DF_WANT_RECT_OPTIM;
		break;
	
		case GFX_TRIPLESIZE:
			if (_full_screen) {
				warning("full screen in useless in triplesize mode, reverting to normal mode");
				goto normal_mode;
			}
			scaling = 3;
			_internal_scaling = 3;
			_mode_flags = DF_WANT_RECT_OPTIM;		
		break;
	
		case GFX_NORMAL:
	normal_mode:;
			_mode_flags = DF_WANT_RECT_OPTIM;		
		break;
	
	}
	
	if(_sai_func)
	{		
		_mode_flags = DF_WANT_RECT_OPTIM | DF_SEPARATE_TEMPSCREEN | DF_UPDATE_EXPAND_1_PIXEL;

		Init_2xSaI(565);
		_tmp_buf = (int16*)calloc((SCREEN_WIDTH+3)*(SCREEN_HEIGHT+3), sizeof(int16));
		
		scaling = 2;
	}
	else
	{
		switch(scaling) {
		case 3:
			_sai_func = Normal3x;
			break;
		case 2:
			_sai_func = Normal2x;
			break;
		case 1:
			_sai_func = Normal1x;
			break;
		}

		_mode_flags = DF_WANT_RECT_OPTIM | DF_REAL_8BIT;
	}
	
	set_scaling();
}

void OSystem_MAC::unload_gfx_mode() {
	//warning("STUB: unload_gfx_mode()");		/* FIXME: Must free data here */
	
}

void OSystem_MAC::init_size(uint w, uint h) {
	//if (w != SCREEN_WIDTH && h != SCREEN_HEIGHT)
	//	error("320x200 is the only game resolution supported");
	
	SCREEN_WIDTH = w;
	SCREEN_HEIGHT = h;
	CKSUM_NUM = (SCREEN_WIDTH * SCREEN_HEIGHT / (8*8));
	dirty_rect_list = (Rect*)calloc(NUM_DIRTY_RECT, sizeof(Rect));
	_ms_backup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING);
	dirty_checksums = (uint32*)calloc(CKSUM_NUM*2, sizeof(uint32));

	load_gfx_mode();
}

void OSystem_MAC::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (pitch == SCREEN_WIDTH && x==0 && y==0 && w==SCREEN_WIDTH && h==SCREEN_HEIGHT && _mode_flags&DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(buf);
	} else {
		/* Clip the coordinates */
		if (x < 0) { w+=x; buf-=x; x = 0; }

	if (y < 0) { h+=y; buf-=y*pitch; y = 0; }
		if (w >= SCREEN_WIDTH-x) { w = SCREEN_WIDTH - x; }
		if (h >= SCREEN_HEIGHT-y) { h = SCREEN_HEIGHT - y; }
			
		if (w<=0 || h<=0)
			return;

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouse_drawn)
		undraw_mouse();
	
	byte *dst = (byte*)buf + y * SCREEN_WIDTH + x;
	do {
		memcpy(dst, buf, w);
		dst += SCREEN_WIDTH;
		buf += pitch;
	} while(--h);
}

void OSystem_MAC::add_dirty_rect(int x, int y, int w, int h) {
	if (force_full)
		return;

	if (num_dirty_rects == NUM_DIRTY_RECT)
		force_full = true;
	else {
		Rect *r = &dirty_rect_list[num_dirty_rects++];
		
		/* Update the dirty region by 1 pixel for graphics drivers
		 * that "smear" the screen */
		if (_mode_flags & DF_UPDATE_EXPAND_1_PIXEL) {
			x--;
			y--;
			w+=2;
			h+=2;
		}

		/* clip */
		if (x<0) { w+=x; x=0; }
		if (y<0) { h+=y; y=0; }
		if (w>=SCREEN_WIDTH-x) { w=SCREEN_WIDTH-x; }
		if (h>=SCREEN_HEIGHT-y) { h=SCREEN_HEIGHT-y; }
	
		if (_internal_scaling != 1) {
			x *= _internal_scaling;
			y *= _internal_scaling;
			w *= _internal_scaling;
			h *= _internal_scaling;
		}

		r->left = x;
		r->top = y;
		r->right = x + w;
		r->bottom = y + h;
	}
}

#define ROL(a,n) a = (a<<(n)) | (a>>(32-(n)))
#define DOLINE(x) a ^= ((uint32*)buf)[0+(x)*(SCREEN_WIDTH/4)]; b ^= ((uint32*)buf)[1+(x)*(SCREEN_WIDTH/4)]
void OSystem_MAC::mk_checksums(const byte *buf) {
	uint32 *sums = dirty_checksums;
	uint x,y;

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y=0; y!=SCREEN_HEIGHT/8; y++,buf+=SCREEN_WIDTH*(8-1))
		for(x=0; x!=SCREEN_WIDTH/8; x++,buf+=8) {
			uint32 a = x;
			uint32 b = y;

			DOLINE(0); ROL(a,13); ROL(b,11);
			DOLINE(2); ROL(a,13); ROL(b,11);
			DOLINE(4); ROL(a,13); ROL(b,11);
			DOLINE(6); ROL(a,13); ROL(b,11);

			a*=0xDEADBEEF;
			b*=0xBAADF00D;

			DOLINE(1); ROL(a,13); ROL(b,11);
			DOLINE(3); ROL(a,13); ROL(b,11);
			DOLINE(5); ROL(a,13); ROL(b,11);
			DOLINE(7); ROL(a,13); ROL(b,11);

			/* output the checksum for this block */
			*sums++=a+b;
		}
}
#undef DOLINE
#undef ROL


void OSystem_MAC::add_dirty_rgn_auto(const byte *buf) {
	assert( ((uint32)buf & 3) == 0);
	
	/* generate a table of the checksums */
	mk_checksums(buf);

if (!cksum_valid) {
		force_full = true;
		cksum_valid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
	 and add all dirty rectangles to a list. try to combine small rectangles
		 into bigger ones in a simple way */
	if (!force_full) {
		uint x,y,w;
		uint32 *ck = dirty_checksums;

		for(y=0; y!=SCREEN_HEIGHT/8; y++) {
			for(x=0; x!=SCREEN_WIDTH/8; x++,ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w+CKSUM_NUM] = ck[w];
						w++;
					} while (x+w != SCREEN_WIDTH/8 && ck[w] != ck[w+CKSUM_NUM]);

					add_dirty_rect(x*8, y*8, w*8, 8);

					if (force_full)
						goto get_out;
				}
			}
		}
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(dirty_checksums + CKSUM_NUM, dirty_checksums, CKSUM_NUM * sizeof(uint32));
	}
}

void OSystem_MAC::update_screen() {
#if 0
	/* First make sure the mouse is drawn, if it should be drawn. */
	draw_mouse();

	if (_palette_changed_last != 0) {
		//warning("MAC: Palette should be uploaded!");/* FIXME: Add Palette changing code */
		
		/*GDevice **odisplay;
		odisplay = GetGDevice();
		SetGDevice(GetMainDevice());
		SetEntries(0, (**pal).ctSize, (ColorSpec *)&(**pal).ctTable);
		SetGDevice(odisplay);*/
		
		/*_palette_changed_last = 0;
		if (_mode_flags & DF_FORCE_FULL_ON_PALETTE)
			*/force_full = true;
	}

	/* force a full redraw, accomplish that by adding one big rect to the dirty
	 * rect list */
	if (force_full) {
		num_dirty_rects = 1;
		
		dirty_rect_list[0].left = 0;
		dirty_rect_list[0].top = 0;
		dirty_rect_list[0].right = SCREEN_WIDTH;
		dirty_rect_list[0].bottom = SCREEN_HEIGHT;
	}
	
	if (num_dirty_rects > 0)
	{
		Rect *r; 
		uint32 srcPitch, dstPitch;
		Rect *last_rect = dirty_rect_list + num_dirty_rects;
	
		/* Convert appropriate parts of the image into 16bpp */
		if ((_mode_flags & DF_REAL_8BIT) == 0) {
			Rect dst;
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				dst = *r;
				dst.left++;
				dst.top++;
				dst.right++;
				dst.bottom++;
			}
		}
	
		/*srcPitch = sdl_tmpscreen->pitch;
		dstPitch = sdl_hwscreen->pitch;*/
	
		if ((_mode_flags & DF_REAL_8BIT) == 0) {
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				register int dst_y = r->y + _current_shake_pos;
				register int dst_h = 0;
				if (dst_y < SCREEN_HEIGHT) {
					dst_h = r->h;
					if (dst_h > SCREEN_HEIGHT - dst_y)
						dst_h = SCREEN_HEIGHT - dst_y;
					
					r->x <<= 1;
					dst_y <<= 1;
					
					_sai_func((byte*)sdl_tmpscreen->pixels + (r->x+2) + (r->y+1)*srcPitch, srcPitch, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*scaling + dst_y*dstPitch, dstPitch, r->w, dst_h);
				}
				
				r->y = dst_y;
				r->w <<= 1;
				r->h = dst_h << 1;
			}
		} else {
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				register int dst_y = r->y + _current_shake_pos;
				register int dst_h = 0;
				if (dst_y < SCREEN_HEIGHT) {
					dst_h = r->h;
					if (dst_h > SCREEN_HEIGHT - dst_y)
						dst_h = SCREEN_HEIGHT - dst_y;
					
					dst_y *= scaling;
					
					_sai_func((byte*)sdl_tmpscreen->pixels + r->x + r->y*srcPitch, srcPitch, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*scaling + dst_y*dstPitch, dstPitch, r->w, dst_h);
				}
				
				r->x *= scaling;
				r->y = dst_y;
				r->w *= scaling;
				r->h = dst_h * scaling;
			}
		}
	
		if (force_full) {
			dirty_rect_list[0].y = 0;
			dirty_rect_list[0].h = SCREEN_HEIGHT * scaling;
		}
	}
		
	/*if(_mode_flags & DF_2xSAI)
	{
		Rect *r;
		uint32 area = 0;
		
		Rect *dr = dirty_rect_list + num_dirty_rects;
		
		for(r = dirty_rect_list; r != dr; r++)
		{
			GWorldPtr gw;
			Rect rec;
			SetRect(&rec, 0, 0, 320, 200);
			NewGWorldFromPtr(&gw, 16, &rec, NULL, NULL, 0, (char*)_tmp_buf, rec.right);
			CopyBits(GetPortBitMapForCopyBits(gw), GetPortBitMapForCopyBits(screenBuf),
						r, r, srcCopy, 0L);
		}
		
		for(r = dirty_rect_list; r != dr; r++)
		{
			_sai_func((byte*)_tmp_buf + r->left * 2 + r->top * 640, 640, NULL,
				(byte*)_sai_buf + r->left * 4 + r->top * 640 * 4, 640 * 2,
				r->right - r->left, r->bottom - r->top);
			
			area += (r->right - r->left) * (r->bottom - r->top);
			
			r->left <<= 1;
			r->right <<= 1;
			r->top <<= 1;
			r->bottom <<= 1;
		}
	}*/
	
	update_rects();
	//blit_to_screen();
	
	num_dirty_rects = 0;

#endif
}

bool OSystem_MAC::show_mouse(bool visible) {
	if (_mouse_visible == visible)
		return visible;
	
	bool last = _mouse_visible;
	_mouse_visible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}
	
void OSystem_MAC::set_mouse_pos(int x, int y) {
	if (x != _ms_cur.x || y != _ms_cur.y) {
		_ms_cur.x = x;
		_ms_cur.y = y;
		undraw_mouse();
	}
}
	
void OSystem_MAC::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_ms_cur.w = w;
	_ms_cur.h = h;

	_ms_hotspot_x = hotspot_x;
	_ms_hotspot_y = hotspot_y;

	_ms_buf = (byte*)buf;

	undraw_mouse();
}
	
void OSystem_MAC::set_shake_pos(int shake_pos) {
	/*int old_shake_pos = _current_shake_pos;
	int dirty_height, dirty_blackheight;
	int dirty_top, dirty_blacktop;
	
	if (shake_pos != old_shake_pos) {
		_current_shake_pos = shake_pos;
		force_full = true;

		/* Old shake pos was current_shake_pos, new is shake_pos.
		 * Move the screen up or down to account for the change.
		 */
		//SDL_Rect dstr = { 0, shake_pos*scaling, 320*scaling, 200*scaling };
		//SDL_Rect srcr = { 0, old_shake_pos*scaling, 320*scaling, 200*scaling };
		//SDL_BlitSurface(sdl_screen, &srcr, sdl_screen, &dstr);
	/*	Rect srcr, dstr;
		
		SetRect(&srcr, 0, old_shake_pos * scaling, 320 * scaling, 200 * scaling);
		SetRect(&dstr, 0, shake_pos * scaling, 320 * scaling, 200 * scaling);
		
		CopyBits(GetPortBitMapForCopyBits(screenBuf), GetPortBitMapForCopyBits(GetWindowPort(wref)),
			&srcr, &dstr, srcCopy, 0L);
		
		/* Refresh either the upper part of the screen,
		 * or the lower part */
	/*	if (shake_pos > old_shake_pos) {
			dirty_height = MIN(shake_pos, 0) - MIN(old_shake_pos, 0);
			dirty_top = -MIN(shake_pos, 0);
			dirty_blackheight = MAX(shake_pos, 0) - MAX(old_shake_pos, 0);
			dirty_blacktop = MAX(old_shake_pos, 0);
		} else {
			dirty_height = MAX(old_shake_pos, 0) - MAX(shake_pos, 0);
			dirty_top = 200 - MAX(old_shake_pos, 0);
			dirty_blackheight = MIN(old_shake_pos, 0) - MIN(shake_pos, 0);
			dirty_blacktop = 200 + MIN(shake_pos, 0);
		}

		/* Fill the dirty area with blackness or the scumm image */
		//SDL_Rect blackrect = {0, dirty_blacktop*scaling, 320*scaling, dirty_blackheight*scaling};
		//SDL_FillRect(sdl_screen, &blackrect, 0);

		/* FIXME: Um, screen seems to glitch since this
		          'not needed' function was removed */
		//g_scumm->redrawLines(dirty_top, dirty_top + dirty_height);
/*	}*/
}
		
uint32 OSystem_MAC::get_msecs() {
	UnsignedWide ms;

	Microseconds(&ms);
	return(ms.lo / 1000);
}
	
void OSystem_MAC::delay_msecs(uint msecs) {
	uint32 start = get_msecs();
	Event dummy;

	do {
		poll_event(&dummy);	/* Do something to avoid CPU lock */
		if(get_msecs() >= start + msecs)
			break;
	} while (1);
}
	
void *OSystem_MAC::create_thread(ThreadProc *proc, void *param) {
	warning("MAC: Stub create_thread()");
	//NewThread(kCooperativeThread, (void*)proc, param, 0L, kCreateIfNeeded, NULL, NULL);
}

int mapKey(int key, byte code, byte mod)
{
	switch(code) {
	case 0x35:
		key = 27;
	break;
	case 0x31:
		key = 32;
	break;
	case 0x60:
		key = 601;
	break;
	}
	
	return key;
}
	
bool OSystem_MAC::poll_event(Event *event)
{
	EventRef theEvent;
	EventTargetRef theTarget;
	OSStatus theErr;
	
	OSStatus		result = eventNotHandledErr;
	HICommand		command;
	Point			mouse;

	theTarget = GetEventDispatcherTarget();
	theErr = ReceiveNextEvent(GetEventTypeCount(kCmdEvents), kCmdEvents, kEventDurationNoWait,true, &theEvent);
	
	GetEventParameter( theEvent, kEventParamDirectObject, typeHICommand, NULL,
		sizeof( HICommand ), NULL, &command );
	
	switch(GetEventClass(theEvent))
	{
		case kEventClassWindow:
			switch(GetEventKind(theEvent))
			{
				case kEventWindowDrawContent:
				break;
				
				case kEventWindowHandleContentClick:
					EventMouseButton btn;
					
					GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL,
						sizeof(EventMouseButton), NULL, &btn);
					
					if(btn == kEventMouseButtonPrimary)
						event->event_code = EVENT_RBUTTONDOWN;
					else if(btn == kEventMouseButtonSecondary)
						event->event_code = EVENT_LBUTTONDOWN;
					
					debug(1, "Mouse down!");
					
					if(wref != FrontWindow())
					{
// FIXME - seems some versions of the CarbonLib stub are missing ActivateWindow
//						ActivateWindow(wref, true);
						BringToFront(wref);
					}
					return true;
				break;
				
				case kEventWindowClose:
					quit();
				break;
			}
		break;
		
		case kEventClassCommand:
			switch(command.commandID)
			{
				case kNewGameCmd:
					
				break;
				
				case kOpenGameCmd:
					//scumm->_saveLoadSlot = 0;
					//scumm->_saveLoadFlag = 2;
				break;
				
				case kSaveGameCmd:
					//scumm->_saveLoadSlot = 0;
					//sprintf(scumm->_saveLoadName, "Quicksave %d", scumm->_saveLoadSlot);
					//scumm->_saveLoadFlag = 1;
				break;
				
				case kQuitCmd:
					quit();
				break;
				
				case kPrefsCmd:
					//Preferences();
				break;
				
				case kAboutCmd:
					//About();
				break;
			}
		break;
		
		case kEventClassKeyboard:
			if(GetEventKind(theEvent) == kEventRawKeyDown)
			{
				char	key;
				UInt32	mod, code;
				
				GetEventParameter(theEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &code);
				GetEventParameter(theEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key);
				GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &mod);
				
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = code;
				event->kbd.ascii = mapKey(key, code, mod);
				debug(1, "Key down: %c", event->kbd.ascii);
				return true;
			}
		break;
		
		case kEventClassMouse:
			EventMouseButton btn;
			Rect winRect;
			
			switch(GetEventKind(theEvent))
			{			
				case kEventMouseDown:
					WindowRef theWin;
					
					GetEventParameter(theEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef),
						NULL, &theWin);
					if(theWin != FrontWindow())
					{
// FIXME - seems some versions of the CarbonLib stub are missing ActivateWindow
//						ActivateWindow(theWin, true);
						BringToFront(theWin);
					}
					
					GetEventParameter(theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &mouse);
					
					GetWindowBounds(wref, kWindowContentRgn, &winRect);
					if(PtInRect(mouse, &winRect))
					{
						GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL,
							sizeof(EventMouseButton), NULL, &btn);
						
						if(btn == kEventMouseButtonPrimary)
							event->event_code = EVENT_RBUTTONDOWN;
						else if(btn == kEventMouseButtonSecondary)
							event->event_code = EVENT_LBUTTONDOWN;
						
						debug(1, "Mouse down!");
					}				
				break;
				
				case kEventMouseUp:
					
					GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL,
						sizeof(EventMouseButton), NULL, &btn);
					
					if(btn == kEventMouseButtonPrimary)
						event->event_code = EVENT_RBUTTONUP;
					else if(btn == kEventMouseButtonSecondary)
						event->event_code = EVENT_LBUTTONUP;
					
					debug(1, "Mouse up!");
					
					return true;
				break;
				
				case kEventMouseMoved:				
					GetEventParameter(theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &mouse);
					
					GetWindowBounds(wref, kWindowContentRgn, &winRect);
					if(PtInRect(mouse, &winRect))
					{
						CGrafPtr oldPort;
					
						GetPort(&oldPort);
						SetPortWindowPort(wref);
						GlobalToLocal(&mouse);
						
						event->event_code = EVENT_MOUSEMOVE;
						event->mouse.x = mouse.h / scaling;
						event->mouse.y = mouse.v / scaling;
						
						//scumm->mouse.x = mouse.h/wm->scale;
						//scumm->mouse.y = mouse.v/wm->scale;
					}
					Point offset = {0, 0};
					ShieldCursor(&winRect, offset);
					return true;
				break;
			}
		break;
	}
	
	if(theErr == noErr && theEvent != NULL) {
		SendEventToEventTarget (theEvent, theTarget);
		ReleaseEvent(theEvent);
	}
}

pascal void sound_callback(SndChannel *chan, SndCommand *cmd_passed)
{
	OSystem_MAC* syst = (OSystem_MAC*)chan->userInfo;
	syst->sound_callback(chan, cmd_passed);
}

void OSystem_MAC::sound_callback(SndChannel *chan, SndCommand *cmd_passed)
{
	UInt32 fill_me, play_me;
	SndCommand cmd;
	
	fill_me = cmd_passed->param2;
	play_me = ! fill_me;
	
	header.samplePtr = (Ptr)buffer[play_me];
	
	cmd.cmd = bufferCmd;
	cmd.param1 = 0;
	cmd.param2 = (long)&header;
	
	SndDoCommand(chan, &cmd, 0);
	
	memset(buffer[fill_me], 0, size);
	//sndProc(parameter, buffer[fill_me], size);
	//SoundMixer::on_generate_samples(parameter, buffer[fill_me], size);
	
	cmd.cmd = callBackCmd;
	cmd.param1 = 0;
	cmd.param2 = play_me;
	
	SndDoCommand(chan, &cmd, 0);
}

bool OSystem_MAC::set_sound_proc(void *param, SoundProc *proc, byte format)
{
	SndCallBackUPP callback;
	int sample_size;
		
	memset(&header, 0, sizeof(header));
	callback = NewSndCallBackUPP(::sound_callback);
	size = ((0x9010 & 0xFF) / 8) * 2048;
	sample_size = size / 2048 * 8;
	header.numChannels = 1;
	header.sampleSize = sample_size;
	header.sampleRate = SAMPLES_PER_SEC << 16;
	header.numFrames = 	2048;
	header.encode = cmpSH;
	
	for(int i = 0; i < 2; i++)
	{
		buffer[i] = (UInt8*)malloc(sizeof(UInt8) * size);
		memset(buffer[i], 0, size);
	}
	
	channel = (SndChannelPtr)malloc(sizeof(*channel));
	channel->qLength = 128;
	channel->userInfo = (long)this;
	SndNewChannel(&channel, sampledSynth, initMono, callback);
	
	SndCommand cmd;
	cmd.cmd = callBackCmd;
	cmd.param2 = 0;
	SndDoCommand(channel, &cmd, 0);
	
	sndProc = proc;
	parameter = param;

	return true;
}


/* retrieve the 320x200 bitmap currently being displayed */
void OSystem_MAC::get_320x200_image(byte *buf)
{
	/* make sure the mouse is gone */
	undraw_mouse();
	
	byte *src;
	int x,y;

	switch(_internal_scaling) {
	case 1:
		memcpy(buf, _gfx_buf, 320*200);
		break;

	case 2:
		src = (byte*)_gfx_buf;
		for(y=0; y!=200; y++) {
			for(x=0; x!=320; x++)
				buf[x] = src[x*2];
			buf += 320;
			src += 320 * 2 * 2;
		}
		break;

	case 3:
		src = (byte*)_gfx_buf;
		for(y=0; y!=200; y++) {
			for(x=0; x!=320; x++)
				buf[x] = src[x*3];
			buf += 320;
			src += 320 * 3 * 3;
		}
		break;
	}	
}

void OSystem_MAC::hotswap_gfx_mode()
{
	/* hmm, need to allocate a 320x200 bitmap
	 * which will contain the "backup" of the screen during the change.
	 * then draw that to the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(320*200);

	get_320x200_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	force_full = true;

	/* reset palette ? */
	pal = (CTabHandle)NewHandleClear(sizeof(ColorTable) + 255 * sizeof(ColorSpec));
	(*pal)->ctFlags = 0;
	(*pal)->ctSize = 255;
	
	/* blit image */
	copy_rect(bak_mem, 320, 0, 0, 320, 200);
	free(bak_mem);

	update_screen();
}

uint32 OSystem_MAC::property(int param, Property *value) {
	switch(param) {
	case PROP_TOGGLE_FULLSCREEN:
		_full_screen ^= true;
	return 1;

	case PROP_SET_WINDOW_CAPTION:
		StringPtr gameText = CToPascal((char*)value->caption);
		SetWTitle(wref, gameText);
	return 1;

	case PROP_OPEN_CD:
	break;

	case PROP_SET_GFX_MODE:
		if(value->gfx_mode >= 7)
			return 0;
		_mode = value->gfx_mode;
		hotswap_gfx_mode();
	return 1;


	case PROP_SHOW_DEFAULT_CURSOR:
	break;
	
	case PROP_GET_SAMPLE_RATE:
	return SAMPLES_PER_SEC;
	break;
	}
	
	return 0;
}
		
void OSystem_MAC::quit() {
	unload_gfx_mode();

	QuitApplicationEventLoop();
	ExitToShell();
}

void OSystem_MAC::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible)
		return;
	_mouse_drawn = true;
	
	const int ydraw = _ms_cur.y + _current_shake_pos - _ms_hotspot_y;
	const int xdraw = _ms_cur.x - _ms_hotspot_x;
	const int w = _ms_cur.w;
	const int h = _ms_cur.h;
	int x,y;
	byte color;
	byte *dst, *bak = _ms_backup;
	byte *buf = _ms_buf;

	_ms_old.w = w;
	_ms_old.h = h;
	_ms_old.x = xdraw;
	_ms_old.y = ydraw;
	
	byte *src;
	if(_sai_func)
		src = (byte*)_tmp_buf;
	else
		src = _gfx_buf;
	
	switch(_internal_scaling) {
	case 1:
		dst = (byte *)src + ydraw * 320 + xdraw;

		for (y = 0; y < h; y++, dst += 320, bak += MAX_MOUSE_W, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x] = dst[x];
						if ((color = buf[x]) != 0xFF) {
							dst[x] = color;
						}
					}
				}
			}
		}
		break;

	case 2:
		dst = (byte *)src + ydraw * 640 * 2 + xdraw * 2;

		for (y = 0; y < h; y++, dst += 640 * 2, bak += MAX_MOUSE_W * 2, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x * 2] = dst[x * 2];
						bak[x * 2 + 1] = dst[x * 2 + 1];
						if ((color = buf[x]) != 0xFF) {
							dst[x * 2] = color;
							dst[x * 2 + 1] = color;
							dst[x * 2 + 640] = color;
							dst[x * 2 + 1 + 640] = color;
						}
					}
				}
			}
		}
		break;

	case 3:
		dst = (byte *)src + ydraw * 960 * 3 + xdraw * 3;

		for (y = 0; y < h; y++, dst += 960 * 3, bak += MAX_MOUSE_W * 3, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x * 3] = dst[x * 3];
						bak[x * 3 + 1] = dst[x * 3 + 1];
						bak[x * 3 + 2] = dst[x * 3 + 2];
						if ((color = buf[x]) != 0xFF) {
							dst[x * 3] = color;
							dst[x * 3 + 1] = color;
							dst[x * 3 + 2] = color;
							dst[x * 3 + 960] = color;
							dst[x * 3 + 1 + 960] = color;
							dst[x * 3 + 2 + 960] = color;
							dst[x * 3 + 960 + 960] = color;
							dst[x * 3 + 1 + 960 + 960] = color;
							dst[x * 3 + 2 + 960 + 960] = color;
						}
					}
				}
			}
		}
		break;
	}

	add_dirty_rect(xdraw,ydraw,w,h);
}

void OSystem_MAC::undraw_mouse() {
	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;
	
	byte *dst, *bak = _ms_backup;
	byte *src;
	const int old_mouse_x = _ms_old.x;
	const int old_mouse_y = _ms_old.y;
	const int old_mouse_w = _ms_old.w;
	const int old_mouse_h = _ms_old.h;
	int x,y;
	
	if(_sai_func)
		src = (byte*)_tmp_buf;
	else
		src = _gfx_buf;
	
	switch(_internal_scaling) {
	case 1:
		dst = (byte *)src + old_mouse_y * 320 + old_mouse_x;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W, dst += 320) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x] = bak[x];
					}
				}
			}
		}
		break;

	case 2:
		dst = (byte *)src + old_mouse_y * 640 * 2 + old_mouse_x * 2;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W * 2, dst += 640 * 2) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x * 2 + 640] = dst[x * 2] = bak[x * 2];
						dst[x * 2 + 640 + 1] = dst[x * 2 + 1] = bak[x * 2 + 1];
					}
				}
			}
		}
		break;

	case 3:
		dst = (byte *)src + old_mouse_y * 960 * 3 + old_mouse_x * 3;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W * 3, dst += 960 * 3) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x * 3 + 960] = dst[x * 3 + 960 + 960] = dst[x * 3] =
							bak[x * 3];
						dst[x * 3 + 960 + 1] = dst[x * 3 + 960 + 960 + 1] =
							dst[x * 3 + 1] = bak[x * 3 + 1];
						dst[x * 3 + 960 + 2] = dst[x * 3 + 960 + 960 + 2] =
							dst[x * 3 + 2] = bak[x * 3 + 2];
					}
				}
			}
		}
		break;
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);
}

void OSystem_MAC::stop_cdrom() {
}

void OSystem_MAC::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {
	/* Reset sync count */
	g_scumm->_vars[g_scumm->VAR_MI1_TIMER] = 0;
}

bool OSystem_MAC::poll_cdrom() {
}

void OSystem_MAC::update_cdrom() {
}



/*************************************************************/
/** Mac specific code ****************************************/
void OSystem_MAC::set_scaling() {
	Rect rectWin;
	SetRect(&rectWin, 0, 0, 320 * scaling, 200 * scaling);
	HideWindow(wref);
	SetWindowBounds(wref, kWindowContentRgn, &rectWin);
	RepositionWindow(wref, NULL, kWindowCenterOnMainScreen);
	ShowWindow(wref);
	blit_rect = rectWin;
	
	if(_sai_func)
	{
		Rect r;
		
		//SetRect(&r, 0, 0, 320, 240);
		_sai_buf = (int16*)malloc((320 * 200) * 2 * sizeof(int16));		
		
		NewGWorldFromPtr(&screenBuf, 16, &blit_rect, NULL, nil, 0, (char *)_sai_buf, blit_rect.right);
	}
	else
	{
		_gfx_buf = (byte*)malloc((320 * 200) * scaling * sizeof(byte));
		NewGWorldFromPtr(&screenBuf, 8, &blit_rect, pal, nil, 0, (char *)_gfx_buf, blit_rect.right);
	}	
	
	//NewGWorldFromPtr(&screenBuf, 8, &blit_rect, pal, nil, 0, (char *)_gfx_buf, blit_rect.right);
	
	//if(screenBuf != NULL)
	//	UpdateGWorld(&screenBuf, 8, &blit_rect, pal, NULL, NULL);
}

void OSystem_MAC::blit_to_screen()
{
	CopyBits(GetPortBitMapForCopyBits(screenBuf),
	GetPortBitMapForCopyBits(GetWindowPort(wref)), &blit_rect, &blit_rect, srcCopy, 0L);
}

void OSystem_MAC::init_mac_stuff()
{
	Rect			rectWin;

	
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
	
	SetRect(&rectWin, 0, 0, 320, 200);
	UInt32 WinAttrib = (kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute |
				kWindowInWindowMenuAttribute | kWindowStandardHandlerAttribute);
	
	if(noErr != CreateNewWindow(kDocumentWindowClass, WinAttrib, &rectWin, &wref))
	{
		//Error("Couldn't create Window!");
	}
	
	RepositionWindow(wref, NULL, kWindowCenterOnMainScreen);
	
	Str255 WindowTitle = "\pScummVM";
	SetWTitle(wref, WindowTitle);
	
	SetPortWindowPort(wref);
	//ShowWindow(wref);
	
	InstallStandardEventHandler(GetWindowEventTarget(wref));
	
	//OSStatus err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitEventHandler), 0L, false);
	
	blit_rect = rectWin;
	pal = (CTabHandle)NewHandleClear(sizeof(ColorTable) + 255 * sizeof(ColorSpec));
	(*pal)->ctFlags = 0;
	(*pal)->ctSize = 255;
	//NewGWorld(&screenBuf, 8, &blit_rect, 0, 0, 0);
}

void OSystem_MAC::update_rects()
{
	for(int i = 0; i < num_dirty_rects; i++)
	{
		Rect rec = dirty_rect_list[i];
		
		CopyBits(GetPortBitMapForCopyBits(screenBuf),
				GetPortBitMapForCopyBits(GetWindowPort(wref)),
				&rec, &rec, srcCopy, 0L);
	}
}
