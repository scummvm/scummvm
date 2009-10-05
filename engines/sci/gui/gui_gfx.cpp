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
#include "sci/gui/gui_font.h"
#include "sci/gui/gui_picture.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_gfx.h"

namespace Sci {

SciGuiGfx::SciGuiGfx(OSystem *system, EngineState *state, SciGuiScreen *screen)
	: _system(system), _s(state), _screen(screen) {
	init();
	initPalette();
	initTimer();
}

SciGuiGfx::~SciGuiGfx() {
	_system->getTimerManager()->removeTimerProc(&timerHandler);
}

void SciGuiGfx::init() {
	_font = NULL;
	_textFonts = NULL; _textFontsCount = 0;
	_textColors = NULL; _textColorsCount = 0;

	_picNotValid = false;

	_mainPort = mallocPort();
	SetPort(_mainPort);
	OpenPort(_mainPort);

	_menuPort = mallocPort();
	OpenPort(_menuPort);
	SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->_width, _screen->_height);
	_menuRect = Common::Rect(0, 0, _screen->_width, 10);

//	HEAPHANDLE theMenuBarH = heapNewPtr(34, kDataPort, "MenuBar");
//	heapClearPtr(theMenuBarH);
//	_theMenuBar = (Common::Rect *)heap2Ptr(theMenuBarH);
//	*_theMenuBar = Common::Rect(_gfx->RGetPort()->rect.right, 10);

	_sysTicks = 0;
}

void SciGuiGfx::initPalette() {
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
	//if (g_sci->getPlatform() == Common::kPlatformAmiga)
	//	setAmigaPalette();
	//else 

	// Load default palette from resource 999
	if (!SetResPalette(999, 2)) {
		// if not found, we set EGA palette
		SetEGApalette();
	};

	// Init _clrPowers used in MatchColor
	for(i = 0; i < 256; i++)
	  _clrPowers[i] = i*i;
}

void SciGuiGfx::initTimer() {
	_sysSpeed = 1000000 / 60;
	Common::TimerManager *tm = _system->getTimerManager();
	tm->removeTimerProc(&timerHandler);
	tm->installTimerProc(&timerHandler, _sysSpeed, this);
}

void SciGuiGfx::timerHandler(void *ref) {
	((SciGuiGfx *)ref)->_sysTicks++;
}

GuiPort *SciGuiGfx::mallocPort() {
	GuiPort *newPort = (GuiPort *)malloc(sizeof(GuiPort));
	assert(newPort);
	memset(newPort, 0, sizeof(GuiPort));
	return newPort;
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void SciGuiGfx::SetEGApalette() {
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
	setScreenPalette(&_sysPalette);
}

void SciGuiGfx::CreatePaletteFromData(byte *data, GuiPalette *paletteOut) {
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

bool SciGuiGfx::SetResPalette(int16 resourceNo, int16 flag) {
	Resource *palResource = _s->resMan->findResource(ResourceId(kResourceTypePalette, resourceNo), 0);
	GuiPalette palette;

	if (palResource) {
		CreatePaletteFromData(palResource->data, &palette);
		SetPalette(&palette, 2);
		return true;
	}
	return false;
}

void SciGuiGfx::SetPalette(GuiPalette *sciPal, int16 flag) {
	uint32 systime = _sysPalette.timestamp;
	if (flag == 2 || sciPal->timestamp != systime) {
		MergePalettes(sciPal, &_sysPalette, flag);
		sciPal->timestamp = _sysPalette.timestamp;
		if (_picNotValid == 0 && systime != _sysPalette.timestamp)
			setScreenPalette(&_sysPalette);
	}
}

void SciGuiGfx::MergePalettes(GuiPalette *pFrom, GuiPalette *pTo, uint16 flag) {
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
		res = MatchColor(pTo, pFrom->colors[i].r, pFrom->colors[i].g, pFrom->colors[i].b);
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
	pTo->timestamp = _sysTicks;
}

uint16 SciGuiGfx::MatchColor(GuiPalette*pPal, byte r, byte g, byte b) {
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

void SciGuiGfx::setScreenPalette(GuiPalette*pal) {
	if (pal != &_sysPalette)
		memcpy(&_sysPalette,pal,sizeof(GuiPalette));
	// just copy palette to system
	byte bpal[4 * 256];
	// Get current palette, update it and put back
	_system->grabPalette(bpal, 0, 256);
	for (int16 i = 0; i < 256; i++) {
		if (!pal->colors[i].used)
			continue;
		bpal[i * 4] = pal->colors[i].r * pal->intensity[i] / 100;
		bpal[i * 4 + 1] = pal->colors[i].g * pal->intensity[i] / 100;
		bpal[i * 4 + 2] = pal->colors[i].b * pal->intensity[i] / 100;
		bpal[i * 4 + 3] = 100;
	}
	_system->setPalette(bpal, 0, 256);
}

void SciGuiGfx::getSysPalette(GuiPalette*pal) {
	if (pal != &_sysPalette)
		memcpy(pal, &_sysPalette,sizeof(GuiPalette));
}

GuiPort *SciGuiGfx::SetPort(GuiPort *newPort) {
	GuiPort *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

GuiPort *SciGuiGfx::GetPort(void) {
	return _curPort;
}

void SciGuiGfx::SetOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void SciGuiGfx::MoveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void SciGuiGfx::Move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

GuiResourceId SciGuiGfx::GetFontId() {
	return _curPort->fontId;
}

SciGuiFont *SciGuiGfx::GetFont() {
	GuiResourceId fontId = _curPort->fontId;

	// Workaround: lsl1sci mixes its own internal fonts with the global
	// SCI ones, so we translate them here, by removing their extra bits
	if (_s->_gameName == "lsl1sci")
		fontId &= 0x7ff;

	if ((_font == NULL) || (_font->getResourceId() != _curPort->fontId))
		_font = new SciGuiFont(_s->resMan, fontId);

	return _font;
}

void SciGuiGfx::SetFont(GuiResourceId fontId) {
	GuiResourceId actualFontId = fontId;

	// Workaround: lsl1sci mixes its own internal fonts with the global
	// SCI ones, so we translate them here, by removing their extra bits
	if (_s->_gameName == "lsl1sci")
		actualFontId &= 0x7ff;

	if ((_font == NULL) || (_font->getResourceId() != fontId))
		_font = new SciGuiFont(_s->resMan, actualFontId);

	_curPort->fontId = fontId;
	_curPort->fontHeight = _font->getHeight();
}

void SciGuiGfx::OpenPort(GuiPort *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	GuiPort *tmp = _curPort;
	_curPort = port;
	SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->textFace = 0;
	port->penClr = 0;
	port->backClr = 255;
	port->penMode = 0;
	port->rect = _bounds;
}

void SciGuiGfx::PenColor(int16 color) {
	if (!_s->resMan->isVGA())
		_curPort->penClr = color;
	else
		_curPort->penClr = color | (color << 4);
}

void SciGuiGfx::PenMode(int16 mode) {
	_curPort->penMode = mode;
}

void SciGuiGfx::TextFace(int16 textFace) {
	_curPort->textFace = textFace;
}

int16 SciGuiGfx::GetPointSize(void) {
	return _curPort->fontHeight;
}

void SciGuiGfx::ClearScreen(byte color) {
	FillRect(_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void SciGuiGfx::InvertRect(const Common::Rect &rect) {
	int16 oldpenmode = _curPort->penMode;
	_curPort->penMode = 2;
	FillRect(rect, 1, _curPort->penClr, _curPort->backClr);
	_curPort->penMode = oldpenmode;
}
//-----------------------------
void SciGuiGfx::EraseRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->backClr);
}
//-----------------------------
void SciGuiGfx::PaintRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->penClr);
}

void SciGuiGfx::FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
	Common::Rect r(rect.left, rect.top, rect.right, rect.bottom);
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to fill
		return;

	int16 oldPenMode = _curPort->penMode;
	OffsetRect(r);
	int16 x, y;
	byte curVisual;

	// Doing visual first
	if (drawFlags & SCI_SCREEN_MASK_VISUAL) {
		if (oldPenMode == 2) { // invert mode
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					curVisual = _screen->getVisual(x, y);
					if (curVisual == clrPen) {
						_screen->putPixel(x, y, 1, clrBack, 0, 0);
					} else if (curVisual == clrBack) {
						_screen->putPixel(x, y, 1, clrPen, 0, 0);
					}
				}
			}
		} else { // just fill rect with ClrPen
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					_screen->putPixel(x, y, 1, clrPen, 0, 0);
				}
			}
		}
	}

	if (drawFlags < 2)
		return;
	drawFlags &= SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_CONTROL;

	if (oldPenMode != 2) {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, clrBack, bControl);
			}
		}
	} else {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, !_screen->getPriority(x, y), !_screen->getControl(x, y));
			}
		}
	}
}

void SciGuiGfx::FrameRect(const Common::Rect &rect) {
	Common::Rect r;
	// left
	r = rect;
	r.right = rect.left + 1;
	PaintRect(r);
	// right
	r.right = rect.right;
	r.left = rect.right - 1;
	PaintRect(r);
	//top
	r.left = rect.left;
	r.bottom = rect.top + 1;
	PaintRect(r);
	//bottom
	r.bottom = rect.bottom;
	r.top = rect.bottom - 1;
	PaintRect(r);
}

void SciGuiGfx::OffsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

byte SciGuiGfx::CharHeight(int16 ch) {
#if 0
	CResFont *res = getResFont();
	return res ? res->getCharH(ch) : 0;
#endif
	return 0;
}
//-----------------------------
byte SciGuiGfx::CharWidth(int16 ch) {
	SciGuiFont *font = GetFont();
	return font ? font->getCharWidth(ch) : 0;
}

void SciGuiGfx::ClearChar(int16 chr) {
	if (_curPort->penMode != 1)
		return;
	Common::Rect rect;
	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontHeight;
	rect.left = _curPort->curLeft;
	rect.right = rect.left + CharWidth(chr);
	EraseRect(rect);
}

void SciGuiGfx::DrawChar(int16 chr) {
	chr = chr & 0xFF;
	ClearChar(chr);
	StdChar(chr);
	_curPort->curLeft += CharWidth(chr);
}

void SciGuiGfx::StdChar(int16 chr) {
#if 0
	CResFont*res = getResFont();
	if (res)
		res->Draw(chr, _curPort->top + _curPort->curTop, _curPort->left
				+ _curPort->curLeft, _vSeg, 320, _curPort->penClr,
				_curPort->textFace);
#endif
}

void SciGuiGfx::SetTextFonts(int argc, reg_t *argv) {
	int i;

	if (_textFonts) {
		delete _textFonts;
	}
	_textFontsCount = argc;
	_textFonts = new GuiResourceId[argc];
	for (i = 0; i < argc; i++) {
		_textFonts[i] = (GuiResourceId)argv[i].toUint16();
	}
}

void SciGuiGfx::SetTextColors(int argc, reg_t *argv) {
	int i;

	if (_textColors) {
		delete _textColors;
	}
	_textColorsCount = argc;
	_textColors = new uint16[argc];
	for (i = 0; i < argc; i++) {
		_textColors[i] = argv[i].toUint16();
	}
}

// This internal function gets called as soon as a '|' is found in a text
//  It will process the encountered code and set new font/set color
//  We only support one-digit codes currently, don't know if multi-digit codes are possible
//  Returns textcode character count
int16 SciGuiGfx::TextCodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor) {
	const char *textCode = text;
	int16 textCodeSize = 0;
	char curCode;
	unsigned char curCodeParm;

	// Find the end of the textcode
	while ((++textCodeSize) && (*text++ != 0x7C)) { }

	// possible TextCodes:
	//  c -> sets textColor to current port pen color
	//  cX -> sets textColor to _textColors[X-1]
	curCode = textCode[0];
	curCodeParm = textCode[1];
	if (isdigit(curCodeParm)) {
		curCodeParm -= '0';
	} else {
		curCodeParm = 0;
	}
	switch (curCode) {
	case 'c': // set text color
		if (curCodeParm == 0) {
			_curPort->penClr = orgPenColor;
		} else {
			if (curCodeParm < _textColorsCount) {
				_curPort->penClr = _textColors[curCodeParm];
			}
		}
		break;
	case 'f':
		if (curCodeParm == 0) {
			SetFont(orgFontId);
		} else {
			if (curCodeParm < _textFontsCount) {
				SetFont(_textFonts[curCodeParm]);
			}
		}
		break;
	}
	return textCodeSize;
}

// return max # of chars to fit maxwidth with full words
int16 SciGuiGfx::GetLongest(const char *text, int16 maxWidth, GuiResourceId orgFontId) {
	char curChar;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;

	GetFont();
	if (!_font)
		return 0;

	while (width <= maxWidth) {
		curChar = *text++;
		switch (curChar) {
		case 0x7C:
			curCharCount++;
			curCharCount += TextCodeProcessing(text, orgFontId, oldPenColor);
			continue;

		case 0xD:
			curCharCount++;
			continue;

		case 0xA:
			curCharCount++;
		case 0:
			SetFont(oldFontId);
			PenColor(oldPenColor);
			return curCharCount;

		case ' ':
			maxChars = curCharCount + 1;
			break;
		}
		width += _font->getCharWidth(curChar);
		curCharCount++;
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return maxChars;
}

void SciGuiGfx::TextWidth(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	unsigned char curChar;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;

	textWidth = 0; textHeight = 0;

	GetFont();
	if (_font) {
		text += from;
		while (len--) {
			curChar = *text++;
			switch (curChar) {
			case 0x7C:
				len -= TextCodeProcessing(text, orgFontId, 0);
				break;
			case 0x0A:
			case 0x0D:
				textHeight = MAX<int16> (textHeight, _curPort->fontHeight);
				break;
			default:
				textHeight = MAX<int16> (textHeight, _curPort->fontHeight);
				textWidth += _font->getCharWidth(curChar);
			}
		}
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return;
}

void SciGuiGfx::StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	TextWidth(str, 0, (int16)strlen(str), orgFontId, textWidth, textHeight);
}

int16 SciGuiGfx::TextSize(Common::Rect &rect, const char *str, GuiResourceId fontId, int16 maxWidth) {
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;
	int16 charCount;
	int16 maxTextWidth = 0, textWidth;
	int16 totalHeight = 0, textHeight;

	if (fontId != -1)
		SetFont(fontId);
	rect.top = rect.left = 0;

	if (maxWidth < 0) { // force output as single line
		StringWidth(str, oldFontId, textWidth, textHeight);
		rect.bottom = textHeight;
		rect.right = textWidth;
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxWidth ? maxWidth : 192);
		const char*p = str;
		while (*p) {
			//if (*p == 0xD || *p == 0xA) {
			//	p++;
			//	continue;
			//}
			charCount = GetLongest(p, rect.right, oldFontId);
			if (charCount == 0)
				break;
			TextWidth(p, 0, charCount, oldFontId, textWidth, textHeight);
			maxTextWidth = MAX(textWidth, maxTextWidth);
			totalHeight += textHeight;
			p += charCount;
		}
		rect.bottom = totalHeight;
		rect.right = maxWidth ? maxWidth : MIN(rect.right, maxTextWidth);
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return rect.right;
}

// returns maximum font height used
void SciGuiGfx::DrawText(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	int16 curChar, charWidth;
	Common::Rect rect;

	GetFont();
	if (!_font)
		return;

	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontHeight;
	text += from;
	while (len--) {
		curChar = (*text++);
		switch (curChar) {
		case 0x7C:
			len -= TextCodeProcessing(text, orgFontId, orgPenColor);
			break;

		case 0x0A:
		case 0x0D:
		case 0:
			break;

		default:
			charWidth = _font->getCharWidth(curChar);
			// clear char
			if (_curPort->penMode == 1) {
				rect.left = _curPort->curLeft;
				rect.right = rect.left + charWidth;
				EraseRect(rect);
			}
			// CharStd
			_font->draw(_screen, curChar, _curPort->top + _curPort->curTop, _curPort->left + _curPort->curLeft, _curPort->penClr, _curPort->textFace);
			_curPort->curLeft += charWidth;
		}
	}
}

// returns maximum font height used
void SciGuiGfx::ShowText(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	Common::Rect rect;

	rect.top = _curPort->curTop;
	rect.bottom = rect.top + GetPointSize();
	rect.left = _curPort->curLeft;
	DrawText(text, from, len, orgFontId, orgPenColor);
	rect.right = _curPort->curLeft;
	ShowBits(rect, 1);
}

// Draws a text in rect.
void SciGuiGfx::TextBox(const char *text, int16 bshow, const Common::Rect &rect, int16 align, GuiResourceId fontId) {
	int16 textWidth, textHeight, charCount, offset;
	int16 hline = 0;
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _curPort->penClr;

	if (fontId != -1)
		SetFont(fontId);

	while (*text) {
//		if (*text == 0xD || *text == 0xA) {
//			text++;
//			continue;
//		}
		charCount = GetLongest(text, rect.width(), orgFontId);
		if (charCount == 0)
			break;
		TextWidth(text, 0, charCount, orgFontId, textWidth, textHeight);
		switch (align) {
		case -1: // right-aligned
			offset = rect.width() - textWidth;
			break;
		case 1: // center text
			offset = (rect.width() - textWidth) / 2;
			break;
		default: // left-aligned
			offset = 0;
		}
		MoveTo(rect.left + offset, rect.top + hline);

		if (bshow) {
			ShowText(text, 0, charCount, orgFontId, orgPenColor);
		} else {
			DrawText(text, 0, charCount, orgFontId, orgPenColor);
		}

		hline += textHeight;
		text += charCount;
	}
	SetFont(orgFontId);
	PenColor(orgPenColor);
}

// Update (part of) screen
void SciGuiGfx::ShowBits(const Common::Rect &r, uint16 flags) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_curPort->rect);
	if (rect.isEmpty()) // nothing to show
		return;

	OffsetRect(rect);
	assert((flags&0x8000) == 0);
	_screen->copyToScreen();
//	_system->copyRectToScreen(GetSegment(flags) + _baseTable[rect.top] + rect.left, 320, rect.left, rect.top, rect.width(), rect.height());
//	_system->updateScreen();
}

GuiMemoryHandle SciGuiGfx::SaveBits(const Common::Rect &rect, byte screenMask) {
	GuiMemoryHandle memoryId;
	byte *memoryPtr;
	int size;
	
	Common::Rect r(rect.left, rect.top, rect.right, rect.bottom);
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to save
		return NULL_REG;

	OffsetRect(r); //local port coords to screen coords

	// now actually ask _screen how much space it will need for saving
	size = _screen->getBitsDataSize(r, screenMask);

	memoryId = kalloc(_s->_segMan, "SaveBits()", size);
	memoryPtr = kmem(_s->_segMan, memoryId);
	_screen->saveBits(r, screenMask, memoryPtr);
	return memoryId;
}

void SciGuiGfx::RestoreBits(GuiMemoryHandle memoryHandle) {
	byte *memoryPtr = kmem(_s->_segMan, memoryHandle);;

	if (memoryPtr) {
		_screen->restoreBits(memoryPtr);
		kfree(_s->_segMan, memoryHandle);
	}
}

void SciGuiGfx::Draw_Line(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
	//set_drawing_flag
	byte flag = _screen->getDrawingMask(color, prio, control);
	prio &= 0xF0;
	control &= 0x0F;

	// offseting the line
	left += _curPort->left;
	right += _curPort->left;
	top += _curPort->top;
	bottom += _curPort->top;
	// horizontal line
	if (top == bottom) {
		Draw_Horiz(left, right, top, flag, color, prio, control);
		return;
	}
	// vertical line
	if (left == right) {
		Draw_Vert(top, bottom, left, flag, color, prio, control);
		return;
	}
	// sloped line - draw with Bresenham algorithm
	int dy = bottom - top;
	int dx = right - left;
	int stepy = dy < 0 ? -1 : 1;
	int stepx = dx < 0 ? -1 : 1;
	dy = ABS(dy) << 1;
	dx = ABS(dx) << 1;

	// setting the 1st and last pixel
	_screen->putPixel(left, top, flag, color, prio, control);
	_screen->putPixel(right, bottom, flag, color, prio, control);
	// drawing the line
	if (dx > dy) // going horizontal
	{
		int fraction = dy - (dx >> 1);
		while (left != right) {
			if (fraction >= 0) {
				top += stepy;
				fraction -= dx;
			}
			left += stepx;
			fraction += dy;
			_screen->putPixel(left, top, flag, color, prio, control);
		}
	} else // going vertical
	{
		int fraction = dx - (dy >> 1);
		while (top != bottom) {
			if (fraction >= 0) {
				left += stepx;
				fraction -= dy;
			}
			top += stepy;
			fraction += dx;
			_screen->putPixel(left, top, flag, color, prio, control);
		}
	}
	//g_sci->eventMgr->waitUntil(5);
	//ShowBits(&_rThePort->rect,6);
}

void SciGuiGfx::Draw_Horiz(int16 left, int16 right, int16 top, byte flag, byte color, byte prio, byte control) {
	if (right < left)
		SWAP(right, left);
	for (int i = left; i <= right; i++)
		_screen->putPixel(i, top, flag, color, prio, control);
}

//--------------------------------
void SciGuiGfx::Draw_Vert(int16 top, int16 bottom, int16 left, byte flag, byte color, byte prio, byte control) {
	if (top > bottom)
		SWAP(top, bottom);
	for (int i = top; i <= bottom; i++)
		_screen->putPixel(left, i, flag, color, prio, control);
}

// Bitmap for drawing sierra circles
const byte pattern_Circles[8][15] = {
	{ 0x01 },
	{ 0x03, 0x03, 0x03 },
	{ 0x02, 0x07, 0x07, 0x07, 0x02 },
	{ 0x06, 0x06, 0x0F, 0x0F, 0x0F, 0x06, 0x06 },
	{ 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x04 },
	{ 0x0C, 0x1E, 0x1E, 0x1E, 0x3F, 0x3F, 0x3F, 0x1E, 0x1E, 0x1E, 0x0C },
	{ 0x1C, 0x3E, 0x3E, 0x3E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3E, 0x3E, 0x3E, 0x1C },
	{ 0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x7E, 0x3C, 0x18 }
};

// TODO: perhaps this is a better way to set the pattern_Textures array below?
#if 0
const byte patternTextures[32 * 2] = {
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
	// Now the table is actually duplicated, so we won't need to wrap around
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
};
#endif

const bool pattern_Textures[32 * 8 * 2] = {
	false, false, false, false, false,  true, false, false, // 0x20
	false, false,  true, false, true,  false, false,  true, // 0x94
	false,  true, false, false, false, false, false, false, // 0x02
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false,  true, false, false,  true, // 0x90
	false,  true, false, false, false, false, false,  true, // 0x82
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false,  true, false, false, false,  true, false,  true, // 0xA2
	false,  true, false, false, false, false, false,  true, // 0x82
	 true, false, false,  true, false, false, false, false, // 0x09
	false,  true, false,  true, false, false, false, false, // 0x0A
	false,  true, false, false, false,  true, false, false, // 0x22
	false,  true, false, false,  true, false, false, false, // 0x12
	false, false, false, false,  true, false, false, false, // 0x10
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false,  true, false,  true, false, false, false, // 0x14
	 true, false, false, false,  true, false, false,  true, // 0x91
	false,  true, false,  true, false, false,  true, false, // 0x4A
	 true, false, false, false,  true, false, false,  true, // 0x91
	 true, false, false, false,  true, false, false, false, // 0x11
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false,  true, false, false, false, // 0x12
	 true, false,  true, false, false,  true, false, false, // 0x25
	false, false, false, false,  true, false, false, false, // 0x10
	false,  true, false, false, false,  true, false, false, // 0x22
	false, false, false,  true, false,  true, false,  true, // 0xA8
	false, false,  true, false,  true, false, false, false, // 0x14
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false,  true, false, false, false, false,        // 0x04 (last bit is not mentioned cause original interpreter also ignores that bit)
	// Now the table is actually duplicated, so we won't need to wrap around
	false, false, false, false, false,  true, false, false, // 0x20
	false, false,  true, false, true,  false, false,  true, // 0x94
	false,  true, false, false, false, false, false, false, // 0x02
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false,  true, false, false,  true, // 0x90
	false,  true, false, false, false, false, false,  true, // 0x82
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false,  true, false, false, false,  true, false,  true, // 0xA2
	false,  true, false, false, false, false, false,  true, // 0x82
	 true, false, false,  true, false, false, false, false, // 0x09
	false,  true, false,  true, false, false, false, false, // 0x0A
	false,  true, false, false, false,  true, false, false, // 0x22
	false,  true, false, false,  true, false, false, false, // 0x12
	false, false, false, false,  true, false, false, false, // 0x10
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false,  true, false,  true, false, false, false, // 0x14
	 true, false, false, false,  true, false, false,  true, // 0x91
	false,  true, false,  true, false, false,  true, false, // 0x4A
	 true, false, false, false,  true, false, false,  true, // 0x91
	 true, false, false, false,  true, false, false, false, // 0x11
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false,  true, false, false, false, // 0x12
	 true, false,  true, false, false,  true, false, false, // 0x25
	false, false, false, false,  true, false, false, false, // 0x10
	false,  true, false, false, false,  true, false, false, // 0x22
	false, false, false,  true, false,  true, false,  true, // 0xA8
	false, false,  true, false,  true, false, false, false, // 0x14
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false,  true, false, false, false, false,        // 0x04 (last bit is not mentioned cause original interpreter also ignores that bit)
};
	
// Bit offsets into pattern_textures
const byte pattern_TextureOffset[128] = {
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
	0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
	0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
	0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
	0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
	0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
	0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
};

void SciGuiGfx::Draw_Box(Common::Rect box, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			_screen->putPixel(x, y, flag, color, prio, control);
		}
	}
}

void SciGuiGfx::Draw_TexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const bool *textureData = &pattern_Textures[pattern_TextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (*textureData) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			textureData++;
		}
	}
}

void SciGuiGfx::Draw_Circle(Common::Rect box, byte size, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	byte *circle = (byte *)&pattern_Circles[size];
	byte circleBitmap;
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		circleBitmap = *circle;
		for (x = box.left; x < box.right; x++) {
			if (circleBitmap & 1) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			circleBitmap = circleBitmap >> 1;
		}
		circle++;
	}
}

void SciGuiGfx::Draw_TexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	byte *circle = (byte *)&pattern_Circles[size];
	byte circleBitmap;
	const bool *textureData = &pattern_Textures[pattern_TextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		circleBitmap = *circle;
		for (x = box.left; x < box.right; x++) {
			if (circleBitmap & 1) {
				if (*textureData) {
					_screen->putPixel(x, y, flag, color, prio, control);
				}
				textureData++;
			}
			circleBitmap = circleBitmap >> 1;
		}
		circle++;
	}
}

