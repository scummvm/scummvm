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
#include "got/data/high_scores.h"
#include "got/got.h"

namespace Got {

static const HighScore DEFAULTS[HIGH_SCORES_PER_AREA] = {
	{ 0, 10000, "Ron Davis" },
	{ 0,  9000, "Gary Sirois" },
	{ 0,  8000, "Adam Pedersen" },
	{ 0,  7000, "Jason Blochowiak" },
	{ 0,  6000, "Roy Davis" },
	{ 0,  5000, "Wayne Timmerman" },
	{ 0,  4000, "Dan Linton" }
};

void HighScore::sync(Common::Serializer &s) {
	s.syncAsUint16LE(_unused);
	s.syncAsUint32LE(_total);
	s.syncBytes((byte *)_name, 32);
}

void HighScores::sync(Common::Serializer &s) {
	for (int area = 0; area < 3; ++area)
		for (int num = 0; num < HIGH_SCORES_PER_AREA; ++num)
			_scores[area][num].sync(s);
}

void HighScores::load() {
	Common::File f;
	Common::InSaveFile *sf;
	Common::String scoresName = g_engine->getHighScoresSaveName();

	if ((sf = g_system->getSavefileManager()->openForLoading(scoresName)) != nullptr) {
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
		for (int area = 0; area < 3; ++area)
			for (int num = 0; num < HIGH_SCORES_PER_AREA; ++num)
				_scores[area][num] = DEFAULTS[num];
	}
}

void HighScores::save() {
	Common::OutSaveFile *sf = g_system->getSavefileManager()->openForSaving(
		g_engine->getHighScoresSaveName());
	if (!sf)
		error("Error creating high scores save file");

	Common::Serializer s(nullptr, sf);
	sync(s);
	sf->finalize();
	delete sf;
}

static int sortScores(const HighScore &hs1, const HighScore &hs2) {
	if (hs1._total < hs2._total)
		return -1;
	else if (hs1._total > hs2._total)
		return 1;
	else
		return 0;
}

void HighScores::add(int area, const Common::String &name, int total) {
	// Make a temporary copy of the area scores and add the new one
	Common::Array<HighScore> temp;
	for (int i = 0; i < HIGH_SCORES_PER_AREA; ++i)
		temp.push_back(_scores[area - 1][i]);

	temp.push_back(HighScore());
	Common::strcpy_s(temp.back()._name, name.c_str());
	temp.back()._total = total;

	// Sort the scores by descending total
	Common::sort(temp.begin(), temp.end(), sortScores);

	// Copy all but the lowest resulting score back into table
	for (int i = 0; i < HIGH_SCORES_PER_AREA; ++i)
		_scores[area - 1][i] = temp[i];

	// Save the resulting table
	save();
}

} // namespace Got
