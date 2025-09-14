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

#include "alcachofa/common.h"
#include "alcachofa/detection.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

float ease(float t, EasingType type) {
	switch (type) {
	case EasingType::Linear:
		return t;
	case EasingType::InOut:
		return (1 - cosf(t * M_PI)) * 0.5f;
	case EasingType::In:
		return 1 - cosf(t * M_PI * 0.5f);
	case EasingType::Out:
		return sinf(t * M_PI * 0.5f);
	default:
		return 0.0f;
	}
}

FakeSemaphore::FakeSemaphore(const char *name, uint initialCount)
	: _name(name)
	, _counter(initialCount) {}

FakeSemaphore::~FakeSemaphore() {
	assert(_counter == 0);
}

void FakeSemaphore::sync(Serializer &s, FakeSemaphore &semaphore) {
	// if we are still holding locks during loading these locks will
	// try to decrease the counter which will fail, let's find this out already here
	assert(s.isSaving() || semaphore.isReleased());
	(void)s;
	(void)semaphore;

	// We should not actually serialize the counter, just make sure it is empty
	// When the locks are loaded, they will increase the counter themselves
}

FakeLock::FakeLock() {}

FakeLock::FakeLock(const char *name, FakeSemaphore &semaphore)
	: _name(name)
	, _semaphore(&semaphore) {
	_semaphore->_counter++;
	debug("ctor");
}

FakeLock::FakeLock(const FakeLock &other)
	: _name(other._name)
	, _semaphore(other._semaphore) {
	assert(_semaphore != nullptr);
	_semaphore->_counter++;
	debug("copy");
}

FakeLock::FakeLock(FakeLock &&other)
	: _name(other._name)
	, _semaphore(other._semaphore) {
	other._name = "<moved>";
	other._semaphore = nullptr;
	if (_semaphore != nullptr)
		debug("move-ctor");
}

FakeLock::~FakeLock() {
	release();
}

void FakeLock::operator= (FakeLock &&other) {
	release();
	_name = other._name;
	_semaphore = other._semaphore;
	other._name = "<moved>";
	other._semaphore = nullptr;
	debug("move-assign");
}

void FakeLock::release() {
	if (_semaphore == nullptr)
		return;
	assert(_semaphore->_counter > 0);
	_semaphore->_counter--;
	debug("release");
	_semaphore = nullptr;
}

void FakeLock::debug(const char *action) {
	const char *myName = _name == nullptr ? "<null>" : _name;
	if (_semaphore == nullptr)
		debugC(kDebugSemaphores, "Lock %s %s nullptr", myName, action);
	else
		debugC(kDebugSemaphores, "Lock %s %s %s at %u", myName, action, _semaphore->_name, _semaphore->_counter);
}

Vector3d as3D(const Vector2d &v) {
	return Vector3d(v.getX(), v.getY(), 0.0f);
}

Vector3d as3D(Common::Point p) {
	return Vector3d((float)p.x, (float)p.y, 0.0f);
}

Vector2d as2D(const Vector3d &v) {
	return Vector2d(v.x(), v.y());
}

Vector2d as2D(Point p) {
	return Vector2d((float)p.x, (float)p.y);
}

bool readBool(ReadStream &stream) {
	return stream.readByte() != 0;
}

Point readPoint(ReadStream &stream) {
	return { (int16)stream.readSint32LE(), (int16)stream.readSint32LE() };
}

static uint32 readVarInt(ReadStream &stream) {
	uint32 length = stream.readByte();
	if (length != 0xFF)
		return length;
	length = stream.readUint16LE();
	if (length != 0xFFFF)
		return length;
	return stream.readUint32LE();
}

String readVarString(ReadStream &stream) {
	uint32 length = readVarInt(stream);
	if (length == 0)
		return Common::String();

	char *buffer = new char[length];
	if (buffer == nullptr)
		error("Out of memory in readVarString");
	if (stream.read(buffer, length) != length)
		error("Could not read all %u bytes in readVarString", length);

	String result(buffer, buffer + length);
	delete[] buffer;
	return result;
}

void skipVarString(SeekableReadStream &stream) {
	stream.skip(readVarInt(stream));
}

void syncPoint(Serializer &serializer, Point &point) {
	serializer.syncAsSint32LE(point.x);
	serializer.syncAsSint32LE(point.y);
}

}
