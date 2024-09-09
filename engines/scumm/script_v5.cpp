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

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v3.h"
#include "scumm/scumm_v5.h"
#include "scumm/sound.h"
#include "scumm/players/player_towns.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

#include "common/savefile.h"
#include "common/config-manager.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v5, x)

void ScummEngine_v5::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o5_stopObjectCode);
	OPCODE(0x01, o5_putActor);
	OPCODE(0x02, o5_startMusic);
	OPCODE(0x03, o5_getActorRoom);
	/* 04 */
	OPCODE(0x04, o5_isGreaterEqual);
	OPCODE(0x05, o5_drawObject);
	OPCODE(0x06, o5_getActorElevation);
	OPCODE(0x07, o5_setState);
	/* 08 */
	OPCODE(0x08, o5_isNotEqual);
	OPCODE(0x09, o5_faceActor);
	OPCODE(0x0a, o5_startScript);
	OPCODE(0x0b, o5_getVerbEntrypoint);
	/* 0C */
	OPCODE(0x0c, o5_resourceRoutines);
	OPCODE(0x0d, o5_walkActorToActor);
	OPCODE(0x0e, o5_putActorAtObject);
	OPCODE(0x0f, o5_getObjectState);
	/* 10 */
	OPCODE(0x10, o5_getObjectOwner);
	OPCODE(0x11, o5_animateActor);
	OPCODE(0x12, o5_panCameraTo);
	OPCODE(0x13, o5_actorOps);
	/* 14 */
	OPCODE(0x14, o5_print);
	OPCODE(0x15, o5_actorFromPos);
	OPCODE(0x16, o5_getRandomNr);
	OPCODE(0x17, o5_and);
	/* 18 */
	OPCODE(0x18, o5_jumpRelative);
	OPCODE(0x19, o5_doSentence);
	OPCODE(0x1a, o5_move);
	OPCODE(0x1b, o5_multiply);
	/* 1C */
	OPCODE(0x1c, o5_startSound);
	OPCODE(0x1d, o5_ifClassOfIs);
	OPCODE(0x1e, o5_walkActorTo);
	OPCODE(0x1f, o5_isActorInBox);
	/* 20 */
	OPCODE(0x20, o5_stopMusic);
	OPCODE(0x21, o5_putActor);
	OPCODE(0x22, o5_getAnimCounter);
	OPCODE(0x23, o5_getActorY);
	/* 24 */
	OPCODE(0x24, o5_loadRoomWithEgo);
	OPCODE(0x25, o5_pickupObject);
	OPCODE(0x26, o5_setVarRange);
	OPCODE(0x27, o5_stringOps);
	/* 28 */
	OPCODE(0x28, o5_equalZero);
	OPCODE(0x29, o5_setOwnerOf);
	OPCODE(0x2a, o5_startScript);
	OPCODE(0x2b, o5_delayVariable);
	/* 2C */
	OPCODE(0x2c, o5_cursorCommand);
	OPCODE(0x2d, o5_putActorInRoom);
	OPCODE(0x2e, o5_delay);
//	OPCODE(0x2f, o5_ifNotState);
	/* 30 */
	OPCODE(0x30, o5_matrixOps);
	OPCODE(0x31, o5_getInventoryCount);
	OPCODE(0x32, o5_setCameraAt);
	OPCODE(0x33, o5_roomOps);
	/* 34 */
	OPCODE(0x34, o5_getDist);
	OPCODE(0x35, o5_findObject);
	OPCODE(0x36, o5_walkActorToObject);
	OPCODE(0x37, o5_startObject);
	/* 38 */
	OPCODE(0x38, o5_isLessEqual);
	OPCODE(0x39, o5_doSentence);
	OPCODE(0x3a, o5_subtract);
	OPCODE(0x3b, o5_getActorScale);
	/* 3C */
	OPCODE(0x3c, o5_stopSound);
	OPCODE(0x3d, o5_findInventory);
	OPCODE(0x3e, o5_walkActorTo);
	OPCODE(0x3f, o5_drawBox);
	/* 40 */
	OPCODE(0x40, o5_cutscene);
	OPCODE(0x41, o5_putActor);
	OPCODE(0x42, o5_chainScript);
	OPCODE(0x43, o5_getActorX);
	/* 44 */
	OPCODE(0x44, o5_isLess);
//	OPCODE(0x45, o5_drawObject);
	OPCODE(0x46, o5_increment);
	OPCODE(0x47, o5_setState);
	/* 48 */
	OPCODE(0x48, o5_isEqual);
	OPCODE(0x49, o5_faceActor);
	OPCODE(0x4a, o5_startScript);
	OPCODE(0x4b, o5_getVerbEntrypoint);
	/* 4C */
	OPCODE(0x4c, o5_soundKludge);
	OPCODE(0x4d, o5_walkActorToActor);
	OPCODE(0x4e, o5_putActorAtObject);
//	OPCODE(0x4f, o5_ifState);
	/* 50 */
//	OPCODE(0x50, o5_pickupObjectOld);
	OPCODE(0x51, o5_animateActor);
	OPCODE(0x52, o5_actorFollowCamera);
	OPCODE(0x53, o5_actorOps);
	/* 54 */
	OPCODE(0x54, o5_setObjectName);
	OPCODE(0x55, o5_actorFromPos);
	OPCODE(0x56, o5_getActorMoving);
	OPCODE(0x57, o5_or);
	/* 58 */
	OPCODE(0x58, o5_beginOverride);
	OPCODE(0x59, o5_doSentence);
	OPCODE(0x5a, o5_add);
	OPCODE(0x5b, o5_divide);
	/* 5C */
//	OPCODE(0x5c, o5_oldRoomEffect);
	OPCODE(0x5d, o5_setClass);
	OPCODE(0x5e, o5_walkActorTo);
	OPCODE(0x5f, o5_isActorInBox);
	/* 60 */
	OPCODE(0x60, o5_freezeScripts);
	OPCODE(0x61, o5_putActor);
	OPCODE(0x62, o5_stopScript);
	OPCODE(0x63, o5_getActorFacing);
	/* 64 */
	OPCODE(0x64, o5_loadRoomWithEgo);
	OPCODE(0x65, o5_pickupObject);
	OPCODE(0x66, o5_getClosestObjActor);
	OPCODE(0x67, o5_getStringWidth);
	/* 68 */
	OPCODE(0x68, o5_isScriptRunning);
	OPCODE(0x69, o5_setOwnerOf);
	OPCODE(0x6a, o5_startScript);
	OPCODE(0x6b, o5_debug);
	/* 6C */
	OPCODE(0x6c, o5_getActorWidth);
	OPCODE(0x6d, o5_putActorInRoom);
	OPCODE(0x6e, o5_stopObjectScript);
//	OPCODE(0x6f, o5_ifNotState);
	/* 70 */
	OPCODE(0x70, o5_lights);
	OPCODE(0x71, o5_getActorCostume);
	OPCODE(0x72, o5_loadRoom);
	OPCODE(0x73, o5_roomOps);
	/* 74 */
	OPCODE(0x74, o5_getDist);
	OPCODE(0x75, o5_findObject);
	OPCODE(0x76, o5_walkActorToObject);
	OPCODE(0x77, o5_startObject);
	/* 78 */
	OPCODE(0x78, o5_isGreater);
	OPCODE(0x79, o5_doSentence);
	OPCODE(0x7a, o5_verbOps);
	OPCODE(0x7b, o5_getActorWalkBox);
	/* 7C */
	OPCODE(0x7c, o5_isSoundRunning);
	OPCODE(0x7d, o5_findInventory);
	OPCODE(0x7e, o5_walkActorTo);
	OPCODE(0x7f, o5_drawBox);
	/* 80 */
	OPCODE(0x80, o5_breakHere);
	OPCODE(0x81, o5_putActor);
	OPCODE(0x82, o5_startMusic);
	OPCODE(0x83, o5_getActorRoom);
	/* 84 */
	OPCODE(0x84, o5_isGreaterEqual);
	OPCODE(0x85, o5_drawObject);
	OPCODE(0x86, o5_getActorElevation);
	OPCODE(0x87, o5_setState);
	/* 88 */
	OPCODE(0x88, o5_isNotEqual);
	OPCODE(0x89, o5_faceActor);
	OPCODE(0x8a, o5_startScript);
	OPCODE(0x8b, o5_getVerbEntrypoint);
	/* 8C */
	OPCODE(0x8c, o5_resourceRoutines);
	OPCODE(0x8d, o5_walkActorToActor);
	OPCODE(0x8e, o5_putActorAtObject);
	OPCODE(0x8f, o5_getObjectState);
	/* 90 */
	OPCODE(0x90, o5_getObjectOwner);
	OPCODE(0x91, o5_animateActor);
	OPCODE(0x92, o5_panCameraTo);
	OPCODE(0x93, o5_actorOps);
	/* 94 */
	OPCODE(0x94, o5_print);
	OPCODE(0x95, o5_actorFromPos);
	OPCODE(0x96, o5_getRandomNr);
	OPCODE(0x97, o5_and);
	/* 98 */
	OPCODE(0x98, o5_systemOps);
	OPCODE(0x99, o5_doSentence);
	OPCODE(0x9a, o5_move);
	OPCODE(0x9b, o5_multiply);
	/* 9C */
	OPCODE(0x9c, o5_startSound);
	OPCODE(0x9d, o5_ifClassOfIs);
	OPCODE(0x9e, o5_walkActorTo);
	OPCODE(0x9f, o5_isActorInBox);
	/* A0 */
	OPCODE(0xa0, o5_stopObjectCode);
	OPCODE(0xa1, o5_putActor);
	OPCODE(0xa2, o5_getAnimCounter);
	OPCODE(0xa3, o5_getActorY);
	/* A4 */
	OPCODE(0xa4, o5_loadRoomWithEgo);
	OPCODE(0xa5, o5_pickupObject);
	OPCODE(0xa6, o5_setVarRange);
	OPCODE(0xa7, o5_dummy);
	/* A8 */
	OPCODE(0xa8, o5_notEqualZero);
	OPCODE(0xa9, o5_setOwnerOf);
	OPCODE(0xaa, o5_startScript);
	OPCODE(0xab, o5_saveRestoreVerbs);
	/* AC */
	OPCODE(0xac, o5_expression);
	OPCODE(0xad, o5_putActorInRoom);
	OPCODE(0xae, o5_wait);
//	OPCODE(0xaf, o5_ifNotState);
	/* B0 */
	OPCODE(0xb0, o5_matrixOps);
	OPCODE(0xb1, o5_getInventoryCount);
	OPCODE(0xb2, o5_setCameraAt);
	OPCODE(0xb3, o5_roomOps);
	/* B4 */
	OPCODE(0xb4, o5_getDist);
	OPCODE(0xb5, o5_findObject);
	OPCODE(0xb6, o5_walkActorToObject);
	OPCODE(0xb7, o5_startObject);
	/* B8 */
	OPCODE(0xb8, o5_isLessEqual);
	OPCODE(0xb9, o5_doSentence);
	OPCODE(0xba, o5_subtract);
	OPCODE(0xbb, o5_getActorScale);
	/* BC */
	OPCODE(0xbc, o5_stopSound);
	OPCODE(0xbd, o5_findInventory);
	OPCODE(0xbe, o5_walkActorTo);
	OPCODE(0xbf, o5_drawBox);
	/* C0 */
	OPCODE(0xc0, o5_endCutscene);
	OPCODE(0xc1, o5_putActor);
	OPCODE(0xc2, o5_chainScript);
	OPCODE(0xc3, o5_getActorX);
	/* C4 */
	OPCODE(0xc4, o5_isLess);
//	OPCODE(0xc5, o5_drawObject);
	OPCODE(0xc6, o5_decrement);
	OPCODE(0xc7, o5_setState);
	/* C8 */
	OPCODE(0xc8, o5_isEqual);
	OPCODE(0xc9, o5_faceActor);
	OPCODE(0xca, o5_startScript);
	OPCODE(0xcb, o5_getVerbEntrypoint);
	/* CC */
	OPCODE(0xcc, o5_pseudoRoom);
	OPCODE(0xcd, o5_walkActorToActor);
	OPCODE(0xce, o5_putActorAtObject);
//	OPCODE(0xcf, o5_ifState);
	/* D0 */
//	OPCODE(0xd0, o5_pickupObjectOld);
	OPCODE(0xd1, o5_animateActor);
	OPCODE(0xd2, o5_actorFollowCamera);
	OPCODE(0xd3, o5_actorOps);
	/* D4 */
	OPCODE(0xd4, o5_setObjectName);
	OPCODE(0xd5, o5_actorFromPos);
	OPCODE(0xd6, o5_getActorMoving);
	OPCODE(0xd7, o5_or);
	/* D8 */
	OPCODE(0xd8, o5_printEgo);
	OPCODE(0xd9, o5_doSentence);
	OPCODE(0xda, o5_add);
	OPCODE(0xdb, o5_divide);
	/* DC */
//	OPCODE(0xdc, o5_oldRoomEffect);
	OPCODE(0xdd, o5_setClass);
	OPCODE(0xde, o5_walkActorTo);
	OPCODE(0xdf, o5_isActorInBox);
	/* E0 */
	OPCODE(0xe0, o5_freezeScripts);
	OPCODE(0xe1, o5_putActor);
	OPCODE(0xe2, o5_stopScript);
	OPCODE(0xe3, o5_getActorFacing);
	/* E4 */
	OPCODE(0xe4, o5_loadRoomWithEgo);
	OPCODE(0xe5, o5_pickupObject);
	OPCODE(0xe6, o5_getClosestObjActor);
	OPCODE(0xe7, o5_getStringWidth);
	/* E8 */
	OPCODE(0xe8, o5_isScriptRunning);
	OPCODE(0xe9, o5_setOwnerOf);
	OPCODE(0xea, o5_startScript);
	OPCODE(0xeb, o5_debug);
	/* EC */
	OPCODE(0xec, o5_getActorWidth);
	OPCODE(0xed, o5_putActorInRoom);
	OPCODE(0xee, o5_stopObjectScript);
//	OPCODE(0xef, o5_ifNotState);
	/* F0 */
	OPCODE(0xf0, o5_lights);
	OPCODE(0xf1, o5_getActorCostume);
	OPCODE(0xf2, o5_loadRoom);
	OPCODE(0xf3, o5_roomOps);
	/* F4 */
	OPCODE(0xf4, o5_getDist);
	OPCODE(0xf5, o5_findObject);
	OPCODE(0xf6, o5_walkActorToObject);
	OPCODE(0xf7, o5_startObject);
	/* F8 */
	OPCODE(0xf8, o5_isGreater);
	OPCODE(0xf9, o5_doSentence);
	OPCODE(0xfa, o5_verbOps);
	OPCODE(0xfb, o5_getActorWalkBox);
	/* FC */
	OPCODE(0xfc, o5_isSoundRunning);
	OPCODE(0xfd, o5_findInventory);
	OPCODE(0xfe, o5_walkActorTo);
	OPCODE(0xff, o5_drawBox);
}

int ScummEngine_v5::getVar() {
	return readVar(fetchScriptWord());
}

int ScummEngine_v5::getVarOrDirectByte(byte mask) {
	if (_opcode & mask)
		return getVar();
	return fetchScriptByte();
}

int ScummEngine_v5::getVarOrDirectWord(byte mask) {
	if (_opcode & mask)
		return getVar();
	return fetchScriptWordSigned();
}

void ScummEngine_v5::getResultPos() {
	int a;

	_resultVarNumber = fetchScriptWord();
	if (_resultVarNumber & 0x2000) {
		a = fetchScriptWord();
		if (a & 0x2000) {
			_resultVarNumber += readVar(a & ~0x2000);
		} else {
			_resultVarNumber += a & 0xFFF;
		}
		_resultVarNumber &= ~0x2000;
	}
}

