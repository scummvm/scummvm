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
 * aint32 with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_MAZE_DOOM_H
#define HODJNPODJ_MAZE_DOOM_H

#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define	NUM_TRAP_MAPS	 7							// There are seven trap icons available

class MazeDoom : public MinigameView {
private:
	const char *_upBmp = nullptr, *_downBmp = nullptr,
		*_leftBmp = nullptr, *_rightBmp = nullptr;
	BmpButton _scrollButton;
	GfxSurface _background;
	GfxSurface _locale, _blank;
	GfxSurface _mazeBitmap;
	GfxSurface _partsBitmap, pWallBitmap,
		pPathBitmap, pStartBitmap;
	GfxSurface pLeftEdgeBmp, pRightEdgeBmp,
		pTopEdgeBmp, pBottomEdgeBmp;
	GfxSurface TrapBitmap[NUM_TRAP_MAPS];
	GfxSurface pLocaleBitmap;
	GfxSurface pBlankBitmap;
	Sprite pPlayerSprite;
	const Common::Rect _timeRect;
	bool bSuccess = false;
	bool m_bIgnoreScrollClick = false;
	bool bPlaying = false;
	bool m_bGameOver = false;
	Common::Point m_PlayerPos;
	int m_nDifficulty = 0;
	int m_nTime = 0, nSeconds = 0, nMinutes = 0;
	int tempDifficulty = 0;
	int tempTime = 0;

	void setupHodjPodj();
	void loadBitmaps();
	void showMainMenu();
	void setupSettings();

public:
	MazeDoom();
	virtual ~MazeDoom() {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
