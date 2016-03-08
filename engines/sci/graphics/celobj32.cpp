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
#include "sci/graphics/picture.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/view.h"

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

void CelObj::init() {
	CelObj::deinit();
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
	const byte *_row;
	READER _reader;
	const int16 _lastIndex;

	SCALER_NoScale(const CelObj &celObj, const int16 maxWidth) :
	_reader(celObj, FLIP ? celObj._width : maxWidth),
	_lastIndex(celObj._width - 1) {}

	inline void setSource(const int16 x, const int16 y) {
		_row = _reader.getRow(y);

		if (FLIP) {
			_row += _lastIndex - x;
		} else {
			_row += x;
		}
	}

	inline byte read() {
		if (FLIP) {
			return *_row--;
		} else {
			return *_row++;
		}
	}
};

template<bool FLIP, typename READER>
struct SCALER_Scale {
	const byte *_row;
	READER _reader;
	const CelScalerTable *_table;
	int16 _x;
	const uint16 _lastIndex;

	SCALER_Scale(const CelObj &celObj, const int16 maxWidth, const Ratio scaleX, const Ratio scaleY) :
	// The maximum width of the scaled object may not be as
	// wide as the source data it requires if downscaling,
	// so just always make the reader decompress an entire
	// line of source data when scaling
	_reader(celObj, celObj._width),
	_table(CelObj::_scaler->getScalerTable(scaleX, scaleY)),
	_lastIndex(maxWidth - 1) {}

	inline void setSource(const int16 x, const int16 y) {
		_row = _reader.getRow(_table->valuesY[y]);
		if (FLIP) {
			_x = _lastIndex - x;
		} else {
			_x = x;
		}
	}

	inline byte read() {
		if (FLIP) {
			return _row[_table->valuesX[_x--]];
		} else {
			return _row[_table->valuesX[_x++]];
		}
	}
};

#pragma mark -
#pragma mark CelObj - Resource readers

struct READER_Uncompressed {
private:
	byte *_pixels;
	const int16 _sourceWidth;

public:
	READER_Uncompressed(const CelObj &celObj, const int16) :
	_sourceWidth(celObj._width) {
		byte *resource = celObj.getResPointer();
		_pixels = resource + READ_SCI11ENDIAN_UINT32(resource + celObj._celHeaderOffset + 24);
	}

	inline const byte *getRow(const int16 y) const {
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
		assert(_maxWidth <= celObj._width);

		byte *celHeader = _resource + celObj._celHeaderOffset;
		_dataOffset = READ_SCI11ENDIAN_UINT32(celHeader + 24);
		_uncompressedDataOffset = READ_SCI11ENDIAN_UINT32(celHeader + 28);
		_controlOffset = READ_SCI11ENDIAN_UINT32(celHeader + 32);
	}

