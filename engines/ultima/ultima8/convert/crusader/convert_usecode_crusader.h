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

#ifndef ULTIMA8_CONVERT_U8_CONVERTUSECODECRUSADER_H
#define ULTIMA8_CONVERT_U8_CONVERTUSECODECRUSADER_H

#include "ultima/ultima8/convert/convert_usecode.h"

namespace Ultima {
namespace Ultima8 {

class ConvertUsecodeCrusader : public ConvertUsecode {
public:
	const char* const *intrinsics() override  { return _intrinsics;  };
	const char* const *event_names() override { return _event_names; };

	static const char* const _event_names[];
private:
	static const char* const _intrinsics[512];
};

// By convention, last pushed argument goes first on the list.
const char* const ConvertUsecodeCrusader::_intrinsics[] = {
	// 0000
	"byte World::I_getAlertActive(void)",
	"int16 Item::I_getFrame(Item *)",
	"void Item::I_setFrame(Item *, frame)",
	"int16 Item::I_getMapArray(Item *)", // See TRIGGER::ordinal21 - stored in a variable 'mapNum'
	"int16 Item::I_getStatus(Item *)",
	"void Item::I_orStatus(Item *, uint16 flags)",
	"int16 Item::I_equip(6 bytes)", // same coff as 0B5
	"byte Item::I_isOnScreen(Item *)", // called for gattling guns and camera
	"byte Actor::I_isNPC(Item *)", // proably - actually checks is itemno < 256?
	"byte Item::I_getZ(Item *)",
	"void Item::I_destroy(Item *)", // probably? often called after creating a replacement object and setting it to the same position (eg, LUGGAGE::gotHit)
	"int16 Actor::I_GetNPCDataField0x63_00B(Actor *)", // Some unknown value set for NPCs based on Q of egg.
	"void Ultima8Engine::I_setAvatarInStasis(int)",
	"byte Item::I_getDirToItem(Item *, itemno)", // based on disasm
	"int16 Actor::I_turnToward(Actor *, direction, dir_16)",
	"void I_playFlic(void), int16 I_playFlic(Item *, char *name, int16 sizex, int16 sizey)",
	// 0010
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff as 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Actor::I_getMap(4 bytes)", // based on disasm.
	"void MusicProcess:I_playMusic(int trackno)",
	"int16 Item::I_getX(Item *)",
	"int16 Item::I_getY(Item *)",
	"void AudioProcess::I_playSFXCru(Item *, uint16 sfxnum)",
	"int16 Item::I_getShape(Item *)", // in STEAMBOX::func0A, is compared to 0x511 (the STEAM2 shape number) to determine direction
	"void Item::I_explode(Item *, exptype, destroy_item)",
	"int16 UCMachine::I_rndRange(uint16 x, uint16 y)", // // probably.. always called with 2 constants, then result often compared to some number between
	"byte Item::I_legalCreateAtCoords(Item *, int16 shapeno, int16 frame, int16 x, int16 y, int16 z)", // probably, see usage in DOOR2::ordinal37
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132. Always associated with a bitwise-not or bitmask
	"int16 World::I_getControlledNPCNum()",
	"byte Actor::I_getDir(4 bytes)", // based on disasm. same coff as 112, 121
	"int16 Actor::I_getLastAnimSet(4 bytes)", // based on disasm. part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"int16 Item::I_fireWeapon(Item *, x, y, z, byte, int, byte)",
	"byte Item::I_create(Item *, uint16 shapenum, uint16 framenum)", // probably - used in MISS1EGG referencing keycards and NPCDEATH in creating blood spills
	// 0020
	"void Item::I_popToCoords(Item *, uint16 x, uint16 y, uint16 z)", // set coords, used after creating blood spills in NPCDEATH
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Item::I_push(Item *)", // same code as U8
	"int16 Item::I_getEtherealTop(void)", // based on disasm
	"void Item::I_setShape(Item *, int16 shapeno)", // probably. See PEPSIEW::gotHit.
	"void Item::I_touch(Item *)", // same code as U8
	"int16 Item::I_getQHi(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"int16 I_getClosestDirectionInRange(x1, y1, x2, y2, numdirs, mindir, maxdir)",
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 World::I_gameDifficulty(void)",
	"void AudioProcess::I_playAmbientSFXCru(Item *, sndno)",
	"int16 Item::I_getQLo(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"byte Item::I_inFastArea(Item *)",
	"void Item::I_setQHi(Item *, uint16 qhi)", // probably setQHi, see usage in FREE::ordinal2E where object position is copied
	"byte Item::I_legalMoveToPoint(Item *, Point *, int16 force)", // based on disasm
	"byte CurrentMap::I_canExistAtPoint(int, int, shapeno, Point *)",
	// 0030
	"void Item::I_pop(Item *)", // same code as U8
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Item::I_receiveHit(Item *, other, dir, damage, damagetype)", // based on disasm
	"byte Actor::I_isBusy(Actor *)", // same code as U8
	"int16 Item::I_getDirFromTo16(x1, y1, x2, y2)",
	"byte Actor::I_isKneeling(Actor *)",
	"int16 Actor::I_doAnim(12 bytes)", // v. similar code to U8
	"byte MainActor::I_addItemCru(4 bytes)", // same coff as 0B8
	"void AudioProcess::I_stopSFXCru(Item *, int16 sndno)",
	"byte Actor::I_isDead(Item *)", // same coff as 122, 12E
	"byte AudioProcess::I_isSFXPlayingForObject(Item *, int16 unk)",
	"void Item::I_setQLo(Item *, int16 qlo)", // probably setQLo, see usage in FREE::ordinal2E where object position is copied.  Disassembly confirms.
	"int16 Item::I_getItemFamily(Item *)", // based on disasm
	"void Container::I_destroyContents(Item *)",
	"void Item::I_fallProbably_03E(Item *)", // similar disasm to U8, but not totally the same.
	"int16 Egg::I_getEggId(Item *)", // from disasm
	// 0040
	"void CameraProcess::I_moveTo(x, y, z)",
	"void CameraProcess::I_setCenterOn(objid)",
	"byte Item::I_getRangeIfVisible(Item *, otheritem)",
	"void AudioProcess::I_playSFXCru(Item *, soundno)", // TODO: Work out how this is different from Int015 - to a first approximation they are quite similar.
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno))", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 0050
	"int16 Actor::I_getCurrentActivityNo(Actor *)",
	"void Actor::I_clrInCombat(Actor *)", // probably, based on disasm.
	"void Actor::I_setDefaultActivity0(Actor *, int)",
	"void Actor::I_setDefaultActivity1(Actor *, int)",
	"void Actor::I_setDefaultActivity2(Actor *, int)",
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void World::I_setControlledNPCNum(int itemno)", // when you take over the Thermatron etc.
	"int16 Item::I_getSurfaceWeight(Item *)",
	"byte Item::I_isCentreOn(Item *, uint16 other)",
	"void Item::I_setFrame(Item *, frame)", // based on same coff as 002
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"byte Item::I_legalCreateAtPoint(Item *, int16 shape, int16 frame, Point *)", // see PEPSIEW::use
	"void Item::I_getPoint(Item *, Point *)",
	"void StatusGump::I_hideStatusGump(void)", // Probably hides gumps at the bottom, among other things.
	"int16 MovieGump::I_playMovieOverlay(uint32, char *, int16 a, int16 b)", // Play video (as texture? parameters like (150, 250, "MVA11A") and other mvas)
	"void StatusGump::I_showStatusGump(void)",  // Probably shows gumps at the bottom, among other things.
	// 0060
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Actor::I_create(8 bytes)",
	"void CameraProcess::I_somethingAboutCameraUpdate(void)",
	"void Actor::I_teleport(12 bytes)", // based on disasm same as U8
	"void Item::I_getFootpad(Item *, uint *, uint *, uint *)", // based on disasm. same coff as 12D
	"byte Item::I_isInNPC(Item *)", // based on disasm - following parent containers, is one of them an NPC
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_setNpcNum(Item *, uint16 npcnum)", // maybe, see EVENT::func0A or VALUEBOX::ordinal20.. right next to getNPCNum in coff (confirmed by disassembly)
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Item::I_move(Item *, int16 x, int16 y, uint16 z)",
	"int16 Game::I_isViolenceEnabled(void)",
	"void Kernel::I_resetRef(int16, int16)", // same disasm as U8
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Item::I_isCompletelyOn(Item *, uint16 other)",
	// 0070
	"byte Ultima8Engine::I_getUnkCrusaderFlag(void)",
	"void Ultima8Engine::I_setUnkCrusaderFlag(void)",
	"void Ultima8Engine::I_setCruStasis(void)",
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Ultima8Engine::I_clrUnkCrusaderFlag(void)",
	"void Ultima8Engine::I_clrCruStasis(void)",
	"void AudioProcess::I_stopSFX(Item *)",
	"int16 PaletteFaderProcess::I_fadeToBlack(void)", // fade to black, no args (40 frames)
	"void MainActor::I_clrKeycards(void)",
	"int16 MainActor::I_teleportToEgg(int, int, int)",
	"int16 PaletteFaderProcess::I_fadeFromBlack(void)", // from black, no arg (40 frames)
	"void Actor::I_clrImmortal(Actor *)", // same coff as 130
	"int16 Actor::I_getHp(Actor *)",
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Item::I_getQuality(Item *)", // based on disassembly
	"void Item::I_setQuality(Item *, int)", // based on disassembly. same coff as 0BA, 125
	// 0080
	"int16 Item::I_use(Item *)", // same coff as 0D0, 0D5
	"int16 MainActor::I_getMaxEnergy(Actor *)",
	"int16 Actor::I_getMana(Actor *)",
	"void Actor::I_setMana(Actor *, int)",
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void Actor::I_setImmortal(Actor *)",
	"int16 CameraProcess::I_getCameraX(void)",
	"int16 CameraProcess::I_getCameraY(void)",
	"void Item::I_setMapArray(Item *, uint16 maparray)", // based on decompile - sets same value as read by getmaparray .. see VALUEBOX:ordinal20
	"int16 Item::I_getNpcNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Intrinsic08A(12 bytes)", // TODO: No idea here.. something about hurling? look at the usecode.
	"int16 Item::I_enterFastArea(Item *)", // based on disasm, v similar to U8
	"void Item::I_setIsBroken(Item *)", // same coff as 119, 12A
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void PaletteFaderProcess::I_jumpToAllBlack(void)",
	// 0090
	"void MusicProcess::I_stopMusic(void)",
	"void I_setSomeMovieGlobal(void)", // sets some global (cleared by 93)
	"void I_playFlic092(char *)", // same coff as 0A9
	"void I_clearSomeMovieGlobal(void)", // clears some global (set by 91)
	"void Game::I_playCredits(void)",
	"byte Kernel::I_getCurrentKeyDown(void)", // get global - something about keyboard (by disasm)
	"int16 MainActor::I_teleportToEgg(int, int)", // a bit different to the U8 one - uses main actor map by default.
	"void PaletteFaderProcess:I_jumpToGreyScale(void)",
	"void I_resetVargasHealthTo500(void)", // TODO: look how this is used in disasm and usecode .. seems weird.
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void PaletteFaderProcess::I_stopFadesAndResetToGamePal(void)", // TODO: Implement this.
	"int16 PaletteFaderProcess::I_fadeFromBlack(nsteps)",
	"int16 PaletteFaderProcess::I_fadeFromBlackWithParam(nsteps, unk)", // TODO: what's the param?
	"int16 PaletteFaderProcess::I_fadeToBlack(nsteps)",
	"int16 PaletteFaderProcess::I_fadeToBlackWithParam(nsteps, unk)", // TODO: what's the param?
	"int16 PaletteFaderProcess::I_fadeToColor(r, g, b, nsteps, unk)", // TODO: what's the other param?
	// 00A0
	"void Actor::I_setDead(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getUnkEggType(Item *)", // based on disassembly, same as U8
	"void Egg::I_setEggXRange(Egg *, int)", // based on disasm
	"byte Item::I_overlaps(Item *, uint16 unk)", // same disasm as U8
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 I_getAnimationsDisabled(void)", // From disasm. Not implemented, that's ok..
	"int16 Egg::I_getEggXRange(Egg *)", // based on disasm
	"void Actor::I_setDead(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void I_playFlic0A9(char *)", // same coff as 092
	"void AudioProcess::I_playSFX(2 bytes)", // same coff as 0D4
	"byte Actor::I_getField0x59Bit1(Actor *)",
	"int16 Item::I_getFamilyOfType(Item *)", // per pentagram notes, matches disasm.
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 00B0
	"int16 Item::I_unequip(6 bytes)",
	"int16 Item::I_spawnUsecodeEvent0x13(Item *, 2 bytes)", // based on disasm - what is event 0x13? "avatar stole something" in U8..
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int32 I_getCurrentTimerTick(void)",
	"void Ultima8Engine::I_setAlertActive(void)",
	"int16 Item::I_equip(6 bytes)",
	"void Ultima8Engine::I_clrAlertActive(void)",
	"int16 Ultima8Engine::I_getAvatarInStasis(void)",
	"byte MainActor::I_addItemCru(4 bytes)", // same coff as 037
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Item::I_setQuality(Item *, int)", // same coff as 07F, 125
	"byte CurrentMap::I_canExistAt(int shapeno, word x, word y, byte z)", // NOTE: actually slightly different, uses shape info for an imaginary chequered wall shape? (0x31A)
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	// 00C0
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 KeypadGump::I_showKeypad(int targetCode)",
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void SpriteProcess::I_createSprite(word, word, word, word, uword, uword, ubyte)",
	"byte Item::I_getDirFromItem(Item *, itemno)", // same disasm as U8
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Actor::I_addHp(Actor *, int)",
	"void I_createMapJumpProcess(int16 mapnum)", // TODO: Implement me
	"byte Actor::I_getInCombat(Actor *)",
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Game::I_isReleaseBuild(void)", // whether the string "GAME COMPILE=1" has the 1.  Might be interesting to see what this does..
	"void Item::I_setQAndCombine(Item *, int16 q)", // based on disassembly
	// 00D0
	"int16 Item::I_use(Item *)", // same coff as 080, 0D5
	"void AudioProcess:I_stopAllSFX(void)", // based on disasm.
	"void I_playMovieCutscene(int *item,char *flicname,word sizex,word sizey)", // play flic
	"void I_clearKeyboardState(void)", // clears some globals and calls a keyboard reset function.. TODO: work out what those globals do?
	"void I_playSFX(2 bytes)", // same coff as 0AA.  Based on disasm.
	"int16 Item::I_use(Item *)", // same coff as 080, 0D0
	"byte CameraProcess::I_getCameraZ(void)",
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void PaletteFaderProcess::I_setPalToAllGrey(void)", // sets all colors to 0x3F3F3F
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Actor::I_getLastActivityNo(Actor *)",
	"void Actor::I_setCombatTactic(Actor *, int)",
	"int16 Actor::I_getEquip(6 bytes)", // based on disasm
	// 00E0
	"void Actor::I_setEquip(8 bytes)",
	"int16 Actor::I_getDefaultActivity0(Actor *)",
	"int16 Actor::I_getDefaultActivity1(Actor *)",
	"int16 Actor::I_getDefaultActivity2(Actor *)",
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Actor::I_attack(Actor *, uint16 target)",
	"void Actor::I_SetNPCDataField0x63_0E6(Actor *, int)",
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_cast(6 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 I_GetQOfAvatarInventoryItem0x4ed_0EB(void)",
	"void Item::I_popToEnd(Item*, int)", // similar code to U8
	"void Item::I_popToContainer(Item*, int)",  // same code as U8
	"void BatteryChargerProcess::I_create(void)",
	"int16 Kernel::I_getNumProcesses(int, int)", // same code as U8
	// 00F0
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"int16 Item::I_getQ(Item *)", // based on disassembly
	"void Item::I_setQ(Item *, uint16 q)", // based on disassembly
	"void CruHealer::I_create_0F6(void)",
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte MainActor::I_hasKeycard(int)",
	"void ComputerGump::I_readComputer(char *)",
	"int16 UCMachine::I_numToStr(int16 num)", // same as 113 based on same coff set 0FF, 113, 126
	// 0100
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // Based on variable name in TRIGGER::ordinal21. Part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Item::I_isCrusTypeNPC(uint16 shapenum)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	// 0110
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Actor::I_getDir(Actor *)", // same coff as 01C, 121
	"int16 UCMachine::I_numToStr(int16 num)", // based on VMAIL::func0A example usage
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Item::I_fireDistance(14 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_setIsBroken(Item *)", // same coff as 08C, 12A
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Item::I_getTypeFlag(Item *, uint16 shift)",
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getCY(Item *)", // same coff as 12B
	"byte Item::I_getCZ(Item *)", // based on disasm
	// 0120
	"int16 Item::I_getCX(Item *)",
	"byte Actor::I_getDir(4 bytes)", // same coff as 01C, 112
	"byte Actor::I_isDead(Item *)", // same coff as 12E, 039
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Item::I_setQuality(Item *, int)", // same coff as 07F, 0BA
	"int16 UCMachine::I_numToStr(int16 num)", // same as 113 based on same coff set 0FF, 113, 126
	"byte Item::I_getDirToCoords(Item *, uin16 x, uint16 y)", // based on disassembly
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_setIsBroken(Item *)", // same coff as 08C, 119
	"int16 IItem::I_getCY(Item *)", // same coff as 11E
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void Item::I_getFootpadData(Item *, uint *, uint *, uint *)", // same coff as 064
	"byte Actor::I_isDead(Item *)", // same coff as 122, 039
	"int16 Actor::I_createActorCru(Item *, uint16 other_itemno)",
	// 0130
	"void Actor::I_clrImmortal(Actor *)", // same coff as 07B
	"void Actor::I_setActivity(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void Item::I_andStatus(Item *, int16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void WeaselGump::I_showGump(uint16 param)",
	"void Actor::I_setDead(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void UNUSEDInt136()",
	"void UNUSEDInt137()"
};

const char * const ConvertUsecodeCrusader::_event_names[] = {
	"look()",						// 0x00
	"use()",						// 0x01
	"anim()",						// 0x02
	"setActivity()",				// 0x03
	"cachein()",					// 0x04
	"hit(uword, word)",				// 0x05
	"gotHit(uword, word)",			// 0x06
	"hatch()",						// 0x07
	"schedule()",					// 0x08
	"release()",					// 0x09
	"equip()",						// 0x0A
	"unequip()",					// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"cast(uword)",					// 0x11
	"justMoved()",					// 0x12
	"avatarStoleSomething(uword)",	// 0x13
	"animGetHit()",					// 0x14
	"unhatch(word)",				// 0x15
	"func16",						// 0x16
	"func17",						// 0x17
	"func18",						// 0x18
	"func19",						// 0x19
	"func1A",						// 0x1A
	"func1B",						// 0x1B
	"func1C",						// 0x1C
	"func1D",						// 0x1D
	"func1E",						// 0x1E
	"func1F",						// 0x1F
	0
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
