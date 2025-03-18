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
#include "bagel/hodjnpodj/mazedoom/maze_gen.h"
#include "bagel/hodjnpodj/mazedoom/options.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

class MazeDoom : public MinigameView, public MazeGen {
	struct Move {
		Common::Point _newPosition, _step;
		Common::Point _hit;
		bool _walking = false;
		int _substepCtr = 0;

		void clear() {
			_hit = _newPosition = _step = Common::Point();
			_substepCtr = 0;
			_walking = false;
		}
		void walk() {
			_walking = true;
		}
		bool isWalking() const {
			return _walking;
		}
	};
private:
	Options _options;
	Sprite _upBitmap, _downBitmap,
		_leftBitmap, _rightBitmap;
	BmpButton _scrollButton;
	GfxSurface _background;
	GfxSurface _partsBitmap;
	GfxSurface TrapBitmap[NUM_TRAP_MAPS];
	GfxSurface pLocaleBitmap;
	GfxSurface pBlankBitmap;
	Sprite pPlayerSprite;
	const Common::Rect _timeRect;
	bool bSuccess = false;
	bool m_bIgnoreScrollClick = false;
	bool bPlaying = false;
	bool m_bGameOver = false;
	uint32 _priorTime = 0;
	int m_nTime = 0, nSeconds = 0, nMinutes = 0;
	CBofSound *pGameSound = nullptr;
	Move _move;

	void setupHodjPodj();
	void loadBitmaps();
	void loadIniSettings();
	void newGame();
	void updateTimer();
	void gameOver();
	void exitCheck();
	void setupMaze();

	/**
	 * Mouse movement processing function
	 *
	 *  FORMAL PARAMETERS:
	 *
	 *      UINT nFlags     Virtual key info
	 *      Point point     Location of cursor
	 */
	void movePlayer(const Common::Point &point);

	/**
	 * Converts a point in screen coordinates to x & y location on maze grid
	 *
	 *  FORMAL PARAMETERS:
	 *
	 *      Point pointScreen      a point in screen coordinates
	 *
	 *  RETURN VALUE:
	 *
	 *      Point  point           the x & y grid coordinates where pointScreen fell
	 */
	Common::Point screenToTile(const Common::Point &pointScreen) const;

	void playerWalk1();
	void playerWalk2();
	void playerWalk3();

	/**
	 * Checks to see if a point is within the Artwork region of the window
	 *
	 *  FORMAL PARAMETERS:
	 *
	 *      Point point    The point to check
	 *
	 *  RETURN VALUE:
	 *
	 *      BOOL:   TRUE if point is within the Art Region,
	 *              FALSE if point is outside the Art Region
	 */
	bool inArtRegion(const Common::Point &point) const;

	/**
	 * Loads up a new cursor with regard to the current cursor position, and the player position
	 */
	void getNewCursor(const Common::Point &mousePos);

protected:
	void showMainMenu() override;

public:
	MazeDoom();
	virtual ~MazeDoom() {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
