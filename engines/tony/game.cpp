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
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Loc.CPP..............  *
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

#include "common/file.h"
#include "common/savefile.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpalutils.h"
#include "tony/mpal/stubs.h"
#include "tony/custom.h"
#include "tony/game.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

using namespace MPAL;

/****************************************/
/*  Variabili globali di configurazione */
/****************************************/

// FIXME: Move into a class instance
bool bCfgInvLocked;
bool bCfgInvNoScroll;
bool bCfgTimerizedText;
bool bCfgInvUp;
bool bCfgAnni30;
bool bCfgAntiAlias;
bool bCfgSottotitoli;
bool bCfgTransparence;
bool bCfgInterTips;
bool bCfgDubbing;
bool bCfgMusic;
bool bCfgSFX;
int  nCfgTonySpeed;
int  nCfgTextSpeed;
int	 nCfgDubbingVolume;
int	 nCfgMusicVolume;
int	 nCfgSFXVolume;
bool bSkipSfxNoLoop;

bool bPatIrqFreeze;

/****************************************/
/*  Funzioni globali per la DLL Custom	*/
/****************************************/

static char path_buffer[_MAX_PATH];
static char drive[_MAX_DRIVE];
static char dir[_MAX_DIR];
static char fname[_MAX_FNAME];
static char ext[_MAX_EXT];

HANDLE MainLoadLocation(int nLoc, RMPoint pt, RMPoint start) {
	return _vm->GetEngine()->LoadLocation(nLoc,pt,start);
}

HANDLE MainUnloadLocation(bool bDoOnExit) {
	return _vm->GetEngine()->UnloadLocation(bDoOnExit);
}

void MainLinkGraphicTask(RMGfxTask *task) {
	_vm->GetEngine()->LinkGraphicTask(task);
}

void MainFreeze(void) {
	_vm->GetEngine()->Freeze();
}

void MainUnfreeze(void) {
	_vm->GetEngine()->Unfreeze();
}

void MainWaitFrame(void) {
	WaitForSingleObject(_vm->m_hEndOfFrame, INFINITE);
}

void MainShowMouse(void) {
  _vm->GetEngine()->EnableMouse();
}

void MainHideMouse(void) {
  _vm->GetEngine()->DisableMouse();
}

void MainPlayMusic(int nChannel, char* filename, int nFX, bool bLoop, int nSync) {
	_vm->PlayMusic(nChannel, filename, nFX, bLoop, nSync);
}

void MainDisableInput(void) {
	_vm->GetEngine()->DisableInput();
}

void MainEnableInput(void) {
	_vm->GetEngine()->EnableInput();
}

void MainInitWipe(int type) {
	_vm->GetEngine()->InitWipe(type);
}

void MainCloseWipe(void) {
	_vm->GetEngine()->CloseWipe();
}

void MainWaitWipeEnd(void) {
	_vm->GetEngine()->WaitWipeEnd();
}

void MainEnableGUI(void) {
	_vm->GetEngine()->m_bGUIInterface = true;
	_vm->GetEngine()->m_bGUIInventory = true;
	_vm->GetEngine()->m_bGUIOption = true;
}

void MainDisableGUI(void) {
	_vm->GetEngine()->m_bGUIInterface = false;
	_vm->GetEngine()->m_bGUIInventory = false;
	_vm->GetEngine()->m_bGUIOption = false;
}

void MainSetPalesati(bool bPalesati) {
	_vm->GetEngine()->SetPalesati(bPalesati);
}

/****************************************************************************\
*       Metodi di RMOptionButton
\****************************************************************************/

RMOptionButton::RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState) {
	RMResRaw raw(dwRes);
	assert(raw.IsValid());
	m_buf = new RMGfxSourceBuffer16(false);
	m_buf->Init(raw, raw.Width(), raw.Height());

	m_rect.SetRect(pt.x, pt.y, pt.x + raw.Width() - 1, pt.y + raw.Height() - 1);
	m_bActive = false;
	m_bHasGfx = true;
	m_bDoubleState = bDoubleState;
}

RMOptionButton::RMOptionButton(RMRect pt) {
	m_rect = pt;
	m_bActive = false;
	m_bHasGfx = false;
	m_bDoubleState = false;
}

RMOptionButton::~RMOptionButton() {
	if (m_bHasGfx)
		delete m_buf;
}

bool RMOptionButton::DoFrame(RMPoint mousePos, bool bLeftClick, bool bRightClick) {
	if (!m_bDoubleState) {
		if (m_rect.PtInRect(mousePos)) {
			if (!m_bActive) {
				m_bActive=true;
				return true;
			}
		} else {
			if (m_bActive) {
				m_bActive=false;
				return true;
			}
		}
	} else {
		if (bLeftClick && m_rect.PtInRect(mousePos)) {	
			m_bActive = !m_bActive;
			return true;
		}
	}

	return false;
}



void RMOptionButton::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (!m_bActive)
		return;

	if (m_bHasGfx)
		m_buf->Draw(bigBuf,prim);
}

void RMOptionButton::AddToList(RMGfxTargetBuffer &bigBuf) {
	if (m_bHasGfx)
		bigBuf.AddPrim(new RMGfxPrimitive(this, m_rect));
}

} // End of namespace Tony
