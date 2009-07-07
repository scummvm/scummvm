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
 
#ifndef DRACI_ANIMATION_H
#define DRACI_ANIMATION_H

#include "draci/sprite.h"

namespace Draci {

enum { kOverlayImage = -1, kUnused = -2 };

class DraciEngine;

class Animation {

public:
	Animation(DraciEngine *vm);
	~Animation();	
	
	uint getZ();
	void setZ(uint z);
	
	void setID(int id);
	int getID();

	void setDelay(uint delay);

	void nextFrame(bool force = false);
	void drawFrame(Surface *surface);

	void addFrame(Drawable *frame);
	uint getFramesNum();
	void deleteFrames();

	bool isPlaying();
	void setPlaying(bool playing);

	bool isLooping();
	void setLooping(bool looping);

private:
	
	uint nextFrameNum();

	int _id;	
	uint _currentFrame;
	uint _z;
	uint _delay;
	uint _tick;
	bool _playing;
	bool _looping;
	Common::Array<Drawable*> _frames;

	DraciEngine *_vm;
};


class AnimationManager {

public:
	AnimationManager(DraciEngine *vm) : _vm(vm) {};
	~AnimationManager() { deleteAll(); }

	Animation *addAnimation(int id, uint z, bool playing = false);
	void addOverlay(Drawable *overlay, uint z);
	
	void play(int id);
	void stop(int id);

	void deleteAnimation(int id);
	void deleteOverlays();
	void deleteAll();

	void drawScene(Surface *surf);

	Animation *getAnimation(int id);

private:
	
	void insertAnimation(Animation *anim);

	DraciEngine *_vm;
	Common::List<Animation *> _animations;
};

}

#endif // DRACI_ANIMATION_H
