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

#include "larryScale.h"
#include "common/array.h"

namespace Graphics {

typedef LarryScaleColor Color;

const int kMargin = 2;

// A bitmap that has a margin of `kMargin` pixels all around it.
// Allows fast access without time-consuming bounds checking.
template<typename T>
class MarginedBitmap {
	int _width;
	int _height;
	int _stride;
	Common::Array<T> _buffer;
	T *_origin;

public:
	MarginedBitmap(int width, int height, T marginValue) :
		_width(width),
		_height(height),
		_stride(width + 2 * kMargin),
		_buffer(_stride * (height + 2 * kMargin)),
		_origin(calculateOrigin())
	{
		fillMargin(marginValue);
	}

	// We need a custom copy constructor.
	// Otherwise, _origin would point to the original buffer.
	MarginedBitmap(const MarginedBitmap &rhs) :
		_width(rhs._width),
		_height(rhs._height),
		_stride(rhs._stride),
		_buffer(rhs._buffer),
		_origin(calculateOrigin())
	{}

	// We need a custom assignment operator.
	// Otherwise, _origin would point to the original buffer.
	MarginedBitmap &operator =(const MarginedBitmap &rhs) {
		_width = rhs._width;
		_height = rhs._height;
		_stride = rhs._stride;
		_buffer = rhs._buffer;
		_origin = calculateOrigin();
		return this;
	}

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	int getStride() const { return _stride; }
	const T *getOrigin() const { return _origin; }
	T *getOrigin() { return _origin; }

	const T *getPointerTo(int x, int y) const {
		return _origin + y * _stride + x;
	}
	T *getPointerTo(int x, int y) {
		return _origin + y * _stride + x;
	}

	T get(int x, int y) const {
		return _origin[y * _stride + x];
	}
	void set(int x, int y, T value) {
		_origin[y * _stride + x] = value;
	}

	void fillMargin(T value);

private:
	T *calculateOrigin() {
		return _buffer.data() + kMargin * _stride + kMargin;
	}
};

template<typename T>
void MarginedBitmap<T>::fillMargin(T value) {
	T * const data = getOrigin();
	const int stride = getStride();

	// Fill top margin
	for (int y = -kMargin; y < 0; ++y) {
		for (int x = -kMargin; x < _width + kMargin; ++x) {
			data[y * stride + x] = value;
		}
	}

	// Fill sideways margins
	for (int y = 0; y < _height; ++y) {
		for (int x = -kMargin; x < 0; ++x) {
			data[y * stride + x] = value;
		}
		for (int x = _width; x < _width + kMargin; ++x) {
			data[y * stride + x] = value;
		}
	}

	// Fill bottom margin
	for (int y = _height; y < _height + kMargin; ++y) {
		for (int x = -kMargin; x < _width + kMargin; ++x) {
			data[y * stride + x] = value;
		}
	}
}

MarginedBitmap<Color> createMarginedBitmap(int width, int height, Color marginColor, RowReader &rowReader) {
	MarginedBitmap<Color> result(width, height, marginColor);
	for (int y = 0; y < height; ++y) {
		memcpy(result.getPointerTo(0, y), rowReader.readRow(y), width * sizeof(Color));
	}
	return result;
}

class MarginedBitmapWriter : public RowWriter {
	MarginedBitmap<Color> &_target;
public:
	explicit MarginedBitmapWriter(MarginedBitmap<Color> &target)
		: _target(target) {}

