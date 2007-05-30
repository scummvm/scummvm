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
 */

#include "kyra/kyra_v1.h"

namespace Kyra {

KyraEngine_v1::KyraEngine_v1(OSystem *system, const GameFlags &flags)
	: KyraEngine(system, flags) {
}

KyraEngine_v1::~KyraEngine_v1() {
}

#define Opcode(x) OpcodeV1(this, &KyraEngine_v1::x)
void KyraEngine_v1::setupOpcodeTable() {
	static const OpcodeV1 opcodeTable[] = {
		// 0x00
		Opcode(o1_magicInMouseItem),
		Opcode(o1_characterSays),
		Opcode(o1_pauseTicks),
		Opcode(o1_drawSceneAnimShape),
		// 0x04
		Opcode(o1_queryGameFlag),
		Opcode(o1_setGameFlag),
		Opcode(o1_resetGameFlag),
		Opcode(o1_runNPCScript),
		// 0x08
		Opcode(o1_setSpecialExitList),
		Opcode(o1_blockInWalkableRegion),
		Opcode(o1_blockOutWalkableRegion),
		Opcode(o1_walkPlayerToPoint),
		// 0x0c
		Opcode(o1_dropItemInScene),
		Opcode(o1_drawAnimShapeIntoScene),
		Opcode(o1_createMouseItem),
		Opcode(o1_savePageToDisk),
		// 0x10
		Opcode(o1_sceneAnimOn),
		Opcode(o1_sceneAnimOff),
		Opcode(o1_getElapsedSeconds),
		Opcode(o1_mouseIsPointer),
		// 0x14
		Opcode(o1_destroyMouseItem),
		Opcode(o1_runSceneAnimUntilDone),
		Opcode(o1_fadeSpecialPalette),
		Opcode(o1_playAdlibSound),
		// 0x18
		Opcode(o1_playAdlibScore),
		Opcode(o1_phaseInSameScene),
		Opcode(o1_setScenePhasingFlag),
		Opcode(o1_resetScenePhasingFlag),
		// 0x1c
		Opcode(o1_queryScenePhasingFlag),
		Opcode(o1_sceneToDirection),
		Opcode(o1_setBirthstoneGem),
		Opcode(o1_placeItemInGenericMapScene),
		// 0x20
		Opcode(o1_setBrandonStatusBit),
		Opcode(o1_pauseSeconds),
		Opcode(o1_getCharactersLocation),
		Opcode(o1_runNPCSubscript),
		// 0x24
		Opcode(o1_magicOutMouseItem),
		Opcode(o1_internalAnimOn),
		Opcode(o1_forceBrandonToNormal),
		Opcode(o1_poisonDeathNow),
		// 0x28
		Opcode(o1_setScaleMode),
		Opcode(o1_openWSAFile),
		Opcode(o1_closeWSAFile),
		Opcode(o1_runWSAFromBeginningToEnd),
		// 0x2c
		Opcode(o1_displayWSAFrame),
		Opcode(o1_enterNewScene),
		Opcode(o1_setSpecialEnterXAndY),
		Opcode(o1_runWSAFrames),
		// 0x30
		Opcode(o1_popBrandonIntoScene),
		Opcode(o1_restoreAllObjectBackgrounds),
		Opcode(o1_setCustomPaletteRange),
		Opcode(o1_loadPageFromDisk),
		// 0x34
		Opcode(o1_customPrintTalkString),
		Opcode(o1_restoreCustomPrintBackground),
		Opcode(o1_hideMouse),
		Opcode(o1_showMouse),
		// 0x38
		Opcode(o1_getCharacterX),
		Opcode(o1_getCharacterY),
		Opcode(o1_changeCharactersFacing),
		Opcode(o1_copyWSARegion),
		// 0x3c
		Opcode(o1_printText),
		Opcode(o1_random),
		Opcode(o1_loadSoundFile),
		Opcode(o1_displayWSAFrameOnHidPage),
		// 0x40
		Opcode(o1_displayWSASequentialFrames),
		Opcode(o1_drawCharacterStanding),
		Opcode(o1_internalAnimOff),
		Opcode(o1_changeCharactersXAndY),
		// 0x44
		Opcode(o1_clearSceneAnimatorBeacon),
		Opcode(o1_querySceneAnimatorBeacon),
		Opcode(o1_refreshSceneAnimator),
		Opcode(o1_placeItemInOffScene),
		// 0x48
		Opcode(o1_wipeDownMouseItem),
		Opcode(o1_placeCharacterInOtherScene),
		Opcode(o1_getKey),
		Opcode(o1_specificItemInInventory),
		// 0x4c
		Opcode(o1_popMobileNPCIntoScene),
		Opcode(o1_mobileCharacterInScene),
		Opcode(o1_hideMobileCharacter),
		Opcode(o1_unhideMobileCharacter),
		// 0x50
		Opcode(o1_setCharactersLocation),
		Opcode(o1_walkCharacterToPoint),
		Opcode(o1_specialEventDisplayBrynnsNote),
		Opcode(o1_specialEventRemoveBrynnsNote),
		// 0x54
		Opcode(o1_setLogicPage),
		Opcode(o1_fatPrint),
		Opcode(o1_preserveAllObjectBackgrounds),
		Opcode(o1_updateSceneAnimations),
		// 0x58
		Opcode(o1_sceneAnimationActive),
		Opcode(o1_setCharactersMovementDelay),
		Opcode(o1_getCharactersFacing),
		Opcode(o1_bkgdScrollSceneAndMasksRight),
		// 0x5c
		Opcode(o1_dispelMagicAnimation),
		Opcode(o1_findBrightestFireberry),
		Opcode(o1_setFireberryGlowPalette),
		Opcode(o1_setDeathHandlerFlag),
		// 0x60
		Opcode(o1_drinkPotionAnimation),
		Opcode(o1_makeAmuletAppear),
		Opcode(o1_drawItemShapeIntoScene),
		Opcode(o1_setCharactersCurrentFrame),
		// 0x64
		Opcode(o1_waitForConfirmationMouseClick),
		Opcode(o1_pageFlip),
		Opcode(o1_setSceneFile),
		Opcode(o1_getItemInMarbleVase),
		// 0x68
		Opcode(o1_setItemInMarbleVase),
		Opcode(o1_addItemToInventory),
		Opcode(o1_intPrint),
		Opcode(o1_shakeScreen),
		// 0x6c
		Opcode(o1_createAmuletJewel),
		Opcode(o1_setSceneAnimCurrXY),
		Opcode(o1_poisonBrandonAndRemaps),
		Opcode(o1_fillFlaskWithWater),
		// 0x70
		Opcode(o1_getCharactersMovementDelay),
		Opcode(o1_getBirthstoneGem),
		Opcode(o1_queryBrandonStatusBit),
		Opcode(o1_playFluteAnimation),
		// 0x74
		Opcode(o1_playWinterScrollSequence),
		Opcode(o1_getIdolGem),
		Opcode(o1_setIdolGem),
		Opcode(o1_totalItemsInScene),
		// 0x78
		Opcode(o1_restoreBrandonsMovementDelay),
		Opcode(o1_setMousePos),
		Opcode(o1_getMouseState),
		Opcode(o1_setEntranceMouseCursorTrack),
		// 0x7c
		Opcode(o1_itemAppearsOnGround),
		Opcode(o1_setNoDrawShapesFlag),
		Opcode(o1_fadeEntirePalette),
		Opcode(o1_itemOnGroundHere),
		// 0x80
		Opcode(o1_queryCauldronState),
		Opcode(o1_setCauldronState),
		Opcode(o1_queryCrystalState),
		Opcode(o1_setCrystalState),
		// 0x84
		Opcode(o1_setPaletteRange),
		Opcode(o1_shrinkBrandonDown),
		Opcode(o1_growBrandonUp),
		Opcode(o1_setBrandonScaleXAndY),
		// 0x88
		Opcode(o1_resetScaleMode),
		Opcode(o1_getScaleDepthTableValue),
		Opcode(o1_setScaleDepthTableValue),
		Opcode(o1_message),
		// 0x8c
		Opcode(o1_checkClickOnNPC),
		Opcode(o1_getFoyerItem),
		Opcode(o1_setFoyerItem),
		Opcode(o1_setNoItemDropRegion),
		// 0x90
		Opcode(o1_walkMalcolmOn),
		Opcode(o1_passiveProtection),
		Opcode(o1_setPlayingLoop),
		Opcode(o1_brandonToStoneSequence),
		// 0x94
		Opcode(o1_brandonHealingSequence),
		Opcode(o1_protectCommandLine),
		Opcode(o1_pauseMusicSeconds),
		Opcode(o1_resetMaskRegion),
		// 0x98
		Opcode(o1_setPaletteChangeFlag),
		Opcode(o1_fillRect),
		Opcode(o1_vocUnload),
		Opcode(o1_vocLoad),
		Opcode(o1_dummy)
	};
	
	for (int i = 0; i < ARRAYSIZE(opcodeTable); ++i)
		_opcodes.push_back(&opcodeTable[i]);
}
#undef Opcode

} // end of namespace Kyra

