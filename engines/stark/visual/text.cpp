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

namespace Stark {

VisualText::VisualText(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_color(0),
		_backgroundColor(0),
		_targetWidth(600),
		_fontCustomIndex(-1),
		_fontType(FontProvider::kBigFont) {
	_surfaceRenderer = _gfx->createSurfaceRenderer();
	_surfaceRenderer->setNoScalingOverride(true);
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

void VisualText::setColor(uint32 color) {
	if (_color != color) {
		freeTexture();
		_color = color;
	}
}

void VisualText::setBackgroundColor(uint32 color) {
	if (color != _backgroundColor) {
		freeTexture();
		_backgroundColor = color;
	}
}

void VisualText::setTargetWidth(uint32 width) {
	if (width != _targetWidth) {
		freeTexture();
		_targetWidth = width;
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
	// Get the font and required metrics
	const Graphics::Font *font = StarkFontProvider->getScaledFont(_fontType, _fontCustomIndex);
	uint scaledLineHeight = StarkFontProvider->getScaledFontHeight(_fontType, _fontCustomIndex);
	uint originalLineHeight = StarkFontProvider->getOriginalFontHeight(_fontType, _fontCustomIndex);
	uint maxScaledLineWidth = StarkGfx->scaleWidthOriginalToCurrent(_targetWidth);

	// Word wrap the text and compute the scaled and original resolution bounding boxes
	Common::Rect scaledRect;
	Common::Array<Common::String> lines;
	scaledRect.right = scaledRect.left + font->wordWrapText(_text, maxScaledLineWidth, lines);
	scaledRect.bottom = scaledRect.top + scaledLineHeight * lines.size();
	_originalRect.right = _originalRect.left + StarkGfx->scaleWidthCurrentToOriginal(scaledRect.width());
	_originalRect.bottom = _originalRect.top + originalLineHeight * lines.size();

	// Create a surface to render to
	Graphics::Surface surface;
	surface.create(scaledRect.width(), scaledRect.height(), Gfx::Driver::getRGBAPixelFormat());
	surface.fillRect(scaledRect, _backgroundColor);

	// Render the lines to the surface
	for (uint i = 0; i < lines.size(); i++) {
		font->drawString(&surface, lines[i], 0, scaledLineHeight * i, scaledRect.width(), _color);
	}

	// Create a texture from the surface
	_texture = _gfx->createTexture(&surface);
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
	createTexture();
}

} // End of namespace Stark
