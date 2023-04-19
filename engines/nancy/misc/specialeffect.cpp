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

#include "engines/nancy/misc/specialeffect.h"

namespace Nancy {
namespace Misc {

void SpecialEffect::init() {
	_specialEffectData = g_nancy->_specialEffectData;
	assert(_specialEffectData);

	_numFrames = _type == kSceneChangeFadeOutToBlack ? _specialEffectData->fadeToBlackNumFrames : _specialEffectData->crossDissolveNumFrames;
	_frameTime = _type == kSceneChangeFadeOutToBlack ? _specialEffectData->fadeToBlackFrameTime : _frameTime;

	_drawSurface.create(g_nancy->_viewportData->bounds.width(), g_nancy->_viewportData->bounds.height(), g_nancy->_graphicsManager->getScreenPixelFormat());
	moveTo(g_nancy->_viewportData->screenPosition);
	setTransparent(false);

	RenderObject::init();
}

void SpecialEffect::updateGraphics() {
	if (g_nancy->getTotalPlayTime() > _nextFrameTime && _currentFrame < _numFrames) {
		++_currentFrame;
		_nextFrameTime += _frameTime;

		GraphicsManager::crossDissolve(_fadeFrom, _fadeTo, 255 * _currentFrame / _numFrames, _drawSurface);
		setVisible(true);
	}
}

void SpecialEffect::onSceneChange() {
	g_nancy->_graphicsManager->screenshotViewport(_fadeFrom);
	_drawSurface.rawBlitFrom(_fadeFrom, _fadeFrom.getBounds(), Common::Point());
}

void SpecialEffect::afterSceneChange() {
	if (_type == kSceneChangeFadeCrossDissolve) {
		g_nancy->_graphicsManager->screenshotViewport(_fadeTo);
	} else {
		_fadeTo.create(_drawSurface.w, _drawSurface.h, _drawSurface.format);
		_fadeTo.clear();
	}

	// Workaround for the way ManagedSurface handles transparency. Both pure black
	// and pure white appear in scenes with SpecialEffects, and those happen to be
	// the two default values transBlitFrom uses for transColor. By doing this
	// transColor gets set to the one color guaranteed to not appear in any scene,
	// and transparency works correctly
	_fadeTo.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
	
	registerGraphics();
	_nextFrameTime = g_nancy->getTotalPlayTime() + _frameTime;
	_fadeToBlackEndTime = g_nancy->getTotalPlayTime() + _fadeToBlackTime;
	_initialized = true;
}

bool SpecialEffect::isDone() const {
	if (_type == kSceneChangeFadeCrossDissolve) {
		return _currentFrame >= _numFrames;
	} else {
		return g_nancy->getTotalPlayTime() > _fadeToBlackEndTime;
	}
}

} // End of namespace Misc
} // End of namespace Nancy
