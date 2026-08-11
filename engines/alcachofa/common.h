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

#ifndef ALCACHOFA_COMMON_H
#define ALCACHOFA_COMMON_H

#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/str-enc.h"
#include "common/stack.h"
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

	// V1 has some fields with these additional directions
	// but they are largely irrelevant so we map them to the main four
	UpRight,
	DownRight,
	DownLeft,
	UpLeft,

	Invalid = -1
};

enum class MainCharacterKind {
	None,
	Mortadelo,
	Filemon
};

enum class EasingType {
	Linear,
	InOut,
	In,
	Out
};

constexpr const int32 kDirectionCount = 4;
constexpr const int32 kFullDirectionCount = 8; ///< only to be used for IO or mapping V1 values
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
static constexpr const Color kDebugLightBlue = { 80, 80, 255, 190 };

/**
 * @brief This *fake* semaphore does not work in multi-threaded scenarios
 * It is used as a safer option for a simple "isBusy" counter
 */
struct FakeSemaphore {
	FakeSemaphore(const char *name, uint initialCount = 0);
	~FakeSemaphore();

	inline bool isReleased() const { return _counter == 0; }
	inline uint counter() const { return _counter; }

	static void sync(Common::Serializer &s, FakeSemaphore &semaphore);
private:
	friend struct FakeLock;
	const char *const _name;
	uint _counter = 0;
};

struct FakeLock {
	FakeLock();
	FakeLock(const char *name, FakeSemaphore &semaphore);
	FakeLock(const FakeLock &other);
	FakeLock(FakeLock &&other);
	~FakeLock();
	void operator= (FakeLock &&other);
	FakeLock &operator= (const FakeLock &other);
	void release();

	inline bool isReleased() const { return _semaphore == nullptr; }
private:
	void debug(const char *action);

	const char *_name = "<uninitialized>";
	FakeSemaphore *_semaphore = nullptr;
};

bool isPowerOfTwo(int16 x);

float ease(float t, EasingType type);

Common::String reencode(
	const Common::String &string,
	Common::CodePage from = Common::CodePage::kISO8859_1, // "Western European", used for the spanish special characters
	Common::CodePage to = Common::CodePage::kUtf8);

Math::Vector3d as3D(const Math::Vector2d &v);
Math::Vector3d as3D(Common::Point p);
Math::Vector2d as2D(const Math::Vector3d &v);
Math::Vector2d as2D(Common::Point p);

bool readBool(Common::ReadStream &stream);
Common::Point readPoint16(Common::ReadStream &stream);
Common::Point readPoint32(Common::ReadStream &stream);
Common::String readVarString(Common::ReadStream &stream);
void skipVarString(Common::SeekableReadStream &stream);
void syncPoint(Common::Serializer &serializer, Common::Point &point);
Direction intToDirection(int32 value);
Direction readDirection(Common::ReadStream &stream);

template<typename T>
inline void syncArray(Common::Serializer &serializer, Common::Array<T> &array, void (*serializeFunction)(Common::Serializer &, T &)) {
	auto size = array.size();
	serializer.syncAsUint32LE(size);
	array.resize(size);
	serializer.syncArray(array.data(), size, serializeFunction);
}

template<typename T>
inline void syncStack(
	Common::Serializer &serializer,
	Common::Stack<T> &stack,
	void (*serializeFunction)(Common::Serializer &, T &),
	Common::Serializer::Version minVersion = 0) {
	if (serializer.getVersion() < minVersion)
		return;

	auto size = stack.size();
	serializer.syncAsUint32LE(size);
	if (serializer.isLoading()) {
		for (uint i = 0; i < size; i++) {
			T value = {};
			serializeFunction(serializer, value);
			stack.push(value);
		}
	} else {
		for (uint i = 0; i < size; i++)
			serializeFunction(serializer, stack[i]);
	}
}

template<typename T>
inline void syncEnum(Common::Serializer &serializer, T &enumValue) {
	// syncAs does not have a cast for saving
	int32 intValue = static_cast<int32>(enumValue);
	serializer.syncAsSint32LE(intValue);
	enumValue = static_cast<T>(intValue);
}

/**
 * @brief References a game file either as path or as embedded byte range
 *
 * In V1 all files (except videos) are stored within the EMC file. Some of
 * those are within an embedded archive, most animations however are stored
 * at their graphics and have their original filenames. We reference them
 * by their byte range.
 *
 * V2/V3 store files outside with normal paths to use
 */
struct GameFileReference {
	Common::String _path;
	uint32
		_fileIndex = UINT32_MAX,
		_position = UINT32_MAX,
		_size = 0;

	GameFileReference() {}

	explicit GameFileReference(const Common::String &path)
		: _path(path) {}

	// in this case, path is only for debugging purposes
	GameFileReference(const Common::String &path, uint32 fileIndex, int64 position, uint32 size)
		: _path(path)
		, _fileIndex(fileIndex)
		, _position(position)
		, _size(size) {}

	inline bool isValid() const {
		return !_path.empty() || _fileIndex != UINT32_MAX;
	}

	inline bool isEmbedded() const {
		return _fileIndex != UINT32_MAX;
	}
};

}

#endif // ALCACHOFA_COMMON_H
