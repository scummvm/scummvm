
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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "draci/draci.h"
#include "draci/animation.h"

namespace Draci {

Animation::Animation(DraciEngine *vm, int index) : _vm(vm) {
	_id = kUnused;
	_index = index;
	_z = 0;
	_displacement = kNoDisplacement;
	_playing = false;
	_looping = false;
	_paused = false;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
	_hasChangedFrame = true;
	_callback = &Animation::doNothing;
}

Animation::~Animation() {
	deleteFrames();
}

void Animation::setRelative(int relx, int rely) {
	// Delete the previous frame if there is one
	if (_frames.size() > 0)
		markDirtyRect(_vm->_screen->getSurface());

	_displacement.relX = relx;
	_displacement.relY = rely;
}

void Animation::setLooping(bool looping) {
	_looping = looping;
	debugC(7, kDraciAnimationDebugLevel, "Setting looping to %d on animation %d",
		looping, _id);
}

void Animation::markDirtyRect(Surface *surface) const {
	// Fetch the current frame's rectangle
	Drawable *frame = _frames[_currentFrame];
	Common::Rect frameRect = frame->getRect(_displacement);

	// Mark the rectangle dirty on the surface
	surface->markDirtyRect(frameRect);
}

void Animation::nextFrame(bool force) {
	// If there are no frames or if the animation is not playing, return
	if (getFrameCount() == 0 || !_playing)
		return;

	Drawable *frame = _frames[_currentFrame];
	Surface *surface = _vm->_screen->getSurface();

	if (force || (_tick + frame->getDelay() <= _vm->_system->getMillis())) {
		// If we are at the last frame and not looping, stop the animation
		// The animation is also restarted to frame zero
		if ((_currentFrame == getFrameCount() - 1) && !_looping) {
			// When the animation reaches its end, call the preset callback
			(this->*_callback)();
		} else {
			// Mark old frame dirty so it gets deleted
			markDirtyRect(surface);

			_currentFrame = nextFrameNum();
			_tick = _vm->_system->getMillis();

			// Fetch new frame and mark it dirty
			markDirtyRect(surface);

			_hasChangedFrame = true;
		}
	}

	debugC(6, kDraciAnimationDebugLevel,
	"anim=%d tick=%d delay=%d tick+delay=%d currenttime=%d frame=%d framenum=%d x=%d y=%d z=%d",
	_id, _tick, frame->getDelay(), _tick + frame->getDelay(), _vm->_system->getMillis(),
	_currentFrame, _frames.size(), frame->getX() + getRelativeX(), frame->getY() + getRelativeY(), _z);
}

uint Animation::nextFrameNum() const {
	if (_paused)
		return _currentFrame;

	if ((_currentFrame == getFrameCount() - 1) && _looping)
		return 0;
	else
		return _currentFrame + 1;
}

void Animation::drawFrame(Surface *surface) {
	// If there are no frames or the animation is not playing, return
	if (_frames.size() == 0 || !_playing)
		return;

	const Drawable *frame = _frames[_currentFrame];

	if (_id == kOverlayImage) {
		frame->draw(surface, false, 0, 0);
	} else {
		// Draw frame
		frame->drawReScaled(surface, false, _displacement);
	}

	const SoundSample *sample = _samples[_currentFrame];
	if (_hasChangedFrame && sample) {
		debugC(3, kDraciSoundDebugLevel,
			"Playing sample on animation %d, frame %d: %d+%d at %dHz",
			_id, _currentFrame, sample->_offset, sample->_length, sample->_frequency);
		_vm->_sound->playSound(sample, Audio::Mixer::kMaxChannelVolume, false);
	}
	_hasChangedFrame = false;
}

void Animation::setPlaying(bool playing) {
	_tick = _vm->_system->getMillis();
	_playing = playing;

	// When restarting an animation, allow playing sounds.
	_hasChangedFrame |= playing;
}

void Animation::setScaleFactors(double scaleX, double scaleY) {
	debugC(5, kDraciAnimationDebugLevel,
		"Setting scaling factors on anim %d (scaleX: %.3f scaleY: %.3f)",
		_id, scaleX, scaleY);

	markDirtyRect(_vm->_screen->getSurface());

	_displacement.extraScaleX = scaleX;
	_displacement.extraScaleY = scaleY;
}

void Animation::addFrame(Drawable *frame, const SoundSample *sample) {
	_frames.push_back(frame);
	_samples.push_back(sample);
}

void Animation::replaceFrame(int i, Drawable *frame, const SoundSample *sample) {
	_frames[i] = frame;
	_samples[i] = sample;
}

Drawable *Animation::getCurrentFrame() {
	// If there are no frames stored, return NULL
	return _frames.size() > 0 ? _frames[_currentFrame] : NULL;
}

Drawable *Animation::getFrame(int frameNum) {
	// If there are no frames stored, return NULL
	return _frames.size() > 0 ? _frames[frameNum] : NULL;
}

void Animation::setCurrentFrame(uint frame) {
	// Check whether the value is sane
	if (frame >= _frames.size()) {
		return;
	}

	_currentFrame = frame;
}

void Animation::deleteFrames() {
	// If there are no frames to delete, return
	if (_frames.size() == 0) {
		return;
	}

	markDirtyRect(_vm->_screen->getSurface());

	for (int i = getFrameCount() - 1; i >= 0; --i) {
		delete _frames[i];
		_frames.pop_back();
	}
	_samples.clear();
}

void Animation::stopAnimation() {
	_vm->_anims->stop(_id);
}

void Animation::exitGameLoop() {
	_vm->_game->setExitLoop(true);
}

void Animation::tellWalkingState() {
	_vm->_game->heroAnimationFinished();
}

Animation *AnimationManager::addAnimation(int id, uint z, bool playing) {
	// Increment animation index
	++_lastIndex;

	Animation *anim = new Animation(_vm, _lastIndex);

	anim->setID(id);
	anim->setZ(z);
	anim->setPlaying(playing);

	insertAnimation(anim);

	return anim;
}

Animation *AnimationManager::addItem(int id, bool playing) {
	Animation *anim = new Animation(_vm, kIgnoreIndex);

	anim->setID(id);
	anim->setZ(256);
	anim->setPlaying(playing);

	insertAnimation(anim);

	return anim;
}

Animation *AnimationManager::addText(int id, bool playing) {
	Animation *anim = new Animation(_vm, kIgnoreIndex);

	anim->setID(id);
	anim->setZ(257);
	anim->setPlaying(playing);

	insertAnimation(anim);

	return anim;
}

void AnimationManager::play(int id) {
	Animation *anim = getAnimation(id);

	if (anim) {
		// Mark the first frame dirty so it gets displayed
		anim->markDirtyRect(_vm->_screen->getSurface());

		anim->setPlaying(true);

		debugC(3, kDraciAnimationDebugLevel, "Playing animation %d...", id);
	}
}

void AnimationManager::stop(int id) {
	Animation *anim = getAnimation(id);

	if (anim) {
		// Clean up the last frame that was drawn before stopping
		anim->markDirtyRect(_vm->_screen->getSurface());

		anim->setPlaying(false);

		// Reset the animation to the beginning
		anim->setCurrentFrame(0);

		debugC(3, kDraciAnimationDebugLevel, "Stopping animation %d...", id);
	}
}

void AnimationManager::pauseAnimations() {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() > 0 || (*it)->getID() == kTitleText) {
			// Clean up the last frame that was drawn before stopping
			(*it)->markDirtyRect(_vm->_screen->getSurface());

			(*it)->setPaused(true);
		}
	}
}

