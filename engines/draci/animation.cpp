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

Animation::Animation(DraciEngine *vm) : _vm(vm) {
	_id = kUnused;
	_z = 0;
	_playing = false;
	_looping = false;
	_delay = 0;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
}	

Animation::~Animation() {
	deleteFrames();
}

bool Animation::isLooping() {
	return _looping;
}

void Animation::setLooping(bool looping) {
	_looping = looping;
}

void Animation::setDelay(uint delay) {
	_delay = delay;
}

void Animation::nextFrame(bool force) {

	// If there's only one or no frames, return
	if (getFramesNum() < 2)
		return;

	Common::Rect frameRect = _frames[_currentFrame]->getRect();

	// If we are at the last frame and not looping, stop the animation
	// The animation is also restarted to frame zero
	if ((_currentFrame == nextFrameNum() - 1) && !_looping) {
		_currentFrame = 0;
		_playing = false;
		return;
	}

	if (force || (_tick + _delay <= _vm->_system->getMillis())) {
		_vm->_screen->getSurface()->markDirtyRect(frameRect);
		_currentFrame = nextFrameNum();
		_tick = _vm->_system->getMillis();
	}

	debugC(6, kDraciAnimationDebugLevel, 
	"tick=%d delay=%d tick+delay=%d currenttime=%d frame=%d framenum=%d", 
	_tick, _delay, _tick + _delay, _vm->_system->getMillis(), _currentFrame, _frames.size());
}

uint Animation::nextFrameNum() {

	if ((_currentFrame == getFramesNum() - 1) && _looping)
		return 0;
	else
		return _currentFrame + 1;
}

void Animation::drawFrame(Surface *surface) {
	
	if (_frames.size() == 0)
		return;

	if (_id == kOverlayImage) {			
		_frames[_currentFrame]->draw(surface, false);
	}
	else {
		_frames[_currentFrame]->draw(surface, true);
	}
}

void Animation::setID(int id) {
	
	_id = id;
}

int Animation::getID() {
	
	return _id;
}

void Animation::setZ(uint z) {

	_z = z;
}

uint Animation::getZ() {
	
	return _z;
}

bool Animation::isPlaying() {

	return _playing;
}

void Animation::setPlaying(bool playing) {
	
	_playing = playing;
}

void Animation::addFrame(Drawable *frame) {
	
	_frames.push_back(frame);	
}

uint Animation::getFramesNum() {
	
	return _frames.size();
}

void Animation::deleteFrames() {
	
	for (uint i = 0; i < getFramesNum(); ++i) {		
		delete _frames[i];
		_frames.pop_back();	
	}
}

Animation *AnimationManager::addAnimation(int id, uint z, bool playing) {
	
	Animation *anim = new Animation(_vm);
	
	anim->setID(id);
	anim->setZ(z);
	anim->setPlaying(playing);
	anim->setLooping(false);

	insertAnimation(anim);

	return anim;
}

void AnimationManager::play(int id) {

	Animation *anim = getAnimation(id);

	anim->setPlaying(true);
}

void AnimationManager::stop(int id) {

	Animation *anim = getAnimation(id);

	anim->setPlaying(false);
}

Animation *AnimationManager::getAnimation(int id) {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}

	return *_animations.end();
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
	Animation *anim = new Animation(_vm);
	anim->setID(kOverlayImage);
	anim->setZ(z);
	anim->setPlaying(true);
	anim->addFrame(overlay);

	insertAnimation(anim);
}

void AnimationManager::drawScene(Surface *surf) {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (! ((*it)->isPlaying()) ) {
			continue;
		}	
	
		(*it)->nextFrame();
		(*it)->drawFrame(surf);
	}
}

void AnimationManager::deleteAnimation(int id) {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id)
			break;
	}
	
	(*it)->deleteFrames();

	_animations.erase(it);
}
	
void AnimationManager::deleteAll() {
	
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		(*it)->deleteFrames();	
	}

	_animations.clear();
}

}
