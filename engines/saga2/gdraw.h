/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_GDRAW_H
#define SAGA2_GDRAW_H

#include "saga2/rect.h"

namespace Saga2 {

/* ===================================================================== *
                                Drawing Pens
 * ===================================================================== */

//  A drawing pen. Note that this should later be a class?

typedef uint8       gPen;               // a pen index number

/* ============================================================================ *
   PixelMap: defines a chunky bitmap
 * ============================================================================ */

//  A general purpose image

struct StaticPixelMap {
	StaticPoint16 size;
	uint8 *data;

	int32 bytes() {
		return size.x * size.y;
	}
};

class gPixelMap {
public:
	Extent16        _size;                   // image size
	uint8           *_data;

	gPixelMap() : _data(nullptr) {}

	gPixelMap(StaticPixelMap m) : _size(m.size), _data(m.data) {}

	//  Compute the number of bytes in the pixel map
	int32 bytes() {
		return _size.x * _size.y;
	}
};

//  The simplest kind of pixel map is a static image

class gStaticImage : public gPixelMap {
public:
	//  constructors:

	gStaticImage() {
		_size.x = _size.y = 0;
		_data = NULL;
	}
	gStaticImage(int w, int h, uint8 *buffer) {
		_size.x = w;
		_size.y = h;
		_data = buffer;
	}
};

//  Automatically-remapped images: Eacn image adds itself to a
//  chain at the time it is constructed, and will automatically
//  remap to a new palette by calling the global remap() function.

class gMappedImage : public gPixelMap {
	static gMappedImage *_head;              // first image in map chain

	gMappedImage    *_next;                  // next image to remap
	gPixelMap       _original;

public:
	//  Constructor and destructor
	gMappedImage(int w, int h, uint8 *buffer);
	virtual ~gMappedImage() {
		if (_data) free(_data);
	}
	static void remap(gPen[]);
};

/* ============================================================================ *
   gFont: A bitmapped font structure
 * ============================================================================ */

struct gFont {
	uint16          height;                 // height of the font
	uint16          baseLine;               // baseline of the font
	uint16          rowMod;                 // row modulus for char data

	uint16          charXOffset[256];     // offset of each char in data

	int8            charWidth[256],       // width of character in pixels
	                charKern[256],        // character kern value
	                charSpace[256];       // character space value

	// followed by (row_mod * height) bytes of character data
	byte			*fontdata;
};

//  Prototypes

gFont *LoadFont(char *fontname);
void DisposeFont(gFont *font);
int16 TextWidth(gFont *font, const char *s, int16 length, int16 styles);
int16 WhichChar(gFont *font, uint8 *s, int16 length, int16 maxLen);
int16 WhichIChar(gFont *font, uint8 *s, int16 length, int16 maxLen);
int32 GTextWrap(gFont *font, char *mark, uint16 &count, uint16 width, int16 styles);

/* ============================================================================ *
   gPenState: Used by gPorts to save the current state
 * ============================================================================ */

struct gPenState {
	gPen            fgPen,                  // current foregroung pen
	                bgPen,                  // current drawing mode
	                olPen,                  // text outline pen
	                shPen;                  // text shadow pen
	uint8           drawMode;               // current drawing mode
};

/* ============================================================================ *
   gPort: Facilitates redering operations on PixelMaps
 * ============================================================================ */

enum DrawModes {
	kDrawModeMatte = 0,                      // use transparency
	kDrawModeColor,                          // solid color, use transparency
	kDrawModeReplace,                        // don't use transparency
	kDrawModeComplement,                     // blit in complement mode

