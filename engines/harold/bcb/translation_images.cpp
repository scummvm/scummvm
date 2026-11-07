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

bool parseImageTextLine(const char *definition, Common::U32String &text, int &x, int &y, int &size, int &strokeWidth,
						uint32 &color, const Graphics::PixelFormat &format) {
	if (!definition || !definition[0])
		return false;

	Common::String fields[6];
	Common::String remaining(definition);
	for (int i = 0; i < 6; ++i) {
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
	uint r, g, b;
	if (sscanf(fields[5].c_str(), "%u,%u,%u", &r, &g, &b) == 3)
		color = format.ARGBToColor(255, MIN(r, 255U), MIN(g, 255U), MIN(b, 255U));
	return true;
}

void drawConfiguredHighQualityTextLines(Graphics::Surface &surface, const TextTranslationData &translation, const Common::String &prefix,
										const char *defaultFont, uint8 defaultTextR, uint8 defaultTextG, uint8 defaultTextB,
										uint8 defaultShadowR, uint8 defaultShadowG, uint8 defaultShadowB, bool resampleBackground = false) {
	const TextStringTable &strings = translation.strings;
	const char *fontName = strings.getString(prefix + "font");
	if (!fontName || !fontName[0])
		fontName = defaultFont;

	const Common::Array<Common::Point> shadowOffsets = getTranslationShadowOffsets(strings, prefix + "shadow_offsets");
	const bool drawShadowWithoutStroke = getTranslationInt(strings, prefix + "shadow_without_stroke", 0) != 0;
	const int scale = resampleBackground ? 4 : CLIP<int>(getTranslationInt(strings, prefix + "scale", 4), 1, 8);
	Graphics::Surface layer;
	layer.create(surface.w * scale, surface.h * scale, Graphics::PixelFormat::createFormatRGBA32());
	if (resampleBackground) {
		for (int y = 0; y < surface.h; ++y) {
			uint32 *row = static_cast<uint32 *>(layer.getBasePtr(0, y * scale));
			for (int x = 0; x < surface.w; ++x) {
				uint8 r, g, b;
				surface.format.colorToRGB(surface.getPixel(x, y), r, g, b);
				const uint32 pixel = layer.format.RGBToColor(r, g, b);
				for (int i = 0; i < scale; ++i)
					row[x * scale + i] = pixel;
			}
			for (int i = 1; i < scale; ++i)
				memcpy(layer.getBasePtr(0, y * scale + i), row, layer.w * 4);
		}
	} else {
		layer.fillRect(Common::Rect(layer.w, layer.h), 0);
	}
	const uint32 scaledTextColor = getTranslationColor(strings, prefix + "text_color", layer.format, defaultTextR, defaultTextG, defaultTextB);
	const uint32 scaledShadowColor = getTranslationColor(strings, prefix + "shadow_color", layer.format, defaultShadowR, defaultShadowG, defaultShadowB);
	Common::Array<Common::Point> scaledShadowOffsets;
	for (uint i = 0; i < shadowOffsets.size(); ++i)
		scaledShadowOffsets.push_back(Common::Point(shadowOffsets[i].x * scale, shadowOffsets[i].y * scale));

	HighQualityTextRenderer *renderer = translation.imageFonts.getValOrDefault(fontName, nullptr);
	Common::ScopedPtr<HighQualityTextRenderer> fallbackRenderer;
	if (!renderer) {
		fallbackRenderer.reset(new HighQualityTextRenderer(fontName));
		renderer = fallbackRenderer.get();
	}
	for (int i = 1; ; ++i) {
		const char *definition = strings.getString(Common::String::format("%sline%d", prefix.c_str(), i));
		if (!definition)
			break;
		Common::U32String text;
		int x = 0;
		int y = 0;
		int size = 44;
		int strokeWidth = 0;
		uint32 textColor = scaledTextColor;
		if (!parseImageTextLine(definition, text, x, y, size, strokeWidth, textColor, layer.format))
			continue;
		size = CLIP(size, 1, 128);
		strokeWidth = CLIP(strokeWidth, 0, 16);

		const Common::Array<Common::Point> emptyShadowOffsets;
		renderer->drawLine(layer, size * scale, (strokeWidth == 0 && !drawShadowWithoutStroke) ? emptyShadowOffsets : scaledShadowOffsets,
								text, x * scale, y * scale, textColor, scaledShadowColor, strokeWidth * scale);
	}

	Graphics::Surface *downscaledLayer = resampleBackground ? downscaleRgbaLayerLanczos(layer, surface.w, surface.h) : downscaleRgbaLayerBicubic(layer, surface.w, surface.h);
	layer.free();
	if (!downscaledLayer)
		return;

	alphaCompositeRgbaLayer(surface, *downscaledLayer);
	downscaledLayer->free();
	delete downscaledLayer;
}

void drawTranslatedStillHeading(Graphics::Surface &still, int stillIndex, const TextTranslationData &translation) {
	if (stillIndex < 0 || stillIndex >= 5)
		return;

	const int stillNumber = stillIndex + 1;
	const Common::String prefix = Common::String::format("still_headings.%d.", stillNumber);
	drawConfiguredHighQualityTextLines(still, translation, prefix, "LiberationSerif-BoldItalic.ttf", 255, 255, 255, 0, 0, 0);
}

void drawTranslatedEnding(Graphics::Surface &surface, const TextTranslationData &translation, const Common::String &prefix) {
#ifdef USE_FREETYPE2
	const TextStringTable &strings = translation.strings;
	const char *patch = strings.getString(prefix + "patch");
	const char *font = strings.getString(prefix + "font");
	HighQualityTextRenderer *renderer = font ? translation.imageFonts.getValOrDefault(font, nullptr) : nullptr;
	if (!translation.packageArchive || !patch || !renderer || !strings.getString(prefix + "line1"))
		return;
	if (!applyPatchImage(surface, translation.packageArchive->createReadStreamForMember(Common::Path(patch, '/')), "drawTranslatedEnding"))
		return;

	const int scale = 4;
	const int widthPercent = CLIP(getTranslationInt(strings, prefix + "width_percent", 100), 1, 100);
	const int shadowX = getTranslationInt(strings, prefix + "shadow_x", 2);
	const int shadowY = getTranslationInt(strings, prefix + "shadow_y", 2);
	const int opacity = CLIP(getTranslationInt(strings, prefix + "shadow_opacity", 100), 0, 100);
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatRGBA32();
	const uint32 black = format.RGBToColor(0, 0, 0);
	const uint32 white = format.RGBToColor(255, 255, 255);
	Graphics::Surface mask, line;
	mask.create(surface.w * scale, surface.h * scale, format);
	mask.fillRect(Common::Rect(mask.w, mask.h), black);
	line.create(mask.w, 60 * scale, format);
	const Common::Array<Common::Point> noShadow;
	for (int i = 1; ; ++i) {
		const char *definition = strings.getString(Common::String::format("%sline%d", prefix.c_str(), i));
		if (!definition)
			break;
		Common::U32String text;
		int x = 0, y = 0, size = 23, stroke = 0;
		uint32 color = white;
		if (!parseImageTextLine(definition, text, x, y, size, stroke, color, format))
			continue;
		line.fillRect(Common::Rect(line.w, line.h), black);
		renderer->drawLine(line, CLIP(size, 1, 128) * scale, noShadow, text, 0, 40 * scale, white, black, 0, true);
		Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> narrowed(
			downscaleRgbaLayerBicubic(line, MAX(1, (line.w * widthPercent + 50) / 100), line.h));
		for (int row = 0; row < narrowed->h; ++row) {
			const int destY = (y - 40) * scale + row;
			if (destY < 0 || destY >= mask.h)
				continue;
			for (int col = 0; col < narrowed->w; ++col) {
				const int destX = x * scale + col;
				if (destX < 0 || destX >= mask.w)
					continue;
				const byte value = *static_cast<const byte *>(narrowed->getBasePtr(col, row));
				byte *dest = static_cast<byte *>(mask.getBasePtr(destX, destY));
				dest[0] = dest[1] = dest[2] = MAX(dest[0], value);
			}
		}
	}
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> reduced(downscaleRgbaLayerBicubic(mask, surface.w, surface.h));
	mask.free();
	line.free();
	uint8 textR, textG, textB, shadowR, shadowG, shadowB;
	format.colorToRGB(getTranslationColor(strings, prefix + "text_color", format, 255, 237, 214), textR, textG, textB);
	format.colorToRGB(getTranslationColor(strings, prefix + "shadow_color", format, 77, 0, 18), shadowR, shadowG, shadowB);
	for (int y = 0; y < surface.h; ++y) {
		for (int x = 0; x < surface.w; ++x) {
			const int sx = x - shadowX, sy = y - shadowY;
			const byte ink = *static_cast<const byte *>(reduced->getBasePtr(x, y));
			const byte shade = sx >= 0 && sx < reduced->w && sy >= 0 && sy < reduced->h ?
				*static_cast<const byte *>(reduced->getBasePtr(sx, sy)) : 0;
			if (!ink && !shade)
				continue;
			uint8 r, g, b;
			surface.format.colorToRGB(surface.getPixel(x, y), r, g, b);
			const double alpha = ink / 255.0;
			const double shadow = shade / 255.0 * (opacity / 100.0);
			const auto blend = [&](uint8 background, uint8 tint, uint8 foreground) {
				const double under = background * (1 - shadow) + MIN(background, tint) * shadow;
				const double value = under * (1 - alpha) + foreground * alpha;
				const int integral = (int)value;
				return (uint8)(integral + (value - integral > 0.5 || (value - integral == 0.5 && (integral & 1))));
			};
			surface.setPixel(x, y, surface.format.RGBToColor(blend(r, shadowR, textR), blend(g, shadowG, textG), blend(b, shadowB, textB)));
		}
	}
#endif
}

} // End of anonymous namespace

