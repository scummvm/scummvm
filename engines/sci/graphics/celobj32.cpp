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

#include "sci/resource/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/text32.h"
#include "sci/engine/workarounds.h"
#include "sci/util.h"
#include "graphics/larryScale.h"
#include "common/config-manager.h"
#include "common/gui_options.h"

namespace Sci {
#pragma mark CelScaler

CelScaler *CelObj::_scaler = nullptr;

void CelScaler::activateScaleTables(const Ratio &scaleX, const Ratio &scaleY) {
	for (int i = 0; i < ARRAYSIZE(_scaleTables); ++i) {
		if (_scaleTables[i].scaleX == scaleX && _scaleTables[i].scaleY == scaleY) {
			_activeIndex = i;
			return;
		}
	}

	const int i = 1 - _activeIndex;
	_activeIndex = i;
	CelScalerTable &table = _scaleTables[i];

	if (table.scaleX != scaleX) {
		buildLookupTable(table.valuesX, scaleX, kCelScalerTableSize);
		table.scaleX = scaleX;
	}

	if (table.scaleY != scaleY) {
		buildLookupTable(table.valuesY, scaleY, kCelScalerTableSize);
		table.scaleY = scaleY;
	}
}

void CelScaler::buildLookupTable(int *table, const Ratio &ratio, const int size) {
	int value = 0;
	int remainder = 0;
	const int num = ratio.getNumerator();
	for (int i = 0; i < size; ++i) {
		*table++ = value;
		remainder += ratio.getDenominator();
		if (remainder >= num) {
			value += remainder / num;
			remainder %= num;
		}
	}
}

const CelScalerTable &CelScaler::getScalerTable(const Ratio &scaleX, const Ratio &scaleY) {
	activateScaleTables(scaleX, scaleY);
	return _scaleTables[_activeIndex];
}

#pragma mark -
#pragma mark CelObj
bool CelObj::_drawBlackLines = false;

void CelObj::init() {
	CelObj::deinit();
	_drawBlackLines = false;
	_nextCacheId = 1;
	_scaler = new CelScaler();
	_cache = new CelCache(100);
}

void CelObj::deinit() {
	delete _scaler;
	_scaler = nullptr;
	delete _cache;
	_cache = nullptr;
}

#pragma mark -
#pragma mark CelObj - Scalers

template<bool FLIP, typename READER>
struct SCALER_NoScale {
#ifndef RELEASE_BUILD
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
#ifndef RELEASE_BUILD
			_rowEdge = _row - 1;
#endif
			_row += _lastIndex - (x - _sourceX);
#ifndef RELEASE_BUILD

			assert(_row > _rowEdge);
#endif
		} else {
#ifndef RELEASE_BUILD
			_rowEdge = _row + _lastIndex + 1;
#endif
			_row += x - _sourceX;
#ifndef RELEASE_BUILD
			assert(_row < _rowEdge);
#endif
		}
	}

	inline byte read() {
#ifndef RELEASE_BUILD
		assert(_row != _rowEdge);
#endif
		if (FLIP) {
			return *_row--;
		} else {
			return *_row++;
		}
	}
};

template<bool FLIP, typename READER>
struct SCALER_Scale {
#ifndef RELEASE_BUILD
	int16 _minX;
	int16 _maxX;
#endif
	const byte *_row;
	READER _reader;
	// If _sourceBuffer is set, it contains the full (possibly scaled) source
	// image and takes precedence over _reader.
	Common::SharedPtr<Buffer> _sourceBuffer;
	int16 _x;
	static int16 _valuesX[kCelScalerTableSize];
	static int16 _valuesY[kCelScalerTableSize];

