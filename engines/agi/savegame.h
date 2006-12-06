/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_SAVEGAME_H
#define AGI_SAVEGAME_H

#include "agi/agi.h"

class Common::File;

namespace Agi {

#define ADD_PIC 1
#define ADD_VIEW 2

class AgiEngine;
class SpritesMgr;
class GfxMgr;
class SoundMgr;
class PictureMgr;

class SaveGameMgr {
private:

	AgiEngine *_vm;
	SpritesMgr *_sprites;
	GfxMgr *_gfx;
	SoundMgr *_sound;
	PictureMgr *_picture;

	void write_string(Common::File *f, const char *s);
	void read_string(Common::File *f, char *s);
	void write_bytes(Common::File *f, const char *s, int16 size);
	void read_bytes(Common::File *f, char *s, int16 size);
	int save_game(char *s, const char *d);
	int load_game(char *s);
	int select_slot();

public:
	SaveGameMgr(AgiEngine *agi, SpritesMgr *sprites,
				GfxMgr *gfx,
				SoundMgr *sound, PictureMgr *picture) {
		_vm = agi;
		_sprites = sprites;
		_gfx = gfx;
		_sound = sound;
		_picture = picture;
	}

	int savegame_dialog();
	int loadgame_dialog();
	int savegame_simple();
	int loadgame_simple();
};

}                             // End of namespace Agi

#endif				/* AGI_SAVEGAME_H */
