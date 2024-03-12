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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/mouselightpuzzle.h"

namespace Nancy {
namespace Action {

void MouseLightPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();

	Graphics::ManagedSurface baseImage;
	g_nancy->_resource->loadImage(_imageName, baseImage);

	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.blitFrom(baseImage);
	((Graphics::Surface)_drawSurface).setAlpha(0);

	setVisible(true);
	moveTo(screenBounds);

	_maskCircle.create(_radius * 2, _radius * 2, g_nancy->_graphics->getInputPixelFormat());
	_maskCircle.clear();

	if (_smoothEdges) {
		for (int y = -_radius; y < _radius; ++y) {
			for (int x = -_radius; x < _radius; ++x) {
				_maskCircle.setPixel(x + _radius, y + _radius, (uint16)(expf(-(float)((y * y + x * x) * (y * y + x * x)) / (float)(_radius * _radius * _radius * _radius / 4)) * 0xFF));
			}
		}
	} else {
		for (int y = -_radius; y < _radius; ++y) {
			for (int x = -_radius; x < _radius; ++x) {
				if (sqrt(y * y + x * x) < _radius) {
					_maskCircle.setPixel(x + _radius, y + _radius, 0xFF);
				}
			}
		}
	}
}

void MouseLightPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}
}

void MouseLightPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	_radius = stream.readByte();
	_smoothEdges = stream.readByte();
}

void MouseLightPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	if (_lastMousePos == input.mousePos) {
		return;
	}

	_lastMousePos = input.mousePos;
	((Graphics::Surface)_drawSurface).setAlpha(0);
	_needsRedraw = true;

	Common::Rect vpScreenPos = NancySceneState.getViewport().convertViewportToScreen(_screenPosition);
	if (!vpScreenPos.contains(input.mousePos)) {
		return;
	}

	Common::Point blitDestPoint = input.mousePos;
	blitDestPoint -= { vpScreenPos.left, vpScreenPos.top };
	blitDestPoint -= { _radius, _radius };

	Common::Rect srcRect = _maskCircle.getBounds();
	Common::Rect::getBlitRect(blitDestPoint, srcRect, _drawSurface.getBounds());

	// Copy over the transparency to the draw surface
	for (int y = srcRect.top; y < srcRect.bottom; ++y) {
		uint32 *drawSurfPtr = (uint32 *)_drawSurface.getBasePtr(blitDestPoint.x, y + blitDestPoint.y - srcRect.top);
		uint16 *circlePtr = (uint16 *)_maskCircle.getBasePtr(srcRect.left, y);
		for (int x = srcRect.left; x < srcRect.right; ++x) {
			*drawSurfPtr = (*drawSurfPtr & 0xFFFFFF00) | (byte)*circlePtr;
			++drawSurfPtr;
			++circlePtr;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
