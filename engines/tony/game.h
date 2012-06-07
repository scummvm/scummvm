/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_GAME_H
#define TONY_GAME_H

#include "tony/adv.h"
#include "tony/gfxcore.h"
#include "tony/input.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Tony {

#define INIT_GFX16_FROMRAW(dwRes, buf16)                 \
	raw = new RMResRaw(dwRes);                             \
	assert(raw->isValid());                                \
	assert((buf16) == NULL);                               \
	(buf16) = new RMGfxSourceBuffer16(false);              \
	(buf16)->init(*raw,raw->width(),raw->height());        \
	delete raw;

#define INIT_GFX8_FROMRAW(raw, dwRes, buf8)              \
	raw = new RMResRaw(dwRes);                             \
	assert(raw->isValid());                                \
	assert((buf8) == NULL);                                \
	(buf8) = new RMGfxSourceBuffer8RLEByte();              \
	(buf8)->init(*raw, raw->width(), raw->height(), true); \
	delete raw;


class RMPointer : public RMGfxTask {
private:
	RMGfxSourceBuffer8 *_pointer[16];
	RMPoint _hotspot[16];
	RMPoint _pos;

	RMItem *_specialPointer[16];

	int _nCurPointer;
	int _nCurSpecialPointer;

	RMGfxSourceBuffer8 *_nCurCustomPointer;

public:
	enum POINTER {
		PTR_NONE = 0,
		PTR_FRECCIASU,
		PTR_FRECCIAGIU,
		PTR_FRECCIASINISTRA,
		PTR_FRECCIADESTRA,
		PTR_FRECCIAMAPPA,
		PTR_CUSTOM
	};

public:
	// Constructor & destructor
	RMPointer();
	virtual ~RMPointer();

	// Initialisation
	void init(void);

	// Deinitialisation
	void close(void);

	// Process a frame
	void doFrame(RMGfxTargetBuffer *bigBuf);

	// Overloading of priorities
	int priority();

	// Overloading draw method
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Sets the current co-ordinates
	void setCoord(const RMPoint &pt) {
		_pos = pt;
	}

	// Overloading of the method to see if rising from the list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Sets a new action as current
	void setAction(RMTonyAction action) {
		_nCurPointer = action;
	}

	// Sets a new pointer
	void setSpecialPointer(POINTER ptr) {
		_nCurSpecialPointer = ptr;
		if (_nCurSpecialPointer && _nCurSpecialPointer != PTR_CUSTOM)
			_specialPointer[ptr - 1]->setPattern(1);
	}
	POINTER getSpecialPointer(void) {
		return (POINTER)_nCurSpecialPointer;
	}

	// Set the new custom pointer
	void setCustomPointer(RMGfxSourceBuffer8 *ptr) {
		_nCurCustomPointer = ptr;
	}

	// Return the current action to be applied according to the pointer
	int curAction(void);
};

class RMOptionButton: public RMGfxTaskSetPrior {
public:
	RMRect _rect;
	RMGfxSourceBuffer16 *_buf;
	bool _bActive;
	bool _bHasGfx;
	bool _bDoubleState;

public:
	RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState = false);
	RMOptionButton(const RMRect &pt);
	virtual ~RMOptionButton();

	bool doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void addToList(RMGfxTargetBuffer &bigBuf);
	bool isActive() {
		return _bActive;
	}
	void setActiveState(bool bState) {
		_bActive = bState;
	}
};

class RMOptionSlide : public RMGfxTaskSetPrior {
private:
	RMOptionButton *_PushLeft;
	RMOptionButton *_PushRight;
	RMGfxSourceBuffer16 *_SliderCenter;
	RMGfxSourceBuffer16 *_SliderLeft;
	RMGfxSourceBuffer16 *_SliderRight;
	RMGfxSourceBuffer16 *_SliderSingle;
	int _nSlideSize;
	RMPoint _pos;
	int _nValue;
	int _nMax;
	int _nStep;

public:
	RMOptionSlide(const RMPoint &pt, int m_nRange = 100, int m_nStartValue = 0, int slideSize = 300);
	virtual ~RMOptionSlide();

