/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_H
#define CINE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

#include "base/engine.h"
#include "base/gameDetector.h"

namespace Cine {

enum CineGameId {
	GID_FW = 1,
	GID_OS
};

class CineEngine : public ::Engine {
	int _gameId;

	void errorString(const char *buf_input, char *buf_output);

protected:
	int init(GameDetector & detector);
	int go();
	void shutdown();

	const char *_dataPath;
	const char *_savePath;

public:
	CineEngine(GameDetector *detector, OSystem *syst);
	virtual ~CineEngine();
	int getGameId() {
		return _gameId;
	}
};

}

extern char *savePath;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;

typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;

#define ASSERT_PTR assert
#define ASSERT assert

#pragma pack(1)

#define BOOT_PRC_NAME "AUTO00.PRC"

#include "cine/font.h"
#include "cine/various.h"
#include "cine/flip_support.h"
#include "cine/texte.h"
#include "cine/object.h"
#include "cine/rel.h"
#include "cine/script.h"
#include "cine/part.h"
#include "cine/prc.h"
#include "cine/main_loop.h"
#include "cine/resource.h"
#include "cine/msg.h"
#include "cine/bg.h"
#include "cine/pal.h"
#include "cine/gfx.h"
#include "cine/anim.h"
#include "cine/auto00.h"
#include "cine/unpack.h"
#include "cine/bg_list.h"
#include "cine/sfx_player.h"
#include "cine/sound_driver.h"

extern AdlibMusic *g_cine_adlib;

#endif
