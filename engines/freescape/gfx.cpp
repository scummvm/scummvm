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

#include "gui/message.h"
#include "graphics/renderer.h"
#include "engines/util.h"

#include "freescape/gfx.h"

namespace Freescape {

Renderer::Renderer(OSystem *system, int screenW, int screenH, Common::RenderMode renderMode)
	: _system(system) {

	_screenW = screenW;
	_screenH = screenH;
	_currentPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_originalPixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_palettePixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	_keyColor = -1;
	_palette = nullptr;
	_colorMap = nullptr;
	_renderMode = renderMode;
}

Renderer::~Renderer() {}

void Renderer::readFromPalette(uint8 index, uint8 &r, uint8 &g, uint8 &b) {
	r = _palette[3 * index + 0];
	g = _palette[3 * index + 1];
	b = _palette[3 * index + 2];
}

bool Renderer::getRGBAt(uint8 index, uint8 &r, uint8 &g, uint8 &b) {

	if (index == _keyColor)
		return false;

	if (index == 0) {
		readFromPalette(0, r, g, b);
		return true;
	}

	if (_renderMode == Common::kRenderAmiga || _renderMode == Common::kRenderAtariST) {
		readFromPalette(index, r, g, b);
		return true;
	}

	// assert(index-1 < _colorMap->size());
	byte *entry = (*_colorMap)[index - 1];
	uint8 color = 0;
	uint8 acc = 1;
	for (int i = 0; i < 4; i++) {
		byte be = *entry;
		if (be != 0 && be != 0xff) {
			// TODO: fix colors for non-DOS releases
			readFromPalette(index, r, g, b);
			return true;
		}

		if (be == 0xff)
			color = color + acc;

		acc = acc << 1;
		entry++;
	}
	assert(color < 16);
	readFromPalette(color, r, g, b);
	return true;
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

void Renderer::computeScreenViewport() {
	_screenViewport = Common::Rect(_screenW, _screenH);
}

Renderer *createRenderer(OSystem *system, int screenW, int screenH, Common::RenderMode renderMode) {
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	// This code will allow different renderers, but right now, it only support TinyGL
	Graphics::RendererType desiredRendererType = Graphics::kRendererTypeTinyGL;

	initGraphics(screenW, screenH, &pixelFormat);

	#if defined(USE_TINYGL)
	if (desiredRendererType == Graphics::kRendererTypeTinyGL) {
		return CreateGfxTinyGL(system, screenW, screenH, renderMode);
	}
	#endif

	GUI::MessageDialog dialog("No available renderers enabled");
	// TODO: improve message with other renders
	dialog.runModal();
	return nullptr;
}

} // End of namespace Freescape
