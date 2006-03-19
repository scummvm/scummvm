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
 * $Id: scumm.cpp 20883 2006-02-25 18:24:55Z fingolfin $
 *
 */

#ifndef SCUMM_PLUGIN_H
#define SCUMM_PLUGIN_H

#include "common/util.h"

/**
 * Descriptor of a specific SCUMM game. Used internally to store
 * information about the tons of game variants that exist.
 */
struct ScummGameSettings {
	const char *gameid;
	const char *extra;
	byte id, version, heversion;
	int midi; // MidiDriverFlags values
	uint32 features;
	Common::Platform platform;

};


enum GenMethods {
	kGenMac,
	kGenMacNoParens,
	kGenPC,
	kGenAsIs
};

struct SubstResFileNames {
	const char *almostGameID;
	const char *expandedName;
	GenMethods genMethod;
};


extern bool applySubstResFileName(const SubstResFileNames &subst, const char *filename, char *buf, int bufsize);
extern int findSubstResFileName(SubstResFileNames &subst, const char *filename, int index);


#endif