	SCALER_Scale(const CelObj &celObj, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio scaleX, const Ratio scaleY) :
	_row(nullptr),
#ifndef RELEASE_BUILD
	_minX(targetRect.left),
	_maxX(targetRect.right - 1),
#endif
	// The maximum width of the scaled object may not be as wide as the source
	// data it requires if downscaling, so just always make the reader
	// decompress an entire line of source data when scaling
	_reader(celObj, celObj._width),
	_sourceBuffer() {
#ifndef RELEASE_BUILD
		assert(_minX <= _maxX);
#endif

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

		const CelScalerTable &table = CelObj::_scaler->getScalerTable(scaleX, scaleY);

		const bool useLarryScale = Common::checkGameGUIOption(GAMEOPTION_LARRYSCALE, ConfMan.get("guioptions")) && ConfMan.getBool("enable_larryscale");
		if (useLarryScale) {
			// LarryScale is an alternative, high-quality cel scaler implemented
			// for ScummVM. Due to the nature of smooth upscaling, it does *not*
			// respect the global scaling pattern. Instead, it simply scales the
			// cel to the extent of targetRect.

			class Copier: public Graphics::RowReader, public Graphics::RowWriter {
				READER &_souceReader;
				Buffer &_targetBuffer;
			public:
				Copier(READER& souceReader, Buffer& targetBuffer) :
					_souceReader(souceReader),
					_targetBuffer(targetBuffer) {}
				const Graphics::LarryScaleColor* readRow(int y) override {
					return _souceReader.getRow(y);
				}
				void writeRow(int y, const Graphics::LarryScaleColor* row) override {
					memcpy(_targetBuffer.getBasePtr(0, y), row, _targetBuffer.w);
				}
			};

			// Scale the cel using LarryScale and write it to _sourceBuffer
			// scaledImageRect is not necessarily identical to targetRect
			// because targetRect may be cropped to render only a segment.
			Common::Rect scaledImageRect(
				scaledPosition.x,
				scaledPosition.y,
				scaledPosition.x + (celObj._width * scaleX).toInt(),
				scaledPosition.y + (celObj._height * scaleY).toInt());
			_sourceBuffer = Common::SharedPtr<Buffer>(new Buffer(), Graphics::SurfaceDeleter());
			_sourceBuffer->create(
				scaledImageRect.width(), scaledImageRect.height(),
				Graphics::PixelFormat::createFormatCLUT8());
			Copier copier(_reader, *_sourceBuffer);
			Graphics::larryScale(
				celObj._width, celObj._height, celObj._skipColor, copier,
				scaledImageRect.width(), scaledImageRect.height(), copier);

			// Set _valuesX and _valuesY to reference the scaled image without additional scaling
			for (int16 x = targetRect.left; x < targetRect.right; ++x) {
				const int16 unsafeValue = FLIP
					? scaledImageRect.right - x - 1
					: x - scaledImageRect.left;
				_valuesX[x] = CLIP<int16>(unsafeValue, 0, scaledImageRect.width() - 1);
			}
			for (int16 y = targetRect.top; y < targetRect.bottom; ++y) {
				const int16 unsafeValue = y - scaledImageRect.top;
				_valuesY[y] = CLIP<int16>(unsafeValue, 0, scaledImageRect.height() - 1);
			}
		} else {
			const bool useGlobalScaling = g_sci->_gfxFrameout->getScriptWidth() == kLowResX;
			if (useGlobalScaling) {
				const int16 unscaledX = (scaledPosition.x / scaleX).toInt();
				if (FLIP) {
					const int lastIndex = celObj._width - 1;
					for (int16 x = targetRect.left; x < targetRect.right; ++x) {
						_valuesX[x] = lastIndex - (table.valuesX[x] - unscaledX);
					}
				} else {
					for (int16 x = targetRect.left; x < targetRect.right; ++x) {
						_valuesX[x] = table.valuesX[x] - unscaledX;
					}
				}

				const int16 unscaledY = (scaledPosition.y / scaleY).toInt();
				for (int16 y = targetRect.top; y < targetRect.bottom; ++y) {
					_valuesY[y] = table.valuesY[y] - unscaledY;
				}
			} else {
				if (FLIP) {
					const int lastIndex = celObj._width - 1;
					for (int16 x = targetRect.left; x < targetRect.right; ++x) {
						_valuesX[x] = lastIndex - table.valuesX[x - scaledPosition.x];
					}
				} else {
					for (int16 x = targetRect.left; x < targetRect.right; ++x) {
						_valuesX[x] = table.valuesX[x - scaledPosition.x];
					}
				}

				for (int16 y = targetRect.top; y < targetRect.bottom; ++y) {
					_valuesY[y] = table.valuesY[y - scaledPosition.y];
				}
			}
		}
	}

	inline void setTarget(const int16 x, const int16 y) {
		_row = _sourceBuffer
			? static_cast<const byte *>( _sourceBuffer->getBasePtr(0, _valuesY[y]))
			: _reader.getRow(_valuesY[y]);
		_x = x;
#ifndef RELEASE_BUILD
		assert(_x >= _minX && _x <= _maxX);
#endif
	}

