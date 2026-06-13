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

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"
#include "common/util.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "private/paper.h"

namespace Private {

const int kPaperScanTransparentBackgroundLuma = 180;

// Nearest palette color in YCbCr space among the allowed entries. Chroma
// errors are weighted heavier than luminance ones: a wrong brightness just
// looks like paper grain, while a wrong hue stands out as colored speckle.
byte findNearestUsedColor(const byte *luma, const byte *cb, const byte *cr,
		const Common::Array<byte> &allowedColors, int targetY, int targetCb, int targetCr) {
	byte bestColor = allowedColors[0];
	int bestDistance = 0x7fffffff;

	for (uint i = 0; i < allowedColors.size(); i++) {
		byte color = allowedColors[i];
		int dy = luma[color] - targetY;
		int dcb = cb[color] - targetCb;
		int dcr = cr[color] - targetCr;
		int distance = 2 * dy * dy + 5 * (dcb * dcb + dcr * dcr);

		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = color;
		}
	}

	return bestColor;
}

void blurPlane(const Graphics::Surface &image, byte transparentColor, const byte *src, byte *dst, byte *tmp) {
	const int kernel[5] = {1, 14, 34, 14, 1};
	const int width = image.w;
	const int height = image.h;

	for (int y = 0; y < height; y++) {
		const byte *row = (const byte *)image.getBasePtr(0, y);
		const byte *srcRow = src + y * width;
		byte *tmpRow = tmp + y * width;
		for (int x = 0; x < width; x++) {
			int center = srcRow[x];
			int sum = 0;
			for (int k = -2; k <= 2; k++) {
				int xx = x + k;
				bool outside = xx < 0 || xx >= width || row[xx] == transparentColor;
				sum += kernel[k + 2] * (outside ? center : srcRow[xx]);
			}
			tmpRow[x] = (sum + 32) >> 6;
		}
	}

	for (int y = 0; y < height; y++) {
		const byte *tmpRow = tmp + y * width;
		byte *dstRow = dst + y * width;
		for (int x = 0; x < width; x++) {
			int center = tmpRow[x];
			int sum = 0;
			for (int k = -2; k <= 2; k++) {
				int yy = y + k;
				bool outside = yy < 0 || yy >= height;
				if (!outside) {
					const byte *row = (const byte *)image.getBasePtr(0, yy);
					outside = row[x] == transparentColor;
				}
				sum += kernel[k + 2] * (outside ? center : tmp[yy * width + x]);
			}
			dstRow[x] = (sum + 32) >> 6;
		}
	}
}

bool isPaperScanImage(const Common::Path &path) {
	Common::String pathString = path.toString('/');
	pathString.toLowercase();

	size_t fileNamePos = pathString.findLastOf('/');
	Common::String fileName = (fileNamePos == Common::String::npos) ?
		pathString : pathString.substr(fileNamePos + 1);

	if (!fileName.hasSuffix(".bmp"))
		return false;

	if (fileName.contains("mask") || fileName.contains("msk") || fileName.contains("mk"))
		return false;

	if (pathString.contains("inface/dossiers/") && fileName.hasPrefix("filec"))
		return true;

	if (!pathString.contains("/search_s/"))
		return false;

	return fileName.contains("nwsc") ||
	       fileName.contains("newsc") ||
	       fileName.contains("magc") ||
	       fileName.contains("formcu") ||
	       fileName.contains("ltrcu") ||
	       fileName.contains("liccu") ||
	       fileName.contains("billcu") ||
	       fileName.contains("bilcu");
}