void ScummEngine_v5::setResult(int value) {
	writeVar(_resultVarNumber, value);
}

void ScummEngine_v5::jumpRelative(bool cond) {
	// We explicitly call ScummEngine::fetchScriptWord()
	// to make this method work also in v0, which overloads
	// fetchScriptWord to only read bytes (which is the right thing
	// to do for most opcodes, but not for jump offsets).
	int16 offset = ScummEngine::fetchScriptWord();
	if (!cond)
		_scriptPointer += offset;
}

void ScummEngine_v5::o5_actorFollowCamera() {
	actorFollowCamera(getVarOrDirectByte(0x80));
}

void ScummEngine_v5::o5_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectWord(PARAM_1);
	y = getVarOrDirectWord(PARAM_2);
	setResult(getActorFromPos(x, y));
}

void ScummEngine_v5::o5_actorOps() {
	static const byte convertTable[20] =
		{ 1, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	// WORKAROUND bug #2233 "MI2 FM-TOWNS: Elaine's mappiece directly flies to treehouse"
	// There's extra code inserted in script 45 from room 45 that caused that behaviour,
	// the code below just skips the extra script code.  As confirmed by Aric Wilmunder,
	// "the fishing pole puzzle had been removed for the Towns because vertical scrolling
	// hadn't been implemented", but it appears to work nonetheless, which is what they
	// also observed when doing the QA for the PC version.
	if (_game.id == GID_MONKEY2 && _game.platform == Common::kPlatformFMTowns &&
		vm.slot[_currentScript].number == 45 && _currentRoom == 45 &&
		(_scriptPointer - _scriptOrgPointer == 0xA9) && enhancementEnabled(kEnhRestoredContent)) {
		_scriptPointer += 0xCF - 0xA1;
		writeVar(32811, 0); // clear bit 43
		return;
	}
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_actorOps");
	int i, j;

	// WORKAROUND: There's a continuity error in Monkey 1, in that the Jolly Roger should
	// only appear in the first scene showing the Sea Monkey in the middle of the sea,
	// since Guybrush must have picked it for the two other ship cutscenes to happen.
	//
	// Some official releases appear to have a fix for this (e.g. the English floppy VGA
	// version), but most releases don't. The fixed release would check whether the
	// script describing that "the crew begins to plan their voyage" is running in order
	// to display the flag, so we just reuse this check. The Ultimate Talkie also fixed
	// this, but in a different way which doesn't look as portable between releases.
	if ((_game.id == GID_MONKEY_EGA || _game.id == GID_MONKEY_VGA || (_game.id == GID_MONKEY && !(_game.features & GF_ULTIMATE_TALKIE))) &&
		_roomResource == 87 && vm.slot[_currentScript].number == 10002 && act == 9 &&
		enhancementEnabled(kEnhVisualChanges)) {
		const int scriptNr = (_game.version == 5) ? 122 : 119;
		if (!isScriptRunning(scriptNr)) {
			a->putActor(0);
			stopObjectCode();
			return;
		}
	}

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		if (_game.features & GF_SMALL_HEADER)
			_opcode = (_opcode & 0xE0) | convertTable[(_opcode & 0x1F) - 1];

		switch (_opcode & 0x1F) {
		case 0:										/* dummy case */
			getVarOrDirectByte(PARAM_1);
			break;
		case 1:			// SO_COSTUME
			i = getVarOrDirectByte(PARAM_1);

			// WORKAROUND: In the VGA floppy version of Monkey
			// Island 1, there are two different costumes for the
			// captain Smirk close-up: 0 for when the game is run
			// from floppies, and 76 for when the game is run from
			// hard disk, I believe.
			//
			// Costume 0 doesn't have any cigar smoke, perhaps to
			// cut down on disk access.
			//
			// But in the VGA CD version, only costume 0 is used
			// and the close-up is missing the cigar smoke.

			if (_game.id == GID_MONKEY && _currentRoom == 76 && act == 12 && i == 0 && enhancementEnabled(kEnhVisualChanges)) {
				i = 76;
			}

			a->setActorCostume(i);
			break;
		case 2:			// SO_STEP_DIST
			i = getVarOrDirectByte(PARAM_1);
			j = getVarOrDirectByte(PARAM_2);
			a->setActorWalkSpeed(i, j);
			break;
		case 3:			// SO_SOUND
			a->_sound[0] = getVarOrDirectByte(PARAM_1);
			break;
		case 4:			// SO_WALK_ANIMATION
			a->_walkFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 5:			// SO_TALK_ANIMATION
			a->_talkStartFrame = getVarOrDirectByte(PARAM_1);
			a->_talkStopFrame = getVarOrDirectByte(PARAM_2);
			break;
		case 6:			// SO_STAND_ANIMATION
			a->_standFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 7:			// SO_ANIMATION
			getVarOrDirectByte(PARAM_1);
			getVarOrDirectByte(PARAM_2);
			getVarOrDirectByte(PARAM_3);
			break;
		case 8:			// SO_DEFAULT
			a->initActor(0);
			break;
		case 9:			// SO_ELEVATION
			a->setElevation(getVarOrDirectWord(PARAM_1));
			break;
		case 10:		// SO_ANIMATION_DEFAULT
			a->_initFrame = 1;
			a->_walkFrame = 2;
			a->_standFrame = 3;
			a->_talkStartFrame = 4;
			a->_talkStopFrame = 5;
			break;
		case 11:		// SO_PALETTE
			i = getVarOrDirectByte(PARAM_1);
			j = getVarOrDirectByte(PARAM_2);
			assertRange(0, i, 31, "o5_actorOps: palette slot");

			// WORKAROUND: In the corridors of Castle Brunwald,
			// there is a 'continuity error' with the Nazi guards
			// in the FM-TOWNS version. They still have their
			// palette override from the EGA version, making them
			// appear in gray there, although their uniforms are
			// green when you fight them or meet them again in
			// the zeppelin. The PC VGA version fixed this.

			if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns &&
				(a->_costume == 23 || a->_costume == 28 || a->_costume == 29) &&
				(_currentRoom == 20 || _currentRoom == 28 || _currentRoom == 32) && enhancementEnabled(kEnhVisualChanges)) {
				break;
			}

			// WORKAROUND: The smoke animation is the same as
			// what's used for the voodoo lady's cauldron. But
			// for some reason, the colors changed between the
			// VGA floppy and CD versions. So when it tries to
			// remap the colors, it uses the wrong indexes. The
			// CD animation uses colors 1-3, where the floppy
			// version uses 2, 3, and 9.
			//
			// We don't touch the colours in general - the Special
			// edition have pretty much made them canon anyway -
			// but for the Smirk close-up we want the same colors
			// as the floppy version.

			if (_game.id == GID_MONKEY && _currentRoom == 76 && enhancementEnabled(kEnhVisualChanges)) {
				if (i == 3)
					i = 1;
				else if (i == 9)
					i = 3;
			}

			// WORKAROUND for original bug. The original interpreter has a color fix for CGA mode which can be seen
			// in Actor::setActorCostume(). Sometimes (e. g. when Bobbin walks out of the darkened tent) the actor
			// colors are changed via script without taking into account the need to repeat the color fix.
			if (_game.id == GID_LOOM && _renderMode == Common::kRenderCGA && act == 1) {
				if (i == 6 && j == 6)
					j = 5;
				else if (i == 7 && j == 7)
					j = 15;
				else if (i == 8 && j == 8)
					j = 0;
			}

			// Setting palette color 0 to 0 appears to be a way to
			// reset the actor palette in the TurboGrafx-16 version
			// of Loom. It's used in several places, but the only
			// one where I can see any visible difference is when
			// leaving the darkened tent.

			if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && i == 0 && j == 0) {
				for (int k = 0; k < 32; k++)
					a->setPalette(k, 0xFF);
			} else {
				a->setPalette(i, j);
			}
			break;
		case 12:		// SO_TALK_COLOR
			a->_talkColor = getVarOrDirectByte(PARAM_1);
			break;
		case 13:		// SO_ACTOR_NAME
			loadPtrToResource(rtActorName, a->_number, nullptr);
			break;
		case 14:		// SO_INIT_ANIMATION
			a->_initFrame = getVarOrDirectByte(PARAM_1);
			break;
		case 16:		// SO_ACTOR_WIDTH
			a->_width = getVarOrDirectByte(PARAM_1);
			break;
		case 17:		// SO_ACTOR_SCALE
			if (_game.version == 4) {
				i = j = getVarOrDirectByte(PARAM_1);
			} else {
				i = getVarOrDirectByte(PARAM_1);
				j = getVarOrDirectByte(PARAM_2);
			}

			a->_boxscale = i;
			a->setScale(i, j);
			break;
		case 18:		// SO_NEVER_ZCLIP
			a->_forceClip = 0;
			break;
		case 19:		// SO_ALWAYS_ZCLIP
			a->_forceClip = getVarOrDirectByte(PARAM_1);
			break;
		case 20:		// SO_IGNORE_BOXES
		case 21:		// SO_FOLLOW_BOXES
			a->_ignoreBoxes = !(_opcode & 1);
			a->_forceClip = 0;
			if (a->isInCurrentRoom())
				a->putActor();
			break;

		case 22:		// SO_ANIMATION_SPEED
			a->setAnimSpeed(getVarOrDirectByte(PARAM_1));
			break;
		case 23:		// SO_SHADOW
			a->_shadowMode = getVarOrDirectByte(PARAM_1);
			break;
		default:
			error("o5_actorOps: default case %d", _opcode & 0x1F);
		}
	}
}

void ScummEngine_v5::o5_setClass() {
	int obj = getVarOrDirectWord(PARAM_1);
	int cls;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(PARAM_1);

		// WORKAROUND: In the CD versions of Monkey 1 with the full 256-color
		// inventory, going at Stan's messes up the color of some objects, such
		// as the "striking yellow color" of the flower from the forest, the
		// rubber chicken, or Guybrush's trousers. The following palette fixes
		// are taken from the Ultimate Talkie Edition.
		if (_game.id == GID_MONKEY && _game.platform != Common::kPlatformFMTowns &&
		    _game.platform != Common::kPlatformSegaCD && _roomResource == 59 &&
			_currentScript != 0xFF && vm.slot [_currentScript].number == 10002 &&
			obj == 915 && cls == 6 && _currentPalette[251 * 3] == 0 &&
			enhancementEnabled(kEnhVisualChanges) && !(_game.features & GF_ULTIMATE_TALKIE)) {
			// True as long as Guybrush isn't done with the voodoo recipe on the
			// Sea Monkey. The Ultimate Talkie Edition probably does this as a way
			// to limit this palette override to Part One; just copy this behavior.
			if (_scummVars[260] < 8) {
				setPalColor(245,  68,  68, 68); // gray
				setPalColor(247, 252, 244,  0); // yellow
				setPalColor(249, 112, 212,  0); // lime
			}
			setPalColor(251, 32, 84, 0); // green
		}

		// WORKAROUND bug #3099: Due to a script bug, the wrong opcode is
		// used to test and set the state of various objects (e.g. the inside
		// door (object 465) of the of the Hostel on Mars), when opening the
		// Hostel door from the outside.
		if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns &&
		    vm.slot[_currentScript].number == 205 && _currentRoom == 185 &&
		    (cls == 0 || cls == 1)) {
			putState(obj, cls);
		} else if (cls == 0) {
			// Class '0' means: clean all class data
			_classData[obj] = 0;
			if ((_game.features & GF_SMALL_HEADER) && objIsActor(obj)) {
				Actor *a = derefActor(obj, "o5_setClass");
				a->_ignoreBoxes = false;
				a->_forceClip = 0;
			}
		} else
			putClass(obj, cls, (cls & 0x80) ? true : false);
	}
}

void ScummEngine_v5::o5_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);

	// WORKAROUND bug #994: This works around a script bug in LoomCD. To
	// understand the reasoning behind this, compare script 210 and 218 in
	// room 20. Apparently they made a mistake when converting the absolute
	// delays into relative ones.
	if (_game.id == GID_LOOM && _game.version == 4 && vm.slot[_currentScript].number == 210 && _currentRoom == 20 && _resultVarNumber == 0x4000) {
		switch (a) {
		// Fix for the Var[250] == 11 case
		case 138:
			a = 145;
			break;
		case 324:
			a = 324 - 138;
			break;
		// Fixes for the Var[250] == 14 case
		case 130:
			a = 170;
			break;
		case 342:
			a = 342 - 130 + 15;	// Small extra adjustment for the "OUCH"
			break;
		case 384:
			a -= 342;
			break;
		case 564:
			a -= 384;
			break;
		default:
			break;
		}
	}

	// WORKAROUND: The clock tower is controlled by two variables: 163 and
	// 247 in the floppy VGA version, 164 and 248 in the CD version. I
	// don't know about the EGA version, but this fix only concerns the
	// CD version.
	//
	// Whenever you enter the room, the first variable is cleared. It is
	// then set if you examine the clock tower. The second variable
	// determines which description you see, e.g. "Ten o'clock.", "Hmm.
	// Still ten o'clock.", etc.
	//
	// If the first variable was set, the second is incremented when you
	// leave the room. That means that every time you examine the clock
	// tower, you get a new description (there are three of them, with a
	// random variation on the last one) but only if you've been away from
	// the room in between.
	//
	// But in the CD version, someone has attempted to "fix" this behavior
	// by always incrementing the second variable when the clock tower is
	// examined. So you don't have to leave the room in between, and if
	// you examine the clock tower once and then leave, the second variable
	// is incremented twice so you'll never see the second description.
	//
	// We restore the old behavior by adding 0, not 1, to the second
	// variable when examining the clock tower.

	if (_game.id == GID_MONKEY && vm.slot[_currentScript].number == 210 && _currentRoom == 35 && _resultVarNumber == 248 && a == 1 && enhancementEnabled(kEnhRestoredContent)) {
		a = 0;
	}

	setResult(readVar(_resultVarNumber) + a);
}

void ScummEngine_v5::o5_and() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) & a);
}

void ScummEngine_v5::o5_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);

	// WORKAROUND bug #1265: This script calls animateCostume(86,255) and
	// animateCostume(31,255), with 86 and 31 being script numbers used as
	// (way out of range) actor numbers. This seems to be yet another script
	// bug which the original engine let slip by.
	// For more information about why this happens, see o5_getActorRoom().
	if (!isValidActor(act)) {
		return;
	}

	// WORKAROUND bug #1339: While on Mars, going outside without your helmet
	// (or missing some other part of your "space suite" will cause your
	// character to complain ("I can't breathe."). Unfortunately, this is
	// coupled with an animate command, making it very difficult to return to
	// safety (from where you came). The following hack works around this by
	// ignoring that particular turn command.
	if (_game.id == GID_ZAK && _currentRoom == 182 && anim == 246 &&
			((_game.version < 3 && vm.slot[_currentScript].number == 82)
			|| (_game.version == 3 && vm.slot[_currentScript].number == 131))) {
		return;
	}

	Actor *a = derefActor(act, "o5_animateActor");
	a->animateActor(anim);
}

void ScummEngine_v5::o5_breakHere() {
	// WORKAROUND: The English PC Engine version of Loom shows a Turbo
	// Technologies loading screen. In the Mednafen emulator it's shown for
	// about 10 seconds while the game is loading resources. ScummVM does
	// that in the blink of an eye.
	//
	// Injecting the delay into the breakHere instruction seems like the
	// least intrusive way of adding the delay. The script calls it a number
	// of times, but only once from room 69.

	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && _language == Common::EN_ANY && _currentScript != 0xFF && vm.slot [_currentScript].number == 44 && _currentRoom == 69) {
		vm.slot[_currentScript].delay = 120;
		vm.slot[_currentScript].status = ssPaused;
	}

	updateScriptPtr();
	_currentScript = 0xFF;
}

