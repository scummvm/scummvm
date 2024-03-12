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
	if (g_nancy->getGameType() <= kGameTypeNancy6) {
		// nancy2-6 have a fixed number of frames for the effect, which is defined in the SPEC chunk
		auto *specialEffectData = GetEngineData(SPEC);
		assert(specialEffectData);

		_numFrames = _type == kBlackout ? specialEffectData->fadeToBlackNumFrames : specialEffectData->crossDissolveNumFrames;
		_frameTime = _type == kBlackout ? specialEffectData->fadeToBlackFrameTime : _frameTime;

		// We use the type definitions in nancy7, which are 1-indexed
		++_type;
	}

	// nancy7 got rid of the SPEC chunk, and the data now contains the total amount of time
	// that the effect should run for instead.
	if (_rect.isEmpty()) {
		if (g_nancy->getGameType() <= kGameTypeNancy6 && _type == kCrossDissolve) {
			// Earlier games did the whole screen (most easily testable in the nancy3 intro if one moves the scrollbar)
			_rect = Common::Rect(640, 480);
		} else {
			const VIEW *viewportData = (const VIEW *)g_nancy->getEngineData("VIEW");
			assert(viewportData);

			_rect = viewportData->screenPosition;
		}
	}

	_drawSurface.create(_rect.width(), _rect.height(), g_nancy->_graphics->getScreenPixelFormat());
	moveTo(_rect);
	setTransparent(false);

	RenderObject::init();
}

void SpecialEffect::updateGraphics() {
	if (_numFrames) {
		// Early version with constant number of frames, linear interpolation
		if (g_nancy->getTotalPlayTime() > _nextFrameTime && _currentFrame < (int)_numFrames && isInitialized()) {
			++_currentFrame;
			_nextFrameTime += _frameTime;

			GraphicsManager::crossDissolve(_fadeFrom, _fadeTo, 255 * _currentFrame / _numFrames, _rect, _drawSurface);
			setVisible(true);
		}
	} else {
		// nancy7+ version, draws as many frames as possible, ease in/out interpolation
		if (_startTime == 0) {
			_startTime = g_nancy->getTotalPlayTime();

			// The original code times how long the first frame takes to draw,
			// divides the total time by that time, and uses the result to
			// decide how many frames need to be drawn. This results in highly
			// variable timing depending on the machine the game is played on.
			// On modern PCs, it even results in a divide by 0 that effectively
			// stops the special effect from playing. Using the original _totalTime
			// value results in the effect taking much longer than the developers
			// intended (as made obvious in the dog scare sequence in the beginning
			// of nancy7), so we manually shorten the timings to better match what
			// the developers would've seen when on their machines.
			_totalTime /= 2;
			_fadeToBlackTime /= 2;
		}

		if (g_nancy->getTotalPlayTime() > _startTime + _totalTime && (_type != kThroughBlack || _throughBlackStarted2nd)) {
			if (_currentFrame == 0) {
				// Ensure at least one dissolve frame is shown
				++_currentFrame;
				GraphicsManager::crossDissolve(_fadeFrom, _fadeTo, 128, _rect, _drawSurface);
				setVisible(true);
			}
		} else {
			// Use a curve for all fades. Not entirely accurate to the original engine,
			// since that pre-calculated the number of frames and did some exponent magic on them
			float alpha = (float)(g_nancy->getTotalPlayTime() - _startTime) / (float)_totalTime;
			bool start2nd = alpha > 1;
			alpha = alpha * alpha * (3.0 - 2.0 * alpha);
			alpha *= 255;
			GraphicsManager::crossDissolve(_fadeFrom, _fadeTo, alpha, _rect, _drawSurface);
			setVisible(true);
			++_currentFrame;

			if (start2nd && _type == kThroughBlack) {
				_throughBlackStarted2nd = true;
				_fadeFrom.clear();
				setVisible(false);
				g_nancy->_graphics->screenshotScreen(_fadeTo);
				setVisible(true);
				_startTime = g_nancy->getTotalPlayTime();
				_currentFrame = 0;
			}
		}
	}
}

void SpecialEffect::onSceneChange() {
	g_nancy->_graphics->screenshotScreen(_fadeFrom);
	_drawSurface.rawBlitFrom(_fadeFrom, _rect, Common::Point());
}

void SpecialEffect::afterSceneChange() {
	if (_fadeFrom.empty()) {
		return;
	}

	if (_type == kCrossDissolve) {
		g_nancy->_graphics->screenshotScreen(_fadeTo);
	} else {
		_fadeTo.create(640, 480, _drawSurface.format);
		_fadeTo.clear();
	}

	// Workaround for the way ManagedSurface handles transparency. Both pure black
	// and pure white appear in scenes with SpecialEffects, and those happen to be
	// the two default values transBlitFrom uses for transColor. By doing this,
	// transColor gets set to the one color guaranteed to not appear in any scene,
	// and transparency works correctly
	_fadeTo.setTransparentColor(g_nancy->_graphics->getTransColor());

	registerGraphics();
	_nextFrameTime = g_nancy->getTotalPlayTime() + _frameTime;
	_fadeToBlackEndTime = g_nancy->getTotalPlayTime() + _totalTime + _fadeToBlackTime;
	_initialized = true;
}

bool SpecialEffect::isDone() const {
	if (_type == kBlackout) {
		return g_nancy->getTotalPlayTime() > _fadeToBlackEndTime;
	} else {
		bool canFinish = (_type == kThroughBlack) ? _throughBlackStarted2nd : true;
		return _totalTime ? ((g_nancy->getTotalPlayTime() > _startTime + _totalTime) && (_currentFrame != 0) && canFinish) : (_currentFrame >= (int)_numFrames);
	}
}

} // End of namespace Misc
} // End of namespace Nancy
