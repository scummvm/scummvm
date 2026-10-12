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

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "harold/bcb/text_renderer.h"
#include "harold/bcb/translation.h"
#include "image/png.h"

namespace BCB {
namespace {

bool parseTranslationInt(const char *text, int &value) {
	if (!text || !text[0])
		return false;
	return sscanf(text, "%d", &value) == 1;
}

int getTranslationInt(const TextStringTable &strings, const Common::String &key, int defaultValue) {
	int value = defaultValue;
	parseTranslationInt(strings.getString(key), value);
	return value;
}

uint32 getTranslationColor(const TextStringTable &strings, const Common::String &key, const Graphics::PixelFormat &format, uint8 defaultR, uint8 defaultG, uint8 defaultB) {
	const char *text = strings.getString(key);
	uint r = defaultR;
	uint g = defaultG;
	uint b = defaultB;
	if (text && text[0])
		sscanf(text, "%u,%u,%u", &r, &g, &b);
	return format.ARGBToColor(0xff, CLIP<uint>(r, 0, 255), CLIP<uint>(g, 0, 255), CLIP<uint>(b, 0, 255));
}

Common::Array<Common::Point> getTranslationShadowOffsets(const TextStringTable &strings, const Common::String &key) {
	Common::Array<Common::Point> offsets;
	const char *text = strings.getString(key);
	if (!text || !text[0]) {
		offsets.push_back(Common::Point(0, 2));
		offsets.push_back(Common::Point(0, 3));
		offsets.push_back(Common::Point(0, 4));
		offsets.push_back(Common::Point(1, 4));
		offsets.push_back(Common::Point(-1, 4));
		offsets.push_back(Common::Point(0, 5));
		return offsets;
	}

	Common::String offsetsText(text);
	while (!offsetsText.empty()) {
		Common::String item = offsetsText;
		const uint separator = offsetsText.findFirstOf(";");
		if (separator != Common::String::npos) {
			item = offsetsText.substr(0, separator);
			offsetsText = offsetsText.substr(separator + 1);
		} else {
			offsetsText.clear();
		}

		int x = 0;
		int y = 0;
		if (sscanf(item.c_str(), "%d,%d", &x, &y) == 2)
			offsets.push_back(Common::Point(x, y));
	}

	return offsets;
}

bool parseStillHeadingLine(const char *definition, Common::U32String &text, int &x, int &y, int &size, int &strokeWidth) {
	if (!definition || !definition[0])
		return false;

	Common::String fields[5];
	Common::String remaining(definition);
	for (int i = 0; i < 5; ++i) {
		const uint separator = remaining.findFirstOf(";");
		if (separator == Common::String::npos) {
			fields[i] = remaining;
			remaining.clear();
		} else {
			fields[i] = remaining.substr(0, separator);
			remaining = remaining.substr(separator + 1);
		}
		fields[i].trim();
	}

	if (fields[0].empty())
		return false;

	text = Common::U32String(fields[0]);
	parseTranslationInt(fields[1].c_str(), x);
	parseTranslationInt(fields[2].c_str(), y);
	parseTranslationInt(fields[3].c_str(), size);
	parseTranslationInt(fields[4].c_str(), strokeWidth);
	return true;
}

void drawConfiguredHighQualityTextLines(Graphics::Surface &surface, const TextStringTable &strings, const Common::String &prefix,
	const char *defaultFont, uint8 defaultTextR, uint8 defaultTextG, uint8 defaultTextB,
	uint8 defaultShadowR, uint8 defaultShadowG, uint8 defaultShadowB) {
	const char *fontName = strings.getString(prefix + "font");
	if (!fontName || !fontName[0])
		fontName = defaultFont;

	const Common::Array<Common::Point> shadowOffsets = getTranslationShadowOffsets(strings, prefix + "shadow_offsets");
	const bool drawShadowWithoutStroke = getTranslationInt(strings, prefix + "shadow_without_stroke", 0) != 0;
	const int scale = CLIP<int>(getTranslationInt(strings, prefix + "scale", 4), 1, 8);
	Graphics::Surface layer;
	layer.create(surface.w * scale, surface.h * scale, Graphics::PixelFormat::createFormatRGBA32());
	layer.fillRect(Common::Rect(layer.w, layer.h), 0);
	const uint32 scaledTextColor = getTranslationColor(strings, prefix + "text_color", layer.format, defaultTextR, defaultTextG, defaultTextB);
	const uint32 scaledShadowColor = getTranslationColor(strings, prefix + "shadow_color", layer.format, defaultShadowR, defaultShadowG, defaultShadowB);
	Common::Array<Common::Point> scaledShadowOffsets;
	for (uint i = 0; i < shadowOffsets.size(); ++i)
		scaledShadowOffsets.push_back(Common::Point(shadowOffsets[i].x * scale, shadowOffsets[i].y * scale));

	for (int i = 1; i <= 16; ++i) {
		Common::U32String text;
		int x = 0;
		int y = 0;
		int size = 44;
		int strokeWidth = 0;
		if (!parseStillHeadingLine(strings.getString(Common::String::format("%sline%d", prefix.c_str(), i)), text, x, y, size, strokeWidth))
			continue;

		const Common::Array<Common::Point> emptyShadowOffsets;
		drawHighQualityTextLine(layer, fontName, size * scale, (strokeWidth == 0 && !drawShadowWithoutStroke) ? emptyShadowOffsets : scaledShadowOffsets,
			text, x * scale, y * scale, scaledTextColor, scaledShadowColor, strokeWidth * scale);
	}

	Graphics::Surface *downscaledLayer = downscaleRgbaLayerBicubic(layer, surface.w, surface.h);
	layer.free();
	if (!downscaledLayer)
		return;

	alphaCompositeRgbaLayer(surface, *downscaledLayer);
	downscaledLayer->free();
	delete downscaledLayer;
}

void drawTranslatedStillHeading(Graphics::Surface &still, int stillIndex, const TextStringTable &strings) {
	if (stillIndex < 0 || stillIndex >= 5)
		return;

	const int stillNumber = stillIndex + 1;
	const Common::String prefix = Common::String::format("still_headings.%d.", stillNumber);
	drawConfiguredHighQualityTextLines(still, strings, prefix, "LiberationSerif-BoldItalic.ttf", 255, 255, 255, 0, 0, 0);
}

void applyPatchImage(Graphics::Surface &surface, Common::SeekableReadStream *stream, const char *debugName) {
	Common::ScopedPtr<Common::SeekableReadStream> patchStream(stream);
	if (!patchStream)
		return;

	Image::PNGDecoder decoder;
	if (!decoder.loadStream(*patchStream) || !decoder.getSurface()) {
		warning("%s: Could not decode translated patch", debugName);
		return;
	}

	const Graphics::Surface *patch = decoder.getSurface();
	const int width = MIN<int>(surface.w, patch->w);
	const int height = MIN<int>(surface.h, patch->h);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint8 srcA, srcR, srcG, srcB;
			patch->format.colorToARGB(patch->getPixel(x, y), srcA, srcR, srcG, srcB);
			if (srcA == 0)
				continue;

			if (srcA == 0xff) {
				surface.setPixel(x, y, surface.format.ARGBToColor(0xff, srcR, srcG, srcB));
				continue;
			}

			uint8 dstA, dstR, dstG, dstB;
			surface.format.colorToARGB(surface.getPixel(x, y), dstA, dstR, dstG, dstB);
			const uint invA = 255 - srcA;
			const uint8 outR = (srcR * srcA + dstR * invA) / 255;
			const uint8 outG = (srcG * srcA + dstG * invA) / 255;
			const uint8 outB = (srcB * srcA + dstB * invA) / 255;
			surface.setPixel(x, y, surface.format.ARGBToColor(0xff, outR, outG, outB));
		}
	}
}

} // End of anonymous namespace

