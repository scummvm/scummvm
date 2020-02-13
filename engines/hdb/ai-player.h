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

#ifndef HDB_AI_PLAYER_H
#define HDB_AI_PLAYER_H

namespace HDB {

void aiPlayerInit(AIEntity *e);
void aiPlayerInit2(AIEntity *e);
void aiPlayerAction(AIEntity *e);
void aiPlayerDraw(AIEntity *e, int mx, int my);
void aiGemAttackInit(AIEntity *e);
void aiGemAttackInit2(AIEntity *e);
void aiGemAttackAction(AIEntity *e);
void aiChickenAction(AIEntity *e);
void aiChickenUse(AIEntity *e);
void aiChickenInit(AIEntity *e);
void aiChickenInit2(AIEntity *e);
void aiDollyInit(AIEntity *e);
void aiDollyInit2(AIEntity *e);
void aiSergeantInit(AIEntity *e);
void aiSergeantInit2(AIEntity *e);
void aiSergeantAction(AIEntity *e);
void aiSpacedudeInit(AIEntity *e);
void aiSpacedudeInit2(AIEntity *e);

void aiCrateAction(AIEntity *e);
void aiCrateInit2(AIEntity *e);
void aiCrateInit(AIEntity *e);
void aiBarrelLightAction(AIEntity *e);
void aiBarrelLightInit2(AIEntity *e);
void aiBarrelLightInit(AIEntity *e);
void aiBarrelHeavyAction(AIEntity *e);
void aiBarrelHeavyInit(AIEntity *e);
void aiBarrelHeavyInit2(AIEntity *e);
void aiBarrelExplosionEnd(AIEntity *e);
void aiBarrelExplosionAction(AIEntity *e);
void aiBarrelExplode(AIEntity *e);
void aiBarrelExplodeInit(AIEntity *e);
void aiBarrelExplodeInit2(AIEntity *e);
void aiBarrelExplodeAction(AIEntity *e);
void aiBarrelExplodeSpread(AIEntity *e);
void aiBarrelExplosionEnd(int x, int y);
void aiBarrelBlowup(AIEntity *e, int x, int y);

void aiMaintBotInit(AIEntity *e);
void aiMaintBotInit2(AIEntity *e);
void aiMaintBotAction(AIEntity *e);
void aiShockBotAction(AIEntity *e);
void aiShockBotShock(AIEntity *e, int mx, int my);
void aiShockBotInit(AIEntity *e);
void aiShockBotInit2(AIEntity *e);
void aiScientistInit(AIEntity *e);
void aiScientistInit2(AIEntity *e);
void aiFourFirerInit(AIEntity *e);
void aiFourFirerInit2(AIEntity *e);
void aiFourFirerAction(AIEntity *e);
void aiRailRiderInit(AIEntity *e);
void aiRailRiderInit2(AIEntity *e);
void aiRailRiderAction(AIEntity *e);
void aiRailRiderUse(AIEntity *e);
void aiRailRiderOnUse(AIEntity *e);
void aiRailRiderOnAction(AIEntity *e);
void aiOmniBotMissileAction(AIEntity *e);
void aiOmniBotMissileInit(AIEntity *e);
void aiOmniBotMissileInit2(AIEntity *e);

void aiSlugAttackAction(AIEntity *e);
void aiSlugAttackDraw(AIEntity *e, int mx, int my);
void aiSlugAttackInit(AIEntity *e);
void aiSlugAttackInit2(AIEntity *e);
void aiDeadWorkerInit(AIEntity *e);
void aiDeadWorkerInit2(AIEntity *e);
void aiWorkerInit(AIEntity *e);
void aiWorkerInit2(AIEntity *e);
void aiAccountantInit(AIEntity *e);
void aiAccountantInit2(AIEntity *e);
void aiFrogStatueInit(AIEntity *e);
void aiFrogStatueInit2(AIEntity *e);
void aiFrogStatueAction(AIEntity *e);

void aiRoboStunnerAction(AIEntity *e);
void aiRoboStunnerInit(AIEntity *e);
void aiRoboStunnerInit2(AIEntity *e);
void aiClubInit(AIEntity *e);
void aiClubInit2(AIEntity *e);
void aiSlugSlingerInit(AIEntity *e);
void aiSlugSlingerInit2(AIEntity *e);

void aiTurnBotAction(AIEntity *e);
void aiPushBotAction(AIEntity *e);
void aiOmniBotAction(AIEntity *e);
void aiOmniBotMove(AIEntity *e);
void aiLaserAction(AIEntity *e);
void aiLaserDraw(AIEntity *e, int mx, int my);
void aiDiverterAction(AIEntity *e);
void aiDiverterDraw(AIEntity *e, int mx, int my);
void aiRightBotAction(AIEntity *e);
void aiDeadEyeAction(AIEntity *e);
void aiMeerkatDraw(AIEntity *e, int mx, int my);
void aiMeerkatAction(AIEntity *e);
void aiMeerkatLookAround(AIEntity *e);
void aiDeadEyeWalkInPlace(AIEntity *e);
void aiFatFrogAction(AIEntity *e);
void aiFatFrogTongueDraw(AIEntity *e, int mx, int my);
void aiGoodFairyAction(AIEntity *e);
void aiBadFairyAction(AIEntity *e);
void aiGatePuddleAction(AIEntity *e);
void aiIcePuffSnowballAction(AIEntity *e);
void aiIcePuffSnowballDraw(AIEntity *e, int mx, int my);
void aiIcePuffAction(AIEntity *e);
void aiBuzzflyAction(AIEntity *e);
void aiDragonAction(AIEntity *e);
void aiDragonDraw(AIEntity *e, int mx, int my);

void aiTurnBotInit(AIEntity *e);
void aiTurnBotInit2(AIEntity *e);
void aiListenBotInit(AIEntity *e);
void aiListenBotInit2(AIEntity *e);
void aiOmniBotInit(AIEntity *e);
void aiOmniBotInit2(AIEntity *e);
void aiLaserAction(AIEntity *e);
void aiLaserInit(AIEntity *e);
void aiLaserInit2(AIEntity *e);
void aiDiverterInit(AIEntity *e);
void aiDiverterInit2(AIEntity *e);
void aiRightBotInit(AIEntity *e);
void aiRightBotInit2(AIEntity *e);
void aiPushBotInit(AIEntity *e);
void aiPushBotInit2(AIEntity *e);
void aiDeadEyeInit(AIEntity *e);
void aiDeadEyeInit2(AIEntity *e);
void aiMeerkatInit(AIEntity *e);
void aiMeerkatInit2(AIEntity *e);
void aiFatFrogInit(AIEntity *e);
void aiFatFrogInit2(AIEntity *e);
void aiGoodFairyInit(AIEntity *e);
void aiGoodFairyInit2(AIEntity *e);
void aiGoodFairyMoveaway(AIEntity *e);
void aiBadFairyInit(AIEntity *e);
void aiBadFairyInit2(AIEntity *e);
void aiGatePuddleInit(AIEntity *e);
void aiGatePuddleInit2(AIEntity *e);
void aiIcePuffInit(AIEntity *e);
void aiIcePuffInit2(AIEntity *e);
void aiBuzzflyInit(AIEntity *e);
void aiBuzzflyInit2(AIEntity *e);
void aiDragonInit(AIEntity *e);
void aiDragonInit2(AIEntity *e);
void aiDragonWake(AIEntity *e);
void aiDragonUse(AIEntity *e);

void aiEnvelopeGreenInit(AIEntity *e);
void aiEnvelopeGreenInit2(AIEntity *e);
void aiGemBlueInit(AIEntity *e);
void aiGemBlueInit2(AIEntity *e);
void aiGemRedInit(AIEntity *e);
void aiGemRedInit2(AIEntity *e);
void aiGemGreenInit(AIEntity *e);
void aiGemGreenInit2(AIEntity *e);

void aiTeaCupInit(AIEntity *e);
void aiTeaCupInit2(AIEntity *e);
void aiCookieInit(AIEntity *e);
void aiCookieInit2(AIEntity *e);
void aiBurgerInit(AIEntity *e);
void aiBurgerInit2(AIEntity *e);
void aiBookInit(AIEntity *e);
void aiBookInit2(AIEntity *e);
void aiClipboardInit(AIEntity *e);
void aiClipboardInit2(AIEntity *e);
void aiNoteInit(AIEntity *e);
void aiNoteInit2(AIEntity *e);
void aiKeycardWhiteInit(AIEntity *e);
void aiKeycardWhiteInit2(AIEntity *e);
void aiKeycardBlueInit(AIEntity *e);
void aiKeycardBlueInit2(AIEntity *e);
void aiKeycardRedInit(AIEntity *e);
void aiKeycardRedInit2(AIEntity *e);
void aiKeycardGreenInit(AIEntity *e);
void aiKeycardGreenInit2(AIEntity *e);
void aiKeycardPurpleInit(AIEntity *e);
void aiKeycardPurpleInit2(AIEntity *e);
void aiKeycardBlackInit(AIEntity *e);
void aiKeycardBlackInit2(AIEntity *e);
void aiSeedInit(AIEntity *e);
void aiSeedInit2(AIEntity *e);
void aiSodaInit(AIEntity *e);
void aiSodaInit2(AIEntity *e);
void aiDollyTool1Init(AIEntity *e);
void aiDollyTool1Init2(AIEntity *e);
void aiDollyTool2Init(AIEntity *e);
void aiDollyTool2Init2(AIEntity *e);
void aiDollyTool3Init(AIEntity *e);
void aiDollyTool3Init2(AIEntity *e);
void aiDollyTool4Init(AIEntity *e);
void aiDollyTool4Init2(AIEntity *e);
void aiRouterInit(AIEntity *e);
void aiRouterInit2(AIEntity *e);
void aiSlicerInit(AIEntity *e);
void aiSlicerInit2(AIEntity *e);
void aiPackageInit(AIEntity *e);
void aiPackageInit2(AIEntity *e);
void aiMagicEggAction(AIEntity *e);
void aiMagicEggInit(AIEntity *e);
void aiMagicEggInit2(AIEntity *e);
void aiMagicEggUse(AIEntity *e);
void aiIceBlockAction(AIEntity *e);
void aiIceBlockInit(AIEntity *e);
void aiIceBlockInit2(AIEntity *e);

void aiCabKeyInit(AIEntity *e);
void aiCabKeyInit2(AIEntity *e);
void aiItemChickenInit(AIEntity *e);
void aiItemChickenInit2(AIEntity *e);

void aiPdaInit(AIEntity *e);
void aiPdaInit2(AIEntity *e);
void aiCellUse(AIEntity *e);
void aiCellInit2(AIEntity *e);
void aiCellInit(AIEntity *e);
void aiEnvelopeWhiteInit(AIEntity *e);
void aiEnvelopeWhiteInit2(AIEntity *e);
void aiEnvelopeBlueInit(AIEntity *e);
void aiEnvelopeBlueInit2(AIEntity *e);
void aiEnvelopeRedInit(AIEntity *e);
void aiEnvelopeRedInit2(AIEntity *e);
void aiTransceiverInit(AIEntity *e);
void aiTransceiverInit2(AIEntity *e);
void aiTransceiverAction(AIEntity *e);
void aiTransceiverUse(AIEntity *e);
void aiMonkeystoneInit(AIEntity *e);
void aiMonkeystoneAction(AIEntity *e);
void aiMonkeystoneInit2(AIEntity *e);
void aiMonkeystoneUse(AIEntity *e);
void aiMonkeystoneUse2(AIEntity *e);
void aiGemAction(AIEntity *e);
void aiGemAction2(AIEntity *e);
void aiGemWhiteInit(AIEntity *e);
void aiGemWhiteInit2(AIEntity *e);
void aiGooCupUse(AIEntity *e);
void aiGooCupInit(AIEntity *e);
void aiGooCupInit2(AIEntity *e);

void aiVortexianAction(AIEntity *e);
void aiVortexianUse(AIEntity *e);
void aiVortexianInit(AIEntity *e);
void aiVortexianInit2(AIEntity *e);

void aiNoneInit(AIEntity *e);
void aiNoneInit2(AIEntity *e);

void callbackDoorOpenClose(int x, int y);
void callbackAutoDoorOpenClose(int x, int y);

// Utility Functions
void aiAnimateStanddown(AIEntity *e, int speed);
void aiGenericAction(AIEntity *e);
void aiGetItemAction(AIEntity *e);

} // End of Namespace

#endif // !HDB_AI_PLAYER_H
