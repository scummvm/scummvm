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
#include "common/timer.h"
#include "common/util.h"
#include "common/system.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxPalette::GfxPalette(ResourceManager *resMan, GfxScreen *screen)
	: _resMan(resMan), _screen(screen) {
	int16 color;

	_sysPalette.timestamp = 0;
	for (color = 0; color < 256; color++) {
		_sysPalette.colors[color].used = 0;
		_sysPalette.colors[color].r = 0;
		_sysPalette.colors[color].g = 0;
		_sysPalette.colors[color].b = 0;
		_sysPalette.intensity[color] = 100;
		_sysPalette.mapping[color] = color;
	}
	// Black and white are hardcoded
	_sysPalette.colors[0].used = 1;
	_sysPalette.colors[255].used = 1;
	_sysPalette.colors[255].r = 255;
	_sysPalette.colors[255].g = 255;
	_sysPalette.colors[255].b = 255;

	_sysPaletteChanged = false;
	_alwaysForceRealMerge = false;

	// Pseudo-WORKAROUND
	// Quest for Glory 3 demo, Eco Quest 1 demo, Laura Bow 2 demo, Police Quest 1 vga and Nick's Picks Space Quest
	// all use an inbetween interpreter, some parts are SCI1.1, some parts are SCI1
	//  It's not using the SCI1.1 palette merging (copying over all the colors) but the real merging
	//  If we use the copying over, we will get issues because some views have marked all colors as being used
	//  and those will overwrite the current palette in that case
	// FIXME: the other nick's Picks may also use the same interpreter, but we can not verify
	//			if the other games also get reported having palette issues, add them here
	Common::String gameId = g_sci->getGameId();
	if (g_sci->isDemo()) {
		if (gameId == "laurabow2" || gameId == "qfg3" || gameId == "ecoquest")
			_alwaysForceRealMerge = true;
	} else {
		if (gameId == "pq1sci" || gameId == "cnick-sq")
			_alwaysForceRealMerge = true;
	}

	palVaryInit();
}

GfxPalette::~GfxPalette() {
}

