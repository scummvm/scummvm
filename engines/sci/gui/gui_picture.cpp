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
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_picture.h"

namespace Sci {

SciGuiPicture::SciGuiPicture(EngineState *state, SciGuiGfx *gfx, SciGuiScreen *screen, GuiResourceId resourceId)
	: _s(state), _gfx(gfx), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

SciGuiPicture::~SciGuiPicture() {
}

void SciGuiPicture::initData(GuiResourceId resourceId) {
	_resource = _s->resMan->findResource(ResourceId(kResourceTypePic, resourceId), false);
	if (!_resource) {
		error("picture resource %d not found", resourceId);
	}
}

GuiResourceId SciGuiPicture::getResourceId() {
	return _resourceId;
}

#define PALETTE_SIZE 1284
#define CEL_HEADER_SIZE 7
#define EXTRA_MAGIC_SIZE 15

void SciGuiPicture::draw(uint16 style, bool addToFlag, int16 EGApaletteNo) {
	_style = style;
	_addToFlag = addToFlag;
	_EGApaletteNo = EGApaletteNo;
	_curPort = _gfx->GetPort();
	_priority = 0;

	if (READ_LE_UINT16(_resource->data) == 0x26) {
		// SCI 1.1 picture
		draw11();
	} else {
		// Just directly draw the vector data
		_gfx->SetEGApalette();
		drawVectorData(_resource->data, _resource->size);
	}
}

void SciGuiPicture::reset() {
	int16 x, y;
	for (y = _curPort->top; y < _screen->_height; y++) {
		for (x = 0; x < _screen->_width; x++) {
			_screen->putPixel(x, y, SCI_SCREEN_MASK_ALL, 255, 0, 0);
		}
	}
}

void SciGuiPicture::draw11() {
	byte *inbuffer = _resource->data;
	int size = _resource->size;
	int has_view = READ_LE_UINT16(inbuffer + 4);
	int vector_data_ptr = READ_LE_UINT16(inbuffer + 16);
	int vector_size = size - vector_data_ptr;
	int palette_data_ptr = READ_LE_UINT16(inbuffer + 28);
	int view_data_ptr = READ_LE_UINT16(inbuffer + 32);
	int view_size = palette_data_ptr - view_data_ptr;
	int view_rle_ptr = READ_LE_UINT16(inbuffer + view_data_ptr + 24);
	int view_pixel_ptr = READ_LE_UINT16(inbuffer + view_data_ptr + 28);
	byte *view = NULL;
	GuiPalette palette;

	// Create palette and set it
	_gfx->CreatePaletteFromData(inbuffer + palette_data_ptr, &palette);
	_gfx->SetPalette(&palette, 2);

	// display Cel-data
	if (has_view) {
		view = (byte *)malloc(size*2); // is there a way to know how much decoded view-data there will be?
		if (!view) return;
		memcpy(view, inbuffer + view_data_ptr, 8);
		decodeRLE(inbuffer + view_rle_ptr, inbuffer + view_pixel_ptr, view + 8, view_size - 8);
		drawCel(0, 0, view, size * 2);
		free(view);
	}

	// process vector data
	// ?? if we process vector data first some things in sq4 dont seem right, but this way we wont get _priority set
	drawVectorData(inbuffer + vector_data_ptr, vector_size);
}

void SciGuiPicture::decodeRLE(byte *rledata, byte *pixeldata, byte *outbuffer, int size) {
	int pos = 0;
	byte nextbyte;
	byte *rd = rledata;
	byte *ob = outbuffer;
	byte *pd = pixeldata;

	while (pos < size) {
		nextbyte = *(rd++);
		*(ob++) = nextbyte;
		pos ++;
		switch (nextbyte&0xC0) {
		case 0x40 :
		case 0x00 :
			memcpy(ob, pd, nextbyte);
			pd += nextbyte;
			ob += nextbyte;
			pos += nextbyte;
			break;
		case 0xC0 :
			break;
		case 0x80 :
			nextbyte = *(pd++);
			*(ob++) = nextbyte;
			pos ++;
			break;
		}
	}
}

void SciGuiPicture::drawCel(int16 x, int16 y, byte *pdata, int size) {
	byte* pend = pdata + size;
	uint16 width = READ_LE_UINT16(pdata + 0);
	uint16 height = READ_LE_UINT16(pdata + 2);
	signed char dx = *(pdata + 4);
	signed char dy = *(pdata + 5);
	byte priority = _addToFlag ? _priority : 0;
	byte clr = *(pdata + 6);
	if (dx || dy || width != 320)
		debug("Warning: embedded picture cel has width=%d dx=%d dy=%d", width, dx, dy);
	byte *ptr = pdata + 8; // offset to data

	y += _curPort->top;

	uint16 lasty = MIN<int16>(height + y, _curPort->rect.bottom) + _curPort->top;
	byte b, brun;

	while (y < lasty && ptr < pend) {
		b = *ptr++;
		brun = b & 0x3F; // bytes run length on this step
		switch (b & 0xC0) {
		case 0: // copy bytes as-is but skip transparent ones
			while (brun-- && y < lasty && ptr < pend) {
				if ((b = *ptr++) != clr && priority >= _screen->getPriority(x, y)) {
					_screen->putPixel(x, y, 3, b, priority, 0);
				}
				x++;
				if (x >= _screen->_width) {
					x -= _screen->_width; y++;
				}
			}
			break;
		case 0x80: // fill with color
			b = *ptr++;
			while (brun-- && y < lasty) {
				if (priority >= _screen->getPriority(x, y)) {
					_screen->putPixel(x, y, 3, b, priority, 0);
				}
				x++;
				if (x >= _screen->_width) {
					x -= _screen->_width; y++;
				}
			}
			break;
		case 0xC0: // fill with transparent - skip
			x += brun;
			if (x >= _screen->_width) {
				x -= _screen->_width; y++;
			}
			break;
		}
	}
}

void SciGuiPicture::drawCelAmiga(int16 x, int16 y, byte *pdata, int size) {
	byte* pend = pdata + size;
	uint16 width = READ_LE_UINT16(pdata + 0);
	uint16 height = READ_LE_UINT16(pdata + 2);
	signed char dx = *(pdata + 4);
	signed char dy = *(pdata + 5);
	byte priority = _addToFlag ? _priority : 0;
	//byte clr = *(pdata + 8);
	if (dx || dy || width != 320)
		debug("Warning : cel have w=%d dx=%d dy=%d", width, dx, dy);
	byte *ptr = pdata + 8; // offset to data

	y += _curPort->top;

	uint16 lasty = MIN<int16>(height + y, _curPort->rect.bottom) + _curPort->top;
	byte op, col, bytes;
	while (y < lasty && ptr < pend) {
		op = *ptr++;
		if (op & 0x07) {
			bytes = op & 0x07;
			col = op >> 3;
			while (bytes-- && y < lasty) {
				if (priority >= _screen->getPriority(x, y)) {
					_screen->putPixel(x, y, 3, col, priority, 0);
				}
				x++;
				if (x >= _screen->_width) {
					x -= _screen->_width; y++;
				}
			}
		} else {
			bytes = op >> 3;
			x += bytes;
			if (x >= _screen->_width) {
				x -= _screen->_width; y++;
			}
		}
	}
}

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};
#define PIC_OP_FIRST PIC_OP_SET_COLOR