void ScummEngine_v5::o5_chainScript() {
	int vars[NUM_SCRIPT_LOCAL];
	int script;
	int cur;

	script = getVarOrDirectByte(PARAM_1);

	getWordVararg(vars);

	cur = _currentScript;

	// WORKAROUND bug #812: Work around a bug in script 33 in Indy3.
	// That script is used for the fist fights in the Zeppelin. It uses
	// Local[5], even though that is never set to any value. But script 33 is
	// called via chainScript by script 32, and in there Local[5] is set to
	// the actor ID of the opposing soldier. So, we copy that value over to
	// the Local[5] variable of script 33.
	// FIXME: This workaround is meant for Indy3 EGA/VGA, but we make no
	// checks to exclude the Mac/FM-TOWNS versions. We need to check whether
	// those need the same workaround; if they don't, or if they need it in
	// modified form, adjust this workaround accordingly.
	// FIXME: Do we still need this workaround, 19 years later? I can't
	// reproduce the original crash anymore, maybe we handle uninitialized
	// local values the same way the original interpreter did, now?
	if (_game.id == GID_INDY3 && cur != 0xFF && vm.slot[cur].number == 32 && script == 33) {
		vars[5] = vm.localvar[cur][5];
	}

	assert(cur != 0xFF);
	vm.slot[cur].number = 0;
	vm.slot[cur].status = ssDead;
	_currentScript = 0xFF;

	runScript(script, vm.slot[cur].freezeResistant, vm.slot[cur].recursive, vars);
}

void ScummEngine_v5::o5_cursorCommand() {
	int i, j, k;
	int table[32];
	memset(table, 0, sizeof(table));
	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:			// SO_CURSOR_ON
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 2:			// SO_CURSOR_OFF
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 3:			// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 4:			// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 5:			// SO_CURSOR_SOFT_ON
		_cursor.state++;
		verbMouseOver(0);
		break;
	case 6:			// SO_CURSOR_SOFT_OFF
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 7:			// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 8:			// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 10:		// SO_CURSOR_IMAGE
		i = getVarOrDirectByte(PARAM_1);	// Cursor number
		j = getVarOrDirectByte(PARAM_2);	// Charset letter to use
		redefineBuiltinCursorFromChar(i, j);
		break;
	case 11:		// SO_CURSOR_HOTSPOT
		i = getVarOrDirectByte(PARAM_1);
		j = getVarOrDirectByte(PARAM_2);
		k = getVarOrDirectByte(PARAM_3);
		redefineBuiltinCursorHotspot(i, j, k);
		break;
	case 12:		// SO_CURSOR_SET
		i = getVarOrDirectByte(PARAM_1);
		if (i >= 0 && i <= 3)
			_currentCursor = i;
		else
			error("SO_CURSOR_SET: unsupported cursor id %d", i);
		break;
	case 13:		// SO_CHARSET_SET
		initCharset(getVarOrDirectByte(PARAM_1));
		break;
	case 14:											/* unk */
		if (_game.version == 3) {
			/*int a = */ getVarOrDirectByte(PARAM_1);
			/*int b = */ getVarOrDirectByte(PARAM_2);
			// This is some kind of "init charset" opcode. However, we don't have to do anything
			// in here, as our initCharset automatically calls loadCharset for GF_SMALL_HEADER,
			// games if needed.
		} else {
			getWordVararg(table);
			// WORKAROUND bug #13735 - "Inaccurate verb rendering in Monkey 1 FM-TOWNS"
			// MI1 FM-Towns has a bug in the original interpreter which removes the shadow color from the verbs.
			// getWordVararg() will generate a WORD table, but then - right here - it is accessed like a DWORD
			// table. This is actually fixed in the original interpreters for MI2 and INDY4. It could be argued
			// if we even want that "fixed", but it does lead to bug tickets in Monkey 1 FM-TOWNS") and the
			// "fix" restores the original appearance (which - as per usual - is a matter of personal taste...).
			// So let people make their own choice with the Enhancement setting.
			int m = (_game.platform == Common::kPlatformFMTowns && _game.id == GID_MONKEY && !enhancementEnabled(kEnhVisualChanges)) ? 2 : 1;
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)table[i * m];
		}
		break;
	default:
		break;
	}

	if (_game.version >= 4) {
		VAR(VAR_CURSORSTATE) = _cursor.state;
		VAR(VAR_USERPUT) = _userPut;
	}
}

void ScummEngine_v5::o5_cutscene() {
	int args[NUM_SCRIPT_LOCAL];
	getWordVararg(args);

	// WORKAROUND: In Indy 3, the cutscene where Indy and his father escape
	// from the zeppelin with the biplane is missing the `[1]` parameter
	// which disables the verb interface. For some reason, this only causes
	// a problem on the FM-TOWNS version, though... also happens under UNZ.
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns && _currentRoom == 80 && _currentScript != 0xFF && vm.slot[_currentScript].number == 201 && args[0] == 0 && enhancementEnabled(kEnhVisualChanges)) {
		args[0] = 1;
	}

	beginCutscene(args);
}

void ScummEngine_v5::o5_endCutscene() {
	endCutscene();
}

void ScummEngine_v5::o5_debug() {
	int a = getVarOrDirectWord(PARAM_1);
	debugC(DEBUG_GENERAL, "o5_debug(%d)", a);
}

void ScummEngine_v5::o5_decrement() {
	getResultPos();
	setResult(readVar(_resultVarNumber) - 1);
}

void ScummEngine_v5::o5_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	assert(_currentScript != 0xFF);
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o5_breakHere();
}

void ScummEngine_v5::o5_delayVariable() {
	assert(_currentScript != 0xFF);
	vm.slot[_currentScript].delay = getVar();
	vm.slot[_currentScript].status = ssPaused;
	o5_breakHere();
}

void ScummEngine_v5::o5_divide() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	if (a == 0) {
		error("Divide by zero");
		setResult(0);
	} else
		setResult(readVar(_resultVarNumber) / a);
}

void ScummEngine_v5::o5_doSentence() {
	int verb;

	verb = getVarOrDirectByte(PARAM_1);
	if (verb == 0xFE) {
		_sentenceNum = 0;
		stopScript(VAR(VAR_SENTENCE_SCRIPT));
		clearClickedStatus();
		return;
	}

	int objectA = getVarOrDirectWord(PARAM_2);
	int objectB = getVarOrDirectWord(PARAM_3);

	doSentence(verb, objectA, objectB);
}

void ScummEngine_v5::o5_drawBox() {
	int x, y, x2, y2, color;

	x = getVarOrDirectWord(PARAM_1);
	y = getVarOrDirectWord(PARAM_2);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(PARAM_1);
	y2 = getVarOrDirectWord(PARAM_2);
	color = getVarOrDirectByte(PARAM_3);

	drawBox(x, y, x2, y2, color);
}

void ScummEngine_v5::o5_drawObject() {
	int state, obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	state = 1;
	xpos = ypos = 255;
	obj = getVarOrDirectWord(PARAM_1);

	if (_game.features & GF_SMALL_HEADER) {
		xpos = getVarOrDirectWord(PARAM_2);
		ypos = getVarOrDirectWord(PARAM_3);
	} else {
		_opcode = fetchScriptByte();
		switch (_opcode & 0x1F) {
		case 1:										/* draw at */
			xpos = getVarOrDirectWord(PARAM_1);
			ypos = getVarOrDirectWord(PARAM_2);
			break;
		case 2:										/* set state */
			state = getVarOrDirectWord(PARAM_1);
			break;
		case 0x1F:									/* neither */
			break;
		default:
			error("o5_drawObject: unknown subopcode %d", _opcode & 0x1F);
		}
	}

	// WORKAROUND: Captain Dread's head will glitch if you have already talked to him,
	// give him an object and then immediately talk to him again ("It's me again.").
	// This is because the original script forgot to check Bit[129] (= already facing
	// Guybrush) in that particular case, and so Dread would always try to turn and
	// face Guybrush even if he's already looking at him.  drawObject() should never
	// be called if Bit[129] is set in that script, so if it does happen, it means
	// the check was missing, and so we ignore the next 32 bytes of Dread's reaction.
	if (_game.id == GID_MONKEY2 && !(_game.features & GF_ULTIMATE_TALKIE) && _currentRoom == 22 && _currentScript != 0xFF && vm.slot [_currentScript].number == 201 && obj == 237 &&
		state == 1 && readVar(0x8000 + 129) == 1 && enhancementEnabled(kEnhMinorBugFixes)) {
		_scriptPointer += 32;
		return;
	}

	// WORKAROUND: In Indy3, the first close-up frame of Indy's reaction after drinking
	// from the Grail is never shown; it always starts at the second step, with Indy
	// already appearing a bit older. This is a bit unfortunate, especially if you
	// picked up the real Grail. This was probably done as a way to unconditionally
	// reset the animation if it's already been played, but we can just do an
	// unconditional reset of all previous frames instead, restoring the first one.
	if (_game.id == GID_INDY3 && _roomResource == 87 && _currentScript != 0xFF && vm.slot [_currentScript].number == 200 && obj == 899 && state == 1 && VAR(VAR_TIMER_NEXT) != 12 && enhancementEnabled(kEnhRestoredContent)) {
		i = _numLocalObjects - 1;
		do {
			if (_objs[i].obj_nr)
				putState(_objs[i].obj_nr, 0);
		} while (--i);
		return;
	}

	// WORKAROUND: In some of the earliest 16-color releases of Loom, the
	// staircase at the right of room 32 will glitch if Bobbin uses it to exit
	// the room, if he entered it via the other stairs in the ground. This has
	// been officially fixed in some '1.2' releases (e.g. French DOS/EGA) and
	// all later versions; this smaller workaround appears to be enough.
	if (_game.id == GID_LOOM && _game.version == 3 && !(_game.features & GF_OLD256) && _roomResource == 32 &&
		vm.slot[_currentScript].number == 10002 && obj == 540 && state == 1 && xpos == 255 && ypos == 255 &&
		enhancementEnabled(kEnhMinorBugFixes)) {
		if (getState(541) == 1) {
			putState(obj, state);
			obj = 541;
			state = 0;
		}
	}

	idx = getObjectIndex(obj);
	if (idx == -1)
		return;

	od = &_objs[idx];
	if (xpos != 0xFF) {
		od->walk_x += (xpos * 8) - od->x_pos;
		od->x_pos = xpos * 8;
		od->walk_y += (ypos * 8) - od->y_pos;
		od->y_pos = ypos * 8;
	}
	addObjectToDrawQue(idx);

	x = od->x_pos;
	y = od->y_pos;
	w = od->width;
	h = od->height;

	i = _numLocalObjects - 1;
	do {
		if (_objs[i].obj_nr && _objs[i].x_pos == x && _objs[i].y_pos == y && _objs[i].width == w && _objs[i].height == h)
			putState(_objs[i].obj_nr, 0);
	} while (--i);

	putState(obj, state);
}

void ScummEngine_v5::o5_dummy() {
	// The KIXX XL release of Monkey Island 2 (Amiga disk) used opcode 0xa7
	// as dummy, in order to remove copy protection and keep level selection.
	if (_opcode != 0xa7 || _game.id == GID_MONKEY2)
		warning("o5_dummy invoked (opcode %d)", _opcode);
}

void ScummEngine_v5::o5_getStringWidth() {
	int string, width = 0;
	byte *ptr;

	getResultPos();
	string = getVarOrDirectByte(PARAM_1);
	ptr = getResourceAddress(rtString, string);
	assert(ptr);

	width = _charset->getStringWidth(0, ptr);

	setResult(width);
}

void ScummEngine_v5::o5_expression() {
	int dst, i;

	_scummStackPos = 0;
	getResultPos();
	dst = _resultVarNumber;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* varordirect */
			push(getVarOrDirectWord(PARAM_1));
			break;
		case 2:										/* add */
			i = pop();
			push(i + pop());
			break;
		case 3:										/* sub */
			i = pop();
			push(pop() - i);
			break;
		case 4:										/* mul */
			i = pop();
			push(i * pop());
			break;
		case 5:										/* div */
			i = pop();
			if (i == 0)
				error("Divide by zero");
			push(pop() / i);
			break;
		case 6:										/* normal opcode */
			_opcode = fetchScriptByte();
			executeOpcode(_opcode);
			push(_scummVars[0]);
			break;
		default:
			break;
		}
	}

	_resultVarNumber = dst;
	setResult(pop());
}

void ScummEngine_v5::o5_faceActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int obj = getVarOrDirectWord(PARAM_2);
	Actor *a = derefActorSafe(act, "o5_faceActor");
	if (a)
		a->faceToObject(obj);
}

void ScummEngine_v5::o5_findInventory() {
	getResultPos();
	int x = getVarOrDirectByte(PARAM_1);
	int y = getVarOrDirectByte(PARAM_2);
	setResult(findInventory(x, y));
}

void ScummEngine_v5::o5_findObject() {
	getResultPos();
	int x = getVarOrDirectByte(PARAM_1);
	int y = getVarOrDirectByte(PARAM_2);
	int obj = findObject(x, y);

	// WORKAROUND bug #13367: In some versions of Loom, it's possible to
	// walk right through the closed cell door if you allowed Stoke to lead
	// you into the cell rather than skipping the cutscene. This is because
	// the open door (object 623) isn't made non-touchable when the door
	// closes at the end of the cutscene.
	//
	// The FM Towns and TurboGrafx-16 versions fix this by making sure the
	// object is untouchable at the end of the cutscene. The Macintosh and
	// VGA talkie versions make sure the object script checks if the door
	// is open. This makes the script identical to the script for the wall
	// to the left of the door (object 609).
	//
	// These fixes produce subtly different behavior, but since the VGA
	// talkie version (sadly) is the most readily available these days,
	// let's go with that fix. But we do it by redirecting the click to the
	// wall object instead.

	if (_game.id == GID_LOOM && _game.version == 3 &&
	    (_game.platform == Common::kPlatformDOS || _game.platform == Common::kPlatformAmiga || _game.platform == Common::kPlatformAtariST) &&
		_currentRoom == 38 && obj == 623 && enhancementEnabled(kEnhMinorBugFixes)) {
		obj = 609;
	}

	// WORKAROUND bug #13385: Clicking on the cave entrance to go back into
	// the dragon caves registers on the incorrect object. Since the object
	// script is responsible for actually moving you to the other room and
	// this script is empty, redirect the action to the cave object's
	// script instead.
	if (_game.id == GID_LOOM && _game.version == 4 && _currentRoom == 33 && obj == 482 && enhancementEnabled(kEnhMinorBugFixes)) {
		obj = 468;
	}

	setResult(obj);
}

void ScummEngine_v5::o5_freezeScripts() {
	int scr = getVarOrDirectByte(PARAM_1);

	if (scr != 0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void ScummEngine_v5::o5_getActorCostume() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorCostume");
	setResult(a->_costume);
}

void ScummEngine_v5::o5_getActorElevation() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorElevation");
	setResult(a->getElevation());
}

void ScummEngine_v5::o5_getActorFacing() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorFacing");
	setResult(newDirToOldDir(a->getFacing()));
}

void ScummEngine_v5::o5_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorMoving");
	setResult(a->_moving);
}

