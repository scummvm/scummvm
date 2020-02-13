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

#ifndef SHERLOCK_SCALPEL_TSAGE_LOGO_H
#define SHERLOCK_SCALPEL_TSAGE_LOGO_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "common/list.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "sherlock/scalpel/tsage/resources.h"
#include "sherlock/screen.h"

namespace Sherlock {
namespace Scalpel {

class ScalpelEngine;

namespace TsAGE {

class ObjectSurface : public Surface {
public:
	Common::Point _centroid;
public:
	ObjectSurface() : Surface() {}
	~ObjectSurface() override {}
};

class Visage {
private:
	Common::SeekableReadStream *_stream;

	/**
	 * Translates a raw image resource into a graphics surface
	 */
	void surfaceFromRes(ObjectSurface &s);
public:
	static TLib *_tLib;
	int _resNum;
	int _rlbNum;
public:
	Visage();
	~Visage();

	/**
	 * Set the visage number
	 */
	void setVisage(int resNum, int rlbNum = 9999);

	/**
	 * Clear the visage
	 */
	void clear();

	/**
	 * Get a frame from the visage
	 */
	void getFrame(ObjectSurface &s, int frameNum);

	/**
	 * Return the number of frames
	 */
	int getFrameCount() const;

	/**
	 * Returns whether the visage is loaded
	 */
	bool isLoaded() const;
};

class Object {
private:
	Visage _visage;
	uint32 _updateStartFrame;
	bool _isAnimating;
	bool _finished;
	uint32 _walkStartFrame;
	int _angle;
	int _changeCtr;
	int _majorDiff, _minorDiff;
	Common::Point _moveDelta;
	Common::Point _moveSign;

	/**
	 * Return the next frame when the object is animating
	 */
	int changeFrame();

	/**
	 * Gets the next frame in the sequence
	 */
	int getNewFrame();

	/**
	 * Calculate the angle between the current position and a designated destination
	 */
	void calculateMoveAngle();

	/**
	 * Handle any object movement
	 */
	void move();

	/**
	 * Returns whether not to make any movement
	 */
	bool dontMove() const;

	/**
	 * Ends any current movement
	 */
	void endMove();
public:
	static ScalpelEngine *_vm;
	Common::Point _position;
	Common::Point _destination;
	Common::Rect _oldBounds;
	int _frame;
	int _numFrames;
	int _frameChange;
public:
	Object();

	/**
	 * Load the data for the object
	 */
	void setVisage(int visage, int strip);

	/**
	 * Sets whether the object is animating
	 */
	void setAnimMode(bool isAnimating);

	/**
	 * Starts an object moving to a given destination
	 */
	void setDestination(const Common::Point &pt);

	/**
	 * Returns true if an animation is ended
	 */
	bool isAnimEnded() const;

	/**
	 * Return true if object is moving
	 */
	bool isMoving() const;

	/**
	 * Erase the area the object was previously drawn at, by restoring the background
	 */
	void erase();

	/**
	 * Update the frame
	 */
	void update();

	/**
	 * Remove an object from being displayed
	 */
	void remove() { _visage.clear(); }
};

struct AnimationFrame {
	int	frame;
	int	x;
	int	y;
};

class Logo {
private:
	ScalpelEngine *_vm;
	TLib _lib;
	int _counter, _frameCounter;
	bool _finished;
	byte _originalPalette[PALETTE_SIZE];
	byte _palette1[PALETTE_SIZE];
	byte _palette2[PALETTE_SIZE];
	byte _palette3[PALETTE_SIZE];
	Object _objects[4];
	uint _waitFrames;
	uint32 _waitStartFrame;
	int _animateObject;
	uint32 _animateStartFrame;
	uint _animateFrameDelay;
	const AnimationFrame *_animateFrames;
	uint _animateFrame;

	Logo(ScalpelEngine *vm);
	~Logo();

	void nextFrame();

	bool finished() const;

	/**
	 * Wait for a number of frames. Note that the frame count in _events is
	 * not the same as the number of calls to nextFrame().
	 */
	void waitFrames(uint frames);

	/**
	 * Start an animation sequence. Used for sequences that are described
	 * one frame at a time because they do unusual things, or run at
	 * unusual rates.
	 */
	void startAnimation(uint object, uint frameDelay, const AnimationFrame *frames);

	/**
	 * Load the background for the scene
	 */
	void loadBackground();

	/**
	 * Fade from the current palette to a new one
	 */
	void fade(const byte palette[PALETTE_SIZE], int step = 6);
public:
	static bool show(ScalpelEngine *vm);
};

} // end of namespace TsAGE
} // end of namespace Scalpel
} // end of namespace Sherlock

#endif
