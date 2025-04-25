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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_LIFE_GAME_H
#define HODJNPODJ_LIFE_GAME_H

#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

class Life : public MinigameView {
private:

	GfxSurface _background;
	bool _playing = false;
	bool _gameOver = false;
	CBofSound *_gameSound = nullptr;

	void resetFields();
	void loadBitmaps();
	void clearBitmaps();
	void playGame();
	void newGame();

protected:
	void showMainMenu() override;

public:
	Life();
	virtual ~Life() {
	}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel

#endif
