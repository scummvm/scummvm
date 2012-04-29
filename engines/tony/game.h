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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Game.CPP.............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#ifndef TONY_GAME_H
#define TONY_GAME_H

#include "tony/adv.h"
#include "tony/gfxcore.h"
#include "tony/input.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Tony {

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
	void Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	
	// Sets the current co-ordinates
	void SetCoord(RMPoint pt) { m_pos = pt; }

	// Overloading of the method to see if rising from the list
	bool RemoveThis();

	// Sets a new action as current
	void SetAction(RMTonyAction action) {	m_nCurPointer = action; }

	// Sets a new pointer
	void SetSpecialPointer(POINTER ptr) {	
		m_nCurSpecialPointer=ptr; 
		if (m_nCurSpecialPointer && m_nCurSpecialPointer != PTR_CUSTOM) m_specialPointer[ptr-1]->SetPattern(1);  
	}
	POINTER GetSpecialPointer(void) { return (POINTER)m_nCurSpecialPointer; }

	// Set the new custom pointer
	void SetCustomPointer(RMGfxSourceBuffer8 *ptr) { m_nCurCustomPointer = ptr; }

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
	RMOptionButton(RMRect pt);
	virtual ~RMOptionButton();

	bool DoFrame(RMPoint mousePos, bool bLeftClick, bool bRightClick);
	void Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void AddToList(RMGfxTargetBuffer &bigBuf);
	bool IsActive() { return m_bActive; }
	void SetActiveState(bool bState) { m_bActive=bState; }
};

class RMOptionSlide : public RMGfxTaskSetPrior {
private:
	RMOptionButton* m_PushLeft;
	RMOptionButton* m_PushRight;
	RMGfxSourceBuffer16* m_SliderCenter;
	RMGfxSourceBuffer16* m_SliderLeft;
	RMGfxSourceBuffer16* m_SliderRight;
	RMGfxSourceBuffer16* m_SliderSingle;
	int m_nSlideSize;
	RMPoint m_pos;
	int m_nValue;
	int m_nMax;
	int m_nStep;

public:
	RMOptionSlide(RMPoint pt, int m_nRange=100, int m_nStartValue=0, int slideSize=300);
	virtual ~RMOptionSlide();

	bool DoFrame(RMPoint mousePos, bool bLeftClick, bool bRightClick);
	void Draw(RMGfxTargetBuffer& bigBuf, RMGfxPrimitive* prim);
	void AddToList(RMGfxTargetBuffer& bigBuf);

	int GetValue() { return m_nValue; }
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

	bool Init(RMGfxTargetBuffer& bigBuf);
	bool InitLoadMenuOnly(RMGfxTargetBuffer &bigBuf, bool bAlternateGfx = false);
	bool InitSaveMenuOnly(RMGfxTargetBuffer &bigBuf, bool bAlternateGfx = false);
	bool InitNoLoadSave(RMGfxTargetBuffer &bigBuf);
	void ReInit(RMGfxTargetBuffer &bigBuf);
	bool Close();
	bool IsClosing();

	// Metodi in overloading da RMGfxTask
	int Priority();
	void Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	bool RemoveThis();

	// Polling per l'option screen
	void DoFrame(RMInput *m_input);

protected:

	// Inizializza tutto per il nuovo stato
	void InitState(void);
	void CloseState(void);
	void ChangeState(STATE newState);

	// Ridisegna tutto il menu delle opzioni
	void RefreshAll(void);
	void RefreshThumbnails(void);
	
	// Carica lo screenshot per il salvataggio
	bool LoadThumbnailFromSaveState(int numState, byte *lpDestBuf, RMString &name, byte &diff);
};

} // End of namespace Tony

#endif
