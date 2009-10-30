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
#include "draci/sound.h"

namespace Draci {

/**
  * Animation IDs for those animations that don't have their IDs
  * specified in the data files.
  */
enum {
	kOverlayImage = -1,
	kWalkingMapOverlay = -2,
	kTitleText = -3,
	kSpeechText = -4,
	kInventorySprite = -5,
	kDialogueLinesID = -6,
	kUnused = -10,
	kInventoryItemsID = -11
};

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

	uint getZ() const { return _z; }
	void setZ(uint z) { _z = z; }

	void setID(int id) { _id = id; }
	int getID() const { return _id; }

	void nextFrame(bool force);
	void drawFrame(Surface *surface);

	void addFrame(Drawable *frame, const SoundSample *sample);
	Drawable *getCurrentFrame();
	Drawable *getFrame(int frameNum);
	void setCurrentFrame(uint frame);
	uint currentFrameNum() const { return _currentFrame; }
	uint getFrameCount() const { return _frames.size(); }

	bool isPlaying() const { return _playing; }
	void setPlaying(bool playing);

	bool isPaused() const { return _paused; }
	void setPaused(bool paused) { _paused = paused; }

	bool isLooping() const { return _looping; }
	void setLooping(bool looping);

	void setRelative(int relx, int rely);
	int getRelativeX() const { return _displacement.relX; }
	int getRelativeY() const { return _displacement.relY; }
	const Displacement &getDisplacement() const { return _displacement; }

	int getIndex() const { return _index; }
	void setIndex(int index) { _index = index; }

	void setScaleFactors(double scaleX, double scaleY);
	double getScaleX() const { return _displacement.extraScaleX; }
	double getScaleY() const { return _displacement.extraScaleY; }

	void markDirtyRect(Surface *surface) const;

	// Animation callbacks

	void registerCallback(AnimationCallback callback) { _callback = callback; }

	void doNothing() {}
	void stopAnimation();
	void exitGameLoop();

private:
	uint nextFrameNum() const;
	void deleteFrames();

	/** Internal animation ID
	  * (as specified in the data files and the bytecode)
	  */
	int _id;

	/** The recency index of an animation, i.e. the most recently added animation has
	  * the highest index. Some script commands need this.
	  */
	int _index;

	uint _currentFrame;
	uint _z;
	bool _hasChangedFrame;

	Displacement _displacement;

	uint _tick;
	bool _playing;
	bool _looping;
	bool _paused;

	/** Array of frames of the animation.  The animation object owns these pointers.
	 */
	Common::Array<Drawable *> _frames;
	/** Array of samples played during the animation.  The animation
	 * object doesn't own these pointers, but they are stored in the
	 * cache.
	 */
	Common::Array<const SoundSample *> _samples;

	AnimationCallback _callback;

	DraciEngine *_vm;
};


class AnimationManager {

public:
	AnimationManager(DraciEngine *vm) : _vm(vm), _lastIndex(-1) {}
	~AnimationManager() { deleteAll(); }

	Animation *addAnimation(int id, uint z, bool playing);
	Animation *addText(int id, bool playing);
	Animation *addItem(int id, bool playing);
	void addOverlay(Drawable *overlay, uint z);

	void play(int id);
	void stop(int id);
	void pauseAnimations();
	void unpauseAnimations();

	void deleteAnimation(int id);
	void deleteOverlays();
	void deleteAll();

	void drawScene(Surface *surf);

	Animation *getAnimation(int id);

	int getLastIndex() const { return _lastIndex; }
	void deleteAfterIndex(int index);

	int getTopAnimationID(int x, int y) const;

private:
	void sortAnimations();
	void insertAnimation(Animation *anim);

	DraciEngine *_vm;

	/** List of animation objects, maintained sorted by decreasing Z-coordinates.
	 * The animation manager owns the pointers.
	 */
	Common::List<Animation *> _animations;

	/** The index of the most recently added animation.
	  * See Animation::_index for details.
	  */
	int _lastIndex;
};

}

#endif // DRACI_ANIMATION_H