// meant to get called only once during init of engine
void GfxPalette::setDefault() {
	if (_resMan->getViewType() == kViewEga)
		setEGA();
	else if (_resMan->isAmiga32color())
		setAmiga();
	else
		kernelSetFromResource(999, true);
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void GfxPalette::createFromData(byte *data, Palette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(Palette));
	// Setup default mapping
	for (colorNo = 0; colorNo < 256; colorNo++) {
		paletteOut->mapping[colorNo] = colorNo;
	}
	if ((data[0] == 0 && data[1] == 1) || (data[0] == 0 && data[1] == 0 && READ_LE_UINT16(data + 29) == 0)) {
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
bool GfxPalette::setAmiga() {
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
		// Directly set the palette, because setOnScreen() wont do a thing for amiga
		_screen->setPalette(&_sysPalette);
		return true;
	}
	return false;
}

// Called from picture class, some amiga sci1 games set half of the palette
void GfxPalette::modifyAmigaPalette(byte *data) {
	int16 curColor, curPos = 0;
	byte byte1, byte2;
	for (curColor = 0; curColor < 16; curColor++) {
		byte1 = data[curPos++];
		byte2 = data[curPos++];
		_sysPalette.colors[curColor].r = (byte1 & 0x0F) * 0x11;
		_sysPalette.colors[curColor].g = ((byte2 & 0xF0) >> 4) * 0x11;
		_sysPalette.colors[curColor].b = (byte2 & 0x0F) * 0x11;
	}
	_screen->setPalette(&_sysPalette);
}

void GfxPalette::setEGA() {
	int curColor;
	byte color1, color2;

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
	for (curColor = 0; curColor <= 15; curColor++) {
		_sysPalette.colors[curColor].used = 1;
	}
	// Now setting colors 16-254 to the correct mix colors that occur when not doing a dithering run on
	//  finished pictures
	for (curColor = 0x10; curColor <= 0xFE; curColor++) {
		_sysPalette.colors[curColor].used = 1;
		color1 = curColor & 0x0F; color2 = curColor >> 4;
		_sysPalette.colors[curColor].r = (_sysPalette.colors[color1].r >> 1) + (_sysPalette.colors[color2].r >> 1);
		_sysPalette.colors[curColor].g = (_sysPalette.colors[color1].g >> 1) + (_sysPalette.colors[color2].g >> 1);
		_sysPalette.colors[curColor].b = (_sysPalette.colors[color1].b >> 1) + (_sysPalette.colors[color2].b >> 1);
	}
	_sysPalette.timestamp = 1;
	setOnScreen();
}

void GfxPalette::set(Palette *newPalette, bool force, bool forceRealMerge) {
	uint32 systime = _sysPalette.timestamp;

	if (force || newPalette->timestamp != systime) {
		_sysPaletteChanged |= merge(newPalette, force, forceRealMerge);
		newPalette->timestamp = _sysPalette.timestamp;

		bool updatePalette = _sysPaletteChanged && _screen->_picNotValid == 0;

		if (_palVaryResourceId != -1) {
			// Pal-vary currently active, we don't set at any time, but also insert into origin palette
			insert(newPalette, &_palVaryOriginPalette);
			palVaryProcess(0, updatePalette);
			return;
		}

		if (updatePalette) {
			setOnScreen();
			_sysPaletteChanged = false;
		}
	}
}

bool GfxPalette::insert(Palette *newPalette, Palette *destPalette) {
	bool paletteChanged = false;

	for (int i = 1; i < 255; i++) {
		if (newPalette->colors[i].used) {
			if ((newPalette->colors[i].r != destPalette->colors[i].r) || (newPalette->colors[i].g != destPalette->colors[i].g) || (newPalette->colors[i].b != destPalette->colors[i].b)) {
				destPalette->colors[i].r = newPalette->colors[i].r;
				destPalette->colors[i].g = newPalette->colors[i].g;
				destPalette->colors[i].b = newPalette->colors[i].b;
				paletteChanged = true;
			}
			destPalette->colors[i].used = newPalette->colors[i].used;
			newPalette->mapping[i] = i;
		}
	}
	return paletteChanged;
}

bool GfxPalette::merge(Palette *newPalette, bool force, bool forceRealMerge) {
	uint16 res;
	int i,j;
	bool paletteChanged = false;

	if ((!forceRealMerge) && (!_alwaysForceRealMerge) && (getSciVersion() >= SCI_VERSION_1_1)) {
		// SCI1.1+ doesnt do real merging anymore, but simply copying over the used colors from other palettes
		//  There are some games with inbetween SCI1.1 interpreters, use real merging for them (e.g. laura bow 2 demo)

		// We don't update the timestamp for SCI1.1, it's only updated on kDrawPic calls
		return insert(newPalette, &_sysPalette);

	} else {
		// colors 0 (black) and 255 (white) are not affected by merging
		for (i = 1 ; i < 255; i++) {
			if (!newPalette->colors[i].used)// color is not used - so skip it
				continue;
			// forced palette merging or dest color is not used yet
			if (force || (!_sysPalette.colors[i].used)) {
				_sysPalette.colors[i].used = newPalette->colors[i].used;
				if ((newPalette->colors[i].r != _sysPalette.colors[i].r) || (newPalette->colors[i].g != _sysPalette.colors[i].g) || (newPalette->colors[i].b != _sysPalette.colors[i].b)) {
					_sysPalette.colors[i].r = newPalette->colors[i].r;
					_sysPalette.colors[i].g = newPalette->colors[i].g;
					_sysPalette.colors[i].b = newPalette->colors[i].b;
					paletteChanged = true;
				}
				newPalette->mapping[i] = i;
				continue;
			}
			// is the same color already at the same position? -> match it directly w/o lookup
			//  this fixes games like lsl1demo/sq5 where the same rgb color exists multiple times and where we would
			//  otherwise match the wrong one (which would result into the pixels affected (or not) by palette changes)
			if ((_sysPalette.colors[i].r == newPalette->colors[i].r) && (_sysPalette.colors[i].g == newPalette->colors[i].g) && (_sysPalette.colors[i].b == newPalette->colors[i].b)) {
				newPalette->mapping[i] = i;
				continue;
			}
			// check if exact color could be matched
			res = matchColor(newPalette->colors[i].r, newPalette->colors[i].g, newPalette->colors[i].b);
			if (res & 0x8000) { // exact match was found
				newPalette->mapping[i] = res & 0xFF;
				continue;
			}
			// no exact match - see if there is an unused color
			for (j = 1; j < 256; j++)
				if (!_sysPalette.colors[j].used) {
					_sysPalette.colors[j].used = newPalette->colors[i].used;
					_sysPalette.colors[j].r = newPalette->colors[i].r;
					_sysPalette.colors[j].g = newPalette->colors[i].g;
					_sysPalette.colors[j].b = newPalette->colors[i].b;
					newPalette->mapping[i] = j;
					paletteChanged = true;
					break;
				}
			// if still no luck - set an approximate color
			if (j == 256) {
				newPalette->mapping[i] = res & 0xFF;
				_sysPalette.colors[res & 0xFF].used |= 0x10;
			}
		}
	}

	if (!forceRealMerge)
		_sysPalette.timestamp = g_system->getMillis() * 60 / 1000;
	return paletteChanged;
}

// This is called for SCI1.1, when kDrawPic got done. We update sysPalette timestamp this way for SCI1.1 and also load
//  target-palette, if palvary is active
void GfxPalette::drewPicture(GuiResourceId pictureId) {
	if (!_alwaysForceRealMerge) // Don't do this on inbetween SCI1.1 games
		_sysPalette.timestamp++;

	if (_palVaryResourceId != -1) {
		palVaryLoadTargetPalette(pictureId);
	}
}

uint16 GfxPalette::matchColor(byte r, byte g, byte b) {
	byte found = 0xFF;
	int diff = 0x2FFFF, cdiff;
	int16 dr,dg,db;

	for (int i = 1; i < 255; i++) {
		if ((!_sysPalette.colors[i].used))
			continue;
		dr = _sysPalette.colors[i].r - r;
		dg = _sysPalette.colors[i].g - g;
		db = _sysPalette.colors[i].b - b;
//		minimum squares match
		cdiff = (dr*dr) + (dg*dg) + (db*db);
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

void GfxPalette::getSys(Palette *pal) {
	if (pal != &_sysPalette)
		memcpy(pal, &_sysPalette,sizeof(Palette));
}

void GfxPalette::setOnScreen() {
	// We dont change palette at all times for amiga
	if (_resMan->isAmiga32color())
		return;
	_screen->setPalette(&_sysPalette);

	// Redraw the Mac SCI1.1 Icon bar every palette change
	if (g_sci->_gfxMacIconBar)
		g_sci->_gfxMacIconBar->drawIcons();
}

bool GfxPalette::kernelSetFromResource(GuiResourceId resourceId, bool force) {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
	Palette palette;

	if (palResource) {
		createFromData(palResource->data, &palette);
		set(&palette, force);
		return true;
	}
	return false;
}

void GfxPalette::kernelSetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used |= flag;
	}
}

void GfxPalette::kernelUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	uint16 colorNr;
	for (colorNr = fromColor; colorNr < toColor; colorNr++) {
		_sysPalette.colors[colorNr].used &= ~flag;
	}
}

