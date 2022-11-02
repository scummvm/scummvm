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

#include "ags/lib/std/vector.h"
#include "ags/shared/ac/common_defines.h" // constants
#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define MAX_INV             301
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
// Speechcol is no longer part of the flags as of v2.5
#define OCHF_SPEECHCOL      0xff000000
#define OCHF_SPEECHCOLSHIFT 24
#define UNIFORM_WALK_SPEED  0
#define FOLLOW_ALWAYSONTOP  0x7ffe

struct CharacterExtras; // forward declaration
// IMPORTANT: exposed to script API, and plugin API as AGSCharacter!
// For older script compatibility the struct also has to maintain its size;
// do not extend or change existing fields, unless planning breaking compatibility.
// Use CharacterExtras struct for any extensions
struct CharacterInfo {
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
	short walking, animating;
	short walkspeed, animspeed;
	short inv[MAX_INV];
	short actx, acty;
	char  name[40];
	char  scrname[MAX_SCRIPT_NAME_LEN];
	int8  on;

	int get_effective_y();   // return Y - Z
	int get_baseline();      // return baseline, or Y if not set
	int get_blocking_top();    // return Y - BlockingHeight/2
	int get_blocking_bottom(); // return Y + BlockingHeight/2

	inline bool has_explicit_light() const {
		return (flags & CHF_HASLIGHT) != 0;
	}
	inline bool has_explicit_tint()  const {
		return (flags & CHF_HASTINT) != 0;
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

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
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