void ScummEngine_v5::o5_getActorRoom() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);

	// Sometimes this function is called with an invalid actor argument.
	// An example of that is INDY4 bug #832, in which (quoting dwatteau):
	//
	// ---
	// Script 94-206 is started by script 94-200 this way:
	//
	// Var[442] = getObjectOwner(586)  (the metal rod)
	// startScript(201,[Var[442]],F)
	// startScript(206,[Var[442]],F,R)
	//
	// Script 201 gets to run first, and it changes the value of Var[442],
	// so by the time script 206 is invoked, it gets a bad value as param.
	// ---
	//
	// The original doesn't use a structure for actors' data, and instead uses
	// several scattered arrays (one for each parameter, e.g. x, y, room,
	// elevation, etc.), each one with as many max elements as the max number
	// of actors allowed in the engine. Whenever an edge case like this happens,
	// _actorRoom[<invalid-idx>] basically yields whichever value is found in
	// memory after the bounds of the array, so it's pretty much undefined behavior.
	//
	// We certainly can't allow that in our code, so we just set the result to 0.
	if (!isValidActor(act)) {
		setResult(0);
		return;
	}

	Actor *a = derefActor(act, "o5_getActorRoom");
	setResult(a->_room);
}

void ScummEngine_v5::o5_getActorScale() {
	Actor *a;

	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	a = derefActor(act, "o5_getActorScale");
	setResult(a->_scalex);
}

void ScummEngine_v5::o5_getActorWalkBox() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorWalkBox");
	setResult(a->_walkbox);
}

void ScummEngine_v5::o5_getActorWidth() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getActorWidth");
	setResult(a->_width);
}

void ScummEngine_v5::o5_getActorX() {
	int a;
	getResultPos();

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh))
		a = getVarOrDirectByte(PARAM_1);
	else
		a = getVarOrDirectWord(PARAM_1);

	setResult(getObjX(a));
}

void ScummEngine_v5::o5_getActorY() {
	int a;
	getResultPos();

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh))
		a = getVarOrDirectByte(PARAM_1);
	else
		a = getVarOrDirectWord(PARAM_1);

	setResult(getObjY(a));
}

void ScummEngine_v5::o5_getAnimCounter() {
	getResultPos();

	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o5_getAnimCounter");
	setResult(a->_cost.animCounter);
}

void ScummEngine_v5::o5_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This code can't detect any actors farther away than 255 units
	// (pixels in newer games, characters in older ones.) But this is
	// perfectly OK, as it is exactly how the original behaved.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectWord(PARAM_1);
	obj = VAR(VAR_ACTOR_RANGE_MAX);

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj >= VAR(VAR_ACTOR_RANGE_MIN));

	setResult(closest_obj);
}

void ScummEngine_v5::o5_getDist() {
	int o1, o2;
	int r;

	getResultPos();

	o1 = getVarOrDirectWord(PARAM_1);
	o2 = getVarOrDirectWord(PARAM_2);

	if (_game.version == 0) // in v0 both parameters are always actor IDs, never objects
		r = getObjActToObjActDist(actorToObj(o1), actorToObj(o2));
	else
		r = getObjActToObjActDist(o1, o2);

	setResult(r);
}

void ScummEngine_v5::o5_getInventoryCount() {
	getResultPos();
	setResult(getInventoryCount(getVarOrDirectByte(PARAM_1)));
}

void ScummEngine_v5::o5_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(PARAM_1)));
}

void ScummEngine_v5::o5_getObjectState() {
	getResultPos();
	setResult(getState(getVarOrDirectWord(PARAM_1)));
}

void ScummEngine_v5::o5_getRandomNr() {
	getResultPos();
	int max = getVarOrDirectByte(PARAM_1);
	int rnd = _rnd.getRandomNumber(max);
	setResult(rnd);

	debug(6, "o5_getRandomNr(): %d (min: 0, max: %d)", rnd, max);
}

void ScummEngine_v5::o5_isScriptRunning() {
	getResultPos();
	setResult(isScriptRunning(getVarOrDirectByte(PARAM_1)));

	// WORKAROUND bug #346 (also occurs in original): Object stopped with active cutscene
	// In script 204 room 25 (Cannibal Village) a crash can occur when you are
	// expected to give something to the cannibals, but instead wait a bit and look at
	// certain items, like the compass or kidnap note. Those inventory items contain little
	// cutscenes and are abrubtly stopped by the endcutscene in script 204 at 0x0060.
	// This patch changes the result of isScriptRunning(164) to also wait for any
	// inventory scripts that are in a cutscene state, preventing the crash.
	//
	// (The original interpreter would print a fatal "Object xxx stopped with active
	// cutscene/override" error.)
	//
	// Note: the SCUMMv4 releases also produce strange animation results in this case, but
	// (AFAICS) no error. Fixing this would be nice as well (as a `kEnhMinorBugFixes` fix)
	// but it would require a different workaround in a different place, since the script
	// is a bit different.
	if (_game.id == GID_MONKEY && _currentScript != 0xFF && vm.slot [_currentScript].number == 204 && _currentRoom == 25 &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		ScriptSlot *ss = vm.slot;
		for (int i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
			if (ss->status != ssDead && ss->where == WIO_INVENTORY && ss->cutsceneOverride) {
				setResult(1);
				return;
			}
		}
	}
}

void ScummEngine_v5::o5_getVerbEntrypoint() {
	int a, b;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	b = getVarOrDirectWord(PARAM_2);

	setResult(getVerbEntrypoint(a, b));
}

void ScummEngine_v5::o5_ifClassOfIs() {
	int obj, cls, b = 0;
	bool cond = true;

	obj = getVarOrDirectWord(PARAM_1);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(PARAM_1);

		// WORKAROUND bug #3099: Due to a script bug, the wrong opcode is
		// used to test and set the state of various objects (e.g. the inside
		// door (object 465) of the of the Hostel on Mars), when opening the
		// Hostel door from the outside.
		//
		// TODO: check the behavior of the original interpreter against ours,
		// in this particular case.
		if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns &&
		    vm.slot[_currentScript].number == 205 && _currentRoom == 185 &&
		    obj == 465 && cls == 0 && enhancementEnabled(kEnhGameBreakingBugFixes)) {
			cond = (getState(obj) == 0);
		} else {
			b = getClass(obj, cls);
			if (((cls & 0x80) && !b) || (!(cls & 0x80) && b))
				cond = false;
		}
	}
	jumpRelative(cond);
}

void ScummEngine_v5::o5_increment() {
	getResultPos();
	setResult(readVar(_resultVarNumber) + 1);
}

void ScummEngine_v5::o5_isActorInBox() {
	int act = getVarOrDirectByte(PARAM_1);
	int box = getVarOrDirectByte(PARAM_2);
	Actor *a = derefActor(act, "o5_isActorInBox");

	jumpRelative(checkXYInBoxBounds(box, a->getRealPos().x, a->getRealPos().y));
}

void ScummEngine_v5::o5_isEqual() {
	int16 a, b;
	int var;

	if (_game.version <= 2)
		var = fetchScriptByte();
	else
		var = fetchScriptWord();
	a = readVar(var);
	b = getVarOrDirectWord(PARAM_1);

	// WORKAROUND: Looking at the map on Hook Isle or in Meathook's house
	// causes various issues. The CD release prevented this by making
	// Guybrush say "I don't need to look at a map right now." instead, so
	// backport this fix to the floppy EGA/VGA releases.
	//
	// (It looks like the (or some?) Amiga releases already have this
	// fix, but it's written in a way so that it can't hurt.)
	if ((_game.id == GID_MONKEY_EGA || _game.id == GID_MONKEY_VGA) && _currentScript != 0xFF && vm.slot [_currentScript].number == 120 && var == VAR_ROOM && b == 29) {
		// In Meathook's house; check is missing from both EGA and VGA SCUMMv4
		// releases. It can cause a fatal "ERROR: (63:49:0x14A): Local script 207
		// is not in room 63!" error, if one looks at the map instead of using
		// any verb on the parrot, so this requires `kEnhGameBreakingBugFixes`.
		if (a == 37 && enhancementEnabled(kEnhGameBreakingBugFixes))
			b = a;

		// Hook Isle Shore; check is missing from EGA SCUMMv4 releases only.
		// It just causes strange animations, so `kEnhMinorBugFixes` is enough.
		if (_game.id == GID_MONKEY_EGA && a == 48 && enhancementEnabled(kEnhMinorBugFixes))
			b = a;
	}

	// HACK: See bug report #441. The sound effects for Largo's screams
	// are only played on type 5 soundcards. However, there is at least one
	// other sound effect (the bartender spitting) which is only played on
	// type 3 soundcards.
	if (_game.id == GID_MONKEY2 && var == VAR_SOUNDCARD && b == 5)
		b = a;

	// WORKAROUND: The Ultimate Talkie edition of Monkey Island 2 doesn't
	// check the proper objects when you sell back the hub cap and the
	// pirate hat to the antique dealer on Booty Island, making Guybrush
	// silent when he asks about these two particular objects.
	//
	// Not using enhancementEnabled, since this small oversight only
	// exists in this fan-made edition which was made for enhancements.
	if (_game.id == GID_MONKEY2 && (_game.features & GF_ULTIMATE_TALKIE) && _roomResource == 48 && _currentScript != 0xFF && vm.slot [_currentScript].number == 215 && a == vm.localvar[_currentScript][0]) {
		if (a == 550 && b == 530)
			b = a;
		else if (a == 549 && b == 529)
			b = a;
	}

	// WORKAROUND: The Ultimate Talkie edition of Monkey Island 2 has no
	// audio for the "Hey spitter" and "C'mon!  What are you?  Afraid?"
	// lines from the crowd in the spitting contest. It's there in the
	// 000001e1, 00000661, 00000caf, 000001e8, 0000066c and 00000cba.wav
	// files, but these resources are not called in this script, and it also
	// looks like they're not built into the MONSTER.SOU file either, so
	// these lines remain silent at the moment, although they were voiced.
	//
	// Try to detect and skip them, which as much care as possible for any
	// future update or fan translation which would change this.
	//
	// Intentionally not using enhancementEnabled for this version.
	if (_game.id == GID_MONKEY2 && (_game.features & GF_ULTIMATE_TALKIE) &&
		_roomResource == 47 && vm.slot[_currentScript].number == 218 &&
		var == 0x4000 + 1 && a == vm.localvar[_currentScript][1] &&
		a == b && (b == 7 || b == 13)) {
		// No need to skip any line if playing in always-prefer-original-text
		// mode (Bit[588]) where silent lines are expected, or if speech is muted.
		if (readVar(0x8000 + 588) == 1 && !ConfMan.getBool("speech_mute")) {
			// Only skip the line when we can detect one and it has no sound prologue.
			if (memcmp(_scriptPointer + 2, "\x27\x01\x1D", 3) == 0 && memcmp(_scriptPointer + 5, "\xFF\x0A", 2) != 0) {
				// Cheat and use the next recorded line, but do it in a way so that it
				// shouldn't be played twice in a row.
				if (vm.localvar[_currentScript][1] == _scummVars[516])
					_scummVars[516]++;
				vm.localvar[_currentScript][1]++;
				a = -1;
			}
		}
	}

	// HACK: To allow demo script of Maniac Mansion V2
	// The camera x position is only 100, instead of 180, after game title name scrolls.
	if (_game.id == GID_MANIAC && _game.version == 2 && (_game.features & GF_DEMO) && isScriptRunning(173) && b == 180)
		b = 100;

	jumpRelative(b == a);
}

void ScummEngine_v5::o5_isGreater() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	jumpRelative(b > a);
}

void ScummEngine_v5::o5_isGreaterEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	jumpRelative(b >= a);
}

void ScummEngine_v5::o5_isLess() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	jumpRelative(b < a);
}

void ScummEngine_v5::o5_isLessEqual() {
	int var = fetchScriptWord();
	int16 a = readVar(var);
	int16 b = getVarOrDirectWord(PARAM_1);

	// WORKAROUND bug #1266: INDY3TOWNS: Biplane controls are haywire.
	// This is broken under UNZ too; the script does an incorrect signed
	// comparison, possibly with the intent of checking for a gamepad.
	//
	// Since the biplane is unplayable without this, we use
	// `kEnhGameBreakingBugFixes`.
	if (_game.id == GID_INDY3 && (_game.platform == Common::kPlatformFMTowns) &&
	    (vm.slot[_currentScript].number == 200 || vm.slot[_currentScript].number == 203) &&
	    _currentRoom == 70 && b == -256 && enhancementEnabled(kEnhGameBreakingBugFixes)) {
		o5_jumpRelative();
		return;
	}

	// WORKAROUND: When Mandible uses the distaff, it seems to light up
	// only three times with no animation for the second note. Actually,
	// the animations for the first and second notes are played so closely
	// together that they look like one. This adjusts the timing of the
	// second one.

	if (_game.id == GID_LOOM && _game.version >= 4 && _language == Common::EN_ANY && vm.slot[_currentScript].number == 95 && var == VAR_MUSIC_TIMER && b == 1708 && enhancementEnabled(kEnhVisualChanges)) {
		b = 1815;
	}

	jumpRelative(b <= a);
}

void ScummEngine_v5::o5_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectWord(PARAM_1);
	jumpRelative(b != a);
}

void ScummEngine_v5::o5_notEqualZero() {
	int a;

	// WORKAROUND for a possible dead-end in Monkey Island 2. By luck, this
	// only happens in the Ultimate Talkie Edition (because it fixes another
	// script error which unveils this one), or when one enables the second
	// workaround just below in one of the original releases.
	//
	// Once Bit[70] has been properly set by one of the configurations above,
	// Captain Dread will have his intended reaction of forcing you to go back
	// to Scabb Island, once you've got the four map pieces. But, unless you're
	// playing in Lite mode, you'll need the lens from the model lighthouse,
	// otherwise Wally won't be able to read the map, and you'll be completely
	// stuck on Scabb Island with no way of going back to the Phatt Island
	// Library, since Dread's ship is gone.
	if (_game.id == GID_MONKEY2 && ((_roomResource == 22 && vm.slot[_currentScript].number == 202) ||
		(_roomResource == 2 && vm.slot[_currentScript].number == 10002) ||
		vm.slot[_currentScript].number == 97) && enhancementEnabled(kEnhGameBreakingBugFixes)) {
		int var = fetchScriptWord();
		a = readVar(var);

		// WORKAROUND: When Guybrush buys a map piece from the antiques dealer,
		// the script forgets to set Bit[70], which means that an intended
		// reaction from Captain Dread forcing you to go back to Scabb when you
		// get the full map was never triggered in the original game.
		//
		// The Ultimate Edition fixed this in script 48-207 (when you buy the
		// map piece), but for the other versions we're fixing it on-the-fly
		// at the last moment instead (by checking for the object in the
		// inventory instead of Bit[70]), so that it will also work with older
		// savegames, and so that you can uncheck the Enhancement option at any
		// moment if you realize that you want the original behavior.
		//
		// Note that fixing this unveils the script error causing the possible
		// dead-end described above.
		if (!(_game.features & GF_ULTIMATE_TALKIE) && var == 0x8000 + 70 && a == 0 && getOwner(519) == VAR(VAR_EGO) && enhancementEnabled(kEnhRestoredContent)) {
			a = 1;
		}

		// [Back to the previous "dead-end" workaround.]
		// If you've got the four map pieces and the script is checking this...
		else if (var == 0x8000 + 69 && a == 1 && getOwner(519) == VAR(VAR_EGO) && readVar(0x8000 + 55) == 1 && readVar(0x8000 + 366) == 1) {
			// ...but you don't have the lens and you never gave it to Wally...
			// (and you're not playing the Lite mode, where this doesn't matter)
			if (getOwner(295) != VAR(VAR_EGO) && readVar(0x8000 + 67) != 0 && readVar(0x8000 + 567) == 0) {
				// ...then short-circuit this condition, so that you can still go back
				// to Phatt Island to pick up the lens, as in the original game.
				a = 0;
			}
		}
	} else {
		a = getVar();
	}

	jumpRelative(a != 0);
}