	inline byte read() {
#ifndef RELEASE_BUILD
		assert(_x >= _minX && _x <= _maxX);
#endif
		return _row[_valuesX[_x++]];
	}
};

template<bool FLIP, typename READER>
int16 SCALER_Scale<FLIP, READER>::_valuesX[kCelScalerTableSize];
template<bool FLIP, typename READER>
int16 SCALER_Scale<FLIP, READER>::_valuesY[kCelScalerTableSize];

#pragma mark -
#pragma mark CelObj - Resource readers

struct READER_Uncompressed {
private:
#ifndef RELEASE_BUILD
	int16 _sourceHeight;
#endif
	const byte *_pixels;
	const int16 _sourceWidth;

public:
	READER_Uncompressed(const CelObj &celObj, const int16) :
#ifndef RELEASE_BUILD
	_sourceHeight(celObj._height),
#endif
	_sourceWidth(celObj._width) {
		const SciSpan<const byte> resource = celObj.getResPointer();
		const uint32 pixelsOffset = resource.getUint32SEAt(celObj._celHeaderOffset + 24);
		const int32 numPixels = MIN<int32>(resource.size() - pixelsOffset, celObj._width * celObj._height);

		if (numPixels < celObj._width * celObj._height) {
			warning("%s is truncated", celObj._info.toString().c_str());
#ifndef RELEASE_BUILD
			_sourceHeight = numPixels / celObj._width;
#endif
		}

		_pixels = resource.getUnsafeDataAt(pixelsOffset, numPixels);
	}

	inline const byte *getRow(const int16 y) const {
#ifndef RELEASE_BUILD
		assert(y >= 0 && y < _sourceHeight);
#endif
		return _pixels + y * _sourceWidth;
	}
};

struct READER_Compressed {
private:
	const SciSpan<const byte> _resource;
	byte _buffer[kCelScalerTableSize];
	uint32 _controlOffset;
	uint32 _dataOffset;
	uint32 _uncompressedDataOffset;
	int16 _y;
	const int16 _sourceHeight;
	const uint8 _skipColor;
	const int16 _maxWidth;

public:
	READER_Compressed(const CelObj &celObj, const int16 maxWidth) :
	_resource(celObj.getResPointer()),
	_y(-1),
	_sourceHeight(celObj._height),
	_skipColor(celObj._skipColor),
	_maxWidth(maxWidth) {
		assert(maxWidth <= celObj._width);

		const SciSpan<const byte> celHeader = _resource.subspan(celObj._celHeaderOffset);
		_dataOffset = celHeader.getUint32SEAt(24);
		_uncompressedDataOffset = celHeader.getUint32SEAt(28);
		_controlOffset = celHeader.getUint32SEAt(32);
	}

