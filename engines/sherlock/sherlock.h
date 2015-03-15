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

#ifndef SHERLOCK_HOLMES_H
#define SHERLOCK_HOLMES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/savefile.h"
#include "common/hash-str.h"
#include "engines/engine.h"
#include "sherlock/journal.h"
#include "sherlock/resources.h"
#include "sherlock/room.h"
#include "sherlock/talk.h"

namespace Sherlock {

enum {
	kFileTypeHash
};

enum {
	kDebugScript = 1 << 0
};

enum {
	GType_SerratedScalpel = 0,
	GType_RoseTattoo = 1
};

#define SHERLOCK_SCREEN_WIDTH 320
#define SHERLOCK_SCREEN_HEIGHT 200

struct SherlockGameDescription;

class Resource;

class SherlockEngine : public Engine {
private:
	bool detectGame();

	void initialize();
public:
	const SherlockGameDescription *_gameDescription;
	Journal *_journal;
	Resources *_res;
	Rooms *_rooms;
	Talk *_talk;
	Common::Array<bool> _flags;
public:
	SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	virtual ~SherlockEngine();

	virtual Common::Error run();

	virtual void initFlags() = 0;

	int getGameType() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	Common::String getGameFile(int fileType);
};

} // End of namespace Sherlock

#endif
