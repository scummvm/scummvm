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

#include "common/events.h"
#include "common/config-manager.h"

#include "engines/myst3/transition.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

namespace Myst3 {

Transition::Transition(Myst3Engine *vm) :
		_vm(vm),
		_type(kTransitionNone),
		_sourceScreenshot(nullptr),
		_frameLimiter(new FrameLimiter(g_system, ConfMan.getInt("engine_speed"))) {

	// Capture a screenshot of the source node
	int durationTicks = computeDuration();
	if (durationTicks) {
		_sourceScreenshot = _vm->_gfx->copyScreenshotToTexture();
	}
}

Transition::~Transition() {
	_vm->_gfx->freeTexture(_sourceScreenshot);

	delete _frameLimiter;
}

int Transition::computeDuration() {
	int durationTicks = 30 * (100 - ConfMan.getInt("transition_speed")) / 100;
	if (_type == kTransitionZip) {
		durationTicks >>= 1;
	}

	return durationTicks;
}

void Transition::playSound() {
	if (_vm->_state->getTransitionSound()) {
		_vm->_sound->playEffect(_vm->_state->getTransitionSound(),
				_vm->_state->getTransitionSoundVolume());
	}
	_vm->_state->setTransitionSound(0);
}

void Transition::draw(TransitionType type) {
	_type = type;

	// Play the transition sound
	playSound();

	int durationTicks = computeDuration();

	// Got any transition to draw?
	if (!_sourceScreenshot || type == kTransitionNone || durationTicks == 0) {
		return;
	}

	// Capture a screenshot of the destination node
	_vm->drawFrame(true);
	Texture *targetScreenshot = _vm->_gfx->copyScreenshotToTexture();

	// Compute the start and end frames for the animation
	int startTick = _vm->_state->getTickCount();
	uint endTick = startTick + durationTicks;

	// Draw on the full screen
	_vm->_gfx->selectTargetWindow(nullptr, false, false);

	// Draw each step until completion
	int completion = 0;
	while ((_vm->_state->getTickCount() <= endTick || completion < 100) && !_vm->shouldQuit()) {
		_frameLimiter->startFrame();

		completion = CLIP<int>(100 * (_vm->_state->getTickCount() - startTick) / durationTicks, 0, 100);

		_vm->_gfx->clear();

		drawStep(targetScreenshot, _sourceScreenshot, completion);

		_vm->_gfx->flipBuffer();
		_frameLimiter->delayBeforeSwap();
		g_system->updateScreen();
		_vm->_state->updateFrameCounters();

		Common::Event event;
		while (_vm->getEventManager()->pollEvent(event)) {
			// Ignore all the events happening during transitions, so that the view does not move
			// between the initial transition screen shoot and the first frame drawn after the transition.

			// However, keep updating the keyboard state so we don't end up in
			// an unbalanced state where the engine believes keys are still
			// pressed while they are not.
			_vm->processEventForKeyboardState(event);

			if (_vm->_state->hasVarGamePadUpPressed()) {
				_vm->processEventForGamepad(event);
			}
		}
	}

	_vm->_gfx->freeTexture(targetScreenshot);
	_vm->_gfx->freeTexture(_sourceScreenshot);
	_sourceScreenshot = nullptr;
}

void Transition::drawStep(Texture *targetTexture, Texture *sourceTexture, uint completion) {
	Common::Rect viewport = _vm->_gfx->viewport();

	switch (_type) {
	case kTransitionNone:
		break;

	case kTransitionFade:
	case kTransitionZip: {
			Common::Rect textureRect = Common::Rect(sourceTexture->width, sourceTexture->height);
			_vm->_gfx->drawTexturedRect2D(viewport, textureRect, sourceTexture);
			_vm->_gfx->drawTexturedRect2D(viewport, textureRect, targetTexture, completion / 100.0);
		}
		break;

	case kTransitionLeftToRight: {
			int16 transitionX = (viewport.width() * (100 - completion)) / 100;
			Common::Rect sourceTextureRect(0, 0, transitionX, sourceTexture->height);
			Common::Rect sourceScreenRect(sourceTextureRect.width(), sourceTextureRect.height());
			sourceScreenRect.translate(viewport.left, viewport.top);

			Common::Rect targetTextureRect(transitionX, 0, targetTexture->width, targetTexture->height);
			Common::Rect targetScreenRect(targetTextureRect.width(), targetTextureRect.height());
			targetScreenRect.translate(viewport.left + transitionX, viewport.top);

			_vm->_gfx->drawTexturedRect2D(sourceScreenRect, sourceTextureRect, sourceTexture);
			_vm->_gfx->drawTexturedRect2D(targetScreenRect, targetTextureRect, targetTexture);
		}
		break;

	case kTransitionRightToLeft: {
			int16 transitionX = viewport.width() * completion / 100;
			Common::Rect sourceTextureRect(transitionX, 0, sourceTexture->width, sourceTexture->height);
			Common::Rect sourceScreenRect(sourceTextureRect.width(), sourceTextureRect.height());
			sourceScreenRect.translate(viewport.left + transitionX, viewport.top);

			Common::Rect targetTextureRect(0, 0, transitionX, targetTexture->height);
			Common::Rect targetScreenRect(targetTextureRect.width(), targetTextureRect.height());
			targetScreenRect.translate(viewport.left, viewport.top);

			_vm->_gfx->drawTexturedRect2D(sourceScreenRect, sourceTextureRect, sourceTexture);
			_vm->_gfx->drawTexturedRect2D(targetScreenRect, targetTextureRect, targetTexture);
		}
		break;
	}
}

} // End of namespace Myst3