void AnimationManager::unpauseAnimations() {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->isPaused()) {
			// Clean up the last frame that was drawn before stopping
			(*it)->markDirtyRect(_vm->_screen->getSurface());

			(*it)->setPaused(false);
		}
	}
}

Animation *AnimationManager::getAnimation(int id) {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}

	return NULL;
}

void AnimationManager::insertAnimation(Animation *anim) {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (anim->getZ() < (*it)->getZ())
			break;
	}

	_animations.insert(it, anim);
}

void AnimationManager::addOverlay(Drawable *overlay, uint z) {
	// Since this is an overlay, we don't need it to be deleted
	// when the GPL Release command is invoked so we pass the index
	// as kIgnoreIndex
	Animation *anim = new Animation(_vm, kIgnoreIndex);

	anim->setID(kOverlayImage);
	anim->setZ(z);
	anim->setPlaying(true);
	anim->addFrame(overlay, NULL);

	insertAnimation(anim);
}

void AnimationManager::drawScene(Surface *surf) {
	// Fill the screen with colour zero since some rooms may rely on the screen being black
	_vm->_screen->getSurface()->fill(0);

	sortAnimations();

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (! ((*it)->isPlaying()) ) {
			continue;
		}

		(*it)->nextFrame(false);
		(*it)->drawFrame(surf);
	}
}

