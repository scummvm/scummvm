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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/fonts/freetype.h"
#include "graphics/surface.h"

#include <math.h>

#include "harold/bcb/text_renderer.h"

#ifdef USE_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#if __has_include(<hb.h>) && __has_include(<hb-ft.h>)
#include <hb-ft.h>
#include <hb.h>
#define BCB_USE_HARFBUZZ_TEXT_LAYOUT
#endif
#endif

namespace BCB {
namespace {

static inline uint pillowShiftForDiv255(uint value) {
	return (((value >> 8) + value) >> 8);
}

static inline uint pillowDiv255(uint value) {
	return pillowShiftForDiv255(value + 128);
}

static inline uint pillowMulDiv255(uint a, uint b) {
	return pillowShiftForDiv255(a * b + 128);
}

static inline uint pillowBlend(uint mask, uint destination, uint source) {
	return pillowDiv255(destination * (255 - mask) + source * mask);
}

#ifdef USE_FREETYPE2
static int ftPixel(FT_Pos value) {
	return (int)((value + 32) >> 6);
}

bool loadFontArchiveMember(const Common::String &fontName, const char *archiveName, Common::Array<byte> &fontData) {
	Common::SeekableReadStream *archiveStream = nullptr;
	if (ConfMan.hasKey("extrapath")) {
		Common::FSDirectory extrapath(ConfMan.getPath("extrapath"));
		archiveStream = extrapath.createReadStreamForMember(Common::Path(archiveName, Common::Path::kNoSeparator));
	}

	if (!archiveStream)
		archiveStream = SearchMan.createReadStreamForMember(Common::Path(archiveName, Common::Path::kNoSeparator));

	Common::ScopedPtr<Common::Archive> archive(Common::makeZipArchive(archiveStream));
	if (!archive)
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(archive->createReadStreamForMember(Common::Path(fontName, Common::Path::kNoSeparator)));
	if (!stream)
		return false;

	fontData.resize(stream->size());
	return stream->read(fontData.data(), fontData.size()) == fontData.size();
}

bool loadFontDataFromArchives(const Common::String &fontName, Common::Array<byte> &fontData) {
	return loadFontArchiveMember(fontName, "fonts.dat", fontData) || loadFontArchiveMember(fontName, "fonts-cjk.dat", fontData);
}

struct PillowGlyphInfo {
	FT_UInt index;
	FT_Pos xAdvance;
	FT_Pos xOffset;
};

#ifdef BCB_USE_HARFBUZZ_TEXT_LAYOUT
bool layoutPillowGlyphsWithHarfBuzz(FT_Face face, const Common::U32String &text, Common::Array<PillowGlyphInfo> &glyphs) {
	glyphs.clear();
	if (text.empty())
		return true;

	hb_font_t *font = hb_ft_font_create_referenced(face);
	if (!font)
		return false;
	hb_buffer_t *buffer = hb_buffer_create();
	if (!buffer) {
		hb_font_destroy(font);
		return false;
	}

	Common::Array<uint32> codepoints;
	codepoints.resize(text.size());
	for (uint i = 0; i < text.size(); ++i)
		codepoints[i] = text[i];

	hb_buffer_add_utf32(buffer, codepoints.data(), codepoints.size(), 0, codepoints.size());
	hb_buffer_guess_segment_properties(buffer);
	hb_shape(font, buffer, nullptr, 0);

	uint glyphCount = 0;
	hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
	hb_glyph_position_t *glyphPosition = hb_buffer_get_glyph_positions(buffer, &glyphCount);
	if (!glyphInfo || !glyphPosition) {
		hb_buffer_destroy(buffer);
		hb_font_destroy(font);
		return false;
	}

	for (uint i = 0; i < glyphCount; ++i) {
		PillowGlyphInfo glyph;
		glyph.index = glyphInfo[i].codepoint;
		glyph.xOffset = glyphPosition[i].x_offset;
		glyph.xAdvance = glyphPosition[i].x_advance;
		glyphs.push_back(glyph);
	}

	hb_buffer_destroy(buffer);
	hb_font_destroy(font);
	return true;
}
#endif

bool layoutPillowGlyphs(FT_Face face, const Common::U32String &text, Common::Array<PillowGlyphInfo> &glyphs) {
#ifdef BCB_USE_HARFBUZZ_TEXT_LAYOUT
	if (layoutPillowGlyphsWithHarfBuzz(face, text, glyphs))
		return true;
#endif

	glyphs.clear();
	FT_UInt previousIndex = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const FT_UInt glyphIndex = Graphics::FreeType::Get_Char_Index(face, text[i]);
		if (!glyphIndex)
			continue;

		FT_Vector kerning;
		FT_Pos xOffset = 0;
		if (previousIndex && FT_HAS_KERNING(face) && !Graphics::FreeType::Get_Kerning(face, previousIndex, glyphIndex, FT_KERNING_DEFAULT, &kerning))
			xOffset = kerning.x;

		if (Graphics::FreeType::Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
			return false;

		PillowGlyphInfo glyph;
		glyph.index = glyphIndex;
		glyph.xOffset = xOffset;
		glyph.xAdvance = face->glyph->advance.x;
		glyphs.push_back(glyph);
		previousIndex = glyphIndex;
	}

	return true;
}

bool getPillowTextMaskMetrics(FT_Face face, const Common::Array<PillowGlyphInfo> &glyphs, int strokeWidth,
							  int &xOffset, int &yOffset, int &xMin, int &yMax, int &maskWidth, int &maskHeight, int loadFlags) {
	int bboxXMin = 0;
	int bboxXMax = 0;
	int yMin = 0;
	int bboxYMax = 0;
	FT_Pos penX = 0;

	for (uint i = 0; i < glyphs.size(); ++i) {
		const int px = ftPixel(penX + glyphs[i].xOffset);
		penX += glyphs[i].xOffset + glyphs[i].xAdvance;
		const int advanced = ftPixel(penX);
		if (advanced > bboxXMax)
			bboxXMax = advanced;

		if (Graphics::FreeType::Load_Glyph(face, glyphs[i].index, loadFlags))
			return false;

		FT_Glyph glyph;
		if (Graphics::FreeType::Get_Glyph(face->glyph, &glyph))
			return false;

		FT_BBox bbox;
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
		Graphics::FreeType::Done_Glyph(glyph);

		bboxXMin = MIN<int>(bboxXMin, bbox.xMin + px);
		bboxXMax = MAX<int>(bboxXMax, bbox.xMax + px);
		yMin = MIN<int>(yMin, bbox.yMin);
		bboxYMax = MAX<int>(bboxYMax, bbox.yMax);
	}

	if (glyphs.empty()) {
		xOffset = 0;
		yOffset = 0;
		xMin = 0;
		yMax = 0;
		maskWidth = 0;
		maskHeight = 0;
		return true;
	}

	const int ascender = ftPixel(face->size->metrics.ascender);
	xMin = bboxXMin;
	yMax = bboxYMax;
	maskWidth = bboxXMax - bboxXMin + strokeWidth * 2;
	maskHeight = bboxYMax - yMin + strokeWidth * 2;
	xOffset = (int)round((double)bboxXMin - strokeWidth);
	yOffset = (int)round((double)(ascender - bboxYMax) - strokeWidth);
	return true;
}

void renderPillowGlyphBitmapToSurface(Graphics::Surface &surface, const FT_Bitmap &bitmap, int destX, int destY, uint32 color) {
	uint8 inkA, inkR, inkG, inkB;
	surface.format.colorToARGB(color, inkA, inkR, inkG, inkB);

	for (uint row = 0; row < bitmap.rows; ++row) {
		const byte *source = bitmap.buffer + row * bitmap.pitch;
		const int y = destY + row;
		if (y < 0 || y >= surface.h)
			continue;
		byte *targetRow = (byte *)surface.getBasePtr(0, y);
		for (uint col = 0; col < bitmap.width; ++col) {
			const int x = destX + col;
			if (x < 0 || x >= surface.w)
				continue;

			const uint sourceAlpha = source[col];
			if (sourceAlpha == 0)
				continue;

			byte *target = targetRow + x * 4;
			if (target[3] > 0) {
				target[0] = pillowBlend(sourceAlpha, target[0], inkR);
				target[1] = pillowBlend(sourceAlpha, target[1], inkG);
				target[2] = pillowBlend(sourceAlpha, target[2], inkB);
				target[3] = CLIP<uint>(sourceAlpha + pillowMulDiv255(target[3], 255 - sourceAlpha), 0, 255);
			} else {
				target[0] = inkR;
				target[1] = inkG;
				target[2] = inkB;
				target[3] = sourceAlpha;
			}
		}
	}
}

bool drawFreeTypeTextPass(Graphics::Surface &surface, FT_Face face, const Common::Array<PillowGlyphInfo> &glyphs,
						  int x, int y, uint32 color, int strokeWidth, bool strokeOnly) {
	const int loadFlags = strokeOnly ? FT_LOAD_NO_BITMAP : (FT_LOAD_DEFAULT | FT_LOAD_RENDER);
	int xOffset = 0;
	int yOffset = 0;
	int xMin = 0;
	int yMax = 0;
	int maskWidth = 0;
	int maskHeight = 0;
	if (!getPillowTextMaskMetrics(face, glyphs, strokeOnly ? strokeWidth : 0, xOffset, yOffset, xMin, yMax, maskWidth, maskHeight, strokeOnly ? FT_LOAD_NO_BITMAP : FT_LOAD_DEFAULT))
		return false;
	if (maskWidth == 0 || maskHeight == 0)
		return true;

	FT_Stroker stroker = nullptr;
	if (strokeOnly) {
		if (FT_Stroker_New(face->glyph->library, &stroker))
			return false;
		FT_Stroker_Set(stroker, (FT_Fixed)round(strokeWidth * 64.0), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	FT_Pos penX = 0;
	const FT_Pos originX = (FT_Pos)round((-xMin + (strokeOnly ? strokeWidth : 0)) * 64.0);
	const FT_Pos originY = (FT_Pos)round((-yMax - (strokeOnly ? strokeWidth : 0)) * 64.0);
	for (uint i = 0; i < glyphs.size(); ++i) {
		const int px = ftPixel(originX + penX + glyphs[i].xOffset);
		const int py = ftPixel(originY);
		if (Graphics::FreeType::Load_Glyph(face, glyphs[i].index, loadFlags)) {
			if (stroker)
				FT_Stroker_Done(stroker);
			return false;
		}

		if (strokeOnly) {
			FT_Glyph glyph;
			if (Graphics::FreeType::Get_Glyph(face->glyph, &glyph)) {
				FT_Stroker_Done(stroker);
				return false;
			}
			if (FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1) ||
				Graphics::FreeType::Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1)) {
				Graphics::FreeType::Done_Glyph(glyph);
				FT_Stroker_Done(stroker);
				return false;
			}
			FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
			const int destX = x + xOffset + px + bitmapGlyph->left;
			const int destY = y + yOffset - (py + bitmapGlyph->top);
			renderPillowGlyphBitmapToSurface(surface, bitmapGlyph->bitmap, destX, destY, color);
			Graphics::FreeType::Done_Glyph(glyph);
		} else {
			const FT_GlyphSlot slot = face->glyph;
			const int destX = x + xOffset + px + slot->bitmap_left;
			const int destY = y + yOffset - (py + slot->bitmap_top);
			renderPillowGlyphBitmapToSurface(surface, slot->bitmap, destX, destY, color);
		}

		penX += glyphs[i].xOffset + glyphs[i].xAdvance;
	}

	if (stroker)
		FT_Stroker_Done(stroker);
	return true;
}

bool drawFreeTypeTextLine(Graphics::Surface &surface, const Common::String &fontName, int size,
						  const Common::Array<Common::Point> &shadowOffsets, const Common::U32String &text,
						  int x, int y, uint32 textColor, uint32 shadowColor, int strokeWidth) {
	Common::Array<byte> fontData;
	if (!loadFontDataFromArchives(fontName, fontData))
		return false;

	FT_Library library = nullptr;
	if (Graphics::FreeType::Init_FreeType(&library))
		return false;

	FT_Face face = nullptr;
	bool ok = !Graphics::FreeType::New_Memory_Face(library, fontData.data(), fontData.size(), 0, &face);
	if (ok) {
		FT_Size_RequestRec request;
		memset(&request, 0, sizeof(request));
		request.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
		request.width = size * 64;
		request.height = size * 64;
		ok = !FT_Request_Size(face, &request);
	}

	Common::Array<PillowGlyphInfo> glyphs;
	if (ok)
		ok = layoutPillowGlyphs(face, text, glyphs);

	if (ok) {
		for (uint i = 0; i < shadowOffsets.size(); ++i)
			ok = drawFreeTypeTextPass(surface, face, glyphs, x + shadowOffsets[i].x, y + shadowOffsets[i].y, shadowColor, 0, false) && ok;
		if (strokeWidth > 0)
			ok = drawFreeTypeTextPass(surface, face, glyphs, x, y, shadowColor, strokeWidth, true) && ok;
		ok = drawFreeTypeTextPass(surface, face, glyphs, x, y, textColor, 0, false) && ok;
	}

	if (face)
		Graphics::FreeType::Done_Face(face);
	Graphics::FreeType::Done_FreeType(library);
	return ok;
}
#endif

void alphaCompositeRgbaLayerInternal(Graphics::Surface &target, const Graphics::Surface &layer) {
	const int width = MIN<int>(target.w, layer.w);
	const int height = MIN<int>(target.h, layer.h);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint8 srcA, srcR, srcG, srcB;
			layer.format.colorToARGB(layer.getPixel(x, y), srcA, srcR, srcG, srcB);
			if (srcA == 0)
				continue;

			uint8 dstA, dstR, dstG, dstB;
			target.format.colorToARGB(target.getPixel(x, y), dstA, dstR, dstG, dstB);
			const uint32 blend = dstA * (255 - srcA);
			const uint32 outA255 = srcA * 255 + blend;
			const uint32 coef1 = srcA * 255 * 255 * 128 / outA255;
			const uint32 coef2 = 255 * 128 - coef1;
			const uint8 outR = pillowShiftForDiv255(srcR * coef1 + dstR * coef2 + (0x80 << 7)) >> 7;
			const uint8 outG = pillowShiftForDiv255(srcG * coef1 + dstG * coef2 + (0x80 << 7)) >> 7;
			const uint8 outB = pillowShiftForDiv255(srcB * coef1 + dstB * coef2 + (0x80 << 7)) >> 7;
			const uint8 outA = pillowShiftForDiv255(outA255 + 0x80);
			target.setPixel(x, y, target.format.ARGBToColor(outA, outR, outG, outB));
		}
	}
}