void ScummEngine_v5::o5_equalZero() {
	const byte *oldaddr = _scriptPointer - 1;
	int a;

	// WORKAROUND: Examining the dragon's pile of gold a second time causes
	// Bobbin to animate as if he's talking, but no text is displayed. When
	// running the game in an emulator, there's neither text nor animation
	// when examining the pile again. While the symptoms are slightly
	// different, this points to a script bug.
	//
	// I think this happens because in the PC Engine version the entire
	// scene is a cutscene. In the EGA version, only the part where the
	// dragon responds is. So the cutscene starts, the message is printed
	// and then the cutscene immediately ends, which triggers an "end of
	// cutscene" script. This is probably what clears the text.
	//
	// The script sets Bit[92] to indicate that the dragon has responded.
	// If the bit has been set, we simulate a WaitForMessage() instruction
	// here, so that the script pauses until the "Wow!" message is gone.

	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && vm.slot[_currentScript].number == 109 &&
		enhancementEnabled(kEnhMinorBugFixes)) {
		int var = fetchScriptWord();
		a = readVar(var);

		if (var == 32860 && a == 1 && VAR(VAR_HAVE_MSG)) {
			_scriptPointer = oldaddr;
			o5_breakHere();
			return;
		}
	} else {
		a = getVar();
	}

	jumpRelative(a == 0);
}

void ScummEngine_v5::o5_jumpRelative() {
	jumpRelative(false);
}

void ScummEngine_v5::o5_lights() {
	int a, b, c;

	a = getVarOrDirectByte(PARAM_1);
	b = fetchScriptByte();
	c = fetchScriptByte();

	if (c == 0)
		VAR(VAR_CURRENT_LIGHTS) = a;
	else if (c == 1) {
		_flashlight.xStrips = a;
		_flashlight.yStrips = b;
	}
	_fullRedraw = true;
}

void ScummEngine_v5::o5_loadRoom() {
	int room;

	room = getVarOrDirectByte(PARAM_1);

	// WORKAROUND bug #12420 (also occurs in original) Broken window and coat missing
	// This happens when you skip the cutscenes in the beginning, in particular
	// the one where Indy enters the office for the first time. If object 23 (National
	// Archeology) is in possession of Indy (owner == 1) then it's safe to force the
	// coat (object 24) and broken window (object 25) into the room.
	if (_game.id == GID_INDY4 && room == 1 && _objectOwnerTable[23] == 1 && enhancementEnabled(kEnhMinorBugFixes)) {
		putState(24, 1);
		putState(25, 1);
	}

	// WORKAROUND: The first time you examine Rusty while he's sleeping,
	// you will get a close-up of him. Which one should depend on whether
	// or not you've used the Reflection draft on him. But in some versions,
	// you will always get the close-up where he's wearing his own clothes.

	if (_game.id == GID_LOOM && _game.version == 3 && room == 29 &&
		vm.slot[_currentScript].number == 112 && enhancementEnabled(kEnhVisualChanges)) {
		Actor *a = derefActorSafe(VAR(VAR_EGO), "o5_loadRoom");

		// Bobbin's normal costume is number 1. If he's wearing anything
		// else, he's presumably disguised as Rusty. The game also sets
		// a variable, but uses different ones for different versions of
		// the game. You can't even assume that every English version
		// uses the same one!

		if (a && a->_costume != 1)
			room = 68;
	}

	// For small header games, we only call startScene if the room
	// actually changed. This avoid unwanted (wrong) fades in Zak256
	// and others. OTOH, it seems to cause a problem in newer games.
	if (!(_game.features & GF_SMALL_HEADER) || room != _currentRoom)
		startScene(room, nullptr, 0);

	// DIG and COMI don't flag a full redraw after starting the scene.
	if (_game.version < 7 || _game.id == GID_FT)
		_fullRedraw = true;
}

void ScummEngine_v5::o5_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;
	int x2, y2, dir, oldDir;

	obj = getVarOrDirectWord(PARAM_1);
	room = getVarOrDirectByte(PARAM_2);

	a = derefActor(VAR(VAR_EGO), "o5_loadRoomWithEgo");

	a->putActor(room);
	oldDir = a->getFacing();
	_egoPositioned = false;

	x = fetchScriptWordSigned();
	y = fetchScriptWordSigned();

	VAR(VAR_WALKTO_OBJ) = obj;
	startScene(a->_room, a, obj);
	VAR(VAR_WALKTO_OBJ) = 0;

	if (_game.version <= 4) {
		if (whereIsObject(obj) != WIO_ROOM)
			error("o5_loadRoomWithEgo: Object %d is not in room %d", obj, _currentRoom);
		if (!_egoPositioned) {
			getObjectXYPos(obj, x2, y2, dir);
			a->putActor(x2, y2, _currentRoom);
			if (a->getFacing() == oldDir)
				a->setDirection(dir + 180);
		}
		a->_moving = 0;
	}

	// This is based on disassembly
	camera._cur.x = camera._dest.x = a->getPos().x;
	if ((_game.id == GID_ZAK || _game.id == GID_LOOM) && (_game.platform == Common::kPlatformFMTowns)) {
		setCameraAt(a->getPos().x, a->getPos().y);
	}
	setCameraFollows(a);

	_fullRedraw = true;

	if (x != -1) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v5::o5_matrixOps() {
	int a, b;

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxFlags(a, b);
		break;
	case 2:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxScale(a, b);
		break;
	case 3:
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		setBoxScale(a, (b - 1) | 0x8000);
		break;
	case 4:
		createBoxMatrix();
		break;
	default:
		break;
	}
}

void ScummEngine_v5::o5_move() {
	getResultPos();
	setResult(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_multiply() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) * a);
}

void ScummEngine_v5::o5_or() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) | a);
}

void ScummEngine_v5::o5_beginOverride() {
	if (fetchScriptByte() != 0)
		beginOverride();
	else
		endOverride();
}

void ScummEngine_v5::o5_panCameraTo() {
	panCameraTo(getVarOrDirectWord(PARAM_1), 0);
}

void ScummEngine_v5::o5_pickupObject() {
	int obj, room;
	obj = getVarOrDirectWord(PARAM_1);
	room = getVarOrDirectByte(PARAM_2);
	if (room == 0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void ScummEngine_v5::o5_print() {
	// WORKAROUND bug #13374: The patched script for the Ultimate Talkie
	// is missing a WaitForMessage() after Lemonhead says "Oooh, that's
	// nice." so we insert one here. If there is a future version that
	// fixes this, the workaround still shouldn't do any harm.
	//
	// The workaround is deliberately not marked as an enhancement, since
	// this version makes so many changes of its own.
	if (_game.id == GID_MONKEY && (_game.features & GF_ULTIMATE_TALKIE) && _currentRoom == 25 && vm.slot[_currentScript].number == 205 && VAR(VAR_HAVE_MSG)) {
		_scriptPointer--;
		o5_breakHere();
		return;
	}

	_actorToPrintStrFor = getVarOrDirectByte(PARAM_1);
	decodeParseString();
}

void ScummEngine_v5::o5_printEgo() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void ScummEngine_v5::o5_pseudoRoom() {
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j & 0x7F] = i;
		}
	}
}

void ScummEngine_v5::o5_putActor() {
	int act, x, y;

	act = getVarOrDirectByte(PARAM_1);
	x = getVarOrDirectWord(PARAM_2);
	y = getVarOrDirectWord(PARAM_3);

	// WORKAROUND: When enabling the cigar smoke in the captain Smirk
	// close-up, it turns out that the coordinates in the CD
	// version's script were taken from the EGA version.
	//
	// The coordinates below are taken from the VGA floppy version. The
	// "Ultimate Talkie" version also corrects the positions, but uses
	// other coordinates. The difference is never more than a single pixel,
	// so there's not much reason to correct those.

	if (_game.id == GID_MONKEY && _currentRoom == 76 && act == 12 && enhancementEnabled(kEnhVisualChanges)) {
		if (x == 176 && y == 80) {
			x = 174;
			y = 86;
		} else if (x == 176 && y == 78) {
			x = 172;
		}
	} else if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns && _currentRoom == 42 && vm.slot[_currentScript].number == 201 && act == 6 && x == 136 && y == 0 && enhancementEnabled(kEnhVisualChanges)) {
		// WORKAROUND: bug #2762: When switching back to Zak after using the blue
		// crystal on the bird in Lima, the bird will disappear, come back and
		// disappear again. This is really strange and only happens with the
		// FM-TOWNS version, which adds an unconditional putActor(6,136,0) sequence
		// that will always negate the getActorX()/getActorY() checks that follow.
		return;
	}

	Actor *a = derefActor(act, "o5_putActor");
	a->putActor(x, y);
}

void ScummEngine_v5::o5_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_putActorAtObject");
	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;

		// WORKAROUND: When Guybrush dives down to the Mad Monkey, he
		// is positioned near the anchor (though since it can't be
		// found yet, it uses this default position). He's then lowered
		// to the ocean floor by adjusting his elevation. But he will
		// be drawn for a split second at the unelevated position. This
		// is a bug in the original game, and we work around it by
		// adjusting the elevation immediately.

		if (_game.id == GID_MONKEY2 && a->_number == 1 && vm.slot[_currentScript].number == 58 && enhancementEnabled(kEnhMinorBugFixes)) {
			a->setElevation(99);
		}
	}
	a->putActor(x, y);
}

void ScummEngine_v5::o5_putActorInRoom() {
	Actor *a;
	int act = getVarOrDirectByte(PARAM_1);
	int room = getVarOrDirectByte(PARAM_2);

	a = derefActor(act, "o5_putActorInRoom");

	if (a->_visible && _currentRoom != room && getTalkingActor() == a->_number) {
		stopTalk();
	}
	a->_room = room;
	if (!room)
		a->putActor(0, 0, 0);
}

void ScummEngine_v5::o5_systemOps() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 1:		// SO_RESTART
		restart();
		break;
	case 2:		// SO_PAUSE
		pauseGame();
		break;
	case 3:		// SO_QUIT
		_quitFromScriptCmd = true;
		quitGame();
		break;
	default:
		error("o5_systemOps: unknown subopcode %d", subOp);
	}
}

void ScummEngine_v5::o5_resourceRoutines() {
	const ResType resType[4] = { rtScript, rtSound, rtCostume, rtRoom };
	int resid = 0;
	int foo, bar;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		resid = getVarOrDirectByte(PARAM_1);
	if (!(_game.platform == Common::kPlatformFMTowns)) {
		// FIXME - this probably can be removed eventually, I don't think the following
		// check will ever be triggered, but then I could be wrong and it's better
		// to play it safe.
		if ((_opcode & 0x3F) != (_opcode & 0x1F))
			error("Oops, this shouldn't happen: o5_resourceRoutines opcode %d", _opcode);
	}

	int op = _opcode & 0x3F;

	// FIXME: Sound resources are currently missing
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine &&
		(op == 2 || op == 6)) {
			return;
	}

	switch (op) {
	case 1:			// SO_LOAD_SCRIPT
	case 2:			// SO_LOAD_SOUND
	case 3:			// SO_LOAD_COSTUME
		ensureResourceLoaded(resType[op - 1], resid);
		break;
	case 4:			// SO_LOAD_ROOM
		ensureResourceLoaded(rtRoom, resid);
		if (_game.version == 3) {
			if (resid > 0x7F)
				resid = _resourceMapper[resid & 0x7F];

			if (_currentRoom != resid) {
				_res->setResourceCounter(rtRoom, resid, 1);
			}
		}
		break;

	case 5:			// SO_NUKE_SCRIPT
	case 6:			// SO_NUKE_SOUND
	case 7:			// SO_NUKE_COSTUME
	case 8:			// SO_NUKE_ROOM
		if (_game.id == GID_ZAK && (_game.platform == Common::kPlatformFMTowns))
			error("o5_resourceRoutines %d should not occur in Zak256", op);
		else
			_res->setResourceCounter(resType[op-5], resid, 0x7F);
		break;
	case 9:			// SO_LOCK_SCRIPT
		if (resid >= _numGlobalScripts)
			break;
		_res->lock(rtScript, resid);
		break;
	case 10:		// SO_LOCK_SOUND
		// FIXME: Sound resources are currently missing
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)
			break;
		_res->lock(rtSound, resid);
		break;
	case 11:		// SO_LOCK_COSTUME
		_res->lock(rtCostume, resid);
		break;
	case 12:		// SO_LOCK_ROOM
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->lock(rtRoom, resid);
		break;

	case 13:		// SO_UNLOCK_SCRIPT
		if (resid >= _numGlobalScripts)
			break;
		_res->unlock(rtScript, resid);
		break;
	case 14:		// SO_UNLOCK_SOUND
		// FIXME: Sound resources are currently missing
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)
			break;
		_res->unlock(rtSound, resid);
		break;
	case 15:		// SO_UNLOCK_COSTUME
		_res->unlock(rtCostume, resid);
		break;
	case 16:		// SO_UNLOCK_ROOM
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->unlock(rtRoom, resid);
		break;

	case 17:		// SO_CLEAR_HEAP
		//heapClear(0);
		//unkHeapProc2(0, 0);
		break;
	case 18:		// SO_LOAD_CHARSET
		loadCharset(resid);
		break;
	case 19:		// SO_NUKE_CHARSET
		nukeCharset(resid);
		break;
	case 20:		// SO_LOAD_OBJECT
		loadFlObject(getVarOrDirectWord(PARAM_2), resid);
		break;

	// TODO: For the following see also Hibernatus' information on bug #7315.
	case 32:
		// TODO (apparently never used in FM-TOWNS)
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op, _currentScript != 0xFF ? vm.slot [_currentScript].number : -1);
		break;
	case 33:
		// TODO (apparently never used in FM-TOWNS)
		debug(0, "o5_resourceRoutines %d not yet handled (script %d)", op, _currentScript != 0xFF ? vm.slot [_currentScript].number : -1);
		break;
	case 35:
		if (_townsPlayer)
			_townsPlayer->setVolumeCD(getVarOrDirectByte(PARAM_2), resid);
		break;
	case 36:
		foo = getVarOrDirectByte(PARAM_2);
		bar = fetchScriptByte();
		if (_townsPlayer)
			_townsPlayer->setSoundVolume(resid, foo, bar);
		break;
	case 37:
		if (_townsPlayer)
			_townsPlayer->setSoundNote(resid, getVarOrDirectByte(PARAM_2));
		break;

	default:
		error("o5_resourceRoutines: default case %d", op);
	}
}

