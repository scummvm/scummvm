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

#include "common/system.h"
#include "common/util.h"
#include "gui/animation/FluidScroll.h"

namespace GUI {

const float FluidScroller::kVelocityThreshold = 0.01f;
const float FluidScroller::kValueThreshold = 0.1f;
const float FluidScroller::kDefaultSpringResponse = 0.575f;
const float FluidScroller::kRubberBandCoefficient = 0.55f;
const float FluidScroller::kRubberBandStretchFraction = 0.25f;
const float FluidScroller::kDecelerationRate = 0.998f;

FluidScroller::VelocityTracker::VelocityTracker() {
	reset();
}

void FluidScroller::VelocityTracker::reset() {
	index = 0;
	count = 0;
	memset(samples, 0, sizeof(samples));
}

void FluidScroller::VelocityTracker::addPoint(uint32 time, float position) {
	samples[index].time = time;
	samples[index].position = position;
	index = (index + 1) % kHistorySize;
	if (count < kHistorySize)
		count++;
}

float FluidScroller::VelocityTracker::calculateVelocity() const {
	if (count < 2)
		return 0.0f;

	// We look at the last few samples (up to 4) to determine the average velocity
	float velocities[4];
	int validVelocities = 0;

	for (int i = 0; i < 4 && i < count - 1; ++i) {
		int i1 = (index + kHistorySize - 1 - i) % kHistorySize; // current point
		int i2 = (index + kHistorySize - 2 - i) % kHistorySize; // previous point
		
		uint32 dt = samples[i1].time - samples[i2].time;

		if (dt > 0)
			velocities[validVelocities++] = (samples[i1].position - samples[i2].position) / (float)dt;
		else
			break;
	}

	if (validVelocities == 0)
		return 0.0f;
	
	// Weighted average of historical velocities
	float totalVelocity = 0.0f;
	float totalWeight = 0.0f;
	float weight = 1.0f;
	for (int i = 0; i < validVelocities; ++i) {
		totalVelocity += velocities[i] * weight;
		totalWeight += weight;
		weight *= 0.6f;
	}
	return totalVelocity / totalWeight;
}

FluidScroller::FluidScroller() : 
	_mode(kModeNone), 
	_startTime(0), 
	_scrollPosRaw(0.0f), 
	_animationOffset(0.0f), 
	_maxScroll(0.0f), 
	_viewportHeight(0),
	_initialVelocity(0.0f),
	_lambda(0.0f),
	_stretchDistance(0.0f),
	_impactVelocity(0.0f) {
}

void FluidScroller::setBounds(float maxScroll, int viewportHeight) {
	_maxScroll = maxScroll;
	_viewportHeight = viewportHeight;
}

void FluidScroller::reset() {
	_mode = kModeNone;
	_startTime = 0;
	_initialVelocity = 0.0f;
	_scrollPosRaw = 0.0f;
	_velocityTracker.reset();
}

void FluidScroller::stopAnimation() {
	_mode = kModeNone;
	_velocityTracker.reset();
}

void FluidScroller::feedDrag(uint32 time, int deltaY) {
	_scrollPosRaw += (float)deltaY;
	_velocityTracker.addPoint(time, _scrollPosRaw);
}

float FluidScroller::setPosition(float pos, bool checkBound) {
	_scrollPosRaw = pos;
	if (checkBound)
		checkBoundaries();
	return getVisualPosition();
}

void FluidScroller::startFling() {
	float velocity = _velocityTracker.calculateVelocity();
	
	if (fabsf(velocity) < 0.1f) {
		checkBoundaries();
		return;
	}

	_mode = kModeFling;
	_startTime = g_system->getMillis();
	_initialVelocity = velocity;
	_animationOffset = _scrollPosRaw;
}

void FluidScroller::absorb(float velocity, float distance) {
	_mode = kModeSpringBack;
	_startTime = g_system->getMillis();
	
	_lambda = 2.0f * (float)M_PI / kDefaultSpringResponse;
	_stretchDistance = distance;

	// Convert velocity from pixels/ms to pixels/s for the spring formula
	_impactVelocity = velocity * 1000.0f + _lambda * distance;
}

bool FluidScroller::update(uint32 time, float &outVisualPos) {
	if (_mode == kModeNone) {
		outVisualPos = getVisualPosition();
		return false;
	}

	float elapsed = (float)(time - _startTime);

	if (_mode == kModeFling) {
		float coefficient = powf(kDecelerationRate, elapsed);
		float velocity = _initialVelocity * coefficient;
		float offset = _initialVelocity * (1.0f / logf(kDecelerationRate)) * (coefficient - 1.0f);

		if (fabsf(velocity) < kVelocityThreshold) {
			_mode = kModeNone;
			checkBoundaries();
			outVisualPos = getVisualPosition();
			return _mode != kModeNone;
		}

		_scrollPosRaw = _animationOffset + offset;

		// Boundaries during fling
		if (_scrollPosRaw < 0) {
			absorb(velocity, _scrollPosRaw);
			_animationOffset = 0;
		} else if (_scrollPosRaw > _maxScroll) {
			absorb(velocity, _scrollPosRaw - _maxScroll);
			_animationOffset = _maxScroll;
		}

	} else if (_mode == kModeSpringBack) {
		float t = elapsed / 1000.0f;
		float offset = (_stretchDistance + _impactVelocity * t) * expf(-_lambda * t);
		float velocity = getVelocityAt(t);

		if (fabsf(offset) < kValueThreshold && fabsf(velocity) / 1000.0f < kVelocityThreshold) {
			_mode = kModeNone;
			_scrollPosRaw = _animationOffset;
			outVisualPos = getVisualPosition();
			return false;
		}

		_scrollPosRaw = _animationOffset + offset;
	}

	outVisualPos = getVisualPosition();
	return true;
}

float FluidScroller::getVisualPosition() const {
	float rubberBandRange = (float)_viewportHeight * kRubberBandStretchFraction;

	if (_scrollPosRaw < 0)
		return -calculateRubberBandOffset(-_scrollPosRaw, rubberBandRange);
	else if (_scrollPosRaw > _maxScroll)
		return _maxScroll + calculateRubberBandOffset(_scrollPosRaw - _maxScroll, rubberBandRange);
	
	return _scrollPosRaw;
}

void FluidScroller::checkBoundaries() {
	if (_scrollPosRaw < 0) {
		absorb(0, _scrollPosRaw);
		_animationOffset = 0;
	} else if (_scrollPosRaw > _maxScroll) {
		absorb(0, _scrollPosRaw - _maxScroll);
		_animationOffset = _maxScroll;
	}
}

float FluidScroller::getVelocityAt(float timeInSeconds) const {
	if (_mode != kModeSpringBack)
		return 0.0f;
	return (_impactVelocity - _lambda * (_stretchDistance + _impactVelocity * timeInSeconds)) * expf(-_lambda * timeInSeconds);
}

float FluidScroller::calculateRubberBandOffset(float offset, float range) {
	if (range <= 0)
		return 0;
	return (1.0f - (1.0f / ((offset * kRubberBandCoefficient / range) + 1.0f))) * range;
}

} // End of namespace GUI
