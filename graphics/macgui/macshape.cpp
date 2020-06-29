/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macshape.h"
#include "graphics/primitives.h"

namespace Graphics {

MacShape::MacShape(MacShapeType shapeType, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, uint fgcolor, uint bgcolor, uint thickness, uint fillType) : _shapeType(shapeType), _thickness(thickness), _fillType(fillType), MacWidget(parent, x, y, w, h, wm, false, 0, 0, 0, fgcolor, bgcolor) {
		_contentIsDirty = true;
		render();
	}

void MacShape::setStyle(MacShapeType shapeType, uint fgcolor, uint bgcolor, uint thickness, uint fillType) {
	_contentIsDirty = true;

	_shapeType = shapeType;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_thickness = thickness;
	_fillType = fillType;
}

void MacShape::render() {
	if (!_contentIsDirty)
		return;

	_maskSurface->clear(0);
	_composeSurface->clear(_bgcolor);

	Common::Rect fillRect((int)_dims.width(), (int)_dims.height());
	Graphics::MacPlotData plotFill(_composeSurface, _maskSurface, &_wm->getPatterns(), _fillType, -_dims.left, -_dims.top, 1, _bgcolor);

	Common::Rect strokeRect(MAX((int)_dims.width() - _thickness, 0), MAX((int)_dims.height() - _thickness, 0));
	Graphics::MacPlotData plotStroke(_composeSurface, _maskSurface, &_wm->getPatterns(), 1, -_dims.left, -_dims.top, _thickness, _bgcolor);

	switch (_shapeType) {
	case kShapeRectangle:
		if (_fillType)
			Graphics::drawFilledRect(fillRect, _fgcolor, Graphics::macDrawPixel, &plotFill);

		Graphics::drawRect(strokeRect, _fgcolor, Graphics::macDrawPixel, &plotStroke);
		break;

	case kShapeRoundRect:
		if (_fillType)
			Graphics::drawRoundRect(fillRect, 12, _fgcolor, true, Graphics::macDrawPixel, &plotFill);

		Graphics::drawRoundRect(strokeRect, 12, _fgcolor, false, Graphics::macDrawPixel, &plotStroke);
		break;

	case kShapeOval:
		if (_fillType)
			Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, _fgcolor, true, Graphics::macDrawPixel, &plotFill);

		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, _fgcolor, false, Graphics::macDrawPixel, &plotStroke);
		break;

	case kShapeLine:
		warning("MacShape::render(): Line drawing not yet implemented");
		break;
	}

	_contentIsDirty = true;
}

} // end of namespace Graphics