	void writeRow(int y, const LarryScaleColor *row) {
		memcpy(_target.getPointerTo(0, y), row, _target.getWidth() * sizeof(Color));
	}
};

inline bool isLinePixel(const MarginedBitmap<Color> &src, int x, int y) {
#define EQUALS(xOffset, yOffset) (src.get(x + xOffset, y + yOffset) == pixel)

	const Color pixel = src.get(x, y);

	// Single pixels are fills
	if (!EQUALS(-1, -1) && !EQUALS(0, -1) && !EQUALS(1, -1) && !EQUALS(1, 0) && !EQUALS(1, 1) && !EQUALS(0, 1) && !EQUALS(-1, 1) && !EQUALS(-1, 0)) {
		return false;
	}

	// 2x2 blocks are fills
	if (EQUALS(0, -1) && EQUALS(1, -1) && EQUALS(1, 0)) return false;
	if (EQUALS(1, 0) && EQUALS(1, 1) && EQUALS(0, 1)) return false;
	if (EQUALS(0, 1) && EQUALS(-1, 1) && EQUALS(-1, 0)) return false;
	if (EQUALS(-1, 0) && EQUALS(-1, -1) && EQUALS(0, -1)) return false;

	// A pixel adjacent to a 2x2 block is a fill.
	if (EQUALS(-1, -1) && EQUALS(0, -1) && EQUALS(-1, -2) && EQUALS(0, -2)) return false;
	if (EQUALS(0, -1) && EQUALS(1, -1) && EQUALS(0, -2) && EQUALS(1, -2)) return false;
	if (EQUALS(1, -1) && EQUALS(1, 0) && EQUALS(2, -1) && EQUALS(2, 0)) return false;
	if (EQUALS(1, 0) && EQUALS(1, 1) && EQUALS(2, 0) && EQUALS(2, 1)) return false;
	if (EQUALS(1, 1) && EQUALS(0, 1) && EQUALS(1, 2) && EQUALS(0, 2)) return false;
	if (EQUALS(0, 1) && EQUALS(-1, 1) && EQUALS(0, 2) && EQUALS(-1, 2)) return false;
	if (EQUALS(-1, 1) && EQUALS(-1, 0) && EQUALS(-2, 1) && EQUALS(-2, 0)) return false;
	if (EQUALS(-1, 0) && EQUALS(-1, -1) && EQUALS(-2, 0) && EQUALS(-2, -1)) return false;

	// Everything else is part of a line
	return true;

#undef EQUALS
}

MarginedBitmap<bool> createMarginedLinePixelsBitmap(const MarginedBitmap<Color> &src) {
	MarginedBitmap<bool> result(src.getWidth(), src.getHeight(), false);
	for (int y = 0; y < src.getHeight(); ++y) {
		for (int x = 0; x < src.getWidth(); ++x) {
			result.set(x, y, isLinePixel(src, x, y));
		}
	}
	return result;
}

void scaleDown(
	const MarginedBitmap<Color> &src,
	Color transparentColor,
	int dstWidth, int dstHeight,
	RowWriter &rowWriter
) {
	assert(src.getWidth() > 0);
	assert(src.getHeight() > 0);
	assert(dstWidth > 0 && dstWidth <= src.getWidth());
	assert(dstHeight > 0 && dstHeight <= src.getHeight());

	Common::Array<Color> dstRow(dstWidth);
	for (int dstY = 0; dstY < dstHeight; ++dstY) {
		const int srcY1 = dstY * src.getHeight() / dstHeight;
		const int srcY2 = (dstY + 1) * src.getHeight() / dstHeight;

		for (int dstX = 0; dstX < dstWidth; ++dstX) {
			const int srcX1 = dstX * src.getWidth() / dstWidth;
			const int srcX2 = (dstX + 1) * src.getWidth() / dstWidth;

			const int blockPixelCount = (srcX2 - srcX1) * (srcY2 - srcY1);
			if (blockPixelCount <= 4) {
				// Downscaling to 50% or more. Prefer line pixels.
				Color bestLineColor = 0;
				int linePixelCount = 0;
				for (int srcY = srcY1; srcY < srcY2; ++srcY) {
					for (int srcX = srcX1; srcX < srcX2; ++srcX) {
						const bool colorIsFromLine = isLinePixel(src, srcX, srcY);
						if (colorIsFromLine) {
							bestLineColor = src.get(srcX, srcY);
							++linePixelCount;
						}
					}
				}
				const bool sufficientLinePixels = linePixelCount * 2 >= blockPixelCount;
				const Color resultColor = sufficientLinePixels
					? bestLineColor
					: src.get(srcX1, srcY1);
				dstRow[dstX] = resultColor;
			} else {
				// Downscaling significantly. Prefer outline pixels.
				Color bestColor = src.get(srcX1, srcY1);
				for (int srcY = srcY1; srcY < srcY2; ++srcY) {
					for (int srcX = srcX1; srcX < srcX2; ++srcX) {
						const Color pixelColor = src.get(srcX, srcY);
						const bool isOutlinePixel = pixelColor != transparentColor && (
							src.get(srcX - 1, srcY) == transparentColor
							|| src.get(srcX + 1, srcY) == transparentColor
							|| src.get(srcX, srcY - 1) == transparentColor
							|| src.get(srcX, srcY + 1) == transparentColor
							);
						if (isOutlinePixel) {
							bestColor = pixelColor;
							goto foundOutlinePixel;
						}
					}
				}
			foundOutlinePixel:
				dstRow[dstX] = bestColor;
			}
		}
		rowWriter.writeRow(dstY, dstRow.data());
	}
}

// An equality matrix is a combination of eight Boolean flags indicating whether
// each of the surrounding pixels has the same color as the central pixel.
//
// +------+------+------+
// | 0x02 | 0x04 | 0x08 |
// +------+------+------+
// | 0x01 | Ref. | 0x10 |
// +------+------+------+
// | 0x80 | 0x40 | 0x20 |
// +------+------+------+
typedef byte EqualityMatrix;

EqualityMatrix getEqualityMatrix(const Color *pixel, int stride) {
#define EQUALS(x, y) (pixel[y * stride + x] == *pixel)

	return (EQUALS(-1, 0) ? 0x01 : 0x00)
		| (EQUALS(-1, -1) ? 0x02 : 0x00)
		| (EQUALS(0, -1) ? 0x04 : 0x00)
		| (EQUALS(1, -1) ? 0x08 : 0x00)
		| (EQUALS(1, 0) ? 0x10 : 0x00)
		| (EQUALS(1, 1) ? 0x20 : 0x00)
		| (EQUALS(0, 1) ? 0x40 : 0x00)
		| (EQUALS(-1, 1) ? 0x80 : 0x00);

#undef EQUALS
}

// scapeUp() requires generated functions
#include "larryScale_generated.cpp"

void scaleUp(
	const MarginedBitmap<Color> &src,
	int dstWidth, int dstHeight,
	RowWriter &rowWriter
) {
	const int srcWidth = src.getWidth();
	const int srcHeight = src.getHeight();

	assert(srcWidth > 0);
	assert(srcHeight > 0);
	assert(dstWidth >= srcWidth && dstWidth <= 2 * src.getWidth());
	assert(dstHeight >= srcHeight && dstHeight <= 2 * src.getHeight());

	const MarginedBitmap<bool> linePixels = createMarginedLinePixelsBitmap(src);
	Common::Array<Color> topDstRow(dstWidth);
	Common::Array<Color> bottomDstRow(dstWidth);
	for (int srcY = 0; srcY < src.getHeight(); ++srcY) {
		const int dstY1 = srcY * dstHeight / src.getHeight();
		const int dstY2 = (srcY + 1) * dstHeight / src.getHeight();
		const int dstBlockHeight = dstY2 - dstY1;

		for (int srcX = 0; srcX < src.getWidth(); ++srcX) {
			const int dstX1 = srcX * dstWidth / src.getWidth();
			const int dstX2 = (srcX + 1) * dstWidth / src.getWidth();
			const int dstBlockWidth = dstX2 - dstX1;

			if (dstBlockWidth == 1) {
				if (dstBlockHeight == 1) {
					// 1x1
					topDstRow[dstX1] = src.get(srcX, srcY);
				} else {
					// 1x2
					Color &top = topDstRow[dstX1];
					Color &bottom = bottomDstRow[dstX1];
					scalePixelTo1x2(src, linePixels, srcX, srcY, top, bottom);
				}
			} else {
				if (dstBlockHeight == 1) {
					// 2x1
					Color &left = topDstRow[dstX1];
					Color &right = topDstRow[dstX1 + 1];
					scalePixelTo2x1(src, linePixels, srcX, srcY, left, right);
				} else {
					// 2x2
					Color &topLeft = topDstRow[dstX1];
					Color &topRight = topDstRow[dstX1 + 1];
					Color &bottomLeft = bottomDstRow[dstX1];
					Color &bottomRight = bottomDstRow[dstX1 + 1];
					scalePixelTo2x2(src, linePixels, srcX, srcY, topLeft, topRight, bottomLeft, bottomRight);
				}
			}
		}
		rowWriter.writeRow(dstY1, topDstRow.data());
		if (dstBlockHeight == 2) {
			rowWriter.writeRow(dstY1 + 1, bottomDstRow.data());
		}
	}
}

void copyRows(int height, RowReader &rowReader, RowWriter &rowWriter) {
	for (int y = 0; y < height; ++y) {
		rowWriter.writeRow(y, rowReader.readRow(y));
	}
}

void larryScale(
	const MarginedBitmap<Color> &src,
	Color transparentColor,
	int dstWidth, int dstHeight,
	RowWriter &rowWriter
) {
	const int srcWidth = src.getWidth();
	const int srcHeight = src.getHeight();

	if (
		(dstWidth > srcWidth && dstHeight < srcHeight)		// Upscaling along x axis, downscaling along y axis
		|| (dstWidth < srcWidth && dstHeight > srcHeight)	// Downscaling along x axis, upscaling along y axis
		|| (dstWidth > 2 * srcWidth)						// Upscaling to more than 200% along x axis
		|| (dstHeight > 2 * srcHeight)						// Upscaling to more than 200% along y axis
	) {
		// We can't handle these cases with a single upscale.
		// Let's do an intermediate scale.
		const int tmpWidth = CLIP(dstWidth, srcWidth, 2 * srcWidth);
		const int tmpHeight = CLIP(dstHeight, srcHeight, 2 * srcHeight);
		MarginedBitmap<Color> tmp(tmpWidth, tmpHeight, transparentColor);
		MarginedBitmapWriter writer = MarginedBitmapWriter(tmp);
		larryScale(src, transparentColor, tmpWidth, tmpHeight, writer);
		larryScale(tmp, transparentColor, dstWidth, dstHeight, rowWriter);
	} else if (dstWidth > srcWidth || dstHeight > srcHeight) {
		// Upscaling to no more than 200%
		scaleUp(src, dstWidth, dstHeight, rowWriter);
	} else {
		// Downscaling
		scaleDown(src, transparentColor, dstWidth, dstHeight, rowWriter);
	}
}

void larryScale(
	int srcWidth, int srcHeight,
	Color transparentColor,
	RowReader &rowReader,
	int dstWidth, int dstHeight,
	RowWriter &rowWriter
) {
	// Select the appropriate scaler
	if (srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0) {
		// Nothing to do
	} else if (dstWidth == srcWidth && dstHeight == srcHeight) {
		copyRows(srcHeight, rowReader, rowWriter);
	} else {
		const MarginedBitmap<Color> src =
			createMarginedBitmap(srcWidth, srcHeight, transparentColor, rowReader);
		larryScale(src, transparentColor, dstWidth, dstHeight, rowWriter);
	}
}

}