void applyTranslatedStillImage(Graphics::Surface &still, int stillIndex, const TextTranslationData &translation) {
	if (translation.packageArchive && stillIndex >= 0 && stillIndex < 5) {
		applyPatchImage(still, translation.packageArchive->createReadStreamForMember(Common::Path(Common::String::format("patches/pstill_%d_patch.png", stillIndex + 1), '/')), "applyTranslatedStillImage");
	}

	drawTranslatedStillHeading(still, stillIndex, translation.strings);
}

void applyTranslatedComputerButtons(Graphics::Surface &computer, const TextTranslationData &translation) {
	if (!translation.packageArchive)
		return;

	Common::ScopedPtr<Common::SeekableReadStream> stream(translation.packageArchive->createReadStreamForMember(Common::Path("256com_buttons.png")));
	if (!stream)
		return;
	Image::PNGDecoder decoder;
	if (!decoder.loadStream(*stream) || !decoder.getSurface()) {
		warning("Could not decode translated computer buttons");
		return;
	}
	const Graphics::Surface *atlas = decoder.getSurface();
	if (atlas->w != 234 || atlas->h != 265 || computer.w < 640 || computer.h < 738) {
		warning("Invalid translated computer button atlas or computer bitmap dimensions");
		return;
	}
	Graphics::Surface *converted = atlas->convertTo(computer.format, decoder.getPalette().data(), decoder.getPalette().size());
	if (!converted)
		return;

	// The compact atlas has shared one-pixel borders, excluded from every copy.
	for (int state = 0; state < 3; ++state)
		computer.copyRectToSurface(*converted, 0, 672 + state * 22, Common::Rect(1, 1 + state * 23, 233, 23 + state * 23));
	for (int page = 0; page < 2; ++page)
		computer.copyRectToSurface(*converted, 392 + page * 160, 672, Common::Rect(1 + page * 81, 70, 81 + page * 81, 94));
	// Captions follow the sheet's 29 populated icons; the unused choice has no cell.
	for (int icon = 0; icon < 29; ++icon) {
		const int x = 1 + (icon % 3) * 65;
		const int y = 95 + (icon / 3) * 17;
		computer.copyRectToSurface(*converted, (icon % 10) * 64, 528 + (icon / 10) * 64, Common::Rect(x, y, x + 64, y + 16));
	}
	converted->free();
	delete converted;
}

void applyTranslatedTitleMenu(Graphics::Surface &title, const TextTranslationData &translation) {
	if (!translation.packageArchive)
		return;

	applyPatchImage(title, translation.packageArchive->createReadStreamForMember(Common::Path("patches/16mtitle_patch.png", '/')), "applyTranslatedTitleMenu");
	drawConfiguredHighQualityTextLines(title, translation.strings, "title_menu.", "LiberationSans-BoldItalic.ttf", 53, 55, 106, 142, 146, 232);
	drawConfiguredHighQualityTextLines(title, translation.strings, "title_menu_pressed.", "LiberationSans-BoldItalic.ttf", 136, 90, 155, 183, 169, 211);
}

} // End of namespace BCB
