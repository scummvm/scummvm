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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dune/dune.h"
#include "dune/resources.h"
#include "dune/sprite_sheet.h"
#include "dune/video.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

namespace Dune {

bool DuneEngine::showingDefeat() const {
	return _defeatReason && !_state.empty() && (int8)_state[0x46d9] < 0;
}

// Never let a cinematic input become a room/map command. Quit still works
// throughout both the initial flash and the movie, including fast diagnostics.
bool DuneEngine::waitDefeat(uint32 milliseconds, bool shortenOnInput) {
	const uint32 started = _system->getMillis();
	do {
		Common::Event event;
		bool input = false;
		while (_system->getEventManager()->pollEvent(event)) {
			updatePointerClickKeys(event);
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				quitGame();
			// E353 polls DD63 during each fade step: a new key or mouse press
			// ends this wait, but does not skip the following palette steps.
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_RBUTTONDOWN || event.type == Common::EVENT_MBUTTONDOWN)
				input = true;
		}
		updateGameMusic();
		tick(false); // EF6A keeps running; defeat suppresses1B0D, not IRQ time.
		{
			const AnimationClock::Poll poll(_animationClock, _system->getMillis());
			updateSkyFade(); // DD63 also services an already registered3916.
		}
		if (shouldQuit())
			return false;
		if (ConfMan.getBool("dune_nodelay") || (shortenOnInput && input))
			break;
		_system->delayMillis(5);
	} while (_system->getMillis() - started < milliseconds);
	return true;
}

/** D45: three flashes from SKY[40], fading toward the current hour. */
void DuneEngine::flashDefeatPalette() {
	// 39EC defaults to SKYDN; only the sietch balcony (1005) uses SKY.
	const bool wide = READ_LE_UINT16(_state.begin() + 4) != 0x1005 || _inDesert;
	_state[0x22e3] = wide;
	const uint first = wide ? 73 : 128, count = wide ? 151 : 80;
	byte flash[151 * 3];
	if (!loadSkyFadeTarget(wide, skyPaletteForHour()) ||
			!currentSpriteSheet->read(40, 6, flash, count * 3))
		return;
	_state[0x227d]++;
	for (uint burst = 0; burst < 3 && !shouldQuit(); burst++) {
		_state[0x46d6] = 40; // 3971 selects the flash without replacing39B9's target.
		applyPalette(first, count, flash);
		_system->updateScreen();
		_state[0x46d7] = 32;
		// 391D/driver 175 use signed truncation, then decrement the divisor.
		// The first two bursts stop at 15; the final one fades all the way.
		do {
			stepSkyFade(); // Same displayed driver page/divisor as an overlapping3916.
			_system->updateScreen();
			if (!waitDefeat(15, true))
				break;
		} while (_state[0x46d7] >= (burst == 2 ? 1 : 16));
	}
	_state[0x227d]--;
}

/** DOS movie ids 12/13 are DEAD/DEAD2, not native enum values 12/13. */
void DuneEngine::playDefeatMovie(bool restarting) {
	const uint dosMovie = restarting ? 13 : 12;
	const uint movie = restarting ? HNM_DEAD2 : HNM_DEAD;
	const uint table = READ_LE_UINT16(_state.begin() + 0x33a3 + 2 * dosMovie);
	const uint32 frameMs = 5 * _state[table + 1]; // CAD4: resource tick interval.
	Resource resource = _archive.openMember(getResourceFilenameByIndex(INDEX_FIRST_HNM + movie));
	if (!resource)
		return;
	_openMovie = 0xffff;
	_video->setReader(resource);
	_video->setInterlace(false);
	_video->start();
	while (!_video->done() && !shouldQuit()) {
		_graphics.setFrameBuffer(_video->_frameBuffer);
		_video->decodeAVFrame();
		memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kSceneHeight);
		_graphics.setFrameBuffer(_framebuffer1);
		drawFace(); // C4F0 overlays the current panel phase on the movie.
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		// C8FB first runs CA1B/C4DD, then BP=181E from E66/E6C.
		// Both DEAD and DEAD2 present their first frame and palette before
		// lowering the face; later frames keep the lowered phase.
		if (_video->_currentFrame == 1)
			lowerRetainedPanelFace();
		if (!waitDefeat(frameMs))
			break;
	}
	_video->stopAudio();
	_graphics.setFrameBuffer(_framebuffer1);
}

