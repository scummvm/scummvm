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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sci/resource.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/text32.h"

namespace Sci {
#pragma mark CelScaler

CelScaler *CelObj::_scaler = nullptr;

void CelScaler::activateScaleTables(const Ratio &scaleX, const Ratio &scaleY) {
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	for (int i = 0; i < ARRAYSIZE(_scaleTables); ++i) {
		if (_scaleTables[i].scaleX == scaleX && _scaleTables[i].scaleY == scaleY) {
			_activeIndex = i;
			return;
		}
	}

	int i = 1 - _activeIndex;
	_activeIndex = i;
	CelScalerTable &table = _scaleTables[i];

	if (table.scaleX != scaleX) {
		assert(screenWidth <= ARRAYSIZE(table.valuesX));
		buildLookupTable(table.valuesX, scaleX, screenWidth);
		table.scaleX = scaleX;
	}

	if (table.scaleY != scaleY) {
		assert(screenHeight <= ARRAYSIZE(table.valuesY));
		buildLookupTable(table.valuesY, scaleY, screenHeight);
		table.scaleY = scaleY;
	}
}

void CelScaler::buildLookupTable(int *table, const Ratio &ratio, const int size) {
	int value = 0;
	int remainder = 0;
	int num = ratio.getNumerator();
	for (int i = 0; i < size; ++i) {
		*table++ = value;
		remainder += ratio.getDenominator();
		if (remainder >= num) {
			value += remainder / num;
			remainder %= num;
		}
	}
}

const CelScalerTable *CelScaler::getScalerTable(const Ratio &scaleX, const Ratio &scaleY) {
	activateScaleTables(scaleX, scaleY);
	return &_scaleTables[_activeIndex];
}

#pragma mark -
#pragma mark CelObj
bool CelObj::_drawBlackLines = false;

void CelObj::init() {
	CelObj::deinit();
	_drawBlackLines = false;
	_nextCacheId = 1;
	_scaler = new CelScaler();
	_cache = new CelCache;
	_cache->resize(100);
}

void CelObj::deinit() {
	delete _scaler;
	_scaler = nullptr;
	if (_cache != nullptr) {
		for (CelCache::iterator it = _cache->begin(); it != _cache->end(); ++it) {
			delete it->celObj;
		}
	}
	delete _cache;
	_cache = nullptr;
}

#pragma mark -
#pragma mark CelObj - Scalers

template<bool FLIP, typename READER>
struct SCALER_NoScale {
#ifndef NDEBUG
	const byte *_rowEdge;
#endif
	const byte *_row;
	READER _reader;
	const int16 _lastIndex;
	const int16 _sourceX;
	const int16 _sourceY;

	SCALER_NoScale(const CelObj &celObj, const int16 maxWidth, const Common::Point &scaledPosition) :
	_row(nullptr),
	_reader(celObj, FLIP ? celObj._width : maxWidth),
	_lastIndex(celObj._width - 1),
	_sourceX(scaledPosition.x),
	_sourceY(scaledPosition.y) {}

	inline void setTarget(const int16 x, const int16 y) {
		_row = _reader.getRow(y - _sourceY);

		if (FLIP) {
#ifndef NDEBUG
			_rowEdge = _row - 1;
#endif
			_row += _lastIndex - (x - _sourceX);
			assert(_row > _rowEdge);
		} else {
#ifndef NDEBUG
			_rowEdge = _row + _lastIndex + 1;
#endif
			_row += x - _sourceX;
			assert(_row < _rowEdge);
		}
	}

	inline byte read() {
		assert(_row != _rowEdge);

		if (FLIP) {
			return *_row--;
		} else {
			return *_row++;
		}
	}
};

template<bool FLIP, typename READER>
struct SCALER_Scale {
#ifndef NDEBUG
	int16 _maxX;
#endif
	const byte *_row;
	READER _reader;
	int16 _x;
	static int16 _valuesX[1024];
	static int16 _valuesY[1024];

