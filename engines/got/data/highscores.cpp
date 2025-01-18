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
#include "common/savefile.h"
#include "common/system.h"
#include "got/data/highscores.h"
#include "got/got.h"

namespace Got {

static const char *DEFAULT_NAMES[HIGH_SCORES_PER_AREA] = {
	"Ron Davis", "Gary Sirois", "Adam Pedersen", "Jason Blochowiak",
	"Roy Davis", "Wayne Timmerman", "Dan Linton"};

void HighScore::sync(Common::Serializer &s) {
	s.syncAsUint16LE(_unused);
	s.syncAsUint32LE(_total);
	s.syncBytes((byte *)_name, 32);
}

void HighScores::sync(Common::Serializer &s) {
	for (int area = 0; area < 3; ++area) {
		for (int num = 0; num < HIGH_SCORES_PER_AREA; ++num)
			_scores[area][num].sync(s);
	}
}

void HighScores::load() {
	Common::File f;
	Common::String scoresName = g_engine->getHighScoresSaveName();
	Common::InSaveFile *sf = g_system->getSavefileManager()->openForLoading(scoresName);

	if (sf != nullptr) {
		// ScummVM high scores data present
		Common::Serializer s(sf, nullptr);
		sync(s);
		delete sf;
	} else if (f.open("config.got")) {
		// Otherwise fall back on original generated config.got
		f.seek(0x32);
		Common::Serializer s(&f, nullptr);
		sync(s);
	} else {
		// Generate new data
		for (int area = 0; area < 3; ++area) {
			for (int num = 0; num < HIGH_SCORES_PER_AREA; ++num)
				_scores[area][num] = HighScore(DEFAULT_NAMES[num], 10000 - (num * 1000));
		}
	}
}

void HighScores::save() {
	Common::OutSaveFile *sf = g_system->getSavefileManager()->openForSaving(g_engine->getHighScoresSaveName());
	if (!sf)
		error("Error creating high scores save file");

	Common::Serializer s(nullptr, sf);
	sync(s);
	sf->finalize();
	delete sf;
}

void HighScores::add(int area, const Common::String &name, uint total) {
	// Find the index for the new score in the list
	int newIndex;
	for (newIndex = 0; newIndex < HIGH_SCORES_PER_AREA && total < _scores[area - 1][newIndex]._total; ++newIndex) {
	}
	
	if (newIndex == HIGH_SCORES_PER_AREA)
		// Lower than all current scores, so ignore it
		return;

	// Shift any lower scores to make space
	for (int i = HIGH_SCORES_PER_AREA - 1; i > newIndex; --i)
		_scores[area - 1][i] = _scores[area - 1][i - 1];

	// Insert in new score
	HighScore &hs = _scores[area - 1][newIndex];
	Common::fill(hs._name, hs._name + 32, 0);
	Common::strcpy_s(hs._name, name.c_str());
	hs._total = total;

	// Save the resulting table
	save();
}

} // namespace Got
