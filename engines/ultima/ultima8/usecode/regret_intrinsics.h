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

#ifndef ULTIMA8_USECODE_REGRETINTRINSICS_H
#define ULTIMA8_USECODE_REGRETINTRINSICS_H

#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/world/item.h"

namespace Ultima {
namespace Ultima8 {

// Crusader: No Regret Intrinsics
// Main list for the updated version (1.06) (as on GOG)
const Intrinsic RegretIntrinsics[] = {
	World::I_getAlertActive, // Intrinsic000()
	Item::I_getFrame, // Intrinsic001()
	Item::I_setFrame, // Intrinsic002()
	Item::I_getMapArray, // Intrinsic003()
	Item::I_getStatus, // Intrinsic004()
	Item::I_orStatus, // Intrinsic005()
	Item::I_equip, // Intrinsic006()
	Item::I_isPartlyOnScreen, // Intrinsic007()
	Actor::I_isNPC, // Intrinsic008()
	Item::I_getZ, // Intrinsic009()
	World::I_gameDifficulty, // Intrinsic00A()
	Item::I_getQLo, // Intrinsic00B()
	Item::I_destroy, // Intrinsic00C()
	Actor::I_getUnkByte, // Intrinsic00D()
	Item::I_getX, // Intrinsic00E()
	Item::I_getY, // Intrinsic00F()
	// 0010
	AudioProcess::I_playSFXCru, // Intrinsic010()
	Item::I_getShape, // Intrinsic011()
	Item::I_explode, // Intrinsic012()
	UCMachine::I_rndRange, // Intrinsic013()
	Item::I_legalCreateAtCoords, // Intrinsic014()
	Item::I_andStatus, // Intrinsic015()
	World::I_getControlledNPCNum, // Intrinsic016()
	Actor::I_getDir, // Intrinsic017()
	Actor::I_getLastAnimSet, // Intrinsic018()
	Item::I_fireWeapon, // Intrinsic019()
	Item::I_create, // Intrinsic01A()
	Item::I_popToCoords, // Intrinsic01B()
	Actor::I_setDead, // Intrinsic01C()
	Item::I_push, // Intrinsic01D()
	Item::I_getEtherealTop, // Intrinsic01E()
	Item::I_getQLo, // Intrinsic01F()
	// 0020
	Item::I_setQLo, // Intrinsic020()
	Item::I_getQHi, // Intrinsic021()
	Item::I_setQHi, // Intrinsic022()
	Item::I_getClosestDirectionInRange, // Intrinsic023()
	Item::I_hurl, // Intrinsic024()
	Item::I_getCY, // Intrinsic025()
	Item::I_getCX, // Intrinsic026()
	SpriteProcess::I_createSprite, // Intrinsic027()
	Item::I_setNpcNum, // Intrinsic028()
	AudioProcess::I_playSFXCru, // Intrinsic029() TODO: Not exactly the same.
	Item::I_setShape, // Intrinsic02A()
	Item::I_pop, // Intrinsic02B()
	AudioProcess::I_stopSFXCru, // Intrinsic02C(), I_stopAllSFXForItem
	Item::I_isCompletelyOn, // Intrinsic02D()
	Item::I_popToContainer, // Intrinsic02E()
	Actor::I_getHp, // Intrinsic02F()
	// 0030
	MainActor::I_getMana, // Intrinsic030()
	Item::I_getFamily, // Intrinsic031()
	Actor::I_destroyContents, // Intrinsic032()
	AudioProcess::I_setVolumeForObjectSFX, // Intrinsic033()
	Item::I_getDirToItem, // Intrinsic034()
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic035()
	Item::I_getRangeIfVisible, // Intrinsic036()
	AudioProcess::I_playSFXCru, // Intrinsic037() TODO: Not exactly the same.
	Item::I_andStatus, // Intrinsic038()
	Kernel::I_resetRef, // Intrinsic039()
	Item::I_touch, // Intrinsic03A()
	Egg::I_getEggId, // Intrinsic03B()
	MainActor::I_addItemCru, // Intrinsic03C()
	Actor::I_getMap, // Intrinsic03D()
	Item::I_cast, // Intrinsic03E()
	0, // Intrinsic03F() - CameraProcess::I_somethingAboutCameraUpdate, not needed
	// 0040
	AudioProcess::I_stopSFXCru, // Intrinsic040()
	Item::I_isOn, // Intrinsic041()
	Item::I_getQHi, // Intrinsic042()
	Item::I_isOn, // Intrinsic043()
	Item::I_getQHi, // Intrinsic044()
	Item::I_isOn, // Intrinsic045()
	Item::I_getQHi, // Intrinsic046()
	Item::I_isOn, // Intrinsic047()
	Item::I_getQHi, // Intrinsic048()
	Item::I_isOn, // Intrinsic049()
	Item::I_getQHi, // Intrinsic04A()
	Item::I_isOn, // Intrinsic04B()
	Item::I_getQHi, // Intrinsic04C()
	Actor::I_getCurrentActivityNo, // Intrinsic04D()
	Actor::I_isDead, // Intrinsic04E()
	Actor::I_clrInCombat, // Intrinsic04F()
	// 0050
	Actor::I_setDefaultActivity0, // Intrinsic050()
	Actor::I_setDefaultActivity1, // Intrinsic051()
	Actor::I_setDefaultActivity2, // Intrinsic052()
	Actor::I_setActivity, // Intrinsic053()
	World::I_setControlledNPCNum, // Intrinsic054()
	Item::I_receiveHit, // Intrinsic055()
	UCMachine::I_true, // Actually Game::I_isReleaseBuild
	MainActor::I_setMana, // Intrinsic057()
	Item::I_use, // Intrinsic058()
	Item::I_setUnkEggType, // Intrinsic059()
	MusicProcess::I_playMusic, // Intrinsic05A()
	Item::I_getSurfaceWeight, // Intrinsic05B()
	Item::I_isCentreOn, // Intrinsic05C()
	Item::I_setFrame, // Intrinsic05D()
	Actor::I_getLastAnimSet, // Intrinsic05E()
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic05F()
	// 0060
	Actor::I_isBusy, // Intrinsic060()
	0, // Actor::I_getField0x13Flag2()
	Actor::I_doAnim, // Intrinsic062()
	Item::I_legalCreateAtPoint, // Intrinsic063()
	Item::I_getPoint, // Intrinsic064()
	Item::I_legalMoveToPoint, // Intrinsic065()
	Item::I_fall, // Intrinsic066()
	Item::I_hurl, // Intrinsic067()
	Kernel::I_getNumProcesses, // Intrinsic068()
	Item::I_getCY, // Intrinsic069()
	0, // Intrinsic06A() I_isAnimDisabled
	0, // Intrinsic06B() I_resetKeyboardState?
	MusicProcess::I_pauseMusic, // Intrinsic06C()
	MovieGump::I_playMovieCutsceneRegret, // Intrinsic06D()
	MusicProcess::I_unpauseMusic, // Intrinsic06E()
	Item::I_isInNpc, // Intrinsic06F()
	// 0070
	Ultima8Engine::I_setCruStasis, // Intrinsic070()
	Ultima8Engine::I_clrCruStasis, // Intrinsic071()
	PaletteFaderProcess::I_jumpToAllGivenColor, // Intrinsic072() PaletteFaderProcess::I_jumpToColor
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic073()
	Actor::I_isDead, // Intrinsic074()
	Actor::I_getNpcNum, // Intrinsic075()
	UCMachine::I_false, // Intrinsic076() - null intrinsic
	UCMachine::I_true, // Actually Game::I_isViolenceEnabled
	Item::I_unequip, // Intrinsic078()
	Item::I_andStatus, // Intrinsic079()
	Item::I_move, // Intrinsic07A()
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic07B()
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic07C()
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic07D()
	Actor::I_turnToward, // Intrinsic07E()
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic07F()
	// 0080
	MainActor::I_clrKeycards, // Intrinsic080()
	MusicProcess::I_stopMusic, // Intrinsic081()
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic082()
	0, // I_setUnkFlagA4()
	0, // I_clearUnkFlagA4()
	MainActor::I_switchMap, // Intrinsic085()
	MainActor::I_teleportToEgg, // Intrinsic086()
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic087()
	Actor::I_clrImmortal, // Intrinsic088()
	Actor::I_setActivity, // Intrinsic089()
	Item::I_getQuality, // Intrinsic08A()
	Item::I_setQuality, // Intrinsic08B()
	MainActor::I_getMaxEnergy, // Intrinsic08C()
	CameraProcess::I_moveTo, // Intrinsic08D()
	Actor::I_setImmortal, // Intrinsic08E()
	CameraProcess::I_getCameraX, // Intrinsic08F()
	// 0090
	CameraProcess::I_getCameraY, // Intrinsic090()
	Item::I_setMapArray, // Intrinsic091()
	Actor::I_getNpcNum, // Intrinsic092()
	Item::I_shoot, // Intrinsic093()
	CameraProcess::I_setCenterOn, // Intrinsic094()
	Item::I_enterFastArea, // Intrinsic095()
	Item::I_setBroken, // Intrinsic096()
	Item::I_hurl, // Intrinsic097()
	Actor::I_getNpcNum, // Intrinsic098()
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic099()
	MainActor::I_teleportToEgg, // Intrinsic09A()
	Actor::I_createActor, // Intrinsic09B()
	Actor::I_clrInCombat, // Intrinsic09C()
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic09D()
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic09E()
	CruStatusGump::I_showStatusGump, // Intrinsic09F(), same as Int05F in Remorse
	// 00A0
	Item::I_andStatus, // Intrinsic0A0()
	Egg::I_getUnkEggType, // Intrinsic0A1()
	Egg::I_setEggXRange, // Intrinsic0A2()
	Item::I_setFrame, // Intrinsic0A3()
	Item::I_overlaps, // Intrinsic0A4()
	Item::I_isOn, // Intrinsic0A5()
	Item::I_getQHi, // Intrinsic0A6()
	Actor::I_getLastAnimSet, // Intrinsic0A7()
	Item::I_getCY, // Intrinsic0A8()
	CurrentMap::I_canExistAt, // Intrinsic0A9()
	Item::I_isOn, // Intrinsic0AA()
	Actor::I_isDead, // Intrinsic0AB()
	Item::I_hurl, // Intrinsic0AC()
	Item::I_inFastArea, // Intrinsic0AD()
	Item::I_getQHi, // Intrinsic0AE()
	Item::I_andStatus, // Intrinsic0AF()
	// 00B0
	Item::I_hurl, // Intrinsic0B0()
	Item::I_andStatus, // Intrinsic0B1()
	Item::I_hurl, // Intrinsic0B2()
	Item::I_andStatus, // Intrinsic0B3()
	Item::I_getDirToCoords, // Intrinsic0B4()
	MainActor::I_removeItemCru, // Intrinsic0B5()
	UCMachine::I_true, // I_updateInventoryUI() - probably not needed?
	Actor::I_getNpcNum, // Intrinsic0B7()
	Item::I_getCY, // Intrinsic0B8()
	Item::I_isOn, // Intrinsic0B9()
	Item::I_getFootpadData, // Intrinsic0BA()
	Actor::I_isDead, // Intrinsic0BB()
	Actor::I_createActorCru, // Intrinsic0BC()
	Actor::I_setActivity, // Intrinsic0BD()
	KeypadGump::I_showKeypad, // Intrinsic0BE()
	Item::I_andStatus, // Intrinsic0BF()
	// 00C0
	ComputerGump::I_readComputer, // Intrinsic0C0()
	UCMachine::I_numToStr, // Intrinsic0C1()
	UCMachine::I_false, // Intrinsic0C2() - return 0
	Actor::I_getDir, // Intrinsic0C3()
	Item::I_getQHi, // Intrinsic0C4()
	Item::I_setQuality, // Intrinsic0C5()
	Item::I_hurl, // Intrinsic0C6()
	Actor::I_addHp, // Intrinsic0C7()
	CruHealerProcess::I_create, // Intrinsic0C8()
	Item::I_equip, // Intrinsic0C9()
	Item::I_setBroken, // Intrinsic0CA()
	Item::I_isOn, // Intrinsic0CB()
	Actor::I_teleport, // Intrinsic0CC()
	Item::I_getDirFromTo16, // Intrinsic0CD()
	Item::I_getQHi, // Intrinsic0CE()
	Item::I_isOn, // Intrinsic0CF()
	// 00D0
	Actor::I_isInCombat, // Intrinsic0D0()
	Actor::I_getLastActivityNo, // Actor::I_getNPCDataFi2eld0x4()
	Actor::I_setCombatTactic, // Intrinsic0D2()
	Actor::I_setDead, // Intrinsic0D3()
	CameraProcess::I_getCameraY, // Intrinsic0D4()
	Actor::I_getEquip, // Intrinsic0D5()
	Actor::I_setEquip, // Intrinsic0D6()
	Actor::I_getDefaultActivity0, // Intrinsic0D7()
	Actor::I_getDefaultActivity1, // Intrinsic0D8()
	Actor::I_getDefaultActivity2, // Intrinsic0D9()
	Actor::I_getLastAnimSet, // Intrinsic0DA()
	Actor::I_isFalling, // Actor::I_isFalling()
	Item::I_getQLo, // Intrinsic0DC()
	Item::I_getQHi, // Intrinsic0DD()
	Actor::I_getNpcNum, // Intrinsic0DE()
	Actor::I_setUnkByte, // Intrinsic0DF() Item::I_setField0x81
	// 00E0
	Item::I_hurl, // Intrinsic0E0()
	Actor::I_setDead, // Intrinsic0E1()
	Item::I_getQLo, // Intrinsic0E2()
	Item::I_getCY, // Intrinsic0E3()
	Actor::I_getNpcNum, // Intrinsic0E4()
	Item::I_hurl, // Intrinsic0E5()
	Actor::I_getNpcNum, // Intrinsic0E6()
	Item::I_hurl, // Intrinsic0E7()
	Actor::I_getNpcNum, // Intrinsic0E8()
	Item::I_hurl, // Intrinsic0E9()
	Actor::I_getNpcNum, // Intrinsic0EA()
	Item::I_hurl, // Intrinsic0EB()
	Actor::I_getNpcNum, // Intrinsic0EC()
	Item::I_hurl, // Intrinsic0ED()
	Actor::I_getNpcNum, // Intrinsic0EE()
	Item::I_hurl, // Intrinsic0EF()
	// 00F0
	Actor::I_getNpcNum, // Intrinsic0F0()
	Item::I_hurl, // Intrinsic0F1()
	Actor::I_getNpcNum, // Intrinsic0F2()
	Item::I_hurl, // Intrinsic0F3()
	Actor::I_getNpcNum, // Intrinsic0F4()
	Item::I_hurl, // Intrinsic0F5()
	Actor::I_getNpcNum, // Intrinsic0F6()
	Item::I_andStatus, // Intrinsic0F7()
	Actor::I_setDead, // Intrinsic0F8()
	Item::I_getQLo, // Intrinsic0F9()
	Actor::I_setDead, // Intrinsic0FA()
	Actor::I_getMaxHp, // DTable::I_getMaxHPForNPC()
	Actor::I_setHp, // Intrinsic0FC()
	Item::I_getQLo, // Intrinsic0FD()
	BatteryChargerProcess::I_create, // Intrinsic0FE()
	Item::I_hurl, // Intrinsic0FF()
	// 0100
	Item::I_andStatus, // Intrinsic100()
	Item::I_isOn, // Intrinsic101()
	Actor::I_isDead, // Intrinsic102()
	Actor::I_setActivity, // Intrinsic103()
	Item::I_getQHi, // Intrinsic104()
	Actor::I_getLastAnimSet, // Intrinsic105()
	Actor::I_setDead, // Intrinsic106()
	Item::I_getQLo, // Intrinsic107()
	Item::I_isOn, // Intrinsic108()
	Item::I_getQHi, // Intrinsic109()
	Item::I_isOn, // Intrinsic10A()
	Item::I_getQHi, // Intrinsic10B()
	Item::I_hurl, // Intrinsic10C()
	Actor::I_getNpcNum, // Intrinsic10D()
	Item::I_getCY, // Intrinsic10E()
	Item::I_hurl, // Intrinsic10F()
	// 0110
	Item::I_isOn, // Intrinsic110()
	MainActor::I_hasKeycard, // Intrinsic111()
	UCMachine::I_false, // Intrinsic112() - null function (return 0)
	Actor::I_isDead, // Intrinsic113()
	Actor::I_clrImmortal, // Intrinsic114()
	UCMachine::I_numToStr, // Intrinsic115()
	Item::I_getQHi, // Intrinsic116()
	Actor::I_setActivity, // Intrinsic117()
	Item::I_andStatus, // Intrinsic118()
	Actor::I_getNpcNum, // Intrinsic119()
	Item::I_andStatus, // Intrinsic11A()
	Actor::I_getNpcNum, // Intrinsic11B()
	Item::I_isCrusTypeNPC, // Intrinsic11C()
	Item::I_andStatus, // Intrinsic11D()
	Actor::I_getNpcNum, // Intrinsic11E()
	Item::I_avatarStoleSomething, // Intrinsic11F()
	// 0120
	Item::I_andStatus, // Intrinsic120()
	Actor::I_getNpcNum, // Intrinsic121()
	Item::I_getQ, // Intrinsic122()
	Item::I_setQ, // Intrinsic123()
	Item::I_andStatus, // Intrinsic124()
	Actor::I_getNpcNum, // Intrinsic125()
	Item::I_andStatus, // Intrinsic126()
	Actor::I_getNpcNum, // Intrinsic127()
	Item::I_andStatus, // Intrinsic128()
	Actor::I_getNpcNum, // Intrinsic129()
	Item::I_andStatus, // Intrinsic12A()
	Actor::I_getNpcNum, // Intrinsic12B()
	Item::I_andStatus, // Intrinsic12C()
	Actor::I_getNpcNum, // Intrinsic12D()
	Actor::I_getDir, // Intrinsic12E()
	Item::I_andStatus, // Intrinsic12F()
	// 0130
	Actor::I_getNpcNum, // Intrinsic130()
	Item::I_fireDistance, // Intrinsic131() - same as Intrinsic 116 in No Remorse
	Item::I_andStatus, // Intrinsic132()
	Item::I_hurl, // Intrinsic133()
	Item::I_andStatus, // Intrinsic134()
	CameraProcess::I_getCameraY, // Intrinsic135()
	CameraProcess::I_getCameraZ, // Intrinsic136()
	CruStatusGump::I_hideStatusGump, // Intrinsic137(), same as Int05D in Remorse
	Actor::I_clrInCombat, // Intrinsic138()
	Item::I_getTypeFlag, // Intrinsic139()
	Actor::I_getNpcNum, // Intrinsic13A()
	Item::I_hurl, // Intrinsic13B()
	Item::I_getCY, // Intrinsic13C()
	Item::I_getCZ, // Intrinsic13D()
	Item::I_setFrame, // Intrinsic13E()
	AudioProcess::I_playSFX, // Intrinsic13F()
	// 0140
	AudioProcess::I_isSFXPlaying, // Intrinsic140()
	World::I_clrAlertActive,
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic142()
	Actor::I_isDead, // Intrinsic143()
	Actor::I_setDead, // Intrinsic144()
	Game::I_playCredits, // Intrinsic145()
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic146()
	Item::I_getFamilyOfType, // Intrinsic147()
	Actor::I_getNpcNum, // Intrinsic148()
	Item::I_getQLo, // Intrinsic149()
	Item::I_andStatus, // Intrinsic14A()
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic14B()
	World::I_setAlertActive,
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic14D()
	MainActor::I_addItemCru, // Intrinsic14E()
	Egg::I_getEggXRange, // Intrinsic14F()
	// 0150
	Actor::I_clrInCombat, // Intrinsic150()
	PaletteFaderProcess::I_jumpToAllGivenColor, // Intrinsic151()
	Item::I_setFrame, // Intrinsic152()
	UCMachine::I_numToStr, // Intrinsic153()
	Actor::I_getDir, // Intrinsic154()
	UCMachine::I_numToStr, // Intrinsic155()
	Item::I_isOn, // Intrinsic156()
	Actor::I_getDir, // Intrinsic157()
	Actor::I_setDead, // Intrinsic158()
	Item::I_getQHi, // Intrinsic159()
	Item::I_getQLo, // Intrinsic15A()
	UCMachine::I_numToStr, // Intrinsic15B()
	Actor::I_getDir, // Intrinsic15C()
	0  // Intrinsic15D()
};

// =========================================================
// Intrinsics for Demo.
// At first glance this list looks the same, but it diverges
// changing part way through.
//

const Intrinsic RegretDemoIntrinsics[] = {
	World::I_getAlertActive,        // Intrinsic000 10e8:048d
	Item::I_getFrame,               // Intrinsic001 10b0:0826
	Item::I_setFrame,               // Intrinsic002 10b0:0882
	Item::I_getMapArray,            // Intrinsic003 10b0:0cea
	Item::I_getStatus,              // Intrinsic004 10b0:10aa
	Item::I_orStatus,               // Intrinsic005 10b0:25b6
	Item::I_equip,                  // Intrinsic006 10b0:2a1e
	Item::I_isPartlyOnScreen,       // Intrinsic007 10b0:41c1
	Actor::I_isNPC,                 // Intrinsic008 10b0:24ca
	Item::I_getZ,                   // Intrinsic009 10b0:0266
	World::I_gameDifficulty,        // Intrinsic00A 10f8:3fcd
	Item::I_getQLo,                 // Intrinsic00B 10b0:38a9
	Item::I_destroy,                // Intrinsic00C 10b0:13d7
	Actor::I_getUnkByte,            // Intrinsic00D 10f8:44f4
	Item::I_getX,                   // Intrinsic00E 10b0:013e
	Item::I_getY,                   // Intrinsic00F 10b0:0176
	// 0x010
	AudioProcess::I_playSFXCru,     // Intrinsic010 10b0:3954
	Item::I_getShape,               // Intrinsic011 10b0:068f
	Item::I_explode,                // Intrinsic012 1160:0a77
	UCMachine::I_rndRange,          // Intrinsic013 12d8:0293
	Item::I_legalCreateAtCoords,    // Intrinsic014 10b0:114d
	Item::I_andStatus,              // Intrinsic015 10b0:25cf
	World::I_getControlledNPCNum,   // Intrinsic016 1148:0048
	Actor::I_getDir,                // Intrinsic017 10f8:3e0c
	Actor::I_getLastAnimSet,        // Intrinsic018 10f8:3e26
	Item::I_fireWeapon,             // Intrinsic019 1140:1a86
	Item::I_create,                 // Intrinsic01A 10b0:1259
	Item::I_popToCoords,            // Intrinsic01B 10b0:1383
	Actor::I_setDead,               // Intrinsic01C 10f8:22a8
	Item::I_push,                   // Intrinsic01D 10b0:13c4
	Item::I_getEtherealTop,         // Intrinsic01E 10b8:26f8
	Item::I_getQLo,                 // Intrinsic01F 10b0:38a9
	// 0x020
	Item::I_setQLo,                 // Intrinsic020 10b0:38c2
	Item::I_getQHi,                 // Intrinsic021 10b0:385e
	Item::I_setQHi,                 // Intrinsic022 10b0:387a
	Item::I_getClosestDirectionInRange, // Intrinsic023 10e0:0000
	Item::I_hurl,                   // Intrinsic024 1048:134f
	Item::I_getCY,                  // Intrinsic025 10b0:02de
	Item::I_getCX,                  // Intrinsic026 10b0:029a
	SpriteProcess::I_createSprite,  // Intrinsic027 1150:00de
	Item::I_setNpcNum,              // Intrinsic028 10b0:0c96
	AudioProcess::I_playSFXCru,     // Intrinsic029 10b0:3973
	Item::I_setShape,               // Intrinsic02A 10b0:0717
	Item::I_pop,                    // Intrinsic02B 10b0:13aa
	AudioProcess::I_stopSFXCru,     // Intrinsic02C 10b0:39ef
	Item::I_isCompletelyOn,         // Intrinsic02D 10b0:1fa3
	Item::I_popToContainer,         // Intrinsic02E 10b0:139a
	Actor::I_getHp,                 // Intrinsic02F 10f8:39ea
	// 0x030
	MainActor::I_getMana,           // Intrinsic030 10f8:3b4a
	Item::I_getFamily,              // Intrinsic031 10b0:1036
	Actor::I_destroyContents,       // Intrinsic032 10b0:14f3
	AudioProcess::I_setVolumeForObjectSFX, // Intrinsic033 10b0:3a33
	Item::I_getDirToItem,           // Intrinsic034 10b0:1ab8
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic035 10b0:3a0b
	Item::I_touch,                  // Intrinsic036 10b0:2558
	Item::I_inFastArea,             // Intrinsic037 10b0:3a56
	AudioProcess::I_stopSFXCru,     // Intrinsic038 10b0:39d0
	Item::I_fall,                   // Intrinsic039 1048:13eb
	Item::I_getRangeIfVisible,      // Intrinsic03A 1140:258e
	AudioProcess::I_playSFXCru,     // Intrinsic03B 10b0:39b1
	Item::I_andStatus,              // Intrinsic03C 10b0:25cf
	Kernel::I_resetRef,             // Intrinsic03D 11e8:0c63
	Egg::I_getEggId,                // Intrinsic03E 10a0:05bb
	MainActor::I_addItemCru,        // Intrinsic03F 1008:03ab
	// 0x040
	Actor::I_getMap,                // Intrinsic040 10f8:3243
	Item::I_cast,                   // Intrinsic041 10b0:2f5e
	0,                              // Intrinsic042 1198:0013
	Item::I_isOn,                   // Intrinsic043 10b0:1f2e
	Item::I_legalMoveToPoint,       // Intrinsic044 10b0:1791
	Item::I_getQHi,                 // Intrinsic045 10b0:385e
	Item::I_isOn,                   // Intrinsic046 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic047 10b0:385e
	Item::I_isOn,                   // Intrinsic048 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic049 10b0:385e
	Item::I_isOn,                   // Intrinsic04A 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic04B 10b0:385e
	Item::I_isOn,                   // Intrinsic04C 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic04D 10b0:385e
	Item::I_isOn,                   // Intrinsic04E 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic04F 10b0:385e
	// 0x050
	Item::I_isOn,                   // Intrinsic050 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic051 10b0:385e
	Actor::I_getCurrentActivityNo,  // Intrinsic052 10f8:3a03
	Actor::I_isDead,                // Intrinsic053 10f8:2278
	Actor::I_clrInCombat,           // Intrinsic054 10f8:389f
	Actor::I_setDefaultActivity0,   // Intrinsic055 10f8:329d
	Actor::I_setDefaultActivity1,   // Intrinsic056 10f8:32ba
	Actor::I_setDefaultActivity2,   // Intrinsic057 10f8:32d7
	Actor::I_setActivity,           // Intrinsic058 10f8:33b5
	World::I_setControlledNPCNum,   // Intrinsic059 1148:0038
	Item::I_receiveHit,             // Intrinsic05A 1140:04cb
	UCMachine::I_true,              // Intrinsic05B 1058:00c1
	MainActor::I_setMana,           // Intrinsic05C 10f8:3b64
	Item::I_use,                    // Intrinsic05D 10b0:26ab
	Item::I_setUnkEggType,          // Intrinsic05E 10b0:0e43
	MusicProcess::I_playMusic,		// Intrinsic05F 1210:0173
	// 0x060
	Item::I_getSurfaceWeight,       // Intrinsic060 10b0:0602
	Item::I_isCentreOn,             // Intrinsic061 10b0:446e
	Item::I_setFrame,               // Intrinsic062 10b0:0882
	Actor::I_getLastAnimSet,        // Intrinsic063 10f8:3e26
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic064 10f8:3d96
	Actor::I_isBusy,                // Intrinsic065 10f8:0261
	0, // Actor::I_getField0x13Flag2(), // Intrinsic066 10f8:2499
	Actor::I_doAnim,                // Intrinsic067 10f8:039a
	Item::I_legalCreateAtPoint,     // Intrinsic068 10b0:10c0
	Item::I_hurl,                   // Intrinsic069 1048:134f
	Kernel::I_getNumProcesses,      // Intrinsic06A 11e8:0977
	Item::I_getCY,                  // Intrinsic06B 10b0:02de
	Item::I_getPoint,               // Intrinsic06C 10b0:247a
	0,                              // Intrinsic06D 1008:0134
	0,                              // Intrinsic06E 1008:0123
	CruStatusGump::I_hideStatusGump, // Intrinsic06F 11c8:0000
	// 0x070
	Actor::I_turnToward,            // Intrinsic070 10f8:3961
	ComputerGump::I_readComputer,   // Intrinsic071 1390:0005
	MovieGump::I_playMovieCutsceneRegret, // Intrinsic072 1448:0b5a
	UCMachine::I_true, // update inventory ui updates (not needed)
	MusicProcess::I_pauseMusic,
	MusicProcess::I_unpauseMusic,
	Item::I_isInNpc,                // Intrinsic076 10b0:24f9
	Ultima8Engine::I_setCruStasis,  // Intrinsic077 1008:00e7
	Ultima8Engine::I_clrCruStasis,  // Intrinsic078 1008:00ed
	PaletteFaderProcess::I_jumpToAllGivenColor, // Intrinsic079 1418:0923
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic07A 1418:0729
	Actor::I_isDead,                // Intrinsic07B 10f8:2278
	Actor::I_getNpcNum,             // Intrinsic07C 10b0:0cb2
	UCMachine::I_false,             // Intrinsic07D 10b0:2952
	UCMachine::I_true,              // Intrinsic07E 1058:00a1
	Item::I_unequip,                // Intrinsic07F 10b0:2a51
	// 0x080
	Item::I_andStatus,              // Intrinsic080 10b0:25cf
	Item::I_move,                   // Intrinsic081 10b0:1575
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic082 1008:0f1c
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic083 1008:0f20
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic084 1008:0f16
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic085 1418:071b
	MainActor::I_clrKeycards,       // Intrinsic086 10f8:4443
	MusicProcess::I_pauseMusic,     // Intrinsic087 1210:02b0
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic088 1418:080f
	0, // I_setUnkFlagA4
	0, // I_clrUnkFlagA4
	Game::I_playDemoScreen,
	MainActor::I_teleportToEgg,     // Intrinsic08C 10a0:04c4
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic08D 1418:0801
	Actor::I_clrImmortal,           // Intrinsic08E 10f8:241e
	Actor::I_setActivity,           // Intrinsic08F 10f8:33b5
	// 0x090
	Item::I_getQuality,             // Intrinsic090 10b0:0d1c
	Item::I_setQuality,             // Intrinsic091 10b0:0d60
	MainActor::I_getMaxEnergy,      // Intrinsic092 10f8:3b81
	CameraProcess::I_moveTo,        // Intrinsic093 1198:0ce8
	Actor::I_setImmortal,           // Intrinsic094 10f8:2403
	CameraProcess::I_getCameraX,    // Intrinsic095 1198:1d06
	CameraProcess::I_getCameraY,    // Intrinsic096 1198:1d0e
	Item::I_setMapArray,            // Intrinsic097 10b0:0cce
	Actor::I_getNpcNum,             // Intrinsic098 10b0:0cb2
	Item::I_shoot,                  // Intrinsic099 10b0:38f1
	CameraProcess::I_setCenterOn,   // Intrinsic09A 1198:1ae6
	Item::I_enterFastArea,          // Intrinsic09B 10b0:2b34
	Item::I_setBroken,              // Intrinsic09C 10b0:2614
	Item::I_hurl,                   // Intrinsic09D 1048:134f
	Actor::I_getNpcNum,             // Intrinsic09E 10b0:0cb2
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic09F 10b0:4437
	// 0x0A0
	MainActor::I_teleportToEgg,     // Intrinsic0A0 10a0:04ed
	Actor::I_createActor,           // Intrinsic0A1 10f8:280e
	Actor::I_clrInCombat,           // Intrinsic0A2 10f8:389f
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic0A3 1038:0605
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic0A4 1418:0b05
	CruStatusGump::I_showStatusGump, // Intrinsic0A5 11c8:0006
	Item::I_andStatus,              // Intrinsic0A6 10b0:25cf
	AudioProcess::I_playSFX,        // Intrinsic0A7 12d8:04d0
	AudioProcess::I_isSFXPlaying,   // Intrinsic0A8 12d8:05ba
	Item::I_getDirFromTo16,         // Intrinsic0A9 10e0:01c7
	Item::I_setFrame,               // Intrinsic0AA 10b0:0882
	Egg::I_getUnkEggType,           // Intrinsic0AB 10b0:0da3
	Egg::I_setEggXRange,            // Intrinsic0AC 10a0:0551
	Item::I_overlaps,               // Intrinsic0AD 10b0:1d09
	Item::I_isOn,                   // Intrinsic0AE 10b0:1f2e
	Egg::I_getEggXRange,            // Intrinsic0AF 10a0:0513
	// 0x0B0
	World::I_clrAlertActive,       // Intrinsic0B0 10e8:0247
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic0B1 1418:0b3b
	Actor::I_setDead,               // Intrinsic0B2 10f8:22a8
	Game::I_playCredits,            // Intrinsic0B3 1008:009c
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic0B4 1418:0899
	Actor::I_isFalling,             // Intrinsic0B5 10f8:2198
	Item::I_getFamilyOfType,        // Intrinsic0B6 1110:03ec
	Actor::I_getNpcNum,             // Intrinsic0B7 10b0:0cb2
	Item::I_getQLo,                 // Intrinsic0B8 10b0:38a9
	Item::I_getQHi,                 // Intrinsic0B9 10b0:385e
	Item::I_avatarStoleSomething,   // Intrinsic0BA 10b0:2ffa
	Item::I_andStatus,              // Intrinsic0BB 10b0:25cf
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic0BC 1040:03b8
	World::I_setAlertActive,       // Intrinsic0BD 10e8:0000
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic0BE 10f8:3d8e
	MainActor::I_addItemCru,        // Intrinsic0BF 1008:03ab
	// 0x0C0
	Actor::I_getLastAnimSet,        // Intrinsic0C0 10f8:3e26
	Item::I_getCY,                  // Intrinsic0C1 10b0:02de
	CurrentMap::I_canExistAt,       // Intrinsic0C2 1160:0d96
	Item::I_isOn,                   // Intrinsic0C3 10b0:1f2e
	Actor::I_isDead,                // Intrinsic0C4 10f8:2278
	Item::I_hurl,                   // Intrinsic0C5 1048:134f
	Item::I_getQHi,                 // Intrinsic0C6 10b0:385e
	Item::I_andStatus,              // Intrinsic0C7 10b0:25cf
	Item::I_hurl,                   // Intrinsic0C8 1048:134f
	Item::I_andStatus,              // Intrinsic0C9 10b0:25cf
	Item::I_hurl,                   // Intrinsic0CA 1048:134f
	Item::I_andStatus,              // Intrinsic0CB 10b0:25cf
	Item::I_getDirToCoords,         // Intrinsic0CC 10b0:1a2d
	MainActor::I_removeItemCru,     // Intrinsic0CD 1008:0b7f
	Actor::I_getNpcNum,             // Intrinsic0CE 10b0:0cb2
	Item::I_getCY,                  // Intrinsic0CF 10b0:02de
	// 0x0D0
	Item::I_isOn,                   // Intrinsic0D0 10b0:1f2e
	Item::I_getFootpadData,         // Intrinsic0D1 10b0:1aed
	Actor::I_isDead,                // Intrinsic0D2 10f8:2278
	Actor::I_createActorCru,        // Intrinsic0D3 10b0:3cd8
	Actor::I_setActivity,           // Intrinsic0D4 10f8:33b5
	KeypadGump::I_showKeypad,       // Intrinsic0D5 13b0:00fe
	Item::I_andStatus,              // Intrinsic0D6 10b0:25cf
	UCMachine::I_numToStr,          // Intrinsic0D7 13f0:0073
	UCMachine::I_false,             // Intrinsic0D8 10b0:2952
	Actor::I_getDir,                // Intrinsic0D9 10f8:3e0c
	Item::I_getQHi,                 // Intrinsic0DA 10b0:385e
	Item::I_setQuality,             // Intrinsic0DB 10b0:0d60
	Item::I_use,                    // Intrinsic0DC 10b0:26ab
	Item::I_hurl,                   // Intrinsic0DD 1048:134f
	Actor::I_addHp,                 // Intrinsic0DE 10f8:3e40
	CruHealerProcess::I_create,     // Intrinsic0DF 1140:2a98
	// 0x0E0
	Item::I_equip,                  // Intrinsic0E0 10b0:2a1e
	Item::I_setBroken,              // Intrinsic0E1 10b0:2614
	Item::I_isOn,                   // Intrinsic0E2 10b0:1f2e
	Actor::I_teleport,              // Intrinsic0E3 10f8:30b8
	Item::I_getQHi,                 // Intrinsic0E4 10b0:385e
	Item::I_isOn,                   // Intrinsic0E5 10b0:1f2e
	Actor::I_isInCombat,            // Intrinsic0E6 10f8:21b7
	Actor::I_getLastActivityNo,     // Intrinsic0E7 10f8:452f
	Actor::I_setCombatTactic,       // Intrinsic0E8 10f8:3c8c
	Actor::I_setDead,               // Intrinsic0E9 10f8:22a8
	CameraProcess::I_getCameraY,    // Intrinsic0EA 1198:1d0e
	Actor::I_getEquip,              // Intrinsic0EB 10f8:2e86
	Actor::I_setEquip,              // Intrinsic0EC 10f8:2eb9
	Actor::I_getDefaultActivity0,   // Intrinsic0ED 10f8:332b
	Actor::I_getDefaultActivity1,   // Intrinsic0EE 10f8:3345
	Actor::I_getDefaultActivity2,   // Intrinsic0EF 10f8:335f
	// 0x0F0
	Actor::I_getLastAnimSet,        // Intrinsic0F0 10f8:3e26
	Item::I_getQLo,                 // Intrinsic0F1 10b0:38a9
	Item::I_getQHi,                 // Intrinsic0F2 10b0:385e
	Actor::I_getNpcNum,             // Intrinsic0F3 10b0:0cb2
	Actor::I_setUnkByte,            // Intrinsic0F4 10f8:4511
	Item::I_hurl,                   // Intrinsic0F5 1048:134f
	Actor::I_setDead,               // Intrinsic0F6 10f8:22a8
	Item::I_getQLo,                 // Intrinsic0F7 10b0:38a9
	Item::I_getCY,                  // Intrinsic0F8 10b0:02de
	Actor::I_getNpcNum,             // Intrinsic0F9 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0FA 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0FB 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0FC 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0FD 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0FE 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0FF 10b0:0cb2
	// 0x100
	Item::I_hurl,                   // Intrinsic100 1048:134f
	Actor::I_getNpcNum,             // Intrinsic101 10b0:0cb2
	Item::I_hurl,                   // Intrinsic102 1048:134f
	Actor::I_getNpcNum,             // Intrinsic103 10b0:0cb2
	Item::I_hurl,                   // Intrinsic104 1048:134f
	Actor::I_getNpcNum,             // Intrinsic105 10b0:0cb2
	Item::I_hurl,                   // Intrinsic106 1048:134f
	Actor::I_getNpcNum,             // Intrinsic107 10b0:0cb2
	Item::I_hurl,                   // Intrinsic108 1048:134f
	Actor::I_getNpcNum,             // Intrinsic109 10b0:0cb2
	Item::I_hurl,                   // Intrinsic10A 1048:134f
	Actor::I_getNpcNum,             // Intrinsic10B 10b0:0cb2
	Item::I_andStatus,              // Intrinsic10C 10b0:25cf
	Actor::I_setDead,               // Intrinsic10D 10f8:22a8
	Item::I_getQLo,                 // Intrinsic10E 10b0:38a9
	Actor::I_setDead,               // Intrinsic10F 10f8:22a8
	// 0x110
	Actor::I_getMaxHp,              // Intrinsic110 10f8:3a6b
	Actor::I_setHp,                 // Intrinsic111 10f8:3c56
	Item::I_getQLo,                 // Intrinsic112 10b0:38a9
	BatteryChargerProcess::I_create, // Intrinsic113 1140:2a78
	Item::I_hurl,                   // Intrinsic114 1048:134f
	Item::I_andStatus,              // Intrinsic115 10b0:25cf
	Item::I_isOn,                   // Intrinsic116 10b0:1f2e
	Actor::I_isDead,                // Intrinsic117 10f8:2278
	Actor::I_setActivity,           // Intrinsic118 10f8:33b5
	Item::I_getQHi,                 // Intrinsic119 10b0:385e
	Actor::I_getLastAnimSet,        // Intrinsic11A 10f8:3e26
	Actor::I_setDead,               // Intrinsic11B 10f8:22a8
	Item::I_getQLo,                 // Intrinsic11C 10b0:38a9
	Item::I_isOn,                   // Intrinsic11D 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic11E 10b0:385e
	Item::I_isOn,                   // Intrinsic11F 10b0:1f2e
	// 0x120
	Item::I_getQHi,                 // Intrinsic120 10b0:385e
	Item::I_hurl,                   // Intrinsic121 1048:134f
	Actor::I_getNpcNum,             // Intrinsic122 10b0:0cb2
	Item::I_getCY,                  // Intrinsic123 10b0:02de
	Item::I_hurl,                   // Intrinsic124 1048:134f
	Item::I_isOn,                   // Intrinsic125 10b0:1f2e
	MainActor::I_hasKeycard,        // Intrinsic126 10f8:43e0
	UCMachine::I_false,             // Intrinsic127 10b0:2952
	Actor::I_isDead,                // Intrinsic128 10f8:2278
	Actor::I_clrImmortal,           // Intrinsic129 10f8:241e
	UCMachine::I_numToStr,          // Intrinsic12A 13f0:0073
	Item::I_getQHi,                 // Intrinsic12B 10b0:385e
	Actor::I_setActivity,           // Intrinsic12C 10f8:33b5
	Item::I_andStatus,              // Intrinsic12D 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic12E 10b0:0cb2
	Item::I_andStatus,              // Intrinsic12F 10b0:25cf
	// 0x130
	Actor::I_getDir,                // Intrinsic130 10f8:3e0c
	Actor::I_setDead,               // Intrinsic131 10f8:22a8
	Actor::I_getNpcNum,             // Intrinsic132 10b0:0cb2
	Item::I_getQLo,                 // Intrinsic133 10b0:38a9
	Item::I_isCrusTypeNPC,          // Intrinsic134 10b0:443f
	Item::I_andStatus,              // Intrinsic135 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic136 10b0:0cb2
	Item::I_andStatus,              // Intrinsic137 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic138 10b0:0cb2
	Item::I_getQ,                   // Intrinsic139 10b0:0d06
	Item::I_setQ,                   // Intrinsic13A 10b0:0fd1
	Item::I_andStatus,              // Intrinsic13B 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic13C 10b0:0cb2
	Item::I_andStatus,              // Intrinsic13D 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic13E 10b0:0cb2
	Item::I_andStatus,              // Intrinsic13F 10b0:25cf
	// 0x140
	Actor::I_getNpcNum,             // Intrinsic140 10b0:0cb2
	Item::I_andStatus,              // Intrinsic141 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic142 10b0:0cb2
	Item::I_andStatus,              // Intrinsic143 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic144 10b0:0cb2
	Item::I_andStatus,              // Intrinsic145 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic146 10b0:0cb2
	Item::I_fireDistance,           // Intrinsic147 1140:20cb
	Item::I_andStatus,              // Intrinsic148 10b0:25cf
	Item::I_hurl,                   // Intrinsic149 1048:134f
	Item::I_andStatus,              // Intrinsic14A 10b0:25cf
	CameraProcess::I_getCameraY,    // Intrinsic14B 1198:1d0e
	CameraProcess::I_getCameraZ,    // Intrinsic14C 1198:1d16
	Actor::I_clrInCombat,           // Intrinsic14D 10f8:389f
	Item::I_getTypeFlag,            // Intrinsic14E 10b0:1061
	Actor::I_getNpcNum,             // Intrinsic14F 10b0:0cb2
	// 0x150
	Item::I_hurl,                   // Intrinsic150 1048:134f
	Item::I_getCY,                  // Intrinsic151 10b0:02de
	Item::I_getCZ,                  // Intrinsic152 10b0:0322
	Item::I_setFrame,               // Intrinsic153 10b0:0882
	0								// Unused Intrinsic154 1050:0079
};

//////////////////////////////////////////////////////////

// =========================================================
// Intrinsics for German version.
// This is annoyingly similar to the main version, but has an extra intrinsic inserted,
// at 13F so they are all offset after that
//

const Intrinsic RegretDeIntrinsics[] = {
	// 0x000
	World::I_getAlertActive,        // Intrinsic000 10e8:048d
	Item::I_getFrame,               // Intrinsic001 10b0:0826
	Item::I_setFrame,               // Intrinsic002 10b0:0882
	Item::I_getMapArray,            // Intrinsic003 10b0:0cea
	Item::I_getStatus,              // Intrinsic004 10b0:10aa
	Item::I_orStatus,               // Intrinsic005 10b0:25b6
	Item::I_equip,                  // Intrinsic006 10b0:2a1e
	Item::I_isPartlyOnScreen,       // Intrinsic007 10b0:41c1
	Actor::I_isNPC,                 // Intrinsic008 10b0:24ca
	Item::I_getZ,                   // Intrinsic009 10b0:0266
	World::I_gameDifficulty,        // Intrinsic00A 10f8:3fcd
	Item::I_getQLo,                 // Intrinsic00B 10b0:38a9
	Item::I_destroy,                // Intrinsic00C 10b0:13d7
	Actor::I_getUnkByte,            // Intrinsic00D 10f8:44f4
	Item::I_getX,                   // Intrinsic00E 10b0:013e
	Item::I_getY,                   // Intrinsic00F 10b0:0176
	// 0x010
	AudioProcess::I_playSFXCru,     // Intrinsic010 10b0:3954
	Item::I_getShape,               // Intrinsic011 10b0:068f
	Item::I_explode,                // Intrinsic012 1160:0a77
	UCMachine::I_rndRange,          // Intrinsic013 12d8:0293
	Item::I_legalCreateAtCoords,    // Intrinsic014 10b0:114d
	Item::I_andStatus,              // Intrinsic015 10b0:25cf
	World::I_getControlledNPCNum,   // Intrinsic016 1148:0048
	Actor::I_getDir,                // Intrinsic017 10f8:3e0c
	Actor::I_getLastAnimSet,        // Intrinsic018 10f8:3e26
	Item::I_fireWeapon,             // Intrinsic019 1140:1a86
	Item::I_create,                 // Intrinsic01A 10b0:1259
	Item::I_popToCoords,            // Intrinsic01B 10b0:1383
	Actor::I_setDead,               // Intrinsic01C 10f8:22a8
	Item::I_push,                   // Intrinsic01D 10b0:13c4
	Item::I_getEtherealTop,         // Intrinsic01E 10b8:26f8
	Item::I_getQLo,                 // Intrinsic01F 10b0:38a9
	// 0x020
	Item::I_setQLo,                 // Intrinsic020 10b0:38c2
	Item::I_getQHi,                 // Intrinsic021 10b0:385e
	Item::I_setQHi,                 // Intrinsic022 10b0:387a
	Item::I_getClosestDirectionInRange, // Intrinsic023 10e0:0000
	Item::I_hurl,                   // Intrinsic024 1048:134f
	Item::I_getCY,                  // Intrinsic025 10b0:02de
	Item::I_getCX,                  // Intrinsic026 10b0:029a
	SpriteProcess::I_createSprite,  // Intrinsic027 1150:00de
	Item::I_setNpcNum,              // Intrinsic028 10b0:0c96
	AudioProcess::I_playSFXCru,     // Intrinsic029 10b0:3973
	Item::I_setShape,               // Intrinsic02A 10b0:0717
	Item::I_pop,                    // Intrinsic02B 10b0:13aa
	AudioProcess::I_stopSFXCru,     // Intrinsic02C 10b0:39ef
	Item::I_isCompletelyOn,         // Intrinsic02D 10b0:1fa3
	Item::I_popToContainer,         // Intrinsic02E 10b0:139a
	Actor::I_getHp,                 // Intrinsic02F 10f8:39ea
	// 0x030
	MainActor::I_getMana,           // Intrinsic030 10f8:3b4a
	Item::I_getFamily,              // Intrinsic031 10b0:1036
	Actor::I_destroyContents,       // Intrinsic032 10b0:14f3
	AudioProcess::I_setVolumeForObjectSFX,  // Intrinsic033 10b0:3a33
	Item::I_getDirToItem,           // Intrinsic034 10b0:1ab8
	AudioProcess::I_isSFXPlayingForObject, // Intrinsic035 10b0:3a0b
	Item::I_getRangeIfVisible,      // Intrinsic036 1140:258e
	AudioProcess::I_playSFXCru,     // Intrinsic037 10b0:39b1
	Item::I_andStatus,              // Intrinsic038 10b0:25cf
	Kernel::I_resetRef,             // Intrinsic039 11e8:0c63
	Item::I_touch,                  // Intrinsic03A 10b0:2558
	Egg::I_getEggId,                // Intrinsic03B 10a0:05bb
	MainActor::I_addItemCru,        // Intrinsic03C 1008:03ab
	Actor::I_getMap,                // Intrinsic03D 10f8:3243
	Item::I_cast,                   // Intrinsic03E 10b0:2f5e
	0,                              // Intrinsic03F 1198:0013
	// 0x040
	AudioProcess::I_stopSFXCru,     // Intrinsic040 10b0:39d0
	Item::I_isOn,                   // Intrinsic041 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic042 10b0:385e
	Item::I_isOn,                   // Intrinsic043 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic044 10b0:385e
	Item::I_isOn,                   // Intrinsic045 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic046 10b0:385e
	Item::I_isOn,                   // Intrinsic047 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic048 10b0:385e
	Item::I_isOn,                   // Intrinsic049 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic04A 10b0:385e
	Item::I_isOn,                   // Intrinsic04B 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic04C 10b0:385e
	Actor::I_getCurrentActivityNo,  // Intrinsic04D 10f8:3a03
	Actor::I_isDead,                // Intrinsic04E 10f8:2278
	Actor::I_clrInCombat,           // Intrinsic04F 10f8:389f
	// 0x050
	Actor::I_setDefaultActivity0,   // Intrinsic050 10f8:329d
	Actor::I_setDefaultActivity1,   // Intrinsic051 10f8:32ba
	Actor::I_setDefaultActivity2,   // Intrinsic052 10f8:32d7
	Actor::I_setActivity,           // Intrinsic053 10f8:33b5
	World::I_setControlledNPCNum,   // Intrinsic054 1148:0038
	Item::I_receiveHit,             // Intrinsic055 1140:04cb
	UCMachine::I_true,              // Intrinsic056 1058:00c1
	MainActor::I_setMana,           // Intrinsic057 10f8:3b64
	Item::I_use,                    // Intrinsic058 10b0:26ab
	Item::I_setUnkEggType,          // Intrinsic059 10b0:0e43
	MusicProcess::I_playMusic,      // Intrinsic05A 1210:01a2
	Item::I_getSurfaceWeight,       // Intrinsic05B 10b0:0602
	Item::I_isCentreOn,             // Intrinsic05C 10b0:446e
	Item::I_setFrame,               // Intrinsic05D 10b0:0882
	Actor::I_getLastAnimSet,        // Intrinsic05E 10f8:3e26
	Ultima8Engine::I_setAvatarInStasis, // Intrinsic05F 10f8:3d96
	// 0x060
	Actor::I_isBusy,                // Intrinsic060 10f8:0261
	0, // Actor::I_getField0x13Flag2(),   // Intrinsic061 10f8:2499
	Actor::I_doAnim,                // Intrinsic062 10f8:039a
	Item::I_legalCreateAtPoint,     // Intrinsic063 10b0:10c0
	Item::I_getPoint,               // Intrinsic064 10b0:247a
	Item::I_legalMoveToPoint,       // Intrinsic065 10b0:1791
	Item::I_fall,                   // Intrinsic066 1048:13eb
	Item::I_hurl,                   // Intrinsic067 1048:134f
	Kernel::I_getNumProcesses,      // Intrinsic068 11e8:0977
	Item::I_getCY,                  // Intrinsic069 10b0:02de
	0,                              // Intrinsic06A 1008:0134
	0,                              // Intrinsic06B 1008:0123
	MusicProcess::I_pauseMusic,     // Intrinsic06C 1210:02a3
	MovieGump::I_playMovieCutsceneRegret, // Intrinsic06D 1448:0b5a
	MusicProcess::I_unpauseMusic,   // Intrinsic06E 1210:02c1
	Item::I_isInNpc,                // Intrinsic06F 10b0:24f9
	// 0x070
	Ultima8Engine::I_setCruStasis,  // Intrinsic070 1008:00e7
	Ultima8Engine::I_clrCruStasis,  // Intrinsic071 1008:00ed
	PaletteFaderProcess::I_jumpToAllGivenColor, // Intrinsic072 1418:0923
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic073 1418:0729
	Actor::I_isDead,                // Intrinsic074 10f8:2278
	Actor::I_getNpcNum,             // Intrinsic075 10b0:0cb2
	UCMachine::I_false,             // Intrinsic076 10b0:2952
	UCMachine::I_true,              // Intrinsic077 1058:00a1
	Item::I_unequip,                // Intrinsic078 10b0:2a51
	Item::I_andStatus,              // Intrinsic079 10b0:25cf
	Item::I_move,                   // Intrinsic07A 10b0:1575
	Ultima8Engine::I_getCrusaderTeleporting, // Intrinsic07B 1008:0f1c
	Ultima8Engine::I_setCrusaderTeleporting, // Intrinsic07C 1008:0f20
	Ultima8Engine::I_clrCrusaderTeleporting, // Intrinsic07D 1008:0f16
	Actor::I_turnToward,            // Intrinsic07E 10f8:3961
	PaletteFaderProcess::I_fadeToBlack, // Intrinsic07F 1418:071b
	// 0x080
	MainActor::I_clrKeycards,       // Intrinsic080 10f8:4443
	MusicProcess::I_stopMusic,      // Intrinsic081 1210:02df
	PaletteFaderProcess::I_jumpToAllBlack, // Intrinsic082 1418:080f
	0, // I_setUnkFlagA4(),               // Intrinsic083 1008:0090
	0, // I_clearUnkFlagA4(),             // Intrinsic084 1008:0096
	MainActor::I_switchMap,         // Intrinsic085 1030:0c60
	MainActor::I_teleportToEgg,     // Intrinsic086 10a0:04c4
	PaletteFaderProcess::I_fadeToGamePal, // Intrinsic087 1418:0801
	Actor::I_clrImmortal,           // Intrinsic088 10f8:241e
	Actor::I_setActivity,           // Intrinsic089 10f8:33b5
	Item::I_getQuality,             // Intrinsic08A 10b0:0d1c
	Item::I_setQuality,             // Intrinsic08B 10b0:0d60
	MainActor::I_getMaxEnergy,      // Intrinsic08C 10f8:3b81
	CameraProcess::I_moveTo,        // Intrinsic08D 1198:0ce8
	Actor::I_setImmortal,           // Intrinsic08E 10f8:2403
	CameraProcess::I_getCameraX,    // Intrinsic08F 1198:1d06
	// 0x090
	CameraProcess::I_getCameraY,    // Intrinsic090 1198:1d0e
	Item::I_setMapArray,            // Intrinsic091 10b0:0cce
	Actor::I_getNpcNum,             // Intrinsic092 10b0:0cb2
	Item::I_shoot,                  // Intrinsic093 10b0:38f1
	CameraProcess::I_setCenterOn,   // Intrinsic094 1198:1ae6
	Item::I_enterFastArea,          // Intrinsic095 10b0:2b34
	Item::I_setBroken,              // Intrinsic096 10b0:2614
	Item::I_hurl,                   // Intrinsic097 1048:134f
	Actor::I_getNpcNum,             // Intrinsic098 10b0:0cb2
	Ultima8Engine::I_moveKeyDownRecently, // Intrinsic099 10b0:4437
	MainActor::I_teleportToEgg,     // Intrinsic09A 10a0:04ed
	Actor::I_createActor,           // Intrinsic09B 10f8:280e
	Actor::I_clrInCombat,           // Intrinsic09C 10f8:389f
	PaletteFaderProcess::I_jumpToGreyScale, // Intrinsic09D 1038:0605
	PaletteFaderProcess::I_jumpToNormalPalette, // Intrinsic09E 1418:0b05
	CruStatusGump::I_showStatusGump, // Intrinsic09F 11c8:0006
	// 0x0A0
	Item::I_andStatus,              // Intrinsic0A0 10b0:25cf
	Egg::I_getUnkEggType,           // Intrinsic0A1 10b0:0da3
	Egg::I_setEggXRange,            // Intrinsic0A2 10a0:0551
	Item::I_setFrame,               // Intrinsic0A3 10b0:0882
	Item::I_overlaps,               // Intrinsic0A4 10b0:1d09
	Item::I_isOn,                   // Intrinsic0A5 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic0A6 10b0:385e
	Actor::I_getLastAnimSet,        // Intrinsic0A7 10f8:3e26
	Item::I_getCY,                  // Intrinsic0A8 10b0:02de
	CurrentMap::I_canExistAt,       // Intrinsic0A9 1160:0d96
	Item::I_isOn,                   // Intrinsic0AA 10b0:1f2e
	Actor::I_isDead,                // Intrinsic0AB 10f8:2278
	Item::I_hurl,                   // Intrinsic0AC 1048:134f
	Item::I_inFastArea,             // Intrinsic0AD 10b0:3a56
	Item::I_getQHi,                 // Intrinsic0AE 10b0:385e
	Item::I_andStatus,              // Intrinsic0AF 10b0:25cf
	// 0x0B0
	Item::I_hurl,                   // Intrinsic0B0 1048:134f
	Item::I_andStatus,              // Intrinsic0B1 10b0:25cf
	Item::I_hurl,                   // Intrinsic0B2 1048:134f
	Item::I_andStatus,              // Intrinsic0B3 10b0:25cf
	Item::I_getDirToCoords,         // Intrinsic0B4 10b0:1a2d
	MainActor::I_removeItemCru,     // Intrinsic0B5 1008:0b7f
	UCMachine::I_true,              // Intrinsic0B6 1008:005d
	Actor::I_getNpcNum,             // Intrinsic0B7 10b0:0cb2
	Item::I_getCY,                  // Intrinsic0B8 10b0:02de
	Item::I_isOn,                   // Intrinsic0B9 10b0:1f2e
	Item::I_getFootpadData,         // Intrinsic0BA 10b0:1aed
	Actor::I_isDead,                // Intrinsic0BB 10f8:2278
	Actor::I_createActorCru,        // Intrinsic0BC 10b0:3cd8
	Actor::I_setActivity,           // Intrinsic0BD 10f8:33b5
	KeypadGump::I_showKeypad,       // Intrinsic0BE 13b0:00fe
	Item::I_andStatus,              // Intrinsic0BF 10b0:25cf
	// 0x0C0
	ComputerGump::I_readComputer,   // Intrinsic0C0 1390:0005
	UCMachine::I_numToStr,          // Intrinsic0C1 13f0:0073
	UCMachine::I_false,             // Intrinsic0C2 10b0:2952
	Actor::I_getDir,                // Intrinsic0C3 10f8:3e0c
	Item::I_getQHi,                 // Intrinsic0C4 10b0:385e
	Item::I_setQuality,             // Intrinsic0C5 10b0:0d60
	Item::I_hurl,                   // Intrinsic0C6 1048:134f
	Actor::I_addHp,                 // Intrinsic0C7 10f8:3e40
	CruHealerProcess::I_create,     // Intrinsic0C8 1140:2a98
	Item::I_equip,                  // Intrinsic0C9 10b0:2a1e
	Item::I_setBroken,              // Intrinsic0CA 10b0:2614
	Item::I_isOn,                   // Intrinsic0CB 10b0:1f2e
	Actor::I_teleport,              // Intrinsic0CC 10f8:30b8
	Item::I_getDirFromTo16,         // Intrinsic0CD 10e0:01c7
	Item::I_getQHi,                 // Intrinsic0CE 10b0:385e
	Item::I_isOn,                   // Intrinsic0CF 10b0:1f2e
	// 0x0D0
	Actor::I_isInCombat,            // Intrinsic0D0 10f8:21b7
	Actor::I_getLastActivityNo,     // Intrinsic0D1 10f8:452f
	Actor::I_setCombatTactic,       // Intrinsic0D2 10f8:3c8c
	Actor::I_setDead,               // Intrinsic0D3 10f8:22a8
	CameraProcess::I_getCameraY,    // Intrinsic0D4 1198:1d0e
	Actor::I_getEquip,              // Intrinsic0D5 10f8:2e86
	Actor::I_setEquip,              // Intrinsic0D6 10f8:2eb9
	Actor::I_getDefaultActivity0,   // Intrinsic0D7 10f8:332b
	Actor::I_getDefaultActivity1,   // Intrinsic0D8 10f8:3345
	Actor::I_getDefaultActivity2,   // Intrinsic0D9 10f8:335f
	Actor::I_getLastAnimSet,        // Intrinsic0DA 10f8:3e26
	Actor::I_isFalling,             // Intrinsic0DB 10f8:2198
	Item::I_getQLo,                 // Intrinsic0DC 10b0:38a9
	Item::I_getQHi,                 // Intrinsic0DD 10b0:385e
	Actor::I_getNpcNum,             // Intrinsic0DE 10b0:0cb2
	Actor::I_setUnkByte,            // Intrinsic0DF 10f8:4511
	// 0x0E0
	Item::I_hurl,                   // Intrinsic0E0 1048:134f
	Actor::I_setDead,               // Intrinsic0E1 10f8:22a8
	Item::I_getQLo,                 // Intrinsic0E2 10b0:38a9
	Item::I_getCY,                  // Intrinsic0E3 10b0:02de
	Actor::I_getNpcNum,             // Intrinsic0E4 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0E5 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0E6 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0E7 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0E8 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0E9 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0EA 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0EB 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0EC 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0ED 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0EE 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0EF 1048:134f
	// 0x0F0
	Actor::I_getNpcNum,             // Intrinsic0F0 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0F1 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0F2 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0F3 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0F4 10b0:0cb2
	Item::I_hurl,                   // Intrinsic0F5 1048:134f
	Actor::I_getNpcNum,             // Intrinsic0F6 10b0:0cb2
	Item::I_andStatus,              // Intrinsic0F7 10b0:25cf
	Actor::I_setDead,               // Intrinsic0F8 10f8:22a8
	Item::I_getQLo,                 // Intrinsic0F9 10b0:38a9
	Actor::I_setDead,               // Intrinsic0FA 10f8:22a8
	Actor::I_getMaxHp,              // Intrinsic0FB 10f8:3a6b
	Actor::I_setHp,                 // Intrinsic0FC 10f8:3c56
	Item::I_getQLo,                 // Intrinsic0FD 10b0:38a9
	BatteryChargerProcess::I_create, // Intrinsic0FE 1140:2a78
	Item::I_hurl,                   // Intrinsic0FF 1048:134f
	// 0x100
	Item::I_andStatus,              // Intrinsic100 10b0:25cf
	Item::I_isOn,                   // Intrinsic101 10b0:1f2e
	Actor::I_isDead,                // Intrinsic102 10f8:2278
	Actor::I_setActivity,           // Intrinsic103 10f8:33b5
	Item::I_getQHi,                 // Intrinsic104 10b0:385e
	Actor::I_getLastAnimSet,        // Intrinsic105 10f8:3e26
	Actor::I_setDead,               // Intrinsic106 10f8:22a8
	Item::I_getQLo,                 // Intrinsic107 10b0:38a9
	Item::I_isOn,                   // Intrinsic108 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic109 10b0:385e
	Item::I_isOn,                   // Intrinsic10A 10b0:1f2e
	Item::I_getQHi,                 // Intrinsic10B 10b0:385e
	Item::I_hurl,                   // Intrinsic10C 1048:134f
	Actor::I_getNpcNum,             // Intrinsic10D 10b0:0cb2
	Item::I_getCY,                  // Intrinsic10E 10b0:02de
	Item::I_hurl,                   // Intrinsic10F 1048:134f
	// 0x110
	Item::I_isOn,                   // Intrinsic110 10b0:1f2e
	MainActor::I_hasKeycard,        // Intrinsic111 10f8:43e0
	UCMachine::I_false,             // Intrinsic112 10b0:2952
	Actor::I_isDead,                // Intrinsic113 10f8:2278
	Actor::I_clrImmortal,           // Intrinsic114 10f8:241e
	UCMachine::I_numToStr,          // Intrinsic115 13f0:0073
	Item::I_getQHi,                 // Intrinsic116 10b0:385e
	Actor::I_setActivity,           // Intrinsic117 10f8:33b5
	Item::I_andStatus,              // Intrinsic118 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic119 10b0:0cb2
	Item::I_andStatus,              // Intrinsic11A 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic11B 10b0:0cb2
	Item::I_isCrusTypeNPC,          // Intrinsic11C 10b0:443f
	Item::I_andStatus,              // Intrinsic11D 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic11E 10b0:0cb2
	Item::I_avatarStoleSomething,   // Intrinsic11F 10b0:2ffa
	// 0x120
	Item::I_andStatus,              // Intrinsic120 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic121 10b0:0cb2
	Item::I_getQ,                   // Intrinsic122 10b0:0d06
	Item::I_setQ,                   // Intrinsic123 10b0:0fd1
	Item::I_andStatus,              // Intrinsic124 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic125 10b0:0cb2
	Item::I_andStatus,              // Intrinsic126 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic127 10b0:0cb2
	Item::I_andStatus,              // Intrinsic128 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic129 10b0:0cb2
	Item::I_andStatus,              // Intrinsic12A 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic12B 10b0:0cb2
	Item::I_andStatus,              // Intrinsic12C 10b0:25cf
	Actor::I_getNpcNum,             // Intrinsic12D 10b0:0cb2
	Actor::I_getDir,                // Intrinsic12E 10f8:3e0c
	Item::I_andStatus,              // Intrinsic12F 10b0:25cf
	// 0x130
	Actor::I_getNpcNum,             // Intrinsic130 10b0:0cb2
	Item::I_fireDistance,           // Intrinsic131 1140:20cb
	Item::I_andStatus,              // Intrinsic132 10b0:25cf
	Item::I_hurl,                   // Intrinsic133 1048:134f
	Item::I_andStatus,              // Intrinsic134 10b0:25cf
	CameraProcess::I_getCameraY,    // Intrinsic135 1198:1d0e
	CameraProcess::I_getCameraZ,    // Intrinsic136 1198:1d16
	CruStatusGump::I_hideStatusGump, // Intrinsic137 11c8:0000
	Actor::I_clrInCombat,           // Intrinsic138 10f8:389f
	Item::I_getTypeFlag,            // Intrinsic139 10b0:1061
	Actor::I_getNpcNum,             // Intrinsic13A 10b0:0cb2
	Item::I_hurl,                   // Intrinsic13B 1048:134f
	Item::I_getCY,                  // Intrinsic13C 10b0:02de
	Item::I_getCZ,                  // Intrinsic13D 10b0:0322
	Item::I_setFrame,               // Intrinsic13E 10b0:0882
	UCMachine::I_numToStr,          // Intrinsic13F 13f0:0073
	// 0x140
	AudioProcess::I_playSFX,        // Intrinsic140 12d8:04d0
	AudioProcess::I_isSFXPlaying,   // Intrinsic141 12d8:05ba
	World::I_clrAlertActive ,       // Intrinsic142 10e8:0247
	PaletteFaderProcess::I_fadeToGivenColor, // Intrinsic143 1418:0b3b
	Actor::I_isDead,                // Intrinsic144 10f8:2278
	Actor::I_setDead,               // Intrinsic145 10f8:22a8
	Game::I_playCredits,            // Intrinsic146 1008:009c
	PaletteFaderProcess::I_jumpToAllWhite, // Intrinsic147 1418:0899
	Item::I_getFamilyOfType,        // Intrinsic148 1110:03ec
	Actor::I_getNpcNum,             // Intrinsic149 10b0:0cb2
	Item::I_getQLo,                 // Intrinsic14A 10b0:38a9
	Item::I_andStatus,              // Intrinsic14B 10b0:25cf
	Ultima8Engine::I_getCurrentTimerTick, // Intrinsic14C 1040:03b8
	World::I_setAlertActive,        // Intrinsic14D 10e8:0000
	Ultima8Engine::I_getAvatarInStasis, // Intrinsic14E 10f8:3d8e
	MainActor::I_addItemCru,        // Intrinsic14F 1008:03ab
	// 0x150
	Egg::I_getEggXRange,            // Intrinsic150 10a0:0513
	Actor::I_clrInCombat,           // Intrinsic151 10f8:389f
	PaletteFaderProcess::I_jumpToAllGivenColor, // Intrinsic152 1418:0923
	Item::I_setFrame,               // Intrinsic153 10b0:0882
	Actor::I_getDir,                // Intrinsic154 10f8:3e0c
	UCMachine::I_numToStr,          // Intrinsic155 13f0:0073
	Item::I_isOn,                   // Intrinsic156 10b0:1f2e
	Actor::I_getDir,                // Intrinsic157 10f8:3e0c
	Actor::I_setDead,               // Intrinsic158 10f8:22a8
	Item::I_getQHi,                 // Intrinsic159 10b0:385e
	Item::I_getQLo,                 // Intrinsic15A 10b0:38a9
	UCMachine::I_numToStr,          // Intrinsic15B 13f0:0073
	Actor::I_getDir,                // Intrinsic15C 10f8:3e0c
	0							    // UNUSED15D  Intrinsic15D 1050:0079
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