	SCALER_Scale(const CelObj &celObj, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio scaleX, const Ratio scaleY) :
	_row(nullptr),
#ifndef NDEBUG
	_maxX(targetRect.right - 1),
#endif
	// The maximum width of the scaled object may not be as
	// wide as the source data it requires if downscaling,
	// so just always make the reader decompress an entire
	// line of source data when scaling
	_reader(celObj, celObj._width) {
		// In order for scaling ratios to apply equally across objects that
		// start at different positions on the screen (like the cels of a
		// picture), the pixels that are read from the source bitmap must all
		// use the same pattern of division. In other words, cels must follow
		// a global scaling pattern as if they were always drawn starting at an
		// even multiple of the scaling ratio, even if they are not.
		//
		// To get the correct source pixel when reading out through the scaler,
		// the engine creates a lookup table for each axis that translates
		// directly from target positions to the indexes of source pixels using
		// the global cadence for the given scaling ratio.
		//
		// Note, however, that not all games use the global scaling mode.
		//
		// SQ6 definitely uses the global scaling mode (an easy visual
		// comparison is to leave Implants N' Stuff and then look at Roger);
		// Torin definitely does not (scaling subtitle backgrounds will cause it
		// to attempt a read out of bounds and crash). They are both SCI
		// "2.1mid" games, so currently the common denominator looks to be that
		// games which use global scaling are the ones that use low-resolution
		// script coordinates too.

		const CelScalerTable *table = CelObj::_scaler->getScalerTable(scaleX, scaleY);

		if (g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth == kLowResX) {
			const int16 unscaledX = (scaledPosition.x / scaleX).toInt();
			if (FLIP) {
				int lastIndex = celObj._width - 1;
				for (int16 x = targetRect.left; x < targetRect.right; ++x) {
					_valuesX[x] = lastIndex - (table->valuesX[x] - unscaledX);
				}
			} else {
				for (int16 x = targetRect.left; x < targetRect.right; ++x) {
					_valuesX[x] = table->valuesX[x] - unscaledX;
				}
			}

			const int16 unscaledY = (scaledPosition.y / scaleY).toInt();
			for (int16 y = targetRect.top; y < targetRect.bottom; ++y) {
				_valuesY[y] = table->valuesY[y] - unscaledY;
			}
		} else {
			if (FLIP) {
				int lastIndex = celObj._width - 1;
				for (int16 x = 0; x < targetRect.width(); ++x) {
					_valuesX[targetRect.left + x] = lastIndex - table->valuesX[x];
				}
			} else {
				for (int16 x = 0; x < targetRect.width(); ++x) {
					_valuesX[targetRect.left + x] = table->valuesX[x];
				}
			}

			for (int16 y = 0; y < targetRect.height(); ++y) {
				_valuesY[targetRect.top + y] = table->valuesY[y];
			}
		}
	}

	inline void setTarget(const int16 x, const int16 y) {
		_row = _reader.getRow(_valuesY[y]);
		_x = x;
		assert(_x >= 0 && _x <= _maxX);
	}

	inline byte read() {
		assert(_x >= 0 && _x <= _maxX);
		return _row[_valuesX[_x++]];
	}
};

template<bool FLIP, typename READER>
int16 SCALER_Scale<FLIP, READER>::_valuesX[1024];
template<bool FLIP, typename READER>
int16 SCALER_Scale<FLIP, READER>::_valuesY[1024];

#pragma mark -
#pragma mark CelObj - Resource readers

struct READER_Uncompressed {
private:
#ifndef NDEBUG
	const int16 _sourceHeight;
#endif
	byte *_pixels;
	const int16 _sourceWidth;

public:
	READER_Uncompressed(const CelObj &celObj, const int16) :
#ifndef NDEBUG
	_sourceHeight(celObj._height),
#endif
	_sourceWidth(celObj._width) {
		byte *resource = celObj.getResPointer();
		_pixels = resource + READ_SCI11ENDIAN_UINT32(resource + celObj._celHeaderOffset + 24);
	}

	inline const byte *getRow(const int16 y) const {
		assert(y >= 0 && y < _sourceHeight);
		return _pixels + y * _sourceWidth;
	}
};

struct READER_Compressed {
private:
	byte *_resource;
	byte _buffer[1024];
	uint32 _controlOffset;
	uint32 _dataOffset;
	uint32 _uncompressedDataOffset;
	int16 _y;
	const int16 _sourceHeight;
	const uint8 _transparentColor;
	const int16 _maxWidth;

public:
	READER_Compressed(const CelObj &celObj, const int16 maxWidth) :
	_resource(celObj.getResPointer()),
	_y(-1),
	_sourceHeight(celObj._height),
	_transparentColor(celObj._transparentColor),
	_maxWidth(maxWidth) {
		assert(maxWidth <= celObj._width);

		byte *celHeader = _resource + celObj._celHeaderOffset;
		_dataOffset = READ_SCI11ENDIAN_UINT32(celHeader + 24);
		_uncompressedDataOffset = READ_SCI11ENDIAN_UINT32(celHeader + 28);
		_controlOffset = READ_SCI11ENDIAN_UINT32(celHeader + 32);
	}

