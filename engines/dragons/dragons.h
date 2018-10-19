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
 */
#ifndef SCUMMVM_DRAGONS_H
#define SCUMMVM_DRAGONS_H

#include "gui/EventRecorder.h"
#include "engines/engine.h"

namespace Dragons {

enum {
	kGameIdDragons = 1
};

struct DragonsGameDescription {
	ADGameDescription desc;
	int gameId;
};

struct SaveHeader {
	Common::String description;
	uint32 version;
	uint32 flags;
	uint32 saveDate;
	uint32 saveTime;
	uint32 playTime;
	Graphics::Surface *thumbnail;
};

enum kReadSaveHeaderError {
	kRSHENoError = 0,
	kRSHEInvalidType = 1,
	kRSHEInvalidVersion = 2,
	kRSHEIoError = 3
};

class DragonsEngine : public Engine {
public:
	DragonsEngine(OSystem *syst);
	~DragonsEngine();

	virtual Common::Error run();

	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);
};

} // End of namespace Dragons

#endif //SCUMMVM_DRAGONS_H
