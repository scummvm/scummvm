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
 */

#include "common/span.h"
#include "common/stack.h"
#include "common/system.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/picture.h"

namespace Sci {

//#define DEBUG_PICTURE_DRAW

GfxPicture::GfxPicture(ResourceManager *resMan, GfxCoordAdjuster16 *coordAdjuster, GfxPorts *ports, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId, bool EGAdrawingVisualize)
	: _resMan(resMan),
	_coordAdjuster(coordAdjuster),
	_ports(ports), _screen(screen),
	_palette(palette),
	_resourceId(resourceId),
	_resourceType(SCI_PICTURE_TYPE_REGULAR),
	_mirroredFlag(false),
	_addToFlag(false),
	_EGApaletteNo(0),
	_priority(0),
	_EGAdrawingVisualize(EGAdrawingVisualize) {

	assert(resourceId != -1);
	initData(resourceId);
}

GfxPicture::~GfxPicture() {
	_resMan->unlockResource(_resource);
}

void GfxPicture::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypePic, resourceId), true);
	if (!_resource) {
		error("picture resource %d not found", resourceId);
	}
}

GuiResourceId GfxPicture::getResourceId() {
	return _resourceId;
}

// differentiation between various picture formats can NOT get done using sci-version checks.
//  Games like PQ1 use the "old" vector data picture format, but are actually SCI1.1
//  We should leave this that way to decide the format on-the-fly instead of hardcoding it in any way
void GfxPicture::draw(bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	uint16 headerSize;

	_mirroredFlag = mirroredFlag;
	_addToFlag = addToFlag;
	_EGApaletteNo = EGApaletteNo;
	_priority = 0;

	headerSize = _resource->getUint16LEAt(0);
	switch (headerSize) {
	case 0x26: // SCI 1.1 VGA picture
		_resourceType = SCI_PICTURE_TYPE_SCI11;
		drawSci11Vga();
		break;
	default:
		// VGA, EGA or Amiga vector data
		_resourceType = SCI_PICTURE_TYPE_REGULAR;
		drawVectorData(*_resource);
	}
}

#if 0
void GfxPicture::reset() {
	int16 startY = _ports->getPort()->top;
	int16 startX = 0;
	int16 x, y;
	_screen->vectorAdjustCoordinate(&startX, &startY);
	for (y = startY; y < _screen->getHeight(); y++) {
		for (x = startX; x < _screen->getWidth(); x++) {
			_screen->vectorPutPixel(x, y, GFX_SCREEN_MASK_ALL, 255, 0, 0);
		}
	}
}
#endif

void GfxPicture::drawSci11Vga() {
	SciSpan<const byte> inbuffer(*_resource);
	int priorityBandsCount = inbuffer[3];
	int has_cel = inbuffer[4];
	int vector_dataPos = inbuffer.getUint32LEAt(16);
	int vector_size = _resource->size() - vector_dataPos;
	int palette_data_ptr = inbuffer.getUint32LEAt(28);
	int cel_headerPos = inbuffer.getUint32LEAt(32);
	int cel_RlePos = inbuffer.getUint32LEAt(cel_headerPos + 24);
	int cel_LiteralPos = inbuffer.getUint32LEAt(cel_headerPos + 28);
	Palette palette;

	// Header
	// [headerSize:WORD] [unknown:BYTE] [priorityBandCount:BYTE] [hasCel:BYTE] [unknown:BYTE]
	// [unknown:WORD] [unknown:WORD] [unknown:WORD] [unknown:WORD] [unknown:WORD]
	// Offset 16
	// [vectorDataOffset:DWORD] [unknown:DWORD] [unknown:DWORD] [paletteDataOffset:DWORD]
	// Offset 32
	// [celHeaderOffset:DWORD] [unknown:DWORD]
	// [priorityBandData:WORD] * priorityBandCount
	// [priority:BYTE] [unknown:BYTE]

	// priority bands are supposed to be 14 for sci1.1 pictures
	assert(priorityBandsCount == 14);

	if (_addToFlag) {
		_priority = inbuffer[40 + priorityBandsCount * 2] & 0xF;
	}

	// display Cel-data
	if (has_cel) {
		// Create palette and set it
		_palette->createFromData(inbuffer.subspan(palette_data_ptr), &palette);
		_palette->set(&palette, true);

		drawCelData(inbuffer, cel_headerPos, cel_RlePos, cel_LiteralPos, 0, 0, 0, 0, false);
	}

	// process vector data
	drawVectorData(inbuffer.subspan(vector_dataPos, vector_size));

	// Set priority band information
	_ports->priorityBandsInitSci11(inbuffer.subspan(40));
}