// Readability enhancement for document close-ups (newspapers, magazines,
// letters): stretch the luminance histogram so it spans the full range
// (clipping 1% of pixels at each end) and apply a gentle unsharp mask.
//
// The paper in these scans is dithered with alternately tinted entries
// that read as colored speckle once the contrast is raised, so the
// chroma is smoothed over a small neighborhood and every pixel is mapped
// to the palette entry nearest to (new luminance, smoothed chroma),
// which collapses the dither into a uniform paper tint.
//
// The palette itself is never modified, and pixels are only remapped to
// palette indices the image already uses: the palette installed when the
// image is drawn is not necessarily this image's own (scene images share
// index conventions), so an index this image never referenced may show
// up as an arbitrary color on screen even though it looks correct in the
// image's own palette.
bool enhancePaperScanImage(Graphics::Surface *image, const byte *palette, byte transparentColor) {
	if (image->format.bytesPerPixel != 1)
		return false;

	const int width = image->w;
	const int height = image->h;

	byte luma[Graphics::PALETTE_COUNT], chromaB[Graphics::PALETTE_COUNT], chromaR[Graphics::PALETTE_COUNT];
	for (uint32 color = 0; color < Graphics::PALETTE_COUNT; color++) {
		int r = palette[3 * color + 0];
		int g = palette[3 * color + 1];
		int b = palette[3 * color + 2];
		luma[color] = (77 * r + 150 * g + 29 * b) >> 8;
		chromaB[color] = (byte)(128 + (-43 * r - 85 * g + 128 * b) / 256);
		chromaR[color] = (byte)(128 + (128 * r - 107 * g - 21 * b) / 256);
	}

	uint32 histogram[Graphics::PALETTE_COUNT] = {};
	bool used[Graphics::PALETTE_COUNT] = {};
	uint32 opaquePixelCount = 0;
	uint32 histogramPixelCount = 0;

	for (int y = 0; y < height; y++) {
		const byte *row = (const byte *)image->getBasePtr(0, y);
		for (int x = 0; x < width; x++) {
			byte color = row[x];
			if (color == transparentColor) {
				histogram[kPaperScanTransparentBackgroundLuma]++;
				histogramPixelCount++;
				continue;
			}

			histogram[luma[color]]++;
			used[color] = true;
			opaquePixelCount++;
			histogramPixelCount++;
		}
	}

	if (opaquePixelCount < (uint32)(width * height / 30))
		return false;

	Common::Array<byte> allowedColors;
	allowedColors.reserve(Graphics::PALETTE_COUNT);
	for (uint32 color = 0; color < Graphics::PALETTE_COUNT; color++) {
		if (used[color] && color != transparentColor)
			allowedColors.push_back(color);
	}
	if (allowedColors.empty())
		return false;

	uint32 cutoff = histogramPixelCount / 100;
	uint32 accumulated = 0;
	int lowCut = 0;
	while (lowCut < 255 && accumulated + histogram[lowCut] <= cutoff)
		accumulated += histogram[lowCut++];

	accumulated = 0;
	int highCut = 255;
	while (highCut > lowCut && accumulated + histogram[highCut] <= cutoff)
		accumulated += histogram[highCut--];

	if (highCut <= lowCut) {
		lowCut = 0;
		highCut = 255;
	}

	byte stretched[Graphics::PALETTE_COUNT];
	for (int level = 0; level < Graphics::PALETTE_COUNT; level++)
		stretched[level] = (byte)CLIP((level - lowCut) * 255 / (highCut - lowCut), 0, 255);

	const uint32 planeSize = width * height;
	Common::Array<byte> yPlane(planeSize), yBlur(planeSize);
	Common::Array<byte> cbPlane(planeSize), crPlane(planeSize);
	Common::Array<byte> scratch(planeSize);
	const uint32 cacheSize = Graphics::PALETTE_COUNT * 64 * 64;
	Common::Array<int16> nearestCache(cacheSize, -1);

	for (int y = 0; y < height; y++) {
		const byte *row = (const byte *)image->getBasePtr(0, y);
		byte *yRow = yPlane.data() + y * width;
		byte *cbRow = cbPlane.data() + y * width;
		byte *crRow = crPlane.data() + y * width;
		for (int x = 0; x < width; x++) {
			byte color = row[x];
			yRow[x] = stretched[luma[color]];
			cbRow[x] = chromaB[color];
			crRow[x] = chromaR[color];
		}
	}

	blurPlane(*image, transparentColor, yPlane.data(), yBlur.data(), scratch.data());
	blurPlane(*image, transparentColor, cbPlane.data(), cbPlane.data(), scratch.data());
	blurPlane(*image, transparentColor, cbPlane.data(), cbPlane.data(), scratch.data());
	blurPlane(*image, transparentColor, crPlane.data(), crPlane.data(), scratch.data());
	blurPlane(*image, transparentColor, crPlane.data(), crPlane.data(), scratch.data());

	for (int y = 0; y < height; y++) {
		byte *row = (byte *)image->getBasePtr(0, y);
		const byte *yRow = yPlane.data() + y * width;
		const byte *blurRow = yBlur.data() + y * width;
		const byte *cbRow = cbPlane.data() + y * width;
		const byte *crRow = crPlane.data() + y * width;
		for (int x = 0; x < width; x++) {
			byte color = row[x];
			if (color == transparentColor)
				continue;

			int value = yRow[x];
			int diff = value - blurRow[x];
			if (diff > 2 || diff < -2)
				value += diff * 120 / 100;
			value = CLIP(value, 0, 255);

			int targetCb = (cbRow[x] & ~3) + 2;
			int targetCr = (crRow[x] & ~3) + 2;

			uint32 cacheKey = (value << 12) | ((cbRow[x] >> 2) << 6) | (crRow[x] >> 2);
			if (nearestCache[cacheKey] < 0)
				nearestCache[cacheKey] = findNearestUsedColor(luma, chromaB, chromaR,
					allowedColors, value, targetCb, targetCr);
			row[x] = (byte)nearestCache[cacheKey];
		}
	}

	return true;
}

} // End of namespace Private