/** DE8..E1B: the framed pair at the end of the selected character bank. */
void DuneEngine::drawDefeatPortrait(byte reason) {
	const byte *box = _state.begin() + 0x2254;
	const int left = READ_LE_UINT16(box), top = READ_LE_UINT16(box + 2);
	const int right = READ_LE_UINT16(box + 4), bottom = READ_LE_UINT16(box + 6);
	_graphics.fillRect(left, top, right, bottom, box[8]);
	_graphics.fillRect(left + 1, top + 1, right - 1, bottom - 1, box[9]);
	const int person = reason < 8 ? (int8)_state[0x225d + reason] : -1;
	if (person < 0)
		return;
	openSpriteSheet(getResourceFilenameByIndex(person + 2));
	const int first = currentSpriteSheet->entryCount() - 3;
	if (first >= 0) {
		drawSprite(first, left + 1, top + 1);
		drawSprite(first + 1, left + 1, top + 1);
	}
}

void DuneEngine::playDefeatSpeech() {
	const int person = _defeatReason < 8 ? (int8)_state[0x225d + _defeatReason] : -1;
	if (person < 0)
		return;
	// A6D1 toggles the 1000 bit, selecting FFF and FFFB. These are not
	// normal dialogue lines and must not subtract the person's phrase base.
	const char letter = 'A' + person;
	const char *suffix = _defeatVoiceAlternate ? "B" : "";
	_defeatVoiceAlternate = !_defeatVoiceAlternate;
	const bool indoors = (int8)_state[0xea] <= 0 && _state[6] == 0x80 && _state[4] != 1;
	for (uint attempt = 0; attempt < 2; attempt++) {
		const char take = (indoors != (attempt != 0)) ? 'I' : 'O';
		const Common::String name = Common::String::format("P%c\\P%cFFF%c%s.VOC", letter, letter, take, suffix);
		Resource voice = _archive.openMember(name);
		if (voice) {
			playVoice(voice, name);
			return;
		}
	}
}

bool DuneEngine::updateDefeat() {
	// A remote report is inside DDB0/DD63, which does not re-enter D83E's
	// defeat dispatcher until acknowledgement or the reading timeout.
	if (_menuKind == kMenuTroopNotice)
		return false;
	if (_state.empty() || !_state[0x46d9] || (int8)_state[0x46d9] < 0 || _state[0x47a9])
		return false;
	_state[0x473b] = _state[0x46ec] = 0;
	if (_onWorld || _onGlobe || _onMixer || _inCabin) {
		finishSurveyDrag(false);
		if (hasTroopContact())
			finishTroopContact();
		_onWorld = _onGlobe = _onMixer = _inCabin = false;
		_state[0xfb] = 1;
		_state[0x46eb] = 0;
		_speakerPerson = -1;
		_speech.clear();
		_menuKind = kMenuRoom;
		showRoomMenu();
		return true; // DAD/186B: leave the map, then present on the next poll.
	}
	_defeatPlaying = true;
	_defeatReason = _state[0x46d9];
	stopSpeech();
	stopBattleAnimation();
	stopFlightMovie();
	CursorMan.showMouse(false);
	flashDefeatPalette();
	WRITE_LE_UINT16(_state.begin() + 2, 0x16c5);
	playTheRightSong();
	if (!shouldQuit())
		playDefeatMovie(false);
	if (!shouldQuit()) {
		_state[0x11ca] = 1;
		WRITE_LE_UINT16(_state.begin() + 0x47c4, 0xffff);
		_state[0x46d9] = 0x80;
		_speakerPerson = -1;
		_speech = command(0xb9 + _defeatReason); // CF70 decrements DOS BA + reason.
		WRITE_LE_UINT16(_state.begin() + 0x4780, 0xba + _defeatReason);
		drawDefeatText();
		stopSkyFade(); // 0DEE/DA53, after the movie/text box, before the final portrait.
		drawDefeatPortrait(_defeatReason);
		_state[0x4727] = 0;
		_state[0x47a7] = 1;
		// D741 erases the compass, leaving the existing clock/panel intact.
		const byte *box = _state.begin() + 0x2458;
		_graphics.fillRect(READ_LE_UINT16(box), READ_LE_UINT16(box + 2),
				READ_LE_UINT16(box + 4), READ_LE_UINT16(box + 6), 0xf0);
		_defeatScreen.resize(kScreenWidth * kScreenHeight);
		memcpy(_defeatScreen.begin(), _framebuffer1, _defeatScreen.size());
		_system->getPaletteManager()->grabPalette(_defeatPalette, 0, 256);
		showDefeatMenu();
		playDefeatSpeech();
	}
	_defeatPlaying = false;
	setCursor(Cursor::Pointer);
	CursorMan.showMouse(true);
	return true;
}