enum {
	PIC_OPX_SCI0_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_SCI0_SET_PALETTE = 1,
	PIC_OPX_SCI0_MONO0 = 2,
	PIC_OPX_SCI0_MONO1 = 3,
	PIC_OPX_SCI0_MONO2 = 4,
	PIC_OPX_SCI0_MONO3 = 5,
	PIC_OPX_SCI0_MONO4 = 6,
	PIC_OPX_SCI0_EMBEDDED_VIEW = 7,
	PIC_OPX_SCI0_SET_PRIORITY_TABLE = 8
};

enum {
	PIC_OPX_SCI1_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_SCI1_EMBEDDED_VIEW = 1,
	PIC_OPX_SCI1_SET_PALETTE = 2,
	PIC_OPX_SCI1_PRIORITY_TABLE_EQDIST = 3,
	PIC_OPX_SCI1_PRIORITY_TABLE_EXPLICIT = 4
};

#define PIC_EGAPALETTE_COUNT 4
#define PIC_EGAPALETTE_SIZE  40
#define PIC_EGAPALETTE_TOTALSIZE PIC_EGAPALETTE_COUNT*PIC_EGAPALETTE_SIZE

static const byte vector_defaultEGApalette[PIC_EGAPALETTE_SIZE] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x88,
	0x88, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88,
	0x88, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x08, 0x91, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x88
};