	inline const byte *getRow(const int16 y) {
		assert(y >= 0 && y < _sourceHeight);
		if (y != _y) {
			// compressed data segment for row
			byte *row = _resource + _dataOffset + READ_SCI11ENDIAN_UINT32(_resource + _controlOffset + y * 4);

			// uncompressed data segment for row
			byte *literal = _resource + _uncompressedDataOffset + READ_SCI11ENDIAN_UINT32(_resource + _controlOffset + _sourceHeight * 4 + y * 4);

			uint8 length;
			for (int16 i = 0; i < _maxWidth; i += length) {
				byte controlByte = *row++;
				length = controlByte;

				// Run-length encoded
				if (controlByte & 0x80) {
					length &= 0x3F;
					assert(i + length < (int)sizeof(_buffer));

					// Fill with skip color
					if (controlByte & 0x40) {
						memset(_buffer + i, _transparentColor, length);
					// Next value is fill color
					} else {
						memset(_buffer + i, *literal, length);
						++literal;
					}
				// Uncompressed
				} else {
					assert(i + length < (int)sizeof(_buffer));
					memcpy(_buffer + i, literal, length);
					literal += length;
				}
			}
			_y = y;
		}

		return _buffer;
	}
};

#pragma mark -
#pragma mark CelObj - Remappers

/**
 * Pixel mapper for a CelObj with transparent pixels and no
 * remapping data.
 */
struct MAPPER_NoMD {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor) const {
		if (pixel != skipColor) {
			*target = pixel;
		}
	}
};

/**
 * Pixel mapper for a CelObj with no transparent pixels and
 * no remapping data.
 */
struct MAPPER_NoMDNoSkip {
	inline void draw(byte *target, const byte pixel, const uint8) const {
		*target = pixel;
	}
};

/**
 * Pixel mapper for a CelObj with transparent pixels,
 * remapping data, and remapping enabled.
 */
struct MAPPER_Map {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor) const {
		if (pixel != skipColor) {
			// NOTE: For some reason, SSCI never checks if the source
			// pixel is *above* the range of remaps.
			if (pixel < g_sci->_gfxRemap32->getStartColor()) {
				*target = pixel;
			} else if (g_sci->_gfxRemap32->remapEnabled(pixel)) {
				*target = g_sci->_gfxRemap32->remapColor(pixel, *target);
			}
		}
	}
};

/**
 * Pixel mapper for a CelObj with transparent pixels,
 * remapping data, and remapping disabled.
 */
struct MAPPER_NoMap {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor) const {
		// NOTE: For some reason, SSCI never checks if the source
		// pixel is *above* the range of remaps.
		if (pixel != skipColor && pixel < g_sci->_gfxRemap32->getStartColor()) {
			*target = pixel;
		}
	}
};

