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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"

namespace Sci {

SciGuiPalette::SciGuiPalette(EngineState *state, SciGui *gui, SciGuiScreen *screen)
	: _s(state), _gui(gui), _screen(screen) {
	init();
}

SciGuiPalette::~SciGuiPalette() {
}

void SciGuiPalette::init() {
	int16 i;
	for (i = 0; i < 256; i++) {
		_sysPalette.colors[i].used = 0;
		_sysPalette.colors[i].r = 0;
		_sysPalette.colors[i].g = 0;
		_sysPalette.colors[i].b = 0;
		_sysPalette.intensity[i] = 100;
		_sysPalette.mapping[i] = i;
	}
	_sysPalette.colors[0].used = 1;
	_sysPalette.colors[255].used = 1;
	_sysPalette.colors[255].r = 255;
	_sysPalette.colors[255].g = 255;
	_sysPalette.colors[255].b = 255;

	// Load default palette from resource 999
	if (!setFromResource(999, 2)) {
		// if not found, we try to set amiga palette
		if (!setAmiga()) {
			// if that also doesnt work out, set EGA palette
			setEGA();
		}
	};

	// Init _clrPowers used in MatchColor
	for(i = 0; i < 256; i++)
	  _clrPowers[i] = i*i;
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void SciGuiPalette::createFromData(byte *data, GuiPalette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(GuiPalette));
	// Setup default mapping
	for (colorNo = 0; colorNo < 256; colorNo++) {
		paletteOut->mapping[colorNo] = colorNo;
	}
	if (data[0] == 0 && data[1] == 1) {
		// SCI0/SCI1 palette
		palFormat = SCI_PAL_FORMAT_VARIABLE; // CONSTANT;
		palOffset = 260;
		palColorStart = 0; palColorCount = 256;
		//memcpy(&paletteOut->mapping, data, 256);
	} else {
		// SCI1.1 palette
		palFormat = data[32];
		palOffset = 37;
		palColorStart = READ_LE_UINT16(data + 25); palColorCount = READ_LE_UINT16(data + 29);
	}
	switch (palFormat) {
		case SCI_PAL_FORMAT_CONSTANT:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = 1;
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
		case SCI_PAL_FORMAT_VARIABLE:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = data[palOffset++];
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
	}
}


// Will try to set amiga palette by using "spal" file. If not found, we return false
bool SciGuiPalette::setAmiga() {
	Common::File file;
	int curColor, byte1, byte2;

	if (file.open("spal")) {
		for (curColor = 0; curColor < 32; curColor++) {
			byte1 = file.readByte();
			byte2 = file.readByte();
			if ((byte1 == EOF) || (byte2 == EOF))
				error("Amiga palette file ends prematurely");
			_sysPalette.colors[curColor].used = 1;
			_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
			_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
			_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;
		}
		file.close();
		return true;
	}
	return false;
}

void SciGuiPalette::setEGA() {
	int i;
	_sysPalette.colors[1].r  = 0x000; _sysPalette.colors[1].g  = 0x000; _sysPalette.colors[1].b  = 0x0AA;
	_sysPalette.colors[2].r  = 0x000; _sysPalette.colors[2].g  = 0x0AA; _sysPalette.colors[2].b  = 0x000;
	_sysPalette.colors[3].r  = 0x000; _sysPalette.colors[3].g  = 0x0AA; _sysPalette.colors[3].b  = 0x0AA;
	_sysPalette.colors[4].r  = 0x0AA; _sysPalette.colors[4].g  = 0x000; _sysPalette.colors[4].b  = 0x000;
	_sysPalette.colors[5].r  = 0x0AA; _sysPalette.colors[5].g  = 0x000; _sysPalette.colors[5].b  = 0x0AA;
	_sysPalette.colors[6].r  = 0x0AA; _sysPalette.colors[6].g  = 0x055; _sysPalette.colors[6].b  = 0x000;
	_sysPalette.colors[7].r  = 0x0AA; _sysPalette.colors[7].g  = 0x0AA; _sysPalette.colors[7].b  = 0x0AA;
	_sysPalette.colors[8].r  = 0x055; _sysPalette.colors[8].g  = 0x055; _sysPalette.colors[8].b  = 0x055;
	_sysPalette.colors[9].r  = 0x055; _sysPalette.colors[9].g  = 0x055; _sysPalette.colors[9].b  = 0x0FF;
	_sysPalette.colors[10].r = 0x055; _sysPalette.colors[10].g = 0x0FF; _sysPalette.colors[10].b = 0x055;
	_sysPalette.colors[11].r = 0x055; _sysPalette.colors[11].g = 0x0FF; _sysPalette.colors[11].b = 0x0FF;
	_sysPalette.colors[12].r = 0x0FF; _sysPalette.colors[12].g = 0x055; _sysPalette.colors[12].b = 0x055;
	_sysPalette.colors[13].r = 0x0FF; _sysPalette.colors[13].g = 0x055; _sysPalette.colors[13].b = 0x0FF;
	_sysPalette.colors[14].r = 0x0FF; _sysPalette.colors[14].g = 0x0FF; _sysPalette.colors[14].b = 0x055;
	_sysPalette.colors[15].r = 0x0FF; _sysPalette.colors[15].g = 0x0FF; _sysPalette.colors[15].b = 0x0FF;
	for (i = 0; i <= 15; i++) {
		_sysPalette.colors[i].used = 1;
	}
	for (i = 16; i <= 254; i++) {
		_sysPalette.colors[i].r = 200;
		_sysPalette.colors[i].used = 1;
	}
	setOnScreen();
}

bool SciGuiPalette::setFromResource(int16 resourceNo, int16 flag) {
	Resource *palResource = _s->resMan->findResource(ResourceId(kResourceTypePalette, resourceNo), 0);
	GuiPalette palette;

	if (palResource) {
		createFromData(palResource->data, &palette);
		set(&palette, 2);
		return true;
	}
	return false;
}

void SciGuiPalette::set(GuiPalette *sciPal, int16 flag) {
	uint32 systime = _sysPalette.timestamp;
	if (flag == 2 || sciPal->timestamp != systime) {
		merge(sciPal, &_sysPalette, flag);
		sciPal->timestamp = _sysPalette.timestamp;
		if (_gui->_picNotValid == 0 && systime != _sysPalette.timestamp)
			setOnScreen();
	}
}

void SciGuiPalette::merge(GuiPalette *pFrom, GuiPalette *pTo, uint16 flag) {
	uint16 res;
	int i,j;
	// colors 0 (black) and 255 (white) are not affected by merging
	for (i = 1 ; i < 255; i++) {
		if (!pFrom->colors[i].used)// color is not used - so skip it
			continue;
		// forced palette merging or dest color is not used yet
		if (flag == 2 || (!pTo->colors[i].used)) {
			pTo->colors[i].used = pFrom->colors[i].used;
			pTo->colors[i].r = pFrom->colors[i].r;
			pTo->colors[i].g = pFrom->colors[i].g;
			pTo->colors[i].b = pFrom->colors[i].b;
			pFrom->mapping[i] = i;
			continue;
		}
		// check if exact color could be matched
		res = matchColor(pTo, pFrom->colors[i].r, pFrom->colors[i].g, pFrom->colors[i].b);
		if (res & 0x8000) { // exact match was found
			pFrom->mapping[i] = res & 0xFF;
			continue;
		}
		// no exact match - see if there is an unused color
		for (j = 1; j < 256; j++)
			if (!pTo->colors[j].used) {
				pTo->colors[j].used = pFrom->colors[i].used;
				pTo->colors[j].r = pFrom->colors[i].r;
				pTo->colors[j].g = pFrom->colors[i].g;
				pTo->colors[j].b = pFrom->colors[i].b;
				pFrom->mapping[i] = j;
				break;
			}
		// if still no luck - set an approximate color
		if (j == 256) {
			pFrom->mapping[i] = res & 0xFF;
			pTo->colors[res & 0xFF].used |= 0x10;
		}
	}
	pTo->timestamp = (g_system->getMillis() - _s->game_start_time) * 60 / 1000;;
}

uint16 SciGuiPalette::matchColor(GuiPalette *pPal, byte r, byte g, byte b) {
	byte found = 0xFF;
	int diff = 0x2FFFF, cdiff;
	int16 dr,dg,db;

	for (int i = 0; i < 256; i++) {
		if ((!pPal->colors[i].used))
			continue;
		dr = pPal->colors[i].r - r;
		dg = pPal->colors[i].g - g;
		db = pPal->colors[i].b - b;
//		minimum squares match
		cdiff = _clrPowers[ABS(dr)] + _clrPowers[ABS(dg)] + _clrPowers[ABS(db)];
//		minimum sum match (Sierra's)
//		cdiff = ABS(dr) + ABS(dg) + ABS(db);
		if (cdiff < diff) {
			if (cdiff == 0)
				return i | 0x8000; // setting this flag to indicate exact match
			found = i;
			diff = cdiff;
		}
	}
	return found;
}

void SciGuiPalette::getSys(GuiPalette*pal) {
	if (pal != &_sysPalette)
		memcpy(pal, &_sysPalette,sizeof(GuiPalette));
}

void SciGuiPalette::setOnScreen() {
//	if (pal != &_sysPalette)
//		memcpy(&_sysPalette,pal,sizeof(GuiPalette));
	_screen->setPalette(&_sysPalette);
}

void SciGuiPalette::setIntensity(int fromColor, int toColor, int intensity, GuiPalette *destPalette) {
	memset(destPalette->intensity + fromColor, intensity, toColor - fromColor);
}

void SciGuiPalette::animate(byte fromColor, byte toColor, int speed) {
	GuiColor col;
	int len = toColor - fromColor - 1;
	uint32 now = (g_system->getMillis() - _s->game_start_time) * 60 / 1000;;
	// search for sheduled animations with the same 'from' value
	int sz = _palSchedules.size();
	for (int i = 0; i < sz; i++) {
		if (_palSchedules[i].from == fromColor) {
			if (_palSchedules[i].schedule < now) {
				if (speed > 0) {
					col = _sysPalette.colors[fromColor];
					memmove(&_sysPalette.colors[fromColor], &_sysPalette.colors[fromColor + 1], len * sizeof(GuiColor));
					_sysPalette.colors[toColor - 1] = col;
				} else {
					col = _sysPalette.colors[toColor - 1];
					memmove(&_sysPalette.colors[fromColor + 1], &_sysPalette.colors[fromColor], len * sizeof(GuiColor));
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_palSchedules.remove_at(i);
			}
			setOnScreen();
			return;
		}
	}
	// adding a new schedule
	GuiPalSchedule sched;
	sched.from = fromColor;
	sched.schedule = now + ABS(speed);
	_palSchedules.push_back(sched);
}

} // End of namespace Sci
