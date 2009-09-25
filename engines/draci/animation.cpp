
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
	_relX = 0;
	_relY = 0;
	_scaleX = 1.0;
	_scaleY = 1.0;
	_playing = false;
	_looping = false;
	_paused = false;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
	_callback = &Animation::doNothing;
}	

Animation::~Animation() {
	deleteFrames();
}

bool Animation::isLooping() const {
	return _looping;
}

void Animation::setRelative(int relx, int rely) {

	// Delete the previous frame if there is one
	if (_frames.size() > 0)	
		markDirtyRect(_vm->_screen->getSurface());

	_relX = relx;
	_relY = rely;
}

void Animation::setLooping(bool looping) {
	_looping = looping;
	debugC(7, kDraciAnimationDebugLevel, "Setting looping to %d on animation %d", 
		looping, _id);
}

void Animation::markDirtyRect(Surface *surface) const {
	// Fetch the current frame's rectangle
	Drawable *frame = _frames[_currentFrame];
	Common::Rect frameRect = frame->getRect();			

	// Translate rectangle to compensate for relative coordinates	
	frameRect.translate(_relX, _relY);
	
	// Take animation scaling into account
	frameRect.setWidth((int) (frameRect.width() * _scaleX));
	frameRect.setHeight((int) (frameRect.height() * _scaleY));

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
		}
	}

	debugC(6, kDraciAnimationDebugLevel, 
	"anim=%d tick=%d delay=%d tick+delay=%d currenttime=%d frame=%d framenum=%d x=%d y=%d", 
	_id, _tick, frame->getDelay(), _tick + frame->getDelay(), _vm->_system->getMillis(), 
	_currentFrame, _frames.size(), frame->getX(), frame->getY());
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

	Drawable *frame = _frames[_currentFrame];

	if (_id == kOverlayImage) {			
		frame->draw(surface, false);
	} else {

		int x = frame->getX();
		int y = frame->getY();

		// Take account relative coordinates
		int newX = x + _relX;
		int newY = y + _relY;

		// Translate the frame to those relative coordinates
		frame->setX(newX);
		frame->setY(newY);

		// Save scaled width and height
		int scaledWidth = frame->getScaledWidth();
		int scaledHeight = frame->getScaledHeight();

		// Take into account per-animation scaling and adjust the current frames dimensions	
		if (_scaleX != 1.0 || _scaleY != 1.0)
			frame->setScaled(
				(int) (scaledWidth * _scaleX),
				(int) (scaledHeight * _scaleY));

		// Draw frame
		frame->drawScaled(surface, false);

		// Revert back to old coordinates
		frame->setX(x);
		frame->setY(y);

		// Revert back to old dimensions
		frame->setScaled(scaledWidth, scaledHeight);
	}
}

void Animation::setID(int id) {
	_id = id;
}

int Animation::getID() const {
	return _id;
}

void Animation::setZ(uint z) {
	_z = z;
}

uint Animation::getZ() const {
	return _z;
}

int Animation::getRelativeX() const {
	return _relX;
}

int Animation::getRelativeY() const {
	return _relY;
}

bool Animation::isPlaying() const {
	return _playing;
}

void Animation::setPlaying(bool playing) {
	_tick = _vm->_system->getMillis();
	_playing = playing;
}

bool Animation::isPaused() const {
	return _paused;
}

void Animation::setPaused(bool paused) {
	_paused = paused;
}

void Animation::setScaleFactors(double scaleX, double scaleY) {
	
	debugC(5, kDraciAnimationDebugLevel, 
		"Setting scaling factors on anim %d (scaleX: %.3f scaleY: %.3f)", 
		_id, scaleX, scaleY);

	markDirtyRect(_vm->_screen->getSurface());	
	
	_scaleX = scaleX;
	_scaleY = scaleY;
}

double Animation::getScaleX() const {
	return _scaleX;
}

double Animation::getScaleY() const {
	return _scaleY;
}

void Animation::addFrame(Drawable *frame) {
	_frames.push_back(frame);	
}

int Animation::getIndex() const {
	return _index;
}

void Animation::setIndex(int index) {
	_index = index;
}