void CelObj::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect) const {
	const Common::Point &scaledPosition = screenItem._scaledPosition;
	const Ratio &scaleX = screenItem._ratioX;
	const Ratio &scaleY = screenItem._ratioY;
	_drawBlackLines = screenItem._drawBlackLines;

	if (_remap) {
		// NOTE: In the original code this check was `g_Remap_numActiveRemaps && _remap`,
		// but since we are already in a `_remap` branch, there is no reason to check it
		// again
		if (g_sci->_gfxRemap32->getRemapCount()) {
			if (scaleX.isOne() && scaleY.isOne()) {
				if (_compressionType == kCelCompressionNone) {
					if (_drawMirrored) {
						drawUncompHzFlipMap(target, targetRect, scaledPosition);
					} else {
						drawUncompNoFlipMap(target, targetRect, scaledPosition);
					}
				} else {
					if (_drawMirrored) {
						drawHzFlipMap(target, targetRect, scaledPosition);
					} else {
						drawNoFlipMap(target, targetRect, scaledPosition);
					}
				}
			} else {
				if (_compressionType == kCelCompressionNone) {
					scaleDrawUncompMap(target, scaleX, scaleY, targetRect, scaledPosition);
				} else {
					scaleDrawMap(target, scaleX, scaleY, targetRect, scaledPosition);
				}
			}
		} else {
			if (scaleX.isOne() && scaleY.isOne()) {
				if (_compressionType == kCelCompressionNone) {
					if (_drawMirrored) {
						drawUncompHzFlip(target, targetRect, scaledPosition);
					} else {
						drawUncompNoFlip(target, targetRect, scaledPosition);
					}
				} else {
					if (_drawMirrored) {
						drawHzFlip(target, targetRect, scaledPosition);
					} else {
						drawNoFlip(target, targetRect, scaledPosition);
					}
				}
			} else {
				if (_compressionType == kCelCompressionNone) {
					scaleDrawUncomp(target, scaleX, scaleY, targetRect, scaledPosition);
				} else {
					scaleDraw(target, scaleX, scaleY, targetRect, scaledPosition);
				}
			}
		}
	} else {
		if (scaleX.isOne() && scaleY.isOne()) {
			if (_compressionType == kCelCompressionNone) {
				if (_transparent) {
					if (_drawMirrored) {
						drawUncompHzFlipNoMD(target, targetRect, scaledPosition);
					} else {
						drawUncompNoFlipNoMD(target, targetRect, scaledPosition);
					}
				} else {
					if (_drawMirrored) {
						drawUncompHzFlipNoMDNoSkip(target, targetRect, scaledPosition);
					} else {
						drawUncompNoFlipNoMDNoSkip(target, targetRect, scaledPosition);
					}
				}
			} else {
				if (_drawMirrored) {
					drawHzFlipNoMD(target, targetRect, scaledPosition);
				} else {
					drawNoFlipNoMD(target, targetRect, scaledPosition);
				}
			}
		} else {
			if (_compressionType == kCelCompressionNone) {
				scaleDrawUncompNoMD(target, scaleX, scaleY, targetRect, scaledPosition);
			} else {
				scaleDrawNoMD(target, scaleX, scaleY, targetRect, scaledPosition);
			}
		}
	}

	_drawBlackLines = false;
}

void CelObj::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect, bool mirrorX) {
	_drawMirrored = mirrorX;
	draw(target, screenItem, targetRect);
}

void CelObj::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX) {
	_drawMirrored = mirrorX;
	Ratio square;
	drawTo(target, targetRect, scaledPosition, square, square);
}

void CelObj::drawTo(Buffer &target, Common::Rect const &targetRect, Common::Point const &scaledPosition, Ratio const &scaleX, Ratio const &scaleY) const {
	if (_remap) {
		if (scaleX.isOne() && scaleY.isOne()) {
			if (_compressionType == kCelCompressionNone) {
				if (_drawMirrored) {
					drawUncompHzFlipMap(target, targetRect, scaledPosition);
				} else {
					drawUncompNoFlipMap(target, targetRect, scaledPosition);
				}
			} else {
				if (_drawMirrored) {
					drawHzFlipMap(target, targetRect, scaledPosition);
				} else {
					drawNoFlipMap(target, targetRect, scaledPosition);
				}
			}
		} else {
			if (_compressionType == kCelCompressionNone) {
				scaleDrawUncompMap(target, scaleX, scaleY, targetRect, scaledPosition);
			} else {
				scaleDrawMap(target, scaleX, scaleY, targetRect, scaledPosition);
			}
		}
	} else {
		if (scaleX.isOne() && scaleY.isOne()) {
			if (_compressionType == kCelCompressionNone) {
				if (_drawMirrored) {
					drawUncompHzFlipNoMD(target, targetRect, scaledPosition);
				} else {
					drawUncompNoFlipNoMD(target, targetRect, scaledPosition);
				}
			} else {
				if (_drawMirrored) {
					drawHzFlipNoMD(target, targetRect, scaledPosition);
				} else {
					drawNoFlipNoMD(target, targetRect, scaledPosition);
				}
			}
		} else {
			if (_compressionType == kCelCompressionNone) {
				scaleDrawUncompNoMD(target, scaleX, scaleY, targetRect, scaledPosition);
			} else {
				scaleDrawNoMD(target, scaleX, scaleY, targetRect, scaledPosition);
			}
		}
	}
}

uint8 CelObj::readPixel(uint16 x, const uint16 y, bool mirrorX) const {
	if (mirrorX) {
		x = _width - x - 1;
	}

	if (_compressionType == kCelCompressionNone) {
		READER_Uncompressed reader(*this, x + 1);
		return reader.getRow(y)[x];
	} else {
		READER_Compressed reader(*this, x + 1);
		return reader.getRow(y)[x];
	}
}

