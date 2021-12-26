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

#ifndef AGS_ENGINE_AC_INVENTORY_ITEM_H
#define AGS_ENGINE_AC_INVENTORY_ITEM_H

#include "ags/engine/ac/dynobj/script_inv_item.h"

namespace AGS3 {

void InventoryItem_SetCursorGraphic(ScriptInvItem *iitem, int newSprite);
int  InventoryItem_GetCursorGraphic(ScriptInvItem *iitem);
void InventoryItem_SetGraphic(ScriptInvItem *iitem, int piccy);
void InventoryItem_SetName(ScriptInvItem *scii, const char *newname);
int  InventoryItem_GetID(ScriptInvItem *scii);
ScriptInvItem *GetInvAtLocation(int xx, int yy);
void InventoryItem_GetName(ScriptInvItem *iitem, char *buff);
const char *InventoryItem_GetName_New(ScriptInvItem *invitem);
int  InventoryItem_GetGraphic(ScriptInvItem *iitem);
void InventoryItem_RunInteraction(ScriptInvItem *iitem, int mood);
int  InventoryItem_CheckInteractionAvailable(ScriptInvItem *iitem, int mood);
int  InventoryItem_GetProperty(ScriptInvItem *scii, const char *property);
void InventoryItem_GetPropertyText(ScriptInvItem *scii, const char *property, char *bufer);
const char *InventoryItem_GetTextProperty(ScriptInvItem *scii, const char *property);

void set_inv_item_cursorpic(int invItemId, int piccy);

} // namespace AGS3

#endif
