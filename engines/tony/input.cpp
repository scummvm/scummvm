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
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Input.CPP............  *
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

#include "tony/gfxEngine.h"

namespace Tony {

#define DIRELEASE(x)		if (x) { (x)->Release(); (x)=NULL; }

void RMInput::DIInit(/*HINSTANCE hInst*/) {
#ifdef REFACTOR_ME
	HRESULT err;

	// Crea l'oggetto madre
	err = DirectInputCreate(hInst, DIRECTINPUT_VERSION, &m_DI, NULL);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error initializing DirectInput!","RMInput::DIInit()",MB_OK);
		assert(0);
	}

	// Crea il device del mouse
	// ************************
	err=m_DI->CreateDevice(GUID_SysMouse,&m_DIDMouse,NULL);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error creating mouse device!","RMInput::DIInit()",MB_OK);
		assert(0);
	}
	
	// Setta il data format
	err=m_DIDMouse->SetDataFormat(&c_dfDIMouse);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error setting mouse data format!","RMInput::DIInit()",MB_OK);
		assert(0);
	}		

	// Setta il cooperative level
	err=m_DIDMouse->SetCooperativeLevel(theGame.m_wnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error setting cooperative level!","RMInput::DIInit()",MB_OK);
		assert(0);
	}	

	// Crea il device della tastiera
	// *****************************
	err=m_DI->CreateDevice(GUID_SysKeyboard,&m_DIDKeyboard,NULL);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error creating keyboard device!","RMInput::DIInit()",MB_OK);
		assert(0);
	}
	
	// Setta il data format
	err=m_DIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error setting keyboard data format!","RMInput::DIInit()",MB_OK);
		assert(0);
	}	

	// Setta il cooperative level
	err=m_DIDKeyboard->SetCooperativeLevel(theGame.m_wnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if (err!=DI_OK) {
		MessageBox(theGame.m_wnd,"Error setting cooperative level!","RMInput::DIInit()",MB_OK);
		assert(0);
	}
#endif
}

void RMInput::SwitchFullscreen(bool bFull) {
#ifdef REFACTOR_ME
	HRESULT err;
	
	// Il camping del mouse e' attivo solo in fullscreen
	m_bClampMouse=bFull;
		
	// Prima di tutto leviamo l'acquiring ai device, altrimenti non possiamo cambiare il
	//  cooperative level
	Unacquire();

	if (bFull) {
		// Setta il cooperative level
		err=m_DIDMouse->SetCooperativeLevel(theGame.m_wnd,DISCL_FOREGROUND|DISCL_EXCLUSIVE);
		if (err!=DI_OK) {
			MessageBox(theGame.m_wnd,"Error setting cooperative level!","RMInput::DIInit()",MB_OK);
			assert(0);
		}	
	} else {
		// Setta il cooperative level
		err=m_DIDMouse->SetCooperativeLevel(theGame.m_wnd,DISCL_FOREGROUND|DISCL_EXCLUSIVE);
		if (err!=DI_OK) {
			MessageBox(theGame.m_wnd,"Error setting cooperative level!","RMInput::DIInit()",MB_OK);
			assert(0);
		}	
	}
#endif
}

void RMInput::DIClose(void)
{
#ifdef REFACTOR_ME
	DIRELEASE(m_DIDMouse);
	DIRELEASE(m_DI);
#endif
}


bool RMInput::Acquire(void) {
#ifdef REFACTOR_ME
	HRESULT err;
	
	if (m_DIDMouse == NULL)
		return true;

	// Inizializza la posizione del mouse
	POINT pt;
	RECT rc;
	GetWindowRect(theGame.m_wnd,&rc);
	GetCursorPos(&pt);
	m_mX=pt.x-rc.left-GetSystemMetrics(SM_CXDLGFRAME);
	m_mY=pt.y-rc.top-(GetSystemMetrics(SM_CYDLGFRAME)+GetSystemMetrics(SM_CYCAPTION));

	err=m_DIDMouse->Acquire();
	if (err!=DI_OK)
		return false;

	err=m_DIDKeyboard->Acquire();
	if (err!=DI_OK)
		return false;
#endif
	return true;
}

void RMInput::Unacquire(void) {
#ifdef REFACTOR_ME
	if (m_DIDMouse!=NULL)
		m_DIDMouse->Unacquire();
	
	if (m_DIDKeyboard!=NULL)
		m_DIDKeyboard->Unacquire();
#endif
}


void RMInput::GDIControl(bool bCon) {
#ifdef REFACTOR_ME
	if (bCon) {
		Unacquire();
		ShowCursor(true);
	} else {
		ShowCursor(false);
		Acquire();
	}
#endif
}

void RMInput::Poll(void) {
#ifdef REFACTOR_ME
	HRESULT err;
	bool mLastLeft, mLastRight;

	mLastLeft=MouseLeft();
	mLastRight=MouseRight();
	m_bLeftClickMouse=m_bLeftReleaseMouse=m_bRightClickMouse=m_bRightReleaseMouse=false;

	// Legge lo stato del mouse
	while (1) {
		ZeroMemory(&m_mState,sizeof(m_mState));
		err=m_DIDMouse->GetDeviceState(sizeof(m_mState),&m_mState);
		if (err==DI_OK)
			break;
		else if (err==DIERR_INPUTLOST || err==DIERR_NOTACQUIRED) {
			ZeroMemory(&m_mState,sizeof(m_mState));
			// Se l'acquire non funziona (ad esempio, quando siamo in background, allora lasciamo perdere
			if (!Acquire())
				return;
		}
		else
			break;
	}

	// Aggiorna le coordinate del mouse
	m_mX+=m_mState.lX;
	m_mY+=m_mState.lY;

	// Clamp dei valori sui bordi della finestra
	if (m_bClampMouse) {
		if (m_mX<0) m_mX=0;
		if (m_mY<0) m_mY=0;
		if (m_mX>=640) m_mX=639;
		if (m_mY>=480) m_mY=479;
	}

	// Controlla se e' cambiato lo stato dei bottoni
	if (mLastLeft && !MouseLeft())
		m_bLeftReleaseMouse=true;

	if (!mLastLeft && MouseLeft())
		m_bLeftClickMouse=true;

	if (mLastRight && !MouseRight())
		m_bRightReleaseMouse=true;

	if (!mLastRight && MouseRight())
		m_bRightClickMouse=true;
#endif
}


RMInput::RMInput() {
	m_bClampMouse = false;	
}

RMInput::~RMInput() {
	
}

void RMInput::Init(/*HINSTANCE hInst*/) {
#ifdef REFACTOR_ME
	DIInit(hInst);
#ifdef STARTFULLSCREEN
	SwitchFullscreen(true);
#endif
	Acquire();
#endif
}

void RMInput::Close(void) {
	Unacquire();
	DIClose();
}


bool RMInput::MouseLeft() {
	return false;
#ifdef REFACTOR_ME
	return (m_mState.rgbButtons[0] & 0x80);
#endif
}

bool RMInput::MouseRight() {
	return false;
#ifdef REFACTOR_ME
	return (m_mState.rgbButtons[1] & 0x80);
#endif
}

} // End of namespace Tony