void CelObj::submitPalette() const {
	if (_hunkPaletteOffset) {
		HunkPalette palette(getResPointer() + _hunkPaletteOffset);
		g_sci->_gfxPalette32->submit(palette);
	}
}

#pragma mark -
#pragma mark CelObj - Caching

int CelObj::_nextCacheId = 1;
CelCache *CelObj::_cache = nullptr;

int CelObj::searchCache(const CelInfo32 &celInfo, int *nextInsertIndex) const {
	*nextInsertIndex = -1;
	int oldestId = _nextCacheId + 1;
	int oldestIndex = 0;

	for (int i = 0, len = _cache->size(); i < len; ++i) {
		CelCacheEntry &entry = (*_cache)[i];

		if (entry.celObj == nullptr) {
			if (*nextInsertIndex == -1) {
				*nextInsertIndex = i;
			}
		} else if (entry.celObj->_info == celInfo) {
			entry.id = ++_nextCacheId;
			return i;
		} else if (oldestId > entry.id) {
			oldestId = entry.id;
			oldestIndex = i;
		}
	}

	if (*nextInsertIndex == -1) {
		*nextInsertIndex = oldestIndex;
	}

	return -1;
}

void CelObj::putCopyInCache(const int cacheIndex) const {
	if (cacheIndex == -1) {
		error("Invalid cache index");
	}

	CelCacheEntry &entry = (*_cache)[cacheIndex];

	if (entry.celObj != nullptr) {
		delete entry.celObj;
	}

	entry.celObj = duplicate();
	entry.id = ++_nextCacheId;
}

#pragma mark -
#pragma mark CelObj - Drawing

template<typename MAPPER, typename SCALER, bool DRAW_BLACK_LINES>
struct RENDERER {
	MAPPER &_mapper;
	SCALER &_scaler;
	const uint8 _skipColor;

	RENDERER(MAPPER &mapper, SCALER &scaler, const uint8 skipColor) :
	_mapper(mapper),
	_scaler(scaler),
	_skipColor(skipColor) {}

	inline void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
		byte *targetPixel = (byte *)target.getPixels() + target.screenWidth * targetRect.top + targetRect.left;

		const int16 skipStride = target.screenWidth - targetRect.width();
		const int16 targetWidth = targetRect.width();
		const int16 targetHeight = targetRect.height();
		for (int16 y = 0; y < targetHeight; ++y) {
			if (DRAW_BLACK_LINES && (y % 2) == 0) {
				memset(targetPixel, 0, targetWidth);
				targetPixel += targetWidth + skipStride;
				continue;
			}

			_scaler.setTarget(targetRect.left, targetRect.top + y);

			for (int16 x = 0; x < targetWidth; ++x) {
				_mapper.draw(targetPixel++, _scaler.read(), _skipColor);
			}

			targetPixel += skipStride;
		}
	}
};

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect.left - scaledPosition.x + targetRect.width(), scaledPosition);
	RENDERER<MAPPER, SCALER, false> renderer(mapper, scaler, _transparentColor);
	renderer.draw(target, targetRect, scaledPosition);
}

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio &scaleX, const Ratio &scaleY) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect, scaledPosition, scaleX, scaleY);
	if (_drawBlackLines) {
		RENDERER<MAPPER, SCALER, true> renderer(mapper, scaler, _transparentColor);
		renderer.draw(target, targetRect, scaledPosition);
	} else {
		RENDERER<MAPPER, SCALER, false> renderer(mapper, scaler, _transparentColor);
		renderer.draw(target, targetRect, scaledPosition);
	}
}

void CelObj::drawHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMap, SCALER_NoScale<true, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMap, SCALER_NoScale<false, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMap, SCALER_NoScale<false, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMap, SCALER_NoScale<true, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::scaleDraw(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_NoMap, SCALER_Scale<true, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMap, SCALER_Scale<false, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

void CelObj::scaleDrawUncomp(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_NoMap, SCALER_Scale<true, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMap, SCALER_Scale<false, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

void CelObj::drawHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_Map, SCALER_NoScale<true, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_Map, SCALER_NoScale<false, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_Map, SCALER_NoScale<false, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_Map, SCALER_NoScale<true, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::scaleDrawMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_Map, SCALER_Scale<true, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_Map, SCALER_Scale<false, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

void CelObj::scaleDrawUncompMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_Map, SCALER_Scale<true, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_Map, SCALER_Scale<false, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

void CelObj::drawNoFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMD, SCALER_NoScale<false, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawHzFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMD, SCALER_NoScale<true, READER_Compressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompNoFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMD, SCALER_NoScale<false, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompNoFlipNoMDNoSkip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMDNoSkip, SCALER_NoScale<false, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompHzFlipNoMD(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMD, SCALER_NoScale<true, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::drawUncompHzFlipNoMDNoSkip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	render<MAPPER_NoMDNoSkip, SCALER_NoScale<true, READER_Uncompressed> >(target, targetRect, scaledPosition);
}

void CelObj::scaleDrawNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored)
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	else
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
}