static double cubicWeight(double value) {
	value = fabs(value);
	if (value < 1.0)
		return (1.5 * value - 2.5) * value * value + 1.0;
	if (value < 2.0)
		return (((value - 5.0) * value + 8.0) * value - 4.0) * -0.5;
	return 0.0;
}

static const int kPillowResamplePrecisionBits = 22;

static uint8 pillowResampleClip8(int value) {
	value >>= kPillowResamplePrecisionBits;
	return CLIP<int>(value, 0, 255);
}

int precomputePillowCoefficients(int inputSize, float inputStart, float inputEnd, int outputSize,
								 Common::Array<int> &bounds, Common::Array<int> &coefficients) {
	const double scale = (double)(inputEnd - inputStart) / outputSize;
	const double filterScale = MAX<double>(scale, 1.0);
	const double support = 2.0 * filterScale;
	const int kernelSize = (int)ceil(support) * 2 + 1;

	bounds.resize(outputSize * 2);
	Common::Array<double> precomputed;
	precomputed.resize(outputSize * kernelSize);
	const double invFilterScale = 1.0 / filterScale;
	for (int outX = 0; outX < outputSize; ++outX) {
		const double center = inputStart + (outX + 0.5) * scale;
		int xmin = (int)(center - support + 0.5);
		if (xmin < 0)
			xmin = 0;
		int xmax = (int)(center + support + 0.5);
		if (xmax > inputSize)
			xmax = inputSize;
		xmax -= xmin;

		double weightSum = 0.0;
		for (int x = 0; x < xmax; ++x) {
			const double weight = cubicWeight((x + xmin - center + 0.5) * invFilterScale);
			precomputed[outX * kernelSize + x] = weight;
			weightSum += weight;
		}
		if (weightSum != 0.0) {
			for (int x = 0; x < xmax; ++x)
				precomputed[outX * kernelSize + x] /= weightSum;
		}
		for (int x = xmax; x < kernelSize; ++x)
			precomputed[outX * kernelSize + x] = 0.0;

		bounds[outX * 2 + 0] = xmin;
		bounds[outX * 2 + 1] = xmax;
	}

	coefficients.resize(outputSize * kernelSize);
	for (int i = 0; i < outputSize * kernelSize; ++i) {
		if (precomputed[i] < 0.0)
			coefficients[i] = (int)(-0.5 + precomputed[i] * (1 << kPillowResamplePrecisionBits));
		else
			coefficients[i] = (int)(0.5 + precomputed[i] * (1 << kPillowResamplePrecisionBits));
	}

	return kernelSize;
}

