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

// We place selector vocab name tables here for any game that doesn't have
// them. This includes the King's Quest IV Demo and LSL3 Demo.

#ifndef SCI_STATIC_SELECTORS_H
#define SCI_STATIC_SELECTORS_H

#include "sci/engine/kernel.h"

namespace Sci {
	
struct SelectorRemap {
	const char *name;
	uint32 slot;
};
	
// Taken from King's Quest IV (Full Game)
static const SelectorRemap kq4_demo_selectors[] = {
	{ "init", 87 },
	{ "play", 42 },
	{ "replay", 65 },
	{ "x", 4 },
	{ "y", 3 },
	{ "z", 85 },
	{ "priority", 63 },
	{ "view", 5 },
	{ "loop", 6 },
	{ "cel", 7 },
	{ "brLeft", 20 },
	{ "brRight", 22 },
	{ "brTop", 19 },
	{ "brBottom", 21 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "nsBottom", 11 },
	{ "nsTop", 9 },
	{ "nsLeft", 10 },
	{ "nsRight", 12 },
	{ "font", 33 },
	{ "text", 26 },
	{ "type", 34 },
	{ "state", 32 },
	{ "doit", 60 },
	{ "delete", 84 },
	{ "signal", 17 },
	{ "underBits", 8 },
	{ "canBeHere", 57 },
	{ "client", 45 },
	{ "dx", 46 },
	{ "dy", 47 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "b-moveCnt", 48 },
	{ "b-i1", 49 },
	{ "b-i2", 50 },
	{ "b-di", 51 },
	{ "b-xAxis", 52 },
	{ "b-incr", 53 },
	{ "completed", 158 },
	{ "illegalBits", 18 },
	{ "dispose", 88 },
	{ "prevSignal", 129 },
	{ "message", 40 },
	{ "modifiers", 64 },
	{ "cue", 121 },
	{ "owner", 130 },
	{ "handle", 44 },
	{ "number", 43 },
	{ "max", 37 },
	{ "cursor", 36 },
	{ "claimed", 76 },
	{ "edgeHit", 225 },
	{ "wordFail", 71 },
	{ "syntaxFail", 72 },
	{ "semanticFail", 73 },
	{ "cycler", 165 },
	{ "elements", 27 },
	{ "lsTop", 13 },
	{ "lsBottom", 15 },
	{ "lsLeft", 14 },
	{ "lsRight", 16 },
	{ "baseSetter", 208 },
	{ "who", 39 },
	{ "distance", 173 },
	{ "mover", 59 },
	{ "looper", 62 },
	{ "isBlocked", 61 },
	{ "heading", 58 },
	{ "mode", 30 },
	{ "caller", 119 },
	{ "moveDone", 169 },
	{ "size", 96 },
	{ "moveSpeed", 56 },
	{ "motionCue", 161 },
	{ "setTarget", 171 }
};
	
// Taken from EcoQuest 2 (Demo)
static const SelectorRemap christmas1992_selectors[] = {
	{ "init", 110 },
	{ "play", 39 },
	{ "replay", 62 },
	{ "x", 1 },
	{ "y", 0 },
	{ "z", 82 },
	{ "priority", 60 },
	{ "view", 2 },
	{ "loop", 3 },
	{ "cel", 4 },
	{ "brLeft", 17 },
	{ "brRight", 19 },
	{ "brTop", 16 },
	{ "brBottom", 18 },
	{ "xStep", 51 },
	{ "yStep", 52 },
	{ "nsBottom", 8 },
	{ "nsTop", 6 },
	{ "nsLeft", 7 },
	{ "nsRight", 9 },
	{ "font", 30 },
	{ "text", 23 },
	{ "type", 31 },
	{ "state", 29 },
	{ "doit", 57 },
	{ "delete", 81 },
	{ "signal", 14 },
	{ "underBits", 5 },
	{ "canBeHere", 450 },
	{ "client", 42 },
	{ "dx", 43 },
	{ "dy", 44 },
	{ "xStep", 51 },
	{ "yStep", 52 },
	{ "b-moveCnt", 45 },
	{ "b-i1", 46 },
	{ "b-i2", 47 },
	{ "b-di", 48 },
	{ "b-xAxis", 49 },
	{ "b-incr", 50 },
	{ "completed", 250 },
	{ "illegalBits", 15 },
	{ "dispose", 111 },
	{ "prevSignal", 171 },
	{ "message", 37 },
	{ "modifiers", 61 },
	{ "cue", 145 },
	{ "owner", 172 },
	{ "handle", 90 },
	{ "number", 40 },
	{ "max", 34 },
	{ "cursor", 33 },
	{ "claimed", 73 },
	{ "edgeHit", 333 },
	{ "wordFail", 68 },
	{ "syntaxFail", 69 },
	{ "semanticFail", 70 },
	{ "cycler", 255 },
	{ "elements", 24 },
	{ "lsTop", 10 },
	{ "lsBottom", 12 },
	{ "lsLeft", 11 },
	{ "lsRight", 13 },
	{ "baseSetter", 310 },
	{ "who", 36 },
	{ "distance", 264 },
	{ "mover", 56 },
	{ "looper", 59 },
	{ "isBlocked", 58 },
	{ "heading", 55 },
	{ "mode", 27 },
	{ "caller", 143 },
	{ "moveDone", 97 },
	{ "vol", 94 },
	{ "pri", 95 },
	{ "min", 91 },
	{ "sec", 92 },
	{ "frame", 93 },
	{ "dataInc", 89 },
	{ "size", 86 },
	{ "palette", 88 },
	{ "moveSpeed", 53 },
	{ "cantBeHere", 54 },
	{ "nodePtr", 41 },
	{ "flags", 99 },
	{ "points", 87 },
	{ "syncCue", 271 },
	{ "syncTime", 270 },
	{ "printLang", 84 },
	{ "subtitleLang", 85 },
	{ "parseLang", 83 },
	{ "setVol", 178 }
};

// Taken from Leisure Suit Larry 1 VGA (Full Game)
static const SelectorRemap lsl1_demo_selectors[] = {
	{ "init", 104 },
	{ "play", 42 },
	{ "replay", 65 },
	{ "x", 4 },
	{ "y", 3 },
	{ "z", 85 },
	{ "priority", 63 },
	{ "view", 5 },
	{ "loop", 6 },
	{ "cel", 7 },
	{ "brLeft", 20 },
	{ "brRight", 22 },
	{ "brTop", 19 },
	{ "brBottom", 21 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "nsBottom", 11 },
	{ "nsTop", 9 },
	{ "nsLeft", 10 },
	{ "nsRight", 12 },
	{ "font", 33 },
	{ "text", 26 },
	{ "type", 34 },
	{ "state", 32 },
	{ "doit", 60 },
	{ "delete", 84 },
	{ "signal", 17 },
	{ "underBits", 8 },
	{ "canBeHere", 232 },
	{ "client", 45 },
	{ "dx", 46 },
	{ "dy", 47 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "b-moveCnt", 48 },
	{ "b-i1", 49 },
	{ "b-i2", 50 },
	{ "b-di", 51 },
	{ "b-xAxis", 52 },
	{ "b-incr", 53 },
	{ "completed", 210 },
	{ "illegalBits", 18 },
	{ "dispose", 105 },
	{ "prevSignal", 149 },
	{ "message", 40 },
	{ "modifiers", 64 },
	{ "cue", 136 },
	{ "owner", 150 },
	{ "handle", 93 },
	{ "number", 43 },
	{ "max", 37 },
	{ "cursor", 36 },
	{ "claimed", 76 },
	{ "edgeHit", 321 },
	{ "wordFail", 71 },
	{ "syntaxFail", 72 },
	{ "semanticFail", 73 },
	{ "cycler", 215 },
	{ "elements", 27 },
	{ "lsTop", 13 },
	{ "lsBottom", 15 },
	{ "lsLeft", 14 },
	{ "lsRight", 16 },
	{ "baseSetter", 290 },
	{ "who", 39 },
	{ "distance", 224 },
	{ "mover", 59 },
	{ "looper", 62 },
	{ "isBlocked", 61 },
	{ "heading", 58 },
	{ "mode", 30 },
	{ "caller", 134 },
	{ "moveDone", 100 },
	{ "vol", 97 },
	{ "pri", 98 },
	{ "min", 94 },
	{ "sec", 95 },
	{ "frame", 96 },
	{ "dataInc", 92 },
	{ "size", 89 },
	{ "palette", 91 },
	{ "moveSpeed", 56 },
	{ "cantBeHere", 57 },
	{ "nodePtr", 44 },
	{ "flags", 102 },
	{ "points", 90 },
	{ "syncCue", 248 },
	{ "syncTime", 247 },
	{ "printLang", 87 },
	{ "subtitleLang", 88 },
	{ "parseLang", 86 },
	{ "setVol", 156 },
	{ "motionCue", 213 },
	{ "setTarget", 221 },
	{ "egoMoveSpeed", 370 }
};
	
// Taken from Codename: Iceman (Full Game)
static const SelectorRemap iceman_demo_selectors[] = {
	{ "init", 87 },
	{ "play", 42 },
	{ "replay", 65 },
	{ "x", 4 },
	{ "y", 3 },
	{ "z", 85 },
	{ "priority", 63 },
	{ "view", 5 },
	{ "loop", 6 },
	{ "cel", 7 },
	{ "brLeft", 20 },
	{ "brRight", 22 },
	{ "brTop", 19 },
	{ "brBottom", 21 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "nsBottom", 11 },
	{ "nsTop", 9 },
	{ "nsLeft", 10 },
	{ "nsRight", 12 },
	{ "font", 33 },
	{ "text", 26 },
	{ "type", 34 },
	{ "state", 32 },
	{ "doit", 60 },
	{ "delete", 84 },
	{ "signal", 17 },
	{ "underBits", 8 },
	{ "canBeHere", 57 },
	{ "client", 45 },
	{ "dx", 46 },
	{ "dy", 47 },
	{ "xStep", 54 },
	{ "yStep", 55 },
	{ "b-moveCnt", 48 },
	{ "b-i1", 49 },
	{ "b-i2", 50 },
	{ "b-di", 51 },
	{ "b-xAxis", 52 },
	{ "b-incr", 53 },
	{ "completed", 159 },
	{ "illegalBits", 18 },
	{ "dispose", 88 },
	{ "prevSignal", 129 },
	{ "message", 40 },
	{ "modifiers", 64 },
	{ "cue", 121 },
	{ "owner", 130 },
	{ "handle", 44 },
	{ "number", 43 },
	{ "max", 37 },
	{ "cursor", 36 },
	{ "claimed", 76 },
	{ "edgeHit", 236 },
	{ "wordFail", 71 },
	{ "syntaxFail", 72 },
	{ "semanticFail", 73 },
	{ "cycler", 164 },
	{ "elements", 27 },
	{ "lsTop", 13 },
	{ "lsBottom", 15 },
	{ "lsLeft", 14 },
	{ "lsRight", 16 },
	{ "baseSetter", 207 },
	{ "who", 39 },
	{ "distance", 173 },
	{ "mover", 59 },
	{ "looper", 62 },
	{ "isBlocked", 61 },
	{ "heading", 58 },
	{ "mode", 30 },
	{ "caller", 119 },
	{ "moveDone", 170 },
	{ "size", 96 },
	{ "moveSpeed", 56 },
	{ "flags", 368 },
	{ "points", 316 },
	{ "motionCue", 162 },
	{ "setTarget", 171 }
};

// A macro for loading one of the above tables in the function below
#define USE_SELECTOR_TABLE(x) \
	do { \
		for (uint32 i = 0; i < ARRAYSIZE(x); i++) { \
			if (x[i].slot >= names.size()) \
				names.resize(x[i].slot + 1); \
			names[x[i].slot] = x[i].name; \
		} \
	} while (0)

Common::StringList Kernel::checkStaticSelectorNames() {
	Common::String gameID = ((SciEngine*)g_engine)->getGameID();
	
	Common::StringList names;
	
	if (gameID == "kq4sci")
		USE_SELECTOR_TABLE(kq4_demo_selectors);
	else if (gameID == "lsl3" || gameID == "iceman") // identical, except iceman has "flags" 
		USE_SELECTOR_TABLE(iceman_demo_selectors);
	else if (gameID == "christmas1992")
		USE_SELECTOR_TABLE(christmas1992_selectors);
	else if (gameID == "lsl1sci")
		USE_SELECTOR_TABLE(lsl1_demo_selectors);
	
	return names;
}
	
} // End of namespace Sci

#endif // SCI_STATIC_SELECTORS_H
