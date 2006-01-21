/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
 * $Header$
 *
 */

#ifndef GOB_GOB_H
#define GOB_GOB_H

#include "common/stdafx.h"
#include "common/system.h"
#include "sound/mixer.h"
#include "common/config-manager.h"

#include "base/engine.h"
#include "base/gameDetector.h"

#include "gob/dataio.h"
#include "gob/video.h"
#include "common/file.h"

namespace Gob {

class Game;
class Snd;
class Video;
class Global;
class Draw;
class Anim;
class CDROM;
class DataIO;
class Goblin;
class Init;
class Inter;
class Map;
class Mult;
class Pack;
class PalAnim;
class Parse;
class Scenery;
class GTimer;
class Util;
class Music;

#define	VAR_OFFSET(offs)		(*(uint32 *)(_vm->_global->_inter_variables + (offs)))
#define	VAR(var)			VAR_OFFSET((var) << 2)
#define VAR_ADDRESS(var)		(&VAR(var))

#define	WRITE_VAR_OFFSET(offs, val)	(VAR_OFFSET(offs) = (val))
#define WRITE_VAR(var, val)		WRITE_VAR_OFFSET((var) << 2, (val))

enum {
	GF_GOB1 = 1 << 0,
	GF_GOB2 = 1 << 1,
	GF_GOB3 = 1 << 2,
	GF_WOODRUFF = 1 << 3,
	GF_CD = 1 << 4,
	GF_MAC = 1 << 5
};

typedef struct GobGameSettings {
	const char *gameid;
	const char *description;
	uint32 features;
	const char *md5sum;
	GameSettings toGameSettings() const {
		GameSettings dummy = { gameid, description, features };
		return dummy;
	}
} GobGameSettings;

class GobEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);

protected:
	int go();
	int init(GameDetector &detector);

public:
	GobEngine(GameDetector * detector, OSystem * syst, uint32 features);
	virtual ~GobEngine();

	void shutdown();

	Common::RandomSource _rnd;

	int32 _features;
	bool _copyProtection;

	Game *_game;
	Snd *_snd;
	Video *_video;
	Global *_global;
	Draw *_draw;
	Anim *_anim;
	CDROM *_cdrom;
	DataIO *_dataio;
	Goblin *_goblin;
	Init *_init;
	Map *_map;
	Mult *_mult;
	Pack *_pack;
	PalAnim *_palanim;
	Parse *_parse;
	Scenery *_scenery;
	GTimer *_gtimer;
	Util *_util;
	Inter *_inter;
	Music *_music;
};

} // End of namespace Gob
#endif