static inline void premultiplyPillowRgba(uint8 a, uint8 &r, uint8 &g, uint8 &b) {
	r = pillowMulDiv255(r, a);
	g = pillowMulDiv255(g, a);
	b = pillowMulDiv255(b, a);
}

static inline void unpremultiplyPillowRgba(uint8 a, uint8 &r, uint8 &g, uint8 &b) {
	if (a == 0 || a == 0xff)
		return;

	r = CLIP<int>((255 * r) / a, 0, 255);
	g = CLIP<int>((255 * g) / a, 0, 255);
	b = CLIP<int>((255 * b) / a, 0, 255);
}

Common::Rect findSurfaceAlphaBounds(const Graphics::Surface &surface) {
	Common::Rect bounds;
	bool found = false;
	for (int y = 0; y < surface.h; ++y) {
		const byte *row = (const byte *)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; ++x) {
			if (row[x * 4 + 3] == 0)
				continue;

			if (!found) {
				bounds.left = x;
				bounds.right = x + 1;
				bounds.top = y;
				bounds.bottom = y + 1;
				found = true;
			} else {
				if (x < bounds.left)
					bounds.left = x;
				if (x + 1 > bounds.right)
					bounds.right = x + 1;
				if (y < bounds.top)
					bounds.top = y;
				if (y + 1 > bounds.bottom)
					bounds.bottom = y + 1;
			}
		}
	}

	return found ? bounds : Common::Rect();
}

