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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NUVIE_KEYBINDING_KEY_ACTIONS_H
#define NUVIE_KEYBINDING_KEY_ACTIONS_H

namespace Ultima {
namespace Nuvie {

void ActionWalkWest(int const *params);
void ActionWalkEast(int const *params);
void ActionWalkNorth(int const *params);
void ActionWalkSouth(int const *params);
void ActionWalkNorthEast(int const *params);
void ActionWalkSouthEast(int const *params);
void ActionWalkNorthWest(int const *params);
void ActionWalkSouthWest(int const *params);

void ActionCast(int const *params);
void ActionLook(int const *params);
void ActionTalk(int const *params);
void ActionUse(int const *params);
void ActionGet(int const *params);
void ActionMove(int const *params); //PUSH EVENT
void ActionDrop(int const *params);
void ActionToggleCombat(int const *params);
void ActionAttack(int const *params);
void ActionRest(int const *params);
void ActionMultiUse(int const *params);
void ActionSelectCommandBar(int const *params);
void ActionSelectNewCommandBar(int const *params);

void ActionDollGump(int const *params);
void ActionShowStats(int const *params);
void ActionInventory(int const *params);
void ActionPartyView(int const *params);
void ActionNextPartyMember(int const *params);
void ActionPreviousPartyMember(int const *params);
void ActionHome(int const *params);
void ActionEnd(int const *params);
void ActionToggleView(int const *params);

void ActionSoloMode(int const *params);
void ActionPartyMode(int const *params);

void ActionSaveDialog(int const *params);
void ActionLoadLatestSave(int const *params);
void ActionQuickSave(int const *params);
void ActionQuickLoad(int const *params);
void ActionQuitDialog(int const *params);
void ActionQuitNODialog(int const *params);
void ActionGameMenuDialog(int const *params);

void ActionToggleFullscreen(int const *params);
void ActionToggleCursor(int const *params);
void ActionToggleCombatStrategy(int const *params);
void ActionToggleFps(int const *params);
void ActionToggleAudio(int const *params);
void ActionToggleMusic(int const *params);
void ActionToggleSFX(int const *params);
void ActionToggleOriginalStyleCommandBar(int const *params);

void ActionDoAction(int const *params);
void ActionCancelAction(int const *params);

void ActionMsgScrollUP(int const *params);
void ActionMsgScrollDown(int const *params);
void ActionShowKeys(int const *params);
void ActionDecreaseDebug(int const *params);
void ActionIncreaseDebug(int const *params);
void ActionCloseGumps(int const *params);
void ActionUseItem(int const *params);

void ActionShowEggs(int const *params);
void ActionToggleHackmove(int const *params);
void ActionToggleEggSpawn(int const *params);
void ActionToggleUnlimitedCasting(int const *params);
void ActionToggleNoDarkness(int const *params);
void ActionTogglePickpocket(int const *params);
void ActionToggleGodMode(int const *params);
void ActionToggleEthereal(int const *params);
void ActionToggleX_Ray(int const *params);
void ActionHealParty(int const *params);
void ActionTeleportToCursor(int const *params);
void ActionToggleCheats(int const *params);

void ActionDoNothing(int const *params);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