extern void unpackCelData(const SciSpan<const byte> &inBuffer, SciSpan<byte> &celBitmap, byte clearColor, int rlePos, int literalPos, ViewType viewType, uint16 width, bool isMacSci11ViewData);

void GfxPicture::drawCelData(const SciSpan<const byte> &inbuffer, int headerPos, int rlePos, int literalPos, int16 drawX, int16 drawY, int16 pictureX, int16 pictureY, bool isEGA) {
	const SciSpan<const byte> headerPtr = inbuffer.subspan(headerPos);
	const SciSpan<const byte> rlePtr = inbuffer.subspan(rlePos);

	// if the picture is not an overlay and we are also not in EGA mode, use priority 0
	const byte priority = (!isEGA && !_addToFlag) ? 0 : _priority;

	// Width/height here are always LE, even in Mac versions
	uint16 width = headerPtr.getUint16LEAt(0);
	uint16 height = headerPtr.getUint16LEAt(2);

	// displaceX, displaceY fields are ignored, and may contain garbage
	// (e.g. pic 261 in Dr. Brain 1 Spanish - bug #6388)
	//int16 displaceX = (signed char)headerPtr[4];
	//int16 displaceY = (unsigned char)headerPtr[5];
	//if (displaceX || displaceY)
	//	error("unsupported embedded cel-data in picture");

	byte clearColor;
	if (_resourceType == SCI_PICTURE_TYPE_SCI11)
		// SCI1.1 uses hardcoded clearcolor for pictures, even if cel header specifies otherwise
		clearColor = _screen->getColorWhite();
	else
		clearColor = headerPtr[6];

	// We will unpack cel-data into a temporary buffer and then plot it to screen
	//  That needs to be done cause a mirrored picture may be requested
	int pixelCount = width * height;
	Common::SpanOwner<SciSpan<byte> > celBitmap;
	celBitmap->allocate(pixelCount, _resource->name());
	unpackCelData(inbuffer, *celBitmap, clearColor, rlePos, literalPos, _resMan->getViewType(), width, false);

	Common::Rect displayArea = _coordAdjuster->pictureGetDisplayArea();

	// Horizontal clipping
	uint16 skipCelBitmapPixels = 0;
	int16 displayWidth = width;
	if (pictureX) {
		// horizontal scroll position for picture active, we need to adjust drawX accordingly
		drawX -= pictureX;
		if (drawX < 0) {
			skipCelBitmapPixels = -drawX;
			displayWidth -= skipCelBitmapPixels;
			drawX = 0;
		}
	}

	// Vertical clipping
	uint16 skipCelBitmapLines = 0;
	int16 displayHeight = height;
	if (pictureY) {
		// vertical scroll position for picture active, we need to adjust drawY accordingly
		// TODO: Finish this
		/*drawY -= pictureY;
		if (drawY < 0) {
			skipCelBitmapLines = -drawY;
			displayHeight -= skipCelBitmapLines;
			drawY = 0;
		}*/
	}

	if (displayWidth > 0 && displayHeight > 0) {
		int16 y = displayArea.top + drawY;
		int16 lastY = MIN<int16>(height + y, displayArea.bottom);
		int16 leftX = displayArea.left + drawX;
		int16 rightX = MIN<int16>(displayWidth + leftX, displayArea.right);

		uint16 sourcePixelSkipPerRow = 0;
		if (width > rightX - leftX)
			sourcePixelSkipPerRow = width - (rightX - leftX);

		// Change clearcolor to white, if we dont add to an existing picture. That way we will paint everything on screen
		// but white and that won't matter because the screen is supposed to be already white. It seems that most (if not all)
		// SCI1.1 games use color 0 as transparency and SCI1 games use color 255 as transparency. Sierra SCI seems to paint
		// the whole data to screen and won't skip over transparent pixels. So this will actually make it work like Sierra.
		if (!_addToFlag)
			clearColor = _screen->getColorWhite();

		byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY;

		SciSpan<const byte> ptr = *celBitmap;
		ptr += skipCelBitmapPixels;
		ptr += skipCelBitmapLines * width;

		if ((!isEGA) || (priority < 16)) {
			// VGA + EGA, EGA only checks priority, when given priority is below 16
			if (!_mirroredFlag) {
				// Draw bitmap to screen
				int16 x = leftX;
				while (y < lastY) {
					byte curByte = *ptr++;
					if ((curByte != clearColor) && (priority >= _screen->getPriority(x, y)))
						_screen->putPixel(x, y, drawMask, curByte, priority, 0);

					x++;

					if (x >= rightX) {
						ptr += sourcePixelSkipPerRow;
						x = leftX;
						y++;
					}
				}
			} else {
				// Draw bitmap to screen (mirrored)
				int16 x = rightX - 1;
				while (y < lastY) {
					byte curByte = *ptr++;
					if ((curByte != clearColor) && (priority >= _screen->getPriority(x, y)))
						_screen->putPixel(x, y, drawMask, curByte, priority, 0);

					if (x == leftX) {
						ptr += sourcePixelSkipPerRow;
						x = rightX;
						y++;
					}

					x--;
				}
			}
		} else {
			// EGA, when priority is above 15
			//  we don't check priority and also won't set priority at all
			//  fixes picture 48 of kq5 (island overview). Bug #5182
			if (!_mirroredFlag) {
				// EGA+priority>15: Draw bitmap to screen
				int16 x = leftX;
				while (y < lastY) {
					byte curByte = *ptr++;
					if (curByte != clearColor)
						_screen->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, curByte, 0, 0);

					x++;

					if (x >= rightX) {
						ptr += sourcePixelSkipPerRow;
						x = leftX;
						y++;
					}
				}
			} else {
				// EGA+priority>15: Draw bitmap to screen (mirrored)
				int16 x = rightX - 1;
				while (y < lastY) {
					byte curByte = *ptr++;
					if (curByte != clearColor)
						_screen->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, curByte, 0, 0);

					if (x == leftX) {
						ptr += sourcePixelSkipPerRow;
						x = rightX;
						y++;
					}

					x--;
				}
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
	PIC_OPX_EGA_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_EGA_SET_PALETTE = 1,
	PIC_OPX_EGA_MONO0 = 2,
	PIC_OPX_EGA_MONO1 = 3,
	PIC_OPX_EGA_MONO2 = 4,
	PIC_OPX_EGA_MONO3 = 5,
	PIC_OPX_EGA_MONO4 = 6,
	PIC_OPX_EGA_EMBEDDED_VIEW = 7,
	PIC_OPX_EGA_SET_PRIORITY_TABLE = 8
};

enum {
	PIC_OPX_VGA_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_VGA_EMBEDDED_VIEW = 1,
	PIC_OPX_VGA_SET_PALETTE = 2,
	PIC_OPX_VGA_PRIORITY_TABLE_EQDIST = 3,
	PIC_OPX_VGA_PRIORITY_TABLE_EXPLICIT = 4
};

//#define DEBUG_PICTURE_DRAW 1

#ifdef DEBUG_PICTURE_DRAW
const char *picOpcodeNames[] = {
	"Set color",
	"Disable visual",
	"Set priority",
	"Disable priority",
	"Short patterns",
	"Medium lines",
	"Long lines",
	"Short lines",
	"Fill",
	"Set pattern",
	"Absolute pattern",
	"Set control",
	"Disable control",
	"Medium patterns",
	"Extended opcode",
	"Terminate"
};

const char *picExOpcodeNamesEGA[] = {
	"Set palette entries",
	"Set palette",
	"Mono0",
	"Mono1",
	"Mono2",
	"Mono3",
	"Mono4",
	"Embedded view",
	"Set priority table"
};

const char *picExOpcodeNamesVGA[] = {
	"Set palette entries",
	"Embedded view",
	"Set palette",
	"Set priority table (eqdist)",
	"Set priority table (explicit)"
};
#endif

#define PIC_EGAPALETTE_COUNT 4
#define PIC_EGAPALETTE_SIZE  40
#define PIC_EGAPALETTE_TOTALSIZE PIC_EGAPALETTE_COUNT*PIC_EGAPALETTE_SIZE
#define PIC_EGAPRIORITY_SIZE PIC_EGAPALETTE_SIZE

static const byte vector_defaultEGApalette[PIC_EGAPALETTE_SIZE] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x88,
	0x88, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88,
	0x88, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x08, 0x91, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x88
};

