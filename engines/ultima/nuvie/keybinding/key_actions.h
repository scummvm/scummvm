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

#ifndef NUVIE_KEYBINDING_KEY_ACTIONS_H
#define NUVIE_KEYBINDING_KEY_ACTIONS_H

namespace Ultima {
namespace Nuvie {

void ActionWalkWest(int param);
void ActionWalkEast(int param);
void ActionWalkNorth(int param);
void ActionWalkSouth(int param);
void ActionWalkNorthEast(int param);
void ActionWalkSouthEast(int param);
void ActionWalkNorthWest(int param);
void ActionWalkSouthWest(int param);

void ActionCast(int param);
void ActionLook(int param);
void ActionTalk(int param);
void ActionUse(int param);
void ActionGet(int param);
void ActionMove(int param); //PUSH EVENT
void ActionDrop(int param);
void ActionToggleCombat(int param);
void ActionAttack(int param);
void ActionRest(int param);
void ActionMultiUse(int param);
void ActionSelectCommandBar(int param);
void ActionSelectNewCommandBar(int param);

void ActionDollGump(int param);
void ActionShowStats(int param);
void ActionInventory(int param);
void ActionPartyView(int param);
void ActionNextPartyMember(int param);
void ActionPreviousPartyMember(int param);
void ActionHome(int param);
void ActionEnd(int param);
void ActionToggleView(int param);

void ActionSoloMode(int param);
void ActionPartyMode(int param);

void ActionSaveDialog(int param);
void ActionLoadLatestSave(int param);
void ActionQuickSave(int param);
void ActionQuickLoad(int param);
void ActionQuitDialog(int param);
void ActionQuitNODialog(int param);
void ActionGameMenuDialog(int param);

void ActionToggleFullscreen(int param);
void ActionToggleCursor(int param);
void ActionToggleCombatStrategy(int param);
void ActionToggleFps(int param);
void ActionToggleAudio(int param);
void ActionToggleMusic(int param);
void ActionToggleSFX(int param);
void ActionToggleOriginalStyleCommandBar(int param);

void ActionDoAction(int param);
void ActionCancelAction(int param);

void ActionMsgScrollUP(int param);
void ActionMsgScrollDown(int param);
void ActionShowKeys(int param);
void ActionDecreaseDebug(int param);
void ActionIncreaseDebug(int param);
void ActionCloseGumps(int param);
void ActionToggleAltCodeMode(int param);
void ActionAppendAltCode(int param);
void ActionUseItem(int param);

void ActionAssetViewer(int param);
void ActionShowEggs(int param);
void ActionToggleHackmove(int param);
void ActionToggleEggSpawn(int param);
void ActionToggleUnlimitedCasting(int param);
void ActionToggleNoDarkness(int param);
void ActionTogglePickpocket(int param);
void ActionToggleGodMode(int param);
void ActionToggleEthereal(int param);
void ActionToggleX_Ray(int param);
void ActionHealParty(int param);
void ActionTeleportToCursor(int param);
void ActionToggleCheats(int param);

void ActionDoNothing(int param);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