void GfxPalette::kernelSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette) {
	memset(&_sysPalette.intensity[0] + fromColor, intensity, toColor - fromColor);
	if (setPalette) {
		setOnScreen();
		g_sci->getEngineState()->_throttleTrigger = true;
	}
}

int16 GfxPalette::kernelFindColor(uint16 r, uint16 g, uint16 b) {
	return matchColor(r, g, b) & 0xFF;
}

// Returns true, if palette got changed
bool GfxPalette::kernelAnimate(byte fromColor, byte toColor, int speed) {
	Color col;
	//byte colorNr;
	int16 colorCount;
	uint32 now = g_system->getMillis() * 60 / 1000;

	// search for sheduled animations with the same 'from' value
	// schedule animation...
	int scheduleCount = _schedules.size();
	int scheduleNr;
	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor)
			break;
	}
	if (scheduleNr == scheduleCount) {
		// adding a new schedule
		PalSchedule newSchedule;
		newSchedule.from = fromColor;
		newSchedule.schedule = now + ABS(speed);
		_schedules.push_back(newSchedule);
		scheduleCount++;
	}

	g_sci->getEngineState()->_throttleTrigger = true;

	for (scheduleNr = 0; scheduleNr < scheduleCount; scheduleNr++) {
		if (_schedules[scheduleNr].from == fromColor) {
			if (_schedules[scheduleNr].schedule <= now) {
				if (speed > 0) {
					// TODO: Not really sure about this, sierra sci seems to do exactly this here
					col = _sysPalette.colors[fromColor];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor], &_sysPalette.colors[fromColor + 1], colorCount * sizeof(Color));
					}
					_sysPalette.colors[toColor - 1] = col;
				} else {
					col = _sysPalette.colors[toColor - 1];
					if (fromColor < toColor) {
						colorCount = toColor - fromColor - 1;
						memmove(&_sysPalette.colors[fromColor + 1], &_sysPalette.colors[fromColor], colorCount * sizeof(Color));
					}
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_schedules[scheduleNr].schedule = now + ABS(speed);
				// TODO: Not sure when sierra actually removes a schedule
				//_schedules.remove_at(scheduleNr);
				return true;
			}
			return false;
		}
	}
	return false;
}

