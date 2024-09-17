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

#ifndef AGS_SHARED_AC_CHARACTER_INFO_H
#define AGS_SHARED_AC_CHARACTER_INFO_H

#include "common/std/vector.h"
#include "ags/shared/ac/common_defines.h" // constants
#include "ags/shared/ac/game_version.h"
#include "ags/shared/core/types.h"
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/string.h"


namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define MAX_INV             301
// Character flags
#define CHF_MANUALSCALING   1
#define CHF_FIXVIEW         2     // between SetCharView and ReleaseCharView
#define CHF_NOINTERACT      4
#define CHF_NODIAGONAL      8
#define CHF_ALWAYSIDLE      0x10
#define CHF_NOLIGHTING      0x20
#define CHF_NOTURNING       0x40
#define CHF_NOWALKBEHINDS   0x80
#define CHF_FLIPSPRITE      0x100  // ?? Is this used??
#define CHF_NOBLOCKING      0x200
#define CHF_SCALEMOVESPEED  0x400
#define CHF_NOBLINKANDTHINK 0x800
#define CHF_SCALEVOLUME     0x1000
#define CHF_HASTINT         0x2000   // engine only
#define CHF_BEHINDSHEPHERD  0x4000   // engine only
#define CHF_AWAITINGMOVE    0x8000   // engine only
#define CHF_MOVENOTWALK     0x10000   // engine only - do not do walk anim
#define CHF_ANTIGLIDE       0x20000
#define CHF_HASLIGHT        0x40000
#define CHF_TINTLIGHTMASK   (CHF_NOLIGHTING | CHF_HASTINT | CHF_HASLIGHT)
// Speechcol is no longer part of the flags as of v2.5
#define OCHF_SPEECHCOL      0xff000000
#define OCHF_SPEECHCOLSHIFT 24
#define UNIFORM_WALK_SPEED  0
#define FOLLOW_ALWAYSONTOP  0x7ffe

// Character's internal flags, packed in CharacterInfo::animating
#define CHANIM_MASK         0xFF
#define CHANIM_ON           0x01
#define CHANIM_REPEAT       0x02
#define CHANIM_BACKWARDS    0x04

// Converts character flags (CHF_*) to matching RoomObject flags (OBJF_*)
inline int CharFlagsToObjFlags(int chflags) {
	using namespace AGS::Shared;
	return FlagToFlag(chflags, CHF_NOINTERACT, OBJF_NOINTERACT) |
		   FlagToFlag(chflags, CHF_NOWALKBEHINDS, OBJF_NOWALKBEHINDS) |
		   FlagToFlag(chflags, CHF_HASTINT, OBJF_HASTINT) |
		   FlagToFlag(chflags, CHF_HASLIGHT, OBJF_HASLIGHT) |
		   // following flags are inverse
		   FlagToNoFlag(chflags, CHF_NOLIGHTING, OBJF_USEREGIONTINTS) |
		   FlagToNoFlag(chflags, CHF_MANUALSCALING, OBJF_USEROOMSCALING) |
		   FlagToNoFlag(chflags, CHF_NOBLOCKING, OBJF_SOLID);
}

// Length of deprecated character name field, in bytes
#define LEGACY_MAX_CHAR_NAME_LEN 40

// CharacterInfoBase contains original set of character fields.
// It's picked out from CharacterInfo for convenience of adding
// new design-time fields; easier to maintain backwards compatibility.
// IMPORTANT: exposed to script API, and plugin API as AGSCharacter!
// For older script compatibility the struct also has to maintain its size;
// do not extend or change existing fields, unless planning breaking compatibility.
// Prefer to use CharacterExtras struct for any extensions.
struct CharacterInfoBase {
	int   defview;
	int   talkview;
	int   view;
	int   room, prevroom;
	int   x, y, wait;
	int   flags;
	short following;
	short followinfo;
	int   idleview;           // the loop will be randomly picked
	short idletime, idleleft; // num seconds idle before playing anim
	short transparency;       // if character is transparent
	short baseline;
	int   activeinv;
	int   talkcolor;
	int   thinkview;
	short blinkview, blinkinterval; // design time
	short blinktimer, blinkframe;   // run time
	short walkspeed_y;
	short pic_yoffs; // this is fixed in screen coordinates
	int   z;    // z-location, for flying etc
	int   walkwait;
	short speech_anim_speed, idle_anim_speed;
	short blocking_width, blocking_height;
	int   index_id;  // used for object functions to know the id
	short pic_xoffs; // this is fixed in screen coordinates
	short walkwaitcounter;
	uint16_t loop, frame;
	short walking;  // stores movelist index, optionally +TURNING_AROUND
	short animating; // stores CHANIM_* flags in lower byte and delay in upper byte
	short walkspeed, animspeed;
	short inv[MAX_INV];
	short actx, acty;
	// These two name fields are deprecated, but must stay here
	// for compatibility with the plugin API (unless the plugin interface is reworked)
	char legacy_name[LEGACY_MAX_CHAR_NAME_LEN];
	char legacy_scrname[LEGACY_MAX_SCRIPT_NAME_LEN];
	int8  on;
};