void ScummEngine_v5::o5_roomOps() {
	int a = 0, b = 0, c, d, e;
	const bool paramsBeforeOpcode = ((_game.version == 3) && (_game.platform != Common::kPlatformPCEngine));

	if (paramsBeforeOpcode) {
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:		// SO_ROOM_SCROLL
		if (!paramsBeforeOpcode) {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
		}
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;
	case 2:		// SO_ROOM_COLOR
		if (_game.features & GF_SMALL_HEADER) {
			if (!paramsBeforeOpcode) {
				a = getVarOrDirectWord(PARAM_1);
				b = getVarOrDirectWord(PARAM_2);
			}
			assertRange(0, a, 256, "o5_roomOps: 2: room color slot");
			_roomPalette[b] = a;
			_fullRedraw = true;
		} else {
			error("room-color is no longer a valid command");
		}
		break;

	case 3:		// SO_ROOM_SCREEN
		if (!paramsBeforeOpcode) {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
		}

		// Mac version, draw the screens 20 pixels lower to account for the extra 40 pixels
		if (_game.platform == Common::kPlatformMacintosh && _game.version == 3 && _useMacScreenCorrectHeight) {
			a += _screenDrawOffset;
			b += _screenDrawOffset;
		}

		initScreens(a, b);
		break;
	case 4:		// SO_ROOM_PALETTE
		if (_game.features & GF_SMALL_HEADER) {
			if (!paramsBeforeOpcode) {
				a = getVarOrDirectWord(PARAM_1);
				b = getVarOrDirectWord(PARAM_2);
			}
			assertRange(0, a, 256, "o5_roomOps: 4: room color slot");
			_shadowPalette[b] = a;

			// In b/w Mac rendering mode, the shadow palette is
			// handled by the renderer itself. See comment in
			// mac_drawStripToScreen().

			if (_renderMode == Common::kRenderMacintoshBW) {
				_fullRedraw = true;
			} else
				setDirtyColors(b, b);
		} else {
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectWord(PARAM_2);
			c = getVarOrDirectWord(PARAM_3);
			_opcode = fetchScriptByte();
			d = getVarOrDirectByte(PARAM_1);

			// WORKAROUND: The CD version of Monkey Island 1 will
			// set a couple of default colors, presumably for the
			// GUI to use. But in the close-up of captain Smirk,
			// we want the original color 3 for the cigar smoke. It
			// should be ok since there is no GUI in this scene.

			if (_game.id == GID_MONKEY && _currentRoom == 76 && d == 3 && enhancementEnabled(kEnhVisualChanges)) {
				// Do nothing
			} else {
				setPalColor(d, a, b, c);	/* index, r, g, b */
			}
		}
		break;
	case 5:		// SO_ROOM_SHAKE_ON
		setShake(1);
		break;
	case 6:		// SO_ROOM_SHAKE_OFF
		setShake(0);
		break;
	case 7:		// SO_ROOM_SCALE
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		c = getVarOrDirectByte(PARAM_1);
		d = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		e = getVarOrDirectByte(PARAM_2);
		setScaleSlot(e - 1, 0, b, a, 0, d, c);
		break;
	case 8:		// SO_ROOM_INTENSITY
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		c = getVarOrDirectByte(PARAM_3);
		darkenPalette(a, a, a, b, c);
		break;
	case 9:		// SO_ROOM_SAVEGAME
		_saveLoadFlag = getVarOrDirectByte(PARAM_1);
		_saveLoadSlot = getVarOrDirectByte(PARAM_2);
		_saveLoadSlot = 99;					/* use this slot */
		_saveTemporaryState = true;
		break;
	case 10:	// SO_ROOM_FADE
		a = getVarOrDirectWord(PARAM_1);
		if (a) {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (_game.platform == Common::kPlatformFMTowns) {
				switch (a) {
				case 8:
					towns_drawStripToScreen(&_virtscr[kMainVirtScreen], 0, _virtscr[kMainVirtScreen].topline, 0, 0, _virtscr[kMainVirtScreen].w, _virtscr[kMainVirtScreen].topline + _virtscr[kMainVirtScreen].h);
					_townsScreen->update();
					return;
				case 9:
					_townsActiveLayerFlags = 2;
					_townsScreen->toggleLayers(_townsActiveLayerFlags);
					return;
				case 10:
					_townsActiveLayerFlags = 3;
					_townsScreen->toggleLayers(_townsActiveLayerFlags);
					return;
				case 11:
					_townsScreen->clearLayer(1);
					return;
				case 12:
					_townsActiveLayerFlags = 0;
					_townsScreen->toggleLayers(_townsActiveLayerFlags);
					return;
				case 13:
					_townsActiveLayerFlags = 1;
					_townsScreen->toggleLayers(_townsActiveLayerFlags);
					return;
				case 16: // enable clearing of layer 2 buffer in drawBitmap()
					_townsPaletteFlags |= 2;
					return;
				case 17: // disable clearing of layer 2 buffer in drawBitmap()
					_townsPaletteFlags &= ~2;
					return;
				case 18: // clear kMainVirtScreen layer 2 buffer
					_textSurface.fillRect(Common::Rect(0, _virtscr[kMainVirtScreen].topline * _textSurfaceMultiplier, _textSurface.pitch, (_virtscr[kMainVirtScreen].topline + _virtscr[kMainVirtScreen].h) * _textSurfaceMultiplier), 0);
					return;
				case 19: // enable palette operations (palManipulate(), cyclePalette() etc.)
					_townsPaletteFlags |= 1;
					return;
				case 20: // disable palette operations
					_townsPaletteFlags &= ~1;
					return;
				case 21: // disable clearing of layer 0 in initScreens()
					_townsClearLayerFlag = 1;
					return;
				case 22: // enable clearing of layer 0 in initScreens()
					_townsClearLayerFlag = 0;
					return;
				case 30:
					_townsOverrideShadowColor = 3;
					return;
				default:
					break;
				}
			}
#endif // DISABLE_TOWNS_DUAL_LAYER_MODE
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case 11:	// SO_RGB_ROOM_INTENSITY
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
		c = getVarOrDirectWord(PARAM_3);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		e = getVarOrDirectByte(PARAM_2);
		darkenPalette(a, b, c, d, e);
		break;
	case 12:	// SO_ROOM_SHADOW
		a = getVarOrDirectWord(PARAM_1);
		b = getVarOrDirectWord(PARAM_2);
		c = getVarOrDirectWord(PARAM_3);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		e = getVarOrDirectByte(PARAM_2);
		setShadowPalette(a, b, c, d, e, 0, 256);
		break;

	case 13:	// SO_SAVE_STRING
		{
			// This subopcode is used in Indy 4 to save the IQ points
			// data. No other LucasArts game uses it. We use this fact
			// to substitute a filename based on the targetname
			// ("TARGET.iq").
			//
			// This way, the iq data of each Indy 4 variant stays
			// separate. Moreover, the filename now clearly reflects to
			// which target it belongs (as it should).
			//
			// In addition, the Monkey Island fan patch (which adds
			// speech support and more things to MI 1 and 2) uses
			// this opcode to generate a "monkey.cfg" file containing.
			// some user controllable settings.
			// Once more we use a custom filename ("TARGET.cfg").
			Common::String filename;
			char chr;

			a = getVarOrDirectByte(PARAM_1);
			while ((chr = fetchScriptByte()))
				filename += chr;

			if (_game.id == GID_INDY4) {
				filename = _targetName + ".iq";
			} else if (_game.id == GID_MONKEY || _game.id == GID_MONKEY2) {
				filename = _targetName + ".cfg";
			} else {
				error("SO_SAVE_STRING: Unsupported filename %s", filename.c_str());
			}

			Common::OutSaveFile *file = _saveFileMan->openForSaving(filename);
			if (file != nullptr) {
				byte *ptr;
				ptr = getResourceAddress(rtString, a);
				file->write(ptr, resStrLen(ptr) + 1);
				delete file;
				VAR(VAR_SOUNDRESULT) = 0;
			}
			break;
		}
	case 14:	// SO_LOAD_STRING
		{
			// This subopcode is used in Indy 4 to load the IQ points data.
			// See SO_SAVE_STRING for details
			Common::String filename;
			char chr;

			a = getVarOrDirectByte(PARAM_1);
			while ((chr = fetchScriptByte()))
				filename += chr;

			if (_game.id == GID_INDY4) {
				filename = _targetName + ".iq";
			} else if (_game.id == GID_MONKEY || _game.id == GID_MONKEY2) {
				filename = _targetName + ".cfg";
			} else {
				error("SO_LOAD_STRING: Unsupported filename %s", filename.c_str());
			}

			Common::InSaveFile *file = _saveFileMan->openForLoading(filename);
			if (file != nullptr) {
				byte *ptr;
				const int len = file->size();
				ptr = (byte *)malloc(len + 1);
				assert(ptr);
				int r = file->read(ptr, len);
				assert(r == len);
				ptr[len] = '\0';
				loadPtrToResource(rtString, a, ptr);
				free(ptr);
				delete file;
			}
			break;
		}
	case 15:	// SO_ROOM_TRANSFORM
		a = getVarOrDirectByte(PARAM_1);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(PARAM_1);
		c = getVarOrDirectByte(PARAM_2);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(PARAM_1);
		palManipulateInit(a, b, c, d);
		break;

	case 16:	// SO_CYCLE_SPEED
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		assertRange(1, a, 16, "o5_roomOps: 16: color cycle");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;
	default:
		error("o5_roomOps: unknown subopcode %d", _opcode & 0x1F);
		break;
	}
}

void ScummEngine_v5::o5_saveRestoreVerbs() {
	int a, b, c, slot, slot2;

	_opcode = fetchScriptByte();

	a = getVarOrDirectByte(PARAM_1);
	b = getVarOrDirectByte(PARAM_2);
	c = getVarOrDirectByte(PARAM_3);

	switch (_opcode) {
	case 1:		// SO_SAVE_VERBS
		while (a <= b) {
			slot = getVerbSlot(a, 0);
			if (slot && _verbs[slot].saveid == 0) {
				_verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 2:		// SO_RESTORE_VERBS
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a, 0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a, c);
				_verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 3:		// SO_DELETE_VERBS
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o5_saveRestoreVerbs: unknown subopcode %d", _opcode);
	}
}

void ScummEngine_v5::o5_setCameraAt() {
	setCameraAtEx(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_setObjectName() {
	// WORKAROUND bug #10571 (also occurs in original) Object stopped with active cutscene
	// Script 68 contains the code for handling the mugs. The issue occurs when a mug
	// changes state. It will call setObjectName for the new state which in its turn
	// restarts objects in inventory. Some objects (kidnap note) can be in a cutscene state
	// what causes a crash if the object gets restarted. This workaround waits for cutscenes
	// to end, preventing the crash.
	//
	// (The original interpreter would print a fatal "Object xxx stopped with active
	// cutscene/override" error.)
	if (_game.id == GID_MONKEY && vm.slot[_currentScript].number == 68 && enhancementEnabled(kEnhGameBreakingBugFixes)) {
		ScriptSlot *ss = vm.slot;
		for (int i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
			if (ss->status != ssDead && ss->where == WIO_INVENTORY && ss->cutsceneOverride) {
				_scriptPointer--;
				return o5_breakHere();
			}
		}
	}
	int obj = getVarOrDirectWord(PARAM_1);
	setObjectName(obj);
}

void ScummEngine_v5::o5_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(PARAM_1);
	owner = getVarOrDirectByte(PARAM_2);

	setOwnerOf(obj, owner);
}

void ScummEngine_v5::o5_setState() {
	int obj, state;

	obj = getVarOrDirectWord(PARAM_1);
	state = getVarOrDirectByte(PARAM_2);

	// WORKAROUND: The door will glitch if one closes it before using the voodoo
	// doll on Largo. Script 13-213 triggers the same action without any glitch,
	// though, since it properly resets the state of the (invisible) laundry claim
	// ticket part of the door, so we just reuse its setState and setClass calls.
	if (_game.id == GID_MONKEY2 && _currentRoom == 13 && vm.slot[_currentScript].number == 200 &&
		obj == 108 && state == 1 && getState(100) != 1 && getState(111) != 2 && enhancementEnabled(kEnhMinorBugFixes)) {
		putState(111, 2);
		markObjectRectAsDirty(111);
		putClass(111, 160, true);
	}

	putState(obj, state);
	markObjectRectAsDirty(obj);
	if (_bgNeedsRedraw)
		clearDrawObjectQueue();
}

void ScummEngine_v5::o5_setVarRange() {
	int a, b;

	getResultPos();
	a = fetchScriptByte();
	do {
		if (_opcode & 0x80)
			b = fetchScriptWordSigned();
		else
			b = fetchScriptByte();

		setResult(b);
		_resultVarNumber++;
	} while (--a);
}

void ScummEngine_v5::o5_startMusic() {
	if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
		// In FM-TOWNS games this is some kind of Audio CD status query function.
		// See also bug #927 (thanks to Hibernatus for providing the information).
		getResultPos();
		int b = getVarOrDirectByte(PARAM_1);
		int result = 0;
		switch (b) {
		case 0:
			result = _sound->pollCD() == 0;
			break;
		case 0xFC:
			// TODO: Unpause (resume) audio track. We'll have to extend Sound and OSystem for this.
			break;
		case 0xFD:
			// TODO: Pause audio track. We'll have to extend Sound and OSystem for this.
			break;
		case 0xFE:
			result = _sound->getCurrentCDSound();
			break;
		case 0xFF:
			result = _townsPlayer->getCurrentCdaVolume();
			break;
		default:
			// TODO: return track length in seconds. We'll have to extend Sound and OSystem for this.
			// To check scummvm returns the right track length you
			// can look at the global script #9 (0x888A in 49.LFL).
			break;
		}
		debugC(DEBUG_GENERAL,"o5_startMusic(%d)", b);
		setResult(result);
	} else {
		_sound->startSound(getVarOrDirectByte(PARAM_1));
	}
}

void ScummEngine_v5::o5_startSound() {
	const byte *oldaddr = _scriptPointer - 1;
	int sound = getVarOrDirectByte(PARAM_1);

	// WORKAROUND: There are times when Largo's theme is playing. Once it
	// has finished, the old music should resume. But the scripts don't
	// actually check that, they just wait for the scene to end. So it may
	// work fine, if the subtitles are timed correctly, but it may not.
	//
	// The Amiga version cut much of the music, so it shouldn't be needed
	// for that version.
	//
	// Sound 103 is Largo talking to the bartender.
	// Sound 110 is Largo talking to Mad Marty.

	if (_game.id == GID_MONKEY2 && _game.platform != Common::kPlatformAmiga && (sound == 103 || sound == 110) && _sound->isSoundRunning(151)) {
		debug(1, "Delaying music until Largo's theme has finished");
		_scriptPointer = oldaddr;
		o5_breakHere();
		return;
	}

	if (VAR_MUSIC_TIMER != 0xFF)
		VAR(VAR_MUSIC_TIMER) = 0;
	_sound->startSound(sound);
}

void ScummEngine_v5::o5_stopMusic() {
	_sound->stopAllSounds();
}

void ScummEngine_v5::o5_stopSound() {
	int sound = getVarOrDirectByte(PARAM_1);

	// WORKAROUND: Don't stop the background audio when showing the close-up
	// of captain Smirk. You are still outdoors, so it makes more sense if
	// they keep playing like they do in the Special Edition. (Though there
	// the background makes it more obvious.)
	//
	// The sound is stopped by the exit script, which always has number
	// 10001 regardless of which room it is. We figure out which one by
	// looking at which rooms we're moving between.

	if (_game.id == GID_MONKEY && (_game.features & GF_AUDIOTRACKS) && sound == 126 && vm.slot[_currentScript].number == 10001 && VAR(VAR_ROOM) == 43 && VAR(VAR_NEW_ROOM) == 76 && enhancementEnabled(kEnhAudioChanges)) {
		return;
	}

	// WORKAROUND: In MM NES, Wendy's CD player script forgets to update the
	// music status variable when you stop it. Wendy's music would then
	// resume when leaving some rooms (such as room 3 with the chandelier),
	// even though her CD player was off.
	if (_game.id == GID_MANIAC && _game.platform == Common::kPlatformNES && sound == 75 && vm.slot[_currentScript].number == 50 && VAR(VAR_EGO) == 6 && VAR(224) == sound && enhancementEnabled(kEnhAudioChanges)) {
		VAR(224) = 0;
	}

	_sound->stopSound(sound);
}

void ScummEngine_v5::o5_isSoundRunning() {
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(PARAM_1);
	if (snd)
		snd = _sound->isSoundRunning(snd);
	setResult(snd);
}

void ScummEngine_v5::o5_soundKludge() {
	int items[NUM_SCRIPT_LOCAL];
	int num = getWordVararg(items);
	_sound->soundKludge(items, num);
}

void ScummEngine_v5::o5_startObject() {
	int obj, script;
	int data[NUM_SCRIPT_LOCAL];

	obj = getVarOrDirectWord(PARAM_1);
	script = getVarOrDirectByte(PARAM_2);

	getWordVararg(data);
	runObjectScript(obj, script, 0, 0, data);
}

void ScummEngine_v5::o5_startScript() {
	int op, script;
	int data[NUM_SCRIPT_LOCAL];

	op = _opcode;
	script = getVarOrDirectByte(PARAM_1);

	getWordVararg(data);

	// WORKAROUND bug #13370: If you try to leave the plateau before
	// healing Rusty, his ghost will block the way. But this should not
	// happen during the cutscene where he first appears, because then he
	// will appear to teleport from one spot to another.
	//
	// In the VGA talkie version Rusty just appears in the rift, rather
	// than gliding in from off-stage. The only thing that's affected is
	// whether Bobbin or Rusty speaks first, and the dialog makes sense
	// either way.
	if (_game.id == GID_LOOM && _game.version == 3 && script == 207 && isScriptRunning(98) && enhancementEnabled(kEnhVisualChanges))
		return;

	// WORKAROUND bug #2198: Script 171 loads a complete room resource,
	// instead of the actual script, causing invalid opcode cases
	//
	// TODO: what does the original interpreter do in this case?
	if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns && script == 171 && enhancementEnabled(kEnhGameBreakingBugFixes))
		return;

	// WORKAROUND bug #5709 (also occurs in original): Some old versions of
	// Indy3 sometimes fail to allocate IQ points correctly. To quote:
	// "In the Amiga version you get the 15 points for puzzle 30 if you give the
	// book or KO the guy. The PC version correctly gives 10 points for puzzle
	// 29 for KO and 15 for puzzle 30 when giving the book."
	// This workaround is meant to address that.
	//
	// See also the similar ScummEngine_v4::o4_ifState() workaround.
	if (_game.id == GID_INDY3 && _currentScript != 0xFF && vm.slot[_currentScript].number == 106 && script == 125 && VAR(115) != 2 &&
		enhancementEnabled(kEnhMinorBugFixes)) {
		// If Var[115] != 2, then:
		// Correct: startScript(125,[29,10]);
		// Wrong : startScript(125,[30,15]);
		data[0] = 29;
		data[1] = 10;
	}

	// WORKAROUND: in Loom v3, if one uses the stealth draft on the
	// shepherds, their first reaction line ("We are the masters of
	// stealth") is missing a Local[0] value for the actor number. This
	// causes the line to be silently skipped (as in the original).
	if (_game.id == GID_LOOM && _game.version == 3 && _roomResource == 23 && script == 232 && data[0] == 0 && _currentScript != 0xFF &&
		vm.slot[_currentScript].number >= 422 && vm.slot[_currentScript].number <= 425 && enhancementEnabled(kEnhRestoredContent)) {
		// Restore the missing line by attaching it to the shepherd on which the
		// draft was used.
		data[0] = vm.slot[_currentScript].number % 10;

		// WORKAROUND: in some EGA releases, actor 3 may have been removed from
		// the current room, although he's still on screen (the EGA English 1.1
		// release fixed this), so no line can be attached to him. Forcing his
		// appearance or ignoring his removal doesn't fix this problem, for some
		// reason.  So, if we detect this, we default to actor 4 (since that's
		// what the Talkie version always used), for now.
		if (data[0] == 3 && isValidActor(3) && !_actors[3]->isInCurrentRoom())
			data[0] = 4;
	}

	// Method used by original games to skip copy protection scheme
	if (!_copyProtection) {
		// Copy protection was disabled in LucasArts Classic Adventures (PC Disk)
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformDOS && _game.version == 3 && _currentRoom == 69 && script == 201)
			script = 205;
		// Copy protection was disabled in KIXX XL release (Amiga Disk) and
		// in LucasArts Classic Adventures (PC Disk)
		if (_game.id == GID_MONKEY_VGA && script == 152)
			return;
		// Copy protection was disabled in LucasArts Mac CD Game Pack II (Macintosh CD)
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformMacintosh && script == 155)
			return;
	}

	runScript(script, (op & 0x20) != 0, (op & 0x40) != 0, data);

	// WORKAROUND: Indy3 does not save the series IQ automatically after changing it.
	// Save on IQ increment (= script 125 was executed).
	if (_game.id == GID_INDY3 && script == 125)
		((ScummEngine_v4 *)this)->updateIQPoints();
}

