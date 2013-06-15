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

#ifndef BURIED_BURIED_H
#define BURIED_BURIED_H

#include "common/scummsys.h"
#include "common/array.h"

#include "engines/engine.h"

class OSystem;

namespace Buried {

struct BuriedGameDescription;
class Database;
class GraphicsManager;

class BuriedEngine : public ::Engine {
protected:
	Common::Error run();

public:
	BuriedEngine(OSystem *syst, const BuriedGameDescription *gamedesc);
	virtual ~BuriedEngine();

	// Detection related functions
	const BuriedGameDescription *_gameDescription;
	bool isDemo() const;
	bool isTrueColor() const;
	bool isWin95() const;
	bool isCompressed() const;
	Common::String getEXEName() const;
	Common::String getLibraryName() const;

	bool hasFeature(EngineFeature f) const;

	Common::String getFilePath(uint32 stringID);
	Common::SeekableReadStream *getBitmapStream(uint32 bitmapID);

	GraphicsManager *_gfx;
	Database *_mainEXE;

private:
	Database *_library;
};

} // End of namespace Buried

#endif
