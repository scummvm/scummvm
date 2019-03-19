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

#ifndef HDB_HDB_H
#define HDB_HDB_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/error.h"
#include "common/file.h"
#include "common/events.h"
#include "common/str.h"

#include "gui/debugger.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "console.h"

struct ADGameDescription;

namespace HDB {

enum HDBDebugChannels {
	kDebugExample1 = 1 << 0,
	kDebugExample2 = 1 << 1
};

class HDBGame : public Engine {
public:
	HDBGame(OSystem *syst, const ADGameDescription *gameDesc);
	~HDBGame();

	virtual Common::Error run();

	// Detection related members;
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;

private:
	Console *_console;
	Common::File file;

	struct {
		byte signature[5]; // 4 Bytes + '\0'
		uint32 dirOffset;
		uint32 dirSize;

		bool isValid() {
			return	(signature[0] == 'M') &&
					(signature[1] == 'P') &&
					(signature[2] == 'C') &&
					(signature[3] == 'U') &&
					(signature[4] == '\0');
		}

	} dataHeader;

	struct DataFile {
		byte fileName[65]; // 65 Bytes + '\0'
		uint32 filePosition;
		uint32 fileLength;
		uint32 unknownField1;
		uint32 unknownField2;
	};

	Common::Array<DataFile*> gameData;

	void readMPC(const Common::String &fileName);
};

}// End of namespace HDB

#endif