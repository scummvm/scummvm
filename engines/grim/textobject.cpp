/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/textobject.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"
#include "engines/grim/colormap.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

Common::String parseMsgText(const char *msg, char *msgId);

TextObjectCommon::TextObjectCommon() :
	_x(0), _y(0), _fgColor(0), _justify(0), _width(0), _height(0),
	_disabled(false), _font(NULL), _duration(0) {
}

TextObject::TextObject(bool blastDraw, bool isSpeech) :
		PoolObject<TextObject, MKTAG('T', 'E', 'X', 'T')>(), TextObjectCommon(), _numberLines(1),
		_maxLineWidth(0), _lines(0), _userData(0), _created(false) {
	_blastDraw = blastDraw;
	_isSpeech = isSpeech;
}

TextObject::TextObject() :
	PoolObject<TextObject, MKTAG('T', 'E', 'X', 'T')>(), TextObjectCommon(), _maxLineWidth(0), _lines(NULL) {

}

TextObject::~TextObject() {
	delete[] _lines;
	g_driver->destroyTextObject(this);
}

void TextObject::setText(const Common::String &text) {
	destroy();
	_textID = text;
	setupText();
}

void TextObject::reset() {
	destroy();
	setupText();
}

void TextObject::saveState(SaveGame *state) const {
	state->writeLEUint32(_fgColor->getId());

	state->writeLESint32(_x);
	state->writeLESint32(_y);
	state->writeLESint32(_width);
	state->writeLESint32(_height);
	state->writeLESint32(_justify);
	state->writeLESint32(_numberLines);
	state->writeLESint32(_duration);

	state->writeLESint32(_disabled);
	state->writeLESint32(_blastDraw);
	state->writeLESint32(_isSpeech);
	state->writeLESint32(_elapsedTime);

	state->writeLEUint32(_font->getId());

	state->writeString(_textID);
}

bool TextObject::restoreState(SaveGame *state) {
	_fgColor = PoolColor::getPool()->getObject(state->readLEUint32());

	_x            = state->readLESint32();
	_y            = state->readLESint32();
	_width        = state->readLESint32();
	_height       = state->readLESint32();
	_justify      = state->readLESint32();
	_numberLines  = state->readLESint32();
	_duration     = state->readLESint32();

	_disabled     = state->readLESint32();
	_blastDraw    = state->readLESint32();
	_isSpeech     = state->readLESint32();
	_elapsedTime  = state->readLESint32();

	_font = Font::getPool()->getObject(state->readLEUint32());

	_textID = state->readString();

	setupText();
	_created = false;
	_userData = NULL;

	return true;
}

void TextObject::setDefaults(TextObjectDefaults *defaults) {
	_x = defaults->getX();
	_y = defaults->getY();
	_font = defaults->getFont();
	_fgColor = defaults->getFGColor();
	_justify = defaults->getJustify();
	_disabled = defaults->getDisabled();
}

int TextObject::getBitmapWidth() {
	return _maxLineWidth;
}

int TextObject::getBitmapHeight() {
	return _numberLines * _font->getHeight();
}

int TextObject::getTextCharPosition(int pos) {
	int width = 0;
	Common::String msg = parseMsgText(_textID.c_str(), NULL);
	for (int i = 0; (msg[i] != '\0') && (i < pos); ++i) {
		width += _font->getCharWidth(msg[i]);
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
	Common::String msg = parseMsgText(_textID.c_str(), NULL);
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
		_lines = NULL;
		return;
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		if (_x == 0)
			_x = 320;
		if (_y == 0)
			_y = 240;
	}

	// format the output message to incorporate line wrapping
	// (if necessary) for the text object
	const int SCREEN_WIDTH = _width ? _width : 640;
	const int SCREEN_MARGIN = 75;

	// If the speaker is too close to the edge of the screen we have to make
	// some room for the subtitles.
	if (_isSpeech){
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
	int maxLineWidth = 0;
	for (uint i = 0; i < msg.size(); i++) {
		lineWidth += MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
		if (lineWidth > maxWidth) {
			bool wordSplit = false;
			if (currLine.contains(' ')) {
				while (msg[i] != ' ' && i > 0) {
					lineWidth -= MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
					message.deleteLastChar();
					--i;
				}
			} else if (msg[i] != ' ') { // if it is a unique word
				int dashWidth = MAX(_font->getCharWidth('-'), _font->getCharDataWidth('-'));
				while (lineWidth + dashWidth > maxWidth) {
					lineWidth -= MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
					message.deleteLastChar();
					--i;
				}
				message += '-';
				wordSplit = true;
 			}
			message += '\n';
			currLine.clear();
			_numberLines++;

			if (lineWidth > maxLineWidth) {
				maxLineWidth = lineWidth;
			}
			lineWidth = 0;

			if (wordSplit) {
				lineWidth += MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
			} else {
				continue; // don't add the space back
			}
		}

		if (lineWidth > maxLineWidth)
			maxLineWidth = lineWidth;

		message += msg[i];
		currLine += msg[i];
	}

	// If the text object is a speech subtitle, the y parameter is the
	// coordinate of the bottom of the text block (instead of the top). It means
	// that every extra line pushes the previous lines up, instead of being
	// printed further down the screen.
	const int SCREEN_TOP_MARGIN = 16;
	if (_isSpeech) {
		_y -= _numberLines * _font->getHeight();
		if (_y < SCREEN_TOP_MARGIN) {
			_y = SCREEN_TOP_MARGIN;
		}
	}

	_lines = new Common::String[_numberLines];

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
		int width = _font->getStringLength(currentLine);
		if (width > _maxLineWidth)
			_maxLineWidth = width;
		for (int count = 0; count < cutLen; count++)
			message.deleteChar(0);
	}
	_elapsedTime = 0;
}

int TextObject::getLineX(int line) {
	int x = _x;
	if (_justify == CENTER)
		x = _x - (_font->getStringLength(_lines[line]) / 2);
	else if (_justify == RJUSTIFY)
		x = _x - getBitmapWidth();

	if (x < 0)
		x = 0;
	return x;
}

int TextObject::getLineY(int line) {
	int y = _y;
	if (_blastDraw)
		y = _y + 5;
	else {
		if (_font->getHeight() == 21) // talk_font,verb_font
			y = _y - 6;
		else if (_font->getHeight() == 26) // special_font
			y = _y - 12;
		else if (_font->getHeight() == 13) // computer_font
			y = _y - 6;
		else if (_font->getHeight() == 19) // pt_font
			y = _y - 9;
		else
			y = _y;
	}
	if (y < 0)
		y = 0;
	y += _font->getHeight()*line;

	return y;
}

void TextObject::draw() {
	if (_disabled || !_lines)
		return;

	if (!_created) {
		g_driver->createTextObject(this);
		_created = true;
	}

	if ((_justify > 3 || _justify < 0) && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
		warning("TextObject::draw: Unknown justification code (%d)", _justify);

	g_driver->drawTextObject(this);

}

void TextObject::update() {
	if (!_duration || !_created || _disabled) {
		return;
	}

	_elapsedTime += g_grim->getFrameTime();
	if (_elapsedTime > _duration) {
		_disabled = true;
	}
}

} // end of namespace Grim