void CelObj::scaleDrawUncompNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

#pragma mark -
#pragma mark CelObjView

CelObjView::CelObjView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) {
	_info.type = kCelTypeView;
	_info.resourceId = viewId;
	_info.loopNo = loopNo;
	_info.celNo = celNo;
	_mirrorX = false;
	_compressionType = kCelCompressionInvalid;
	_transparent = true;

	int cacheInsertIndex;
	int cacheIndex = searchCache(_info, &cacheInsertIndex);
	if (cacheIndex != -1) {
		CelCacheEntry &entry = (*_cache)[cacheIndex];
		const CelObjView *const cachedCelObj = dynamic_cast<CelObjView *>(entry.celObj);
		if (cachedCelObj == nullptr) {
			error("Expected a CelObjView in cache slot %d", cacheIndex);
		}
		*this = *cachedCelObj;
		entry.id = ++_nextCacheId;
		return;
	}

	// TODO: The next code should be moved to a common file that
	// generates view resource metadata for both SCI16 and SCI32
	// implementations

	Resource *resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, viewId), false);

	// NOTE: SCI2.1/SQ6 just silently returns here.
	if (!resource) {
		warning("View resource %d not loaded", viewId);
		return;
	}

	byte *data = resource->data;

	_scaledWidth = READ_SCI11ENDIAN_UINT16(data + 14);
	_scaledHeight = READ_SCI11ENDIAN_UINT16(data + 16);

	if (_scaledWidth == 0 || _scaledHeight == 0) {
		byte sizeFlag = data[5];
		if (sizeFlag == 0) {
			_scaledWidth = kLowResX;
			_scaledHeight = kLowResY;
		} else if (sizeFlag == 1) {
			_scaledWidth = 640;
			_scaledHeight = 480;
		} else if (sizeFlag == 2) {
			_scaledWidth = 640;
			_scaledHeight = 400;
		}
	}

	uint16 loopCount = data[2];
	if (_info.loopNo >= loopCount) {
		_info.loopNo = loopCount - 1;
	}

	// NOTE: This is the actual check, in the actual location,
	// from SCI engine.
	if (loopNo < 0) {
		error("Loop is less than 0!");
	}

	const uint16 viewHeaderSize = READ_SCI11ENDIAN_UINT16(data);
	const uint8 loopHeaderSize = data[12];
	const uint8 viewHeaderFieldSize = 2;

	byte *loopHeader = data + viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * _info.loopNo);

	if ((int8)loopHeader[0] != -1) {
		if (loopHeader[1] == 1) {
			_mirrorX = true;
		}

		loopHeader = data + viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * (int8)loopHeader[0]);
	}

	uint8 celCount = loopHeader[2];
	if (_info.celNo >= celCount) {
		_info.celNo = celCount - 1;
	}

	_hunkPaletteOffset = READ_SCI11ENDIAN_UINT32(data + 8);
	_celHeaderOffset = READ_SCI11ENDIAN_UINT32(loopHeader + 12) + (data[13] * _info.celNo);

	byte *celHeader = data + _celHeaderOffset;

	_width = READ_SCI11ENDIAN_UINT16(celHeader);
	_height = READ_SCI11ENDIAN_UINT16(celHeader + 2);
	_displace.x = _width / 2 - (int16)READ_SCI11ENDIAN_UINT16(celHeader + 4);
	_displace.y = _height - (int16)READ_SCI11ENDIAN_UINT16(celHeader + 6) - 1;
	_transparentColor = celHeader[8];
	_compressionType = (CelCompressionType)celHeader[9];

	if (_compressionType != kCelCompressionNone && _compressionType != kCelCompressionRLE) {
		error("Compression type not supported - V: %d  L: %d  C: %d", _info.resourceId, _info.loopNo, _info.celNo);
	}

	if (celHeader[10] & 128) {
		// NOTE: This is correct according to SCI2.1/SQ6/DOS;
		// the engine re-reads the byte value as a word value
		uint16 flags = READ_SCI11ENDIAN_UINT16(celHeader + 10);
		_transparent = flags & 1 ? true : false;
		_remap = flags & 2 ? true : false;
	} else if (_compressionType == kCelCompressionNone) {
		_remap = analyzeUncompressedForRemap();
	} else {
		_remap = analyzeForRemap();
	}

	putCopyInCache(cacheInsertIndex);
}

