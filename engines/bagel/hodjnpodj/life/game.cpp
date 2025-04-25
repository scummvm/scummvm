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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/life/game.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

#define RULES_TXT	"life.txt"
#define RULES_WAV "life/sound/rllf.wav"

Life::Life() : MinigameView("Life", "life/hnplife.dll") {
}

bool Life::msgOpen(const OpenMessage &msg) {
	MinigameView::msgOpen(msg);

	loadBitmaps();

	if (pGameParams->bMusicEnabled) {
		_gameSound = new CBofSound(this, "life/sound/life.mid", SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (_gameSound != nullptr) {
			(*_gameSound).midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
		}
	}

	return true;
}

bool Life::msgClose(const CloseMessage &msg) {
	clearBitmaps();

	MinigameView::msgClose(msg);

	delete _gameSound;
	_gameSound = nullptr;

	return true;
}

bool Life::msgFocus(const FocusMessage &msg) {
	return MinigameView::msgFocus(msg);
}

bool Life::msgGame(const GameMessage &msg) {
	MinigameView::msgGame(msg);

	if (msg._name == "NEW_GAME") {
		assert(!pGameParams->bPlayingMetagame);
		newGame();
		return true;
	}

	return false;
}

bool Life::msgMouseDown(const MouseDownMessage &msg) {
	return true;
}

bool Life::msgMouseMove(const MouseMoveMessage &msg) {
	return true;
}

bool Life::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_F1:
		// Bring up the Rules
		CBofSound::waitWaveSounds();
		Rules::show(RULES_TXT,
			(pGameParams->bSoundEffectsEnabled ? RULES_WAV : nullptr)
		);
		break;

	case Common::KEYCODE_F2:
		// Options menu
		showMainMenu();
		break;


	default:
		return MinigameView::msgKeypress(msg);
	}

	return true;
}

bool Life::msgAction(const ActionMessage &msg) {
	return true;
}

bool Life::tick() {
	MinigameView::tick();

	return true;
}

void Life::draw() {
	GfxSurface s = getSurface();

	// Draw background
	s.blitFrom(_background);

}

void Life::loadBitmaps() {
}

void Life::clearBitmaps() {
}

void Life::showMainMenu() {
	// Flag is not initially set so we can draw the maze
	// in standalone mode withouth the time remaining showing
	_playing = true;

	MainMenu::show(
		pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		RULES_TXT,
		pGameParams->bSoundEffectsEnabled ? RULES_WAV : NULL);
}

void Life::newGame() {
	// Flag the new game to start
	_playing = true;
	_gameOver = false;
}

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel
