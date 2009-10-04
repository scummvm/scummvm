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

static uint32 _sysTicks;

SciGUIgfx::SciGUIgfx(OSystem *system, EngineState *state, SciGUIscreen *screen)
	: _system(system), _s(state), _screen(screen) {
	init();
	initPalette();
	initTimer();
}

SciGUIgfx::~SciGUIgfx() {
	_system->getTimerManager()->removeTimerProc(&timerHandler);
}

void SciGUIgfx::init() {
	uint16 a = 0;

	_font = NULL;
	_textFonts = NULL; _textFontsCount = 0;
	_textColors = NULL; _textColorsCount = 0;

	_mainPort = mallocPort();
	SetPort(_mainPort);
	OpenPort(_mainPort);

	_menuPort = mallocPort();
	OpenPort(_menuPort);
	SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->_width, _screen->_height);

//	HEAPHANDLE theMenuBarH = heapNewPtr(34, kDataPort, "MenuBar");
//	heapClearPtr(theMenuBarH);
//	_theMenuBar = (Common::Rect *)heap2Ptr(theMenuBarH);
//	*_theMenuBar = Common::Rect(_gfx->RGetPort()->rect.right, 10);

	_sysTicks = 0;
}

void SciGUIgfx::initPalette() {
	int16 i;
	for (i = 0; i < 256; i++) {
		_sysPalette.colors[i].used = 0;
		_sysPalette.colors[i].r = 0;
		_sysPalette.colors[i].g = 0;
		_sysPalette.colors[i].b = 0;
		_sysPalette.intencity[i] = 100;
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
	for(int16 i = 0; i < 256; i++)
	  _clrPowers[i] = i*i;
}

void SciGUIgfx::initTimer() {
	_sysSpeed = 1000000 / 60;
	Common::TimerManager *tm = _system->getTimerManager();
	tm->removeTimerProc(&timerHandler);
	tm->installTimerProc(&timerHandler, _sysSpeed, this);
}

void SciGUIgfx::timerHandler(void *ref) {
	((SciGUIgfx *)ref)->_sysTicks++;
}

sciPort *SciGUIgfx::mallocPort () {
	sciPort *newPort = (sciPort *)malloc(sizeof(sciPort));
	assert(newPort);
	memset(newPort, 0, sizeof(sciPort));
	return newPort;
}

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

void SciGUIgfx::SetEGApalette() {
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
	SetCLUT(&_sysPalette);
}

void SciGUIgfx::CreatePaletteFromData(byte *data, sciPalette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(sciPalette));
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

bool SciGUIgfx::SetResPalette(int16 resourceNo, int16 flag) {
	Resource *palResource = _s->resMan->findResource(ResourceId(kResourceTypePalette, resourceNo), 0);
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;
	sciPalette palette;

	if (palResource) {
		CreatePaletteFromData(palResource->data, &palette);
		SetPalette(&palette, 2);
		return true;
	}
	return false;
}

void SciGUIgfx::SetPalette(sciPalette *sciPal, int16 flag) {
	uint32 systime = _sysPalette.timestamp;
	if (flag == 2 || sciPal->timestamp != systime) {
		MergePalettes(sciPal, &_sysPalette, flag);
		sciPal->timestamp = _sysPalette.timestamp;
		if (_s->pic_not_valid == 0 && systime != _sysPalette.timestamp)
			SetCLUT(&_sysPalette);
	}
}

void SciGUIgfx::MergePalettes(sciPalette *pFrom, sciPalette *pTo, uint16 flag) {
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

uint16 SciGUIgfx::MatchColor(sciPalette*pPal, byte r, byte g, byte b) {
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

void SciGUIgfx::SetCLUT(sciPalette*pal) {
	if (pal != &_sysPalette)
		memcpy(&_sysPalette,pal,sizeof(sciPalette));
	// just copy palette to system
	byte bpal[4 * 256];
	// Get current palette, update it and put back
	_system->grabPalette(bpal, 0, 256);
	for (int16 i = 0; i < 256; i++) {
		if (!pal->colors[i].used)
			continue;
		bpal[i * 4] = pal->colors[i].r * pal->intencity[i] / 100;
		bpal[i * 4 + 1] = pal->colors[i].g * pal->intencity[i] / 100;
		bpal[i * 4 + 2] = pal->colors[i].b * pal->intencity[i] / 100;
		bpal[i * 4 + 3] = 100;
	}
	_system->setPalette(bpal, 0, 256);
	_system->updateScreen();
}

void SciGUIgfx::GetCLUT(sciPalette*pal) {
	if (pal != &_sysPalette)
		memcpy(pal,&_sysPalette,sizeof(sciPalette));
}

sciPort *SciGUIgfx::SetPort(sciPort *newPort) {
	sciPort *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

sciPort *SciGUIgfx::GetPort(void) {
	return _curPort;
}

void SciGUIgfx::SetOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void SciGUIgfx::MoveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void SciGUIgfx::Move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

int16 SciGUIgfx::GetFontId() {
	return _curPort->fontId;
}

SciGUIfont *SciGUIgfx::GetFont() {
	if ((_font == NULL) || (_font->getResourceId() != _curPort->fontId)) {
		_font = new SciGUIfont(_system, _s, _screen, _curPort->fontId);
	}
	return _font;
}

void SciGUIgfx::SetFont(int16 fontId) {
	if ((_font == NULL) || (_font->getResourceId() != fontId)) {
		_font = new SciGUIfont(_system, _s, _screen, fontId);
	}
	_curPort->fontId = fontId;
	_curPort->fontH = _font->getHeight();
}

void SciGUIgfx::OpenPort(sciPort *port) {
	port->fontId = 0;
	port->fontH = 8;

	sciPort *tmp = _curPort;
	_curPort = port;
	SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->textFace = 0;
	port->penClr = 0;
	port->backClr = 0xFF;
	port->penMode = 0;
	memcpy(&port->rect, &_bounds, sizeof(_bounds));
}

void SciGUIgfx::PenColor(int16 color) {
	_curPort->penClr = color;
}

void SciGUIgfx::PenMode(int16 mode) {
	_curPort->penMode = mode;
}

void SciGUIgfx::TextFace(int16 textFace) {
	_curPort->textFace = textFace;
}

int16 SciGUIgfx::GetPointSize(void) {
	return _curPort->fontH;
}

void SciGUIgfx::ClearScreen(byte color) {
	FillRect(_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void SciGUIgfx::InvertRect(const Common::Rect &rect) {
	int16 oldpenmode = _curPort->penMode;
	_curPort->penMode = 2;
	FillRect(rect, 1, _curPort->penClr, _curPort->backClr);
	_curPort->penMode = oldpenmode;
}
//-----------------------------
void SciGUIgfx::EraseRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->backClr);
}
//-----------------------------
void SciGUIgfx::PaintRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->penClr);
}

void SciGUIgfx::FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
	Common::Rect r(rect.left, rect.top, rect.right, rect.bottom);
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to fill
		return;

	int16 oldPenMode = _curPort->penMode;
	OffsetRect(r);
	int16 w = r.width();
	int16 h = r.height();
	int16 x, y;
	byte curVisual;

	// Doing visual first
	if (drawFlags & SCI_SCREEN_MASK_VISUAL) {
		if (oldPenMode == 2) { // invert mode
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					curVisual = _screen->Get_Visual(x, y);
					if (curVisual == clrPen) {
						_screen->Put_Pixel(x, y, 1, clrBack, 0, 0);
					} else if (curVisual == clrBack) {
						_screen->Put_Pixel(x, y, 1, clrPen, 0, 0);
					}
				}
			}
		} else { // just fill rect with ClrPen
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					_screen->Put_Pixel(x, y, 1, clrPen, 0, 0);
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
				_screen->Put_Pixel(x, y, drawFlags, 0, clrBack, bControl);
			}
		}
	} else {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->Put_Pixel(x, y, drawFlags, 0, !_screen->Get_Priority(x, y), !_screen->Get_Control(x, y));
			}
		}
	}
}