bool CelObjView::analyzeUncompressedForRemap() const {
	byte *pixels = getResPointer() + READ_SCI11ENDIAN_UINT32(getResPointer() + _celHeaderOffset + 24);
	for (int i = 0; i < _width * _height; ++i) {
		const byte pixel = pixels[i];
		if (
			pixel >= g_sci->_gfxRemap32->getStartColor() &&
			pixel <= g_sci->_gfxRemap32->getEndColor() &&
			pixel != _transparentColor
		) {
			return true;
		}
	}
	return false;
}

bool CelObjView::analyzeForRemap() const {
	READER_Compressed reader(*this, _width);
	for (int y = 0; y < _height; y++) {
		const byte *curRow = reader.getRow(y);
		for (int x = 0; x < _width; x++) {
			const byte pixel = curRow[x];
			if (
				pixel >= g_sci->_gfxRemap32->getStartColor() &&
				pixel <= g_sci->_gfxRemap32->getEndColor() &&
				pixel != _transparentColor
			) {
				return true;
			}
		}
	}
	return false;
}

void CelObjView::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, bool mirrorX, const Ratio &scaleX, const Ratio &scaleY) {
	_drawMirrored = mirrorX;
	drawTo(target, targetRect, scaledPosition, scaleX, scaleY);
}

CelObjView *CelObjView::duplicate() const {
	return new CelObjView(*this);
}

byte *CelObjView::getResPointer() const {
	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, _info.resourceId), false);
	if (resource == nullptr) {
		error("Failed to load view %d from resource manager", _info.resourceId);
	}
	return resource->data;
}

#pragma mark -
#pragma mark CelObjPic

CelObjPic::CelObjPic(const GuiResourceId picId, const int16 celNo) {
	_info.type = kCelTypePic;
	_info.resourceId = picId;
	_info.loopNo = 0;
	_info.celNo = celNo;
	_mirrorX = false;
	_compressionType = kCelCompressionInvalid;
	_transparent = true;
	_remap = false;

	int cacheInsertIndex;
	int cacheIndex = searchCache(_info, &cacheInsertIndex);
	if (cacheIndex != -1) {
		CelCacheEntry &entry = (*_cache)[cacheIndex];
		const CelObjPic *const cachedCelObj = dynamic_cast<CelObjPic *>(entry.celObj);
		if (cachedCelObj == nullptr) {
			error("Expected a CelObjPic in cache slot %d", cacheIndex);
		}
		*this = *cachedCelObj;
		entry.id = ++_nextCacheId;
		return;
	}

	Resource *resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypePic, picId), false);

	// NOTE: SCI2.1/SQ6 just silently returns here.
	if (!resource) {
		warning("Pic resource %d not loaded", picId);
		return;
	}

	byte *data = resource->data;

	_celCount = data[2];

	if (_info.celNo >= _celCount) {
		error("Cel number %d greater than cel count %d", _info.celNo, _celCount);
	}

	_celHeaderOffset = READ_SCI11ENDIAN_UINT16(data) + (READ_SCI11ENDIAN_UINT16(data + 4) * _info.celNo);
	_hunkPaletteOffset = READ_SCI11ENDIAN_UINT32(data + 6);

	byte *celHeader = data + _celHeaderOffset;

	_width = READ_SCI11ENDIAN_UINT16(celHeader);
	_height = READ_SCI11ENDIAN_UINT16(celHeader + 2);
	_displace.x = (int16)READ_SCI11ENDIAN_UINT16(celHeader + 4);
	_displace.y = (int16)READ_SCI11ENDIAN_UINT16(celHeader + 6);
	_transparentColor = celHeader[8];
	_compressionType = (CelCompressionType)celHeader[9];
	_priority = READ_SCI11ENDIAN_UINT16(celHeader + 36);
	_relativePosition.x = (int16)READ_SCI11ENDIAN_UINT16(celHeader + 38);
	_relativePosition.y = (int16)READ_SCI11ENDIAN_UINT16(celHeader + 40);

	uint16 sizeFlag1 = READ_SCI11ENDIAN_UINT16(data + 10);
	uint16 sizeFlag2 = READ_SCI11ENDIAN_UINT16(data + 12);

	if (sizeFlag2) {
		_scaledWidth = sizeFlag1;
		_scaledHeight = sizeFlag2;
	} else if (sizeFlag1 == 0) {
		_scaledWidth = kLowResX;
		_scaledHeight = kLowResY;
	} else if (sizeFlag1 == 1) {
		_scaledWidth = 640;
		_scaledHeight = 480;
	} else if (sizeFlag1 == 2) {
		_scaledWidth = 640;
		_scaledHeight = 400;
	}

	if (celHeader[10] & 128) {
		// NOTE: This is correct according to SCI2.1/SQ6/DOS;
		// the engine re-reads the byte value as a word value
		uint16 flags = READ_SCI11ENDIAN_UINT16(celHeader + 10);
		_transparent = flags & 1 ? true : false;
		_remap = flags & 2 ? true : false;
	} else {
		_transparent = _compressionType != kCelCompressionNone ? true : analyzeUncompressedForSkip();

		if (_compressionType != kCelCompressionNone && _compressionType != kCelCompressionRLE) {
			error("Compression type not supported - P: %d  C: %d", picId, celNo);
		}
	}

	putCopyInCache(cacheInsertIndex);
}