	inline const byte *getRow(const int16 y) {
		assert(y >= 0 && y < _sourceHeight);
		if (y != _y) {
			// compressed data segment for row
			const uint32 rowOffset = _resource.getUint32SEAt(_controlOffset + y * sizeof(uint32));

			uint32 rowCompressedSize;
			if (y + 1 < _sourceHeight) {
				rowCompressedSize = _resource.getUint32SEAt(_controlOffset + (y + 1) * sizeof(uint32)) - rowOffset;
			} else {
				rowCompressedSize = _resource.size() - rowOffset - _dataOffset;
			}

			const byte *row = _resource.getUnsafeDataAt(_dataOffset + rowOffset, rowCompressedSize);

			// uncompressed data segment for row
			const uint32 literalOffset = _resource.getUint32SEAt(_controlOffset + _sourceHeight * sizeof(uint32) + y * sizeof(uint32));

			uint32 literalRowSize;
			if (y + 1 < _sourceHeight) {
				literalRowSize = _resource.getUint32SEAt(_controlOffset + _sourceHeight * sizeof(uint32) + (y + 1) * sizeof(uint32)) - literalOffset;
			} else {
				literalRowSize = _resource.size() - literalOffset - _uncompressedDataOffset;
			}

			const byte *literal = _resource.getUnsafeDataAt(_uncompressedDataOffset + literalOffset, literalRowSize);

			uint8 length;
			for (int16 i = 0; i < _maxWidth; i += length) {
				const byte controlByte = *row++;
				length = controlByte;

				// Run-length encoded
				if (controlByte & 0x80) {
					length &= 0x3F;
					assert(i + length < (int)sizeof(_buffer));

					// Fill with skip color
					if (controlByte & 0x40) {
						memset(_buffer + i, _skipColor, length);
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
 * Translation for pixels from Mac pic and view cels to the PC palette.
 * The Mac OS palette required 0 to be white and 255 to be black, which is the
 * opposite of the PC palette. Mac cels use the Mac palette but the colors in
 * scripts are constant between versions. SSCI handles this with many Mac-only
 * translations throughout the interpreter. We use the PC palette and translate
 * the cel pixels here, similar to the SCI16 code in GfxView::unpackCel. The
 * difference is that in SCI32 we decompress while drawing, while in SCI16 cels
 * are unpacked to a buffer first, making that translation code simpler.
 */
inline byte translateMacColor(bool isMacSource, byte color) {
	if (isMacSource) {
		if (color == 0) {
			return 255;
		} else if (color == 255) {
			return 0;
		}
	}
	return color;
}

/**
 * Pixel mapper for a CelObj with transparent pixels and no
 * remapping data.
 */
struct MAPPER_NoMD {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor, const bool isMacSource) const {
		if (pixel != skipColor) {
			*target = translateMacColor(isMacSource, pixel);
		}
	}
};

/**
 * Pixel mapper for a CelObj with no transparent pixels and
 * no remapping data.
 */
struct MAPPER_NoMDNoSkip {
	inline void draw(byte *target, const byte pixel, const uint8, const bool isMacSource) const {
		*target = translateMacColor(isMacSource, pixel);
	}
};

/**
 * Pixel mapper for a CelObj with transparent pixels,
 * remapping data, and remapping enabled.
 */
struct MAPPER_Map {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor, const bool isMacSource) const {
		if (pixel != skipColor) {
			// For some reason, SSCI never checks if the source pixel is *above*
			// the range of remaps, so we do not either.
			if (pixel < g_sci->_gfxRemap32->getStartColor()) {
				*target = translateMacColor(isMacSource, pixel);
			} else if (g_sci->_gfxRemap32->remapEnabled(pixel)) {
				*target = g_sci->_gfxRemap32->remapColor(translateMacColor(isMacSource, pixel), *target);
			}
		}
	}
};

/**
 * Pixel mapper for a CelObj with transparent pixels,
 * remapping data, and remapping disabled.
 */
struct MAPPER_NoMap {
	inline void draw(byte *target, const byte pixel, const uint8 skipColor, const bool isMacSource) const {
		// For some reason, SSCI never checks if the source pixel is *above* the
		// range of remaps, so we do not either.
		if (pixel != skipColor && pixel < g_sci->_gfxRemap32->getStartColor()) {
			*target = translateMacColor(isMacSource, pixel);
		}
	}
};

void CelObj::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect) const {
	const Common::Point &scaledPosition = screenItem._scaledPosition;
	const Ratio &scaleX = screenItem._ratioX;
	const Ratio &scaleY = screenItem._ratioY;
	_drawBlackLines = screenItem._drawBlackLines;

	if (_remap) {
		// In SSCI, this check was `g_Remap_numActiveRemaps && _remap`, but
		// since we are already in a `_remap` branch, there is no reason to
		// check that again
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
		const SciSpan<const byte> data = getResPointer();
		const HunkPalette palette(data.subspan(_hunkPaletteOffset));
		g_sci->_gfxPalette32->submit(palette);
	}
}

#pragma mark -
#pragma mark CelObj - Caching

int CelObj::_nextCacheId = 1;
CelCache *CelObj::_cache = nullptr;

int CelObj::searchCache(const CelInfo32 &celInfo, int *const nextInsertIndex) const {
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
	entry.celObj.reset(duplicate());
	entry.id = ++_nextCacheId;
}

#pragma mark -
#pragma mark CelObj - Drawing

template<typename MAPPER, typename SCALER, bool DRAW_BLACK_LINES>
struct RENDERER {
	MAPPER &_mapper;
	SCALER &_scaler;
	const uint8 _skipColor;
	const bool _isMacSource;

	RENDERER(MAPPER &mapper, SCALER &scaler, const uint8 skipColor, const bool isMacSource) :
	_mapper(mapper),
	_scaler(scaler),
	_skipColor(skipColor),
	_isMacSource(isMacSource) {}

	inline void draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
		byte *targetPixel = (byte *)target.getPixels() + target.w * targetRect.top + targetRect.left;

		const int16 skipStride = target.w - targetRect.width();
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
				_mapper.draw(targetPixel++, _scaler.read(), _skipColor, _isMacSource);
			}

			targetPixel += skipStride;
		}
	}
};

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect.left - scaledPosition.x + targetRect.width(), scaledPosition);
	RENDERER<MAPPER, SCALER, false> renderer(mapper, scaler, _skipColor, _isMacSource);
	renderer.draw(target, targetRect, scaledPosition);
}

