/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SCUMM_PLUGIN_H
#define SCUMM_PLUGIN_H

#include "common/util.h"

namespace Scumm {

/**
 * Descriptor of a specific SCUMM game. Used internally to store
 * information about the tons of game variants that exist.
 */
struct GameSettings {
	const char *gameid;
	const char *variant;
	byte id, version, heversion;
	int midi; // MidiDriverFlags values
	uint32 features;
	Common::Platform platform;

};

enum FilenameGenMethod {
	kGenDiskNum,
	kGenRoomNum,
	kGenHEMac,
	kGenHEMacNoParens,
	kGenHEPC,
	kGenUnchanged
};

struct FilenamePattern {
	const char *pattern;
	FilenameGenMethod genMethod;
};

struct GameFilenamePattern {
	const char *gameid;
	const char *pattern;
	FilenameGenMethod genMethod;
	Common::Language language;
	Common::Platform platform;
	const char *variant;
};

struct DetectorResult {
	FilenamePattern fp;
	GameSettings game;
	Common::Language language;
	Common::String md5;
	uint8 md5sum[16];
	const char *extra;
};

} // End of namespace Scumm


#endif
