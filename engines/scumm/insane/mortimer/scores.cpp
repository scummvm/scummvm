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

#include "scumm/insane/mortimer/scores.h"
#include "scumm/insane/highscores.h"

#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "graphics/surface.h"

namespace Scumm {

const MortimerScoring kMortimerScoring[] = {
	{100, 0, 2000}, {1000, 3000, 12000}, {3500, 75000, 75000}
};

const MortimerScoring &mortimerScoring(uint flightDifficulty, uint riddleDifficulty) {
	// Scoring uses the integer mean of the two difficulty settings.
	return kMortimerScoring[(MIN(flightDifficulty, 2U) + MIN(riddleDifficulty, 2U)) / 2];
}

bool MortimerHighScores::read(Common::SeekableReadStream &stream) {
	if (stream.size() != 240)
		return false;
	for (uint i = 0; i < ARRAYSIZE(_entries); ++i) {
		Entry &entry = _entries[i];
		if (stream.read(entry.name, sizeof(entry.name)) != sizeof(entry.name) ||
			!memchr(entry.name, 0, sizeof(entry.name)))
			return false;
		entry.score = stream.readUint32LE();
		if (stream.err() || stream.eos() || (i && entry.score > _entries[i - 1].score))
			return false;
	}
	return true;
}

bool MortimerHighScores::load(Common::SaveFileManager &saveFileMan, const Common::String &fileName) {
	Common::File text;
	if (!text.open("RESOURCE/DATA/HISCORE.TRS") || !_text.loadText(text) ||
		!_font.load("RESOURCE/GENERAL/ESTRO20.NUT"))
		return false;
	Common::ScopedPtr<Common::InSaveFile> saved(saveFileMan.openForLoading(fileName));
	if (saved && read(*saved))
		return true;
	Common::File defaults;
	return defaults.open("RESOURCE/DATA/MORTIMER.HSF") && read(defaults);
}

void MortimerHighScores::start(uint32 score, bool allowEntry) {
	_score = score;
	_selected = allowEntry ? findHighScorePosition(_entries, ARRAYSIZE(_entries), score, true) : -1;
	_enteringName = _selected >= 0 && _selected < (int)ARRAYSIZE(_entries);
	_didNotQualify = allowEntry && !_enteringName;
	if (_enteringName) {
		Entry entry = {};
		entry.score = score;
		insertHighScoreEntry(_entries, ARRAYSIZE(_entries), ARRAYSIZE(_entries), _selected, entry);
	} else {
		_selected = -1;
	}
}

bool MortimerHighScores::handleKey(const Common::KeyState &key) {
	if (!_enteringName || (key.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META)))
		return false;
	char *name = _entries[_selected].name;
	const uint length = strlen(name);
	if (key.keycode == Common::KEYCODE_RETURN || key.keycode == Common::KEYCODE_KP_ENTER)
		return true;
	if (key.keycode == Common::KEYCODE_ESCAPE) {
		name[0] = 0;
	} else if (key.keycode == Common::KEYCODE_BACKSPACE) {
		if (length)
			name[length - 1] = 0;
	} else if (key.ascii >= 32 && key.ascii < 127 && key.ascii != '%' && length < 10) {
		name[length] = key.ascii;
		name[length + 1] = 0;
	}
	return false;
}

bool MortimerHighScores::save(Common::SaveFileManager &saveFileMan, const Common::String &fileName) {
	Common::ScopedPtr<Common::OutSaveFile> out(saveFileMan.openForSaving(fileName, false));
	if (!out)
		return false;
	for (uint i = 0; i < ARRAYSIZE(_entries); ++i) {
		out->write(_entries[i].name, sizeof(_entries[i].name));
		out->writeUint32LE(_entries[i].score);
	}
	out->finalize();
	if (out->err())
		return false;
	_enteringName = false;
	return true;
}

Common::String MortimerHighScores::ranking(uint32 score) const {
	const uint32 limits[] = {14999, 34999, 54999, 74999, 134999, 197999, 279999, 607499, 999999};
	uint rank = 0;
	while (rank < ARRAYSIZE(limits) && score > limits[rank])
		++rank;
	return _text.text(rank);
}

void MortimerHighScores::drawText(Graphics::Surface &screen, const Common::String &text, int x, int y,
		byte color, Graphics::TextAlign align) const {
	const int width = _font.getStringWidth(text);
	if (align == Graphics::kTextAlignCenter)
		x -= width / 2;
	else if (align == Graphics::kTextAlignRight)
		x -= width;
	// SHELL centers these text objects vertically, including the table rows.
	_font.drawString(&screen, text, x, y - (_font.getFontHeight() + 1) / 2, width, color, Graphics::kTextAlignLeft, 0, false);
}

void MortimerHighScores::draw(Graphics::Surface &screen, uint32 time) const {
	const int height = _font.getFontHeight() + 1;
	drawText(screen, _text.text(10), 320, 140, 253, Graphics::kTextAlignCenter);
	drawText(screen, _text.text(11), 30, 175, 1, Graphics::kTextAlignLeft);
	drawText(screen, _text.text(12), 350, 175, 1, Graphics::kTextAlignRight);
	drawText(screen, _text.text(13), 610, 175, 1, Graphics::kTextAlignRight);
	for (uint i = 0; i < ARRAYSIZE(_entries); ++i) {
		const Entry &entry = _entries[i];
		const byte color = (int)i == _selected ? 1 : 253;
		const int y = 210 + i * height;
		drawText(screen, entry.name, 30, y, color, Graphics::kTextAlignLeft);
		drawText(screen, Common::String::format("%u", entry.score), 350, y, color, Graphics::kTextAlignRight);
		drawText(screen, ranking(entry.score), 610, y, color, Graphics::kTextAlignRight);
	}
	if (_enteringName) {
		drawMortimerMessage(screen, _font, _text.text(14), 253);
		const Common::String name(_entries[_selected].name);
		const int y = 240 + height + height / 2 + height / 4;
		drawText(screen, name, 320, y, 253, Graphics::kTextAlignCenter);
		if ((time / 300) % 2 == 0) {
			const int x = 322 + _font.getStringWidth(name) / 2;
			screen.fillRect(Common::Rect(x, y - height / 2, x + 2, y + height / 2), 253);
		}
	} else {
		if (_didNotQualify)
			drawMortimerMessage(screen, _font, Common::String::format(_text.text(16).c_str(), (int)_score), 253);
		Common::Array<Common::String> lines;
		_font.wordWrapText(_text.text(15), 640, lines, 0, Graphics::kWordWrapOnExplicitNewLines);
		const int top = 457 - (lines.size() * _font.getFontHeight() + 1) / 2;
		for (uint i = 0; i < lines.size(); ++i)
			drawText(screen, lines[i], 320, top + height / 2 + i * height, 1, Graphics::kTextAlignCenter);
	}
}

} // End of namespace Scumm
