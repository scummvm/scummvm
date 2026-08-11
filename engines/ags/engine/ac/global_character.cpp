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

//=============================================================================
//
// AGS Character functions
//
//=============================================================================

#include "ags/engine/ac/global_character.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_overlay.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/script/script.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;









// defined in character unit






void StopMoving(int chaa) {

	Character_StopMoving(&_GP(game).chars[chaa]);
}

void ReleaseCharacterView(int chat) {
	if (!is_valid_character(chat))
		quit("!ReleaseCahracterView: invalid character supplied");

	Character_UnlockView(&_GP(game).chars[chat]);
}

void MoveToWalkableArea(int charid) {
	if (!is_valid_character(charid))
		quit("!MoveToWalkableArea: invalid character specified");

	Character_PlaceOnWalkableArea(&_GP(game).chars[charid]);
}

void FaceLocation(int cha, int xx, int yy) {
	if (!is_valid_character(cha))
		quit("!FaceLocation: Invalid character specified");

	Character_FaceLocation(&_GP(game).chars[cha], xx, yy, BLOCKING);
}

void FaceCharacter(int cha, int toface) {
	if (!is_valid_character(cha))
		quit("!FaceCharacter: Invalid character specified");
	if (!is_valid_character(toface))
		quit("!FaceCharacter: invalid character specified");

	Character_FaceCharacter(&_GP(game).chars[cha], &_GP(game).chars[toface], BLOCKING);
}


void SetCharacterIdle(int who, int iview, int itime) {
	if (!is_valid_character(who))
		quit("!SetCharacterIdle: Invalid character specified");

	Character_SetIdleView(&_GP(game).chars[who], iview, itime);
}



int GetCharacterWidth(int ww) {
	CharacterInfo *char1 = &_GP(game).chars[ww];

	if (_GP(charextra)[ww].width < 1) {
		if ((char1->view < 0) ||
			(char1->loop >= _GP(views)[char1->view].numLoops) ||
			(char1->frame >= _GP(views)[char1->view].loops[char1->loop].numFrames)) {
			debug_script_warn("GetCharacterWidth: Character %s has invalid frame: view %d, loop %d, frame %d", char1->scrname, char1->view + 1, char1->loop, char1->frame);
			return data_to_game_coord(4);
		}

		return _GP(game).SpriteInfos[_GP(views)[char1->view].loops[char1->loop].frames[char1->frame].pic].Width;
	} else
		return _GP(charextra)[ww].width;
}

int GetCharacterHeight(int charid) {
	CharacterInfo *char1 = &_GP(game).chars[charid];

	if (_GP(charextra)[charid].height < 1) {
		if ((char1->view < 0) ||
		        (char1->loop >= _GP(views)[char1->view].numLoops) ||
		        (char1->frame >= _GP(views)[char1->view].loops[char1->loop].numFrames)) {
			debug_script_warn("GetCharacterHeight: Character %s has invalid frame: view %d, loop %d, frame %d",
							  char1->scrname, char1->view + 1, char1->loop, char1->frame);
			return data_to_game_coord(2);
		}

		return _GP(game).SpriteInfos[_GP(views)[char1->view].loops[char1->loop].frames[char1->frame].pic].Height;
	} else
		return _GP(charextra)[charid].height;
}



