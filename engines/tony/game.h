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
	assert(raw->IsValid());                                \
	assert((buf16) == NULL);                               \
	(buf16) = new RMGfxSourceBuffer16(false);              \
	(buf16)->Init(*raw,raw->Width(),raw->Height());        \
	delete raw;

#define INIT_GFX8_FROMRAW(raw, dwRes, buf8)              \
	raw = new RMResRaw(dwRes);                             \
	assert(raw->IsValid());                                \
	assert((buf8) == NULL);                                \
	(buf8) = new RMGfxSourceBuffer8RLEByte();              \
	(buf8)->Init(*raw, raw->Width(), raw->Height(), true); \
	delete raw;


class RMPointer : public RMGfxTask {
private:
	RMGfxSourceBuffer8 *m_pointer[16];
	RMPoint m_hotspot[16];
	RMPoint m_pos;

	RMItem *m_specialPointer[16];

	int m_nCurPointer;
	int m_nCurSpecialPointer;

	RMGfxSourceBuffer8 *m_nCurCustomPointer;

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
	void Init(void);

	// Deinitialisation
	void Close(void);

	// Process a frame
	void DoFrame(RMGfxTargetBuffer *bigBuf);

	// Overloading of priorities
	int Priority();

	// Overloading draw method
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Sets the current co-ordinates
	void SetCoord(const RMPoint &pt) {
		m_pos = pt;
	}

	// Overloading of the method to see if rising from the list
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Sets a new action as current
	void SetAction(RMTonyAction action) {
		m_nCurPointer = action;
	}

	// Sets a new pointer
	void SetSpecialPointer(POINTER ptr) {
		m_nCurSpecialPointer = ptr;
		if (m_nCurSpecialPointer && m_nCurSpecialPointer != PTR_CUSTOM)
			m_specialPointer[ptr - 1]->SetPattern(1);
	}
	POINTER GetSpecialPointer(void) {
		return (POINTER)m_nCurSpecialPointer;
	}

	// Set the new custom pointer
	void SetCustomPointer(RMGfxSourceBuffer8 *ptr) {
		m_nCurCustomPointer = ptr;
	}

	// Return the current action to be applied according to the pointer
	int CurAction(void);
};

class RMOptionButton: public RMGfxTaskSetPrior {
public:
	RMRect m_rect;
	RMGfxSourceBuffer16 *m_buf;
	bool m_bActive;
	bool m_bHasGfx;
	bool m_bDoubleState;

public:
	RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState = false);
	RMOptionButton(const RMRect &pt);
	virtual ~RMOptionButton();

	bool DoFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void AddToList(RMGfxTargetBuffer &bigBuf);
	bool IsActive() {
		return m_bActive;
	}
	void SetActiveState(bool bState) {
		m_bActive = bState;
	}
};

class RMOptionSlide : public RMGfxTaskSetPrior {
private:
	RMOptionButton *m_PushLeft;
	RMOptionButton *m_PushRight;
	RMGfxSourceBuffer16 *m_SliderCenter;
	RMGfxSourceBuffer16 *m_SliderLeft;
	RMGfxSourceBuffer16 *m_SliderRight;
	RMGfxSourceBuffer16 *m_SliderSingle;
	int m_nSlideSize;
	RMPoint m_pos;
	int m_nValue;
	int m_nMax;
	int m_nStep;

public:
	RMOptionSlide(const RMPoint &pt, int m_nRange = 100, int m_nStartValue = 0, int slideSize = 300);
	virtual ~RMOptionSlide();

	bool DoFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void AddToList(RMGfxTargetBuffer &bigBuf);

	int GetValue() {
		return m_nValue;
	}
};

class RMOptionScreen : public RMGfxWoodyBuffer {
private:
	RMGfxSourceBuffer16 *m_menu;
	RMGfxSourceBuffer16 *m_QuitConfirm;
	RMGfxSourceBuffer16 *m_HideLoadSave;
	RMOptionButton *m_ButtonQuitYes;
	RMOptionButton *m_ButtonQuitNo;
	RMOptionButton *m_ButtonExit;
	RMOptionButton *m_ButtonQuit;
	RMOptionButton *m_ButtonLoad;
	RMOptionButton *m_ButtonSave;
	RMOptionButton *m_ButtonGameMenu;
	RMOptionButton *m_ButtonGfxMenu;
	RMOptionButton *m_ButtonSoundMenu;
	RMGfxSourceBuffer8 *m_SaveEasy;
	RMGfxSourceBuffer8 *m_SaveHard;
	RMGfxSourceBuffer16 *m_curThumb[6];
	RMString m_curThumbName[6];
	byte m_curThumbDiff[6];
	RMOptionButton *m_ButtonSave_States[6];
	RMOptionButton *m_ButtonSave_ArrowLeft;
	RMOptionButton *m_ButtonSave_ArrowRight;
	RMOptionButton *m_ButtonGfx_Tips;

	RMOptionButton *m_ButtonSound_DubbingOn;
	RMOptionButton *m_ButtonSound_MusicOn;
	RMOptionButton *m_ButtonSound_SFXOn;

	RMOptionSlide *m_SlideTonySpeed;
	RMOptionSlide *m_SlideTextSpeed;


	int m_statePos;
	bool m_bEditSaveName;
	int m_nEditPos;
	char m_EditName[256];

	union {
		RMOptionButton *m_ButtonGame_Lock;
		RMOptionButton *m_ButtonGfx_Anni30;
		RMOptionSlide *m_SliderSound_Music;
	};
	union {
		RMOptionButton *m_ButtonGame_TimerizedText;
		RMOptionButton *m_ButtonGfx_AntiAlias;
		RMOptionSlide *m_SliderSound_SFX;
	};
	union {
		RMOptionButton *m_ButtonGame_Scrolling;
		RMOptionButton *m_ButtonGfx_Sottotitoli;
		RMOptionSlide *m_SliderSound_Dubbing;
	};
	union {
		RMOptionButton *m_ButtonGame_InterUp;
		RMOptionButton *m_ButtonGfx_Trans;
	};

	int m_FadeStep;
	bool m_bExit;
	bool m_bQuitConfirm;
	int m_FadeY;
	int m_FadeTime;
	bool m_bLoadMenuOnly;
	bool m_bNoLoadSave;
	bool m_bAlterGfx;

	enum STATE {
	    MENUGAME,
	    MENUGFX,
	    MENUSOUND,
	    MENULOAD,
	    MENUSAVE
	};

	STATE m_nState;
	STATE m_nLastState;

public:
	RMOptionScreen();
	virtual ~RMOptionScreen();

	void Init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void InitLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void  InitSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void InitNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void ReInit(RMGfxTargetBuffer &bigBuf);
	bool Close();
	bool IsClosing();

	// Overloaded methods
	virtual int Priority();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Polling for the option screen
	void DoFrame(CORO_PARAM, RMInput *m_input);

	// Retrieves a savegame's thumbnail, description, and difficulty level
	static bool LoadThumbnailFromSaveState(int numState, byte *lpDestBuf, RMString &name, byte &diff);

protected:

	// Initialisation and state change
	void InitState(CORO_PARAM);
	void CloseState(void);
	void ChangeState(CORO_PARAM, STATE newState);

	// Repaint the options menu
	void RefreshAll(CORO_PARAM);
	void RefreshThumbnails(void);
};

} // End of namespace Tony

#endif
