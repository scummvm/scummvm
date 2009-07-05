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

AnimObj::AnimObj(DraciEngine *vm) : _vm(vm) {
	_id = kUnused;
	_z = 0;
	_playing = false;
	_looping = false;
	_delay = 0;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
}	

AnimObj::~AnimObj() {
	deleteFrames();
}

bool AnimObj::isLooping() {
	return _looping;
}

void AnimObj::setLooping(bool looping) {
	_looping = looping;
}

void AnimObj::setDelay(uint delay) {
	_delay = delay;
}

void AnimObj::nextFrame(bool force) {

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

uint AnimObj::nextFrameNum() {

	if ((_currentFrame == getFramesNum() - 1) && _looping)
		return 0;
	else
		return _currentFrame + 1;
}

void AnimObj::drawFrame(Surface *surface) {
	
	if (_frames.size() == 0)
		return;

	if (_id == kOverlayImage) {			
		_frames[_currentFrame]->draw(surface, false);
	}
	else {
		_frames[_currentFrame]->draw(surface, true);
	}
}

void AnimObj::setID(int id) {
	
	_id = id;
}

int AnimObj::getID() {
	
	return _id;
}

void AnimObj::setZ(uint z) {

	_z = z;
}

uint AnimObj::getZ() {
	
	return _z;
}

bool AnimObj::isPlaying() {

	return _playing;
}

void AnimObj::setPlaying(bool playing) {
	
	_playing = playing;
}

void AnimObj::addFrame(Drawable *frame) {
	
	_frames.push_back(frame);	
}

uint AnimObj::getFramesNum() {
	
	return _frames.size();
}

void AnimObj::deleteFrames() {
	
	for (uint i = 0; i < getFramesNum(); ++i) {		
		delete _frames[i];
		_frames.pop_back();	
	}
}

AnimObj *Animation::addAnimation(int id, uint z, bool playing) {
	
	AnimObj *obj = new AnimObj(_vm);
	obj->setID(id);
	obj->setZ(z);
	obj->setPlaying(playing);
	obj->setLooping(false);

	insertAnimation(obj);

	return obj;
}

void Animation::play(int id) {

	AnimObj *obj = getAnimation(id);

	obj->setPlaying(true);
}

void Animation::stop(int id) {

	AnimObj *obj = getAnimation(id);

	obj->setPlaying(false);
}

AnimObj *Animation::getAnimation(int id) {
	
	Common::List<AnimObj *>::iterator it;

	for (it = _animObjects.begin(); it != _animObjects.end(); ++it) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}

	return *_animObjects.end();
}

void Animation::insertAnimation(AnimObj *animObj) {
	
	Common::List<AnimObj *>::iterator it;	

	for (it = _animObjects.begin(); it != _animObjects.end(); ++it) {
		if (animObj->getZ() < (*it)->getZ()) 
			break;
	}

	_animObjects.insert(it, animObj);
}

void Animation::addOverlay(Drawable *overlay, uint z) {
	AnimObj *obj = new AnimObj(_vm);
	obj->setID(kOverlayImage);
	obj->setZ(z);
	obj->setPlaying(true);
	obj->addFrame(overlay);

	insertAnimation(obj);
}

void Animation::drawScene(Surface *surf) {
	
	Common::List<AnimObj *>::iterator it;

	for (it = _animObjects.begin(); it != _animObjects.end(); ++it) {
		if (! ((*it)->isPlaying()) ) {
			continue;
		}	
	
		(*it)->nextFrame();
		(*it)->drawFrame(surf);
	}
}

void Animation::deleteAnimation(int id) {
	
	Common::List<AnimObj *>::iterator it;

	for (it = _animObjects.begin(); it != _animObjects.end(); ++it) {
		if ((*it)->getID() == id)
			break;
	}
	
	(*it)->deleteFrames();

	_animObjects.erase(it);
}
	
void Animation::deleteAll() {
	
	Common::List<AnimObj *>::iterator it;

	for (it = _animObjects.begin(); it != _animObjects.end(); ++it) {
		(*it)->deleteFrames();	
	}

	_animObjects.clear();
}

}