	bool doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void addToList(RMGfxTargetBuffer &bigBuf);

	int getValue() {
		return _nValue;
	}
};

class RMOptionScreen : public RMGfxWoodyBuffer {
private:
	RMGfxSourceBuffer16 *_menu;
	RMGfxSourceBuffer16 *_QuitConfirm;
	RMGfxSourceBuffer16 *_HideLoadSave;
	RMOptionButton *_ButtonQuitYes;
	RMOptionButton *_ButtonQuitNo;
	RMOptionButton *_ButtonExit;
	RMOptionButton *_ButtonQuit;
	RMOptionButton *_ButtonLoad;
	RMOptionButton *_ButtonSave;
	RMOptionButton *_ButtonGameMenu;
	RMOptionButton *_ButtonGfxMenu;
	RMOptionButton *_ButtonSoundMenu;
	RMGfxSourceBuffer8 *_SaveEasy;
	RMGfxSourceBuffer8 *_SaveHard;
	RMGfxSourceBuffer16 *_curThumb[6];
	RMString _curThumbName[6];
	byte _curThumbDiff[6];
	RMOptionButton *_ButtonSave_States[6];
	RMOptionButton *_ButtonSave_ArrowLeft;
	RMOptionButton *_ButtonSave_ArrowRight;
	RMOptionButton *_ButtonGfx_Tips;

	RMOptionButton *_ButtonSound_DubbingOn;
	RMOptionButton *_ButtonSound_MusicOn;
	RMOptionButton *_ButtonSound_SFXOn;

	RMOptionSlide *_SlideTonySpeed;
	RMOptionSlide *_SlideTextSpeed;


	int _statePos;
	bool _bEditSaveName;
	int _nEditPos;
	char _EditName[256];

	union {
		RMOptionButton *_ButtonGame_Lock;
		RMOptionButton *_ButtonGfx_Anni30;
		RMOptionSlide *_SliderSound_Music;
	};
	union {
		RMOptionButton *_ButtonGame_TimerizedText;
		RMOptionButton *_ButtonGfx_AntiAlias;
		RMOptionSlide *_SliderSound_SFX;
	};
	union {
		RMOptionButton *_ButtonGame_Scrolling;
		RMOptionButton *_ButtonGfx_Sottotitoli;
		RMOptionSlide *_SliderSound_Dubbing;
	};
	union {
		RMOptionButton *_ButtonGame_InterUp;
		RMOptionButton *_ButtonGfx_Trans;
	};

	int _FadeStep;
	bool _bExit;
	bool _bQuitConfirm;
	int _FadeY;
	int _FadeTime;
	bool _bLoadMenuOnly;
	bool _bNoLoadSave;
	bool _bAlterGfx;

	enum STATE {
	    MENUGAME,
	    MENUGFX,
	    MENUSOUND,
	    MENULOAD,
	    MENUSAVE,
		MENUNONE
	};

	STATE _nState;
	STATE _nLastState;

public:
	RMOptionScreen();
	virtual ~RMOptionScreen();

	void init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void initLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void initSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void initNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void reInit(RMGfxTargetBuffer &bigBuf);
	bool close();
	bool isClosing();

	// Overloaded methods
	virtual int priority();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void removeThis(CORO_PARAM, bool &result);

	// Polling for the option screen
	void doFrame(CORO_PARAM, RMInput *m_input);

	// Retrieves a savegame's thumbnail, description, and difficulty level
	static bool loadThumbnailFromSaveState(int numState, byte *lpDestBuf, RMString &name, byte &diff);

protected:

	// Initialisation and state change
	void initState(CORO_PARAM);
	void closeState(void);
	void changeState(CORO_PARAM, STATE newState);

	// Repaint the options menu
	void refreshAll(CORO_PARAM);
	void refreshThumbnails(void);
};

} // End of namespace Tony

#endif
