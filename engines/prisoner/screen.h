#ifndef PRISONER_SCREEN_H
#define PRISONER_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/util.h"

#include "graphics/surface.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"

namespace Prisoner {

class AnimationCelReader {
public:
	AnimationCelReader(AnimationCel *cel) : _state(0), _cel(cel), _frameData(_cel->data), _width(_cel->width), _height(_cel->height) {
	}
	~AnimationCelReader() {
	}
	byte readPixel() {
		do {
			switch (_state) {
			case 0:
				_chunks = *_frameData++;
				if (_chunks == 0) {
					_skip = *_frameData++;
					_state = 5;
					break;
				}
			case 1:
				_skip = *_frameData++;
				_state = 2;
			case 2:
				if (_skip > 0) {
					_skip--;
					return 0;
				} else {
					_count = _frameData[0] * 4 + _frameData[1];
					_frameData += 2;
					_state = 3;
				}
			case 3:
				if (_count > 0) {
					_count--;
					return *_frameData++;
				} else
					_state = 4;
			case 4:
				_chunks--;
				if (_chunks > 0) {
					_state = 1;
					break;
				} else {
					_skip = *_frameData++;
					_state = 5;
				}
			case 5:
				if (_skip > 0) {
					_skip--;
					return 0;
				} else
					_state = 0;
			}
		} while (1);
	}
	int16 getWidth() const { return _width; }
	int16 getHeight() const { return _height; }
protected:
	AnimationCel *_cel;
	int16 _width, _height;
	byte *_frameData;
	int _state;
	int _chunks, _skip;
	int _count;
};

template<typename T>
class AnimationCelScalingFilter {
public:
	AnimationCelScalingFilter(T *reader, int16 scale) {
		_reader = reader;
		_scale = scale;
		_width = reader->getWidth();
		_scaledWidth = _width * _scale / 100;
		_widthIncr = _width / _scaledWidth;
		_widthMod = _width - _widthIncr * _scaledWidth;
		_height = reader->getHeight();
		_scaledHeight = _height * _scale / 100;
		_heightIncr = _height / _scaledHeight;
		_heightMod = _height - _heightIncr * _scaledHeight;
		_currHeight = _scaledHeight;
		_heightErr = _scaledHeight;
		_state = 0;
	}
	~AnimationCelScalingFilter() {
	}
	byte readPixel() {
    	while (1) {
			switch (_state) {
			case 0:
				if (_currHeight == 0)
					return 0;
				_currWidth = 0;
				_widthErr = _scaledWidth;
				_widthAdjust = _width;
				_state = 1;
			case 1:
				if (_currWidth < _scaledWidth) {
					byte pixel = _reader->readPixel();
					_currWidth++;
					_widthAdjust--;
					for (uint16 w = 1; w < _widthIncr; w++) {
						_reader->readPixel();
						_widthAdjust--;
					}
					if (_widthErr <= 0) {
						_reader->readPixel();
						_widthAdjust--;
						_widthErr += _scaledWidth;
					}
					_widthErr -= _widthMod;
					return pixel;
				} else {
					_currHeight--;
					if (_currHeight > 0) {
						int lineSkip = _heightIncr;
						if (_heightErr <= 0) {
							lineSkip++;
							_heightErr += _scaledHeight;
						}
						_heightErr -= _heightMod;
						lineSkip = _width * (lineSkip - 1) + _widthAdjust;
						while (lineSkip--)
							_reader->readPixel();
					}
					_state = 0;
				}
			}
		}
	}
	int16 getWidth() const { return _scaledWidth; }
	int16 getHeight() const { return _scaledHeight; }
protected:
	T *_reader;
	int16 _scale;
	int16 _width, _height;
	int _state;
	uint16 _scaledWidth, _widthIncr, _widthMod;
	uint16 _scaledHeight, _heightIncr, _heightMod;
	int _currWidth, _currHeight, _widthAdjust;
	int _widthErr, _heightErr;
};

template<typename T>
class AnimationCelDrawer {
public:
	AnimationCelDrawer() {}
	~AnimationCelDrawer() {}
	void draw(T *reader, Graphics::Surface *destSurface, int16 x, int16 y, int16 clipX1, int16 clipY1, int16 clipX2, int16 clipY2, bool flip) {

		// TODO: Move clipping code elsewhere to reduce templated code size a little
		byte *dest;
		int16 width = reader->getWidth(), height = reader->getHeight();
		int w, skipX = 0, skipY = 0, skipL, skipR, pitchIncr, destIncr;

		// Clip the sprite
		y -= height;
		y++;

		if (x + width - 1 > clipX2)
			width = clipX2 - x + 1;

		if (y + height - 1 > clipY2)
			height = clipY2 - y + 1;

		if (y < clipY1) {
			if (y + height - 1 < clipY1)
				return;
			height -= clipY1 - y;
			skipY = clipY1 - y;
			y = clipY1;
		}

		if (x < clipX1) {
			if (x + width - 1 < clipX1)
				return;
			skipX = clipX1 - x;
			x = clipX1;
		}

		if (x > clipX2 || y > clipY2)
			return;

		// Skip pixel rows clipped from the top
		while (skipY--) {
			w = reader->getWidth();
			while (w--)
				reader->readPixel();
		}

		dest = (byte*)destSurface->getBasePtr(x, y);

		if (flip) {
			skipL = reader->getWidth() - skipX - (width - skipX);
			skipR = skipX;
			pitchIncr = destSurface->pitch + (width - skipX);
			destIncr = -1;
			dest += width - skipX;
		} else {
			skipL = skipX;
			skipR = reader->getWidth() - skipX - (width - skipX);
			pitchIncr = destSurface->pitch - (width - skipX);
			destIncr = +1;
		}

		while (height--) {
			w = skipL;
			while (w--)
				reader->readPixel();
			w = width - skipX;
			while (w--) {
				byte pixel = reader->readPixel();
				if (pixel != 0)
					*dest = pixel;
				dest += destIncr;
			}
			w = skipR;
			while (w--)
				reader->readPixel();
			dest += pitchIncr;
		}

	}
};

class Screen {
public:
	Screen(PrisonerEngine *vm);
	~Screen();

