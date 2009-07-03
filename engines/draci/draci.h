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
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_H
#define DRACI_H
 
#include "common/system.h"
#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "draci/game.h"
#include "draci/mouse.h"
#include "draci/screen.h"
#include "draci/font.h"
#include "draci/script.h"
#include "draci/barchive.h"
#include "draci/animation.h"

namespace Draci {

class DraciEngine : public Engine {
public:
	DraciEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~DraciEngine();

	int init();
	int go();
	Common::Error run();

	bool hasFeature(Engine::EngineFeature f) const;

	Font *_font;
	Screen *_screen;
	Mouse *_mouse;
	Game *_game;
	Script *_script;
	Animation *_anims;

	BArchive *_objectsArchive;
	BArchive *_spritesArchive;
	BArchive *_paletteArchive;
	BArchive *_roomsArchive;
	BArchive *_overlaysArchive;

private:
	Common::RandomSource _rnd;
};

enum {
	kDraciGeneralDebugLevel = 	1 << 0,
	kDraciBytecodeDebugLevel = 	1 << 1,
	kDraciArchiverDebugLevel = 	1 << 2,
	kDraciLogicDebugLevel = 	1 << 3,
	kDraciAnimationDebugLevel =	1 << 4
};

} // End of namespace Draci

#endif // DRACI_H

