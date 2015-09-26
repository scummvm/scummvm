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
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/stark/debug.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

namespace Stark {

VisualText::VisualText(Gfx::Driver *gfx) :
		Visual(TYPE),
		_gfx(gfx),
		_texture(nullptr),
		_color(0),
		_targetWidth(600) {
}

VisualText::~VisualText() {
	freeTexture();
}

Common::Rect VisualText::getRect() {
	if (!_texture) {
		createTexture();
	}

	return Common::Rect(_texture->width(), _texture->height());
}

void VisualText::setText(const Common::String &text) {
	freeTexture();
	_text = text;
}

void VisualText::setColor(uint32 color) {
	freeTexture();
	_color = color;
}

void VisualText::setTargetWidth(uint32 width) {
	freeTexture();
	_targetWidth = width;
}

void VisualText::createTexture() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);

	Common::Array<Common::String> lines;
	font->wordWrapText(_text, _targetWidth, lines);

	int height = font->getFontHeight();
	int width = 0;
	for (uint i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]));
	}

	Graphics::Surface surface;
	surface.create(width, height*lines.size(), _gfx->getScreenFormat());

	for (uint i = 0; i < lines.size(); i++) {
		font->drawString(&surface, lines[i], 0, height*i, _targetWidth, _color);
	}
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

	_gfx->drawSurface(_texture, position);
}

} // End of namespace Stark