void SciGUIgfx::FrameRect(const Common::Rect &rect) {
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

void SciGUIgfx::OffsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

byte SciGUIgfx::CharHeight(int16 ch) {
#if 0
	CResFont *res = getResFont();
	return res ? res->getCharH(ch) : 0;
#endif
	return 0;
}
//-----------------------------
byte SciGUIgfx::CharWidth(int16 ch) {
	SciGUIfont *font = GetFont();
	return font ? font->getCharWidth(ch) : 0;
}

void SciGUIgfx::ClearChar(int16 chr) {
	if (_curPort->penMode != 1)
		return;
	Common::Rect rect;
	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontH;
	rect.left = _curPort->curLeft;
	rect.right = rect.left + CharWidth(chr);
	EraseRect(rect);
}
//-----------------------------
void SciGUIgfx::DrawChar(int16 chr) {
	chr = chr & 0xFF;
	ClearChar(chr);
	StdChar(chr);
	_curPort->curLeft += CharWidth(chr);
}
//-----------------------------
void SciGUIgfx::StdChar(int16 chr) {
#if 0
	CResFont*res = getResFont();
	if (res)
		res->Draw(chr, _curPort->top + _curPort->curTop, _curPort->left
				+ _curPort->curLeft, _vSeg, 320, _curPort->penClr,
				_curPort->textFace);
#endif
}

void SciGUIgfx::SetTextFonts(int argc, reg_t *argv) {
	int i;

	if (_textFonts) {
		delete _textFonts;
	}
	_textFontsCount = argc;
	_textFonts = new sciResourceId[argc];
	for (i = 0; i < argc; i++) {
		_textFonts[i] = (sciResourceId)argv[i].toUint16();
	}
}

void SciGUIgfx::SetTextColors(int argc, reg_t *argv) {
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

// TODO: implement codes
int16 SciGUIgfx::TextWidth(const char *text, int16 from, int16 len) {
	SciGUIfont *font = GetFont();
	if (font) {
		int16 width = 0;
		for (int i = from; i < len; i++)
			width += _font->getCharWidth(text[i]);
		return width;
	}
	return 0;
}

// TODO: implement codes
int16 SciGUIgfx::TextSize(Common::Rect &rect, const char *str, int16 fontId, int16 maxwidth) {
	char buff[1000] = { 0 };
	int16 oldfont = GetFontId();
	if (fontId != -1)
		SetFont(fontId);
	rect.top = rect.left = 0;

	if (maxwidth < 0) { // force output as single line
		rect.bottom = GetPointSize();
		rect.right = StringWidth(str);
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxwidth ? maxwidth : 192);
		int16 height = 0, maxWidth = 0, width, nc;
		const char*p = str;
		while (*p) {
			if (*p == 0xD || *p == 0xA) {
				p++;
				continue;
			}
			nc = GetLongest(p, rect.right);
			if (nc == 0)
				break;
			width = TextWidth(p, 0, nc);
			maxWidth = MAX(width, maxWidth);
			p += nc;
			height++;
		}
		rect.bottom = height * GetPointSize();
		rect.right = maxwidth ? maxwidth : MIN(rect.right, maxWidth);
	}
	SetFont(oldfont);
	return rect.right;
}

// TODO: implement codes
// return max # of chars to fit maxwidth with full words
int16 SciGUIgfx::GetLongest(const char *str, int16 maxWidth) {
	SciGUIfont *font = GetFont();
	if (!font)
		return 0;

	int16 chars = 0, to = 0;
	uint16 width = 0;
	while (width <= maxWidth) {
		switch (str[to]) {
		case ' ':
			chars = to + 1;
			break;
		case 0:
		case 0xD:
		case 0xA:
			return to;
		}
		width += font->getCharWidth(str[to]);
		to++;
	}
	return chars;
}

// TODO: implement codes
void SciGUIgfx::DrawText(const char *text, int16 from, int16 len) {
	int16 chr, width;
	SciGUIfont *font = GetFont();
	Common::Rect rect;

	if (!font)
		return;

	text += from;
	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontH;
	while (len--) {
		chr = (*text++) & 0xFF;
		switch (chr) {
		case 0x7C:
			while ((len--) && (*text++ != 0x7C)) { }
			break;
			
		default:
			width = font->getCharWidth(chr);
			// clear char
			if (_curPort->penMode == 1) {
				rect.left = _curPort->curLeft;
				rect.right = rect.left + width;
				EraseRect(rect);
			}
			// CharStd
			font->draw(chr, _curPort->top + _curPort->curTop, _curPort->left + _curPort->curLeft, _curPort->penClr, _curPort->textFace);
			_curPort->curLeft += width;
		}
	}
}

void SciGUIgfx::ShowText(const char *text, int16 from, int16 len) {
	Common::Rect rect;

	rect.top = _curPort->curTop;
	rect.bottom = rect.top + GetPointSize();
	rect.left = _curPort->curLeft;
	DrawText(text, from, len);
	rect.right = _curPort->curLeft;
	ShowBits(rect, 1);
}

// Draws a text in rect.
// align : -1-right , 0-left, 1-center
void SciGUIgfx::TextBox(const char *text, int16 bshow, const Common::Rect &rect, int16 align, int16 fontId) {
	int16 w, nc, offset;
	int16 hline = 0;
	int16 oldfont = GetFontId();
	int16 rectWidth = rect.width();

	if (fontId != -1)
		SetFont(fontId);

	while (*text) {
		if (*text == 0xD || *text == 0xA) {
			text++;
			continue;
		}
		nc = GetLongest(text, rect.width());
		if (nc == 0)
			break;
		w = TextWidth(text, 0, nc);
		switch (align) {
		case -1:
			offset = rect.width() - w;
			break;
		case 1:
			offset = (rect.width() - w) / 2;
			break;
		default:
			offset = 0;
		}
		MoveTo(rect.left + offset, rect.top + hline);

		if (bshow)
			ShowText(text, 0, nc);
		else
			DrawText(text, 0, nc);
		hline += GetPointSize();
		text += nc;
	}
	SetFont(oldfont);
}

// Update (part of) screen
void SciGUIgfx::ShowBits(const Common::Rect &r, uint16 flags) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_curPort->rect);
	if (rect.isEmpty()) // nothing to show
		return;

	OffsetRect(rect);
	uint16 w = rect.width();
	uint16 h = rect.height();
	assert((flags&0x8000) == 0);
	_screen->UpdateWhole();