template<typename MAPPER, typename SCALER>
void CelObj::render(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const Ratio &scaleX, const Ratio &scaleY) const {

	MAPPER mapper;
	SCALER scaler(*this, targetRect, scaledPosition, scaleX, scaleY);
	if (_drawBlackLines) {
		RENDERER<MAPPER, SCALER, true> renderer(mapper, scaler, _skipColor, _isMacSource);
		renderer.draw(target, targetRect, scaledPosition);
	} else {
		RENDERER<MAPPER, SCALER, false> renderer(mapper, scaler, _skipColor, _isMacSource);
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
	// In SSCI the checks are > because their rects are BR-inclusive; our checks
	// are >= because our rects are BR-exclusive
	if (targetRect.left >= targetRect.right ||
		 targetRect.top >= targetRect.bottom) {
		return;
	}

	if (_drawMirrored)
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	else
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Compressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
}

void CelObj::scaleDrawUncompNoMD(Buffer &target, const Ratio &scaleX, const Ratio &scaleY, const Common::Rect &targetRect, const Common::Point &scaledPosition) const {
	// In SSCI the checks are > because their rects are BR-inclusive; our checks
	// are >= because our rects are BR-exclusive
	if (targetRect.left >= targetRect.right ||
		 targetRect.top >= targetRect.bottom) {
		return;
	}

	if (_drawMirrored) {
		render<MAPPER_NoMD, SCALER_Scale<true, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	} else {
		render<MAPPER_NoMD, SCALER_Scale<false, READER_Uncompressed> >(target, targetRect, scaledPosition, scaleX, scaleY);
	}
}

#pragma mark -
#pragma mark CelObjView

int16 CelObjView::getNumLoops(const GuiResourceId viewId) {
	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, viewId), false);

	if (!resource) {
		return 0;
	}

	return resource->getUint8At(2);
}

int16 CelObjView::getNumCels(const GuiResourceId viewId, int16 loopNo) {
	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, viewId), false);

	if (!resource) {
		return 0;
	}

	const SciSpan<const byte> &data = *resource;

	const uint16 loopCount = data[2];

	// Every version of SSCI has a logic error in this function that causes
	// random memory to be read if a script requests the cel count for one past
	// the maximum loop index. For example, GK1 room 808 does this, and gets
	// stuck in an infinite loop because the game script expects this method to
	// return a non-zero value.
	// This bug is triggered in basically every SCI32 game and appears to be
	// universally fixable simply by always using the next lowest loop instead.
	if (loopNo == loopCount) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		debugC(kDebugLevelWorkarounds, "Workaround: kNumCels loop %d -> loop %d in view %u, %s", loopNo, loopNo - 1, viewId, origin.toString().c_str());
		--loopNo;
	}

	if (loopNo > loopCount || loopNo < 0) {
		return 0;
	}

	const uint16 viewHeaderSize = data.getUint16SEAt(0);
	const uint8 loopHeaderSize = data[12];
	const uint8 viewHeaderFieldSize = 2;

	SciSpan<const byte> loopHeader = data.subspan(viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * loopNo));

	if (loopHeader.getInt8At(0) != -1) {
		loopHeader = data.subspan(viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * loopHeader.getInt8At(0)));
	}

	return loopHeader[2];
}

