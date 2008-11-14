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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"
#include "graphics/surface.h"

#include "groovie/font.h"

namespace Groovie {

Font::Font(OSystem *syst) :
	_syst(syst), _sphinxfnt(NULL) {

	Common::File fontfile;
	if (!fontfile.open("sphinx.fnt")) {
		error("Groovie::Font: Couldn't open sphinx.fnt");
	}
	uint16 fontfilesize = fontfile.size();
	_sphinxfnt = fontfile.readStream(fontfilesize);
	fontfile.close();
}

Font::~Font() {
	delete _sphinxfnt;
}

void Font::printstring(char *messagein) {
	uint16 totalwidth = 0, currxoffset, i;

	char message[15];
	memset(message, 0, 15);

	// Clear the top bar
	Common::Rect topbar(640, 80);
	Graphics::Surface *gamescreen;
	gamescreen = _syst->lockScreen();
	gamescreen->fillRect(topbar, 0);	
	_syst->unlockScreen();

	for (i = 0; i < 14; i++) {
		char chartocopy = messagein[i];
		if (chartocopy <= 0x00 || chartocopy == 0x24) {
			break;
		}
		message[i] = chartocopy;
	}
	Common::rtrim(message);
	for (i = 0; i < strlen(message); i++) {
		totalwidth += letterwidth(message[i]);
	}
	currxoffset = (640 - totalwidth) / 2;
	char *currpos = message;
	while (*(currpos) != 0) {
		currxoffset += printletter(*(currpos++), currxoffset);
	}
}

uint16 Font::letteroffset(char letter) {
	uint16 offset;
	offset = letter;
	_sphinxfnt->seek(offset);
	offset = _sphinxfnt->readByte() * 2 + 128;
	_sphinxfnt->seek(offset);
	offset = _sphinxfnt->readUint16LE();
	return offset;
}

uint8 Font::letterwidth(char letter) {
	uint16 offset = letteroffset(letter);
	_sphinxfnt->seek(offset);
	return _sphinxfnt->readByte();
}

uint8 Font::letterheight(char letter) {
	uint16 offset, width, julia, data, counter = 0;
	offset = letteroffset(letter);
	_sphinxfnt->seek(offset);
	width = _sphinxfnt->readByte();
	julia = _sphinxfnt->readByte();
	data = _sphinxfnt->readByte();
	while (data != 0xFF) {
		data = _sphinxfnt->readByte();
		counter++;
	}
	if (counter % width != 0) assert("font file corrupt");
	return counter / width;
}


uint8 Font::printletter(char letter, uint16 xoffset) {
	uint16 offset, width, height, julia;
	offset = letteroffset(letter);
	height = letterheight(letter);
	_sphinxfnt->seek(offset);
	width = _sphinxfnt->readByte();
	julia = _sphinxfnt->readByte();

	byte *data = new byte[width * height];
	_sphinxfnt->read(data, width * height);
	_syst->copyRectToScreen(data, width, xoffset, 16, width, height);
	delete data;
	
	return width;
}

} // End of Groovie namespace