void GfxPalette::kernelAnimateSet() {
	setOnScreen();
}

void GfxPalette::kernelAssertPalette(GuiResourceId resourceId) {
	// Sometimes invalid viewIds are asked for, ignore those (e.g. qfg1vga)
	if (!_resMan->testResource(ResourceId(kResourceTypeView, resourceId)))
		return;

	GfxView *view = g_sci->_gfxCache->getView(resourceId);
	Palette *viewPalette = view->getPalette();
	if (viewPalette) {
		// merge/insert this palette
		set(viewPalette, true);
	}
}

// palVary
//  init - only does, if palVaryOn == false
//         target, start, new palette allocation
//         palVaryOn = true
//         palDirection = 1
//         palStop = 64
//         palTime = from caller
//         copy resource palette to target
//         init target palette (used = 1 on all colors, color 0 = RGB 0, 0, 0 color 255 = RGB 0xFF, 0xFF, 0xFF
//         copy sysPalette to startPalette
//         init new palette like target palette
//         palPercent = 1
//         do various things
//         return 1
//  deinit - unloads target palette, kills timer hook, disables palVaryOn
//  pause - counts up or down, if counter != 0 -> signal wont get counted up by timer
//           will only count down to 0
//
// Restarting game
//         palVary = false
//         palPercent = 0
//         call palVary deinit
//
// Saving/restoring
//         need to save start and target-palette, when palVaryOn = true

void GfxPalette::palVaryInit() {
	_palVaryResourceId = -1;
	_palVaryPaused = 0;
	_palVarySignal = 0;
	_palVaryStep = 0;
	_palVaryStepStop = 0;
	_palVaryDirection = 0;
	_palVaryTicks = 0;
}

bool GfxPalette::palVaryLoadTargetPalette(GuiResourceId resourceId) {
	_palVaryResourceId = resourceId;
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
	if (palResource) {
		// Load and initialize destination palette
		createFromData(palResource->data, &_palVaryTargetPalette);
		return true;
	}
	return false;
}

void GfxPalette::palVaryInstallTimer() {
	int16 ticks = _palVaryTicks > 0 ? _palVaryTicks : 1;
	// Call signal increase every [ticks]
	g_sci->getTimerManager()->installTimerProc(&palVaryCallback, 1000000 / 60 * ticks, this);
}

void GfxPalette::palVaryRemoveTimer() {
	g_sci->getTimerManager()->removeTimerProc(&palVaryCallback);
}

bool GfxPalette::kernelPalVaryInit(GuiResourceId resourceId, uint16 ticks, uint16 stepStop, uint16 direction) {
	if (_palVaryResourceId != -1)	// another palvary is taking place, return
		return false;

	if (palVaryLoadTargetPalette(resourceId)) {
		// Save current palette
		memcpy(&_palVaryOriginPalette, &_sysPalette, sizeof(Palette));

		_palVarySignal = 0;
		_palVaryTicks = ticks;
		_palVaryStep = 1;
		_palVaryStepStop = stepStop;
		_palVaryDirection = direction;
		// if no ticks are given, jump directly to destination
		if (!_palVaryTicks)
			_palVaryDirection = stepStop;
		palVaryInstallTimer();
		return true;
	}
	return false;
}

int16 GfxPalette::kernelPalVaryReverse(int16 ticks, uint16 stepStop, int16 direction) {
	if (_palVaryResourceId == -1)
		return 0;

	if (_palVaryStep > 64)
		_palVaryStep = 64;
	if (ticks != -1)
		_palVaryTicks = ticks;
	_palVaryStepStop = stepStop;
	_palVaryDirection = direction != -1 ? -direction : -_palVaryDirection;

	if (!_palVaryTicks)
		_palVaryDirection = _palVaryStepStop - _palVaryStep;
	palVaryInstallTimer();
	return kernelPalVaryGetCurrentStep();
}

