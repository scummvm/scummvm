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

#include "groovie/cursor.h"
#include "groovie/groovie.h"

#include "common/debug.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"

namespace Groovie {

// Cursor Manager

GrvCursorMan::GrvCursorMan(OSystem *system) :
	_syst(system), _lastTime(0), _current(255), _cursor(nullptr), _lastFrame(0) {
}

GrvCursorMan::~GrvCursorMan() {
	// Delete the cursors
	for (uint cursor = 0; cursor < _cursors.size(); cursor++) {
		delete _cursors[cursor];
	}

	CursorMan.popAllCursors();
}

void GrvCursorMan::show(bool visible) {
	CursorMan.showMouse(visible);
}

uint16 GrvCursorMan::getStyle() {
	return _current;
}

void GrvCursorMan::setStyle(uint16 newStyle) {
	// Reset the animation
	_lastFrame = 254;
	_lastTime = 1;

	// Save the new cursor
	_current = newStyle;
	_cursor = _cursors[newStyle & 0xFF];

	// Show the first frame
	_cursor->enable();
	animate();
}

void GrvCursorMan::animate() {
	if (_lastTime) {
		int newTime = _syst->getMillis();
		if (newTime - _lastTime >= 66) {
			_lastFrame++;
			_lastFrame %= _cursor->getFrames();
			_cursor->showFrame(_lastFrame);
			_lastTime = _syst->getMillis();
		}
	}
}


// t7g Cursor

class Cursor_t7g : public Cursor {
public:
	Cursor_t7g(uint8 *img, uint8 *pal);

