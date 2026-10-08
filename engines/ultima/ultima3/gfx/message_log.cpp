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

#include "common/util.h"
#include "ultima/ultima3/gfx/message_log.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int LOG_COL = 24;
constexpr int LOG_ROW = 17;
constexpr char GLYPH_CURSOR = 0x1E;

void MessageLog::clear() {
	for (int i = 0; i < LOG_LINES; ++i)
		_lines[i].clear();
	_column = 0;
}

void MessageLog::scroll() {
	for (int i = 1; i < LOG_LINES; ++i)
		_lines[i - 1] = _lines[i];

	_lines[LOG_LINES - 1].clear();
	_column = 0;
}

void MessageLog::putChar(char ch) {
	Common::String &line = _lines[LOG_LINES - 1];

	// Each line is kept as long as the characters actually placed on it
	while ((int)line.size() < _column)
		line += ' ';
	if ((int)line.size() == _column)
		line += ch;
	else
		line.setChar(ch, _column);
	++_column;
}

void MessageLog::backspace(int count) {
	_column = MAX(0, _column - count);
	_lines[LOG_LINES - 1].erase(_column);
}

void MessageLog::print(const char *text) {
	for (; *text; ++text) {
		if (*text == '\n') {
			scroll();
		} else if (_column == LOG_WIDTH - 1) {
			// The last column is filled, but the cursor stays put
			putChar(*text);
			_column = LOG_WIDTH;
		} else if (_column > LOG_WIDTH - 1) {
			scroll();
			putChar(*text);
		} else {
			putChar(*text);
		}
	}
}

void MessageLog::draw(Graphics::Views::GfxSurface &s, bool showCursor) const {
	s.fillRect(Common::Rect(LOG_COL * 8, LOG_ROW * 8, (LOG_COL + LOG_WIDTH) * 8,
		(LOG_ROW + LOG_LINES) * 8), 0);

	for (int i = 0; i < LOG_LINES; ++i)
		s.writeString(Common::Point(LOG_COL, LOG_ROW + i), _lines[i]);

	if (showCursor) {
		s.setTextPos(Common::Point(LOG_COL + MIN(_column, LOG_WIDTH - 1), LOG_ROW + LOG_LINES - 1));
		s.writeChar(GLYPH_CURSOR);
	}
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
