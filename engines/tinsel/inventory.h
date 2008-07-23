
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
 * Inventory related functions
 */

#ifndef TINSEL_INVENTORY_H	// prevent multiple includes
#define TINSEL_INVENTORY_H

#include "tinsel/dw.h"
#include "tinsel/events.h"	// for KEYEVENT, BUTEVENT

namespace Tinsel {

class Serializer;

enum {
	INV_OPEN	= -1,
	INV_CONV	= 0,
	INV_1		= 1,
	INV_2		= 2,
	INV_CONF	= 3,

	NUM_INV		= 4
};

/** structure of each inventory object */
struct INV_OBJECT {
	int32 id;		// inventory objects id
	SCNHANDLE hFilm;	// inventory objects animation film
	SCNHANDLE hScript;	// inventory objects event handling script
	int32 attribute;		// inventory object's attribute
};
typedef INV_OBJECT *PINV_OBJECT;

void PopUpInventory(int invno);

enum CONFTYPE {
	SAVE, LOAD, QUIT, OPTION, RESTART, SOUND, CONTROLS, SUBT, TOPWIN
};

void PopUpConf(CONFTYPE type);


void Xmovement(int x);
void Ymovement(int y);

void ButtonToInventory(BUTEVENT be);

void KeyToInventory(KEYEVENT ke);


int WhichItemHeld(void);

void HoldItem(int item);
void DropItem(int item);
void AddToInventory(int invno, int icon, bool hold);
bool RemFromInventory(int invno, int icon);


void RegisterIcons(void *cptr, int num);

void idec_convw(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
void idec_inv1(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
void idec_inv2(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

bool InventoryActive(void);

void AddIconToPermanentDefaultList(int icon);

void convPos(int bpos);
void ConvPoly(HPOLYGON hp);
int convIcon(void);
void CloseDownConv(void);
void convHide(bool hide);
bool convHid(void);

enum {
	INV_NOICON		= -1,
	INV_CLOSEICON	= -2,
	INV_OPENICON	= -3,
	INV_HELDNOTIN	= -4
};

void ConvAction(int index);

void InventoryIconCursor(void);

void setInvWinParts(SCNHANDLE hf);
void setFlagFilms(SCNHANDLE hf);
void setConfigStrings(SCNHANDLE *tp);

int InvItem(int *x, int *y, bool update);
int InvItemId(int x, int y);

int InventoryPos(int num);

bool IsInInventory(int object, int invnum);

void KillInventory(void);

void invObjectFilm(int object, SCNHANDLE hFilm);

void syncInvInfo(Serializer &s);

int InvGetLimit(int invno);
void InvSetLimit(int invno, int n);
void InvSetSize(int invno, int MinWidth, int MinHeight,
		int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

int WhichInventoryOpen(void);

bool IsTopWindow(void);
bool IsConfWindow(void);
bool IsConvWindow(void);

} // end of namespace Tinsel

#endif /* TINSEL_INVENTRY_H */