static const byte vector_defaultEGApriority[PIC_EGAPRIORITY_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

void GfxPicture::drawVectorData(const SciSpan<const byte> &data) {
	byte pic_op;
	byte pic_color = _screen->getColorDefaultVectorData();
	byte pic_priority = 255, pic_control = 255;
	int16 x = 0, y = 0, oldx, oldy;
	byte EGApalettes[PIC_EGAPALETTE_TOTALSIZE] = {0};
	byte EGApriority[PIC_EGAPRIORITY_SIZE] = {0};
	bool isEGA = false;
	uint curPos = 0;
	uint16 size;
	byte pixel;
	int i;
	Palette palette;
	int16 pattern_Code = 0, pattern_Texture = 0;
	bool icemanDrawFix = false;

	memset(&palette, 0, sizeof(palette));

	if (_EGApaletteNo >= PIC_EGAPALETTE_COUNT) {
		_EGApaletteNo = 0;
	}
	byte *EGApalette = &EGApalettes[_EGApaletteNo * PIC_EGAPALETTE_SIZE];

	if (_resMan->getViewType() == kViewEga) {
		isEGA = true;
		// setup default mapping tables
		for (i = 0; i < PIC_EGAPALETTE_TOTALSIZE; i += PIC_EGAPALETTE_SIZE)
			memcpy(&EGApalettes[i], &vector_defaultEGApalette, sizeof(vector_defaultEGApalette));
		memcpy(&EGApriority, &vector_defaultEGApriority, sizeof(vector_defaultEGApriority));

		if (g_sci->getGameId() == GID_ICEMAN) {
			// WORKAROUND: we remove certain visual&priority lines in underwater
			// rooms of iceman, when not dithering the picture. Normally those
			// lines aren't shown, because they share the same color as the
			// dithered fill color combination. When not dithering, those lines
			// would appear and get distracting.
			if ((_screen->isUnditheringEnabled()) && ((_resourceId >= 53 && _resourceId <= 58) || (_resourceId == 61)))
				icemanDrawFix = true;
		}
	}

	// Drawing
	while (curPos < data.size()) {
#ifdef DEBUG_PICTURE_DRAW
		debug("Picture op: %X (%s) at %d", data[curPos], picOpcodeNames[data[curPos] - 0xF0], curPos);
		_screen->copyToScreen();
		g_system->updateScreen();
		g_system->delayMillis(400);
#endif
		switch (pic_op = data[curPos++]) {
		case PIC_OP_SET_COLOR:
			pic_color = data[curPos++];
			if (isEGA) {
				pic_color = EGApalette[pic_color];
				pic_color ^= pic_color << 4;
			}
			break;
		case PIC_OP_DISABLE_VISUAL:
			pic_color = 0xFF;
			break;

		case PIC_OP_SET_PRIORITY:
			pic_priority = data[curPos++] & 0x0F;
			if (isEGA)
				pic_priority = EGApriority[pic_priority];
			break;
		case PIC_OP_DISABLE_PRIORITY:
			pic_priority = 255;
			break;

		case PIC_OP_SET_CONTROL:
			pic_control = data[curPos++] & 0x0F;
			break;
		case PIC_OP_DISABLE_CONTROL:
			pic_control = 255;
			break;

		case PIC_OP_SHORT_LINES: // short line
			vectorGetAbsCoords(data, curPos, x, y);
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetRelCoords(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;
		case PIC_OP_MEDIUM_LINES: // medium line
			vectorGetAbsCoords(data, curPos, x, y);
			if (icemanDrawFix) {
				// WORKAROUND: remove certain lines in iceman - see above
				if ((pic_color == 1) && (pic_priority == 14)) {
					if ((y < 100) || (!(y & 1))) {
						pic_color = 255;
						pic_priority = 255;
					}
				}
			}
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetRelCoordsMed(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;
		case PIC_OP_LONG_LINES: // long line
			vectorGetAbsCoords(data, curPos, x, y);
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetAbsCoords(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;

		case PIC_OP_FILL: //fill
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetAbsCoords(data, curPos, x, y);
				vectorFloodFill(x, y, pic_color, pic_priority, pic_control);
			}
			break;

		// Pattern opcodes are handled in sierra sci1.1+ as actual NOPs and
		// normally they definitely should not occur inside picture data for
		// such games.
		case PIC_OP_SET_PATTERN:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11) {
				if (g_sci->getGameId() == GID_SQ4) {
					// WORKAROUND: For SQ4 / for some pictures handle this like
					// a terminator. This picture includes garbage data, first a
					// set pattern w/o parameter and then short pattern. I guess
					// that garbage is a left over from the sq4-floppy (sci1) to
					// sq4-cd (sci1.1) conversion.
					switch (_resourceId) {
					case 35:
					case 381:
					case 376:
					case 390: // NRS SQ4 Update patch
						return;
					default:
						break;
					}
				}
				error("pic-operation set pattern inside sci1.1+ vector data");
			}
			pattern_Code = data[curPos++];
			break;
		case PIC_OP_SHORT_PATTERNS:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation short pattern inside sci1.1+ vector data");
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoords(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_MEDIUM_PATTERNS:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation medium pattern inside sci1.1+ vector data");
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoordsMed(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_ABSOLUTE_PATTERN:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation absolute pattern inside sci1.1+ vector data");
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetAbsCoords(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;

		case PIC_OP_OPX: // Extended functions
			if (isEGA) {
#ifdef DEBUG_PICTURE_DRAW
				debug("* Picture ex op: %X (%s) at %d", data[curPos], picExOpcodeNamesEGA[data[curPos]], curPos);
#endif
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_EGA_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						pixel = data[curPos++];
						if (pixel >= PIC_EGAPALETTE_TOTALSIZE) {
							error("picture trying to write to invalid EGA-palette");
						}
						EGApalettes[pixel] = data[curPos++];
					}
					break;
				case PIC_OPX_EGA_SET_PALETTE:
					pixel = data[curPos++];
					if (pixel >= PIC_EGAPALETTE_COUNT) {
						error("picture trying to write to invalid palette %d", (int)pixel);
					}
					pixel *= PIC_EGAPALETTE_SIZE;
					for (i = 0; i < PIC_EGAPALETTE_SIZE; i++) {
						EGApalettes[pixel + i] = data[curPos++];
					}
					break;
				case PIC_OPX_EGA_MONO0:
					curPos += 41;
					break;
				case PIC_OPX_EGA_MONO1:
				case PIC_OPX_EGA_MONO3:
					curPos++;
					break;
				case PIC_OPX_EGA_MONO2:
				case PIC_OPX_EGA_MONO4:
					break;
				case PIC_OPX_EGA_EMBEDDED_VIEW:
					vectorGetAbsCoordsNoMirror(data, curPos, x, y);
					size = data.getUint16LEAt(curPos); curPos += 2;
					// hardcoded in SSCI, 16 for SCI1early excluding Space Quest 4, 0 for anything else
					//  fixes sq4 pictures 546+547 (Vohaul's head and Roger Jr trapped). Bug #5250
					//  fixes sq4 picture 631 (SQ1 view from cockpit). Bug 5249
					if ((getSciVersion() <= SCI_VERSION_1_EARLY) && (g_sci->getGameId() != GID_SQ4)) {
						_priority = 16;
					} else {
						_priority = 0;
					}
					drawCelData(data, curPos, curPos + 8, 0, x, y, 0, 0, true);
					curPos += size;
					break;
				case PIC_OPX_EGA_SET_PRIORITY_TABLE:
					_ports->priorityBandsInit(data.subspan(curPos, 14));
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			} else {
#ifdef DEBUG_PICTURE_DRAW
				debug("* Picture ex op: %X (%s) at %d", data[curPos], picExOpcodeNamesVGA[data[curPos]], curPos);
#endif
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_VGA_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						curPos++; // skip commands
					}
					break;
				case PIC_OPX_VGA_SET_PALETTE:
					if (_resMan->getViewType() == kViewAmiga || _resMan->getViewType() == kViewAmiga64) {
						if ((data[curPos] == 0x00) && (data[curPos + 1] == 0x01) && ((data[curPos + 32] & 0xF0) != 0xF0)) {
							// Left-Over VGA palette, we simply ignore it
							curPos += 256 + 4 + 1024;
						} else {
							// Setting half of the Amiga palette
							_palette->modifyAmigaPalette(data.subspan(curPos));
							curPos += 32;
						}
					} else {
						curPos += 256 + 4; // Skip over mapping and timestamp
						for (i = 0; i < 256; i++) {
							palette.colors[i].used = data[curPos++];
							palette.colors[i].r = data[curPos++]; palette.colors[i].g = data[curPos++]; palette.colors[i].b = data[curPos++];
						}
						_palette->set(&palette, true);
					}
					break;
				case PIC_OPX_VGA_EMBEDDED_VIEW: // draw cel
					vectorGetAbsCoordsNoMirror(data, curPos, x, y);
					size = data.getUint16LEAt(curPos); curPos += 2;
					if (getSciVersion() <= SCI_VERSION_1_EARLY) {
						// During SCI1Early sierra always used 0 as priority for cels inside picture resources
						//  fixes Space Quest 4 orange ship lifting off (bug #6446)
						_priority = 0;
					} else {
						_priority = pic_priority; // set global priority so the cel gets drawn using current priority as well
					}
					drawCelData(data, curPos, curPos + 8, 0, x, y, 0, 0, false);
					curPos += size;
					break;
				case PIC_OPX_VGA_PRIORITY_TABLE_EQDIST:
					_ports->priorityBandsInit(-1, data.getUint16LEAt(curPos), data.getUint16LEAt(curPos + 2));
					curPos += 4;
					break;
				case PIC_OPX_VGA_PRIORITY_TABLE_EXPLICIT:
					_ports->priorityBandsInit(data.subspan(curPos, 14));
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			}
			break;
		case PIC_OP_TERMINATE:
			_priority = pic_priority;
			// Dithering EGA pictures
			if (isEGA) {
				_screen->dither(_addToFlag);
				switch (g_sci->getGameId()) {
				case GID_SQ3:
					switch (_resourceId) {
					case 154: // SQ3: intro, ship gets sucked in
						_screen->ditherForceDitheredColor(0xD0);
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
			return;
		default:
			error("Unsupported pic-operation %X", pic_op);
		}
		if ((_EGAdrawingVisualize) && (isEGA)) {
			_screen->copyToScreen();
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
	error("picture vector data without terminator");
}

bool GfxPicture::vectorIsNonOpcode(byte pixel) {
	if (pixel >= PIC_OP_FIRST)
		return false;
	return true;
}

void GfxPicture::vectorGetAbsCoords(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	x = data[curPos++] + ((pixel & 0xF0) << 4);
	y = data[curPos++] + ((pixel & 0x0F) << 8);
	if (_mirroredFlag) x = 319 - x;
}

void GfxPicture::vectorGetAbsCoordsNoMirror(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	x = data[curPos++] + ((pixel & 0xF0) << 4);
	y = data[curPos++] + ((pixel & 0x0F) << 8);
}

void GfxPicture::vectorGetRelCoords(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	if (pixel & 0x80) {
		x -= ((pixel >> 4) & 7) * (_mirroredFlag ? -1 : 1);
	} else {
		x += (pixel >> 4) * (_mirroredFlag ? -1 : 1);
	}
	if (pixel & 0x08) {
		y -= (pixel & 7);
	} else {
		y += (pixel & 7);
	}
}

void GfxPicture::vectorGetRelCoordsMed(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	if (pixel & 0x80) {
		y -= (pixel & 0x7F);
	} else {
		y += pixel;
	}
	pixel = data[curPos++];
	if (pixel & 0x80) {
		x -= (128 - (pixel & 0x7F)) * (_mirroredFlag ? -1 : 1);
	} else {
		x += pixel * (_mirroredFlag ? -1 : 1);
	}
}

void GfxPicture::vectorGetPatternTexture(const SciSpan<const byte> &data, uint &curPos, int16 pattern_Code, int16 &pattern_Texture) {
	if (pattern_Code & SCI_PATTERN_CODE_USE_TEXTURE) {
		pattern_Texture = (data[curPos++] >> 1) & 0x7f;
	}
}

// WARNING: Do not replace the following code with something else, like generic
// code. This algo really needs to behave exactly as the one from sierra.
void GfxPicture::vectorFloodFill(int16 x, int16 y, byte color, byte priority, byte control) {
	Port *curPort = _ports->getPort();
	Common::Stack<Common::Point> stack;
	Common::Point p, p1;
	byte screenMask = _screen->getDrawingMask(color, priority, control);
	byte matchedMask, matchMask;

	bool isEGA = (_resMan->getViewType() == kViewEga);

	p.x = x + curPort->left;
	p.y = y + curPort->top;

	_screen->vectorAdjustCoordinate(&p.x, &p.y);

	byte searchColor = _screen->vectorGetVisual(p.x, p.y);
	byte searchPriority = _screen->vectorGetPriority(p.x, p.y);
	byte searchControl = _screen->vectorGetControl(p.x, p.y);

	if (isEGA) {
		// In EGA games a pixel in the framebuffer is only 4 bits. We store
		// a full byte per pixel to allow undithering, but when comparing
		// pixels for flood-fill purposes, we should only compare the
		// visible color of a pixel.

		if ((x ^ y) & 1)
			searchColor = (searchColor ^ (searchColor >> 4)) & 0x0F;
		else
			searchColor = searchColor & 0x0F;
	}

	// This logic was taken directly from sierra sci, floodfill will get aborted on various occations
	if (screenMask & GFX_SCREEN_MASK_VISUAL) {
		if ((color == _screen->getColorWhite()) || (searchColor != _screen->getColorWhite()))
			return;
	} else if (screenMask & GFX_SCREEN_MASK_PRIORITY) {
		if ((priority == 0) || (searchPriority != 0))
			return;
	} else if (screenMask & GFX_SCREEN_MASK_CONTROL) {
		if ((control == 0) || (searchControl != 0))
			return;
	}

	// Now remove screens, that already got the right color/priority/control
	if ((screenMask & GFX_SCREEN_MASK_VISUAL) && (searchColor == color))
		screenMask &= ~GFX_SCREEN_MASK_VISUAL;
	if ((screenMask & GFX_SCREEN_MASK_PRIORITY) && (searchPriority == priority))
		screenMask &= ~GFX_SCREEN_MASK_PRIORITY;
	if ((screenMask & GFX_SCREEN_MASK_CONTROL) && (searchControl == control))
		screenMask &= ~GFX_SCREEN_MASK_CONTROL;

	// Exit, if no screens left
	if (!screenMask)
		return;

	if (screenMask & GFX_SCREEN_MASK_VISUAL) {
		matchMask = GFX_SCREEN_MASK_VISUAL;
	} else if (screenMask & GFX_SCREEN_MASK_PRIORITY) {
		matchMask = GFX_SCREEN_MASK_PRIORITY;
	} else {
		matchMask = GFX_SCREEN_MASK_CONTROL;
	}

	// hard borders for filling
	int16 borderLeft = curPort->rect.left + curPort->left;
	int16 borderTop = curPort->rect.top + curPort->top;
	int16 borderRight = curPort->rect.right + curPort->left - 1;
	int16 borderBottom = curPort->rect.bottom + curPort->top - 1;
	int16 curToLeft, curToRight, a_set, b_set;

	// Translate coordinates, if required (needed for Macintosh 480x300)
	_screen->vectorAdjustCoordinate(&borderLeft, &borderTop);
	_screen->vectorAdjustCoordinate(&borderRight, &borderBottom);
	//return;

	stack.push(p);

	while (stack.size()) {
		p = stack.pop();
		if ((matchedMask = _screen->vectorIsFillMatch(p.x, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)) == 0) // already filled
			continue;
		_screen->vectorPutPixel(p.x, p.y, screenMask, color, priority, control);
		curToLeft = p.x;
		curToRight = p.x;
		// moving west and east pointers as long as there is a matching color to fill
		while (curToLeft > borderLeft && (matchedMask = _screen->vectorIsFillMatch(curToLeft - 1, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)))
			_screen->vectorPutPixel(--curToLeft, p.y, screenMask, color, priority, control);
		while (curToRight < borderRight && (matchedMask = _screen->vectorIsFillMatch(curToRight + 1, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)))
			_screen->vectorPutPixel(++curToRight, p.y, screenMask, color, priority, control);
#if 0
		// debug code for floodfill
		_screen->copyToScreen();
		g_system->updateScreen();
		g_system->delayMillis(100);
#endif
		// checking lines above and below for possible flood targets
		a_set = b_set = 0;
		while (curToLeft <= curToRight) {
			if (p.y > borderTop && (matchedMask = _screen->vectorIsFillMatch(curToLeft, p.y - 1, matchMask, searchColor, searchPriority, searchControl, isEGA))) { // one line above
				if (a_set == 0) {
					p1.x = curToLeft;
					p1.y = p.y - 1;
					stack.push(p1);
					a_set = 1;
				}
			} else
				a_set = 0;

			if (p.y < borderBottom && (matchedMask = _screen->vectorIsFillMatch(curToLeft, p.y + 1, matchMask, searchColor, searchPriority, searchControl, isEGA))) { // one line below
				if (b_set == 0) {
					p1.x = curToLeft;
					p1.y = p.y + 1;
					stack.push(p1);
					b_set = 1;
				}
			} else
				b_set = 0;
			curToLeft++;
		}
	}
}

// Bitmap for drawing sierra circles
static const byte vectorPatternCircles[8][30] = {
	{ 0x01 },
	{ 0x72, 0x02 },
	{ 0xCE, 0xF7, 0x7D, 0x0E },
	{ 0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C, 0x00 },
	{ 0x38, 0xF8, 0xF3, 0xDF, 0x7F, 0xFF, 0xFD, 0xF7, 0x9F, 0x3F, 0x38 },
	{ 0x70, 0xC0, 0x1F, 0xFE, 0xE3, 0x3F, 0xFF, 0xF7, 0x7F, 0xFF, 0xE7, 0x3F, 0xFE, 0xC3, 0x1F, 0xF8, 0x00 },
	{ 0xF0, 0x01, 0xFF, 0xE1, 0xFF, 0xF8, 0x3F, 0xFF, 0xDF, 0xFF, 0xF7, 0xFF, 0xFD, 0x7F, 0xFF, 0x9F, 0xFF,
		0xE3, 0xFF, 0xF0, 0x1F, 0xF0, 0x01 },
	{ 0xE0, 0x03, 0xF8, 0x0F, 0xFC, 0x1F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF,
		0x7F, 0xFF, 0x7F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFC, 0x1F, 0xF8, 0x0F, 0xE0, 0x03 }
//  { 0x01 };
//	{ 0x03, 0x03, 0x03 },
//	{ 0x02, 0x07, 0x07, 0x07, 0x02 },
//	{ 0x06, 0x06, 0x0F, 0x0F, 0x0F, 0x06, 0x06 },
//	{ 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x04 },
//	{ 0x0C, 0x1E, 0x1E, 0x1E, 0x3F, 0x3F, 0x3F, 0x1E, 0x1E, 0x1E, 0x0C },
//	{ 0x1C, 0x3E, 0x3E, 0x3E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3E, 0x3E, 0x3E, 0x1C },
//	{ 0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x7E, 0x3C, 0x18 }
};

// TODO: perhaps this is a better way to set the s_patternTextures array below?
//  in that case one would need to adjust bits of secondary table. Bit 256 is ignored by original interpreter
#if 0
static const byte patternTextures[32 * 2] = {
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

// This table is bitwise upwards (from bit0 to bit7), sierras original table went down the bits (bit7 to bit0)
//  this was done to simplify things, so we can just run through the table w/o worrying too much about clipping
static const bool vectorPatternTextures[32 * 8 * 2] = {
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
	// Now the table is actually duplicated, so we won't need to wrap around
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
};

// Bit offsets into pattern_textures
static const byte vectorPatternTextureOffset[128] = {
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	0x60, 0xbd, 0x89, 0x04, 0x0a, 0xf4, 0x7d, 0x6d,
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
	0x06, 0x6f, 0xc6, 0x4a, 0x75, 0xa3, 0x97, 0xe1
};

void GfxPicture::vectorPatternBox(Common::Rect box, Common::Rect clipBox, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	int y, x;

	box.clip(clipBox);
	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			_screen->vectorPutPixel(x, y, flag, color, prio, control);
		}
	}
}

void GfxPicture::vectorPatternTexturedBox(Common::Rect box, Common::Rect clipBox, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const bool *textureData = &vectorPatternTextures[vectorPatternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (*textureData) {
				if (clipBox.contains(x, y)) {
					_screen->vectorPutPixel(x, y, flag, color, prio, control);
				}
			}
			textureData++;
		}
	}
}

void GfxPicture::vectorPatternCircle(Common::Rect box, Common::Rect clipBox, byte size, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	assert(size < ARRAYSIZE(vectorPatternCircles));
	const byte *circleData = vectorPatternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitNo == 8) {
				circleData++;
				bitmap = *circleData;
				bitNo = 0;
			}
			if (bitmap & 1) {
				if (clipBox.contains(x, y)) {
					_screen->vectorPutPixel(x, y, flag, color, prio, control);
				}
			}
			bitNo++;
			bitmap >>= 1;
		}
	}
}

void GfxPicture::vectorPatternTexturedCircle(Common::Rect box, Common::Rect clipBox, byte size, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	assert(size < ARRAYSIZE(vectorPatternCircles));
	const byte *circleData = vectorPatternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	const bool *textureData = &vectorPatternTextures[vectorPatternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitNo == 8) {
				circleData++;
				bitmap = *circleData;
				bitNo = 0;
			}
			if (bitmap & 1) {
				if (*textureData) {
					if (clipBox.contains(x, y)) {
						_screen->vectorPutPixel(x, y, flag, color, prio, control);
					}
				}
				textureData++;
			}
			bitNo++;
			bitmap >>= 1;
		}
	}
}

void GfxPicture::vectorPattern(int16 x, int16 y, byte color, byte priority, byte control, byte code, byte texture) {
	byte size = code & SCI_PATTERN_CODE_PENSIZE;

	// The pattern box is centered on x,y and one pixel wider than high
	Common::Rect box(x - size, y - size, x + size + 2, y + size + 1);

	// Move the pattern box if it goes off of the left or top of the picture
	if (box.left < 0) {
		box.moveTo(0, box.top);
	}
	if (box.top < 0) {
		box.moveTo(box.left, 0);
	}

	// Adjust the pattern box to the current port
	_ports->offsetRect(box);

	// Move the pattern box if it goes off of the right or bottom of the screen.
	// Although, if it goes off the right edge, leave the last column
	// beyond the screen edge even though it won't get drawn.
	// We also can't just clip the box to the screen here, because the
	// texture and circle data can only be properly consumed by evaluating
	// every pixel during drawing, even the pixels that are then skipped
	// for being out of bounds. (Example: SQ3 picture 2)
	if (box.right >= _screen->getScriptWidth()) {
		box.moveTo(_screen->getScriptWidth() - box.width() + 1, box.top);
	}
	if (box.bottom >= _screen->getScriptHeight()) {
		box.moveTo(box.left, _screen->getScriptHeight() - box.height());
	}
	_screen->vectorAdjustCoordinate(&box.left, &box.top);
	_screen->vectorAdjustCoordinate(&box.right, &box.bottom);

	// Create a box for the pattern drawing functions to clip to
	Common::Rect clipBox(0, 0, _screen->getScriptWidth(), _screen->getScriptHeight());
	_screen->vectorAdjustCoordinate(&clipBox.right, &clipBox.bottom);

	if (code & SCI_PATTERN_CODE_RECTANGLE) {
		// Rectangle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			vectorPatternTexturedBox(box, clipBox, color, priority, control, texture);
		} else {
			vectorPatternBox(box, clipBox, color, priority, control);
		}

	} else {
		// Circle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			vectorPatternTexturedCircle(box, clipBox, size, color, priority, control, texture);
		} else {
			vectorPatternCircle(box, clipBox, size, color, priority, control);
		}
	}
}

} // End of namespace Sci
