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

#ifndef AGS_SHARED_AC_OLD_GAME_SETUP_STRUCT_H
#define AGS_SHARED_AC_OLD_GAME_SETUP_STRUCT_H

#if defined (OBSOLETE)

#include "ags/shared/ac/character_info.h"       // OldCharacterInfo, CharacterInfo
#include "ags/shared/ac/event_block.h"       // EventBlock
#include "ags/shared/ac/interface_element.h"    // InterfaceElement
#include "ags/shared/ac/inventory_item_info.h"   // InventoryItemInfo
#include "ags/shared/ac/mouse_cursor.h"      // MouseCursor
#include "ags/shared/ac/words_dictionary.h"  // WordsDictionary
#include "ags/shared/script/cc_script.h"           // ccScript

namespace AGS3 {

struct OriGameSetupStruct {
	char              gamename[30];
	int8              options[20];
	unsigned char     paluses[256];
	RGB               defpal[256];
	InterfaceElement  iface[10];
	int               numiface;
	int               numviews;
	MouseCursor       mcurs[10];
	char *globalscript;
	int               numcharacters;
	OldCharacterInfo *chars;
#if defined (OBSOLETE)
	EventBlock        __charcond[50];
	EventBlock        __invcond[100];
#endif
	ccScript *compiled_script;
	int               playercharacter;
	unsigned char     __old_spriteflags[2100];
	int               totalscore;
	short             numinvitems;
	InventoryItemInfo invinfo[100];
	int               numdialog, numdlgmessage;
	int               numfonts;
	int               color_depth;              // in bytes per pixel (ie. 1 or 2)
	int               target_win;
	int               dialog_bullet;            // 0 for none, otherwise slot num of bullet point
	short             hotdot, hotdotouter;   // inv cursor hotspot dot
	int               uniqueid;    // random key identifying the game
	int               reserved[2];
	short             numlang;
	char              langcodes[MAXLANGUAGE][3];
	char *messages[MAXGLOBALMES];
};

struct OriGameSetupStruct2 : public OriGameSetupStruct {
	unsigned char   fontflags[10];
	int8            fontoutline[10];
	int             numgui;
	WordsDictionary *dict;
	int             reserved2[8];
};

struct OldGameSetupStruct : public OriGameSetupStruct2 {
	unsigned char spriteflags[LEGACY_MAX_SPRITES_V25];
};

} // namespace AGS3

#endif

#endif