CelObjView::CelObjView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) {
	_info.type = kCelTypeView;
	_info.resourceId = viewId;
	_info.loopNo = loopNo;
	_info.celNo = celNo;
	_mirrorX = false;
	_isMacSource = (g_sci->getPlatform() == Common::kPlatformMacintosh);
	_compressionType = kCelCompressionInvalid;
	_transparent = true;

	int cacheInsertIndex;
	const int cacheIndex = searchCache(_info, &cacheInsertIndex);
	if (cacheIndex != -1) {
		CelCacheEntry &entry = (*_cache)[cacheIndex];
		const CelObjView *const cachedCelObj = dynamic_cast<CelObjView *>(entry.celObj.get());
		if (cachedCelObj == nullptr) {
			error("Expected a CelObjView in cache slot %d", cacheIndex);
		}
		*this = *cachedCelObj;
		entry.id = ++_nextCacheId;
		return;
	}

	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, viewId), false);

	// SSCI just silently returns here
	if (!resource) {
		error("View resource %d not found", viewId);
	}

	const Resource &data = *resource;

	_xResolution = data.getUint16SEAt(14);
	_yResolution = data.getUint16SEAt(16);

	if (_xResolution == 0 && _yResolution == 0) {
		byte sizeFlag = data[5];
		if (sizeFlag == 0) {
			_xResolution = kLowResX;
			_yResolution = kLowResY;
		} else if (sizeFlag == 1) {
			_xResolution = 640;
			_yResolution = 480;
		} else if (sizeFlag == 2) {
			_xResolution = 640;
			_yResolution = 400;
		}
	}

	const uint16 loopCount = data[2];
	if (_info.loopNo >= loopCount) {
		_info.loopNo = loopCount - 1;
	}

	if (loopNo < 0) {
		error("Loop is less than 0");
	}

	const uint16 viewHeaderSize = data.getUint16SEAt(0);
	const uint8 loopHeaderSize = data[12];
	const uint8 viewHeaderFieldSize = 2;

	SciSpan<const byte> loopHeader = data.subspan(viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * _info.loopNo));

	if (loopHeader.getInt8At(0) != -1) {
		if (loopHeader[1] == 1) {
			_mirrorX = true;
		}

		loopHeader = data.subspan(viewHeaderFieldSize + viewHeaderSize + (loopHeaderSize * loopHeader.getInt8At(0)));
	}

	uint8 celCount = loopHeader[2];
	if (_info.celNo >= celCount) {
		_info.celNo = celCount - 1;
	}

	// A celNo can be negative and still valid. At least PQ4CD uses this strange
	// arrangement to load its high-resolution main menu resource. In PQ4CD, the
	// low-resolution menu is at view 23, loop 9, cel 0, and the high-resolution
	// menu is at view 2300, loop 0, cel 0. View 2300 is specially crafted to
	// have 2 loops, with the second loop having 0 cels. When in high-resolution
	// mode, the game scripts only change the view resource ID from 23 to 2300,
	// leaving loop 9 and cel 0 the same. The code in CelObjView constructor
	// auto-corrects loop 9 to loop 1, and then auto-corrects the cel number
	// from 0 to -1, which effectively causes loop 0, cel 0 to be read.
	if (_info.celNo < 0 && _info.loopNo == 0) {
		error("Cel is less than 0 on loop 0");
	}

	_hunkPaletteOffset = data.getUint32SEAt(8);
	_celHeaderOffset = loopHeader.getUint32SEAt(12) + (data[13] * _info.celNo);

	const SciSpan<const byte> celHeader = data.subspan(_celHeaderOffset);

	_width = celHeader.getUint16SEAt(0);
	_height = celHeader.getUint16SEAt(2);
	assert(_width <= kCelScalerTableSize && _height <= kCelScalerTableSize);
	_origin.x = _width / 2 - celHeader.getInt16SEAt(4);
	_origin.y = _height - celHeader.getInt16SEAt(6) - 1;
	_skipColor = celHeader[8];
	_compressionType = (CelCompressionType)celHeader[9];

	if (_compressionType != kCelCompressionNone && _compressionType != kCelCompressionRLE) {
		error("Compression type not supported - V: %d  L: %d  C: %d", _info.resourceId, _info.loopNo, _info.celNo);
	}

	const uint16 flags = celHeader.getUint16SEAt(10);
	if (flags & 0x80) {
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
	const SciSpan<const byte> data = getResPointer();
	const uint32 pixelsOffset = data.getUint32SEAt(_celHeaderOffset + 24);
	const byte *pixels = data.getUnsafeDataAt(pixelsOffset, _width * _height);
	for (int i = 0; i < _width * _height; ++i) {
		const byte pixel = pixels[i];
		if (
			pixel >= g_sci->_gfxRemap32->getStartColor() &&
			pixel <= g_sci->_gfxRemap32->getEndColor() &&
			pixel != _skipColor
		) {
			return true;
		}
	}
	return false;
}

