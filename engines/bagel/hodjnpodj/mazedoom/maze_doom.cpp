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

#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/hodjnpodj/mazedoom/main_window.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

CBmpButton *m_pScrollButton;
CSprite *pPlayerSprite;
CPalette *pGamePalette,                   // Palette of current artwork 
	*pOldPal;
CBitmap *pMazeBitmap,
	*pOldBmp,
	*pWallBitmap,
	*pPathBitmap,
	*pStartBitmap,
	*pTopEdgeBmp,
	*pRightEdgeBmp,
	*pBottomEdgeBmp,
	*pLeftEdgeBmp,
	*TrapBitmap[NUM_TRAP_MAPS];
CDC *pMazeDC;                    // DC for the MazeBitmap
CText *m_pTimeText;              // Time to be posted in Locale box of screen
CBitmap *pLocaleBitmap,          // Locale of game bitmap for title bar
	*pBlankBitmap;               // Blank area of locale for time display

bool bSuccess;
bool m_bIgnoreScrollClick;
bool bPlaying;
bool m_bGameOver;
POINT m_PlayerPos;
uint m_nPlayerID;         // Hodj = 0, Podj = 4 to Offset the Bitmap ID for player

int m_nDifficulty;
int m_nTime, nSeconds, nMinutes;
int tempDifficulty;
int tempTime;

// Data type for each square of the underlying Grid of the Maze
struct TILE {
	POINT   m_nStart;   // Upper-left-hand corner where the bmp is to be drawn (24 X 24)
	uint    m_nWall;    // 0 = Path, 1 = Wall, 2 = Trap, etc...
	uint    m_nTrap;    // Index of trap bitmap to use for drawing
	POINT   m_nDest;    // x,y Tile location of Trap exit point 
	bool    m_bHidden;  // 0 = Visible, 1 = Invisible

	void clear() {
		m_nStart = POINT();
		m_nWall = 0;
		m_nTrap = 0;
		m_nDest = POINT();
		m_bHidden = false;
	}
} mazeTile[NUM_COLUMNS][NUM_ROWS];

static CSound *pGameSound;                             // Game theme song

LPGAMESTRUCT pGameInfo;

static void init() {
	m_pScrollButton = nullptr;
	pPlayerSprite = nullptr;
	pGamePalette = nullptr,                   // Palette of current artwork 
	pOldPal = nullptr;
	pMazeBitmap = nullptr;
	pOldBmp = nullptr;
	pWallBitmap = nullptr;
	pPathBitmap = nullptr;
	pStartBitmap = nullptr;
	pTopEdgeBmp = nullptr;
	pRightEdgeBmp = nullptr;
	pBottomEdgeBmp = nullptr;
	pLeftEdgeBmp = nullptr;
	Common::fill(TrapBitmap, TrapBitmap + NUM_TRAP_MAPS, nullptr);
	pMazeDC = nullptr;
	m_pTimeText = nullptr;
	pLocaleBitmap = nullptr;
	pBlankBitmap = nullptr;

	bSuccess = false;
	m_bIgnoreScrollClick = false;
	bPlaying = false;
	m_bGameOver = false;
	m_PlayerPos = POINT();
	m_nPlayerID = PODJ;         // Hodj = 0; Podj = 4 to Offset the Bitmap ID for player

	m_nDifficulty = 0;
	m_nTime = 0;
	nSeconds = 0;
	nMinutes = 0;
	tempDifficulty = 0;
	tempTime = 0;

	for (int x = 0; x < NUM_COLUMNS; ++x)
		for (int y = 0; y < NUM_ROWS; ++y)
			mazeTile[x][y].clear();
}

void run() {
	CMainWindow *pMain;
	pGameInfo = &g_engine->gGameInfo;
	init();

#if 0
	ghParentWnd = hParentWnd;
#endif

	if ((pMain = new CMainWindow) != nullptr) {

		pMain->ShowWindow(SW_SHOWNORMAL);
		pMain->UpdateWindow();
		pMain->SetActiveWindow();
	}
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