	void update();

	void putPixel(int x, int y, byte color);
	void drawLine(int x1, int y1, int x2, int y2, byte color);
	void hLine(int x, int y, int x2, byte color);
	void vLine(int x, int y, int y2, byte color);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2, byte color);

	void setPartialPalette(byte *palette, int start, int count);
	void setFullPalette(byte *palette);

	void clear();

	Graphics::Surface *getScreen() const {
		return _workScreen;
	}

	void drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, uint16 flags = 0);
	void drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, uint16 parentFlags = 0);
	void drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, uint16 parentFlags = 0);

	int16 getTextWidth(FontResource *fontResource, const Common::String &text);
	void drawChar(const FontChar *fontChar, const byte *fontCharData, int16 x, int16 y);
	void drawText(FontResource *fontResource, const Common::String &text, int16 x, int16 y);

	void setClipRect(int clipX1, int clipY1, int clipX2, int clipY2);

	void copyRectFrom(Graphics::Surface *source, int sourceX, int sourceY, int destX, int destY, int width, int height);

	void setFontColorTable(byte index, byte color);
	void initPaletteTransTable(byte colorIncr);
	void buildPaletteTransTable(const byte *sourcePalette, byte color);
	void drawTransparentRect(int16 x1, int16 y1, int16 x2, int16 y2);

//protected:

	PrisonerEngine *_vm;

	Graphics::Surface *_workScreen;

	int _clipX1, _clipY1, _clipX2, _clipY2;
	byte _fontColorTable[256];

	/* Palette transparency */
	byte _pttTable1[256], _pttTable2[256];
	byte _pttColorIncr;
	byte _paletteTransTable[256];

};

}

#endif