//	_system->copyRectToScreen(GetSegment(flags) + _baseTable[rect.top] + rect.left, 320, rect.left, rect.top, w, h);
//	_system->updateScreen();
}

sciMemoryHandle SciGUIgfx::SaveBits(const Common::Rect &rect, byte screenMask) {
	sciMemoryHandle memoryId;
	byte *memoryPtr;
	int size;
	
	Common::Rect r(rect.left, rect.top, rect.right, rect.bottom);
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to save
		return NULL_REG;

	OffsetRect(r); //local port coords to screen coords

	// now actually ask _screen how much space it will need for saving
	size = _screen->BitsGetDataSize(r, screenMask);

	memoryId = kalloc(_s->segMan, "SaveBits()", size);
	memoryPtr = kmem(_s->segMan, memoryId);
	_screen->BitsSave(r, screenMask, memoryPtr);
	return memoryId;
}

void SciGUIgfx::RestoreBits(sciMemoryHandle memoryHandle) {
	byte *memoryPtr = kmem(_s->segMan, memoryHandle);;

	if (memoryPtr) {
		_screen->BitsRestore(memoryPtr);
		kfree(_s->segMan, memoryHandle);
	}
}

void SciGUIgfx::Draw_Line(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
	//set_drawing_flag
	byte flag = _screen->GetDrawingMask(color, prio, control);
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
	_screen->Put_Pixel(left, top, flag, color, prio, control);
	_screen->Put_Pixel(right, bottom, flag, color, prio, control);
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
			_screen->Put_Pixel(left, top, flag, color, prio, control);
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
			_screen->Put_Pixel(left, top, flag, color, prio, control);
		}
	}
	//g_sci->eventMgr->waitUntil(5);
	//ShowBits(&_rThePort->rect,6);
}