void DuneEngine::drawDefeatScreen() {
	if (_defeatScreen.size() != kScreenWidth * kScreenHeight)
		return;
	memcpy(_framebuffer1, _defeatScreen.begin(), _defeatScreen.size());
	_graphics.setFrameBuffer(_framebuffer1);
	_system->getPaletteManager()->setPalette(_defeatPalette, 0, 256);
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

void DuneEngine::showDefeatMenu(bool leaving) {
	const uint16 rows[] = {185, 179, 186, 148}; // DS:20A2, zero-based command indices.
	const uint16 exitRows[] = {183, 184}; // DS:20B6.
	Common::Array<Common::String> choices;
	_menuParts.clear();
	for (uint i = 0; i < (leaving ? ARRAYSIZE(exitRows) : ARRAYSIZE(rows)); i++) {
		const uint16 commandIndex = leaving ? exitRows[i] : rows[i];
		choices.push_back(command(commandIndex));
		_menuParts.push_back(commandIndex);
	}
	_menuKind = leaving ? kMenuDefeatExit : kMenuDefeat;
	showMenu(choices);
}

void DuneEngine::pollDefeatMenu() {
	Common::Event event;
	bool idlePointerSample = true;
	while (pollPointerEvent(event, idlePointerSample)) {
		if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
			quitGame();
		else if (event.type == Common::EVENT_MAINMENU ||
				(event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
			// MAINMENU was handled by the event manager; only our Escape
			// shortcut still needs to open the dialog here.
			if (event.type == Common::EVENT_KEYDOWN)
				openMainMenuDialog();
			resetPointerButtons();
			redraw();
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			pressAt(event.mouse);
		}
		if (!showingDefeat() || shouldQuit())
			return;
	}
	const Common::Point pointer = _system->getEventManager()->getMousePos();
	const int row = pointer.y >= kMenuTop ? menuRowAt(pointer) : -1;
	if (row != _menuUnder) {
		_menuUnder = row;
		drawDefeatScreen();
	}
	updateGameMusic();
	_system->delayMillis(10);
}

void DuneEngine::restartAfterDefeat() {
	_defeatPlaying = true;
	stopSpeech();
	playDefeatMovie(true); // E47 clears the latch only after the reverse movie.
	_defeatPlaying = false;
	if (shouldQuit() || !loadStaticData())
		return;
	_defeatReason = 0;
	_defeatScreen.clear();
	_onWorld = _onGlobe = _onMixer = _inCabin = _inDesert = false;
	_cabinFromMap = _takingOff = _mixerOrdering = false;
	_cabinTyped = 0;
	_cabinPointerPoint = _wormPointerPoint = _pointedAtPlace = _pressedPlace = -1;
	_flyingTo = _guidedFlightPlace = _desertEntryWay = -1;
	_flightDiscoveryPrompt = _flightToPoint = _flightSteered = false;
	_flightTargetLongitude = _flightTargetLatitude = _flightHeading = 0;
	_nextFlightStep = _flightFrame = _cameFromPlace = 0;
	_flightTrail.clear();
	_hasSpiceDensityMap = false;
	_troopOrdersFromMap = _surveyDragging = false;
	_orderingTroop = _troopInfoPage = _mapAnimationFrame = 0;
	clearMapUnits();
	_mapAnimationActive = false;
	_mapAnimationRemainder = 0;
	_troopContactTop = _surveyLongitude = _surveyLatitude = 0;
	_troopWindowGeometryValid = false;
	_troopRects.clear();
	_moveDestinations.clear();
	memset(_troopRoutes.begin(), 0, _troopRoutes.size());
	_worldLongitude = _worldLatitude = _globeSpin = _globeTilt = _frescoDoors = 0;
	for (uint part = 0; part < _dialogue.count(); part++) {
		Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
		for (uint line = 0; line < records.size(); line++)
			records[line].flags = records[line].baseFlags;
	}
	_speech.clear();
	_speechParts.clear();
	_speechPart = 0;
	_speakerPerson = _listAs = -1;
	_speakerFrame = _speakerAnimation = _speakerReport = 0;
	_dialogueMask = 0x80;
	_dialoguePart = 1;
	_dialogueLine = _dialogue.count() > 1 && !_dialogue.entry(1).empty()
			? _dialogue.entry(1).size() - 1 : 0;
	_lastClockTime = _hourElapsedMs = _nextMouthTime = 0;
	_troopNoticeRemaining = _troopNoticeTime = _troopNoticeButtons = 0;
	_state[0x115]++;
	_state[0x3810] = 0;
	if (_music)
		_state[0x2943] &= ~0x10;
	resetPanel();
	introScene(0, 10, READ_LE_UINT16(_state.begin() + 0x12), -1);
	showRoomMenu();
	raiseOpeningPanelFace(); // B3B0's new-game return follows1860/17E6.
	playTheRightSong();
}

} // End of namespace Dune
