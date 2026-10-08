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

#ifndef SCUMM_INSANE_MORTIMER_SCORES_H
#define SCUMM_INSANE_MORTIMER_SCORES_H

#include "common/keyboard.h"
#include "scumm/insane/mortimer/text.h"

namespace Common {
class SaveFileManager;
}

namespace Scumm {

struct MortimerScoring {
	uint32 stone, initial, castle;
};

const MortimerScoring &mortimerScoring(uint flightDifficulty, uint riddleDifficulty);

class MortimerHighScores {
public:
	bool load(Common::SaveFileManager &saveFileMan, const Common::String &fileName);
	void start(uint32 score, bool allowEntry);
	bool handleKey(const Common::KeyState &key);
	bool save(Common::SaveFileManager &saveFileMan, const Common::String &fileName);
	bool enteringName() const { return _enteringName; }
	void draw(Graphics::Surface &screen, uint32 time) const;

private:
	struct Entry {
		char name[20];
		uint32 score;
	};

	bool read(Common::SeekableReadStream &stream);
	Common::String ranking(uint32 score) const;
	void drawText(Graphics::Surface &screen, const Common::String &text, int x, int y, byte color,
		Graphics::TextAlign align) const;

	Entry _entries[10];
	MortimerFont _font;
	MortimerSubtitles _text;
	uint32 _score = 0;
	int _selected = -1;
	bool _enteringName = false, _didNotQualify = false;
};

} // End of namespace Scumm

#endif
