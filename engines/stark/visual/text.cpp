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

	uint32 color = surface.format.ARGBToColor(
			_color.a, _color.r, _color.g, _color.b
	);
	uint32 bgColor = surface.format.ARGBToColor(
			_backgroundColor.a, _backgroundColor.r, _backgroundColor.g, _backgroundColor.b
	);

	surface.fillRect(Common::Rect(surface.w, surface.h), bgColor);

	// Render the lines to the surface
	for (uint i = 0; i < lines.size(); i++) {
		font->drawString(&surface, lines[i], 0, scaledLineHeight * i, surface.w, color, _align);
	}

	// Create a texture from the surface
	_texture = _gfx->createTexture(&surface);
	_texture->setSamplingFilter(Gfx::Texture::kNearest);

	surface.free();
}

void VisualText::freeTexture() {
	delete _texture;
	_texture = nullptr;
}

void VisualText::render(const Common::Point &position) {
	if (!_texture) {
		createTexture();
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