void SciGuiPicture::drawVectorData(byte *data, int dataSize) {
	byte pic_op;
	byte pic_color = 0, pic_priority = 0x0F, pic_control = 0x0F;
	int16 x = 0, y = 0, oldx, oldy;
	byte EGApalettes[PIC_EGAPALETTE_TOTALSIZE] = {0};
	byte *EGApalette = &EGApalettes[_EGApaletteNo];
	bool EGAmapping = false;
	int curPos = 0;
	uint16 size;
	byte byte;
	int i;
	GuiPalette palette;
	int16 pattern_Code = 0, pattern_Texture = 0;
	bool sci1 = false;

	memset(&palette, 0, sizeof(palette));

	if (_EGApaletteNo >= PIC_EGAPALETTE_COUNT)
		_EGApaletteNo = 0;

	if (getSciVersion() >= SCI_VERSION_1_EGA)
		sci1 = true;

	if (!_s->resMan->isVGA())
		EGAmapping = true;

	for (i = 0; i < PIC_EGAPALETTE_TOTALSIZE; i += PIC_EGAPALETTE_SIZE)
		memcpy(&EGApalettes[i], &vector_defaultEGApalette, sizeof(vector_defaultEGApalette));

	// Drawing
	while (curPos < dataSize) {
		//warning("%X at %d", data[curPos], curPos);
		switch (pic_op = data[curPos++]) {
		case PIC_OP_SET_COLOR:
			byte = data[curPos++];
			pic_color = EGAmapping ? EGApalette[byte] : byte;
			break;
		case PIC_OP_DISABLE_VISUAL:
			pic_color = 0xFF;
			break;

		case PIC_OP_SET_PRIORITY:
			pic_priority = data[curPos++];
			break;
		case PIC_OP_DISABLE_PRIORITY:
			pic_priority = 255;
			break;

		case PIC_OP_SET_CONTROL:
			pic_control = data[curPos++];
			break;
		case PIC_OP_DISABLE_CONTROL:
			pic_control = 255;
			break;

		case PIC_OP_SHORT_LINES: // short line
			vectorGetAbsCoords(data, curPos, oldx, oldy);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetRelCoords(data, curPos, oldx, oldy, x, y);
				//warning("line %d %d -> %d %d", oldy, oldx, y, x);
				_gfx->Draw_Line(oldx, oldy, x, y, pic_color, pic_priority, pic_control);
				oldx = x; oldy = y;
			}
			break;
		case PIC_OP_MEDIUM_LINES: // medium line
			vectorGetAbsCoords(data, curPos, oldx, oldy);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetRelCoordsMed(data, curPos, oldx, oldy, x, y);
				//warning("line %d %d -> %d %d", oldy, oldx, y, x);
				_gfx->Draw_Line(oldx, oldy, x, y, pic_color, pic_priority, pic_control);
				oldx = x; oldy = y;
			}
			break;
		case PIC_OP_LONG_LINES: // long line
			vectorGetAbsCoords(data, curPos, oldx, oldy);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetAbsCoords(data, curPos, x, y);
				//warning("line %d %d -> %d %d", oldy, oldx, y, x);
				_gfx->Draw_Line(oldx, oldy, x, y, pic_color, pic_priority, pic_control);
				oldx = x; oldy = y;
			}
			break;

		case PIC_OP_FILL: //fill
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetAbsCoords(data, curPos, x, y);
				//warning("fill %d %d", y, x);
				_gfx->Pic_Fill(x, y, pic_color, pic_priority, pic_control);
			}
			break;

		case PIC_OP_SET_PATTERN:
			pattern_Code = data[curPos++];
			break;
		case PIC_OP_SHORT_PATTERNS:
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			//warning("pattern %d %d", y, x);
			_gfx->Draw_Pattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoords(data, curPos, x, y, x, y);
				//warning("pattern %d %d", y, x);
				_gfx->Draw_Pattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_MEDIUM_PATTERNS:
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			//warning("pattern %d %d", y, x);
			_gfx->Draw_Pattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoordsMed(data, curPos, x, y, x, y);
				//warning("pattern %d %d", y, x);
				_gfx->Draw_Pattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_ABSOLUTE_PATTERN:
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetAbsCoords(data, curPos, x, y);
				//warning("pattern %d %d", y, x);
				//_gfx->Draw_Pattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;

		case PIC_OP_OPX: // Extended functions
			if (sci1) {
				//warning("OPX SCI1 %X at %d", data[curPos], curPos);
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_SCI1_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						curPos++; // skip commands
					}
					break;
				case PIC_OPX_SCI1_SET_PALETTE:
					curPos += 256 + 4; // Skip over mapping and timestamp
					for (i = 0; i < 256; i++) {
						palette.colors[i].used = data[curPos++];
						palette.colors[i].r = data[curPos++]; palette.colors[i].g = data[curPos++]; palette.colors[i].b = data[curPos++];
					}
					_gfx->SetPalette(&palette, 2);
					break;
				case PIC_OPX_SCI1_EMBEDDED_VIEW: // draw cel
					vectorGetAbsCoords(data, curPos, x, y);
					size = READ_LE_UINT16(data + curPos); curPos += 2;
					drawCel(x, y, data + curPos, size);
					curPos += size;
					break;
				case PIC_OPX_SCI1_PRIORITY_TABLE_EQDIST:
					//FIXME
					//g_sci->InitPri(READ_LE_UINT16(ptr), READ_LE_UINT16(ptr + 2));
					debug(5, "DrawPic::InitPri %d %d", 
						READ_LE_UINT16(data + curPos), READ_LE_UINT16(data + curPos + 2));
					curPos += 4;
					break;
				case PIC_OPX_SCI1_PRIORITY_TABLE_EXPLICIT:
					//FIXME
					//g_sci->PriBands(ptr);
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			} else {
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_SCI0_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						byte = data[curPos++];
						if (byte >= PIC_EGAPALETTE_TOTALSIZE) {
							error("picture trying to write to invalid EGA-palette");
						}
						EGApalettes[byte] = data[curPos++];
					}
					break;
				case PIC_OPX_SCI0_SET_PALETTE:
					byte = data[curPos++];
					if (byte >= PIC_EGAPALETTE_COUNT) {
						error("picture trying to write to invalid palette %d", EGApalette);
					}
					byte *= PIC_EGAPALETTE_SIZE;
					for (i = 0; i < PIC_EGAPALETTE_SIZE; i++) {
						EGApalettes[byte + i] = data[curPos++];
					}
					break;
				case PIC_OPX_SCI0_MONO0:
					curPos += 41;
					break;
				case PIC_OPX_SCI0_MONO1:
				case PIC_OPX_SCI0_MONO3:
					curPos++;
					break;
				case PIC_OPX_SCI0_MONO2:
				case PIC_OPX_SCI0_MONO4:
					break;
				case PIC_OPX_SCI0_EMBEDDED_VIEW:
					vectorGetAbsCoords(data, curPos, x, y);
					size = READ_LE_UINT16(data + curPos); curPos += 2;
					drawCel(x, y, data + curPos, size);
					curPos += size;
					break;
				case PIC_OPX_SCI0_SET_PRIORITY_TABLE:
					//FIXME
					//g_sci->PriBands(ptr);
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			}
			break;
		case PIC_OP_TERMINATE:
			_priority = pic_priority;
			return;
		default:
			error("Unsupported pic-operation %X", pic_op);
		}
	}
	error("picture vector data without terminator");
}