void SciGUIgfx::Draw_Horiz(int16 left, int16 right, int16 top, byte flag, byte color, byte prio, byte control) {
	if (right < left)
		SWAP(right, left);
	for (int i = left; i <= right; i++)
		_screen->Put_Pixel(i, top, flag, color, prio, control);
}

//--------------------------------
void SciGUIgfx::Draw_Vert(int16 top, int16 bottom, int16 left, byte flag, byte color, byte prio, byte control) {
	if (top > bottom)
		SWAP(top, bottom);
	for (int i = top; i <= bottom; i++)
		_screen->Put_Pixel(left, i, flag, color, prio, control);
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
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,	0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,	0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,	0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,	0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,	0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,	0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,	0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1 };

void SciGUIgfx::Draw_Box(Common::Rect box, byte color, byte prio, byte control) {
	byte flag = _screen->GetDrawingMask(color, prio, control);
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			_screen->Put_Pixel(x, y, flag, color, prio, control);
		}
	}
}

void SciGUIgfx::Draw_TexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->GetDrawingMask(color, prio, control);
	const bool *textureData = &pattern_Textures[pattern_TextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (*textureData) {
				_screen->Put_Pixel(x, y, flag, color, prio, control);
			}
			textureData++;
		}
	}
}

void SciGUIgfx::Draw_Circle(Common::Rect box, byte size, byte color, byte prio, byte control) {
	byte flag = _screen->GetDrawingMask(color, prio, control);
	byte *circle = (byte *)&pattern_Circles[size];
	byte circleBitmap;
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		circleBitmap = *circle;
		for (x = box.left; x < box.right; x++) {
			if (circleBitmap & 1) {
				_screen->Put_Pixel(x, y, flag, color, prio, control);
			}
			circleBitmap = circleBitmap >> 1;
		}
		circle++;
	}
}

