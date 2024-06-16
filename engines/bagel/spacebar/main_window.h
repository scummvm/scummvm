
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
public:
	static SBarThud *_pThudBmp;	// Pointer to the THUD object

private:
	static int _nInstances;		// Number of space bar windows
	static bool _bZzazzlVision;	// If Zzazzl vision is on/off

	CBagMenu *_pMenu;
	CBofPalette *_pGamePalette;
	static CBofRect *_xFilterRect;
	CBofPoint _cLastPos;
	CBofPoint _cLastLoc;

public:
	enum GameMode {
		VRPLAYMODE, VRBUILDMODE, VRPANMODE
	} _nGameMode;

	static bool chipdisp;
	static int pause;

	static void initialize();
	static void shutdown();

	CMainWindow();
	virtual ~CMainWindow();

	ErrorCode attach() override;
	ErrorCode detach() override;
	static void setZzazzlVision(bool newValue);
	static CBofRect &getFilterRect() {
		return *_xFilterRect;
	}
	static CBofRect &setFilterRect(CBofRect &newValue) {
		*_xFilterRect = newValue;
		return *_xFilterRect;
	}

	CBagMenu *getMenuPtr() {
		return _pMenu;
	}
	CBofPalette *getPalPtr() {
		return _pGamePalette;
	}
	void setPalPtr(CBofPalette *pal) {
		_pGamePalette = pal;
	}

	GameMode gameMode() {
		return _nGameMode;
	}
	void setMode(GameMode mode) {
		_nGameMode = mode;
	}

	ErrorCode setloadFilePos(CBofPoint dstLoc) override;

	CBagObject *onNewLinkObject(const CBofString &sInit) override;

	void correctZzazzlePoint(CBofPoint *p);

	ErrorCode onCursorUpdate(int nCurrObj) override;

	void onClose() override;
	void onSize(uint32 nType, int cx, int cy) override;
	void onKeyHit(uint32 lKey, uint32 lRepCount) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void * = nullptr) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void * = nullptr) override;
	void onMouseMove(uint32 nFlags, CBofPoint *p, void * = nullptr) override;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