bool CelObjPic::analyzeUncompressedForSkip() const {
	byte *resource = getResPointer();
	byte *pixels = resource + READ_SCI11ENDIAN_UINT32(resource + _celHeaderOffset + 24);
	for (int i = 0; i < _width * _height; ++i) {
		uint8 pixel = pixels[i];
		if (pixel == _transparentColor) {
			return true;
		}
	}

	return false;
}

void CelObjPic::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX) {
	Ratio square;
	_drawMirrored = mirrorX;
	drawTo(target, targetRect, scaledPosition, square, square);
}

CelObjPic *CelObjPic::duplicate() const {
	return new CelObjPic(*this);
}

byte *CelObjPic::getResPointer() const {
	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypePic, _info.resourceId), false);
	if (resource == nullptr) {
		error("Failed to load pic %d from resource manager", _info.resourceId);
	}
	return resource->data;
}

#pragma mark -
#pragma mark CelObjMem

CelObjMem::CelObjMem(const reg_t bitmapObject) {
	_info.type = kCelTypeMem;
	_info.bitmap = bitmapObject;
	_mirrorX = false;
	_compressionType = kCelCompressionNone;
	_celHeaderOffset = 0;
	_transparent = true;

	SciBitmap &bitmap = *g_sci->getEngineState()->_segMan->lookupBitmap(bitmapObject);
	_width = bitmap.getWidth();
	_height = bitmap.getHeight();
	_displace = bitmap.getDisplace();
	_transparentColor = bitmap.getSkipColor();
	_scaledWidth = bitmap.getScaledWidth();
	_scaledHeight = bitmap.getScaledHeight();
	_hunkPaletteOffset = bitmap.getHunkPaletteOffset();
	_remap = bitmap.getRemap();
}

CelObjMem *CelObjMem::duplicate() const {
	return new CelObjMem(*this);
}

byte *CelObjMem::getResPointer() const {
	return g_sci->getEngineState()->_segMan->lookupBitmap(_info.bitmap)->getRawData();
}

#pragma mark -
#pragma mark CelObjColor

CelObjColor::CelObjColor(const uint8 color, const int16 width, const int16 height) {
	_info.type = kCelTypeColor;
	_info.color = color;
	_displace.x = 0;
	_displace.y = 0;
	_scaledWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	_scaledHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
	_hunkPaletteOffset = 0;
	_mirrorX = false;
	_remap = false;
	_width = width;
	_height = height;
}

void CelObjColor::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect, const bool mirrorX) {
	// TODO: The original engine sets this flag but why? One cannot
	// draw a solid color mirrored.
	_drawMirrored = mirrorX;
	draw(target, targetRect);
}
void CelObjColor::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, bool mirrorX) {
	error("Unsupported method");
}
void CelObjColor::draw(Buffer &target, const Common::Rect &targetRect) const {
	target.fillRect(targetRect, _info.color);
}

CelObjColor *CelObjColor::duplicate() const {
	return new CelObjColor(*this);
}

byte *CelObjColor::getResPointer() const {
	error("Unsupported method");
}
} // End of namespace Sci