bool CelObjView::analyzeForRemap() const {
	READER_Compressed reader(*this, _width);
	for (int y = 0; y < _height; y++) {
		const byte *const curRow = reader.getRow(y);
		for (int x = 0; x < _width; x++) {
			const byte pixel = curRow[x];
			if (
				pixel >= g_sci->_gfxRemap32->getStartColor() &&
				pixel <= g_sci->_gfxRemap32->getEndColor() &&
				pixel != _skipColor
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

const SciSpan<const byte> CelObjView::getResPointer() const {
	Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeView, _info.resourceId), false);
	if (resource == nullptr) {
		error("Failed to load view %d from resource manager", _info.resourceId);
	}
	return *resource;
}

Common::Point CelObjView::getLinkPosition(const int16 linkId) const {
	const SciSpan<const byte> resource = getResPointer();

	if (resource[18] < 0x84) {
		error("%s unsupported version %u for Links", _info.toString().c_str(), resource[18]);
	}

	const SciSpan<const byte> celHeader = resource.subspan(_celHeaderOffset);
	const int16 numLinks = celHeader.getInt16SEAt(40);

	if (numLinks) {
		const int recordSize = 6;
		SciSpan<const byte> linkTable = resource.subspan(celHeader.getInt32SEAt(36), recordSize * numLinks);
		for (int16 i = 0; i < numLinks; ++i) {
			if (linkTable[4] == linkId) {
				Common::Point point;
				point.x = linkTable.getInt16SEAt(0);
				if (_mirrorX) {
					// SSCI had an off-by-one error here (missing -1)
					point.x = _width - point.x - 1;
				}
				point.y = linkTable.getInt16SEAt(2);
				return point;
			}

			linkTable += recordSize;
		}
	}

	return Common::Point(-1, -1);
}

#pragma mark -
#pragma mark CelObjPic

CelObjPic::CelObjPic(const GuiResourceId picId, const int16 celNo) {
	_info.type = kCelTypePic;
	_info.resourceId = picId;
	_info.loopNo = 0;
	_info.celNo = celNo;
	_mirrorX = false;
	_isMacSource = (g_sci->getPlatform() == Common::kPlatformMacintosh);
	_compressionType = kCelCompressionInvalid;
	_transparent = true;
	_remap = false;

	int cacheInsertIndex;
	const int cacheIndex = searchCache(_info, &cacheInsertIndex);
	if (cacheIndex != -1) {
		CelCacheEntry &entry = (*_cache)[cacheIndex];
		const CelObjPic *const cachedCelObj = dynamic_cast<CelObjPic *>(entry.celObj.get());
		if (cachedCelObj == nullptr) {
			error("Expected a CelObjPic in cache slot %d", cacheIndex);
		}
		*this = *cachedCelObj;
		entry.id = ++_nextCacheId;
		return;
	}

	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypePic, picId), false);

	// SSCI just silently returns here
	if (!resource) {
		error("Pic resource %d not found", picId);
	}

	const Resource &data = *resource;

	_celCount = data.getUint8At(2);

	if (_info.celNo >= _celCount) {
		error("Cel number %d greater than cel count %d", _info.celNo, _celCount);
	}

	_celHeaderOffset = data.getUint16SEAt(0) + (data.getUint16SEAt(4) * _info.celNo);
	_hunkPaletteOffset = data.getUint32SEAt(6);

	const SciSpan<const byte> celHeader = data.subspan(_celHeaderOffset);

	_width = celHeader.getUint16SEAt(0);
	_height = celHeader.getUint16SEAt(2);
	_origin.x = celHeader.getInt16SEAt(4);
	_origin.y = celHeader.getInt16SEAt(6);
	_skipColor = celHeader[8];
	_compressionType = (CelCompressionType)celHeader[9];
	_priority = celHeader.getInt16SEAt(36);
	_relativePosition.x = celHeader.getInt16SEAt(38);
	_relativePosition.y = celHeader.getInt16SEAt(40);

	const uint16 sizeFlag1 = data.getUint16SEAt(10);
	const uint16 sizeFlag2 = data.getUint16SEAt(12);

	if (sizeFlag2) {
		_xResolution = sizeFlag1;
		_yResolution = sizeFlag2;
	} else if (sizeFlag1 == 0) {
		_xResolution = kLowResX;
		_yResolution = kLowResY;
	} else if (sizeFlag1 == 1) {
		_xResolution = 640;
		_yResolution = 480;
	} else if (sizeFlag1 == 2) {
		_xResolution = 640;
		_yResolution = 400;
	}


	const uint16 flags = celHeader.getUint16SEAt(10);
	if (flags & 0x80) {
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
	const SciSpan<const byte> resource = getResPointer();
	const uint32 pixelsOffset = resource.getUint32SEAt(_celHeaderOffset + 24);
	const int32 numPixels = MIN<int32>(resource.size() - pixelsOffset, _width * _height);

	if (numPixels < _width * _height) {
		warning("%s is truncated", _info.toString().c_str());
	}

	const byte *const pixels = resource.getUnsafeDataAt(pixelsOffset, numPixels);
	for (int32 i = 0; i < numPixels; ++i) {
		uint8 pixel = pixels[i];
		if (pixel == _skipColor) {
			return true;
		}
	}

	return false;
}

void CelObjPic::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, const bool mirrorX) {
	const Ratio square;
	_drawMirrored = mirrorX;
	drawTo(target, targetRect, scaledPosition, square, square);
}

