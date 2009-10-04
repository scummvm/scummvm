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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_font.h"

namespace Sci {

SciGUIfont::SciGUIfont(EngineState *state, SciGUIscreen *screen, GUIResourceId resourceId)
	: _s(state), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	if (_s->_gameName == "lsl1sci") {
		// we remove upper bits for lsl1, because it wants to load font 2107 which is not available
		_resourceId &= 0x7ff;
	}
	initData(_resourceId);
}

SciGUIfont::~SciGUIfont() {
}

void SciGUIfont::initData(GUIResourceId resourceId) {
	Resource *fontResource = _s->resMan->findResource(ResourceId(kResourceTypeFont, resourceId), false);
	if (!fontResource) {
		error("font resource %d not found", resourceId);
	}
	_resourceData = fontResource->data;

	mCharMax = READ_LE_UINT16(_resourceData + 2);
	mFontH = READ_LE_UINT16(_resourceData + 4);
	mChars = new charinfo[mCharMax];
	// filling info for every char
	for (int16 i = 0; i < mCharMax; i++) {
		mChars[i].offset = READ_LE_UINT16(_resourceData + 6 + i * 2);
		mChars[i].w = _resourceData[mChars[i].offset];
		mChars[i].h = _resourceData[mChars[i].offset + 1];
	}
}

GUIResourceId SciGUIfont::getResourceId() {
	return _resourceId;
}

byte SciGUIfont::getHeight() {
	return mFontH;
}
byte SciGUIfont::getCharWidth(byte chr) {
	return chr < mCharMax ? mChars[chr].w : 0;
}
byte SciGUIfont::getCharHeight(byte chr) {
	return chr < mCharMax ? mChars[chr].h : 0;
}
byte *SciGUIfont::getCharData(byte chr) {
	return chr < mCharMax ? _resourceData + mChars[chr].offset + 2 : 0;
}

void SciGUIfont::draw(int16 chr, int16 top, int16 left, byte color, byte textface) {
	int charWidth = MIN<int>(getCharWidth(chr), _screen->_width - left);
	int charHeight = MIN<int>(getCharHeight(chr), 200 - top);
	byte b = 0, mask = 0xFF;
	int y = top;

	byte *pIn = getCharData(chr);
	for (int i = 0; i < charHeight; i++, y++) {
		if (textface & 1) // "grayed" output
			mask = top++ % 2 ? 0xAA : 0x55;
		for (int done = 0; done < charWidth; done++) {
			if ((done & 7) == 0) // fetching next data byte
				b = *(pIn++) & mask;
			if (b & 0x80) // if MSB is set - paint it
				_screen->Put_Pixel(left + done, y, 1, color, 0, 0);
			b = b << 1;
		}
	}
}

} // end of namespace Sci