struct CharacterExtras;

// Design-time Character data.
// TODO: must refactor, some parts of it should be in a runtime Character class.
struct CharacterInfo : public CharacterInfoBase {
	AGS::Shared::String scrname;
	AGS::Shared::String name;

	int get_effective_y() const;   // return Y - Z
	int get_baseline() const;      // return baseline, or Y if not set
	int get_blocking_top() const;    // return Y - BlockingHeight/2
	int get_blocking_bottom() const; // return Y + BlockingHeight/2

	// Returns effective x/y walkspeeds for this character
	void get_effective_walkspeeds(int &walk_speed_x, int &walk_speed_y) const {
		walk_speed_x = walkspeed;
		walk_speed_y = ((walkspeed_y == UNIFORM_WALK_SPEED) ? walkspeed : walkspeed_y);
	}

	inline bool has_explicit_light() const {
		return (flags & CHF_HASLIGHT) != 0;
	}
	inline bool has_explicit_tint()  const {
		return (flags & CHF_HASTINT) != 0;
	}
	inline bool is_animating() const {
		return (animating & CHANIM_ON) != 0;
	}
	inline int get_anim_repeat() const {
		return (animating & CHANIM_REPEAT) ? ANIM_REPEAT : ANIM_ONCE;
	}
	inline bool get_anim_forwards() const {
		return (animating & CHANIM_BACKWARDS) == 0;
	}
	inline int get_anim_delay() const {
		return (animating >> 8) & 0xFF;
	}
	inline void set_animating(bool repeat, bool forwards, int delay) {
		animating = CHANIM_ON |
					(CHANIM_REPEAT * repeat) |
					(CHANIM_BACKWARDS * !forwards) |
					((delay & 0xFF) << 8);
	}

	// [IKM] 2012-06-28: I still have to pass char_index to some of those functions
	// either because they use it to set some variables with it,
	// or because they pass it further to other functions, that are called from various places
	// and it would be too much to change them all simultaneously
	//
	// [IKM] 2016-08-26: these methods should NOT be in CharacterInfo class,
	// bit in distinct runtime character class!
	void UpdateMoveAndAnim(int &char_index, CharacterExtras *chex, std::vector<int> &followingAsSheep);
	void UpdateFollowingExactlyCharacter();

	int  update_character_walking(CharacterExtras *chex);
	void update_character_moving(int &char_index, CharacterExtras *chex, int &doing_nothing);
	int  update_character_animating(int &char_index, int &doing_nothing);
	void update_character_idle(CharacterExtras *chex, int &doing_nothing);
	void update_character_follower(int &char_index, std::vector<int> &followingAsSheep, int &doing_nothing);

	void ReadFromFile(Shared::Stream *in, GameDataVersion data_ver);
	void WriteToFile(Shared::Stream *out) const;
	// TODO: move to runtime-only class (?)
	void ReadFromSavegame(Shared::Stream *in);
	void WriteToSavegame(Shared::Stream *out) const;

private:
	// TODO: this is likely temp here until runtime class is factored out
	void ReadFromFileImpl(Shared::Stream *in, bool is_save);
	void WriteToFileImpl(Shared::Stream *out, bool is_save) const;
};

#if defined (OBSOLETE)
struct OldCharacterInfo {
	int   defview;
	int   talkview;
	int   view;
	int   room, prevroom;
	int   x, y, wait;
	int   flags;
	short following;
	short followinfo;
	int   idleview;           // the loop will be randomly picked
	short idletime, idleleft; // num seconds idle before playing anim
	short transparency;       // if character is transparent
	short baseline;
	int   activeinv;          // this is an INT to support SeeR (no signed shorts)
	short loop, frame;
	short walking, animating;
	short walkspeed, animspeed;
	short inv[100];
	short actx, acty;
	char  name[30];
	char  scrname[16];
	int8  on;
};

void ConvertOldCharacterToNew(OldCharacterInfo *oci, CharacterInfo *ci);
#endif // OBSOLETE

} // namespace AGS3

#endif