	void enable() override;
	void showFrame(uint16 frame) override;

private:
	byte *_img;
	byte *_pal;
};

Cursor_t7g::Cursor_t7g(uint8 *img, uint8 *pal) :
	_pal(pal) {

	_width = img[0];
	_height = img[1];
	_numFrames = img[2];
	uint8 elinor1 = img[3];
	uint8 elinor2 = img[4];

	_img = img + 5;

	debugC(1, kDebugCursor, "Groovie::Cursor: width: %d, height: %d, frames:%d", _width, _height, _numFrames);
	debugC(1, kDebugCursor, "Groovie::Cursor: elinor: 0x%02X (%d), 0x%02X (%d)", elinor1, elinor1, elinor2, elinor2);
}

void Cursor_t7g::enable() {
	// Apply the palette
	CursorMan.replaceCursorPalette(_pal, 0, 32);
}

void Cursor_t7g::showFrame(uint16 frame) {
	// Set the mouse cursor
	int offset = _width * _height * frame;
	CursorMan.replaceCursor((const byte *)_img + offset, _width, _height, _width >> 1, _height >> 1, 0);
}


// t7g Cursor Manager

#define NUM_IMGS 9
static const uint16 cursorDataOffsets[NUM_IMGS] = {
0x0000, 0x182f, 0x3b6d, 0x50cc, 0x6e79, 0x825d, 0x96d7, 0xa455, 0xa776
};

#define NUM_PALS 7
//Pals: 0xb794, 0xb7f4, 0xb854, 0xb8b4, 0xb914, 0xb974, 0xb9d4

#define NUM_STYLES 11
// pyramid is cursor 8, eyes are 9 & 10
const uint GrvCursorMan_t7g::_cursorImg[NUM_STYLES] = {3, 5, 4, 3, 1, 0, 2, 6, 7, 8, 8};
const uint GrvCursorMan_t7g::_cursorPal[NUM_STYLES] = {0, 0, 0, 0, 2, 0, 1, 3, 5, 4, 6};

GrvCursorMan_t7g::GrvCursorMan_t7g(OSystem *system, Common::MacResManager *macResFork) :
	GrvCursorMan(system) {

	Common::SeekableReadStream *robgjd = nullptr;

	if (macResFork) {
		// Open the cursors file from the resource fork
		robgjd = macResFork->getResource("rob.gjd");
	} else {
		// Open the cursors file
		robgjd = SearchMan.createReadStreamForMember("rob.gjd");
	}

	if (!robgjd)
		error("Groovie::Cursor: Couldn't open rob.gjd");

	// Load the images
	for (uint imgnum = 0; imgnum < NUM_IMGS; imgnum++) {
		robgjd->seek(cursorDataOffsets[imgnum]);
		_images.push_back(loadImage(*robgjd));
	}

	// Load the palettes
	robgjd->seek(-0x60 * NUM_PALS, SEEK_END);
	for (uint palnum = 0; palnum < NUM_PALS; palnum++) {
		_palettes.push_back(loadPalette(*robgjd));
	}

	// Build the cursors
	for (uint cursor = 0; cursor < NUM_STYLES; cursor++) {
		Cursor *s = new Cursor_t7g(_images[_cursorImg[cursor]], _palettes[_cursorPal[cursor]]);
		_cursors.push_back(s);
	}

	delete robgjd;
}

GrvCursorMan_t7g::~GrvCursorMan_t7g() {
	// Delete the images
	for (uint img = 0; img < _images.size(); img++) {
		delete[] _images[img];
	}

	// Delete the palettes
	for (uint pal = 0; pal < _palettes.size(); pal++) {
		delete[] _palettes[pal];
	}
}

byte *GrvCursorMan_t7g::loadImage(Common::SeekableReadStream &file) {
	uint16 decompbytes = 0, offset, i, length;
	uint8 flagbyte, lengthmask = 0x0F, offsetlen, var_8;
	byte *cursorStorage = new byte[65536];
	uint8 *runningcursor = cursorStorage;

	bool finished = false;
	while (!(finished || file.eos())) {
		flagbyte = file.readByte();
		for (i = 1; i <= 8; i++) {
			if (!file.eos()) {
				if (flagbyte & 1) {
					*(runningcursor++) = file.readByte();
					decompbytes++;
				} else {
					var_8 = file.readByte();
					offsetlen = file.readByte();
					if (var_8 == 0 && offsetlen == 0) {
						finished = true;
						break;
					}
					length = (offsetlen & lengthmask) + 3;
					offsetlen >>= 4;
					offset = (offsetlen << 8) + var_8;
					decompbytes += length;

					for (; length > 0; length--, runningcursor++) {
						*(runningcursor) = *(runningcursor - offset);
					}
				}
				flagbyte = flagbyte >> 1;
			}
		}
	}
	debug(9, "GrvCursorMan_t7g::loadImage(): decompressed %d bytes", decompbytes);

	return cursorStorage;
}

byte *GrvCursorMan_t7g::loadPalette(Common::SeekableReadStream &file) {
	byte *palette = new byte[3 * 32];
	file.read(palette, 3 * 32);
	return palette;
}


// v2 Cursor

class Cursor_v2 : public Cursor {
public:
	Cursor_v2(Common::File &file);
	~Cursor_v2() override;

	void enable() override;
	void showFrame(uint16 frame) override;
	void blendCursor(uint32 *dst, int frame, int w, int h);
	static void show2Cursors(Cursor_v2 *c1, uint16 frame1, Cursor_v2 *c2, uint16 frame2);

private:
	// Currently locked to 16bit
	byte *_img;

	Graphics::PixelFormat _format;