bool applyPatchImage(Graphics::Surface &surface, Common::SeekableReadStream *stream, const char *debugName) {
	Common::ScopedPtr<Common::SeekableReadStream> patchStream(stream);
	if (!patchStream)
		return false;

	Image::PNGDecoder decoder;
	if (!decoder.loadStream(*patchStream) || !decoder.getSurface()) {
		warning("%s: Could not decode image patch", debugName);
		return false;
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
	return true;
}

void applyTranslatedComputerCard(Graphics::Surface &card, const char *path, const TextTranslationData &translation) {
#ifdef USE_FREETYPE2
	if (!translation.packageArchive)
		return;
	Common::String name = Common::Path(path, '/').getLastComponent().toString();
	name.toLowercase();
	const uint dot = name.findLastOf('.');
	if (dot != Common::String::npos)
		name = name.substr(0, dot);
	const Common::String prefix = "computer_cards." + name + ".";
	const char *patch = translation.strings.getString(prefix + "patch");
	if (!patch || !translation.strings.getString(prefix + "line1"))
		return;
	if (!applyPatchImage(card, translation.packageArchive->createReadStreamForMember(Common::Path(patch, '/')), "applyTranslatedComputerCard"))
		return;
	drawConfiguredHighQualityTextLines(card, translation, prefix, "LiberationSans-Regular.ttf", 218, 218, 236, 0, 0, 0, true);
#endif
}

void applyTranslatedStillImage(Graphics::Surface &still, int stillIndex, const TextTranslationData &translation) {
	if (translation.packageArchive && stillIndex >= 0 && stillIndex < 5) {
		applyPatchImage(still, translation.packageArchive->createReadStreamForMember(Common::Path(Common::String::format("patches/pstill_%d_patch.png", stillIndex + 1), '/')), "applyTranslatedStillImage");
	}

	drawTranslatedStillHeading(still, stillIndex, translation);
	if (stillIndex == 5 || stillIndex == 6)
		drawTranslatedEnding(still, translation, stillIndex == 5 ? "endings.pgood." : "endings.pbad.");
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
	if (atlas->w != 234 || atlas->h != 290 || computer.w < 640 || computer.h < 738) {
		warning("Invalid translated computer button atlas or computer bitmap dimensions");
		return;
	}
	Graphics::Surface *converted = atlas->convertTo(computer.format, decoder.getPalette().data(), decoder.getPalette().size());
	if (!converted)
		return;

	// The compact atlas has shared one-pixel borders, excluded from every copy.
	for (int state = 0; state < 3; ++state)
		computer.copyRectToSurface(*converted, 0, 672 + state * 22, Common::Rect(1, 1 + state * 23, 233, 23 + state * 23));
	for (int page = 0; page < 2; ++page) {
		computer.copyRectToSurface(*converted, 392 + page * 160, 672, Common::Rect(1 + page * 81, 70, 81 + page * 81, 94));
		computer.copyRectToSurface(*converted, 312 + page * 160, 672, Common::Rect(1 + page * 81, 95, 81 + page * 81, 119));
	}
	// Captions follow the sheet's 29 populated icons; the unused choice has no cell.
	for (int icon = 0; icon < 29; ++icon) {
		const int x = 1 + (icon % 3) * 65;
		const int y = 120 + (icon / 3) * 17;
		computer.copyRectToSurface(*converted, (icon % 10) * 64, 528 + (icon / 10) * 64, Common::Rect(x, y, x + 64, y + 16));
	}
	computer.copyRectToSurface(*converted, 576, 608, Common::Rect(163, 70, 217, 94));
	converted->free();
	delete converted;
}

bool applyTranslatedCinemaTitleButton(Graphics::Surface &buttons, const TextTranslationData &translation) {
#ifdef USE_FREETYPE2
	if (translation.strings.getString("cinema_title_button.line1") &&
		translation.strings.getString("cinema_title_button_pressed.line1")) {
		drawConfiguredHighQualityTextLines(buttons, translation, "cinema_title_button.", "LiberationSans-BoldItalic.ttf", 53, 55, 106, 142, 146, 232);
		drawConfiguredHighQualityTextLines(buttons, translation, "cinema_title_button_pressed.", "LiberationSans-BoldItalic.ttf", 136, 90, 155, 183, 169, 211);
		return true;
	}
#endif
	return false;
}

void applyTranslatedCinemaMenu(Graphics::Surface &menu, const TextTranslationData &translation) {
#ifdef USE_FREETYPE2
	if (!translation.packageArchive)
		return;
	const char *patch = translation.strings.getString("cinema_menu.patch");
	if (!patch || !translation.strings.getString("cinema_menu.buttons.line1") ||
		!translation.strings.getString("cinema_menu.instruction.line1"))
		return;
	if (!applyPatchImage(menu, translation.packageArchive->createReadStreamForMember(Common::Path(patch, '/')), "applyTranslatedCinemaMenu"))
		return;

	drawConfiguredHighQualityTextLines(menu, translation, "cinema_menu.buttons.", "LiberationSans-BoldItalic.ttf", 32, 40, 48, 0, 0, 0);
	drawConfiguredHighQualityTextLines(menu, translation, "cinema_menu.instruction.", "LiberationSerif-Regular.ttf", 216, 176, 32, 0, 0, 0);
#endif
}

void applyTranslatedTitleMenu(Graphics::Surface &title, const TextTranslationData &translation) {
	if (!translation.packageArchive)
		return;

	applyPatchImage(title, translation.packageArchive->createReadStreamForMember(Common::Path("patches/16mtitle_patch.png", '/')), "applyTranslatedTitleMenu");
	drawConfiguredHighQualityTextLines(title, translation, "title_menu.", "LiberationSans-BoldItalic.ttf", 53, 55, 106, 142, 146, 232);
	drawConfiguredHighQualityTextLines(title, translation, "title_menu_pressed.", "LiberationSans-BoldItalic.ttf", 136, 90, 155, 183, 169, 211);
}

} // End of namespace BCB
