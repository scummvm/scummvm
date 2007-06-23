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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/kyra/kyra_v1.h $
 * $Id:kyra_v1.h 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#ifndef KYRA_KYRA_V1_H
#define KYRA_KYRA_V1_H

#include "kyra/kyra.h"
#include "kyra/script.h"

namespace Kyra {

class KyraEngine_v1 : public KyraEngine {
public:
	KyraEngine_v1(OSystem *system, const GameFlags &flags);
	~KyraEngine_v1();

protected:
	typedef OpcodeImpl<KyraEngine_v1> OpcodeV1;
	void setupOpcodeTable();

	// Opcodes
	int o1_magicInMouseItem(ScriptState *script);
	int o1_characterSays(ScriptState *script);
	int o1_pauseTicks(ScriptState *script);
	int o1_drawSceneAnimShape(ScriptState *script);
	int o1_queryGameFlag(ScriptState *script);
	int o1_setGameFlag(ScriptState *script);
	int o1_resetGameFlag(ScriptState *script);
	int o1_runNPCScript(ScriptState *script);
	int o1_setSpecialExitList(ScriptState *script);
	int o1_blockInWalkableRegion(ScriptState *script);
	int o1_blockOutWalkableRegion(ScriptState *script);
	int o1_walkPlayerToPoint(ScriptState *script);
	int o1_dropItemInScene(ScriptState *script);
	int o1_drawAnimShapeIntoScene(ScriptState *script);
	int o1_createMouseItem(ScriptState *script);
	int o1_savePageToDisk(ScriptState *script);
	int o1_sceneAnimOn(ScriptState *script);
	int o1_sceneAnimOff(ScriptState *script);
	int o1_getElapsedSeconds(ScriptState *script);
	int o1_mouseIsPointer(ScriptState *script);
	int o1_destroyMouseItem(ScriptState *script);
	int o1_runSceneAnimUntilDone(ScriptState *script);
	int o1_fadeSpecialPalette(ScriptState *script);
	int o1_playAdlibSound(ScriptState *script);
	int o1_playAdlibScore(ScriptState *script);
	int o1_phaseInSameScene(ScriptState *script);
	int o1_setScenePhasingFlag(ScriptState *script);
	int o1_resetScenePhasingFlag(ScriptState *script);
	int o1_queryScenePhasingFlag(ScriptState *script);
	int o1_sceneToDirection(ScriptState *script);
	int o1_setBirthstoneGem(ScriptState *script);
	int o1_placeItemInGenericMapScene(ScriptState *script);
	int o1_setBrandonStatusBit(ScriptState *script);
	int o1_pauseSeconds(ScriptState *script);
	int o1_getCharactersLocation(ScriptState *script);
	int o1_runNPCSubscript(ScriptState *script);
	int o1_magicOutMouseItem(ScriptState *script);
	int o1_internalAnimOn(ScriptState *script);
	int o1_forceBrandonToNormal(ScriptState *script);
	int o1_poisonDeathNow(ScriptState *script);
	int o1_setScaleMode(ScriptState *script);
	int o1_openWSAFile(ScriptState *script);
	int o1_closeWSAFile(ScriptState *script);
	int o1_runWSAFromBeginningToEnd(ScriptState *script);
	int o1_displayWSAFrame(ScriptState *script);
	int o1_enterNewScene(ScriptState *script);
	int o1_setSpecialEnterXAndY(ScriptState *script);
	int o1_runWSAFrames(ScriptState *script);
	int o1_popBrandonIntoScene(ScriptState *script);
	int o1_restoreAllObjectBackgrounds(ScriptState *script);
	int o1_setCustomPaletteRange(ScriptState *script);
	int o1_loadPageFromDisk(ScriptState *script);
	int o1_customPrintTalkString(ScriptState *script);
	int o1_restoreCustomPrintBackground(ScriptState *script);
	int o1_hideMouse(ScriptState *script);
	int o1_showMouse(ScriptState *script);
	int o1_getCharacterX(ScriptState *script);
	int o1_getCharacterY(ScriptState *script);
	int o1_changeCharactersFacing(ScriptState *script);
	int o1_copyWSARegion(ScriptState *script);
	int o1_printText(ScriptState *script);
	int o1_random(ScriptState *script);
	int o1_loadSoundFile(ScriptState *script);
	int o1_displayWSAFrameOnHidPage(ScriptState *script);
	int o1_displayWSASequentialFrames(ScriptState *script);
	int o1_drawCharacterStanding(ScriptState *script);
	int o1_internalAnimOff(ScriptState *script);
	int o1_changeCharactersXAndY(ScriptState *script);
	int o1_clearSceneAnimatorBeacon(ScriptState *script);
	int o1_querySceneAnimatorBeacon(ScriptState *script);
	int o1_refreshSceneAnimator(ScriptState *script);
	int o1_placeItemInOffScene(ScriptState *script);
	int o1_wipeDownMouseItem(ScriptState *script);
	int o1_placeCharacterInOtherScene(ScriptState *script);
	int o1_getKey(ScriptState *script);
	int o1_specificItemInInventory(ScriptState *script);
	int o1_popMobileNPCIntoScene(ScriptState *script);
	int o1_mobileCharacterInScene(ScriptState *script);
	int o1_hideMobileCharacter(ScriptState *script);
	int o1_unhideMobileCharacter(ScriptState *script);
	int o1_setCharactersLocation(ScriptState *script);
	int o1_walkCharacterToPoint(ScriptState *script);
	int o1_specialEventDisplayBrynnsNote(ScriptState *script);
	int o1_specialEventRemoveBrynnsNote(ScriptState *script);
	int o1_setLogicPage(ScriptState *script);
	int o1_fatPrint(ScriptState *script);
	int o1_preserveAllObjectBackgrounds(ScriptState *script);
	int o1_updateSceneAnimations(ScriptState *script);
	int o1_sceneAnimationActive(ScriptState *script);
	int o1_setCharactersMovementDelay(ScriptState *script);
	int o1_getCharactersFacing(ScriptState *script);
	int o1_bkgdScrollSceneAndMasksRight(ScriptState *script);
	int o1_dispelMagicAnimation(ScriptState *script);
	int o1_findBrightestFireberry(ScriptState *script);
	int o1_setFireberryGlowPalette(ScriptState *script);
	int o1_setDeathHandlerFlag(ScriptState *script);
	int o1_drinkPotionAnimation(ScriptState *script);
	int o1_makeAmuletAppear(ScriptState *script);
	int o1_drawItemShapeIntoScene(ScriptState *script);
	int o1_setCharactersCurrentFrame(ScriptState *script);
	int o1_waitForConfirmationMouseClick(ScriptState *script);
	int o1_pageFlip(ScriptState *script);
	int o1_setSceneFile(ScriptState *script);
	int o1_getItemInMarbleVase(ScriptState *script);
	int o1_setItemInMarbleVase(ScriptState *script);
	int o1_addItemToInventory(ScriptState *script);
	int o1_intPrint(ScriptState *script);
	int o1_shakeScreen(ScriptState *script);
	int o1_createAmuletJewel(ScriptState *script);
	int o1_setSceneAnimCurrXY(ScriptState *script);
	int o1_poisonBrandonAndRemaps(ScriptState *script);
	int o1_fillFlaskWithWater(ScriptState *script);
	int o1_getCharactersMovementDelay(ScriptState *script);
	int o1_getBirthstoneGem(ScriptState *script);
	int o1_queryBrandonStatusBit(ScriptState *script);
	int o1_playFluteAnimation(ScriptState *script);
	int o1_playWinterScrollSequence(ScriptState *script);
	int o1_getIdolGem(ScriptState *script);
	int o1_setIdolGem(ScriptState *script);
	int o1_totalItemsInScene(ScriptState *script);
	int o1_restoreBrandonsMovementDelay(ScriptState *script);
	int o1_setMousePos(ScriptState *script);
	int o1_getMouseState(ScriptState *script);
	int o1_setEntranceMouseCursorTrack(ScriptState *script);
	int o1_itemAppearsOnGround(ScriptState *script);
	int o1_setNoDrawShapesFlag(ScriptState *script);
	int o1_fadeEntirePalette(ScriptState *script);
	int o1_itemOnGroundHere(ScriptState *script);
	int o1_queryCauldronState(ScriptState *script);
	int o1_setCauldronState(ScriptState *script);
	int o1_queryCrystalState(ScriptState *script);
	int o1_setCrystalState(ScriptState *script);
	int o1_setPaletteRange(ScriptState *script);
	int o1_shrinkBrandonDown(ScriptState *script);
	int o1_growBrandonUp(ScriptState *script);
	int o1_setBrandonScaleXAndY(ScriptState *script);
	int o1_resetScaleMode(ScriptState *script);
	int o1_getScaleDepthTableValue(ScriptState *script);
	int o1_setScaleDepthTableValue(ScriptState *script);
	int o1_message(ScriptState *script);
	int o1_checkClickOnNPC(ScriptState *script);
	int o1_getFoyerItem(ScriptState *script);
	int o1_setFoyerItem(ScriptState *script);
	int o1_setNoItemDropRegion(ScriptState *script);
	int o1_walkMalcolmOn(ScriptState *script);
	int o1_passiveProtection(ScriptState *script);
	int o1_setPlayingLoop(ScriptState *script);
	int o1_brandonToStoneSequence(ScriptState *script);
	int o1_brandonHealingSequence(ScriptState *script);
	int o1_protectCommandLine(ScriptState *script);
	int o1_pauseMusicSeconds(ScriptState *script);
	int o1_resetMaskRegion(ScriptState *script);
	int o1_setPaletteChangeFlag(ScriptState *script);
	int o1_fillRect(ScriptState *script);
	int o1_dummy(ScriptState *script);
	int o1_vocUnload(ScriptState *script);
	int o1_vocLoad(ScriptState *script);
};

} // end of namespace Kyra

#endif