void SciGuiGfx::Draw_Pattern(int16 x, int16 y, byte color, byte priority, byte control, byte code, byte texture) {
	byte size = code & SCI_PATTERN_CODE_PENSIZE;
	Common::Rect rect;

	// We need to adjust the given coordinates, because the ones given us do not define upper left but somewhat middle
	y -= size;
	x -= (size + 1) >> 1;

	rect.top = y; rect.left = x;
	rect.setHeight((size*2)+1); rect.setWidth(size+1);

	if (code & SCI_PATTERN_CODE_RECTANGLE) {
		// Rectangle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			Draw_TexturedBox(rect, color, priority, control, texture);
		} else {
			Draw_Box(rect, color, priority, control);
		}

	} else {
		// Circle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			Draw_TexturedCircle(rect, size, color, priority, control, texture);
		} else {
			Draw_Circle(rect, size, color, priority, control);
		}
	}
}

void SciGuiGfx::Draw_String(const char *text) {
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _curPort->penClr;

	DrawText(text, 0, strlen(text), orgFontId, orgPenColor);
	SetFont(orgFontId);
	PenColor(orgPenColor);
}

void SciGuiGfx::Pic_Fill(int16 x, int16 y, byte color, byte prio, byte control) {
	Common::Stack<Common::Point> stack;
	Common::Point p, p1;

	byte flag = _screen->getDrawingMask(color, prio, control), fmatch;
	p.x = x + _curPort->left;
	p.y = y + _curPort->top;
	stack.push(p);

	// parameters check
	if ((flag & 2 && prio == 0) || (flag & 3 && control == 0))
		return;

	byte t_col = _screen->getVisual(p.x, p.y);
	byte t_pri = _screen->getPriority(p.x, p.y);
	byte t_con = _screen->getControl(p.x, p.y);
	int16 w, e, a_set, b_set;
	// if in 1st point priority,control or color is already set to target, clear the flag
	if (!_s->resMan->isVGA()) {
		// EGA 16 colors
		if (flag & 1 && ((t_col == (color & 0x0F)) || (t_col == (color >> 4))))
			flag ^= 1;
	} else {
		// VGA 256 colors
		if (flag & 1 && t_col == color)
			flag ^= 1;
	}
	if (flag & 2 && t_pri == prio)
		flag ^= 2;
	if (flag & 4 && t_con == control)
		flag ^= 4;
	if (flag == 0)// nothing to fill
		return;

	// hard borders for filling
	int l = _curPort->rect.left + _curPort->left;
	int t = _curPort->rect.top + _curPort->top;
	int r = _curPort->rect.right + _curPort->left - 1;
	int b = _curPort->rect.bottom + _curPort->top - 1;
	while (stack.size()) {
		p = stack.pop();
		if ((fmatch = _screen->isFillMatch(p.x, p.y, flag, t_col, t_pri, t_con)) == 0) // already filled
			continue;
		_screen->putPixel(p.x, p.y, flag, color, prio, control);
		w = p.x;
		e = p.x;
		// moving west and east pointers as long as there is a matching color to fill
		while (w > l && (fmatch = _screen->isFillMatch(w - 1, p.y, flag, t_col, t_pri, t_con)))
			_screen->putPixel(--w, p.y, fmatch, color, prio, control);
		while (e < r && (fmatch = _screen->isFillMatch(e + 1, p.y, flag, t_col, t_pri, t_con)))
			_screen->putPixel(++e, p.y, fmatch, color, prio, control);
		// checking lines above and below for possible flood targets
		a_set = b_set = 0;
		while (w <= e) {
			if (p.y > t && _screen->isFillMatch(w, p.y - 1, flag, t_col, t_pri, t_con)) { // one line above
				if (a_set == 0) {
					p1.x = w;
					p1.y = p.y - 1;
					stack.push(p1);
					a_set = 1;
				}
			} else
				a_set = 0;

			if (p.y < b && _screen->isFillMatch(w, p.y + 1, flag, t_col, t_pri, t_con)) { // one line below
				if (b_set == 0) {
					p1.x = w;
					p1.y = p.y + 1;
					stack.push(p1);
					b_set = 1;
				}
			} else
				b_set = 0;
			w++;
		}
	}
}