void findOutputBoundsForSourceBounds(const Common::Array<int> &bounds, int outputSize, const Common::Rect &sourceBounds,
									 bool horizontal, int &first, int &last) {
	first = outputSize;
	last = 0;
	const int sourceMin = horizontal ? sourceBounds.left : sourceBounds.top;
	const int sourceMax = horizontal ? sourceBounds.right : sourceBounds.bottom;
	for (int out = 0; out < outputSize; ++out) {
		const int inputMin = bounds[out * 2 + 0];
		const int inputMax = inputMin + bounds[out * 2 + 1];
		if (inputMin >= sourceMax || inputMax <= sourceMin)
			continue;

		first = MIN(first, out);
		last = MAX(last, out + 1);
	}
}

Graphics::Surface *downscaleRgbaLayerBicubicInternal(const Graphics::Surface &source, int width, int height) {
	Graphics::Surface *target = new Graphics::Surface();
	target->create(width, height, source.format);
	target->fillRect(Common::Rect(width, height), 0);

	const Common::Rect sourceAlphaBounds = findSurfaceAlphaBounds(source);
	if (sourceAlphaBounds.isEmpty())
		return target;

	Common::Array<int> horizontalBounds;
	Common::Array<int> horizontalCoefficients;
	const int horizontalKernelSize = precomputePillowCoefficients(source.w, 0.0f, (float)source.w, width,
																  horizontalBounds, horizontalCoefficients);
	Common::Array<int> verticalBounds;
	Common::Array<int> verticalCoefficients;
	const int verticalKernelSize = precomputePillowCoefficients(source.h, 0.0f, (float)source.h, height,
																verticalBounds, verticalCoefficients);

	int firstOutX = 0;
	int lastOutX = 0;
	findOutputBoundsForSourceBounds(horizontalBounds, width, sourceAlphaBounds, true, firstOutX, lastOutX);
	int firstOutY = 0;
	int lastOutY = 0;
	findOutputBoundsForSourceBounds(verticalBounds, height, sourceAlphaBounds, false, firstOutY, lastOutY);
	if (firstOutX >= lastOutX || firstOutY >= lastOutY)
		return target;

	Graphics::Surface temp;
	temp.create(width, source.h, source.format);
	temp.fillRect(Common::Rect(width, source.h), 0);
	for (int y = sourceAlphaBounds.top; y < sourceAlphaBounds.bottom; ++y) {
		const byte *sourceRow = (const byte *)source.getBasePtr(0, y);
		byte *tempRow = (byte *)temp.getBasePtr(0, y);
		for (int outX = firstOutX; outX < lastOutX; ++outX) {
			const int xmin = horizontalBounds[outX * 2 + 0];
			const int xmax = horizontalBounds[outX * 2 + 1];
			const int *kernel = &horizontalCoefficients[outX * horizontalKernelSize];
			int sumA = 1 << (kPillowResamplePrecisionBits - 1);
			int sumR = 1 << (kPillowResamplePrecisionBits - 1);
			int sumG = 1 << (kPillowResamplePrecisionBits - 1);
			int sumB = 1 << (kPillowResamplePrecisionBits - 1);
			for (int x = 0; x < xmax; ++x) {
				const byte *sourcePixel = sourceRow + (x + xmin) * 4;
				uint8 r = sourcePixel[0];
				uint8 g = sourcePixel[1];
				uint8 b = sourcePixel[2];
				const uint8 a = sourcePixel[3];
				premultiplyPillowRgba(a, r, g, b);
				sumR += r * kernel[x];
				sumG += g * kernel[x];
				sumB += b * kernel[x];
				sumA += a * kernel[x];
			}
			byte *tempPixel = tempRow + outX * 4;
			tempPixel[0] = pillowResampleClip8(sumR);
			tempPixel[1] = pillowResampleClip8(sumG);
			tempPixel[2] = pillowResampleClip8(sumB);
			tempPixel[3] = pillowResampleClip8(sumA);
		}
	}

	for (int outY = firstOutY; outY < lastOutY; ++outY) {
		const int ymin = verticalBounds[outY * 2 + 0];
		const int ymax = verticalBounds[outY * 2 + 1];
		const int *kernel = &verticalCoefficients[outY * verticalKernelSize];
		byte *targetRow = (byte *)target->getBasePtr(0, outY);
		for (int x = firstOutX; x < lastOutX; ++x) {
			int sumA = 1 << (kPillowResamplePrecisionBits - 1);
			int sumR = 1 << (kPillowResamplePrecisionBits - 1);
			int sumG = 1 << (kPillowResamplePrecisionBits - 1);
			int sumB = 1 << (kPillowResamplePrecisionBits - 1);
			for (int y = 0; y < ymax; ++y) {
				const byte *tempPixel = (const byte *)temp.getBasePtr(x, y + ymin);
				const uint8 r = tempPixel[0];
				const uint8 g = tempPixel[1];
				const uint8 b = tempPixel[2];
				const uint8 a = tempPixel[3];
				sumR += r * kernel[y];
				sumG += g * kernel[y];
				sumB += b * kernel[y];
				sumA += a * kernel[y];
			}
			uint8 a = pillowResampleClip8(sumA);
			uint8 r = pillowResampleClip8(sumR);
			uint8 g = pillowResampleClip8(sumG);
			uint8 b = pillowResampleClip8(sumB);
			unpremultiplyPillowRgba(a, r, g, b);
			byte *targetPixel = targetRow + x * 4;
			targetPixel[0] = r;
			targetPixel[1] = g;
			targetPixel[2] = b;
			targetPixel[3] = a;
		}
	}

	temp.free();
	return target;
}

} // End of anonymous namespace

Graphics::Surface *downscaleRgbaLayerBicubic(const Graphics::Surface &source, int width, int height) {
	return downscaleRgbaLayerBicubicInternal(source, width, height);
}

void alphaCompositeRgbaLayer(Graphics::Surface &target, const Graphics::Surface &layer) {
	alphaCompositeRgbaLayerInternal(target, layer);
}

void drawHighQualityTextLine(Graphics::Surface &surface, const Common::String &fontName, int size,
							 const Common::Array<Common::Point> &shadowOffsets, const Common::U32String &text,
							 int x, int y, uint32 textColor, uint32 shadowColor, int strokeWidth) {
#ifdef USE_FREETYPE2
	drawFreeTypeTextLine(surface, fontName, size, shadowOffsets, text, x, y, textColor, shadowColor, strokeWidth);
#endif
}

} // End of namespace BCB