bool SciGuiPicture::vectorIsNonOpcode(byte byte) {
	if (byte >= PIC_OP_FIRST)
		return false;
	return true;
}

void SciGuiPicture::vectorGetAbsCoords(byte *data, int &curPos, int16 &x, int16 &y) {
	byte byte = data[curPos++];
	x = data[curPos++] + ((byte & 0xF0) << 4);
	y = data[curPos++] + ((byte & 0x0F) << 8);
	if (_style & PIC_STYLE_MIRRORED) x = 319 - x;
}

void SciGuiPicture::vectorGetRelCoords(byte *data, int &curPos, int16 oldx, int16 oldy, int16 &x, int16 &y) {
	byte byte = data[curPos++];
	if (byte & 0x80) {
		x = oldx - ((byte >> 4) & 7) * (_style & PIC_STYLE_MIRRORED ? -1 : 1);
	} else {
		x = oldx + (byte >> 4) * (_style & PIC_STYLE_MIRRORED ? -1 : 1);
	}
	if (byte & 0x08) {
		y = oldy - (byte & 7);
	} else {
		y = oldy + (byte & 7);
	}
}

void SciGuiPicture::vectorGetRelCoordsMed(byte *data, int &curPos, int16 oldx, int16 oldy, int16 &x, int16 &y) {
	byte byte = data[curPos++];
	if (byte & 0x80) {
		y = oldy - (byte & 0x7F);
	} else {
		y = oldy + byte;
	}
	byte = data[curPos++];
	if (byte & 0x80) {
		x = oldx - (128 - (byte & 0x7F)) * (_style & PIC_STYLE_MIRRORED ? -1 : 1);
	} else {
		x = oldx + byte * (_style & PIC_STYLE_MIRRORED ? -1 : 1);
	}
}

void SciGuiPicture::vectorGetPatternTexture(byte *data, int &curPos, int16 pattern_Code, int16 &pattern_Texture) {
	if (pattern_Code & SCI_PATTERN_CODE_USE_TEXTURE) {
		pattern_Texture = (data[curPos++] >> 1) & 0x7f;
	}
}

} // End of namespace Sci