	void decodeFrame(byte *pal, byte *data, byte *dest, uint32 size);
};

Cursor_v2::Cursor_v2(Common::File &file) {
	byte *pal = new byte[0x20 * 3];

	_format = g_system->getScreenFormat();

	_numFrames = file.readUint16LE();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_hotspotX = file.readUint16LE();
	_hotspotY = file.readUint16LE();

	_img = new byte[_width * _height * _numFrames * 4];

	debugC(1, kDebugCursor, "Groovie::Cursor: width: %d, height: %d, frames:%d, hotspot: %d, %d", _width, _height, _numFrames, _hotspotX, _hotspotY);

	uint16 tmp16;
	int loop2count = file.readUint16LE();
	debugC(5, kDebugCursor, "loop2count?: %d", loop2count);
	for (int l = 0; l < loop2count; l++) {
		tmp16 = file.readUint16LE();
		debugC(5, kDebugCursor, "loop2a: %d", tmp16);	// Index frame can merge to/from?
		tmp16 = file.readUint16LE();
		debugC(5, kDebugCursor, "loop2b: %d", tmp16);	// Number of frames?
	}

	file.read(pal, 0x20 * 3);

	for (int f = 0; f < _numFrames; f++) {
		uint32 tmp32 = file.readUint32LE();
		debugC(5, kDebugCursor, "loop3: %d", tmp32);

		byte *data = new byte[tmp32];
		file.read(data, tmp32);
		decodeFrame(pal, data, _img + (f * _width * _height * 4), tmp32);

		delete[] data;
	}

	delete[] pal;
}

Cursor_v2::~Cursor_v2() {
	delete[] _img;
}

void Cursor_v2::decodeFrame(byte *pal, byte *data, byte *dest, uint32 size) {
	// Scratch memory
	byte *tmp = new byte[_width * _height * 4]();
	byte *ptr = tmp;

	byte ctrA = 0, ctrB = 0;

	byte alpha = 0, palIdx = 0;

	byte r, g, b;

	const byte alphaDecoded[8] = {0, 36, 73, 109, 146, 182, 219, 255};	// Calculated by: alpha = ((int)(*data & 0xE0) * 255) / 224;

	// Start frame decoding
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			if (!size) {
				debugC(1, kDebugCursor, "Cursor_v2::decodeFrame(): Frame underflow");
				delete[] tmp;
				return;
			}

			// If both counters are empty
			if (ctrA == 0 && ctrB == 0) {
				if (*data & 0x80) {
					ctrA = (*data++ & 0x7F) + 1;
					size--;
				} else {
					ctrB = *data++ + 1;
					alpha = alphaDecoded[(*data & 0xE0) >> 5];
					palIdx = *data++ & 0x1F;

					size -= 2;
				}
			}

			if (ctrA) {
				// Block type A - chunk of non-continuous pixels
				palIdx = *data & 0x1F;
				alpha = alphaDecoded[(*data++ & 0xE0) >> 5];
				size--;

				r = *(pal + palIdx);
				g = *(pal + palIdx + 0x20);
				b = *(pal + palIdx + 0x40);

				ctrA--;
			} else {
				// Block type B - chunk of continuous pixels
				r = *(pal + palIdx);
				g = *(pal + palIdx + 0x20);
				b = *(pal + palIdx + 0x40);

				ctrB--;
			}

			// Decode pixel
			if (alpha) {
				ptr[0] = alpha;
				ptr[1] = r;
				ptr[2] = g;
				ptr[3] = b;
			}
			ptr += 4;
		}
	}

	// Convert to screen format
	// NOTE: Currently locked to 32bpp
	ptr = tmp;
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			*(uint32 *)dest = _format.ARGBToColor(ptr[0], ptr[1], ptr[2], ptr[3]);
			dest += 4;
			ptr += 4;
		}
	}

	delete[] tmp;
}

void Cursor_v2::enable() {
}

void Cursor_v2::showFrame(uint16 frame) {
	int offset = _width * _height * frame * 4;
	// SDL uses keycolor even though we're using ABGR8888, so just set it to a pink color that isn't used
	uint32 keycolor = _format.ARGBToColor(0, 255, 128, 255);
	CursorMan.replaceCursor((const byte *)(_img + offset), _width, _height, _hotspotX, _hotspotY, keycolor, false, &_format);
}

void blendCursorPixel(uint32 &d, uint32 &s) {
#ifdef SCUMM_LITTLE_ENDIAN
	static const int kAIndex = 0;
	static const int kBIndex = 1;
	static const int kGIndex = 2;
	static const int kRIndex = 3;

#else
	static const int kAIndex = 3;
	static const int kBIndex = 2;
	static const int kGIndex = 1;
	static const int kRIndex = 0;
#endif

	byte *dst = (byte *)&d;
	byte *src = (byte *)&s;

	if (src[kAIndex] == 255) {
		d = s;
	} else if (src[kAIndex] > 0) {
		dst[kAIndex] = MAX(src[kAIndex], dst[kAIndex]);
		dst[kRIndex] = ((src[kRIndex] * src[kAIndex]) + dst[kRIndex] * (255 - src[kAIndex])) >> 8;
		dst[kGIndex] = ((src[kGIndex] * src[kAIndex]) + dst[kGIndex] * (255 - src[kAIndex])) >> 8;
		dst[kBIndex] = ((src[kBIndex] * src[kAIndex]) + dst[kBIndex] * (255 - src[kAIndex])) >> 8;
	}
	// In case of alpha == 0 just do nothing
}

