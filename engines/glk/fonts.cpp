/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/fonts.h"
#include "glk/glk.h"
#include "glk/screen.h"
#include "glk/windows.h"

namespace Glk {

uint FontInfo::_caretColor;
uint FontInfo::_caretSave;
int FontInfo::_caretShape;


FontInfo::FontInfo() : _size(0), _aspect(0), _cellW(0), _cellH(0), _leading(0), _baseLine(0),
		_linkStyle(0), _moreFont(PROPB), _moreAlign(0), _caps(0), _linkColor(0), _linkSave(0),
		_moreColor(0), _moreSave(0) {
	_caretShape = 0;
	_caretColor = 0;
	_caretSave = 0;
}

void FontInfo::drawCaret(const Point &pos) {
	uint color = _caretColor;
	Graphics::Screen &s = *g_vm->_screen;
	int x = pos.x / GLI_SUBPIX, y = pos.y;

	switch (_caretShape) {
	case SMALL_DOT:
		s.hLine(x + 0, y + 1, x + 0, color);
		s.hLine(x - 1, y + 2, x + 1, color);
		s.hLine(x - 2, y + 3, x + 2, color);
		break;

	case FAT_DOT:
		s.hLine(x + 0, y + 1, x + 0, color);
		s.hLine(x - 1, y + 2, x + 1, color);
		s.hLine(x - 2, y + 3, x + 2, color);
		s.hLine(x - 3, y + 4, x + 3, color);
		break;

	case THIN_LINE:
		s.vLine(x, y - _baseLine + 1, y - 1, color);
		break;

	case FAT_LINE:
		s.fillRect(Rect(x, y - _baseLine + 1, x + 1, y - 1), color);
		break;

	default:
		// BLOCK
		s.fillRect(Rect(x, y - _baseLine + 1, x + _cellW, y - 1), color);
		break;
	}
}


/*--------------------------------------------------------------------------*/

PropFontInfo::PropFontInfo() : _justify(0), _quotes(0), _dashes(0), _spaces(0),
		_lineSeparation(2) {
}

} // End of namespace Glk
