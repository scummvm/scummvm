/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/textobject.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/color.h"

namespace Grim {

TextObjectCommon::TextObjectCommon() :
		_x(0), _y(0), _fgColor(0), _justify(0), _width(0), _height(0),
		_font(nullptr), _duration(0), _layer(0), _coords(0) {
	if (g_grim)
		g_grim->invalidateTextObjectsSortOrder();
}

void TextObjectCommon::setLayer(int layer) {
	_layer = layer;
	if (g_grim)
		g_grim->invalidateTextObjectsSortOrder();
}

TextObject::TextObject() :
		TextObjectCommon(), _numberLines(1), _textID(""), _elapsedTime(0),
		_maxLineWidth(0), _lines(nullptr), _userData(nullptr), _created(false),
		_blastDraw(false), _isSpeech(false), _stackLevel(0) {
}

TextObject::~TextObject() {
	delete[] _lines;
	if (_created) {
		g_driver->destroyTextObject(this);
	}
	if (g_grim)
		g_grim->invalidateTextObjectsSortOrder();
}

void TextObject::setText(const Common::String &text, bool delaySetup) {
	destroy();
	_textID = text;
	if (!delaySetup)
		setupText();
}

void TextObject::reset() {
	destroy();
	setupText();
}

void TextObject::saveState(SaveGame *state) const {
	state->writeColor(_fgColor);

	state->writeLESint32(_x);
	state->writeLESint32(_y);
	state->writeLESint32(_width);
	state->writeLESint32(_height);
	state->writeLESint32(_justify);
	state->writeLESint32(_numberLines);
	state->writeLESint32(_duration);

	state->writeBool(_blastDraw);
	state->writeBool(_isSpeech);
	state->writeLESint32(_elapsedTime);

	if (_font) {
		state->writeLESint32(_font->getId());
	} else {
		state->writeLESint32(-1);
	}

	state->writeString(_textID);

	if (g_grim->getGameType() == GType_MONKEY4) {
		state->writeLESint32(_layer);
		state->writeLESint32(_stackLevel);
	}
}

bool TextObject::restoreState(SaveGame *state) {
	_fgColor = state->readColor();

	_x            = state->readLESint32();
	_y            = state->readLESint32();
	_width        = state->readLESint32();
	_height       = state->readLESint32();
	_justify      = state->readLESint32();
	_numberLines  = state->readLESint32();
	_duration     = state->readLESint32();

	_blastDraw    = state->readBool();
	_isSpeech     = state->readBool();
	_elapsedTime  = state->readLESint32();

	int32 fontId = state->readLESint32();
	if (fontId == -1) {
		_font = nullptr;
	} else {
		_font = Font::getPool().getObject(fontId);
	}

	_textID = state->readString();

	if (g_grim->getGameType() == GType_MONKEY4) {
		_layer = state->readLESint32();
		_stackLevel = state->readLESint32();
		g_grim->invalidateTextObjectsSortOrder();
	}

	setupText();
	_created = false;
	_userData = nullptr;

	return true;
}

void TextObject::setDefaults(const TextObjectDefaults *defaults) {
	_x = defaults->getX();
	_y = defaults->getY();
	_font = defaults->getFont();
	_fgColor = defaults->getFGColor();
	_justify = defaults->getJustify();
}

int TextObject::getBitmapWidth() const {
	return _maxLineWidth;
}

int TextObject::getBitmapHeight() const {
	return _numberLines * _font->getKernedHeight();
}

int TextObject::getTextCharPosition(int pos) {
	int width = 0;
	Common::String msg = LuaBase::instance()->parseMsgText(_textID.c_str(), nullptr);
	for (int i = 0; (msg[i] != '\0') && (i < pos); ++i) {
		width += _font->getCharKernedWidth(msg[i]);
	}
	return width;
}

void TextObject::destroy() {
	if (_created) {
		g_driver->destroyTextObject(this);
		_created = false;
	}
}

void TextObject::setupText() {
	Common::String msg = LuaBase::instance()->parseMsgText(_textID.c_str(), nullptr);
	Common::String message;

	// remove spaces (NULL_TEXT) from the end of the string,
	// while this helps make the string unique it screws up
	// text justification
	// remove char of id 13 from the end of the string,
	int pos = msg.size() - 1;
	while (pos >= 0 && (msg[pos] == ' ' || msg[pos] == 13)) {
		msg.deleteLastChar();
		pos = msg.size() - 1;
	}
	delete[] _lines;
	if (msg.size() == 0) {
		_lines = nullptr;
		return;
	}

	// format the output message to incorporate line wrapping
	// (if necessary) for the text object
	const int SCREEN_WIDTH = _width ? _width : 640;
	const int SCREEN_MARGIN = SCREEN_WIDTH / 10;

	// If the speaker is too close to the edge of the screen we have to make
	// some room for the subtitles.
	if (_isSpeech) {
		if (_x < SCREEN_MARGIN) {
			_x = SCREEN_MARGIN;
		} else if (SCREEN_WIDTH - _x < SCREEN_MARGIN) {
			_x = SCREEN_WIDTH - SCREEN_MARGIN;
		}
	}

	// The maximum width for any line of text is determined by the justification
	// mode. Note that there are no left/right margins -- this is consistent
	// with GrimE.
	int maxWidth = 0;
	if (_justify == CENTER) {
		maxWidth = 2 * MIN(_x, SCREEN_WIDTH - _x);
	} else if (_justify == LJUSTIFY) {
		maxWidth = SCREEN_WIDTH - _x;
	} else if (_justify == RJUSTIFY) {
		maxWidth = _x;
	}

	// We break the message to lines not longer than maxWidth
	Common::String currLine;
	_numberLines = 1;
	int lineWidth = 0;
	for (uint i = 0; i < msg.size(); i++) {
		message += msg[i];
		currLine += msg[i];
		lineWidth += _font->getCharKernedWidth(msg[i]);

		if (currLine.size() > 1 && lineWidth > maxWidth) {
			if (currLine.contains(' ')) {
				while (currLine.lastChar() != ' ' && currLine.size() > 1) {
					lineWidth -= _font->getCharKernedWidth(currLine.lastChar());
					message.deleteLastChar();
					currLine.deleteLastChar();
					--i;
				}
			} else { // if it is a unique word
				int dashWidth = _font->getCharKernedWidth('-');
				while (lineWidth + dashWidth > maxWidth && currLine.size() > 1) {
					lineWidth -= _font->getCharKernedWidth(currLine.lastChar());
					message.deleteLastChar();
					currLine.deleteLastChar();
					--i;
				}
				message += '-';
			}
			message += '\n';
			currLine.clear();
			_numberLines++;

			lineWidth = 0;
		}
	}

	// If the text object is a speech subtitle, the y parameter is the
	// coordinate of the bottom of the text block (instead of the top). It means
	// that every extra line pushes the previous lines up, instead of being
	// printed further down the screen.
	const int SCREEN_TOP_MARGIN = _font->getKernedHeight();
	if (_isSpeech) {
		_y -= _numberLines * _font->getKernedHeight();
		if (_y < SCREEN_TOP_MARGIN) {
			_y = SCREEN_TOP_MARGIN;
		}
	}

	_lines = new Common::String[_numberLines];


	// Reset the max width so it can be recalculated
	_maxLineWidth = 0;

	for (int j = 0; j < _numberLines; j++) {
		int nextLinePos, cutLen;
		const char *breakPos = strchr(message.c_str(), '\n');
		if (breakPos) {
			nextLinePos = breakPos - message.c_str();
			cutLen = nextLinePos + 1;
		} else {
			nextLinePos = message.size();
			cutLen = nextLinePos;
		}
		Common::String currentLine(message.c_str(), message.c_str() + nextLinePos);
		_lines[j] = currentLine;
		int width = _font->getKernedStringLength(currentLine);
		if (width > _maxLineWidth)
			_maxLineWidth = width;
		for (int count = 0; count < cutLen; count++)
			message.deleteChar(0);
	}
	_elapsedTime = 0;
}

int TextObject::getLineX(int line) const {
	int x = _x;
	if (_justify == CENTER)
		x = _x - (_font->getKernedStringLength(_lines[line]) / 2);
	else if (_justify == RJUSTIFY)
		x = _x - getBitmapWidth();

	if (x < 0)
		x = 0;
	return x;
}

int TextObject::getLineY(int line) const {
	int y = _y;

	if (g_grim->getGameType() == GType_GRIM) {
		if (_blastDraw) { // special case for Grim for menu text draw, issue #1083
			y = _y + 5;
		} else {
/*			if (_font->getKernedHeight() == 21) // talk_font,verb_font
				y = _y - 6;
			else if (_font->getKernedHeight() == 26) // special_font
				y = _y - 12;
			else */if (_font->getKernedHeight() == 13) // computer_font
				y = _y - 6;/*
			else if (_font->getKernedHeight() == 19) // pt_font
				y = _y - 9;*/
			else
				y = _y;
		}
	}

	if (y < 0)
		y = 0;
	y += _font->getKernedHeight() * line;

	return y;
}

void TextObject::draw() {
	if (!_lines)
		return;

	if (!_created) {
		g_driver->createTextObject(this);
		_created = true;
	}

	if (_justify > 3 || _justify < 0)
		warning("TextObject::draw: Unknown justification code (%d)", _justify);

	g_driver->drawTextObject(this);

}

void TextObject::update() {
	if (!_duration || !_created) {
		return;
	}

	_elapsedTime += g_grim->getFrameTime();
	if (_elapsedTime > _duration) {
		delete this;
	}
}

} // end of namespace Grim
