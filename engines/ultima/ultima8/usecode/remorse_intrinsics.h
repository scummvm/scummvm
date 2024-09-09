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

#ifndef ULTIMA8_USECODE_REMORSEINTRINSICS_H
#define ULTIMA8_USECODE_REMORSEINTRINSICS_H

#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/world/item.h"

namespace Ultima {
namespace Ultima8 {

// Crusader: No Remorse Intrinsics for V1.21
// Unknown function signatures were generate by the usecode disassembly
// and looking at handling of SP and retval after function.
Intrinsic RemorseIntrinsics[] = {
	// 0x000
	World::I_getAlertActive,
	Item::I_getFrame, // int Intrinsic001(Item *)
	Item::I_setFrame, //
	Item::I_getMapArray, // See TRIGGER::ordinal21 - stored in a variable 'mapNum'
	Item::I_getStatus,
	Item::I_orStatus,
	Item::I_equip, // void Intrinsic006(6 bytes)
	Item::I_isPartlyOnScreen, //
	Actor::I_isNPC, // byte Intrinsic008(Item *)
	Item::I_getZ, // byte Intrinsic009(Item *)
	Item::I_destroy, // void Intrinsic00A(Item *)
	Actor::I_getUnkByte, // get something about npcdata - struct byte 0x63 (99)
	Ultima8Engine::I_setAvatarInStasis, // void Intrinsic00C(2 bytes)
	Item::I_getDirToItem, // byte Intrinsic00D(6 bytes)
	Actor::I_turnToward,
	MovieGump::I_playMovieCutsceneAlt,
	// 0x010
	Item::I_getQLo, // Based on having same coff as 02B
	Actor::I_getMap, // int Intrinsic011(4 bytes)
	MusicProcess::I_playMusic, // void Intrinsic012(2 bytes)
	Item::I_getX, //int Intrinsic013(4 bytes)
	Item::I_getY, //int Intrinsic014(4 bytes)
	AudioProcess::I_playSFXCru,
	Item::I_getShape,
	Item::I_explode, // void Intrinsic017(8 bytes)
	UCMachine::I_rndRange, // int16 Intrinsic018(4 bytes)
	Item::I_legalCreateAtCoords, // byte Intrinsic019(14 bytes)
	Item::I_andStatus, // void Intrinsic01A(6 bytes)
	World::I_getControlledNPCNum,  // int16 Intrinsic01B(void)
	Actor::I_getDir, // byte Intrinsic01C(4 bytes)
	Actor::I_getLastAnimSet, // int Intrinsic01D(4 bytes)
	Item::I_fireWeapon, // int Intrinsic01E(16 bytes)
	Item::I_create,
	// 0x020
	Item::I_popToCoords, // void Intrinsic020(10 bytes)
	Actor::I_setDead, // void Intrinsic021(4 bytes)
	Item::I_push,
	Item::I_getEtherealTop, // int Intrinsic023(void)
	Item::I_setShape, // Probably, see PEPSIEW::gotHit
	Item::I_touch,
	Item::I_getQHi, // int16 Intrinsic026(Item *)
	Item::I_getClosestDirectionInRange, // int Intrinsic027(14 bytes)
	Item::I_hurl, // int Intrinsic028(12 bytes)
	World::I_gameDifficulty,
	AudioProcess::I_playAmbientSFXCru,
	Item::I_getQLo, // int16 Intrinsic02B(4 bytes)
	Item::I_inFastArea, // byte Intrinsic02C(4 bytes) // based on disassembly - checks for flag 0x2000
	Item::I_setQHi,
	Item::I_legalMoveToPoint, // byte Intrinsic02E(12 bytes)
	CurrentMap::I_canExistAtPoint, // byte Intrinsic02F(10 bytes)
	// 0x030
	Item::I_pop,
	Item::I_andStatus,
	Item::I_receiveHit, // void Intrinsic032(12 bytes)
	Actor::I_isBusy, // int Intrinsic033(4 bytes)
	Item::I_getDirFromTo16,
	Actor::I_isKneeling,
	Actor::I_doAnim, // void Intrinsic036(12 bytes)
	MainActor::I_addItemCru, // int Intrinsic037(4 bytes)
	AudioProcess::I_stopSFXCru, // takes Item *, sndno (from disasm)
	Actor::I_isDead, // int Intrinsic039(4 bytes)
	AudioProcess::I_isSFXPlayingForObject,
	Item::I_setQLo,
	Item::I_getFamily,
	Container::I_destroyContents,
	Item::I_fall,
	Egg::I_getEggId, // void Intrinsic03F(4 bytes)
	// 0x040
	CameraProcess::I_moveTo, // void Intrinsic040(8 bytes)
	CameraProcess::I_setCenterOn, // void Intrinsic041(2 bytes)
	Item::I_getRangeIfVisible, // int Intrinsic042(6 bytes)
	AudioProcess::I_playSFXCru, // TODO: Work out how this is different from Int015 - to a first approximation they are quite similar.
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	// 0x050
	Actor::I_getCurrentActivityNo, // void Intrinsic050(4 bytes)
	Actor::I_clrInCombat, // void Intrinsic051(4 bytes)
	Actor::I_setDefaultActivity0, // void Intrinsic052(6 bytes)
	Actor::I_setDefaultActivity1, // void Intrinsic053(6 bytes)
	Actor::I_setDefaultActivity2, // void Intrinsic054(6 bytes)
	Actor::I_setActivity, // void Intrinsic055(6 bytes)
	World::I_setControlledNPCNum, // void Intrinsic056(2 bytes)
	Item::I_getSurfaceWeight, // void Intrinsic057(4 bytes)
	Item::I_isCentreOn, // int Intrinsic058(6 bytes)
	Item::I_setFrame, // based on same coff as 002
	Actor::I_getLastAnimSet, // void Intrinsic05A(4 bytes)
	Item::I_legalCreateAtPoint, // probably. see PEPSIEW::use
	Item::I_getPoint, // void Intrinsic05C(8 bytes)
	CruStatusGump::I_hideStatusGump, // void Intrinsic05D(void)
	MovieGump::I_playMovieOverlay, // int16 Intrinsic05E(uint32, char *, int16 a, int16 b)
	CruStatusGump::I_showStatusGump, // void Intrinsic05F(void)
	// 0x060
	Actor::I_setDead, // void Intrinsic060(4 bytes)
	Actor::I_createActor, // void Intrinsic061(8 bytes)
	0, // void Intrinsic062(void)
	Actor::I_teleport, // void Intrinsic063(12 bytes)
	Item::I_getFootpadData, // void Intrinsic064(16 bytes)
	Item::I_isInNpc,
	Item::I_getQLo, // based on same coff set as 02B
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_setNpcNum, // void Item::I_setSomething068(Item *, int16 something) , see VALUEBOX:ordinal20
	Item::I_andStatus, // void Intrinsic069(6 bytes)
	Item::I_move, // void Intrinsic06A(10 bytes)
	UCMachine::I_true, // Note: This is actually "is compiled with VIOLENCE=1" (was set to 0 in germany). For now just always say yes.
	Kernel::I_resetRef, // void Intrinsic06C(4 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic06E(6 bytes)
	Item::I_isCompletelyOn,
	// 0x070
	Ultima8Engine::I_getCrusaderTeleporting, // int Intrinsic070(void)
	Ultima8Engine::I_setCrusaderTeleporting, // void Intrinsic071(void)
	Ultima8Engine::I_setCruStasis,
	Actor::I_setDead,
	Ultima8Engine::I_clrCrusaderTeleporting, // void Intrinsic074(void)
	Ultima8Engine::I_clrCruStasis,
	AudioProcess::I_stopSFXCru, // takes Item *, from disasm
	PaletteFaderProcess::I_fadeToBlack, // void Intrinsic077(void)
	MainActor::I_clrKeycards, // void Intrinsic078(void)
	MainActor::I_teleportToEgg, // different than U8's? void Intrinsic079(6 bytes)
	PaletteFaderProcess::I_fadeToGamePal, // void Intrinsic07A(void)
	Actor::I_clrImmortal, // based on disasm
	Actor::I_getHp, // void Intrinsic07C(4 bytes)
	Actor::I_setActivity, // void Intrinsic07D(6 bytes)
	Item::I_getQuality,
	Item::I_setQuality,
	// 0x080
	Item::I_use, // void Intrinsic080(4 bytes)
	MainActor::I_getMaxEnergy, // void Intrinsic081(4 bytes)
	Actor::I_getMana, // Actually energy, but map to mana as we don't need that in Crusader.
	Actor::I_setMana, //
	Item::I_getQLo, // based on same coff set as 02B
	Actor::I_setImmortal, // void Intrinsic085(4 bytes)
	CameraProcess::I_getCameraX, // void Intrinsic086(void)
	CameraProcess::I_getCameraY, // void Intrinsic087(void)
	Item::I_setMapArray,
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_shoot, // void Intrinsic08A(12 bytes)
	Item::I_enterFastArea, // void Intrinsic08B(4 bytes)
	Item::I_setBroken, // void Intrinsic08C(4 bytes)
	Item::I_hurl, // void Intrinsic08D(12 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	PaletteFaderProcess::I_jumpToAllBlack, // TODO: should also resume cycle process.
	// 0x090
	MusicProcess::I_stopMusic, // void Intrinsic090(void)
	0, // pause cycler - not needed as our cycler does not run when paused
	MovieGump::I_playMovieCutsceneAlt, // TODO: not exactly the same, Alt includes a fade.
	0, // resume cycler - not needed as our cycler does not run when paused
	Game::I_playCredits,
	Ultima8Engine::I_moveKeyDownRecently,
	MainActor::I_teleportToEgg, // void Intrinsic096(4 bytes)
	PaletteFaderProcess::I_jumpToGreyScale,
	World::I_resetVargasShield, // void Intrinsic098(void)
	Item::I_andStatus, // void Intrinsic099(6 bytes)
	PaletteFaderProcess::I_jumpToNormalPalette, // TODO: should also stop cycle process?
	PaletteFaderProcess::I_fadeToGamePal, // fade to game pal with number of steps
	PaletteFaderProcess::I_fadeToGamePal, // fade from black with nsteps and another unknown param
	PaletteFaderProcess::I_fadeToBlack, // fade to black with number of steps
	PaletteFaderProcess::I_fadeToBlack, // fade to black with nsteps and another unknown param
	PaletteFaderProcess::I_fadeToGivenColor,
	// 0x0A0
	Actor::I_setDead,
	Item::I_getQLo, // based on same coff set as 02B
	Item::I_getUnkEggType,
	Egg::I_setEggXRange, // void Intrinsic0A3(6 bytes)
	Item::I_overlaps,
	Item::I_isOn,
	UCMachine::I_true, // I_getAnimationsEnabled -> default to true (animations enabled)
	Egg::I_getEggXRange, // void Intrinsic0A7(4 bytes)
	Actor::I_setDead,
	MovieGump::I_playMovieCutsceneAlt, // TODO: not exactly the same, Alt includes a fade.
	AudioProcess::I_playSFX, // void Intrinsic0AA(2 bytes)
	Actor::I_isFalling, // int Actor::I_getFlag0x59Field1  Intrinsic0AB(4 bytes)
	Item::I_getFamilyOfType, // void Intrinsic0AC(2 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_getQLo, // based on same coff set as 02B
	Item::I_getQHi,  // based on same coff set as 026
	// 0x0B0
	Item::I_unequip, // void Intrinsic0B0(6 bytes)
	Item::I_avatarStoleSomething, // void Intrinsic0B1(6 bytes)
	Item::I_andStatus, // void Intrinsic0B2(6 bytes)
	Ultima8Engine::I_getCurrentTimerTick, // int32 Intrinsic0B3(void), probably, see FREE::ordinal32
	World::I_setAlertActive, // void Intrinsic0B4(void)
	Item::I_equip, // void Intrinsic0B5(6 bytes)
	World::I_clrAlertActive, // void Intrinsic0B6(void)
	Ultima8Engine::I_getAvatarInStasis, // void Intrinsic0B7(void)
	MainActor::I_addItemCru, // int Intrinsic0B8(4 bytes)
	Actor::I_getLastAnimSet, // void Intrinsic0B9(4 bytes)
	Item::I_setQuality,
	CurrentMap::I_canExistAt, // int Intrinsic0BB(8 bytes)
	Item::I_isOn,
	Item::I_hurl, // void Intrinsic0BD(12 bytes)
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_andStatus, // void Intrinsic0BF(6 bytes)
	// 0x0C0
	Item::I_hurl, // void Intrinsic0C0(12 bytes)
	Item::I_andStatus, // void Intrinsic0C1(6 bytes)
	Item::I_hurl, // void Intrinsic0C2(12 bytes)
	Item::I_andStatus, // void Intrinsic0C3(6 bytes)
	KeypadGump::I_showKeypad, // void Intrinsic0C4(2 bytes)
	Item::I_isOn,
	SpriteProcess::I_createSprite, // void Intrinsic0C6(14 bytes)
	Item::I_getDirFromItem, // int Intrinsic0C7(6 bytes)
	Item::I_hurl, // void Intrinsic0C8(12 bytes)
	Item::I_getQHi,  // based on same coff set as 026
	Actor::I_addHp, // int Intrinsic0CA(6 bytes)
	MainActor::I_switchMap, // 0CB
	Actor::I_isInCombat, // int Intrinsic0CC(4 bytes)
	Actor::I_setActivity, // void Intrinsic0CD(6 bytes)
	UCMachine::I_true, // whether the string "GAME COMPILE=1" has the 1.  Might be interesting to see how this changes the game.. for now just set to true.
	Item::I_setQAndCombine, // void Intrinsic0CF(6 bytes)
	// 0x0D0
	Item::I_use, // void Intrinsic0D0(4 bytes)
	AudioProcess::I_stopAllSFX,
	MovieGump::I_playMovieCutscene, // void I_playFlic(int *item,char *flicname,word sizex,word sizey) // play flic
	0, // void Intrinsic0D3(void)
	AudioProcess::I_playSFX, // void Intrinsic0D4(2 bytes)
	Item::I_use, // void Intrinsic0D5(4 bytes)
	CameraProcess::I_getCameraZ, // int Intrinsic0D6(void)
	Actor::I_getLastAnimSet, // void Intrinsic0D7(4 bytes)
	Actor::I_setDead,
	Item::I_getQLo, // based on same coff set as 02B
	PaletteFaderProcess::I_jumpToAllWhite,
	Actor::I_setActivity, // void Intrinsic0DB(6 bytes)
	Item::I_isOn,
	Actor::I_getLastActivityNo, // void Intrinsic0DD(4 bytes)
	Actor::I_setCombatTactic, // void Intrinsic0DE(6 bytes)
	Actor::I_getEquip, // void Intrinsic0DF(6 bytes)
	// 0x0E0
	Actor::I_setEquip, // void Intrinsic0E0(8 bytes)
	Actor::I_getDefaultActivity0, // void Intrinsic0E1(4 bytes)
	Actor::I_getDefaultActivity1, // void Intrinsic0E2(4 bytes)
	Actor::I_getDefaultActivity2, // void Intrinsic0E3(4 bytes)
	Actor::I_getLastAnimSet, // void Intrinsic0E4(4 bytes)
	Actor::I_setTarget,
	Actor::I_setUnkByte, // void Intrinsic0E6(6 bytes)
	Actor::I_setDead,
	Item::I_cast, // void Intrinsic0E8(6 bytes)
	Item::I_andStatus, // void Intrinsic0E9(6 bytes)
	Item::I_getQLo, // based on same coff set as 02B
	MainActor::I_getNumberOfCredits, // void Intrinsic0EB(void)
	Item::I_popToEnd,
	Item::I_popToContainer,
	BatteryChargerProcess::I_create,
	Kernel::I_getNumProcesses, // void Intrinsic0EF(4 bytes)
	// 0x0F0
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_isOn,
	Actor::I_setActivity, // void Intrinsic0F2(6 bytes)
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_getQ, // void Intrinsic0F4(4 bytes)
	Item::I_setQ, // void Intrinsic0F5(6 bytes)
	CruHealerProcess::I_create,
	Item::I_hurl, // void Intrinsic0F7(12 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_hurl, // void Intrinsic0F9(12 bytes)
	Item::I_isOn,
	Item::I_getQHi,  // based on same coff set as 026
	Item::I_andStatus, // void Intrinsic0FC(6 bytes)
	MainActor::I_hasKeycard, // int Intrinsic0FD(2 bytes)
	ComputerGump::I_readComputer, // void Intrinsic0FE(4 bytes)
	UCMachine::I_numToStr, // same as 113 based on same coff set 0FF, 113, 126
	// 0x100
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic101(6 bytes)
	Item::I_getNpcNum, // Based on variable name in TRIGGER::ordinal21
	Item::I_isCrusTypeNPC,
	Item::I_andStatus, // void Intrinsic104(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic106(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic108(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic10A(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic10C(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_andStatus, // void Intrinsic10E(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	// 0x110
	Item::I_andStatus, // void Intrinsic110(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Actor::I_getDir, // byte Intrinsic112(4 bytes)
	UCMachine::I_numToStr, // see VMAIL::func0A for example usage
	Item::I_andStatus, // void Intrinsic114(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_fireDistance, // byte Intrinsic116(14 bytes)
	Item::I_andStatus, // void Intrinsic117(6 bytes)
	Item::I_hurl, // int16 Intrinsic118(12 bytes)
	Item::I_setBroken, // void Intrinsic119(4 bytes)
	Item::I_andStatus, // void Intrinsic11A(6 bytes)
	Item::I_getTypeFlag, // byte Intrinsic11B(6 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_hurl, // int16 Intrinsic11D(12 bytes)
	Item::I_getCY, // int16 Intrinsic11E(4 bytes)
	Item::I_getCZ, // byte Intrinsic11F(4 bytes)
	// 0x120
	Item::I_getCX, // void Intrinsic120(4 bytes)
	Actor::I_getDir, // int Intrinsic121(4 bytes)
	Actor::I_isDead, // int Intrinsic122(4 bytes)
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Actor::I_getLastAnimSet, // void Intrinsic124(4 bytes)
	Item::I_setQuality,
	UCMachine::I_numToStr, // same as 113 based on same coff set 0FF, 113, 126
	Item::I_getDirToCoords, // int Intrinsic127(8 bytes)
	Item::I_andStatus, // void Intrinsic128(6 bytes) // maybe Item::andStatus?? see ITEM::ordinal22
	Item::I_getNpcNum, // based on same coff as 102 (-> variable name in TRIGGER::ordinal21)
	Item::I_setBroken, // void Intrinsic12A(4 bytes)
	Item::I_getCY, // void Intrinsic12B(4 bytes)
	Item::I_isOn,
	Item::I_getFootpadData, // void Intrinsic12D(16 bytes)
	Actor::I_isDead, // int Intrinsic12E(4 bytes)
	Actor::I_createActorCru, // void Intrinsic12F(6 bytes)
	// 0x130
	Actor::I_clrImmortal, // void Intrinsic130(4 bytes)
	Actor::I_setActivity, // void Intrinsic131(6 bytes)
	Item::I_andStatus, // void Intrinsic132(6 bytes)
	Item::I_getQHi,  // based on same coff set as 026
	WeaselGump::I_showWeaselGump, // void Intrinsic134(2 bytes)
	Actor::I_setDead,
	0, // void UNUSEDInt136()
	0  // void UNUSEDInt137()
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Intrinsics for the Remorse demo.
Intrinsic RemorseDemoIntrinsics[] = {
	World::I_getAlertActive,        // Intrinsic000 10d8:0243
	Item::I_getFrame,               // Intrinsic001 10a0:07b4
	Item::I_setFrame,               // Intrinsic002 10a0:0810
	Item::I_getMapArray,            // Intrinsic003 10a0:0c78
	Item::I_getStatus,              // Intrinsic004 10a0:1038
	Item::I_orStatus,               // Intrinsic005 10a0:2572
	Item::I_equip,                  // Intrinsic006 10a0:2a35
	Item::I_isPartlyOnScreen,       // Intrinsic007 10a0:40ce
	Actor::I_isNPC,                 // Intrinsic008 10a0:2486
	Item::I_getZ,                   // Intrinsic009 10a0:0266
	Item::I_destroy,                // Intrinsic00A 10a0:1365
	Actor::I_getUnkByte,            // Intrinsic00B 10e8:3dfc
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic00C 10e8:369e
	Item::I_getDirToItem,           // Intrinsic00D 10a0:1a74
	Actor::I_turnToward,            // Intrinsic00E 10e8:326b
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic00F 10a0:3abb
	// 0x10
	Item::I_getQLo,                 // Intrinsic010 10a0:3889
	Actor::I_getMap,                // Intrinsic011 10e8:2b90
	MusicProcess::I_playMusic,      // Intrinsic012 11f8:0143
	Item::I_getX,                   // Intrinsic013 10a0:013e
	Item::I_getY,                   // Intrinsic014 10a0:0176
	AudioProcess::I_playSFXCru,     // Intrinsic015 10a0:3934
	Item::I_getShape,               // Intrinsic016 10a0:068f
	Item::I_explode,                // Intrinsic017 1148:0859
	UCMachine::I_rndRange,          // Intrinsic018 12d8:0293
	Item::I_legalCreateAtCoords,    // Intrinsic019 10a0:10db
	Item::I_andStatus,              // Intrinsic01A 10a0:258b
	World::I_getControlledNPCNum,   // Intrinsic01B 1130:0010
	Actor::I_getDir,                // Intrinsic01C 10e8:3714
	Actor::I_getLastAnimSet,        // Intrinsic01D 10e8:372e
	Item::I_fireWeapon,             // Intrinsic01E 1128:11da
	Item::I_create,                 // Intrinsic01F 10a0:11e7
	// 0x20
	Item::I_popToCoords,            // Intrinsic020 10a0:1311
	Actor::I_setDead,               // Intrinsic021 10e8:2010
	Item::I_push,                   // Intrinsic022 10a0:1352
	Item::I_getEtherealTop,			// Intrinsic023 10a8:26a8
	Item::I_setShape,               // Intrinsic024 10a0:06a5
	Item::I_touch,                  // Intrinsic025 10a0:2514
	Item::I_getQHi,                 // Intrinsic026 10a0:383e
	Item::I_getClosestDirectionInRange, // Intrinsic027 10d0:0000
	Item::I_hurl,                   // Intrinsic028 1038:12a6
	World::I_gameDifficulty,        // Intrinsic029 10e8:38d5
	AudioProcess::I_playAmbientSFXCru, // Intrinsic02A 10a0:3953
	Item::I_isCompletelyOn,         // Intrinsic02B 10a0:1f5f
	UCMachine::I_true,              // Intrinsic02C 1048:00c1
	Container::I_destroyContents,   // Intrinsic02D 10a0:14af
	Item::I_getQLo,                 // Intrinsic02E 10a0:3889
	Item::I_inFastArea,             // Intrinsic02F 10a0:3af5
	// 0x30
	Item::I_setQHi,                 // Intrinsic030 10a0:385a
	Item::I_legalMoveToPoint,       // Intrinsic031 10a0:174d
	CurrentMap::I_canExistAtPoint,  // Intrinsic032 10a0:4071
	Item::I_pop,                    // Intrinsic033 10a0:1338
	Item::I_andStatus,              // Intrinsic034 10a0:258b
	Item::I_receiveHit,             // Intrinsic035 1128:0247
	Actor::I_isBusy,                // Intrinsic036 10e8:010c
	Item::I_getDirFromTo16,         // Intrinsic037 10d0:01c7
	Actor::I_isKneeling,            // Intrinsic038 10e8:21fb
	Actor::I_doAnim,                // Intrinsic039 10e8:0245
	MainActor::I_addItemCru,        // Intrinsic03A 13e8:0330
	AudioProcess::I_stopSFXCru,     // Intrinsic03B 10a0:39b0
	Actor::I_isDead,                // Intrinsic03C 10e8:1fe0
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic03D 10a0:39eb
	Item::I_setQLo,                 // Intrinsic03E 10a0:38a2
	Item::I_getFamily,              // Intrinsic03F 10a0:0fc4
	// 0x40
	Item::I_fall,                   // Intrinsic040 1038:1334
	Egg::I_getEggId,                // Intrinsic041 1090:05c5
	CameraProcess::I_moveTo,        // Intrinsic042 1180:0cba
	CameraProcess::I_setCenterOn,   // Intrinsic043 1180:1aa5
	Item::I_getRangeIfVisible,      // Intrinsic044 1128:1c01
	AudioProcess::I_playSFXCru,     // Intrinsic045 10a0:3991
	Item::I_isOn,                   // Intrinsic046 10a0:1eea
	Item::I_getQHi,                 // Intrinsic047 10a0:383e
	Item::I_isOn,                   // Intrinsic048 10a0:1eea
	Item::I_getQHi,                 // Intrinsic049 10a0:383e
	Item::I_isOn,                   // Intrinsic04A 10a0:1eea
	Item::I_getQHi,                 // Intrinsic04B 10a0:383e
	Item::I_isOn,                   // Intrinsic04C 10a0:1eea
	Item::I_getQHi,                 // Intrinsic04D 10a0:383e
	Item::I_isOn,                   // Intrinsic04E 10a0:1eea
	Item::I_getQHi,                 // Intrinsic04F 10a0:383e
	// 0x50
	Item::I_isOn,                   // Intrinsic050 10a0:1eea
	Item::I_getQHi,                 // Intrinsic051 10a0:383e
	Actor::I_getCurrentActivityNo,  // Intrinsic052 10e8:330d
	Actor::I_clrInCombat,           // Intrinsic053 10e8:31a9
	Actor::I_setDefaultActivity0,   // Intrinsic054 10e8:2bea
	Actor::I_setDefaultActivity1,   // Intrinsic055 10e8:2c07
	Actor::I_setDefaultActivity2,   // Intrinsic056 10e8:2c24
	Actor::I_setActivity,           // Intrinsic057 10e8:2d02
	World::I_setControlledNPCNum,   // Intrinsic058 1130:0000
	Item::I_getSurfaceWeight,       // Intrinsic059 10a0:0602
	Item::I_isCentreOn,             // Intrinsic05A 10a0:4377
	Item::I_setFrame,               // Intrinsic05B 10a0:0810
	Actor::I_getLastAnimSet,        // Intrinsic05C 10e8:372e
	Item::I_legalCreateAtPoint,     // Intrinsic05D 10a0:104e
	Item::I_getPoint,               // Intrinsic05E 10a0:2436
	CruStatusGump::I_hideStatusGump, // Intrinsic05F 11b0:0000
	// 0x60
	MovieGump::I_playMovieOverlay,  // Intrinsic060 10a0:3ad7
	CruStatusGump::I_showStatusGump, // Intrinsic061 11b0:0006
	Actor::I_setDead,               // Intrinsic062 10e8:2010
	Actor::I_createActor,           // Intrinsic063 10e8:25b9
	0, // I_forceCameraUpdate,      // Intrinsic064 1180:0013
	Actor::I_teleport,              // Intrinsic065 10e8:29fa
	Item::I_getFootpadData,         // Intrinsic066 10a0:1aa9
	Item::I_isInNpc,                // Intrinsic067 10a0:24b5
	Item::I_getQLo,                 // Intrinsic068 10a0:3889
	Item::I_getNpcNum,              // Intrinsic069 10a0:0c40
	Item::I_setNpcNum,              // Intrinsic06A 10a0:0c24
	Item::I_andStatus,              // Intrinsic06B 10a0:258b
	Item::I_move,                   // Intrinsic06C 10a0:1531
	UCMachine::I_true,              // Intrinsic06D 1048:00a1
	Kernel::I_resetRef,             // Intrinsic06E 11d0:0c63
	Item::I_getNpcNum,              // Intrinsic06F 10a0:0c40
	// 0x70
	Item::I_andStatus,              // Intrinsic070 10a0:258b
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic071 13e8:0e27
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic072 13e8:0e2b
	Ultima8Engine::I_setCruStasis,  // Intrinsic073 13e8:0071
	Actor::I_setDead,               // Intrinsic074 10e8:2010
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic075 13e8:0e21
	Ultima8Engine::I_clrCruStasis,  // Intrinsic076 13e8:0077
	AudioProcess::I_stopSFXCru,     // Intrinsic077 10a0:39cf
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic078 1440:06e6
	MainActor::I_clrKeycards,       // Intrinsic079 10e8:3d4b
	MainActor::I_teleportToEgg,     // Intrinsic07A 1090:04ce
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic07B 1440:07cc
	Actor::I_clrImmortal,           // Intrinsic07C 10e8:2185
	Actor::I_getHp,                 // Intrinsic07D 10e8:32f4
	Actor::I_setActivity,           // Intrinsic07E 10e8:2d02
	Item::I_getQuality,             // Intrinsic07F 10a0:0caa
	// 0x80
	Item::I_setQuality,             // Intrinsic080 10a0:0cee
	Item::I_use,                    // Intrinsic081 10a0:2667
	MainActor::I_getMaxEnergy,      // Intrinsic082 10e8:348b
	Actor::I_getMana,               // Intrinsic083 10e8:3454
	Actor::I_setMana,               // Intrinsic084 10e8:346e
	Item::I_getQLo,                 // Intrinsic085 10a0:3889
	Actor::I_setImmortal,			// Intrinsic086 10e8:216a
	CameraProcess::I_getCameraX,    // Intrinsic087 1180:1cc5
	CameraProcess::I_getCameraY,    // Intrinsic088 1180:1ccd
	Item::I_setMapArray,            // Intrinsic089 10a0:0c5c
	Item::I_getNpcNum,              // Intrinsic08A 10a0:0c40
	Item::I_shoot,                  // Intrinsic08B 10a0:38d1
	Item::I_enterFastArea,          // Intrinsic08C 10a0:2b30
	Item::I_setBroken,              // Intrinsic08D 10a0:25d0
	Item::I_hurl,                   // Intrinsic08E 1038:12a6
	Item::I_getNpcNum,              // Intrinsic08F 10a0:0c40
	// 0x90
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic090 1440:07da
	MusicProcess::I_stopMusic,      // Intrinsic091 11f8:0223
	0,
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic093 10a0:3a9f
	0,
	Game::I_playCredits,            // Intrinsic095 13e8:003f
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic096 10a0:4340
	MainActor::I_teleportToEgg,     // Intrinsic097 1090:04f7
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic098 1028:0605
	World::I_resetVargasShield,     // Intrinsic099 1128:0000
	Item::I_andStatus,              // Intrinsic09A 10a0:258b
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic09B 1440:0a7c
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09C 1440:06f4
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09D 1440:0bc6
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic09E 1440:04ea
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic09F 1440:0c47
	// 0xA0
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic0A0 1440:0ab2
	Actor::I_setDead,               // Intrinsic0A1 10e8:2010
	Item::I_getQLo,                 // Intrinsic0A2 10a0:3889
	Item::I_getUnkEggType,          // Intrinsic0A3 10a0:0d31
	Egg::I_setEggXRange,            // Intrinsic0A4 1090:055b
	Item::I_overlaps,               // Intrinsic0A5 10a0:1cc5
	Item::I_isOn,                   // Intrinsic0A6 10a0:1eea
	UCMachine::I_true,              // Intrinsic0A7 13e8:00b9
	Egg::I_getEggXRange,            // Intrinsic0A8 1090:051d
	Actor::I_setDead,               // Intrinsic0A9 10e8:2010
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic0AA 10a0:3a9f
	AudioProcess::I_playSFX,        // Intrinsic0AB 12d8:0476
	Actor::I_isFalling,             // Intrinsic0AC 10e8:1f01
	Item::I_getFamilyOfType,        // Intrinsic0AD 10f8:054d
	Item::I_getNpcNum,              // Intrinsic0AE 10a0:0c40
	Item::I_getQLo,                 // Intrinsic0AF 10a0:3889
	// 0xB0
	Item::I_getQHi,                 // Intrinsic0B0 10a0:383e
	Item::I_unequip,                // Intrinsic0B1 10a0:2a68
	Item::I_avatarStoleSomething,   // Intrinsic0B2 10a0:2fda
	Item::I_andStatus,              // Intrinsic0B3 10a0:258b
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic0B4 1030:03b8
	World::I_setAlertActive,        // Intrinsic0B5 10d8:0000
	Item::I_equip,                  // Intrinsic0B6 10a0:2a35
	World::I_clrAlertActive,        // Intrinsic0B7 10d8:0121
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic0B8 10e8:3696
	MainActor::I_addItemCru,        // Intrinsic0B9 13e8:0330
	Actor::I_getLastAnimSet,        // Intrinsic0BA 10e8:372e
	Item::I_setQuality,             // Intrinsic0BB 10a0:0cee
	CurrentMap::I_canExistAt,       // Intrinsic0BC 1148:0b67
	Item::I_isOn,                   // Intrinsic0BD 10a0:1eea
	Item::I_hurl,                   // Intrinsic0BE 1038:12a6
	Item::I_getQHi,                 // Intrinsic0BF 10a0:383e
	// 0xC0
	Item::I_andStatus,              // Intrinsic0C0 10a0:258b
	Item::I_hurl,                   // Intrinsic0C1 1038:12a6
	Item::I_andStatus,              // Intrinsic0C2 10a0:258b
	Item::I_hurl,                   // Intrinsic0C3 1038:12a6
	Item::I_andStatus,              // Intrinsic0C4 10a0:258b
	KeypadGump::I_showKeypad,       // Intrinsic0C5 13c0:00fe
	Item::I_isOn,                   // Intrinsic0C6 10a0:1eea
	SpriteProcess::I_createSprite,  // Intrinsic0C7 1138:00de
	Item::I_getDirFromItem,         // Intrinsic0C8 10a0:1a3f
	Item::I_hurl,                   // Intrinsic0C9 1038:12a6
	Item::I_getQHi,                 // Intrinsic0CA 10a0:383e
	Actor::I_addHp,                 // Intrinsic0CB 10e8:3748
	Game::I_playDemoScreen,			// Intrinsic0CC 1020:049c
	Actor::I_isInCombat,            // Intrinsic0CD 10e8:1f20
	Actor::I_setActivity,           // Intrinsic0CE 10e8:2d02
	Item::I_setQAndCombine,         // Intrinsic0CF 10a0:0dd1
	// 0xD0
	Item::I_use,                    // Intrinsic0D0 10a0:2667
	AudioProcess::I_stopAllSFX,     // Intrinsic0D1 12d8:054a
	MovieGump::I_playMovieCutscene, // Intrinsic0D2 10a0:3a5b
	0, //I_clearKeyboardState,      // Intrinsic0D3 13e8:00a8
	AudioProcess::I_playSFX,        // Intrinsic0D4 12d8:0476
	Item::I_use,                    // Intrinsic0D5 10a0:2667
	CameraProcess::I_getCameraZ,    // Intrinsic0D6 1180:1cd5
	Actor::I_getLastAnimSet,        // Intrinsic0D7 10e8:372e
	Actor::I_setDead,               // Intrinsic0D8 10e8:2010
	Item::I_getQLo,                 // Intrinsic0D9 10a0:3889
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic0DA 1440:0853
	Actor::I_setActivity,           // Intrinsic0DB 10e8:2d02
	Item::I_isOn,                   // Intrinsic0DC 10a0:1eea
	Actor::I_getLastActivityNo,     // Intrinsic0DD 10e8:3e35
	Actor::I_setCombatTactic,       // Intrinsic0DE 10e8:3594
	Actor::I_getEquip,              // Intrinsic0DF 10e8:27c8
	// 0xE0
	Actor::I_setEquip,              // Intrinsic0E0 10e8:27fb
	Actor::I_getDefaultActivity0,   // Intrinsic0E1 10e8:2c78
	Actor::I_getDefaultActivity1,   // Intrinsic0E2 10e8:2c92
	Actor::I_getDefaultActivity2,   // Intrinsic0E3 10e8:2cac
	Actor::I_getLastAnimSet,        // Intrinsic0E4 10e8:372e
	Actor::I_setTarget,             // Intrinsic0E5 10e8:1e71
	Actor::I_setUnkByte,            // Intrinsic0E6 10e8:3e18
	Actor::I_setDead,               // Intrinsic0E7 10e8:2010
	Item::I_cast,                   // Intrinsic0E8 10a0:2f3e
	Item::I_andStatus,              // Intrinsic0E9 10a0:258b
	Item::I_getQLo,                 // Intrinsic0EA 10a0:3889
	MainActor::I_getNumberOfCredits, // Intrinsic0EB 10e8:3833
	Item::I_popToEnd,               // Intrinsic0EC 10a0:1342
	Item::I_popToContainer,         // Intrinsic0ED 10a0:1328
	BatteryChargerProcess::I_create, // Intrinsic0EE 1128:20c5
	Kernel::I_getNumProcesses,      // Intrinsic0EF 11d0:0977
	// 0xF0
	Item::I_getQHi,                 // Intrinsic0F0 10a0:383e
	Item::I_isOn,                   // Intrinsic0F1 10a0:1eea
	Actor::I_setActivity,           // Intrinsic0F2 10e8:2d02
	Item::I_getQHi,                 // Intrinsic0F3 10a0:383e
	Item::I_getQ,                   // Intrinsic0F4 10a0:0c94
	Item::I_setQ,                   // Intrinsic0F5 10a0:0f5f
	CruHealerProcess::I_create,     // Intrinsic0F6 1128:20e5
	Item::I_hurl,                   // Intrinsic0F7 1038:12a6
	Item::I_getNpcNum,              // Intrinsic0F8 10a0:0c40
	Item::I_hurl,                   // Intrinsic0F9 1038:12a6
	Item::I_isOn,                   // Intrinsic0FA 10a0:1eea
	Item::I_getQHi,                 // Intrinsic0FB 10a0:383e
	Item::I_andStatus,              // Intrinsic0FC 10a0:258b
	MainActor::I_hasKeycard,        // Intrinsic0FD 10e8:3ce8
	ComputerGump::I_readComputer,   // Intrinsic0FE 1398:0005
	UCMachine::I_numToStr,          // Intrinsic0FF 1418:0073
	// 0x100
	Item::I_getNpcNum,              // Intrinsic100 10a0:0c40
	Item::I_andStatus,              // Intrinsic101 10a0:258b
	Item::I_getNpcNum,              // Intrinsic102 10a0:0c40
	Item::I_isCrusTypeNPC,			// Intrinsic103 10a0:4348
	Item::I_andStatus,              // Intrinsic104 10a0:258b
	Item::I_getNpcNum,              // Intrinsic105 10a0:0c40
	Item::I_andStatus,              // Intrinsic106 10a0:258b
	Item::I_getNpcNum,              // Intrinsic107 10a0:0c40
	Item::I_andStatus,              // Intrinsic108 10a0:258b
	Item::I_getNpcNum,              // Intrinsic109 10a0:0c40
	Item::I_andStatus,              // Intrinsic10A 10a0:258b
	Item::I_getNpcNum,              // Intrinsic10B 10a0:0c40
	Item::I_andStatus,              // Intrinsic10C 10a0:258b
	Item::I_getNpcNum,              // Intrinsic10D 10a0:0c40
	Item::I_andStatus,              // Intrinsic10E 10a0:258b
	Item::I_getNpcNum,              // Intrinsic10F 10a0:0c40
	// 0x110
	Item::I_andStatus,              // Intrinsic110 10a0:258b
	Item::I_getNpcNum,              // Intrinsic111 10a0:0c40
	Actor::I_getDir,                // Intrinsic112 10e8:3714
	UCMachine::I_numToStr,          // Intrinsic113 1418:0073
	Item::I_andStatus,              // Intrinsic114 10a0:258b
	Item::I_getNpcNum,              // Intrinsic115 10a0:0c40
	Item::I_fireDistance,           // Intrinsic116 1128:1755
	Item::I_andStatus,              // Intrinsic117 10a0:258b
	Item::I_hurl,                   // Intrinsic118 1038:12a6
	Item::I_setBroken,              // Intrinsic119 10a0:25d0
	Item::I_andStatus,              // Intrinsic11A 10a0:258b
	Item::I_getTypeFlag,            // Intrinsic11B 10a0:0fef
	Item::I_getNpcNum,              // Intrinsic11C 10a0:0c40
	Item::I_hurl,                   // Intrinsic11D 1038:12a6
	Item::I_getCY,                  // Intrinsic11E 10a0:02de
	Item::I_getCZ,                  // Intrinsic11F 10a0:0322
	// 0x120
	Item::I_getCX,                  // Intrinsic120 10a0:029a
	Actor::I_getDir,                // Intrinsic121 10e8:3714
	UCMachine::I_numToStr,          // Intrinsic122 1418:0073
	Item::I_getDirToCoords,         // Intrinsic123 10a0:19e9
	Item::I_andStatus,              // Intrinsic124 10a0:258b
	Item::I_getNpcNum,              // Intrinsic125 10a0:0c40
	Item::I_isOn,                   // Intrinsic126 10a0:1eea
	Item::I_getFootpadData,         // Intrinsic127 10a0:1aa9
	Actor::I_isDead,                // Intrinsic128 10e8:1fe0
	Actor::I_createActorCru,        // Intrinsic129 10a0:3d3f
	Actor::I_clrImmortal,			// Intrinsic12A 10e8:2185
	Item::I_andStatus,              // Intrinsic12B 10a0:258b
	Item::I_getQHi,                 // Intrinsic12C 10a0:383e
	WeaselGump::I_showWeaselGump,   // Intrinsic12D 13e0:0000
	Actor::I_setDead,               // Intrinsic12E 10e8:2010
	Item::I_getQLo,                 // Intrinsic12F 10a0:3889
	// 0x130
	Item::I_getNpcNum,              // Intrinsic130 10a0:0c40
	Actor::I_getDir,                // Intrinsic131 10e8:3714
	Actor::I_getLastAnimSet,        // Intrinsic132 10e8:372e
	Item::I_setQuality,             // Intrinsic133 10a0:0cee
	0 // Unused // Intrinsic134 1040:0079
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Intrinsics for the Remorse Spanish version.
Intrinsic RemorseEsIntrinsics[] = {
	ComputerGump::I_readComputer,   // Intrinsic000 13a0:0005
	Item::I_getMapArray,            // Intrinsic001 10a8:0c78
	Item::I_getFrame,               // Intrinsic002 10a8:07b4
	Item::I_setFrame,               // Intrinsic003 10a8:0810
	UCMachine::I_rndRange,          // Intrinsic004 12e0:0293
	UCMachine::I_numToStr,          // Intrinsic005 1418:0073
	CruStatusGump::I_hideStatusGump, // Intrinsic006 11b8:0000
	Actor::I_getMap,                // Intrinsic007 10f0:2b90
	MovieGump::I_playMovieOverlay,  // Intrinsic008 10a8:3ad7
	CruStatusGump::I_showStatusGump, // Intrinsic009 11b8:0006
	Item::I_getDirToItem,           // Intrinsic00A 10a8:1a74
	Actor::I_turnToward,            // Intrinsic00B 10f0:326b
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic00C 10a8:3abb
	AudioProcess::I_playSFXCru,     // Intrinsic00D 10a8:3934
	World::I_getAlertActive,        // Intrinsic00E 10e0:0243
	Item::I_getStatus,              // Intrinsic00F 10a8:1038
	// 0x010
	Item::I_orStatus,               // Intrinsic010 10a8:2572
	Item::I_equip,                  // Intrinsic011 10a8:2a35
	Item::I_isPartlyOnScreen,       // Intrinsic012 10a8:40ce
	Actor::I_isNPC,                 // Intrinsic013 10a8:2486
	Item::I_getZ,                   // Intrinsic014 10a8:0266
	Item::I_destroy,                // Intrinsic015 10a8:1365
	Actor::I_getUnkByte,            // Intrinsic016 10f0:3dfc
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic017 10f0:369e
	Item::I_getQLo,                 // Intrinsic018 10a8:3889
	MusicProcess::I_playMusic,      // Intrinsic019 1200:0143
	Item::I_getX,                   // Intrinsic01A 10a8:013e
	Item::I_getY,                   // Intrinsic01B 10a8:0176
	Item::I_getShape,               // Intrinsic01C 10a8:068f
	Item::I_explode,                // Intrinsic01D 1150:0859
	Item::I_legalCreateAtCoords,    // Intrinsic01E 10a8:10db
	Item::I_andStatus,              // Intrinsic01F 10a8:258b
	// 0x020
	World::I_getControlledNPCNum,   // Intrinsic020 1138:0010
	Actor::I_getDir,                // Intrinsic021 10f0:3714
	Actor::I_getLastAnimSet,        // Intrinsic022 10f0:372e
	Item::I_fireWeapon,             // Intrinsic023 1130:11da
	Item::I_create,                 // Intrinsic024 10a8:11e7
	Item::I_popToCoords,            // Intrinsic025 10a8:1311
	Actor::I_setDead,               // Intrinsic026 10f0:2010
	Item::I_push,                   // Intrinsic027 10a8:1352
	Item::I_getEtherealTop,			// Intrinsic028 10b0:2622
	Item::I_setShape,               // Intrinsic029 10a8:06a5
	Item::I_touch,                  // Intrinsic02A 10a8:2514
	Item::I_getQHi,                 // Intrinsic02B 10a8:383e
	Item::I_getClosestDirectionInRange, // Intrinsic02C 10d8:0000
	Item::I_hurl,                   // Intrinsic02D 1040:12a6
	World::I_gameDifficulty,        // Intrinsic02E 10f0:38d5
	AudioProcess::I_playAmbientSFXCru, // Intrinsic02F 10a8:3953
	// 0x030
	Item::I_isCompletelyOn,         // Intrinsic030 10a8:1f5f
	UCMachine::I_true,              // Intrinsic031 1050:00c1
	Container::I_destroyContents,   // Intrinsic032 10a8:14af
	Item::I_getQLo,                 // Intrinsic033 10a8:3889
	Item::I_inFastArea,             // Intrinsic034 10a8:3af5
	Item::I_setQHi,                 // Intrinsic035 10a8:385a
	Item::I_legalMoveToPoint,       // Intrinsic036 10a8:174d
	CurrentMap::I_canExistAtPoint,  // Intrinsic037 10a8:4071
	Item::I_pop,                    // Intrinsic038 10a8:1338
	Item::I_andStatus,              // Intrinsic039 10a8:258b
	Item::I_receiveHit,             // Intrinsic03A 1130:0247
	Actor::I_isBusy,                // Intrinsic03B 10f0:010c
	Item::I_getDirFromTo16,         // Intrinsic03C 10d8:01c7
	Actor::I_isKneeling,            // Intrinsic03D 10f0:21fb
	Actor::I_doAnim,                // Intrinsic03E 10f0:0245
	MainActor::I_addItemCru,        // Intrinsic03F 13e8:0330
	// 0x040
	AudioProcess::I_stopSFXCru,     // Intrinsic040 10a8:39b0
	Actor::I_isDead,                // Intrinsic041 10f0:1fe0
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic042 10a8:39eb
	Item::I_setQLo,                 // Intrinsic043 10a8:38a2
	Item::I_getFamily,              // Intrinsic044 10a8:0fc4
	Item::I_fall,                   // Intrinsic045 1040:1334
	Egg::I_getEggId,                // Intrinsic046 1098:05c5
	CameraProcess::I_moveTo,        // Intrinsic047 1188:0cba
	CameraProcess::I_setCenterOn,   // Intrinsic048 1188:1aa5
	Item::I_getRangeIfVisible,      // Intrinsic049 1130:1c01
	AudioProcess::I_playSFXCru,     // Intrinsic04A 10a8:3991
	Item::I_isOn,                   // Intrinsic04B 10a8:1eea
	Item::I_getQHi,                 // Intrinsic04C 10a8:383e
	Item::I_isOn,                   // Intrinsic04D 10a8:1eea
	Item::I_getQHi,                 // Intrinsic04E 10a8:383e
	Item::I_isOn,                   // Intrinsic04F 10a8:1eea
	// 0x050
	Item::I_getQHi,                 // Intrinsic050 10a8:383e
	Item::I_isOn,                   // Intrinsic051 10a8:1eea
	Item::I_getQHi,                 // Intrinsic052 10a8:383e
	Item::I_isOn,                   // Intrinsic053 10a8:1eea
	Item::I_getQHi,                 // Intrinsic054 10a8:383e
	Item::I_isOn,                   // Intrinsic055 10a8:1eea
	Item::I_getQHi,                 // Intrinsic056 10a8:383e
	Kernel::I_resetRef,             // Intrinsic057 11d8:0c63
	Actor::I_getCurrentActivityNo,  // Intrinsic058 10f0:330d
	Actor::I_clrInCombat,           // Intrinsic059 10f0:31a9
	Actor::I_setDefaultActivity0,   // Intrinsic05A 10f0:2bea
	Actor::I_setDefaultActivity1,   // Intrinsic05B 10f0:2c07
	Actor::I_setDefaultActivity2,   // Intrinsic05C 10f0:2c24
	Actor::I_setActivity,           // Intrinsic05D 10f0:2d02
	World::I_setControlledNPCNum,   // Intrinsic05E 1138:0000
	Item::I_getSurfaceWeight,       // Intrinsic05F 10a8:0602
	// 0x060
	Item::I_isCentreOn,             // Intrinsic060 10a8:4377
	Item::I_setFrame,               // Intrinsic061 10a8:0810
	Actor::I_getLastAnimSet,        // Intrinsic062 10f0:372e
	Item::I_legalCreateAtPoint,     // Intrinsic063 10a8:104e
	Item::I_getPoint,               // Intrinsic064 10a8:2436
	Actor::I_setDead,               // Intrinsic065 10f0:2010
	Actor::I_createActor,           // Intrinsic066 10f0:25b9
	0, //I_forceCameraUpdate,       // Intrinsic067 1188:0013
	Actor::I_teleport,              // Intrinsic068 10f0:29fa
	Item::I_getFootpadData,         // Intrinsic069 10a8:1aa9
	Item::I_isInNpc,                // Intrinsic06A 10a8:24b5
	Item::I_getQLo,                 // Intrinsic06B 10a8:3889
	Item::I_getNpcNum,              // Intrinsic06C 10a8:0c40
	Item::I_setNpcNum,              // Intrinsic06D 10a8:0c24
	Item::I_andStatus,              // Intrinsic06E 10a8:258b
	Item::I_move,                   // Intrinsic06F 10a8:1531
	// 0x070
	UCMachine::I_true,              // Intrinsic070 1050:00a1
	Item::I_getNpcNum,              // Intrinsic071 10a8:0c40
	Item::I_andStatus,              // Intrinsic072 10a8:258b
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic073 13e8:0e27
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic074 13e8:0e2b
	Ultima8Engine::I_setCruStasis,  // Intrinsic075 13e8:0071
	Actor::I_setDead,               // Intrinsic076 10f0:2010
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic077 13e8:0e21
	Ultima8Engine::I_clrCruStasis,  // Intrinsic078 13e8:0077
	AudioProcess::I_stopSFXCru,     // Intrinsic079 10a8:39cf
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic07A 1440:06e6
	MainActor::I_clrKeycards,       // Intrinsic07B 10f0:3d4b
	MainActor::I_teleportToEgg,     // Intrinsic07C 1098:04ce
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic07D 1440:07cc
	Actor::I_clrImmortal,           // Intrinsic07E 10f0:2185
	Actor::I_getHp,                 // Intrinsic07F 10f0:32f4
	// 0x080
	Actor::I_setActivity,           // Intrinsic080 10f0:2d02
	Item::I_getQuality,             // Intrinsic081 10a8:0caa
	Item::I_setQuality,             // Intrinsic082 10a8:0cee
	Item::I_use,                    // Intrinsic083 10a8:2667
	MainActor::I_getMaxEnergy,      // Intrinsic084 10f0:348b
	Actor::I_getMana,               // Intrinsic085 10f0:3454
	Actor::I_setMana,               // Intrinsic086 10f0:346e
	Item::I_getQLo,                 // Intrinsic087 10a8:3889
	Actor::I_setImmortal,			// Intrinsic088 10f0:216a
	CameraProcess::I_getCameraX,    // Intrinsic089 1188:1cc5
	CameraProcess::I_getCameraY,    // Intrinsic08A 1188:1ccd
	Item::I_setMapArray,            // Intrinsic08B 10a8:0c5c
	Item::I_getNpcNum,              // Intrinsic08C 10a8:0c40
	Item::I_shoot,                  // Intrinsic08D 10a8:38d1
	Item::I_enterFastArea,          // Intrinsic08E 10a8:2b30
	Item::I_setBroken,              // Intrinsic08F 10a8:25d0
	// 0x090
	Item::I_hurl,                   // Intrinsic090 1040:12a6
	Item::I_getNpcNum,              // Intrinsic091 10a8:0c40
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic092 1440:07da
	MusicProcess::I_stopMusic,      // Intrinsic093 1200:0223
	0, //I_PauseCycler,             // Intrinsic094 13e8:0033
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic095 10a8:3a9f
	0, //I_ResumeCycler,             // Intrinsic096 13e8:0039
	Game::I_playCredits,            // Intrinsic097 13e8:003f
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic098 10a8:4340
	MainActor::I_teleportToEgg,     // Intrinsic099 1098:04f7
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic09A 1030:0605
	World::I_resetVargasShield,     // Intrinsic09B 1130:0000
	Item::I_andStatus,              // Intrinsic09C 10a8:258b
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic09D 1440:0a7c
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09E 1440:06f4
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09F 1440:0bc6
	// 0x0A0
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic0A0 1440:04ea
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic0A1 1440:0c47
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic0A2 1440:0ab2
	Actor::I_setDead,               // Intrinsic0A3 10f0:2010
	Item::I_getQLo,                 // Intrinsic0A4 10a8:3889
	Item::I_getUnkEggType,          // Intrinsic0A5 10a8:0d31
	Egg::I_setEggXRange,            // Intrinsic0A6 1098:055b
	Item::I_overlaps,               // Intrinsic0A7 10a8:1cc5
	Item::I_isOn,                   // Intrinsic0A8 10a8:1eea
	UCMachine::I_true,              // Intrinsic0A9 13e8:00b9
	KeypadGump::I_showKeypad,       // Intrinsic0AA 13c8:00fe
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic0AB 10a8:3a9f
	Egg::I_getEggXRange,            // Intrinsic0AC 1098:051d
	Actor::I_setDead,               // Intrinsic0AD 10f0:2010
	AudioProcess::I_playSFX,        // Intrinsic0AE 12e0:0476
	Actor::I_isFalling,             // Intrinsic0AF 10f0:1f01
	// 0x0B0
	Item::I_getFamilyOfType,        // Intrinsic0B0 1100:054d
	Item::I_getNpcNum,              // Intrinsic0B1 10a8:0c40
	Item::I_getQLo,                 // Intrinsic0B2 10a8:3889
	Item::I_getQHi,                 // Intrinsic0B3 10a8:383e
	Item::I_unequip,                // Intrinsic0B4 10a8:2a68
	Item::I_avatarStoleSomething,   // Intrinsic0B5 10a8:2fda
	Item::I_andStatus,              // Intrinsic0B6 10a8:258b
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic0B7 1038:03b8
	World::I_setAlertActive,        // Intrinsic0B8 10e0:0000
	Item::I_equip,                  // Intrinsic0B9 10a8:2a35
	World::I_clrAlertActive,        // Intrinsic0BA 10e0:0121
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic0BB 10f0:3696
	MainActor::I_addItemCru,        // Intrinsic0BC 13e8:0330
	Actor::I_getLastAnimSet,        // Intrinsic0BD 10f0:372e
	Item::I_setQuality,             // Intrinsic0BE 10a8:0cee
	CurrentMap::I_canExistAt,       // Intrinsic0BF 1150:0b67
	// 0x0C0
	Item::I_isOn,                   // Intrinsic0C0 10a8:1eea
	Item::I_hurl,                   // Intrinsic0C1 1040:12a6
	Item::I_getQHi,                 // Intrinsic0C2 10a8:383e
	Item::I_andStatus,              // Intrinsic0C3 10a8:258b
	Item::I_hurl,                   // Intrinsic0C4 1040:12a6
	Item::I_andStatus,              // Intrinsic0C5 10a8:258b
	Item::I_hurl,                   // Intrinsic0C6 1040:12a6
	Item::I_andStatus,              // Intrinsic0C7 10a8:258b
	Item::I_getDirToCoords,         // Intrinsic0C8 10a8:19e9
	Item::I_popToContainer,         // Intrinsic0C9 10a8:1328
	Item::I_getNpcNum,              // Intrinsic0CA 10a8:0c40
	Item::I_setBroken,              // Intrinsic0CB 10a8:25d0
	Item::I_getCY,                  // Intrinsic0CC 10a8:02de
	Item::I_getCX,                  // Intrinsic0CD 10a8:029a
	SpriteProcess::I_createSprite,  // Intrinsic0CE 1140:00de
	Item::I_isOn,                   // Intrinsic0CF 10a8:1eea
	// 0x0D0
	Item::I_getFootpadData,         // Intrinsic0D0 10a8:1aa9
	Actor::I_isDead,                // Intrinsic0D1 10f0:1fe0
	Actor::I_createActorCru,        // Intrinsic0D2 10a8:3d3f
	Actor::I_setActivity,           // Intrinsic0D3 10f0:2d02
	Item::I_andStatus,              // Intrinsic0D4 10a8:258b
	Item::I_setQuality,             // Intrinsic0D5 10a8:0cee
	Actor::I_getLastAnimSet,        // Intrinsic0D6 10f0:372e
	Item::I_getDirFromItem,         // Intrinsic0D7 10a8:1a3f
	Item::I_hurl,                   // Intrinsic0D8 1040:12a6
	Item::I_getQHi,                 // Intrinsic0D9 10a8:383e
	Actor::I_addHp,                 // Intrinsic0DA 10f0:3748
	MainActor::I_switchMap,			// Intrinsic0DB 1028:049c
	Actor::I_setActivity,           // Intrinsic0DC 10f0:2d02
	Actor::I_isInCombat,            // Intrinsic0DD 10f0:1f20
	Item::I_setQAndCombine,         // Intrinsic0DE 10a8:0dd1
	Item::I_use,                    // Intrinsic0DF 10a8:2667
	// 0x0E0
	AudioProcess::I_stopAllSFX,     // Intrinsic0E0 12e0:054a
	MovieGump::I_playMovieCutscene, // Intrinsic0E1 10a8:3a5b
	0, // I_clearKeyboardState,     // Intrinsic0E2 13e8:00a8
	AudioProcess::I_playSFX,        // Intrinsic0E3 12e0:0476
	Item::I_use,                    // Intrinsic0E4 10a8:2667
	CameraProcess::I_getCameraZ,    // Intrinsic0E5 1188:1cd5
	Actor::I_getLastAnimSet,        // Intrinsic0E6 10f0:372e
	Actor::I_setDead,               // Intrinsic0E7 10f0:2010
	Item::I_getQLo,                 // Intrinsic0E8 10a8:3889
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic0E9 1440:0853
	Item::I_equip,                  // Intrinsic0EA 10a8:2a35
	Actor::I_setActivity,           // Intrinsic0EB 10f0:2d02
	Item::I_isOn,                   // Intrinsic0EC 10a8:1eea
	Actor::I_getLastActivityNo,     // Intrinsic0ED 10f0:3e35
	Actor::I_setCombatTactic,       // Intrinsic0EE 10f0:3594
	Actor::I_getEquip,              // Intrinsic0EF 10f0:27c8
	// 0x0F0
	Actor::I_setEquip,              // Intrinsic0F0 10f0:27fb
	Actor::I_getDefaultActivity0,   // Intrinsic0F1 10f0:2c78
	Actor::I_getDefaultActivity1,   // Intrinsic0F2 10f0:2c92
	Actor::I_getDefaultActivity2,   // Intrinsic0F3 10f0:2cac
	Actor::I_getLastAnimSet,        // Intrinsic0F4 10f0:372e
	Item::I_getQHi,                 // Intrinsic0F5 10a8:383e
	Item::I_getNpcNum,              // Intrinsic0F6 10a8:0c40
	Item::I_cast,                   // Intrinsic0F7 10a8:2f3e
	MainActor::I_getNumberOfCredits, // Intrinsic0F8 10f0:3833
	Actor::I_setTarget,             // Intrinsic0F9 10f0:1e71
	Actor::I_setUnkByte,            // Intrinsic0FA 10f0:3e18
	Item::I_hurl,                   // Intrinsic0FB 1040:12a6
	Actor::I_setDead,               // Intrinsic0FC 10f0:2010
	Item::I_andStatus,              // Intrinsic0FD 10a8:258b
	Item::I_getQLo,                 // Intrinsic0FE 10a8:3889
	Item::I_popToEnd,               // Intrinsic0FF 10a8:1342
	// 0x100
	BatteryChargerProcess::I_create, // Intrinsic100 1130:20c5
	Kernel::I_getNumProcesses,      // Intrinsic101 11d8:0977
	Item::I_isOn,                   // Intrinsic102 10a8:1eea
	Actor::I_setActivity,           // Intrinsic103 10f0:2d02
	Item::I_getQHi,                 // Intrinsic104 10a8:383e
	Item::I_getQ,                   // Intrinsic105 10a8:0c94
	Item::I_setQ,                   // Intrinsic106 10a8:0f5f
	CruHealerProcess::I_create,     // Intrinsic107 1130:20e5
	Item::I_hurl,                   // Intrinsic108 1040:12a6
	Item::I_getNpcNum,              // Intrinsic109 10a8:0c40
	Item::I_hurl,                   // Intrinsic10A 1040:12a6
	Item::I_isOn,                   // Intrinsic10B 10a8:1eea
	Item::I_andStatus,              // Intrinsic10C 10a8:258b
	Item::I_getQHi,                 // Intrinsic10D 10a8:383e
	MainActor::I_hasKeycard,        // Intrinsic10E 10f0:3ce8
	Actor::I_setDead,               // Intrinsic10F 10f0:2010
	// 0x110
	Actor::I_clrImmortal,			// Intrinsic110 10f0:2185
	Item::I_getQLo,                 // Intrinsic111 10a8:3889
	Item::I_getNpcNum,              // Intrinsic112 10a8:0c40
	Item::I_andStatus,              // Intrinsic113 10a8:258b
	Item::I_getNpcNum,              // Intrinsic114 10a8:0c40
	Item::I_isCrusTypeNPC,			// Intrinsic115 10a8:4348
	Item::I_andStatus,              // Intrinsic116 10a8:258b
	Item::I_getNpcNum,              // Intrinsic117 10a8:0c40
	Item::I_andStatus,              // Intrinsic118 10a8:258b
	Item::I_getNpcNum,              // Intrinsic119 10a8:0c40
	Item::I_andStatus,              // Intrinsic11A 10a8:258b
	Item::I_getNpcNum,              // Intrinsic11B 10a8:0c40
	Item::I_andStatus,              // Intrinsic11C 10a8:258b
	Item::I_getNpcNum,              // Intrinsic11D 10a8:0c40
	Item::I_andStatus,              // Intrinsic11E 10a8:258b
	Item::I_getNpcNum,              // Intrinsic11F 10a8:0c40
	// 0x120
	Item::I_andStatus,              // Intrinsic120 10a8:258b
	Item::I_getNpcNum,              // Intrinsic121 10a8:0c40
	Item::I_andStatus,              // Intrinsic122 10a8:258b
	Item::I_getNpcNum,              // Intrinsic123 10a8:0c40
	Item::I_andStatus,              // Intrinsic124 10a8:258b
	Item::I_getNpcNum,              // Intrinsic125 10a8:0c40
	Item::I_fireDistance,           // Intrinsic126 1130:1755
	Item::I_andStatus,              // Intrinsic127 10a8:258b
	Item::I_hurl,                   // Intrinsic128 1040:12a6
	Item::I_setBroken,              // Intrinsic129 10a8:25d0
	Item::I_andStatus,              // Intrinsic12A 10a8:258b
	WeaselGump::I_showWeaselGump,   // Intrinsic12B 13e0:0000
	Item::I_getTypeFlag,            // Intrinsic12C 10a8:0fef
	Item::I_getNpcNum,              // Intrinsic12D 10a8:0c40
	Item::I_hurl,                   // Intrinsic12E 1040:12a6
	Item::I_getCY,                  // Intrinsic12F 10a8:02de
	// 0x130
	Item::I_getCZ,                  // Intrinsic130 10a8:0322
	0 //UNUSEDInt136,				// Intrinsic131 1048:0079
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Intrinsics for the Remorse French version.
Intrinsic RemorseFrIntrinsics[] = {
	// 0x000
	World::I_getAlertActive,        // Intrinsic000 10e0:0243
	Item::I_getFrame,               // Intrinsic001 10a8:07b4
	Item::I_setFrame,               // Intrinsic002 10a8:0810
	Item::I_getMapArray,            // Intrinsic003 10a8:0c78
	Item::I_getStatus,              // Intrinsic004 10a8:1038
	Item::I_orStatus,               // Intrinsic005 10a8:2572
	Item::I_equip,                  // Intrinsic006 10a8:2a35
	Item::I_isPartlyOnScreen,       // Intrinsic007 10a8:40ce
	Actor::I_isNPC,                 // Intrinsic008 10a8:2486
	Item::I_getZ,                   // Intrinsic009 10a8:0266
	Item::I_destroy,                // Intrinsic00A 10a8:1365
	Actor::I_getUnkByte,            // Intrinsic00B 10f0:3dfc
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic00C 10f0:369e
	Item::I_getDirToItem,           // Intrinsic00D 10a8:1a74
	Actor::I_turnToward,            // Intrinsic00E 10f0:326b
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic00F 10a8:3abb
	// 0x010
	Item::I_getQLo,                 // Intrinsic010 10a8:3889
	Actor::I_getMap,                // Intrinsic011 10f0:2b90
	MusicProcess::I_playMusic,      // Intrinsic012 1200:0143
	Item::I_getX,                   // Intrinsic013 10a8:013e
	Item::I_getY,                   // Intrinsic014 10a8:0176
	AudioProcess::I_playSFXCru,     // Intrinsic015 10a8:3934
	Item::I_getShape,               // Intrinsic016 10a8:068f
	Item::I_explode,                // Intrinsic017 1150:0859
	UCMachine::I_rndRange,          // Intrinsic018 12e0:0293
	Item::I_legalCreateAtCoords,    // Intrinsic019 10a8:10db
	Item::I_andStatus,              // Intrinsic01A 10a8:258b
	World::I_getControlledNPCNum,   // Intrinsic01B 1138:0010
	Actor::I_getDir,                // Intrinsic01C 10f0:3714
	Actor::I_getLastAnimSet,        // Intrinsic01D 10f0:372e
	Item::I_fireWeapon,             // Intrinsic01E 1130:11da
	Item::I_create,                 // Intrinsic01F 10a8:11e7
	// 0x020
	Item::I_popToCoords,            // Intrinsic020 10a8:1311
	Actor::I_setDead,               // Intrinsic021 10f0:2010
	Item::I_push,                   // Intrinsic022 10a8:1352
	Item::I_getEtherealTop,         // Intrinsic023 10b0:2622
	Item::I_setShape,               // Intrinsic024 10a8:06a5
	Item::I_touch,                  // Intrinsic025 10a8:2514
	Item::I_getQHi,                 // Intrinsic026 10a8:383e
	Item::I_getClosestDirectionInRange, // Intrinsic027 10d8:0000
	Item::I_hurl,                   // Intrinsic028 1040:12a6
	World::I_gameDifficulty,        // Intrinsic029 10f0:38d5
	AudioProcess::I_playAmbientSFXCru, // Intrinsic02A 10a8:3953
	Item::I_isCompletelyOn,         // Intrinsic02B 10a8:1f5f
	UCMachine::I_true,              // Intrinsic02C 1050:00c1
	Container::I_destroyContents,   // Intrinsic02D 10a8:14af
	Item::I_getQLo,                 // Intrinsic02E 10a8:3889
	Item::I_inFastArea,             // Intrinsic02F 10a8:3af5
	// 0x030
	Item::I_setQHi,                 // Intrinsic030 10a8:385a
	Item::I_legalMoveToPoint,       // Intrinsic031 10a8:174d
	CurrentMap::I_canExistAtPoint,  // Intrinsic032 10a8:4071
	Item::I_pop,                    // Intrinsic033 10a8:1338
	Item::I_andStatus,              // Intrinsic034 10a8:258b
	Item::I_receiveHit,             // Intrinsic035 1130:0247
	Actor::I_isBusy,                // Intrinsic036 10f0:010c
	Item::I_getDirFromTo16,         // Intrinsic037 10d8:01c7
	Actor::I_isKneeling,            // Intrinsic038 10f0:21fb
	Actor::I_doAnim,                // Intrinsic039 10f0:0245
	MainActor::I_addItemCru,        // Intrinsic03A 13e8:0330
	AudioProcess::I_stopSFXCru,     // Intrinsic03B 10a8:39b0
	Actor::I_isDead,                // Intrinsic03C 10f0:1fe0
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic03D 10a8:39eb
	Item::I_setQLo,                 // Intrinsic03E 10a8:38a2
	Item::I_getFamily,              // Intrinsic03F 10a8:0fc4
	// 0x040
	Item::I_fall,                   // Intrinsic040 1040:1334
	Egg::I_getEggId,                // Intrinsic041 1098:05c5
	CameraProcess::I_moveTo,        // Intrinsic042 1188:0cba
	CameraProcess::I_setCenterOn,   // Intrinsic043 1188:1aa5
	Item::I_getRangeIfVisible,      // Intrinsic044 1130:1c01
	AudioProcess::I_playSFXCru,     // Intrinsic045 10a8:3991
	Item::I_isOn,                   // Intrinsic046 10a8:1eea
	Item::I_getQHi,                 // Intrinsic047 10a8:383e
	Item::I_isOn,                   // Intrinsic048 10a8:1eea
	Item::I_getQHi,                 // Intrinsic049 10a8:383e
	Item::I_isOn,                   // Intrinsic04A 10a8:1eea
	Item::I_getQHi,                 // Intrinsic04B 10a8:383e
	Item::I_isOn,                   // Intrinsic04C 10a8:1eea
	Item::I_getQHi,                 // Intrinsic04D 10a8:383e
	Item::I_isOn,                   // Intrinsic04E 10a8:1eea
	Item::I_getQHi,                 // Intrinsic04F 10a8:383e
	// 0x050
	Item::I_isOn,                   // Intrinsic050 10a8:1eea
	Item::I_getQHi,                 // Intrinsic051 10a8:383e
	Actor::I_getCurrentActivityNo,  // Intrinsic052 10f0:330d
	Actor::I_clrInCombat,           // Intrinsic053 10f0:31a9
	Actor::I_setDefaultActivity0,   // Intrinsic054 10f0:2bea
	Actor::I_setDefaultActivity1,   // Intrinsic055 10f0:2c07
	Actor::I_setDefaultActivity2,   // Intrinsic056 10f0:2c24
	Actor::I_setActivity,           // Intrinsic057 10f0:2d02
	World::I_setControlledNPCNum,   // Intrinsic058 1138:0000
	Item::I_getSurfaceWeight,       // Intrinsic059 10a8:0602
	Item::I_isCentreOn,             // Intrinsic05A 10a8:4377
	Item::I_setFrame,               // Intrinsic05B 10a8:0810
	Actor::I_getLastAnimSet,        // Intrinsic05C 10f0:372e
	Item::I_legalCreateAtPoint,     // Intrinsic05D 10a8:104e
	Item::I_getPoint,               // Intrinsic05E 10a8:2436
	CruStatusGump::I_hideStatusGump, // Intrinsic05F 11b8:0000
	// 0x060
	MovieGump::I_playMovieOverlay,  // Intrinsic060 10a8:3ad7
	CruStatusGump::I_showStatusGump, // Intrinsic061 11b8:0006
	Actor::I_setDead,               // Intrinsic062 10f0:2010
	Actor::I_createActor,           // Intrinsic063 10f0:25b9
	0,                              // Intrinsic064 1188:0013
	Actor::I_teleport,              // Intrinsic065 10f0:29fa
	Item::I_getFootpadData,         // Intrinsic066 10a8:1aa9
	Item::I_isInNpc,                // Intrinsic067 10a8:24b5
	Item::I_getQLo,                 // Intrinsic068 10a8:3889
	Item::I_getNpcNum,              // Intrinsic069 10a8:0c40
	Item::I_setNpcNum,              // Intrinsic06A 10a8:0c24
	Item::I_andStatus,              // Intrinsic06B 10a8:258b
	Item::I_move,                   // Intrinsic06C 10a8:1531
	UCMachine::I_true,              // Intrinsic06D 1050:00a1
	Kernel::I_resetRef,             // Intrinsic06E 11d8:0c63
	Item::I_getNpcNum,              // Intrinsic06F 10a8:0c40
	// 0x070
	Item::I_andStatus,              // Intrinsic070 10a8:258b
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic071 13e8:0e27
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic072 13e8:0e2b
	Ultima8Engine::I_setCruStasis,  // Intrinsic073 13e8:0071
	Actor::I_setDead,               // Intrinsic074 10f0:2010
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic075 13e8:0e21
	Ultima8Engine::I_clrCruStasis,  // Intrinsic076 13e8:0077
	AudioProcess::I_stopSFXCru,     // Intrinsic077 10a8:39cf
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic078 1440:06e6
	MainActor::I_clrKeycards,       // Intrinsic079 10f0:3d4b
	MainActor::I_teleportToEgg,     // Intrinsic07A 1098:04ce
	PaletteFaderProcess::I_fadeFromBlack, // Intrinsic07B 1440:07cc
	Actor::I_clrImmortal,           // Intrinsic07C 10f0:2185
	Actor::I_getHp,                 // Intrinsic07D 10f0:32f4
	Actor::I_setActivity,           // Intrinsic07E 10f0:2d02
	Item::I_getQuality,             // Intrinsic07F 10a8:0caa
	// 0x080
	Item::I_setQuality,             // Intrinsic080 10a8:0cee
	Item::I_use,                    // Intrinsic081 10a8:2667
	MainActor::I_getMaxEnergy,      // Intrinsic082 10f0:348b
	Actor::I_getMana,               // Intrinsic083 10f0:3454
	Actor::I_setMana,               // Intrinsic084 10f0:346e
	Item::I_getQLo,                 // Intrinsic085 10a8:3889
	Actor::I_setImmortal,           // Intrinsic086 10f0:216a
	CameraProcess::I_getCameraX,    // Intrinsic087 1188:1cc5
	CameraProcess::I_getCameraY,    // Intrinsic088 1188:1ccd
	Item::I_setMapArray,            // Intrinsic089 10a8:0c5c
	Item::I_getNpcNum,              // Intrinsic08A 10a8:0c40
	Item::I_shoot,                  // Intrinsic08B 10a8:38d1
	Item::I_enterFastArea,          // Intrinsic08C 10a8:2b30
	Item::I_setBroken,              // Intrinsic08D 10a8:25d0
	Item::I_hurl,                   // Intrinsic08E 1040:12a6
	Item::I_getNpcNum,              // Intrinsic08F 10a8:0c40
	// 0x090
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic090 1440:07da
	MusicProcess::I_stopMusic,      // Intrinsic091 1200:0223
	0,                              // Intrinsic092 13e8:0033
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic093 10a8:3a9f
	0,                              // Intrinsic094 13e8:0039
	Game::I_playCredits,            // Intrinsic095 13e8:003f
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic096 10a8:4340
	MainActor::I_teleportToEgg,     // Intrinsic097 1098:04f7
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic098 1030:0605
	World::I_resetVargasShield,     // Intrinsic099 1130:0000
	Item::I_andStatus,              // Intrinsic09A 10a8:258b
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic09B 1440:0a7c
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09C 1440:06f4
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic09D 1440:0bc6
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic09E 1440:04ea
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic09F 1440:0c47
	// 0x0A0
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic0A0 1440:0ab2
	Actor::I_setDead,               // Intrinsic0A1 10f0:2010
	Item::I_getQLo,                 // Intrinsic0A2 10a8:3889
	Item::I_getUnkEggType,          // Intrinsic0A3 10a8:0d31
	Egg::I_setEggXRange,            // Intrinsic0A4 1098:055b
	Item::I_overlaps,               // Intrinsic0A5 10a8:1cc5
	Item::I_isOn,                   // Intrinsic0A6 10a8:1eea
	UCMachine::I_true,              // Intrinsic0A7 13e8:00b9
	Egg::I_getEggXRange,            // Intrinsic0A8 1098:051d
	Actor::I_setDead,               // Intrinsic0A9 10f0:2010
	MovieGump::I_playMovieCutsceneAlt, // Intrinsic0AA 10a8:3a9f
	AudioProcess::I_playSFX,        // Intrinsic0AB 12e0:0476
	Actor::I_isFalling,             // Intrinsic0AC 10f0:1f01
	Item::I_getFamilyOfType,        // Intrinsic0AD 1100:054d
	Item::I_getNpcNum,              // Intrinsic0AE 10a8:0c40
	Item::I_getQLo,                 // Intrinsic0AF 10a8:3889
	// 0x0B0
	Item::I_getQHi,                 // Intrinsic0B0 10a8:383e
	Item::I_unequip,                // Intrinsic0B1 10a8:2a68
	Item::I_avatarStoleSomething,   // Intrinsic0B2 10a8:2fda
	Item::I_andStatus,              // Intrinsic0B3 10a8:258b
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic0B4 1038:03b8
	World::I_setAlertActive,        // Intrinsic0B5 10e0:0000
	Item::I_equip,                  // Intrinsic0B6 10a8:2a35
	World::I_clrAlertActive,        // Intrinsic0B7 10e0:0121
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic0B8 10f0:3696
	MainActor::I_addItemCru,        // Intrinsic0B9 13e8:0330
	Actor::I_getLastAnimSet,        // Intrinsic0BA 10f0:372e
	Item::I_setQuality,             // Intrinsic0BB 10a8:0cee
	CurrentMap::I_canExistAt,       // Intrinsic0BC 1150:0b67
	Item::I_isOn,                   // Intrinsic0BD 10a8:1eea
	Item::I_hurl,                   // Intrinsic0BE 1040:12a6
	Item::I_getQHi,                 // Intrinsic0BF 10a8:383e
	// 0x0C0
	Item::I_andStatus,              // Intrinsic0C0 10a8:258b
	Item::I_hurl,                   // Intrinsic0C1 1040:12a6
	Item::I_andStatus,              // Intrinsic0C2 10a8:258b
	Item::I_hurl,                   // Intrinsic0C3 1040:12a6
	Item::I_andStatus,              // Intrinsic0C4 10a8:258b
	KeypadGump::I_showKeypad,       // Intrinsic0C5 13c8:00fe
	Item::I_isOn,                   // Intrinsic0C6 10a8:1eea
	SpriteProcess::I_createSprite,  // Intrinsic0C7 1140:00de
	Item::I_getDirFromItem,         // Intrinsic0C8 10a8:1a3f
	Item::I_hurl,                   // Intrinsic0C9 1040:12a6
	Item::I_getQHi,                 // Intrinsic0CA 10a8:383e
	Actor::I_addHp,                 // Intrinsic0CB 10f0:3748
	MainActor::I_switchMap,         //Intrinsic0CC 1028:049c
	Actor::I_isInCombat,            // Intrinsic0CD 10f0:1f20
	Actor::I_setActivity,           // Intrinsic0CE 10f0:2d02
	Item::I_setQAndCombine,         // Intrinsic0CF 10a8:0dd1
	// 0x0D0
	Item::I_use,                    // Intrinsic0D0 10a8:2667
	AudioProcess::I_stopAllSFX,     // Intrinsic0D1 12e0:054a
	MovieGump::I_playMovieCutscene, // Intrinsic0D2 10a8:3a5b
	0,                              // Intrinsic0D3 13e8:00a8
	AudioProcess::I_playSFX,        // Intrinsic0D4 12e0:0476
	Item::I_use,                    // Intrinsic0D5 10a8:2667
	CameraProcess::I_getCameraZ,    // Intrinsic0D6 1188:1cd5
	Actor::I_getLastAnimSet,        // Intrinsic0D7 10f0:372e
	Actor::I_setDead,               // Intrinsic0D8 10f0:2010
	Item::I_getQLo,                 // Intrinsic0D9 10a8:3889
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic0DA 1440:0853
	Actor::I_setActivity,           // Intrinsic0DB 10f0:2d02
	Item::I_isOn,                   // Intrinsic0DC 10a8:1eea
	Actor::I_getLastActivityNo,     // Intrinsic0DD 10f0:3e35
	Actor::I_setCombatTactic,       // Intrinsic0DE 10f0:3594
	Actor::I_getEquip,              // Intrinsic0DF 10f0:27c8
	// 0x0E0
	Actor::I_setEquip,              // Intrinsic0E0 10f0:27fb
	Actor::I_getDefaultActivity0,   // Intrinsic0E1 10f0:2c78
	Actor::I_getDefaultActivity1,   // Intrinsic0E2 10f0:2c92
	Actor::I_getDefaultActivity2,   // Intrinsic0E3 10f0:2cac
	Actor::I_getLastAnimSet,        // Intrinsic0E4 10f0:372e
	Actor::I_setTarget,             // Intrinsic0E5 10f0:1e71
	Actor::I_setUnkByte,            // Intrinsic0E6 10f0:3e18
	Actor::I_setDead,               // Intrinsic0E7 10f0:2010
	Item::I_cast,                   // Intrinsic0E8 10a8:2f3e
	Item::I_andStatus,              // Intrinsic0E9 10a8:258b
	Item::I_getQLo,                 // Intrinsic0EA 10a8:3889
	MainActor::I_getNumberOfCredits, // Intrinsic0EB 10f0:3833
	Item::I_popToEnd,               // Intrinsic0EC 10a8:1342
	Item::I_popToContainer,         // Intrinsic0ED 10a8:1328
	BatteryChargerProcess::I_create, // Intrinsic0EE 1130:20c5
	Kernel::I_getNumProcesses,      // Intrinsic0EF 11d8:0977
	// 0x0F0
	Item::I_getQHi,                 // Intrinsic0F0 10a8:383e
	Item::I_isOn,                   // Intrinsic0F1 10a8:1eea
	Actor::I_setActivity,           // Intrinsic0F2 10f0:2d02
	Item::I_getQHi,                 // Intrinsic0F3 10a8:383e
	Item::I_getQ,                   // Intrinsic0F4 10a8:0c94
	Item::I_setQ,                   // Intrinsic0F5 10a8:0f5f
	CruHealerProcess::I_create,     // Intrinsic0F6 1130:20e5
	Item::I_hurl,                   // Intrinsic0F7 1040:12a6
	Item::I_getNpcNum,              // Intrinsic0F8 10a8:0c40
	Item::I_hurl,                   // Intrinsic0F9 1040:12a6
	Item::I_isOn,                   // Intrinsic0FA 10a8:1eea
	Item::I_getQHi,                 // Intrinsic0FB 10a8:383e
	Item::I_andStatus,              // Intrinsic0FC 10a8:258b
	MainActor::I_hasKeycard,        // Intrinsic0FD 10f0:3ce8
	ComputerGump::I_readComputer,   // Intrinsic0FE 13a0:0005
	UCMachine::I_numToStr,          // Intrinsic0FF 1418:0073
	// 0x100
	Item::I_getNpcNum,              // Intrinsic100 10a8:0c40
	Item::I_andStatus,              // Intrinsic101 10a8:258b
	Item::I_getNpcNum,              // Intrinsic102 10a8:0c40
	Item::I_isCrusTypeNPC, 			// Intrinsic103 10a8:4348
	Item::I_andStatus,              // Intrinsic104 10a8:258b
	Item::I_getNpcNum,              // Intrinsic105 10a8:0c40
	Item::I_andStatus,              // Intrinsic106 10a8:258b
	Item::I_getNpcNum,              // Intrinsic107 10a8:0c40
	Item::I_andStatus,              // Intrinsic108 10a8:258b
	Item::I_getNpcNum,              // Intrinsic109 10a8:0c40
	Item::I_andStatus,              // Intrinsic10A 10a8:258b
	Item::I_getNpcNum,              // Intrinsic10B 10a8:0c40
	Item::I_andStatus,              // Intrinsic10C 10a8:258b
	Item::I_getNpcNum,              // Intrinsic10D 10a8:0c40
	Item::I_andStatus,              // Intrinsic10E 10a8:258b
	Item::I_getNpcNum,              // Intrinsic10F 10a8:0c40
	// 0x110
	Item::I_andStatus,              // Intrinsic110 10a8:258b
	Item::I_getNpcNum,              // Intrinsic111 10a8:0c40
	Actor::I_getDir,                // Intrinsic112 10f0:3714
	UCMachine::I_numToStr,          // Intrinsic113 1418:0073
	Item::I_andStatus,              // Intrinsic114 10a8:258b
	Item::I_getNpcNum,              // Intrinsic115 10a8:0c40
	Item::I_fireDistance,           // Intrinsic116 1130:1755
	Item::I_andStatus,              // Intrinsic117 10a8:258b
	Item::I_hurl,                   // Intrinsic118 1040:12a6
	Item::I_setBroken,              // Intrinsic119 10a8:25d0
	Item::I_andStatus,              // Intrinsic11A 10a8:258b
	Item::I_getTypeFlag,            // Intrinsic11B 10a8:0fef
	Item::I_getNpcNum,              // Intrinsic11C 10a8:0c40
	Item::I_hurl,                   // Intrinsic11D 1040:12a6
	Item::I_getCY,                  // Intrinsic11E 10a8:02de
	Item::I_getCZ,                  // Intrinsic11F 10a8:0322
	// 0x120
	Item::I_getCX,                  // Intrinsic120 10a8:029a
	Actor::I_getDir,                // Intrinsic121 10f0:3714
	UCMachine::I_numToStr,          // Intrinsic122 1418:0073
	Item::I_getDirToCoords,         // Intrinsic123 10a8:19e9
	Item::I_andStatus,              // Intrinsic124 10a8:258b
	Item::I_getNpcNum,              // Intrinsic125 10a8:0c40
	Item::I_isOn,                   // Intrinsic126 10a8:1eea
	Item::I_getFootpadData,         // Intrinsic127 10a8:1aa9
	Actor::I_isDead,                // Intrinsic128 10f0:1fe0
	Actor::I_createActorCru,        // Intrinsic129 10a8:3d3f
	Actor::I_clrImmortal,           // Intrinsic12A 10f0:2185
	Item::I_andStatus,              // Intrinsic12B 10a8:258b
	Item::I_getQHi,                 // Intrinsic12C 10a8:383e
	WeaselGump::I_showWeaselGump,   // Intrinsic12D 13e0:0000
	Actor::I_setDead,               // Intrinsic12E 10f0:2010
	Item::I_getQLo,                 // Intrinsic12F 10a8:3889
	// 0x130
	Item::I_getNpcNum,              // Intrinsic130 10a8:0c40
	Actor::I_getDir,                // Intrinsic131 10f0:3714
	Actor::I_getLastAnimSet,        // Intrinsic132 10f0:372e
	Item::I_setQuality,             // Intrinsic133 10a8:0cee
	0                   			// (Unused) Intrinsic134 1048:0079
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
