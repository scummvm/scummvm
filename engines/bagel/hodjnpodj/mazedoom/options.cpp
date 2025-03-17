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

#include "common/config-manager.h"
#include "bagel/hodjnpodj/mazedoom/options.h"
#include "bagel/hodjnpodj/mazedoom/defines.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define LEFT_SIDE		 30

const int16 TIME_SCALES[10] = {
	30, 45, 60, 75, 90, 120, 180, 240, 300, 600
};

static const char *mDifficultyTable[10] = {
	"Total Wussy",
	"Big Sissy",
	"Major Wimp",
	"Minor Wimp",
	"Majorly Minor Miner",
	"Minor Miner",
	"Miner",
	"Major Miner",
	"Enchantemite Miner",
	"Studly Enchantemite Miner"
};

Options::Options() : View("MazeDoomOptions"),
		_time(TIMER_DEFAULT),
		_difficulty(DEFAULT_DIFFICULTY),
		_timeLeftRect(LEFT_SIDE, 25, LEFT_SIDE + 70, 40),
		_difficultyTitleRect(LEFT_SIDE, 45, LEFT_SIDE + 80, 70),
		_difficultyRect(LEFT_SIDE, 65, LEFT_SIDE + 170, 80),
		_timeRect(LEFT_SIDE, 115, LEFT_SIDE + 100, 130),
		_difficultyScroll("Difficulty", DialogRect(8, 15, 50, 65, 10), this),
		_timeScroll("Time", DialogRect(8, 15, 80, 65, 10), this),
		_okButton(DialogRect(8, 85, 59, 30, 14), this),
		_cancelButton(DialogRect(8, 85, 80, 30, 15), this) {

	_difficultyScroll.setScrollRange(MIN_DIFFICULTY, MAX_DIFFICULTY, 0);
	_timeScroll.setScrollRange(TIMER_MIN, TIMER_MAX - 1, 0);
}

bool Options::msgOpen(const OpenMessage &msg) {
	loadIniSettings();
	putDialogData();

	// Load the scroll background and center the view
	_background.loadBitmap("mazedoom/art/sscroll.bmp");
	_background.setTransparentColor(WHITE);

	Common::Rect r(0, 0, _background.w, _background.h);
	r.moveTo((GAME_WIDTH - _background.w) / 2,
		(GAME_HEIGHT - _background.h) / 2);
	setBounds(r);

	return View::msgOpen(msg);
}

bool Options::msgClose(const CloseMessage &msg) {
	_background.clear();
	return View::msgClose(msg);
}

bool Options::msgGame(const GameMessage &msg) {
	Common::String ctl = msg._stringValue;

	if (msg._name == "BUTTON") {
		if (ctl == "OK") {
			saveIniSettings();
			close();
		} else if (ctl == "CANCEL") {
			close();
		}

		return true;

	} else if (msg._name == "SCROLL") {
		if (ctl == "Time") {
			_time = msg._value;
			_hasChanges = true;
		} else if (ctl == "Difficulty") {
			_difficulty = msg._value;
			_hasChanges = true;
		}

		redraw();
		return true;
	}

	return false;
}

void Options::draw() {
	GfxSurface s = getSurface();
	Common::String text;
	s.setFontSize(8);

	// Copy the background
	s.blitFrom(_background);

	// Write the time left and scrollbars header text
	s.writeString(_timeStr, _timeLeftRect, BLACK);
	s.writeString("Level:", _difficultyTitleRect, BLACK);
	s.writeString(mDifficultyTable[_difficulty],
		_difficultyRect, BLACK);

	int mins = TIME_SCALES[_time] / 60;
	int secs = TIME_SCALES[_time] % 60;
	s.writeString(Common::String::format("Time Limit: %02d:%02d",
		mins, secs), _timeRect, BLACK);
}

void Options::putDialogData() {
	_difficultyScroll.setScrollPos(_difficulty);
	_timeScroll.setScrollPos(_time);
	_hasChanges = false;

	redraw();
}

void Options::loadIniSettings() {
	Common::String domain = ConfMan.getActiveDomainName();
	ConfMan.setActiveDomain("MazeDoom");

	_difficulty = !ConfMan.hasKey("Difficulty") ? DEFAULT_DIFFICULTY :
		CLIP(ConfMan.getInt("Difficulty"), MIN_DIFFICULTY, MAX_DIFFICULTY);
	_time = !ConfMan.hasKey("Time") ? TIMER_DEFAULT :
		CLIP(ConfMan.getInt("Time"), TIMER_MIN, TIMER_MAX);
	_hasChanges = false;

	ConfMan.setActiveDomain(domain);
}

void Options::saveIniSettings() {
	if (!_hasChanges)
		return;

	Common::String domain = ConfMan.getActiveDomainName();
	ConfMan.setActiveDomain("MazeDoom");

	ConfMan.setInt("Difficulty", _difficulty);
	ConfMan.setInt("Time", _time);

	ConfMan.setActiveDomain(domain);
	ConfMan.flushToDisk();
}

void Options::setTime(int nMinutes, int nSeconds) {
	_timeStr = Common::String::format("Time: %02d:%02d",
		nMinutes, nSeconds);
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
