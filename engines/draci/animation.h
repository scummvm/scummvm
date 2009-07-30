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

/**
  * Animation IDs for those animations that don't have their IDs
  * specified in the data files.
  */
enum { kOverlayImage = -1, kWalkingMapOverlay = -2, kTitleText = -3, kUnused = -4 };

/**
  * Default argument to Animation::getFrame() that makes it return 
  * the current frame instead of the user specifying it.
  */
enum { kCurrentFrame = -1 };

/** 
  * Used by overlays as a neutral index that won't get 
  * released with the GPL Release command.
  */
enum { kIgnoreIndex = -2 };

class DraciEngine;

class Animation {

typedef void (Animation::* AnimationCallback)();

public:
	Animation(DraciEngine *v, int index);
	~Animation();	
	
	uint getZ();
	void setZ(uint z);
	
	void setID(int id);
	int getID();

	void nextFrame(bool force = false);
	void drawFrame(Surface *surface);

	void addFrame(Drawable *frame);
	Drawable *getFrame(int frameNum = kCurrentFrame);
	void setCurrentFrame(uint frame);
	uint currentFrameNum();
	uint getFrameCount();
	void deleteFrames();

	bool isPlaying();
	void setPlaying(bool playing);

	bool isLooping();
	void setLooping(bool looping);

	void setRelative(int relx, int rely);
	int getRelativeX();
	int getRelativeY();

	int getIndex();
	void setIndex(int index);

	void setScaleFactors(double scaleX, double scaleY);
	double getScaleX();
	double getScaleY();

	void markDirtyRect(Surface *surface);

	// Animation callbacks

	void registerCallback(AnimationCallback callback) { _callback = callback; }

	void doNothing() {}
	void stopAnimation();
	void exitGameLoop();

private:
	
	uint nextFrameNum();

	/** Internal animation ID 
	  *	(as specified in the data files and the bytecode)
	  */
	int _id; 
	
	/** The recency index of an animation, i.e. the most recently added animation has
	  * the highest index. Some script commands need this.
	  */
	int _index;

	uint _currentFrame;
	uint _z;

	int _relX;
	int _relY;

	double _scaleX;
	double _scaleY;

	uint _tick;
	bool _playing;
	bool _looping;
	Common::Array<Drawable*> _frames;

	AnimationCallback _callback;

	DraciEngine *_vm;
};


class AnimationManager {

public:
	AnimationManager(DraciEngine *vm) : _vm(vm), _lastIndex(-1) {}
	~AnimationManager() { deleteAll(); }

	Animation *addAnimation(int id, uint z, bool playing = false);
	Animation *addText(int id, bool playing = false);
	void addOverlay(Drawable *overlay, uint z);
	
	void play(int id);
	void stop(int id);

	void deleteAnimation(int id);
	void deleteOverlays();
	void deleteAll();

	void drawScene(Surface *surf);

	Animation *getAnimation(int id);

	int getLastIndex();
	void deleteAfterIndex(int index);

	int getTopAnimationID(int x, int y);

private:
	void sortAnimations();	
	void insertAnimation(Animation *anim);

	DraciEngine *_vm;
	Common::List<Animation *> _animations;

	/** The index of the most recently added animation. 
	  *	See Animation::_index for details.
	  */
	int _lastIndex;
};

}

#endif // DRACI_ANIMATION_H