void Cursor_v2::blendCursor(uint32 *dst, int frame, int w, int h) {
	uint32 *src = (uint32 *)_img;
	src += _width * _height * frame;

	int offX = (w - _width) / 2;
	int offY = (h - _height) / 2;

	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			blendCursorPixel(dst[x + offX + (y + offY) * w], src[x + y * _width]);
		}
	}
}

void Cursor_v2::show2Cursors(Cursor_v2 *c1, uint16 frame1, Cursor_v2 *c2, uint16 frame2) {
	int width = MAX(c1->_width, c2->_width);
	int height = MAX(c1->_height, c2->_height);
	uint32 *img = new uint32[width * height]();

	c2->blendCursor(img, frame2, width, height);
	c1->blendCursor(img, frame1, width, height);

	// SDL uses keycolor even though we're using ABGR8888, so just set it to a pink color that isn't used
	Graphics::PixelFormat format = g_system->getScreenFormat();
	uint32 keycolor = format.ARGBToColor(0, 255, 128, 255);

	// replaceCursor copies the buffer, so we're ok to delete it
	CursorMan.replaceCursor((const byte *)img, width, height, c1->_hotspotX, c1->_hotspotY, keycolor, false, &c1->_format);
	delete[] img;
}


// v2 Cursor Manager

GrvCursorMan_v2::GrvCursorMan_v2(OSystem *system) :
	GrvCursorMan(system), _cursor2(nullptr), _lastFrame2(0) {

	// Open the icons file
	Common::File iconsFile;
	if (!iconsFile.open("icons.ph") && !iconsFile.open("icons.bin"))
		error("Groovie::Cursor: Couldn't open icons.ph or icons.bin");

	// Verify the signature
	uint32 tmp32 = iconsFile.readUint32BE();
	uint16 tmp16 = iconsFile.readUint16LE();
	if (tmp32 != MKTAG('i','c','o','n') || tmp16 != 1)
		error("Groovie::Cursor: %s signature failed: %s %d", iconsFile.getName(), tag2str(tmp32), tmp16);


	// Read the number of icons
	uint16 nicons = iconsFile.readUint16LE();

	// Read the icons
	for (int i = 0; i < nicons; i++) {
		Cursor *s = new Cursor_v2(iconsFile);
		_cursors.push_back(s);
	}

	iconsFile.close();
}

GrvCursorMan_v2::~GrvCursorMan_v2() {
}


void GrvCursorMan_v2::animate() {
	if (_lastTime) {
		int newTime = _syst->getMillis();
		if (newTime - _lastTime >= 66) {
			_lastFrame++;
			_lastFrame %= _cursor->getFrames();
			if (_cursor2) {
				_lastFrame2++;
				_lastFrame2 %= _cursor2->getFrames();
				Cursor_v2::show2Cursors((Cursor_v2 *)_cursor, _lastFrame, (Cursor_v2 *)_cursor2, _lastFrame2);
			} else {
				_cursor->showFrame(_lastFrame);
			}
			_lastTime = _syst->getMillis();
		}
	}
}


void GrvCursorMan_v2::setStyle(uint16 newStyle) {
	// HACK: Cursor 4 is actually cursor 3, but with some changes to alpha blending
	// (which is currently not handled)
	uint8 newStyleLow = newStyle & 0xFF;
	GrvCursorMan::setStyle(newStyleLow == 4 ? 3 : newStyle);

	if (newStyle & 0x8000) {
		_cursor2 = _cursors.back();
		_lastFrame2 = 254;
	} else {
		_cursor2 = nullptr;
	}

	// fix _current back to cursor 4 so that getStyle returns the proper number
	if (newStyleLow == 4)
		_current++;
}

} // End of Groovie namespace