void SciGUIgfx::Draw_TexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->GetDrawingMask(color, prio, control);
	byte *circle = (byte *)&pattern_Circles[size];
	byte circleBitmap;
	const bool *textureData = &pattern_Textures[pattern_TextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		circleBitmap = *circle;
		for (x = box.left; x < box.right; x++) {
			if (circleBitmap & 1) {
				if (*textureData) {
					_screen->Put_Pixel(x, y, flag, color, prio, control);
				}
				textureData++;
			}
			circleBitmap = circleBitmap >> 1;
		}
		circle++;
	}
}

void SciGUIgfx::Draw_Pattern(int16 x, int16 y, byte color, byte priority, byte control, byte code, byte texture) {
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

void SciGUIgfx::Pic_Fill(int16 x, int16 y, byte color, byte prio, byte control) {
	Common::Stack<Common::Point> stack;
	Common::Point p, p1;

	byte flag = _screen->GetDrawingMask(color, prio, control), fmatch;
	p.x = x + _curPort->left;
	p.y = y + _curPort->top;
	stack.push(p);

	// parameters check
	if ((flag & 2 && prio == 0) || (flag & 3 && control == 0))
		return;

	byte t_col = _screen->Get_Visual(p.x, p.y);
	byte t_pri = _screen->Get_Priority(p.x, p.y);
	byte t_con = _screen->Get_Control(p.x, p.y);
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
		if ((fmatch = _screen->IsFillMatch(p.x, p.y, flag, t_col, t_pri, t_con)) == 0) // already filled
			continue;
		_screen->Put_Pixel(p.x, p.y, flag, color, prio, control);
		w = p.x;
		e = p.x;
		// moving west and east pointers as long as there is a matching color to fill
		while (w > l && (fmatch = _screen->IsFillMatch(w - 1, p.y, flag, t_col, t_pri, t_con)))
			_screen->Put_Pixel(--w, p.y, fmatch, color, prio, control);
		while (e < r && (fmatch = _screen->IsFillMatch(e + 1, p.y, flag, t_col, t_pri, t_con)))
			_screen->Put_Pixel(++e, p.y, fmatch, color, prio, control);
		// checking lines above and below for possible flood targets
		a_set = b_set = 0;
		while (w <= e) {
			if (p.y > t && _screen->IsFillMatch(w, p.y - 1, flag, t_col, t_pri, t_con)) { // one line above
				if (a_set == 0) {
					p1.x = w;
					p1.y = p.y - 1;
					stack.push(p1);
					a_set = 1;
				}
			} else
				a_set = 0;

			if (p.y < b && _screen->IsFillMatch(w, p.y + 1, flag, t_col, t_pri, t_con)) { // one line below
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

void SciGUIgfx::drawPicture(sciResourceId pictureId, uint16 style, bool addToFlag, sciResourceId paletteId) {
	SciGUIpicture *picture;

	picture = new SciGUIpicture(_system, _s, this, _screen, pictureId);
	// do we add to a picture? if not -> clear screen
	if (!addToFlag) {
		ClearScreen(0);
	}
	picture->draw(style, addToFlag, paletteId);
}

void SciGUIgfx::drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo) {
	SciGUIview *view = new SciGUIview(_system, _s, this, _screen, viewId);
	Common::Rect rect(0, 0);
	Common::Rect clipRect(0, 0);
	if (view) {
		rect.left = leftPos;
		rect.top = topPos;
		rect.right = rect.left + view->getWidth(loopNo, cellNo);
		rect.bottom = rect.top + view->getHeight(loopNo, cellNo);
		clipRect = rect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) // nothing to draw
			return;
		view->draw(rect, clipRect, loopNo, cellNo, priority, paletteNo);
		//if (_picNotValid == 0)
		//	_gfx->ShowBits(rect, 1);
	}
}

void SciGUIgfx::animatePalette(byte fromColor, byte toColor, int speed) {
	sciColor col;
	int len = toColor - fromColor - 1;
	uint32 now = _sysTicks;
	// search for sheduled animations with the same 'from' value
	int sz = _palSchedules.size();
	for (int i = 0; i < sz; i++) {
		if (_palSchedules[i].from == fromColor) {
			if (_palSchedules[i].schedule < now) {
				if (speed > 0) {
					col = _sysPalette.colors[fromColor];
					memmove(&_sysPalette.colors[fromColor], &_sysPalette.colors[fromColor + 1], len * sizeof(sciColor));
					_sysPalette.colors[toColor - 1] = col;
				} else {
					col = _sysPalette.colors[toColor - 1];
					memmove(&_sysPalette.colors[fromColor+1], &_sysPalette.colors[fromColor], len * sizeof(sciColor));
					_sysPalette.colors[fromColor] = col;
				}
				// removing schedule
				_palSchedules.remove_at(i);
			}
			SetCLUT(&_sysPalette);
			return;
		}
	}
	// adding a new schedule
	sciPalSched sched;
	sched.from = fromColor;
	sched.schedule = now + ABS(speed);
	_palSchedules.push_back(sched);
}

} // end of namespace Sci
