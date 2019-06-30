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

#include "bladerunner/ui/kia_log.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

KIALog::KIALog(BladeRunnerEngine *vm) {
	_vm = vm;

	_firstIndex = 0;
	_lastIndex = 0;
	_currentIndex = 0;
	for (int i = 0; i < kSize; ++i) {
		_entries[i].dataSize = 0;
		_entries[i].data = nullptr;
	}
}

KIALog::~KIALog() {
	clear();
}

void KIALog::add(int type, int dataSize, const void *data) {
	if (_currentIndex == _lastIndex) {
		_lastIndex = (_lastIndex + 1) % kSize;
	}

	if (_entries[_currentIndex].data) {
		delete[] _entries[_currentIndex].data;
	}

	_entries[_currentIndex].type = type;
	_entries[_currentIndex].dataSize = dataSize;

	if (dataSize > 0) {
		unsigned char *dataCopy = new unsigned char[dataSize];
		memcpy(dataCopy, data, dataSize);
		_entries[_currentIndex].data = dataCopy;
	} else {
		_entries[_currentIndex].data = nullptr;
	}
}

void KIALog::clear() {
	_firstIndex = 0;
	_lastIndex = 0;
	_currentIndex = 0;
	for (int i = 0; i < kSize; ++i) {
		if (_entries[i].data) {
			delete[] _entries[i].data;
		}
		_entries[i].dataSize = 0;
		_entries[i].data = nullptr;
	}
}

void KIALog::prev() {
	if (_currentIndex != _firstIndex) {
		_currentIndex = (_currentIndex - 1) % kSize;
	}
}

void KIALog::next() {
	if (_currentIndex != _lastIndex) {
		_currentIndex = (_currentIndex + 1) % kSize;
	}
}

void KIALog::clearFuture() {
	_lastIndex = _currentIndex;
	int currentIndex = _currentIndex;
	while (currentIndex != _firstIndex) {
		if (_entries[currentIndex].data) {
			delete[] _entries[currentIndex].data;
			_entries[currentIndex].data = 0;
			_entries[currentIndex].dataSize = 0;
		}
		currentIndex = (currentIndex + 1) % kSize;
	}
}

bool KIALog::hasPrev() const {
	return _currentIndex != _firstIndex;
}

bool KIALog::hasNext() const {
	if (_currentIndex == _lastIndex) {
		return false;
	}

	return (((_currentIndex + 1) % kSize) != _lastIndex);
}

int KIALog::getPrevType() const {
	return _entries[(_currentIndex - 1) % kSize].type;
}

int KIALog::getNextType() const {
	return _entries[(_currentIndex + 1) % kSize].type;
}

const void *KIALog::getCurrentData() const {
	return _entries[_currentIndex].data;
}

} // End of namespace BladeRunner