	kNumDrawModes
};

enum {
	kTextStyleOutline    = (1 << 0),         // outline the characters
	kTextStyleShadow     = (1 << 1),         // drop shadow the characters
	kTextStyleUnderScore = (1 << 2),         // underscore all chars
	kTextStyleUnderBar   = (1 << 3),         // underscore char after a '_'
	kTextStyleHiLiteBar  = (1 << 4),         // highlight char after a '_'
	kTextStyleThickOutline = (1 << 5),       // extra-thick outline
	kTextStyleBold       = (1 << 6),         // bold
	kTextStyleItalics    = (1 << 7)          // italic
};

enum TextPositions {
	kTextPosLeft         = (1 << 0),
	kTextPosRight        = (1 << 1),
	kTextPosHigh         = (1 << 2),
	kTextPosLow          = (1 << 3)
};

class gPort {
public:
	gPixelMap       *_map;                   // pointer to map

	//  Added by Talin to speed up rendering and allow inverted
	//  gPorts for WinG compatibility

	uint8           *_baseRow;               // address of row 0
	int16           _rowMod;                 // modulus or row

	Point16         _origin;                 // origin drawing point
	Rect16          _clip;                   // clip region DrawPort
	gPen            _fgPen,                  // current foregroung pen
	                _bgPen,                  // current drawing mode
	                _olPen,                  // text outline pen
	                _shPen;                  // text shadow pen
	gPen            *_penMap;                // indirect pen map
	DrawModes       _drawMode;               // current drawing mode
	Point16         _penPos;                 // current pen position
	gFont           *_font;                  // current font
	int16           _textSpacing;            // extra space between characters
	uint16          _textStyles;             // text style bits

	//  Constructor
	gPort() {
		_map = nullptr;
		_baseRow = nullptr;

		_rowMod = 0;
		_penMap = nullptr;
		_drawMode = kDrawModeMatte;
		_font = nullptr;
		_textSpacing = 0;
		_textStyles = 0;
		_fgPen = _bgPen = _olPen = _shPen = 0;
	}

	virtual ~gPort() {}

	//  Set attributes

	virtual void setMap(gPixelMap *newmap, bool inverted = false);

	//  Direct colors

	void setColor(gPen color)              {
		_fgPen = color;
	}
	void setBgColor(gPen color)            {
		_bgPen = color;
	}
	void setShadowColor(gPen color)        {
		_shPen = color;
	}
	void setOutlineColor(gPen color)       {
		_olPen = color;
	}

	//  Indirect colors

	void setPenMap(gPen *pmap)              {
		_penMap = pmap;
	}
	void setIndirectColor(uint8 color) {
		_fgPen = _penMap[color];
	}
	void setIndirectBgColor(uint8 color)   {
		_bgPen = _penMap[color];
	}
	void setIndirectShColor(uint8 color)   {
		_shPen = _penMap[color];
	}
	void setIndirectOLColor(uint8 color)   {
		_olPen = _penMap[color];
	}

	//  modes & styles

	void setMode(DrawModes mode) {
		_drawMode = mode;
	}
	void setStyle(int style)               {
		_textStyles = style;
	}

	//  Pen states

	void setState(gPenState &);
	void getState(gPenState &);

	//  REM: calc intersection of pixel map rect...

	void setClip(const Rect16 &newclip)    {
		_clip = newclip;
	}
	void getClip(Rect16 &r)                {
		r = _clip;
	}

	void setOrigin(Point16 pt)         {
		_origin = pt;
	}
	Point16 getOrigin()                {
		return _origin;
	}

	//  Pen position movement

	void move(int16 x, int16 y)    {
		_penPos.x += x;
		_penPos.y += y;
	}
	void move(Vector16 v)          {
		_penPos += v;
	}
	void moveTo(int16 x, int16 y)  {
		_penPos.x = x;
		_penPos.y = y;
	}
	void moveTo(Point16 p)     {
		_penPos = p;
	}

	//  Simple drawing functions
	//  REM: This should clip!

	virtual void clear() {
		memset(_map->_data, (int)_fgPen, (int)_map->bytes());
	}

	//  Functions to set a single pixel
	//  NOTE: should this actually be a function pointer to deal
	//  with drawing mode?