CelObjPic *CelObjPic::duplicate() const {
	return new CelObjPic(*this);
}

const SciSpan<const byte> CelObjPic::getResPointer() const {
	const Resource *const resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypePic, _info.resourceId), false);
	if (resource == nullptr) {
		error("Failed to load pic %d from resource manager", _info.resourceId);
	}
	return *resource;
}

#pragma mark -
#pragma mark CelObjMem

CelObjMem::CelObjMem(const reg_t bitmapObject) {
	_info.type = kCelTypeMem;
	_info.bitmap = bitmapObject;
	_mirrorX = false;
	_isMacSource = false;
	_compressionType = kCelCompressionNone;
	_celHeaderOffset = 0;
	_transparent = true;

	SciBitmap *bitmap = g_sci->getEngineState()->_segMan->lookupBitmap(bitmapObject);

	// SSCI did no error checking here at all so would just end up reading
	// garbage or crashing if this ever happened
	if (!bitmap) {
		error("Bitmap %04x:%04x not found", PRINT_REG(bitmapObject));
	}

	_width = bitmap->getWidth();
	_height = bitmap->getHeight();
	_origin = bitmap->getOrigin();
	_skipColor = bitmap->getSkipColor();
	_xResolution = bitmap->getXResolution();
	_yResolution = bitmap->getYResolution();
	_hunkPaletteOffset = bitmap->getHunkPaletteOffset();
	_remap = bitmap->getRemap();
}

CelObjMem *CelObjMem::duplicate() const {
	return new CelObjMem(*this);
}

const SciSpan<const byte> CelObjMem::getResPointer() const {
	SciBitmap &bitmap = *g_sci->getEngineState()->_segMan->lookupBitmap(_info.bitmap);
	return SciSpan<const byte>(bitmap.getRawData(), bitmap.getRawSize(), Common::String::format("bitmap %04x:%04x", PRINT_REG(_info.bitmap)));
}

#pragma mark -
#pragma mark CelObjColor

CelObjColor::CelObjColor(const uint8 color, const int16 width, const int16 height) {
	_info.type = kCelTypeColor;
	_info.color = color;
	_origin.x = 0;
	_origin.y = 0;
	_xResolution = g_sci->_gfxFrameout->getScriptWidth();
	_yResolution = g_sci->_gfxFrameout->getScriptHeight();
	_hunkPaletteOffset = 0;
	_mirrorX = false;
	_isMacSource = (g_sci->getPlatform() == Common::kPlatformMacintosh);
	_remap = false;
	_width = width;
	_height = height;
}

void CelObjColor::draw(Buffer &target, const ScreenItem &screenItem, const Common::Rect &targetRect, const bool mirrorX) {
	// One cannot draw a solid color mirrored, but SSCI sets it anyway, so we do
	// too
	_drawMirrored = mirrorX;
	draw(target, targetRect);
}
void CelObjColor::draw(Buffer &target, const Common::Rect &targetRect, const Common::Point &scaledPosition, bool mirrorX) {
	error("Unsupported method");
}
void CelObjColor::draw(Buffer &target, const Common::Rect &targetRect) const {
	target.fillRect(targetRect, translateMacColor(_isMacSource, _info.color));
}

CelObjColor *CelObjColor::duplicate() const {
	return new CelObjColor(*this);
}

const SciSpan<const byte> CelObjColor::getResPointer() const {
	error("Unsupported method");
}
} // End of namespace Sci
