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

#include "common/config-manager.h"

#include "engines/myst3/transition.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "graphics/colormasks.h"

namespace Myst3 {

Transition *Transition::initialize(Myst3Engine *vm, TransitionType type) {
	return new Transition(vm, type);
}

Transition::Transition(Myst3Engine *vm, TransitionType type) :
		_vm(vm),
		_type(type),
		_sourceScreenshot(nullptr) {

	// Capture a screenshot of the source node
	if (type != kTransitionNone && computeDuration() != 0) {
		_sourceScreenshot = _vm->_gfx->getScreenshot();
	}
}

Transition::~Transition() {
	if (_sourceScreenshot) {
		_sourceScreenshot->free();
		delete _sourceScreenshot;
	}
}

int Transition::computeDuration() {
	int durationFrames = 30 * (100 - ConfMan.getInt("transition_speed")) / 100;
	if (_type == kTransitionZip) {
		durationFrames >>= 1;
	}

	return durationFrames;
}

void Transition::playSound() {
	if (_vm->_state->getTransitionSound()) {
		_vm->_sound->playEffect(_vm->_state->getTransitionSound(),
				_vm->_state->getTransitionSoundVolume());
	}
	_vm->_state->setTransitionSound(0);
}

void Transition::draw() {
	// Play the transition sound
	playSound();

	// Got any transition to draw?
	if (!_sourceScreenshot) {
		return;
	}

	// Capture a screenshot of the destination node
	_vm->drawFrame(true);
	Graphics::Surface *target = _vm->_gfx->getScreenshot();

	Texture *sourceTexture = _vm->_gfx->createTexture(_sourceScreenshot);
	Texture *targetTexture = _vm->_gfx->createTexture(target);

	target->free();
	delete target;

	// Compute the start and end frames for the animation
	int durationFrames = computeDuration();
	int startFrame = _vm->_state->getFrameCount();
	uint endFrame = startFrame + durationFrames;

	// Draw each step until completion
	int completion = 0;
	while (_vm->_state->getFrameCount() <= endFrame || completion < 100) {
		completion = CLIP<int>(100 * (_vm->_state->getFrameCount() - startFrame) / durationFrames, 0, 100);

		drawStep(targetTexture, sourceTexture, completion);

		_vm->_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(10);
		_vm->_state->updateFrameCounters();
	}

	_vm->_gfx->freeTexture(sourceTexture);
	_vm->_gfx->freeTexture(targetTexture);
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