void SetCharacterBaseline(int obn, int basel) {
	if (!is_valid_character(obn)) quit("!SetCharacterBaseline: invalid object number specified");

	Character_SetBaseline(&_GP(game).chars[obn], basel);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
void SetCharacterTransparency(int obn, int trans) {
	if (!is_valid_character(obn))
		quit("!SetCharTransparent: invalid character number specified");

	Character_SetTransparency(&_GP(game).chars[obn], trans);
}

void AnimateCharacter4(int chh, int loopn, int sppd, int rept) {
	AnimateCharacter6(chh, loopn, sppd, rept, FORWARDS, IN_BACKGROUND);
}

void AnimateCharacter6(int chh, int loopn, int sppd, int rept, int direction, int blocking) {
	if (!is_valid_character(chh))
		quit("AnimateCharacter: invalid character");

	Character_Animate5(&_GP(game).chars[chh], loopn, sppd, rept, blocking, direction);
}

void SetPlayerCharacter(int newchar) {
	if (!is_valid_character(newchar))
		quit("!SetPlayerCharacter: Invalid character specified");

	Character_SetAsPlayer(&_GP(game).chars[newchar]);
}

void FollowCharacterEx(int who, int tofollow, int distaway, int eagerness) {
	if (!is_valid_character(who))
		quit("!FollowCharacter: Invalid character specified");
	CharacterInfo *chtofollow = nullptr;
	if (tofollow != -1) {
		if (!is_valid_character(tofollow))
			quit("!FollowCharacterEx: invalid character to follow");
		else
			chtofollow = &_GP(game).chars[tofollow];
	}

	Character_FollowCharacter(&_GP(game).chars[who], chtofollow, distaway, eagerness);
}

void FollowCharacter(int who, int tofollow) {
	FollowCharacterEx(who, tofollow, 10, 97);
}

void SetCharacterIgnoreLight(int who, int yesorno) {
	if (!is_valid_character(who))
		quit("!SetCharacterIgnoreLight: Invalid character specified");

	Character_SetIgnoreLighting(&_GP(game).chars[who], yesorno);
}




void MoveCharacter(int cc, int xx, int yy) {
	walk_character(cc, xx, yy, 0, true);
}
void MoveCharacterDirect(int cc, int xx, int yy) {
	walk_character(cc, xx, yy, 1, true);
}
void MoveCharacterStraight(int cc, int xx, int yy) {
	if (!is_valid_character(cc))
		quit("!MoveCharacterStraight: invalid character specified");

	Character_WalkStraight(&_GP(game).chars[cc], xx, yy, IN_BACKGROUND);
}

// Append to character path
void MoveCharacterPath(int chac, int tox, int toy) {
	if (!is_valid_character(chac))
		quit("!MoveCharacterPath: invalid character specified");

	Character_AddWaypoint(&_GP(game).chars[chac], tox, toy);
}


int GetPlayerCharacter() {
	return _GP(game).playercharacter;
}

void SetCharacterSpeedEx(int chaa, int xspeed, int yspeed) {
	if (!is_valid_character(chaa))
		quit("!SetCharacterSpeedEx: invalid character");

	Character_SetSpeed(&_GP(game).chars[chaa], xspeed, yspeed);

}

void SetCharacterSpeed(int chaa, int nspeed) {
	SetCharacterSpeedEx(chaa, nspeed, nspeed);
}

void SetTalkingColor(int chaa, int ncol) {
	if (!is_valid_character(chaa)) quit("!SetTalkingColor: invalid character");

	Character_SetSpeechColor(&_GP(game).chars[chaa], ncol);
}

void SetCharacterSpeechView(int chaa, int vii) {
	if (!is_valid_character(chaa))
		quit("!SetCharacterSpeechView: invalid character specified");

	Character_SetSpeechView(&_GP(game).chars[chaa], vii);
}

void SetCharacterBlinkView(int chaa, int vii, int intrv) {
	if (!is_valid_character(chaa))
		quit("!SetCharacterBlinkView: invalid character specified");

	Character_SetBlinkView(&_GP(game).chars[chaa], vii);
	Character_SetBlinkInterval(&_GP(game).chars[chaa], intrv);
}

void SetCharacterView(int chaa, int vii) {
	if (!is_valid_character(chaa))
		quit("!SetCharacterView: invalid character specified");

	Character_LockView(&_GP(game).chars[chaa], vii);
}

void SetCharacterFrame(int chaa, int view, int loop, int frame) {

	Character_LockViewFrame(&_GP(game).chars[chaa], view, loop, frame);
}

// similar to SetCharView, but aligns the frame to make it line up
void SetCharacterViewEx(int chaa, int vii, int loop, int align) {

	Character_LockViewAligned(&_GP(game).chars[chaa], vii, loop, ConvertLegacyScriptAlignment((LegacyScriptAlignment)align));
}

void SetCharacterViewOffset(int chaa, int vii, int xoffs, int yoffs) {

	Character_LockViewOffset(&_GP(game).chars[chaa], vii, xoffs, yoffs);
}


void ChangeCharacterView(int chaa, int vii) {
	if (!is_valid_character(chaa))
		quit("!ChangeCharacterView: invalid character specified");

	Character_ChangeView(&_GP(game).chars[chaa], vii);
}

void SetCharacterClickable(int cha, int clik) {
	if (!is_valid_character(cha))
		quit("!SetCharacterClickable: Invalid character specified");
	// make the character clicklabe (reset "No interaction" bit)
	_GP(game).chars[cha].flags &= ~CHF_NOINTERACT;
	// if they don't want it clickable, set the relevant bit
	if (clik == 0)
		_GP(game).chars[cha].flags |= CHF_NOINTERACT;
}

void SetCharacterIgnoreWalkbehinds(int cha, int clik) {
	if (!is_valid_character(cha))
		quit("!SetCharacterIgnoreWalkbehinds: Invalid character specified");

	Character_SetIgnoreWalkbehinds(&_GP(game).chars[cha], clik);
}


void MoveCharacterToObject(int chaa, int obbj) {
	// invalid object, do nothing
	// this allows MoveCharacterToObject(EGO, GetObjectAt(...));
	if (!is_valid_object(obbj))
		return;

	walk_character(chaa, _G(objs)[obbj].x + 5, _G(objs)[obbj].y + 6, 0, true);

	GameLoopUntilNotMoving(&_GP(game).chars[chaa].walking);
}

void MoveCharacterToHotspot(int chaa, int hotsp) {
	if ((hotsp < 0) || (hotsp >= MAX_ROOM_HOTSPOTS))
		quit("!MovecharacterToHotspot: invalid hotspot");
	if (_GP(thisroom).Hotspots[hotsp].WalkTo.X < 1) return;
	walk_character(chaa, _GP(thisroom).Hotspots[hotsp].WalkTo.X, _GP(thisroom).Hotspots[hotsp].WalkTo.Y, 0, true);

	GameLoopUntilNotMoving(&_GP(game).chars[chaa].walking);
}

int MoveCharacterBlocking(int chaa, int xx, int yy, int direct) {
	if (!is_valid_character(chaa))
		quit("!MoveCharacterBlocking: invalid character");

	// check if they try to move the player when Hide Player Char is
	// ticked -- otherwise this will hang the game
	if (_GP(game).chars[chaa].on != 1) {
		debug_script_warn("MoveCharacterBlocking: character is turned off (is Hide Player Character selected?) and cannot be moved");
		return 0;
	}

	if (direct)
		MoveCharacterDirect(chaa, xx, yy);
	else
		MoveCharacter(chaa, xx, yy);

	GameLoopUntilNotMoving(&_GP(game).chars[chaa].walking);

	return -1; // replicates legacy engine effect
}

int GetCharacterSpeechAnimationDelay(CharacterInfo *cha) {
	if ((_G(loaded_game_file_version) < kGameVersion_312) && (_GP(game).options[OPT_SPEECHTYPE] != 0)) {
		// legacy versions of AGS assigned a fixed delay to Sierra-style speech only
		return 5;
	}
	if (_GP(game).options[OPT_GLOBALTALKANIMSPD] != 0)
		return _GP(play).talkanim_speed;
	else
		return cha->speech_anim_speed;
}

void RunCharacterInteraction(int cc, int mood) {
	if (!is_valid_character(cc))
		quit("!RunCharacterInteraction: invalid character");

	// convert cursor mode to event index (in character event table)
	// TODO: probably move this conversion table elsewhere? should be a global info
	int evnt;
	switch (mood) {
		case MODE_LOOK:	evnt = 0; break;
		case MODE_HAND:	evnt = 1; break;
		case MODE_TALK:	evnt = 2; break;
		case MODE_USE: evnt = 3; break;
		case MODE_PICKUP: evnt = 5;	break;
		case MODE_CUSTOM1: evnt = 6; break;
		case MODE_CUSTOM2: evnt = 7; break;
		default: evnt = -1;	break;
	}
	const int anyclick_evt = 4; // TODO: make global constant (character any-click evt)

	// For USE verb: remember active inventory
	if (mood == MODE_USE) {
		_GP(play).usedinv = _G(playerchar)->activeinv;
	}

	const auto obj_evt = ObjectEvent("character%d", cc,
									 RuntimeScriptValue().SetScriptObject(&_GP(game).chars[cc], &_GP(ccDynamicCharacter)), mood);
	if (_G(loaded_game_file_version) > kGameVersion_272) {
		if ((evnt >= 0) &&
			run_interaction_script(obj_evt, _GP(game).charScripts[cc].get(), evnt, anyclick_evt) < 0)
			return; // game state changed, don't do "any click"
		run_interaction_script(obj_evt, _GP(game).charScripts[cc].get(), anyclick_evt); // any click on char
	} else {
		if ((evnt >= 0) &&
			run_interaction_event(obj_evt, _GP(game).intrChar[cc].get(), evnt, anyclick_evt, (mood == MODE_USE)) < 0)
			return; // game state changed, don't do "any click"
		run_interaction_event(obj_evt, _GP(game).intrChar[cc].get(), anyclick_evt); // any click on char
	}
}

int AreCharObjColliding(int charid, int objid) {
	if (!is_valid_character(charid))
		quit("!AreCharObjColliding: invalid character");
	if (!is_valid_object(objid))
		quit("!AreCharObjColliding: invalid object number");

	return Character_IsCollidingWithObject(&_GP(game).chars[charid], &_G(scrObj)[objid]);
}

int AreCharactersColliding(int cchar1, int cchar2) {
	if (!is_valid_character(cchar1))
		quit("!AreCharactersColliding: invalid char1");
	if (!is_valid_character(cchar2))
		quit("!AreCharactersColliding: invalid char2");

	return Character_IsCollidingWithChar(&_GP(game).chars[cchar1], &_GP(game).chars[cchar2]);
}

int GetCharacterProperty(int cha, const char *property) {
	if (!is_valid_character(cha))
		quit("!GetCharacterProperty: invalid character");
	return get_int_property(_GP(game).charProps[cha], _GP(play).charProps[cha], property);
}

void SetCharacterProperty(int who, int flag, int yesorno) {
	if (!is_valid_character(who))
		quit("!SetCharacterProperty: Invalid character specified");

	Character_SetOption(&_GP(game).chars[who], flag, yesorno);
}

void GetCharacterPropertyText(int item, const char *property, char *bufer) {
	get_text_property(_GP(game).charProps[item], _GP(play).charProps[item], property, bufer);
}

int GetCharIDAtScreen(int xx, int yy) {
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(xx, yy);
	if (vpt.second < 0)
		return -1;
	return is_pos_on_character(vpt.first.X, vpt.first.Y);
}

void SetActiveInventory(int iit) {

	ScriptInvItem *tosend = nullptr;
	if ((iit > 0) && (iit < _GP(game).numinvitems))
		tosend = &_G(scrInv)[iit];
	else if (iit != -1)
		quitprintf("!SetActiveInventory: invalid inventory number %d", iit);

	Character_SetActiveInventory(_G(playerchar), tosend);
}

void update_invorder() {
	for (int cc = 0; cc < _GP(game).numcharacters; cc++) {
		_GP(charextra)[cc].invorder_count = 0;
		int ff, howmany;
		// Iterate through all inv items, adding them once (or multiple
		// times if requested) to the list.
		for (ff = 0; ff < _GP(game).numinvitems; ff++) {
			howmany = _GP(game).chars[cc].inv[ff];
			if ((_GP(game).options[OPT_DUPLICATEINV] == 0) && (howmany > 1))
				howmany = 1;

			for (int ts = 0; ts < howmany; ts++) {
				if (_GP(charextra)[cc].invorder_count >= MAX_INVORDER)
					quit("!Too many inventory items to display: 500 max");

				_GP(charextra)[cc].invorder[_GP(charextra)[cc].invorder_count] = ff;
				_GP(charextra)[cc].invorder_count++;
			}
		}
	}
	// backwards compatibility
	_GP(play).inv_numorder = _GP(charextra)[_GP(game).playercharacter].invorder_count;
	GUI::MarkInventoryForUpdate(_GP(game).playercharacter, true);
}

void add_inventory(int inum) {
	if ((inum < 0) || (inum >= MAX_INV))
		quit("!AddInventory: invalid inventory number");

	Character_AddInventory(_G(playerchar), &_G(scrInv)[inum], SCR_NO_VALUE);

	_GP(play).inv_numorder = _GP(charextra)[_GP(game).playercharacter].invorder_count;
}

void lose_inventory(int inum) {
	if ((inum < 0) || (inum >= MAX_INV))
		quit("!LoseInventory: invalid inventory number");

	Character_LoseInventory(_G(playerchar), &_G(scrInv)[inum]);

	_GP(play).inv_numorder = _GP(charextra)[_GP(game).playercharacter].invorder_count;
}

void AddInventoryToCharacter(int charid, int inum) {
	if (!is_valid_character(charid))
		quit("!AddInventoryToCharacter: invalid character specified");
	if ((inum < 1) || (inum >= _GP(game).numinvitems))
		quit("!AddInventory: invalid inv item specified");

	Character_AddInventory(&_GP(game).chars[charid], &_G(scrInv)[inum], SCR_NO_VALUE);
}

void LoseInventoryFromCharacter(int charid, int inum) {
	if (!is_valid_character(charid))
		quit("!LoseInventoryFromCharacter: invalid character specified");
	if ((inum < 1) || (inum >= _GP(game).numinvitems))
		quit("!AddInventory: invalid inv item specified");

	Character_LoseInventory(&_GP(game).chars[charid], &_G(scrInv)[inum]);
}

void DisplayThought(int chid, const char *text) {
	if ((chid < 0) || (chid >= _GP(game).numcharacters))
		quit("!DisplayThought: invalid character specified");

	_DisplayThoughtCore(chid, text);
}

void __sc_displayspeech(int chid, const char *text) {
	if ((chid < 0) || (chid >= _GP(game).numcharacters))
		quit("!DisplaySpeech: invalid character specified");

	_DisplaySpeechCore(chid, text);
}

// **** THIS IS UNDOCUMENTED BECAUSE IT DOESN'T WORK PROPERLY
// **** AT 640x400 AND DOESN'T USE THE RIGHT SPEECH STYLE
void DisplaySpeechAt(int xx, int yy, int wii, int aschar, const char *spch) {
	data_to_game_coords(&xx, &yy);
	wii = data_to_game_coord(wii);
	_displayspeech(get_translation(spch), aschar, xx, yy, wii, 0);
}

int DisplaySpeechBackground(int charid, const char *speel) {
	// remove any previous background speech for this character
	// TODO: have a map character -> bg speech over?
	const auto &overs = get_overlays();
	for (size_t i = 0; i < overs.size(); ++i) {
		if (overs[i].bgSpeechForChar == charid) {
			remove_screen_overlay(i);
			break;
		}
	}

	int ovrl = CreateTextOverlay(OVR_AUTOPLACE, charid, _GP(play).GetUIViewport().GetWidth() / 2, FONT_SPEECH,
	                             -_GP(game).chars[charid].talkcolor, get_translation(speel), DISPLAYTEXT_NORMALOVERLAY);

	auto *over = get_overlay(ovrl);
	over->bgSpeechForChar = charid;
	over->timeout = GetTextDisplayTime(speel, 1);
	return ovrl;
}

} // namespace AGS3
