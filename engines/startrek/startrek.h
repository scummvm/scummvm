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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/startrek.h $
 * $Id: startrek.h 14 2010-05-26 15:44:12Z clone2727 $
 *
 */

#ifndef STARTREK_H
#define STARTREK_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"

#include "engines/engine.h"

#include "startrek/graphics.h"
#include "startrek/sound.h"

namespace Common {
	class MacResManager;
}

namespace StarTrek {

enum StarTrekGameType {
	GType_ST25 = 1,
	GType_STJR = 2
};

enum StarTrekGameFeatures {
	GF_DEMO =    (1 << 0)
};

struct StarTrekGameDescription;
class Graphics;
class Sound;

class StarTrekEngine : public ::Engine {
protected:
	Common::Error run();

public:
	StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc);
	virtual ~StarTrekEngine();

	// Detection related functions
	const StarTrekGameDescription *_gameDescription;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType();
	Common::Language getLanguage();

	// Resource related functions
	Common::SeekableReadStream *openFile(Common::String filename);

	// Movie related functions
	void playMovie(Common::String filename);
	void playMovieMac(Common::String filename);
	
private:
	Graphics *_gfx;
	Sound *_sound;
	Common::MacResManager *_macResFork;
	
	byte getStartingIndex(Common::String filename);
};

} // End of namespace StarTrek

#endif
