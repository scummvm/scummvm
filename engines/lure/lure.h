/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LURE_H
#define LURE_H

#include "engines/engine.h"
#include "common/rect.h"
#include "sound/mixer.h"
#include "common/file.h"

#include "lure/disk.h"
#include "lure/res.h"
#include "lure/screen.h"
#include "lure/events.h"
#include "lure/menu.h"
#include "lure/system.h"
#include "lure/strings.h"
#include "lure/room.h"

namespace Lure {

class LureEngine : public Engine {
private:
	uint32 _features;
	uint8 _game;
	Common::Language _language;
	Disk *_disk;
	Resources *_resources;
	Screen *_screen;
	Mouse *_mouse;
	Events *_events;
	Menu *_menu;
	System *_sys;
	StringData *_strings;
	Room *_room;

	void detectGame();
	const char *generateSaveName(int slotNumber);
public:
	LureEngine(OSystem *system);
	~LureEngine();
	static LureEngine &getReference();
	
	virtual int init();
	virtual int go();
	void quitGame();

	uint32 features() { return _features; }
	uint8 game() { return _game; }
	Disk &disk() { return *_disk; }

	bool loadGame(uint8 slotNumber);
	bool saveGame(uint8 slotNumber, Common::String &caption);
	Common::String *detectSave(int slotNumber);
};

} // End of namespace Lure

#endif
