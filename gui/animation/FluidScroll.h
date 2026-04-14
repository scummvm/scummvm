/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Based on the implementation by fluid-scroll repository
 * at https://github.com/ktiays/fluid-scroll
 */

#ifndef GUI_ANIMATION_FLUID_SCROLL_H
#define GUI_ANIMATION_FLUID_SCROLL_H

#include "common/scummsys.h"

namespace GUI {

class FluidScroller {
public:
	FluidScroller();
	~FluidScroller() {}

	/**
	 * Configure the constraints for the content
	 * @param maxScroll The maximum scrollable distance (total height - viewport height)
	 * @param viewportHeight The height of the scrolling area, used for rubber-band range
	 */
	void setBounds(float maxScroll, int viewportHeight);

	void reset();

	// Reset the animation state (fling/spring-back), keeping current position
	void stopAnimation();

	/**
	 * Record a pointer movement and update the raw position
	 * @param time Current system time in ms
	 * @param deltaY The movement since the last frame
	 */
	void feedDrag(uint32 time, int deltaY);

	// Start a fling using the recorded velocity
	void startFling();

	// Check if there is an active animation (fling or spring-back)
	bool isAnimating() const { return _mode != kModeNone; }

	/**
	 * Update the internal animation state
	 * @param time Current system time in ms
	 * @param outVisualPos The resulting visual scroll position (including rubber-banding)
	 * @return True if an animation is active and updated
	 */
	bool update(uint32 time, float &outVisualPos);

	float setPosition(float pos, bool checkBound = false);

	// Get the current visual scroll position
	float getVisualPosition() const;

	// Trigger an elastic spring-back if the current position is out of bounds
	void checkBoundaries();

private:
	enum Mode {
		kModeNone,
		kModeFling,
		kModeSpringBack
	};

	// Velocity tracking
	struct VelocityTracker {
		struct Point {
			uint32 time;
			float position;
		};
		static const int kHistorySize = 20;
		Point samples[kHistorySize];
		int index;
		int count;

		VelocityTracker();
		void reset();
		void addPoint(uint32 time, float position);
		float calculateVelocity() const;
	};

	VelocityTracker _velocityTracker;

	Mode _mode;
	uint32 _startTime;

	// Scroll status
	float _scrollPosRaw;    // Physical position (can go out of bounds)
	float _animationOffset; // Anchor position used as the starting point for animation offsets
	float _maxScroll;
	int _viewportHeight;

	// Fling parameter
	float _initialVelocity;

	// Spring parameters
	float _lambda; // Spring stiffness factor
	float _stretchDistance; // Initial distance beyond the edge when spring-back begins
	float _impactVelocity;  // Velocity when hitting the boundary


	float getVelocityAt(float timeInSeconds) const;

	// Transition from movement to spring-back animation when hitting an edge
	void absorb(float velocity, float distance);

	// Returns the visual offset to apply when scrolled past an edge
	static float calculateRubberBandOffset(float offset, float range);

	static const float kRubberBandStretchFraction; // Maximum stretch limit as a fraction of viewport height
	static const float kDecelerationRate; // Rate at which fling velocity slows down
	static const float kVelocityThreshold; // Minimum velocity to keep animation running
	static const float kValueThreshold; // Minimum value difference to keep spring active
	static const float kDefaultSpringResponse; // Natural response time of the spring
	static const float kRubberBandCoefficient; // Coefficient for rubber-band stiffness
};

} // End of namespace GUI

#endif
