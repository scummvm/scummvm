
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_SPACEBAR_MAIN_WINDOW_H
#define BAGEL_SPACEBAR_MAIN_WINDOW_H

#include "bagel/boflib/gfx/palette.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/boflib/list.h"
#include "bagel/baglib/pda.h"
#include "bagel/baglib/wield.h"
#include "bagel/spacebar/inv.h"
#include "bagel/spacebar/thud.h"

namespace Bagel {
namespace SpaceBar {

#define VRFILES               ".\\*.bmp"
#define VRDIR                 ".\\"
#define BMPFILES              ".\\BMP\\*.bmp"
#define BMPDIR                ".\\BMP\\"
#define SPRITEFILES           ".\\SPRITE\\*.bmp"
#define SPRITEDIR             ".\\SPRITE\\"
#define SOUNDFILES            ".\\SPRITE\\*.bmp"
#define SOUNDDIR              ".\\SPRITE\\"
#define LINKFILES             ".\\*.wld"
#define LINKDIR               ".\\"

//
//
// CMainWindow
//
class CMainWindow : public CBagPanWindow {
public:                                 // This must be changed
	static SBarThud *m_pThudBmp;    // Pointer to the THUD object

private:
	static int m_nInstances;    // Number of space bar windows
	static bool m_bZzazzlVision;        // if Zzazzl vision is on/off

	CBagMenu *m_pMenu;
	CBofPalette *m_pGamePalette;
	static CBofRect m_xFilterRect;
	CBofPoint m_cLastPos;
	CBofPoint m_cLastLoc;


#if BOF_WINDOWS
	HDC                 m_hDC;
#endif

public:

	enum GAMEMODE {
		VRPLAYMODE, VRBUILDMODE, VRPANMODE
	} m_nGameMode;

	//char* pointstabuff;
	static bool chipdisp;
	static int pause;
	//CBofRect srcTipRect;

	CMainWindow(const char *sCommandLine = nullptr);
	virtual ~CMainWindow();

	ERROR_CODE Attach();
	ERROR_CODE Detach();
	static bool GetZzazzlVision();
	static bool SetZzazzlVision(bool newValue);
	static CBofRect &GetFilterRect() {
		return m_xFilterRect;
	}
	static CBofRect &SetFilterRect(CBofRect &newValue) {
		m_xFilterRect = newValue;
		return m_xFilterRect;
	}

	CBagMenu *GetMenuPtr() {
		return m_pMenu;
	}
	CBofPalette *GetPalPtr() {
		return m_pGamePalette;
	}
	void SetPalPtr(CBofPalette *pal) {
		m_pGamePalette = pal;
	}

	GAMEMODE GameMode() {
		return m_nGameMode;
	}
	void SetMode(GAMEMODE mode) {
		m_nGameMode = mode;
	}

	ERROR_CODE SetLoadFilePos(const CBofPoint dstLoc);

	CBagObject *OnNewLinkObject(const CBofString &sInit);

	void CorrectZzazzlePoint(CBofPoint *p);

	ERROR_CODE OnCursorUpdate(int nCurrObj);

	void OnSysChar(uint32, uint32, uint32);
	void OnClose();
	void OnSize(uint32 nType, int cx, int cy);
	void OnKeyHit(uint32 lKey, uint32 lRepCount);
	void OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	void OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	void OnMouseMove(uint32 nFlags, CBofPoint *p, void * = nullptr);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