Drawable *Animation::getFrame(int frameNum) {

	// If there are no frames stored, return NULL
	if (_frames.size() == 0) {
		return NULL;
	}

	// If no argument is passed, return the current frame
	if (frameNum == kCurrentFrame) {
		return _frames[_currentFrame];
	} else {
		return _frames[frameNum];
	}
}

uint Animation::getFrameCount() const {
	return _frames.size();
}

uint Animation::currentFrameNum() const {
	return _currentFrame;
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
}

void Animation::stopAnimation() { 
	_vm->_anims->stop(_id);
}

void Animation::exitGameLoop() { 
	_vm->_game->setExitLoop(true);
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

void AnimationManager::insertAnimation(Animation *animObj) {
	
	Common::List<Animation *>::iterator it;	

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (animObj->getZ() < (*it)->getZ()) 
			break;
	}

	_animations.insert(it, animObj);
}

void AnimationManager::addOverlay(Drawable *overlay, uint z) {
	// Since this is an overlay, we don't need it to be deleted 
	// when the GPL Release command is invoked so we pass the index
	// as kIgnoreIndex
	Animation *anim = new Animation(_vm, kIgnoreIndex);
	
	anim->setID(kOverlayImage);
	anim->setZ(z);
	anim->setPlaying(true);
	anim->addFrame(overlay);

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
	
		(*it)->nextFrame();
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

	while(1) {
		next = cur;
		next++;

		// If we are at the last element, we're done
		if (next == _animations.end())
			break;

		// If we find an animation out of order, reinsert it
		if ((*next)->getZ() < (*cur)->getZ()) {

			Animation *anim = *next;
			_animations.erase(next);

			insertAnimation(anim);
		}

		// Advance to next animation
		cur = next;
	}
}

void AnimationManager::deleteAnimation(int id) {
	
	Common::List<Animation *>::iterator it;
  
	int index = -1;

	// Iterate for the first time to delete the animation
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			(*it)->deleteFrames();
			_animations.erase(it);

			// Remember index of the deleted animation
			index = (*it)->getIndex();

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", id);

			break;
		}
	}

	// Iterate the second time to decrease indexes greater than the deleted animation index
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() == index && (*it)->getIndex() != kIgnoreIndex) {
			(*it)->setIndex(index-1);
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
			(*it)->deleteFrames();
			_animations.erase(it);
		}	
	}
}

void AnimationManager::deleteAll() {

	debugC(3, kDraciAnimationDebugLevel, "Deleting all animations...");

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		(*it)->deleteFrames();	
	}

	_animations.clear();

	_lastIndex = -1;
}

int AnimationManager::getLastIndex() const {
	return _lastIndex;
}

void AnimationManager::deleteAfterIndex(int index) {

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() > index) {

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", (*it)->getID());

			(*it)->deleteFrames();
			_animations.erase(it);
		}
	}
	
	_lastIndex = index;
}

int AnimationManager::getTopAnimationID(int x, int y) {

	Common::List<Animation *>::iterator it;

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

		Drawable *frame = anim->getFrame();

		if (frame == NULL) {
			continue;
		}

		int oldX = frame->getX();
		int oldY = frame->getY();

		// Take account relative coordinates
		int newX = oldX + anim->getRelativeX();
		int newY = oldY + anim->getRelativeY();

		// Translate the frame to those relative coordinates
		frame->setX(newX);
		frame->setY(newY);

		// Save scaled width and height
		int scaledWidth = frame->getScaledWidth();
		int scaledHeight = frame->getScaledHeight();

		// Take into account per-animation scaling and adjust the current frames dimensions	
		if (anim->getScaleX() != 1.0 || anim->getScaleY() != 1.0)
			frame->setScaled(
				(int) (scaledWidth * anim->getScaleX()),
				(int) (scaledHeight * anim->getScaleY()));

		if (frame->getRect().contains(x, y)) {

			if (frame->getType() == kDrawableText) {

				retval = anim->getID();

			} else if (frame->getType() == kDrawableSprite && 
					   reinterpret_cast<Sprite *>(frame)->getPixel(x, y) != transparent) {

				retval = anim->getID();
			}	
		}

		// Revert back to old coordinates
		frame->setX(oldX);
		frame->setY(oldY);

		// Revert back to old dimensions
		frame->setScaled(scaledWidth, scaledHeight);

		// Found an animation
		if (retval != kOverlayImage)
			break;
	}

	return retval;
}
			
}