void AnimationManager::sortAnimations() {
	Common::List<Animation *>::iterator cur;
	Common::List<Animation *>::iterator next;

	cur = _animations.begin();

	// If the list is empty, we're done
	if (cur == _animations.end())
		return;

	bool hasChanged;

	do {
		hasChanged = false;
		cur = _animations.begin();
		next = cur;

		while (true) {
			next++;

			// If we are at the last element, we're done
			if (next == _animations.end())
				break;

			// If we find an animation out of order, reinsert it
			if ((*next)->getZ() < (*cur)->getZ()) {

				Animation *anim = *next;
				next = _animations.reverse_erase(next);

				insertAnimation(anim);
				hasChanged = true;
			}

			// Advance to next animation
			cur = next;
		}
	} while (hasChanged);
}

void AnimationManager::deleteAnimation(int id) {
	Common::List<Animation *>::iterator it;

	int index = -1;

	// Iterate for the first time to delete the animation
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			// Remember index of the deleted animation
			index = (*it)->getIndex();

			delete *it;
			_animations.erase(it);

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", id);

			break;
		}
	}

	// Iterate the second time to decrease indexes greater than the deleted animation index
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() > index && (*it)->getIndex() != kIgnoreIndex) {
			(*it)->setIndex((*it)->getIndex() - 1);
		}
	}

	// Decrement index of last animation
	_lastIndex -= 1;
}

void AnimationManager::deleteOverlays() {
	debugC(3, kDraciAnimationDebugLevel, "Deleting overlays...");

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == kOverlayImage) {
			delete *it;
			it = _animations.reverse_erase(it);
		}
	}
}

void AnimationManager::deleteAll() {
	debugC(3, kDraciAnimationDebugLevel, "Deleting all animations...");

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		delete *it;
	}

	_animations.clear();

	_lastIndex = -1;
}

void AnimationManager::deleteAfterIndex(int index) {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() > index) {

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", (*it)->getID());

			delete *it;
			it = _animations.reverse_erase(it);
		}
	}

	_lastIndex = index;
}

int AnimationManager::getTopAnimationID(int x, int y) const {
	Common::List<Animation *>::const_iterator it;

	// The default return value if no animations were found on these coordinates (not even overlays)
	// i.e. the black background shows through so treat it as an overlay
	int retval = kOverlayImage;

	// Get transparent colour for the current screen
	const int transparent = _vm->_screen->getSurface()->getTransparentColour();

	for (it = _animations.reverse_begin(); it != _animations.end(); --it) {

		Animation *anim = *it;

		// If the animation is not playing, ignore it
		if (!anim->isPlaying() || anim->isPaused()) {
			continue;
		}

		const Drawable *frame = anim->getCurrentFrame();

		if (frame == NULL) {
			continue;
		}

		if (frame->getRect(anim->getDisplacement()).contains(x, y)) {
			if (frame->getType() == kDrawableText) {

				retval = anim->getID();

			} else if (frame->getType() == kDrawableSprite &&
					   reinterpret_cast<const Sprite *>(frame)->getPixel(x, y, anim->getDisplacement()) != transparent) {

				retval = anim->getID();
			}
		}

		// Found an animation
		if (retval != kOverlayImage)
			break;
	}

	return retval;
}

}