void ScummEngine_v5::o5_stopObjectCode() {
	stopObjectCode();
}

void ScummEngine_v5::o5_stopObjectScript() {
	stopObjectScript(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v5::o5_stopScript() {
	const byte *oldaddr = _scriptPointer - 1;
	int script;

	script = getVarOrDirectByte(PARAM_1);

	if (_game.id == GID_INDY4 && script == 164 && _roomResource == 50 && _currentScript != 0xFF &&
		vm.slot[_currentScript].number == 213 && VAR(VAR_HAVE_MSG) &&
		getOwner(933) == VAR(VAR_EGO) && getClass(933, 146) && enhancementEnabled(kEnhRestoredContent)) {
		// WORKAROUND bug #2215: Due to a script bug, a line of text is skipped
		// which Indy is supposed to speak when he finds Orichalcum in some old
		// bones in the caves below Crete, if (and only if) he has already put
		// some beads in the gold box beforehand. Also happens in DREAMM.
		_scriptPointer = oldaddr;
		o5_breakHere();
		return;
	}

	if (!script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v5::o5_stringOps() {
	int a, b, c, i;
	byte *ptr;

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* loadstring */
		loadPtrToResource(rtString, getVarOrDirectByte(PARAM_1), nullptr);
		break;

	case 2:											/* copystring */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		assert(a != b);
		_res->nukeResource(rtString, a);
		ptr = getResourceAddress(rtString, b);
		if (ptr)
			loadPtrToResource(rtString, a, ptr);
		break;

	case 3:											/* set string char */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		c = getVarOrDirectByte(PARAM_3);
		ptr = getResourceAddress(rtString, a);
		if (ptr == nullptr)
			error("String %d does not exist", a);
		ptr[b] = c;
		break;

	case 4:											/* get string char */
		getResultPos();
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		ptr = getResourceAddress(rtString, a);
		if (ptr == nullptr)
			error("String %d does not exist", a);
		setResult(ptr[b]);
		break;

	case 5:											/* create empty string */
		a = getVarOrDirectByte(PARAM_1);
		b = getVarOrDirectByte(PARAM_2);
		_res->nukeResource(rtString, a);
		if (b) {
			ptr = _res->createResource(rtString, a, b);
			if (ptr) {
				for (i = 0; i < b; i++)
					ptr[i] = 0;
			}
		}
		break;

	default:
		break;
	}
}

void ScummEngine_v5::o5_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	setResult(readVar(_resultVarNumber) - a);
}

void ScummEngine_v5::o5_verbOps() {
	int verb, slot;
	VerbSlot *vs;
	int a, b;
	byte *ptr;

	verb = getVarOrDirectByte(PARAM_1);

	slot = getVerbSlot(verb, 0);
	assertRange(0, slot, _numVerbs - 1, "new verb slot");

	vs = &_verbs[slot];
	vs->verbid = verb;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:		// SO_VERB_IMAGE
			a = getVarOrDirectWord(PARAM_1);
			if (slot) {
				setVerbObject(_roomResource, a, slot);
				vs->type = kImageVerbType;
			}
			break;
		case 2:		// SO_VERB_NAME
			loadPtrToResource(rtVerb, slot, nullptr);
			if (slot == 0)
				_res->nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 3:		// SO_VERB_COLOR
			vs->color = getVarOrDirectByte(PARAM_1);
			break;
		case 4:		// SO_VERB_HICOLOR
			vs->hicolor = getVarOrDirectByte(PARAM_1);
			break;
		case 5:		// SO_VERB_AT
			vs->curRect.left = getVarOrDirectWord(PARAM_1);
			vs->curRect.top = getVarOrDirectWord(PARAM_2) + _screenDrawOffset;
			if (_game.platform == Common::kPlatformFMTowns && ConfMan.getBool("trim_fmtowns_to_200_pixels")) {
				if (_game.id == GID_ZAK && verb == 116)
					// WORKAROUND: FM-TOWNS Zak used the extra 40 pixels at the bottom to increase the inventory to 10 items
					// if we trim to 200 pixels, we need to move the 'down arrow' (verb 116) to higher location
					vs->curRect.top -= 18;
			}
			vs->origLeft = vs->curRect.left;
			break;
		case 6:		// SO_VERB_ON
			vs->curmode = 1;
			break;
		case 7:		// SO_VERB_OFF
			vs->curmode = 0;
			break;
		case 8:		// SO_VERB_DELETE
			killVerb(slot);
			break;
		case 9:		// SO_VERB_NEW
			slot = getVerbSlot(verb, 0);

			if (_game.platform == Common::kPlatformFMTowns && _game.version == 3 && slot)
				continue;

			if (slot == 0) {
				for (slot = 1; slot < _numVerbs; slot++) {
					if (_verbs[slot].verbid == 0)
						break;
				}
				if (slot == _numVerbs)
					error("Too many verbs");
			}
			vs = &_verbs[slot];
			vs->verbid = verb;
			vs->color = 2;
			vs->hicolor = (_game.version == 3) ? 14 : 0;
			vs->dimcolor = 8;
			vs->type = kTextVerbType;
			vs->charset_nr = _string[0]._default.charset;
			vs->curmode = 0;
			vs->saveid = 0;
			vs->key = 0;
			vs->center = 0;
			vs->imgindex = 0;
			break;

		case 16:	// SO_VERB_DIMCOLOR
			vs->dimcolor = getVarOrDirectByte(PARAM_1);
			break;
		case 17:	// SO_VERB_DIM
			vs->curmode = 2;
			break;
		case 18:	// SO_VERB_KEY
			vs->key = getVarOrDirectByte(PARAM_1);
			break;
		case 19:	// SO_VERB_CENTER
			vs->center = 1;
			break;
		case 20:	// SO_VERB_NAME_STR
			ptr = getResourceAddress(rtString, getVarOrDirectWord(PARAM_1));

			if (!ptr)
				_res->nukeResource(rtVerb, slot);
			else {
				loadPtrToResource(rtVerb, slot, ptr);
			}
			if (slot == 0)
				_res->nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 22:										/* assign object */
			a = getVarOrDirectWord(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);
			if (slot && vs->imgindex != a) {
				setVerbObject(b, a, slot);
				vs->type = kImageVerbType;
				vs->imgindex = a;
			}
			break;
		case 23:										/* set back color */
			vs->bkcolor = getVarOrDirectByte(PARAM_1);
			break;
		default:
			error("o5_verbOps: unknown subopcode %d", _opcode & 0x1F);
		}
	}

	// Force redraw of the modified verb slot
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void ScummEngine_v5::o5_wait() {
	const byte *oldaddr = _scriptPointer - 1;

	if ((_game.id == GID_INDY3) && !(_game.platform == Common::kPlatformMacintosh)) {
		_opcode = 2;
	} else
		_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:		// SO_WAIT_FOR_ACTOR
		{
			Actor *a = derefActorSafe(getVarOrDirectByte(PARAM_1), "o5_wait");
			if (a && a->_moving)
				break;
			return;
		}
	case 2:		// SO_WAIT_FOR_MESSAGE
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 3:		// SO_WAIT_FOR_CAMERA
		if (camera._cur.x / 8 != camera._dest.x / 8)
			break;
		return;
	case 4:		// SO_WAIT_FOR_SENTENCE
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
		} else if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	default:
		error("o5_wait: unknown subopcode %d", _opcode & 0x1F);
		return;
	}

	_scriptPointer = oldaddr;
	o5_breakHere();
}

void ScummEngine_v5::o5_walkActorTo() {
	int x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_walkActorTo");
	x = getVarOrDirectWord(PARAM_2);
	y = getVarOrDirectWord(PARAM_3);

	// WORKAROUND: In MI1 CD, when the storekeeper comes back from outside,
	// he will close the door *after* going to his counter, which looks very
	// strange, since he's then quite far away from the door. Force calling
	// the script which closes the door *before* he starts walking away from
	// it, as in the other releases. Another v5 bug fixed on SegaCD, though!
	if (_game.id == GID_MONKEY && !(_game.features & GF_ULTIMATE_TALKIE) && _game.platform != Common::kPlatformSegaCD &&
		_currentRoom == 30 && _currentScript != 0xFF && vm.slot[_currentScript].number == 207 && a->_number == 11 &&
		x == 232 && y == 141 && enhancementEnabled(kEnhVisualChanges)) {
		if (whereIsObject(387) == WIO_ROOM && getState(387) == 1 && getState(437) == 1) {
			int args[NUM_SCRIPT_LOCAL];
			memset(args, 0, sizeof(args));
			args[0] = 387;
			args[1] = 437;
			runScript(26, 0, 0, args);
		}
	}

	// WORKAROUND: In Indy4, in the Crete room where there's the gold box,
	// Indy can get stuck if one clicks too quickly on the right and the
	// "elevator" isn't there yet (also with the original interpreter).
	// This is because the box matrix is initialized too late in the entry
	// script for that room, so we have to do it a bit earlier.
	//
	// Intentionally using `kEnhGameBreakingBugFixes`, since you can get
	// completely stuck.
	if (_game.id == GID_INDY4 && _currentScript != 0xFF && vm.slot[_currentScript].number == 10002 &&
		_currentRoom == (_game.platform == Common::kPlatformAmiga ? 58 : 60) &&
		VAR(224) == 140 && a->_number == VAR(VAR_EGO) && x == 45 && y == 137 &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		// If the elevator isn't on the current floor yet...
		if (whereIsObject(829) == WIO_ROOM && getState(829) == 0 && getBoxFlags(7) != 128) {
			// ...immediately set its box flags so that you can't walk on it
			setBoxFlags(7, 128);
			for (int i = 12; i <= 15; ++i)
				setBoxFlags(i, 128);
			createBoxMatrix();
		}
	}

	a->startWalkActor(x, y, -1);
}

void ScummEngine_v5::walkActorToActor(int actor, int toActor, int dist) {
	Actor *a = derefActor(actor, "walkActorToActor");
	Actor *to = derefActor(toActor, "walkActorToActor(2)");

	if (_game.version <= 2) {
		dist *= V12_X_MULTIPLIER;
	} else if (dist == 0xFF) {
		dist = a->_scalex * a->_width / 0xFF;
		dist += (to->_scalex * to->_width / 0xFF) / 2;
	}
	int x = to->getPos().x;
	int y = to->getPos().y;
	if (x < a->getPos().x)
		x += dist;
	else
		x -= dist;

	if (_game.version <= 2) {
		x /= V12_X_MULTIPLIER;
		y /= V12_Y_MULTIPLIER;
	}
	if (_game.version <= 3) {
		AdjustBoxResult abr = a->adjustXYToBeInBox(x, y);
		x = abr.x;
		y = abr.y;
	}
	a->startWalkActor(x, y, -1);
}

void ScummEngine_v5::o5_walkActorToActor() {
	Actor *a, *a2;
	int nr = getVarOrDirectByte(PARAM_1);
	int nr2 = getVarOrDirectByte(PARAM_2);
	int dist = fetchScriptByte();

	// We put a guard here, in case the scripts end up giving us
	// an invalid actor id (and FOA does that quite a lot)...
	if (!isValidActor(nr))
		return;

	a = derefActor(nr, "o5_walkActorToActor");
	if (!a->isInCurrentRoom())
		return;

	// Same as before...
	if (!isValidActor(nr2))
		return;

	a2 = derefActor(nr2, "o5_walkActorToActor(2)");
	if (!a2->isInCurrentRoom())
		return;

	walkActorToActor(nr, nr2, dist);
}

void ScummEngine_v5::o5_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o5_walkActorToObject");
	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

int ScummEngine_v5::getWordVararg(int *ptr) {
	int i;

	for (i = 0; i < NUM_SCRIPT_LOCAL; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(PARAM_1);
	}
	return i;
}

