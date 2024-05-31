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

#ifndef COMMON_H
#define COMMON_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "math/vector3d.h"

namespace Alcachofa {

enum class CursorType {
	Point,
	LeaveUp,
	LeaveRight,
	LeaveDown,
	LeaveLeft,
	WalkTo
};

enum class Direction {
	Up,
	Right,
	Down,
	Left,

	Invalid = -1
};

enum class MainCharacterKind {
	None,
	Mortadelo,
	Filemon
};

constexpr const int32 kDirectionCount = 4;
constexpr const int8 kOrderCount = 70;
constexpr const int8 kForegroundOrderCount = 10;

struct Color {
	uint8 r, g, b, a;
};
static constexpr const Color kWhite = { 255, 255, 255, 255 };
static constexpr const Color kBlack = { 0, 0, 0, 255 };
static constexpr const Color kClear = { 0, 0, 0, 0 };
static constexpr const Color kDebugRed = { 250, 0, 0, 70 };
static constexpr const Color kDebugGreen = { 0, 255, 0, 85 };
static constexpr const Color kDebugBlue = { 0, 0, 255, 110 };

/**
 * @brief This *fake* semaphore does not work in multi-threaded scenarios
 * It is used as a safer option for a simple "isBusy" counter
 */
struct FakeSemaphore {
	FakeSemaphore(uint initialCount = 0) : _counter(initialCount) {}
	~FakeSemaphore() {
		assert(_counter == 0);
	}

	inline bool isReleased() const { return _counter == 0; }
	inline uint counter() const { return _counter; }
private:
	friend struct FakeLock;
	uint _counter = 0;
};

struct FakeLock {
	FakeLock() : _semaphore(nullptr) {}

	FakeLock(FakeSemaphore &semaphore) : _semaphore(&semaphore) {
		_semaphore->_counter++;
	}

	FakeLock(const FakeLock &other) : _semaphore(other._semaphore) {
		assert(_semaphore != nullptr);
		_semaphore->_counter++;
	}

	FakeLock(FakeLock &&other) noexcept : _semaphore(other._semaphore) {
		other._semaphore = nullptr;
	}

	~FakeLock() {
		if (_semaphore == nullptr)
			return;
		assert(_semaphore->_counter > 0);
		_semaphore->_counter--;
	}
private:
	FakeSemaphore *_semaphore;
};

inline Math::Vector3d as3D(const Math::Vector2d &v) {
	return Math::Vector3d(v.getX(), v.getY(), 0.0f);
}

inline Math::Vector3d as3D(const Common::Point &p) {
	return Math::Vector3d((float)p.x, (float)p.y, 0.0f);
}

inline Math::Vector2d as2D(const Math::Vector3d &v) {
	return Math::Vector2d(v.x(), v.y());
}

inline Math::Vector2d as2D(const Common::Point &p) {
	return Math::Vector2d((float)p.x, (float)p.y);
}

}

#endif // ALCACHOFA_COMMON_H