	virtual void setPixel(int16 x, int16 y, gPen color) {
		if (x >= _clip.x && x < _clip.x + _clip.width
		        && y >= _clip.y && y < _clip.y + _clip.height) {
			_baseRow[(y + _origin.y) * _rowMod + x + _origin.x] = color;
		}
	}
	void setPixel(int16 x, int16 y) {
		setPixel(x, y, _fgPen);
	}
	void setPixel(Point16 p, gPen color) {
		setPixel(p.x, p.y, color);
	}
	void setPixel(Point16 p) {
		setPixel(p.x, p.y, _fgPen);
	}

	//  pixel query functions

	virtual gPen getPixel(int16 x, int16 y) {
		return _baseRow[(y + _origin.y) * _rowMod + x + _origin.x];
	}
	virtual gPen getPixel(Point16 p) {
		return _baseRow[(p.y + _origin.y) * _rowMod + p.x + _origin.x];
	}

	//  Rectangle fill functions

	virtual void fillRect(const Rect16 r);
	void fillRect(int16 x, int16 y, int16 w, int16 h) {
		fillRect(Rect16(x, y, w, h));
	}
	//  Rectangle frame functions //

	virtual void frameRect(const Rect16 r, int16 thick);
	void frameRect(int16 x, int16 y, int16 w, int16 h, int16 thick) {
		frameRect(Rect16(x, y, w, h), thick);
	}

	//  Horiztonal and vertical lines

	virtual void hLine(int16 x, int16 y, int16 width);
	virtual void vLine(int16 x, int16 y, int16 height);

	//  Bresenham line-drawing functions

	virtual void line(int16 x1, int16 y1, int16 x2, int16 y2);
	void line(Point16 from, Point16 to) {
		line(from.x, from.y, to.x, to.y);
	}
	void drawTo(int16 x, int16 y) {
		line(_penPos.x, _penPos.y, x, y);
		_penPos.x = x;
		_penPos.y = y;
	}
	void drawTo(Point16 to) {
		line(_penPos, to);
		_penPos = to;
	}
	void draw(int16 x, int16 y) {
		line(_penPos.x, _penPos.y, _penPos.x + x, _penPos.y + y);
		_penPos.x += x;
		_penPos.y += y;
	}
	void draw(Vector16 v) {
		line(_penPos, v);
		_penPos += v;
	}

	//  Blitting functions

	virtual void bltPixels(const gPixelMap &src,
	                       int src_x, int src_y,
	                       int dst_x, int dst_y,
	                       int width, int height);

	virtual void bltPixelMask(gPixelMap     &src,
	                          gPixelMap       &msk,
	                          int src_x, int src_y,
	                          int dst_x, int dst_y,
	                          int width, int height);

	virtual void scrollPixels(const Rect16 r, int dx, int dy);

	//  Text rendering functions

	void setFont(gFont *newFont) {
		_font = newFont;
	}
	void setTextSpacing(int16 fs) {
		_textSpacing = fs;
	}

private:
	//  Unclipped text rendering
	void drawStringChars(const char *, int16, gPixelMap &, int, int);
	//  Clipped but still low-level
	int16 drawClippedString(const char *str, int16 len, int xpos, int ypos);
public:

	//  Draw a text string with the current settings at the
	//  current pen position. If the length is < 0, then use the
	//  natural string length.
	void drawText(const char *str, int16 length = -1);
	void drawTextInBox(const char *str, int16 length,
	                   const Rect16 &r, int16 pos,
	                   Point16 borders);
};

void mapImage(gPort &from, gPort &to, gPen map[]);
void mapImage(gPixelMap &from, gPixelMap &to, gPen map[]);

bool NewTempPort(gPort &, int width, int height);
void DisposeTempPort(gPort &);

/* ============================================================================ *
   gSavePort: a class which auto-saves and auto-restores the port state
 * ============================================================================ */

struct gSavePort {
	gPenState       state;                  // saved state of port
public:
	gPort           &port;                  // port which was saved

	gSavePort(gPort &p) : port(p) {
		port.getState(state);
	}
	~gSavePort() {
		port.setState(state);
	}
};

#define SAVE_GPORT_STATE(p) gSavePort sp( p )

} // end of namespace Saga2

#endif
