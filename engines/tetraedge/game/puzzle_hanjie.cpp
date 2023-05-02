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

#include "tetraedge/game/puzzle_hanjie.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

static const char *BG_NAMES[] = {"Amenta", "Croix", "Echelle", "Sang", "Trident"};

PuzzleHanjie::PuzzleHanjie() : _exitButton(nullptr), _entered(false), _foregroundSprite(nullptr), _backgroundNo(0) {
	ARRAYCLEAR(_sprites);
	ARRAYCLEAR(_expectedVals, false);
}

void PuzzleHanjie::wakeUp() {
	_timer.alarmSignal().add(this, &PuzzleHanjie::onWinTimer);
	_timer.start();

	TeInputMgr *inputMgr = g_engine->getInputMgr();
	// TODO: Set callback priority value using world transform here?
	inputMgr->_mouseLUpSignal.add(this, &PuzzleHanjie::onMouseUp);

	_gui.load("Texts/PuzzleHanjie.xml");

	TeButtonLayout *btn = _gui.button("blockButton");
	btn->setVisible(true);
	btn->setEnable(true);
	_foregroundSprite = _gui.sprite("Foreground");
	_exitButton = _gui.button("Exit");
	if (_exitButton) {
		_exitButton->onMouseClickValidated().add(this, &PuzzleHanjie::onExitButton);
	}

	for (uint i = 0; i < 7; i++) {
		for (uint j = 0; j < 7; j++) {
			Common::String sname = Common::String::format("Case%d-%d", i, j);
			_sprites[i * 7 + j] = _gui.sprite(sname);
		}
	}
	_backgroundNo = g_engine->getRandomNumber(4);
	_bgImg = Common::String::format("%s%s.png", _gui.value("Background").c_str(), BG_NAMES[_backgroundNo]);
	_bgSprite.load(_bgImg);
	_bgSprite.setPosition(TeVector3f32(0, 0, 220));
	_bgSprite.setVisible(true);

	for (uint row = 0; row < 7; row++) {
		const Common::String key = Common::String::format("Solution.%s%d", BG_NAMES[_backgroundNo], row);
		const Common::String data = _gui.value(key);
		Common::StringArray splitData = TetraedgeEngine::splitString(data, '-');
		if (splitData.size() != 7)
			error("Invalid puzzle data for %s: %s", key.c_str(), data.c_str());
		for (uint col = 0; col < 7; col++) {
			_expectedVals[row * 7 + col] = (splitData[col] == "1");
		}
	}

	if (_gui.group("Sounds")) {
		const Common::String begin = _gui.value("Sounds.Begin");
		if (!begin.empty())
			_soundBegin = Common::String::format("Sounds/Dialogs/%s", begin.c_str());
	}
	_entered = true;
}

void PuzzleHanjie::sleep() {
	TeInputMgr *inputMgr = g_engine->getInputMgr();
	inputMgr->_mouseLUpSignal.remove(this, &PuzzleHanjie::onMouseUp);

	_timer.alarmSignal().clear();
	_gui.unload();
	_bgSprite.setVisible(false);
	_bgSprite.unload();
	_entered = false;
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->warpY()->setVisible(true, false);
}

bool PuzzleHanjie::isSolved() {
	for (uint i = 0; i < ARRAYSIZE(_expectedVals); i++) {
		if (_expectedVals[i] != _sprites[i]->visible())
			return false;
	}
	return true;
}

bool PuzzleHanjie::onExitButton() {
	sleep();
	return false;
}

bool PuzzleHanjie::onWinTimer() {
	sleep();
	return false;
}

bool PuzzleHanjie::onMouseUp(const Common::Point &pt) {
	error("TODO: Implement PuzzleHanjie::onMouseUp");
}

} // end namespace Tetraedge