void ScummEngine_v5::decodeParseString() {
	int textSlot;
	int color;

	switch (_actorToPrintStrFor) {
	case 252:
		textSlot = 3;
		break;
	case 253:
		textSlot = 2;
		break;
	case 254:
		textSlot = 1;
		break;
	default:
		textSlot = 0;
	}

	_string[textSlot].loadDefault();

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0xF) {
		case 0:		// SO_AT
			_string[textSlot].xpos = getVarOrDirectWord(PARAM_1);
			_string[textSlot].ypos = getVarOrDirectWord(PARAM_2);
			_string[textSlot].overhead = false;
			break;
		case 1:		// SO_COLOR
			color = getVarOrDirectByte(PARAM_1);

			// HACK: The Indy 3 credits script asks for white text
			// with a shadow, but in a Mac emulator the text is
			// drawn in light gray with a shadow instead. Very
			// strange.
			if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh && _currentScript != 0xFF && vm.slot [_currentScript].number == 134 && color == 0x8F)
				color = 0x87;

			// WORKAROUND: In the CD version of MI1, the text in
			// the sign about the dogs only sleeping has the wrong
			// color. We can't find an exact match to what the
			// floppy version used, but we pick on that's as close
			// as we can get.
			//
			// The SEGA CD version uses the old colors already, and
			// the FM Towns version makes the text more readable by
			// giving it a black outline. The Macintosh release only
			// requires changing the color of the background, not the
			// text.
			//
			// See also the related Gdi::drawStrip() workaround.

			else if (_game.id == GID_MONKEY &&
					!(_game.features & GF_ULTIMATE_TALKIE) &&
					_game.platform != Common::kPlatformSegaCD &&
					_game.platform != Common::kPlatformFMTowns &&
					_game.platform != Common::kPlatformMacintosh &&
					_currentRoom == 36 &&
					vm.slot[_currentScript].number == 201 &&
					color == 2 &&
					enhancementEnabled(kEnhVisualChanges)) {
				color = findClosestPaletteColor(_currentPalette, 256, 0, 171, 0);
			}

			_string[textSlot].color = color;
			break;
		case 2:		// SO_CLIPPED
			_string[textSlot].right = getVarOrDirectWord(PARAM_1);
			break;
		case 3:		// SO_ERASE
			{
			int w = getVarOrDirectWord(PARAM_1);
			int h = getVarOrDirectWord(PARAM_2);
			// restoreCharsetBg(xpos, xpos + w, ypos, ypos + h)
			error("ScummEngine_v5::decodeParseString: Unhandled case 3: %d, %d", w, h);
			}
			break;
		case 4:		// SO_CENTER
			_string[textSlot].center = true;
			_string[textSlot].overhead = false;
			break;
		case 6:		// SO_LEFT
			if (_game.version == 3) {
				_string[textSlot].height = getVarOrDirectWord(PARAM_1);
			} else {
				_string[textSlot].center = false;
				_string[textSlot].overhead = false;
			}
			break;
		case 7:		// SO_OVERHEAD
			_string[textSlot].overhead = true;
			break;
		case 8:{	// SO_SAY_VOICE
				int offset = (uint16)getVarOrDirectWord(PARAM_1);
				int delay = (uint16)getVarOrDirectWord(PARAM_2);

				if (_game.id == GID_LOOM && _game.version == 4) {
					if (offset == 0 && delay == 0) {
						VAR(VAR_MUSIC_TIMER) = 0;
						_sound->stopCD();
					} else {
						// Loom specified the offset from the start of the CD;
						// thus we have to subtract the length of the first track
						// (22500 frames) plus the 2 second = 150 frame leadin.
						// I.e. in total 22650 frames.
						offset = (int)(offset * 7.5 - 22500 - 2*75);

						// Add the user-specified adjustment.
						if (ConfMan.hasKey("loom_playback_adjustment")) {
							int adjustment = ConfMan.getInt("loom_playback_adjustment");
							offset += ((75 * adjustment) / 100);
							if (offset < 0)
								offset = 0;
						}

						// Slightly increase the delay (5 frames = 1/25 of a second).
						// This noticeably improves the experience in Loom CD.
						delay = (int)(delay * 7.5 + 5);

						_sound->playCDTrack(1, 1, offset, delay);
					}
				} else {
					error("ScummEngine_v5::decodeParseString: Unhandled case 8");
				}
			}
			break;
		case 15:	// SO_TEXTSTRING
			decodeParseStringTextString(textSlot);
			return;
		default:
			error("ScummEngine_v5::decodeParseString: Unhandled case %d", _opcode & 0xF);
		}
	}

	_string[textSlot].saveDefault();
}

void ScummEngine_v5::decodeParseStringTextString(int textSlot) {
	const int len = resStrLen(_scriptPointer);

	if (_game.id == GID_LOOM && _game.version == 4 && _language == Common::EN_ANY &&
		_currentScript != 0xFF && vm.slot[_currentScript].number == 95 && enhancementEnabled(kEnhTextLocFixes) &&
			strcmp((const char *)_scriptPointer, "I am Choas.") == 0) {
		// WORKAROUND: This happens when Chaos introduces
		// herself to bishop Mandible. Of all the places to put
		// a typo...
		printString(textSlot, (const byte *)"I am Chaos.");
	} else if (_game.id == GID_LOOM && _game.version == 4 && _roomResource == 90 &&
		_currentScript != 0xFF && vm.slot[_currentScript].number == 203 && _string[textSlot].color == 0x0F && enhancementEnabled(kEnhSubFmtCntChanges)) {
		// WORKAROUND: When Mandible speaks with Goodmold, his second
		// speech line is missing its color parameter.
		_string[textSlot].color = 0x0A;
		printString(textSlot, _scriptPointer);
	} else if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns && _roomResource == 80 &&
		_currentScript != 0xFF && vm.slot[_currentScript].number == 201 && enhancementEnabled(kEnhSubFmtCntChanges)) {
		// WORKAROUND: When Indy and his father escape the zeppelin
		// with the biplane in the FM-TOWNS version, they share the
		// same text color. Indeed, they're not given any explicit
		// color, but for some reason this is only a problem on the
		// FM-TOWNS. In order to determine who's who, we look for a
		// `\xFF\x03` wait instruction or the `Junior` word, since
		// only Henry Sr. uses them in this script.
		if (strstr((const char *)_scriptPointer, "\xFF\x03") || strstr((const char *)_scriptPointer, "Junior"))
			_string[textSlot].color = 0x0A;
		else
			_string[textSlot].color = 0x0E;
		printString(textSlot, _scriptPointer);
	} else if (_game.id == GID_INDY4 && _roomResource == 23 && _currentScript != 0xFF && vm.slot[_currentScript].number == 167 &&
		len == 24 && enhancementEnabled(kEnhTextLocFixes) && memcmp(_scriptPointer+16, "pregod", 6) == 0) {
		// WORKAROUND for bug #2961: At the end of Indy4, if Ubermann is told
		// to use 20 orichalcum beads, he'll count "pregod8" and "pregod9"
		// instead of "18" and "19", in some releases.
		//
		// TODO: Check whether this issue also appears in any floppy version,
		// because the current workaround doesn't look compatible with
		// non-talkie releases.
		byte tmpBuf[25];
		memcpy(tmpBuf, _scriptPointer, 25);
		if (tmpBuf[22] == '8')
			Common::strlcpy((char *)tmpBuf+16, "^18^", sizeof(tmpBuf) - 16);
		else
			Common::strlcpy((char *)tmpBuf+16, "^19^", sizeof(tmpBuf) - 16);
		printString(textSlot, tmpBuf);
	} else if (_game.id == GID_INDY4 && _language == Common::EN_ANY && _roomResource == 10 &&
		_currentScript != 0xFF && vm.slot[_currentScript].number == 209 && _actorToPrintStrFor == 4 && len == 81 &&
			strcmp(_game.variant, "Floppy") != 0 && enhancementEnabled(kEnhSubFmtCntChanges)) {
		// WORKAROUND: The English Talkie version of Indy4 changed Kerner's
		// lines when he uses the phone booth in New York, but the text doesn't
		// match the voice and it mentions the wrong person, in most releases.
		// The fixed string is taken from the 1994 Macintosh release.
		const char origText[] = "Fritz^ Fantastic\x10news!\xFF\x03I think we've found the treasure we\x10seek.";
		const char newText[] = "Dr. Ubermann^ Fantastic\x10news!\xFF\x03We've found the treasure we\x10seek.";
		if (strcmp((const char *)_scriptPointer + 16, origText) == 0) {
			byte *tmpBuf = new byte[sizeof(newText) + 16];
			memcpy(tmpBuf, _scriptPointer, 16);
			memcpy(tmpBuf + 16, newText, sizeof(newText));
			printString(textSlot, tmpBuf);
			delete[] tmpBuf;
		} else {
			printString(textSlot, _scriptPointer);
		}
	} else if (_game.id == GID_INDY4 && _currentScript != 0xFF && vm.slot[_currentScript].number == 161 && _actorToPrintStrFor == 2 &&
		_game.platform != Common::kPlatformAmiga && strcmp(_game.variant, "Floppy") != 0 &&
			enhancementEnabled(kEnhAudioChanges)) {
		// WORKAROUND: In Indy 4, if one plays as Sophia and looks at Indy, then
		// her "There's nothing to look at." reaction line will be said with
		// Indy's voice, because script 68-161 doesn't check for Sophia in this
		// case. Script 68-4 has a "There's nothing to look at." line for Sophia,
		// though, so we reuse this if the current line contains the expected
		// audio offset.
		if (memcmp(_scriptPointer, "\xFF\x0A\x5D\x8E\xFF\x0A\x63\x08\xFF\x0A\x0E\x00\xFF\x0A\x00\x00", 16) == 0 && len >= 16) {
			byte *tmpBuf = new byte[len];
			memcpy(tmpBuf, "\xFF\x0A\xCE\x3B\xFF\x0A\x01\x05\xFF\x0A\x0E\x00\xFF\x0A\x00\x00", 16);
			memcpy(tmpBuf + 16, _scriptPointer + 16, len - 16);
			printString(textSlot, tmpBuf);
			delete[] tmpBuf;
		} else {
			printString(textSlot, _scriptPointer);
		}
	} else if (_game.id == GID_MONKEY_EGA && _roomResource == 30 && _currentScript != 0xFF && vm.slot[_currentScript].number == 411 &&
		enhancementEnabled(kEnhTextLocFixes) &&
		strstr((const char *)_scriptPointer, "NCREDIT-NOTE-AMOUNT")) {
		// WORKAROUND for bug #4886 (MI1EGA German: Credit text incorrect)
		// The script contains buggy text.
		const char *tmp = strstr((const char *)_scriptPointer, "NCREDIT-NOTE-AMOUNT");
		char tmpBuf[256];
		const int diff = tmp - (const char *)_scriptPointer;
		memcpy(tmpBuf, _scriptPointer, diff);
		Common::strlcpy(tmpBuf + diff, "5000", sizeof(tmpBuf) - diff);
		Common::strlcpy(tmpBuf + diff + 4, tmp + sizeof("NCREDIT-NOTE-AMOUNT") - 1, sizeof(tmpBuf) - diff - 4);
		printString(textSlot, (byte *)tmpBuf);
	} else if (_game.id == GID_MONKEY && !(_game.features & GF_ULTIMATE_TALKIE) &&
		_game.platform != Common::kPlatformSegaCD &&
			_currentScript != 0xFF && ((_roomResource == 78 && vm.slot[_currentScript].number == 201) ||
			(_roomResource == 45 && vm.slot[_currentScript].number == 200 &&
			isValidActor(10) && _actors[10]->isInCurrentRoom())) &&
			_actorToPrintStrFor == 255 && _string[textSlot].color != 0x0F &&
			enhancementEnabled(kEnhSubFmtCntChanges)) {
		// WORKAROUND: When Guybrush goes to the church at the end of Monkey1,
		// the color for the ghost priest's lines is inconsistent in the v5
		// releases (except for the SegaCD one with the smaller palette).
		// Fix this while making sure that it doesn't apply to Elaine saying
		// "I heard that!" offscreen.
		_string[textSlot].color = (_game.platform == Common::kPlatformFMTowns) ? 0x0A : 0xF9;
		printString(textSlot, _scriptPointer);
	} else if (_game.id == GID_MONKEY && !(_game.features & GF_ULTIMATE_TALKIE) &&
			_game.platform != Common::kPlatformSegaCD && _currentScript != 0xFF &&
			(vm.slot[_currentScript].number == 140 || vm.slot[_currentScript].number == 294) &&
			_actorToPrintStrFor == 255 && _string[textSlot].color == 0x06 &&
			enhancementEnabled(kEnhSubFmtCntChanges)) {
		// WORKAROUND: In MI1 CD, the colors when the navigator head speaks are
		// not the intended ones (dark purple instead of brown), because the
		// original `Color(6)` parameter was kept without adjusting it for the
		// v5 palette changes (a common oversight in that version). The verb
		// options may also look wrong in that scene, but we don't fix that, as
		// this font in displayed in green, white or purple between the
		// different releases and scenes, so we don't know the original intent.
		_string[textSlot].color = (_game.platform == Common::kPlatformFMTowns) ? 0x0C : 0xEA;
		printString(textSlot, _scriptPointer);
	} else if (_game.id == GID_MONKEY && _roomResource == 25 && _currentScript != 0xFF && vm.slot[_currentScript].number == 205) {
		printPatchedMI1CannibalString(textSlot, _scriptPointer);
	} else {
		printString(textSlot, _scriptPointer);
	}

	_scriptPointer += len + 1;


	// In SCUMM V1-V3, there were no 'default' values for the text slot
	// values. Hence to achieve correct behavior, we have to keep the
	// 'default' values in sync with the active values.
	//
	// Note: This is needed for Indy3 (Grail Diary). It's also needed
	// for Loom, or the lines Bobbin speaks during the intro are put
	// at position 0,0.
	//
	// Note: We can't use saveDefault() here because we only want to
	// save the position and color. In particular, we do not want to
	// save the 'center' flag. See bug #1588.
	if (_game.version <= 3) {
		_string[textSlot]._default.xpos = _string[textSlot].xpos;
		_string[textSlot]._default.ypos = _string[textSlot].ypos;
		_string[textSlot]._default.height = _string[textSlot].height;
		_string[textSlot]._default.color = _string[textSlot].color;
	}
}

void ScummEngine_v5::printPatchedMI1CannibalString(int textSlot, const byte *ptr) {
	const char *msg = (const char *)ptr;

	if (strncmp((const char *)ptr, "/LH.ENG/", 8) == 0) {
		msg =
			"Oooh, that's nice.\xFF\x03"
			"Simple.  Just like one of mine.\xFF\x03"
			"And little.  Like mine.";
	} else if (strncmp((const char *)ptr, "/LH.DEU/", 8) == 0) {
		msg =
			"Oooh, das ist nett.\xFF\x03"
			"Einfach.  Wie eines von meinen.\xFF\x03"
			"Und klein.  Wie meine.";
	} else if (strncmp((const char *)ptr, "/LH.ITA/", 8) == 0) {
		msg =
			"Oooh, che bello.\xFF\x03"
			"Semplice.  Proprio come uno dei miei.\xFF\x03"
			"E piccolo.  Come il mio.";
	} else if (strncmp((const char *)ptr, "/LH.ESP/", 8) == 0) {
		msg =
			"Oooh, qu\x82 bonito.\xFF\x03"
			"Simple.  Como uno de los m\xA1os.\xFF\x03"
			"Y peque\xA4o, como los m\xA1os.";
	}

	printString(textSlot, (const byte *)msg);
}

} // End of namespace Scumm
