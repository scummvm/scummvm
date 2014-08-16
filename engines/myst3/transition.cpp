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

	// Create the temporary surface and texture for the transition
	Graphics::Surface *frame = new Graphics::Surface();
	frame->create(target->w, target->h, target->format);
	Common::Rect frameRect = Common::Rect(frame->w, frame->h);

	Texture *frameTexture = _vm->_gfx->createTexture(frame);

	// Compute the start and end frames for the animation
	int durationFrames = computeDuration();
	int startFrame = _vm->_state->getFrameCount();
	uint endFrame = startFrame + durationFrames;

	// Draw each step until completion
	int completion = 0;
	while (_vm->_state->getFrameCount() <= endFrame || completion < 100) {
		completion = CLIP<int>(100 * (_vm->_state->getFrameCount() - startFrame) / durationFrames, 0, 100);

		uint32 *targetPtr = (uint32 *)target->getPixels();
		uint32 *sourcePtr = (uint32 *)_sourceScreenshot->getPixels();
		uint32 *framePtr = (uint32 *)frame->getPixels();
		drawStep(targetPtr, target->pitch, sourcePtr, _sourceScreenshot->pitch, framePtr, frame->pitch, frame->h, completion);

		frameTexture->update(frame);
		_vm->_gfx->drawTexturedRect2D(frameRect, frameRect, frameTexture);
		
		_vm->_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(10);
		_vm->_state->updateFrameCounters();
	}

	_vm->_gfx->freeTexture(frameTexture);

	frame->free();
	delete frame;

	target->free();
	delete target;
}

void Transition::drawStep(uint32 *target, uint targetPitch, uint32 *source, uint sourcePitch, uint32 *destination, uint destinationPitch, uint destinationHeight, uint completion) {
	switch (_type) {
	case kTransitionNone:
		break;

	case kTransitionFade:
	case kTransitionZip: {

			for (uint y = 0; y < destinationHeight; y++) {
				uint32 *sourcePtr = source + (destinationHeight - y - 1) * (sourcePitch / 4);
				uint32 *targetPtr = target + (destinationHeight - y - 1) * (targetPitch / 4);
				uint32 *destinationPtr = destination;

				for (uint x = 0; x < 640; x++) {
					byte sourceR, sourceG, sourceB;
					byte targetR, targetG, targetB;
					byte destR, destG, destB;

					Graphics::colorToRGB< Graphics::ColorMasks<8888> >(*sourcePtr++, sourceR, sourceG, sourceB);
					Graphics::colorToRGB< Graphics::ColorMasks<8888> >(*targetPtr++, targetR, targetG, targetB);

					// TODO: optimize ?
					destR = sourceR * (100 - completion) / 100 + targetR * completion / 100;
					destG = sourceG * (100 - completion) / 100 + targetG * completion / 100;
					destB = sourceB * (100 - completion) / 100 + targetB * completion / 100;

					*destinationPtr++ = Graphics::RGBToColor< Graphics::ColorMasks<8888> >(destR, destG, destB);
				}

				destination += (destinationPitch / 4);
			}
		}
		break;

	case kTransitionLeftToRight: {
			uint transitionX = (640 * 100 - 640 * completion) / 100;
			for (uint y = 0; y < destinationHeight; y++) {
				uint32 *sourcePtr = source + (destinationHeight - y - 1) * (sourcePitch / 4);
				uint32 *destinationPtr = destination;

				for (uint x = 0; x < transitionX; x++) {
					*destinationPtr = *sourcePtr;
					destinationPtr++;
					sourcePtr++;
				}

				uint32 *targetPtr = target + (destinationHeight - y - 1) * (targetPitch / 4) + transitionX;
				for (uint x = transitionX; x < 640; x++) {
					*destinationPtr = *targetPtr;
					destinationPtr++;
					targetPtr++;
				}

				destination += (destinationPitch / 4);
			}
		}
		break;

	case kTransitionRightToLeft: {
			uint transitionX = 640 * completion / 100;
			for (uint y = 0; y < destinationHeight; y++) {
				uint32 *targetPtr = target + (destinationHeight - y - 1) * (targetPitch / 4);
				uint32 *destinationPtr = destination;

				for (uint x = 0; x < transitionX; x++) {
					*destinationPtr = *targetPtr;
					destinationPtr++;
					targetPtr++;
				}

				uint32 *sourcePtr = source + (destinationHeight - y - 1) * (sourcePitch / 4) + transitionX;
				for (uint x = transitionX; x < 640; x++) {
					*destinationPtr = *sourcePtr;
					destinationPtr++;
					sourcePtr++;
				}

				destination += (destinationPitch / 4);
			}
		}
		break;
	}
}

} // End of namespace Myst3