int16 GfxPalette::kernelPalVaryGetCurrentStep() {
	if (_palVaryDirection >= 0)
		return _palVaryStep;
	return -_palVaryStep;
}

int16 GfxPalette::kernelPalVaryChangeTarget(GuiResourceId resourceId) {
	if (_palVaryResourceId != -1) {
		Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);
		if (palResource) {
			Palette insertPalette;
			createFromData(palResource->data, &insertPalette);
			// insert new palette into target
			insert(&insertPalette, &_palVaryTargetPalette);
			// update palette and set on screen
			palVaryProcess(0, true);
		}
	}
	return kernelPalVaryGetCurrentStep();
}

void GfxPalette::kernelPalVaryChangeTicks(uint16 ticks) {
	_palVaryTicks = ticks;
	if (_palVaryStep - _palVaryStepStop) {
		palVaryRemoveTimer();
		palVaryInstallTimer();
	}
}

void GfxPalette::kernelPalVaryPause(bool pause) {
	if (_palVaryResourceId == -1)
		return;
	// this call is actually counting states, so calling this 3 times with true will require calling it later
	// 3 times with false to actually remove pause
	if (pause) {
		_palVaryPaused++;
	} else {
		if (_palVaryPaused)
			_palVaryPaused--;
	}
}

void GfxPalette::kernelPalVaryDeinit() {
	palVaryRemoveTimer();

	_palVaryResourceId = -1;	// invalidate the target palette
}

void GfxPalette::palVaryCallback(void *refCon) {
	((GfxPalette *)refCon)->palVaryIncreaseSignal();
}

void GfxPalette::palVaryIncreaseSignal() {
	if (!_palVaryPaused)
		_palVarySignal++;
}

// Actually do the pal vary processing
void GfxPalette::palVaryUpdate() {
	if (_palVarySignal) {
		palVaryProcess(_palVarySignal, true);
		_palVarySignal = 0;
	}
}

void GfxPalette::palVaryPrepareForTransition() {
	if (_palVaryResourceId != -1) {
		// Before doing transitions, we have to prepare palette
		palVaryProcess(0, false);
	}
}

// Processes pal vary updates
void GfxPalette::palVaryProcess(int signal, bool setPalette) {
	int16 stepChange = signal * _palVaryDirection;

	_palVaryStep += stepChange;
	if (stepChange > 0) {
		if (_palVaryStep > _palVaryStepStop)
			_palVaryStep = _palVaryStepStop;
	} else {
		if (_palVaryStep < _palVaryStepStop) {
			if (signal)
				_palVaryStep = _palVaryStepStop;
		}
	}

	// We don't need updates anymore, if we reached end-position
	if (_palVaryStep == _palVaryStepStop)
		palVaryRemoveTimer();
	if (_palVaryStep == 0)
		_palVaryResourceId = -1;

	// Calculate inbetween palette
	Sci::Color inbetween;
	int16 color;
	for (int colorNr = 1; colorNr < 255; colorNr++) {
		inbetween.used = _sysPalette.colors[colorNr].used;
		color = _palVaryTargetPalette.colors[colorNr].r - _palVaryOriginPalette.colors[colorNr].r;
		inbetween.r = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].r;
		color = _palVaryTargetPalette.colors[colorNr].g - _palVaryOriginPalette.colors[colorNr].g;
		inbetween.g = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].g;
		color = _palVaryTargetPalette.colors[colorNr].b - _palVaryOriginPalette.colors[colorNr].b;
		inbetween.b = ((color * _palVaryStep) / 64) + _palVaryOriginPalette.colors[colorNr].b;

		if (memcmp(&inbetween, &_sysPalette.colors[colorNr], sizeof(Sci::Color))) {
			_sysPalette.colors[colorNr] = inbetween;
			_sysPaletteChanged = true;
		}
	}

	if ((_sysPaletteChanged) && (setPalette) && (_screen->_picNotValid == 0)) {
		setOnScreen();
		_sysPaletteChanged = false;
	}
}

} // End of namespace Sci
