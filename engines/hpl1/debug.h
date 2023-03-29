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

#ifndef HPL1_DEBUG_H
#define HPL1_DEBUG_H

#include "common/debug.h"

#define HPL1_UNIMPLEMENTED(fnName) error("call to unimplemented function " #fnName)

namespace Hpl1 {

enum DebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugResourceLoading = 1 << 1,
	kDebugFilePath = 1 << 2,
	kDebugOpenGL = 1 << 3,
	kDebugRenderer = 1 << 4,
	kDebugAudio = 1 << 5,
	kDebugSaves = 1 << 6,
	kDebugTextures = 1 << 7,
	kDebugScripts = 1 << 8
};

enum DebugLevels {
	kDebugLevelError = 1,
	kDebugLevelWarning = 2,
	kDebugLevelLog = 3
};

template<typename... Args>
void logError(uint32 channels, const char *fmt, Args... args) {
	debugCN(kDebugLevelError, channels, fmt, args...);
}

template<typename... Args>
void logWarning(uint32 channels, const char *fmt, Args... args) {
	debugCN(kDebugLevelWarning, channels, fmt, args...);
}

template<typename... Args>
void logInfo(uint32 channels, const char *fmt, Args... args) {
	debugCN(kDebugLevelLog, channels, fmt, args...);
}

} // namespace Hpl1

#endif
