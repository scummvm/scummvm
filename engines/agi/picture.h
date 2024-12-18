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

#ifndef AGI_PICTURE_H
#define AGI_PICTURE_H

namespace Agi {

#define _DEFAULT_WIDTH      160
#define _DEFAULT_HEIGHT     168

/**
 * AGI picture resource.
 */
struct AgiPicture {
	uint32 flen;            /**< size of raw data */
	uint8 *rdata;           /**< raw vector image data */

	void reset() {
		flen = 0;
		rdata = nullptr;
	}

	AgiPicture() { reset(); }
};

enum AgiPictureVersion {
	AGIPIC_C64,     // Winnie (Apple II, C64, CoCo)
	AGIPIC_V1,      // Currently unused
	AGIPIC_V15,     // Troll (DOS)
	AGIPIC_PREAGI,  // Winnie (DOS, Amiga), Mickey (DOS)
	AGIPIC_V2       // AGIv2, AGIv3
};

enum AgiPictureFlags {
	kPicFNone      = (1 << 0),
	kPicFf3Stop    = (1 << 1), // Troll, certain pictures
	kPicFTrollMode = (1 << 2)  // Troll, drawing the Troll
};

class AgiBase;
class GfxMgr;

class PictureMgr {
	AgiBase *_vm;
	GfxMgr *_gfx;

public:
	PictureMgr(AgiBase *agi, GfxMgr *gfx);

	int16 getResourceNr() const { return _resourceNr; };

private:
	void putVirtPixel(int x, int y);
	void xCorner(bool skipOtherCoords = false);
	void yCorner(bool skipOtherCoords = false);
	void plotPattern(int x, int y);
	void plotBrush();
	void plotPattern_PreAGI(byte x, byte y);
	void plotBrush_PreAGI();

	byte getNextByte();
	bool getNextParamByte(byte &b);
	byte getNextNibble();

	bool getNextXCoordinate(byte &x);
	bool getNextYCoordinate(byte &y);
	bool getNextCoordinates(byte &x, byte &y);

public:
	void decodePicture(int16 resourceNr, bool clearScreen, bool agi256 = false, int16 width = _DEFAULT_WIDTH, int16 height = _DEFAULT_HEIGHT);
	void decodePictureFromBuffer(byte *data, uint32 length, bool clearScreen, int16 width = _DEFAULT_WIDTH, int16 height = _DEFAULT_HEIGHT);

private:
	void drawPicture();
	void drawPictureC64();
	void drawPictureV1();
	void drawPictureV15();
	void drawPicturePreAGI();
	void drawPictureV2();
	void drawPictureAGI256();

	void draw_SetColor();
	void draw_SetPriority();
	void draw_SetNibbleColor();
	void draw_SetNibblePriority();

	void draw_Line(int16 x1, int16 y1, int16 x2, int16 y2);
	void draw_LineShort();
	void draw_LineAbsolute();

	bool draw_FillCheck(int16 x, int16 y);
	void draw_Fill(int16 x, int16 y);
	void draw_Fill();

public:
	void showPicture(int16 x = 0, int16 y = 0, int16 width = _DEFAULT_WIDTH, int16 height = _DEFAULT_HEIGHT);
	void showPictureWithTransition();

	void setPictureVersion(AgiPictureVersion version);

	void setPictureFlags(int flags) { _flags = flags; }

	void setOffset(int offX, int offY) {
		_xOffset = offX;
		_yOffset = offY;
	}

	void setMaxStep(int maxStep) { _maxStep = maxStep; }
	int getMaxStep() const { return _maxStep; }

private:
	int16  _resourceNr;
	uint8 *_data;
	uint32 _dataSize;
	uint32 _dataOffset;
	bool   _dataOffsetNibble;

	uint8 _patCode;
	uint8 _patNum;
	uint8 _priOn;
	uint8 _scrOn;
	uint8 _scrColor;
	uint8 _priColor;

	uint8 _minCommand;

	AgiPictureVersion _pictureVersion;
	int16 _width;
	int16 _height;
	int16 _xOffset;
	int16 _yOffset;

	int _flags;
	int _maxStep; // Max opcodes to draw, zero for all. Used by preagi (Mickey)
};

} // End of namespace Agi

#endif /* AGI_PICTURE_H */
