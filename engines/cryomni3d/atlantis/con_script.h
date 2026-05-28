/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Parser for Atlantis chapter scenario scripts (.CON files in SCENAR\).
//
// CON file structure:
//   /INIT ... /FININIT  — chapter initialisation commands
//   /N                  — section header (decimal integer)
//   /con(conditions)    — player-triggered condition (NPC click)
//   /con*(conditions)   — same but requires item in hand
//   /sel(conditions)    — automatic condition (departure/arrival/game-state check)
//   /sujN               — dialog subject/topic ID for subject-menu gating
//   /goN                — redirect execution to section N
//   /set(command)       — game-state mutation
//   [Speaker,params] text — dialog line
//   /000                — end of script
//
// Condition string format (for both /con and /sel):
//   (key op val)&(key op val)&...   where op is '=', '<', or '>'
//
// Well-known condition keys:
//   departvue   — which WAM zone the player just clicked to depart (/sel only)
//   vue         — which panoramic place is currently active
//   arriveevue  — set by engine to the arrived-at place ID when entering a place
//   clicperso   — which NPC was clicked (/con only)
//   All other keys are general game variables stored in CryOmni3DEngine_Atlantis::_gameVars.

#ifndef CRYOMNI3D_ATLANTIS_CON_SCRIPT_H
#define CRYOMNI3D_ATLANTIS_CON_SCRIPT_H

#include "common/array.h"
#include "common/str.h"

namespace CryOmni3D {
namespace Atlantis {

struct ConLine {
	Common::String speaker;  // e.g. "Seth", "Garde13"
	Common::String params;   // e.g. "cam3", "off" (comma-separated in source)
	Common::String text;

	// Optional panorama angle hints, sourced directly from the CON dialog
	// directive — when present, the original engine writes them straight into
	// the cyclo view's alpha/beta before playing the line (atlantis.exe.c
	// FUN_004283e7 line 22381+22543, then FUN_00451915 line 47181 stores into
	// the global current-view angle).  Syntax in CON files:
	//   [Speaker, X, Y, camN] text       — X,Y present (hasAngle = true)
	//   [Speaker, camN] text             — no numeric tokens (hasAngle = false)
	// X is a panorama-pixel offset for the horizontal angle; Y is the vertical
	// offset.  The dialog renderer converts them to radians using the
	// panorama dimensions (no hardcoded scale factor).
	bool hasAngle = false;
	int  angleX   = 0;
	int  angleY   = 0;
};

// One item in a section's ordered execution list.
// Either a dialog line (!isSet) or a /set command (isSet).
// Preserves the original CON file ordering so cinematics play before or after dialog
// exactly as authored.
struct ConSectionItem {
	bool           isSet = false;
	ConLine        line;    // valid when !isSet
	Common::String setCmd;  // valid when  isSet
};

// One parsed condition from a /con(...) or /sel(...) compound condition string.
//
// Conditions evaluate left-to-right with short-circuit semantics that match
// the exe (atlantis.exe FUN_004298ac):
//   nextSep = '&' : if cond fails -> EXIT FAIL ; if passes -> continue
//   nextSep = '|' : if cond passes -> EXIT SUCCESS ; if fails -> continue
//   nextSep = '\0': last cond; its result is the overall result.
// So `A & B | C & D` reads as `A AND (B OR (C AND D))` — neither standard nor
// OR-tighter precedence; just a left-to-right chain.  All multi-clause CON
// conditions in the game data (CHAPI*.CON) are authored under this rule, e.g.
// /sel(parletemp3=3)&(departvue=38)|(56)|(22) gates the OR-list on the
// leading parletemp3=3 guard the way the exe evaluates it.
struct ConCondition {
	Common::String key;    // lowercase key name
	char           op;     // '=', '<', '>', '!' (not equal)
	int            value;
	char           nextSep = '\0';  // '&', '|', or '\0' (end of cond list)
};

struct ConSection {
	int  id        = 0;
	bool isCon     = false;   // /con or /con* (player-triggered)
	bool needsItem = false;   // /con* (item in hand required)
	bool isTimer   = false;   // /tim* (timer-driven automatic condition)
	int  timerNum  = 0;       // timer index (1 or 2) from /tim1 or /tim2
	int  clicPerso = 0;       // ClicPerso=N from conditions (0 = none); kept for interactNPC()
	int  clicZone  = 0;       // ClicZone=N from conditions (0 = none); for zone-click /con* sections
	int  sujId     = 0;       // /sujN subject gate (0 = always available;
	                          // first id from a comma-list; see sujIds)
	int  gotoId    = 0;       // /goN redirect (0 = no redirect)
	mutable bool played = false;  // set after first execution (for one-shot arrival sections)

	// /suj supports a comma-separated list (`/suj1,2` means "this
	// section fires when ANY of subjects 1 or 2 is enabled for the
	// clicked perso").  CHAPI005 section 113 is the canonical example:
	// `/con(FlagChp8=2)&(...) /suj1,2 /go89` -- the game-over branch is
	// only reachable when interactNPC() treats the section as available
	// for either suj 1 OR suj 2.  sujId == sujIds[0] for the menu icon.
	Common::Array<int>             sujIds;

	Common::Array<ConCondition>    conditions;  // all parsed conditions
	Common::Array<ConSectionItem>  items;       // dialog lines and /set commands in source order
};

// Parses a CON file buffer into sections and INIT commands.
class ConScript {
public:
	void parse(const char *buf, uint32 len);
	void reset();

	Common::Array<ConSection>       &sections()  { return _sections; }
	const Common::Array<ConSection> &sections()  const { return _sections; }
	const Common::Array<Common::String> &initCmds() const { return _initCmds; }

private:
	Common::Array<ConSection>    _sections;
	Common::Array<Common::String> _initCmds;
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_CON_SCRIPT_H