	inline const byte *getRow(const int16 y) {
		if (y != _y) {
			// compressed data segment for row
			byte *row = _resource + _dataOffset + READ_SCI11ENDIAN_UINT32(_resource + _controlOffset + y * 4);

			// uncompressed data segment for row
			byte *literal = _resource + _uncompressedDataOffset + READ_SCI11ENDIAN_UINT32(_resource + _controlOffset + _sourceHeight * 4 + y * 4);

			uint8 length;
			for (int i = 0; i < _maxWidth; i += length) {
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

struct MAPPER_NoMD {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor) const {
		if (pixel != skipColor) {
			*target = pixel;
		}
	}
};
struct MAPPER_NoMDNoSkip {
	inline void draw(byte *target, const byte pixel, const uint8) const {
		*target = pixel;
	}
};

void CelObj::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect) const {
	const Buffer &priorityMap = g_sci->_gfxFrameout->getPriorityMap();
	const Common::Point &scaledPosition = screenItem._scaledPosition;
	const Ratio &scaleX = screenItem._ratioX;
	const Ratio &scaleY = screenItem._ratioY;

	if (_remap) {
		if (g_sci->_gfxFrameout->_hasRemappedScreenItem) {
			const uint8 priority = MAX((int16)0, MIN((int16)255, screenItem._priority));

			// NOTE: In the original engine code, there was a second branch for
			// _remap here that would then call the following functions if _remap was false:
			//
			// drawHzFlip(Buffer &, Buffer &, Common::Rect &, Common::Point &, uint8)
			// drawNoFlip(Buffer &, Buffer &, Common::Rect &, Common::Point &, uint8)
			// drawUncompHzFlip(Buffer &, Buffer &, Common::Rect &, Common::Point &, uint8)
			// drawUncompNoFlip(Buffer &, Buffer &, Common::Rect &, Common::Point &, uint8)
			// scaleDraw(Buffer &, Buffer &, Ratio &, Ratio &, Common::Rect &, Common::Point &, uint8)
			// scaleDrawUncomp(Buffer &, Buffer &, Ratio &, Ratio &, Common::Rect &, Common::Point &, uint8)
			//
			// However, obviously, _remap cannot be false here. This dead code branch existed in
			// at least SCI2/GK1 and SCI2.1/SQ6.

			if (scaleX.isOne() && scaleY.isOne()) {
				if (_compressionType == kCelCompressionNone) {
					if (_drawMirrored) {
						drawUncompHzFlipMap(target, priorityMap, targetRect, scaledPosition, priority);
					} else {
						drawUncompNoFlipMap(target, priorityMap, targetRect, scaledPosition, priority);
					}
				} else {
					if (_drawMirrored) {
						drawHzFlipMap(target, priorityMap, targetRect, scaledPosition, priority);
					} else {
						drawNoFlipMap(target, priorityMap, targetRect, scaledPosition, priority);
					}
				}
			} else {
				if (_compressionType == kCelCompressionNone) {
					scaleDrawUncompMap(target, priorityMap, scaleX, scaleY, targetRect, scaledPosition, priority);
				} else {
					scaleDrawMap(target, priorityMap, scaleX, scaleY, targetRect, scaledPosition, priority);
				}
			}
		} else {
			// NOTE: In the original code this check was `g_Remap_numActiveRemaps && _remap`,
			// but since we are already in a `_remap` branch, there is no reason to check it
			// again
			if (/* TODO: g_Remap_numActiveRemaps */ false) {
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
		}
	} else {
		if (g_sci->_gfxFrameout->_hasRemappedScreenItem) {
			const uint8 priority = MAX((int16)0, MIN((int16)255, screenItem._priority));
			if (scaleX.isOne() && scaleY.isOne()) {
				if (_compressionType == kCelCompressionNone) {
					if (_drawMirrored) {
						drawUncompHzFlipNoMD(target, priorityMap, targetRect, scaledPosition, priority);
					} else {
						drawUncompNoFlipNoMD(target, priorityMap, targetRect, scaledPosition, priority);
					}
				} else {
					if (_drawMirrored) {
						drawHzFlipNoMD(target, priorityMap, targetRect, scaledPosition, priority);
					} else {
						drawNoFlipNoMD(target, priorityMap, targetRect, scaledPosition, priority);
					}
				}
			} else {
				if (_compressionType == kCelCompressionNone) {
					scaleDrawUncompNoMD(target, priorityMap, scaleX, scaleY, targetRect, scaledPosition, priority);
				} else {
					scaleDrawNoMD(target, priorityMap, scaleX, scaleY, targetRect, scaledPosition, priority);
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
	}
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
		Palette palette;

		byte *res = getResPointer();
		// NOTE: In SCI engine this uses HunkPalette::Init.
		// TODO: Use a better size value
		g_sci->_gfxPalette32->createFromData(res + _hunkPaletteOffset, 999999, &palette);
		g_sci->_gfxPalette32->submit(palette);
	}
}

#pragma mark -
#pragma mark CelObj - Caching
int CelObj::_nextCacheId = 1;
CelCache *CelObj::_cache = nullptr;

int CelObj::searchCache(const CelInfo32 &celInfo, int *nextInsertIndex) const {
	int oldestId = _nextCacheId + 1;
	int oldestIndex = -1;

	for (int i = 0, len = _cache->size(); i < len; ++i) {
		CelCacheEntry &entry = (*_cache)[i];

		if (entry.celObj != nullptr) {
			if (entry.celObj->_info == celInfo) {
				entry.id = ++_nextCacheId;
				return i;
			}

			if (oldestId > entry.id) {
				oldestId = entry.id;
				oldestIndex = i;
			}
		} else if (oldestIndex == -1) {
			oldestIndex = i;
		}
	}

	// NOTE: Unlike the original SCI engine code, the out-param
	// here is only updated if there was not a cache hit.
	*nextInsertIndex = oldestIndex;
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

template<typename MAPPER, typename SCALER>
struct RENDERER {
	MAPPER &_mapper;
	SCALER &_scaler;
	const uint8 _skipColor;

	RENDERER(MAPPER &mapper, SCALER &scaler, const uint8 skipColor) :
	_mapper(mapper),
	_scaler(scaler),
	_skipColor(skipColor) {}

	inline void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
		const int16 sourceX = targetRect.left - scaledPosition.x;
		const int16 sourceY = targetRect.top - scaledPosition.y;

		byte *targetPixel = (byte *)target.getPixels() + target.screenWidth * targetRect.top + targetRect.left;

		const int16 skipStride = target.screenWidth - targetRect.width();
		const int16 targetWidth = targetRect.width();
		const int16 targetHeight = targetRect.height();
		for (int y = 0; y < targetHeight; ++y) {
			_scaler.setSource(sourceX, sourceY + y);

			for (int x = 0; x < targetWidth; ++x) {
				_mapper.draw(targetPixel++, _scaler.read(), _skipColor);
			}

			targetPixel += skipStride;
		}
	}
};

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect.left - scaledPosition.x + targetRect.width());
	RENDERER<MAPPER, SCALER> renderer(mapper, scaler, _transparentColor);
	renderer.draw(target, targetRect, scaledPosition);
}

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio &scaleX, const Ratio &scaleY) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect.left - scaledPosition.x + targetRect.width(), scaleX, scaleY);
	RENDERER<MAPPER, SCALER> renderer(mapper, scaler, _transparentColor);
	renderer.draw(target, targetRect, scaledPosition);
}

