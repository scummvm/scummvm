/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/visual/text.h"

#include "graphics/font.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/stark/debug.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/surfacerenderer.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

#include "common/util.h"

namespace Stark {

VisualText::VisualText(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_bgTexture(nullptr),
		_color(Color(0, 0, 0)),
		_backgroundColor(Color(0, 0, 0, 0)),
		_align(Graphics::kTextAlignLeft),
		_targetWidth(600),
		_targetHeight(600),
		_fontType(FontProvider::kBigFont),
		_fontCustomIndex(-1) {
	_surfaceRenderer = _gfx->createSurfaceRenderer();
	_surfaceRenderer->setNoScalingOverride(true);
	_surfaceRenderer->setSnapToGrid(true);
}

VisualText::~VisualText() {
	freeTexture();
	delete _surfaceRenderer;
}

Common::Rect VisualText::getRect() {
	if (!_texture) {
		createTexture();
	}
	return _originalRect;
}

void VisualText::setText(const Common::String &text) {
	if (_text != text) {
		freeTexture();
		_text = text;
	}
}

void VisualText::setColor(const Color &color) {
	if (_color == color) {
		return;
	}

	freeTexture();
	_color = color;
}

void VisualText::setBackgroundColor(const Color &color) {
	if (color == _backgroundColor) {
		return;
	}

	freeTexture();
	_backgroundColor = color;
}

void VisualText::setAlign(Graphics::TextAlign align) {
	if (align != _align) {
		freeTexture();
		_align = align;
	}
}

void VisualText::setTargetWidth(uint32 width) {
	if (width != _targetWidth) {
		freeTexture();
		_targetWidth = width;
	}
}

void VisualText::setTargetHeight(uint32 height) {
	if (height != _targetHeight) {
		freeTexture();
		_targetHeight = height;
	}
}

void VisualText::setFont(FontProvider::FontType type, int32 customFontIndex) {
	if (type != _fontType || customFontIndex != _fontCustomIndex) {
		freeTexture();
		_fontType = type;
		_fontCustomIndex = customFontIndex;
	}
}

/**
 * Convert a color component from the perceptual to the linear color space
 *
 * Gamma 1.8 seems to be an accepted value in the font rendering domain.
 */
static float srgbToLinear(float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else
		return powf(x, 1.8f);
}

/** Convert a color component from the linear to the perceptual color space */
static float linearToSrgb(float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else
		return powf(x, 1.0f / 1.8f);
}

/**
 * Multiply the color components of a surface with the alpha component
 *
 * Linear colorspace aware variant. Anti-aliased fonts at small sized
 * have a very few solid pixels. It's important the semi-transparent
 * pixels have the right value after the pre-multiplication.
 */
static void multiplyColorWithAlpha(Graphics::Surface *source) {
	assert(source->format == Gfx::Driver::getRGBAPixelFormat());

	for (uint y = 0; y < source->h; y++) {
		const uint8 *src = (const uint8 *) source->getBasePtr(0, y);
		uint8 *dst = (uint8 *) source->getBasePtr(0, y);

		for (uint x = 0; x < source->w; x++) {
			uint8 a, r, g, b;
			r = *src++;
			g = *src++;
			b = *src++;
			a = *src++;

			if (a == 0) {
				r = 0;
				g = 0;
				b = 0;

			} else if (a != 0xFF) {
				float aFloat = a / 255.f;
				float linearR = srgbToLinear(r / 255.f);
				float linearG = srgbToLinear(g / 255.f);
				float linearB = srgbToLinear(b / 255.f);

				linearR *= aFloat;
				linearG *= aFloat;
				linearB *= aFloat;

				r = linearToSrgb(linearR) * 255.f;
				g = linearToSrgb(linearG) * 255.f;
				b = linearToSrgb(linearB) * 255.f;
			}

			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			*dst++ = a;
		}
	}
}

/**
 * Blend a grayscale surface with a color
 *
 * Color space aware version.
 */
static void blendWithColor(Graphics::Surface *source, const Color &color) {
	assert(source->format == Gfx::Driver::getRGBAPixelFormat());

	float sRL = srgbToLinear(color.r / 255.f);
	float sGL = srgbToLinear(color.g / 255.f);
	float sBL = srgbToLinear(color.b / 255.f);

	for (uint y = 0; y < source->h; y++) {
		const uint8 *src = (const uint8 *) source->getBasePtr(0, y);
		uint8 *dst = (uint8 *) source->getBasePtr(0, y);

		for (uint x = 0; x < source->w; x++) {
			uint8 a, r, g, b;
			r = *src++;
			g = *src++;
			b = *src++;
			src++;

			a = r;
			if (a != 255) {
				float aFloat = a / 255.f;
				r = linearToSrgb(sRL * aFloat) * 255.f;
				g = linearToSrgb(sGL * aFloat) * 255.f;
				b = linearToSrgb(sBL * aFloat) * 255.f;

			} else {
				r = color.r;
				g = color.g;
				b = color.b;
			}

			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			*dst++ = a;
		}
	}
}

void VisualText::createTexture() {
	Common::CodePage codePage = StarkSettings->getTextCodePage();
	Common::U32String unicodeText = Common::convertToU32String(_text.c_str(), codePage);

	// Get the font and required metrics
	const Graphics::Font *font = StarkFontProvider->getScaledFont(_fontType, _fontCustomIndex);
	uint scaledLineHeight = StarkFontProvider->getScaledFontHeight(_fontType, _fontCustomIndex);
	uint originalLineHeight = StarkFontProvider->getOriginalFontHeight(_fontType, _fontCustomIndex);
	uint maxScaledLineWidth = StarkGfx->scaleWidthOriginalToCurrent(_targetWidth);

	// Word wrap the text
	Common::Array<Common::U32String> lines;
	font->wordWrapText(unicodeText, maxScaledLineWidth, lines);

	// Use the actual font bounding box to prevent text from being cut off
	Common::Rect scaledRect;
	if (!lines.empty()) {
		scaledRect = font->getBoundingBox(lines[0]);
		for (uint i = 1; i < lines.size(); i++) {
			scaledRect.extend(font->getBoundingBox(lines[i], 0, scaledLineHeight * i));
		}
	}

	// Make sure lines have approximately consistent height regardless of the characters they use
	scaledRect.bottom = MAX<int16>(scaledRect.bottom, scaledLineHeight * lines.size());

	if (!isBlank()) {
		_originalRect.right = StarkGfx->scaleWidthCurrentToOriginal(scaledRect.right);
		_originalRect.bottom = originalLineHeight * lines.size();
	} else {
		// For Empty text, preserve the original width and height for being used as clicking area
		_originalRect.right = _targetWidth;
		_originalRect.bottom = _targetHeight;
	}

	// Create a surface to render to
	Graphics::Surface surface;
	surface.create(scaledRect.right, scaledRect.bottom, Gfx::Driver::getRGBAPixelFormat());

	// First render the text as white on a black background to produce an alpha mask
	uint32 black = surface.format.ARGBToColor(0xFF, 0, 0, 0);
	uint32 white = surface.format.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	surface.fillRect(Common::Rect(surface.w, surface.h), black);

	// Render the lines to the surface
	for (uint i = 0; i < lines.size(); i++) {
		font->drawString(&surface, lines[i], 0, scaledLineHeight * i, surface.w, white, _align, 0, false);
	}

	// Blend the text color with the alpha mask to produce an image of the text
	// of the correct color. Anti-aliased pixels use the full alpha range.
	blendWithColor(&surface, _color);
	multiplyColorWithAlpha(&surface);

	// Create a texture from the surface
	_texture = _gfx->createTexture(&surface);
	_texture->setSamplingFilter(Gfx::Texture::kNearest);

	surface.free();

	// If we have a background color, generate a 1x1px texture of that color
	if (_backgroundColor.a != 0) {
		surface.create(1, 1, Gfx::Driver::getRGBAPixelFormat());

		uint32 bgColor = surface.format.ARGBToColor(
		            _backgroundColor.a, _backgroundColor.r, _backgroundColor.g, _backgroundColor.b
		            );

		surface.fillRect(Common::Rect(surface.w, surface.h), bgColor);
		multiplyColorWithAlpha(&surface);

		_bgTexture = _gfx->createTexture(&surface);

		surface.free();
	}
}

void VisualText::freeTexture() {
	delete _texture;
	_texture = nullptr;
	delete _bgTexture;
	_bgTexture = nullptr;
}

void VisualText::render(const Common::Point &position) {
	if (!_texture) {
		createTexture();
	}

	if (_bgTexture) {
		_surfaceRenderer->render(_bgTexture, position, _texture->width(), _texture->height());
	}

	_surfaceRenderer->render(_texture, position);
}

void VisualText::resetTexture() {
	freeTexture();
}

bool VisualText::isBlank() {
	for (uint i = 0; i < _text.size(); ++i) {
		if (!Common::isSpace(_text[i])) {
			return false;
		}
	}
	return true;
}

} // End of namespace Stark
