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
 */

#include <cstring>

#include "graphics/scaler/xbrz.h"
#include "graphics/scaler/xbrz/xbrz.h"

void xBRZScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {

	xbrz::ScalerCfg cfg;
	int BytesPerPixel = sizeof(uint32_t);
	uint32_t *src = (uint32_t *)srcPtr;
	uint32_t *dst = (uint32_t *)dstPtr;

	int srcLineCount = height;
	uint32 srcWidth = width;
	uint32 srcWidthInByte = BytesPerPixel * srcWidth;
	if (srcPitch > srcWidthInByte) {
		src =  (uint32_t *)malloc(srcWidthInByte * srcLineCount);
		uint32_t *tmp = src;
		for (int i = 0; i < height; ++i) {
			int lineOffset = i * srcPitch;
			const uint8 *copyStart = srcPtr + lineOffset;
			std::memcpy(tmp, copyStart, srcWidthInByte);
			tmp += srcWidth;
		}
	}

	int dstLineCount = _factor * srcLineCount;
	uint32 dstWidth = _factor * srcWidth;
	uint32 dstWidthInByte = BytesPerPixel * dstWidth;
	if (dstPitch > dstWidthInByte) {
		dst = (uint32_t *)malloc(dstWidthInByte * dstLineCount);
	}

	xbrz::scale(_factor, src, dst, srcWidth, srcLineCount, xbrz::ColorFormat::ARGB, cfg, 0, srcLineCount);

	if (dstPitch > dstWidthInByte) {
		uint8 *tmp = dstPtr;
		for (int i = 0; i < dstLineCount; ++i) {
			int lineOffset = i * dstWidth;
			const uint32 *copyStart = dst + lineOffset;
			std::memcpy(tmp, copyStart, dstWidthInByte);
			tmp += dstPitch;
		}
		free(dst);
	}

	if (srcPitch > srcWidthInByte) {
		free(src);
	}
}

uint xBRZScaler::increaseFactor() {
	if (_factor < xbrz::SCALE_FACTOR_MAX)
		setFactor(_factor + 1);
	return _factor;
}

uint xBRZScaler::decreaseFactor() {
	if (_factor > 1)
		setFactor(_factor - 1);
	return _factor;
}


class xBRZPlugin final : public ScalerPluginObject {
public:
	xBRZPlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return true; }
	uint extraPixels() const override { return 1; }
	const char *getName() const override { return "xbrz"; }
	const char *getPrettyName() const override  { return "xBRZ"; }
};

xBRZPlugin::xBRZPlugin() {
	for (uint i = 1; i <= xbrz::SCALE_FACTOR_MAX; ++i)
		_factors.push_back(i);
}

Scaler *xBRZPlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new xBRZScaler(format);
}

REGISTER_PLUGIN_STATIC(XBRZ, PLUGIN_TYPE_SCALER, xBRZPlugin);