void SciGuiGfx::drawPicture(GuiResourceId pictureId, uint16 style, bool addToFlag, GuiResourceId paletteId) {
	SciGuiPicture *picture;

	picture = new SciGuiPicture(_s, this, _screen, pictureId);
	// do we add to a picture? if not -> clear screen
	if (!addToFlag) {
		ClearScreen(0);
	}
	picture->draw(style, addToFlag, paletteId);
}

void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_system, _s, this, _screen, viewId);
	Common::Rect rect(0, 0);
	Common::Rect clipRect(0, 0);
	if (view) {
		rect.left = leftPos;
		rect.top = topPos;
		rect.right = rect.left + view->getWidth(loopNo, celNo);
		rect.bottom = rect.top + view->getHeight(loopNo, celNo);
		clipRect = rect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) // nothing to draw
			return;
		view->draw(rect, clipRect, loopNo, celNo, priority, paletteNo);
		//if (_picNotValid == 0)
		//	_gfx->ShowBits(rect, 1);
	}
}

void SciGuiGfx::PaletteSetIntensity(int fromColor, int toColor, int intensity, GuiPalette *destPalette) {
	memset(destPalette->intensity + fromColor, intensity, toColor - fromColor);
}

void SciGuiGfx::PaletteAnimate(byte fromColor, byte toColor, int speed) {
	GuiColor col;
	int len = toColor - fromColor - 1;
	uint32 now = _sysTicks;
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
					memmove(&_sysPalette.colors[fromColor+1], &_sysPalette.colors[fromColor], len * sizeof(GuiColor));
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_palSchedules.remove_at(i);
			}
			setScreenPalette(&_sysPalette);
			return;
		}
	}
	// adding a new schedule
	GuiPalSchedule sched;
	sched.from = fromColor;
	sched.schedule = now + ABS(speed);
	_palSchedules.push_back(sched);
}

int16 SciGuiGfx::onControl(uint16 screenMask, Common::Rect rect) {
	Common::Rect outRect(rect.left, rect.top, rect.right, rect.bottom);
	int16 x, y;
	int16 result = 0;

	outRect.clip(_curPort->rect);
	if (outRect.isEmpty()) // nothing to control
		return 0;
	OffsetRect(outRect);

	if (screenMask & SCI_SCREEN_MASK_PRIORITY) {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getPriority(x, y);
			}
		}
	} else {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getControl(x, y);
			}
		}
	}
	return result;
}

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

void SciGuiGfx::AnimateDisposeLastCast() {
	// FIXME
	//if (!_lastCast->first.isNull())
		//_lastCast->DeleteList();
}

void SciGuiGfx::AnimateInvoke(List *list, int argc, reg_t *argv) {
	reg_t curAddress = list->first;
	Node *curNode = lookup_node(_s, curAddress);
	reg_t curObject;
	//uint16 mask;

	while (curNode) {
		curObject = curNode->value;
//      FIXME: check what this code does and remove it or fix it, gregs engine had this check included
//		mask = cobj[_objOfs[2]];
//		if ((mask & 0x100) == 0) {
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.doit, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = lookup_node(_s, curAddress);
//		}
		curAddress = curNode->succ;
		curNode = lookup_node(_s, curAddress);
	}
}

void SciGuiGfx::AnimateFill() {
}

void SciGuiGfx::AnimateSort() {
}

void SciGuiGfx::AnimateUpdate() {
}

void SciGuiGfx::AnimateDrawCels() {
}

void SciGuiGfx::AnimateRestoreAndDelete() {
}

void SciGuiGfx::SetNowSeen(reg_t objectReference) {
	SegManager *segMan = _s->_segMan;
	SciGuiView *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(objectReference, view);
	GuiViewLoopNo loopNo = sign_extend_byte((GuiViewLoopNo)GET_SEL32V(objectReference, loop));
	GuiViewCelNo celNo = sign_extend_byte((GuiViewCelNo)GET_SEL32V(objectReference, cel));
	int16 x = (int16)GET_SEL32V(objectReference, x);
	int16 y = (int16)GET_SEL32V(objectReference, y);
	int16 z = 0;
	if (_s->_kernel->_selectorCache.z > -1) {
		z = (int16)GET_SEL32V(objectReference, z);
	}

	// now get cel rectangle
	view = new SciGuiView(_system, _s, this, _screen, viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_s->_segMan, objectReference, _s->_kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(objectReference, nsLeft, celRect.left);
		PUT_SEL32V(objectReference, nsRight, celRect.right);
		PUT_SEL32V(objectReference, nsTop, celRect.top);
		PUT_SEL32V(objectReference, nsBottom, celRect.bottom);
	}
}

} // End of namespace Sci