void dummyFill(Buffer &target, const Common::Rect &targetRect) {
	target.fillRect(targetRect, 250);
}

void CelObj::drawHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawHzFlip");
	dummyFill(target, targetRect);
}
void CelObj::drawNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawNoFlip");
	dummyFill(target, targetRect);
}
void CelObj::drawUncompNoFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawUncompNoFlip");
	dummyFill(target, targetRect);
}
void CelObj::drawUncompHzFlip(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawUncompHzFlip");
	dummyFill(target, targetRect);
}
void CelObj::scaleDraw(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("scaleDraw");
	dummyFill(target, targetRect);
}
void CelObj::scaleDrawUncomp(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("scaleDrawUncomp");
	dummyFill(target, targetRect);
}
void CelObj::drawHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawHzFlipMap");
	dummyFill(target, targetRect);
}
void CelObj::drawNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawNoFlipMap");
	dummyFill(target, targetRect);
}
void CelObj::drawUncompNoFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawUncompNoFlipMap");
	dummyFill(target, targetRect);
}
void CelObj::drawUncompHzFlipMap(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("drawUncompHzFlipMap");
	dummyFill(target, targetRect);
}
void CelObj::scaleDrawMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("scaleDrawMap");
	dummyFill(target, targetRect);
}
void CelObj::scaleDrawUncompMap(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	debug("scaleDrawUncompMap");
	dummyFill(target, targetRect);
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
	if (_drawMirrored) {
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

void CelObj::scaleDrawUncompNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	if (_drawMirrored) {
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

// TODO: These functions may all be vestigial.
void CelObj::drawHzFlipMap(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawNoFlipMap(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawUncompNoFlipMap(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawUncompHzFlipMap(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::scaleDrawMap(Buffer &target, const Buffer &priorityMap, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::scaleDrawUncompMap(Buffer &target, const Buffer &priorityMap, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawHzFlipNoMD(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawNoFlipNoMD(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawUncompNoFlipNoMD(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::drawUncompHzFlipNoMD(Buffer &target, const Buffer &priorityMap, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::scaleDrawNoMD(Buffer &target, const Buffer &priorityMap, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}
void CelObj::scaleDrawUncompNoMD(Buffer &target, const Buffer &priorityMap, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition, const uint8 priority) const {}

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
		*this = *dynamic_cast<CelObjView *>(entry.celObj);
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
			_scaledWidth = 320;
			_scaledHeight = 200;
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
		uint8 pixel = pixels[i];
		if (/* TODO: pixel >= Remap::minRemapColor && pixel <= Remap::maxRemapColor */ false && pixel != _transparentColor) {
			return true;
		}
	}
	return false;
}

bool CelObjView::analyzeForRemap() const {
	// TODO: Implement decompression and analysis
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
	return g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, _info.resourceId), false)->data;
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
		*this = *dynamic_cast<CelObjPic *>(entry.celObj);
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
		_scaledWidth = 320;
		_scaledHeight = 200;
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
	return g_sci->getResMan()->findResource(ResourceId(kResourceTypePic, _info.resourceId), false)->data;
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

	BitmapResource bitmap(bitmapObject);
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
	return g_sci->getEngineState()->_segMan->getHunkPointer(_info.bitmap);
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
