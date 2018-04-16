/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/effects/ttytext_effect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/text/text.h"

#include "common/stream.h"
#include "common/file.h"

namespace ZVision {

ttyTextNode::ttyTextNode(ZVision *engine, uint32 key, const Common::String &file, const Common::Rect &r, int32 delay) :
	ScriptingEffect(engine, key, SCRIPTING_EFFECT_TTYTXT),
	_fnt(engine) {
	_delay = delay;
	_r = r;
	_txtpos = 0;
	_nexttime = 0;
	_dx = 0;
	_dy = 0;

	Common::File *infile = _engine->getSearchManager()->openFile(file);
	if (infile) {
		while (!infile->eos()) {
			Common::String asciiLine = readWideLine(*infile);
			if (asciiLine.empty()) {
				continue;
			}
			_txtbuf += asciiLine;
		}

		delete infile;
	}
	_img.create(_r.width(), _r.height(), _engine->_resourcePixelFormat);
	_state._sharp = true;
	_state.readAllStyles(_txtbuf);
	_state.updateFontWithTextState(_fnt);
	_engine->getScriptManager()->setStateValue(_key, 1);
}

ttyTextNode::~ttyTextNode() {
	_engine->getScriptManager()->setStateValue(_key, 2);
	_img.free();
}

bool ttyTextNode::process(uint32 deltaTimeInMillis) {
	_nexttime -= deltaTimeInMillis;

	if (_nexttime < 0) {
		if (_txtpos < _txtbuf.size()) {
			if (_txtbuf[_txtpos] == '<') {
				int32 start = _txtpos;
				int32 end = 0;
				int16 ret = 0;
				while (_txtbuf[_txtpos] != '>' && _txtpos < _txtbuf.size())
					_txtpos++;
				end = _txtpos;
				if (start != -1) {
					if ((end - start - 1) > 0) {
						ret = _state.parseStyle(_txtbuf.c_str() + start + 1, end - start - 1);
					}
				}

				if (ret & (TEXT_CHANGE_FONT_TYPE | TEXT_CHANGE_FONT_STYLE)) {
					_state.updateFontWithTextState(_fnt);
				} else if (ret & TEXT_CHANGE_NEWLINE) {
					newline();
				}

				if (ret & TEXT_CHANGE_HAS_STATE_BOX) {
					Common::String buf;
					buf = Common::String::format("%d", _engine->getScriptManager()->getStateValue(_state._statebox));

					for (uint8 j = 0; j < buf.size(); j++)
						outchar(buf[j]);
				}

				_txtpos++;
			} else {
				int8 charsz = getUtf8CharSize(_txtbuf[_txtpos]);

				uint16 chr = readUtf8Char(_txtbuf.c_str() + _txtpos);

				if (chr == ' ') {
					uint32 i = _txtpos + charsz;
					uint16 width = _fnt.getCharWidth(chr);

					while (i < _txtbuf.size() && _txtbuf[i] != ' ' && _txtbuf[i] != '<') {

						int8 chsz   = getUtf8CharSize(_txtbuf[i]);
						uint16 uchr = readUtf8Char(_txtbuf.c_str() + _txtpos);

						width += _fnt.getCharWidth(uchr);

						i += chsz;
					}

					if (_dx + width > _r.width())
						newline();
					else
						outchar(chr);
				} else
					outchar(chr);

				_txtpos += charsz;
			}
			_nexttime = _delay;
			_engine->getRenderManager()->blitSurfaceToBkg(_img, _r.left, _r.top);
		} else
			return stop();
	}

	return false;
}

void ttyTextNode::scroll() {
	int32 scrl = 0;
	while (_dy - scrl > _r.height() - _fnt.getFontHeight())
		scrl += _fnt.getFontHeight();
	int8 *pixels = (int8 *)_img.getPixels();
	for (uint16 h = scrl; h < _img.h; h++)
		memcpy(pixels + _img.pitch * (h - scrl), pixels + _img.pitch * h, _img.pitch);

	_img.fillRect(Common::Rect(0, _img.h - scrl, _img.w, _img.h), 0);
	_dy -= scrl;
}

void ttyTextNode::newline() {
	_dy += _fnt.getFontHeight();
	_dx = 0;
}

void ttyTextNode::outchar(uint16 chr) {
	uint32 clr = _engine->_resourcePixelFormat.RGBToColor(_state._red, _state._green, _state._blue);

	if (_dx + _fnt.getCharWidth(chr) > _r.width())
		newline();

	if (_dy + _fnt.getFontHeight() >= _r.height())
		scroll();

	_fnt.drawChar(&_img, chr, _dx, _dy, clr);

	_dx += _fnt.getCharWidth(chr);
}

} // End of namespace ZVision
