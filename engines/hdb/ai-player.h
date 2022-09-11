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

#ifndef HDB_AI_PLAYER_H
#define HDB_AI_PLAYER_H

namespace HDB {

void aiPlayerInit(AIEntity *e, int mx, int my);
void aiPlayerInit2(AIEntity *e, int mx, int my);
void aiPlayerAction(AIEntity *e, int mx, int my);
void aiPlayerDraw(AIEntity *e, int mx, int my);
void aiGemAttackInit(AIEntity *e, int mx, int my);
void aiGemAttackInit2(AIEntity *e, int mx, int my);
void aiGemAttackAction(AIEntity *e, int mx, int my);
void aiChickenAction(AIEntity *e, int mx, int my);
void aiChickenUse(AIEntity *e, int mx, int my);
void aiChickenInit(AIEntity *e, int mx, int my);
void aiChickenInit2(AIEntity *e, int mx, int my);
void aiDollyInit(AIEntity *e, int mx, int my);
void aiDollyInit2(AIEntity *e, int mx, int my);
void aiSergeantInit(AIEntity *e, int mx, int my);
void aiSergeantInit2(AIEntity *e, int mx, int my);
void aiSergeantAction(AIEntity *e, int mx, int my);
void aiSpacedudeInit(AIEntity *e, int mx, int my);
void aiSpacedudeInit2(AIEntity *e, int mx, int my);

void aiCrateAction(AIEntity *e, int mx, int my);
void aiCrateInit2(AIEntity *e, int mx, int my);
void aiCrateInit(AIEntity *e, int mx, int my);
void aiBarrelLightAction(AIEntity *e, int mx, int my);
void aiBarrelLightInit2(AIEntity *e, int mx, int my);
void aiBarrelLightInit(AIEntity *e, int mx, int my);
void aiBarrelHeavyAction(AIEntity *e, int mx, int my);
void aiBarrelHeavyInit(AIEntity *e, int mx, int my);
void aiBarrelHeavyInit2(AIEntity *e, int mx, int my);
void aiBarrelExplode(AIEntity *e, int mx, int my);
void aiBarrelExplodeInit(AIEntity *e, int mx, int my);
void aiBarrelExplodeInit2(AIEntity *e, int mx, int my);
void aiBarrelExplodeAction(AIEntity *e, int mx, int my);
void aiBarrelExplodeSpread(AIEntity *e, int mx, int my);
void aiBarrelBlowup(AIEntity *e, int x, int y);

void aiMaintBotInit(AIEntity *e, int mx, int my);
void aiMaintBotInit2(AIEntity *e, int mx, int my);
void aiMaintBotAction(AIEntity *e, int mx, int my);
void aiShockBotAction(AIEntity *e, int mx, int my);
void aiShockBotShock(AIEntity *e, int mx, int my);
void aiShockBotInit(AIEntity *e, int mx, int my);
void aiShockBotInit2(AIEntity *e, int mx, int my);
void aiScientistInit(AIEntity *e, int mx, int my);
void aiScientistInit2(AIEntity *e, int mx, int my);
void aiFourFirerInit(AIEntity *e, int mx, int my);
void aiFourFirerInit2(AIEntity *e, int mx, int my);
void aiFourFirerAction(AIEntity *e, int mx, int my);
void aiRailRiderInit(AIEntity *e, int mx, int my);
void aiRailRiderInit2(AIEntity *e, int mx, int my);
void aiRailRiderAction(AIEntity *e, int mx, int my);
void aiRailRiderUse(AIEntity *e, int mx, int my);
void aiRailRiderOnUse(AIEntity *e, int mx, int my);
void aiRailRiderOnAction(AIEntity *e, int mx, int my);
void aiOmniBotMissileAction(AIEntity *e, int mx, int my);
void aiOmniBotMissileInit(AIEntity *e, int mx, int my);
void aiOmniBotMissileInit2(AIEntity *e, int mx, int my);

void aiSlugAttackAction(AIEntity *e, int mx, int my);
void aiSlugAttackDraw(AIEntity *e, int mx, int my);
void aiSlugAttackInit(AIEntity *e, int mx, int my);
void aiSlugAttackInit2(AIEntity *e, int mx, int my);
void aiDeadWorkerInit(AIEntity *e, int mx, int my);
void aiDeadWorkerInit2(AIEntity *e, int mx, int my);
void aiWorkerInit(AIEntity *e, int mx, int my);
void aiWorkerInit2(AIEntity *e, int mx, int my);
void aiAccountantInit(AIEntity *e, int mx, int my);
void aiAccountantInit2(AIEntity *e, int mx, int my);
void aiFrogStatueInit(AIEntity *e, int mx, int my);
void aiFrogStatueInit2(AIEntity *e, int mx, int my);
void aiFrogStatueAction(AIEntity *e, int mx, int my);

void aiRoboStunnerAction(AIEntity *e, int mx, int my);
void aiRoboStunnerInit(AIEntity *e, int mx, int my);
void aiRoboStunnerInit2(AIEntity *e, int mx, int my);
void aiClubInit(AIEntity *e, int mx, int my);
void aiClubInit2(AIEntity *e, int mx, int my);
void aiSlugSlingerInit(AIEntity *e, int mx, int my);
void aiSlugSlingerInit2(AIEntity *e, int mx, int my);

void aiTurnBotAction(AIEntity *e, int mx, int my);
void aiPushBotAction(AIEntity *e, int mx, int my);
void aiOmniBotAction(AIEntity *e, int mx, int my);
void aiOmniBotMove(AIEntity *e, int mx, int my);
void aiLaserAction(AIEntity *e, int mx, int my);
void aiLaserDraw(AIEntity *e, int mx, int my);
void aiDiverterAction(AIEntity *e, int mx, int my);
void aiDiverterDraw(AIEntity *e, int mx, int my);
void aiRightBotAction(AIEntity *e, int mx, int my);
void aiDeadEyeAction(AIEntity *e, int mx, int my);
void aiMeerkatDraw(AIEntity *e, int mx, int my);
void aiMeerkatAction(AIEntity *e, int mx, int my);
void aiMeerkatLookAround(AIEntity *e, int mx, int my);
void aiDeadEyeWalkInPlace(AIEntity *e, int mx, int my);
void aiFatFrogAction(AIEntity *e, int mx, int my);
void aiFatFrogTongueDraw(AIEntity *e, int mx, int my);
void aiGoodFairyAction(AIEntity *e, int mx, int my);
void aiBadFairyAction(AIEntity *e, int mx, int my);
void aiGatePuddleAction(AIEntity *e, int mx, int my);
void aiIcePuffSnowballAction(AIEntity *e, int mx, int my);
void aiIcePuffSnowballDraw(AIEntity *e, int mx, int my);
void aiIcePuffAction(AIEntity *e, int mx, int my);
void aiBuzzflyAction(AIEntity *e, int mx, int my);
void aiDragonAction(AIEntity *e, int mx, int my);
void aiDragonDraw(AIEntity *e, int mx, int my);
void aiTurnBotInit(AIEntity *e, int mx, int my);
void aiTurnBotInit2(AIEntity *e, int mx, int my);
void aiOmniBotInit(AIEntity *e, int mx, int my);
void aiOmniBotInit2(AIEntity *e, int mx, int my);
void aiLaserInit(AIEntity *e, int mx, int my);
void aiLaserInit2(AIEntity *e, int mx, int my);
void aiDiverterInit(AIEntity *e, int mx, int my);
void aiDiverterInit2(AIEntity *e, int mx, int my);
void aiRightBotInit(AIEntity *e, int mx, int my);
void aiRightBotInit2(AIEntity *e, int mx, int my);
void aiPushBotInit(AIEntity *e, int mx, int my);
void aiPushBotInit2(AIEntity *e, int mx, int my);
void aiDeadEyeInit(AIEntity *e, int mx, int my);
void aiDeadEyeInit2(AIEntity *e, int mx, int my);
void aiMeerkatInit(AIEntity *e, int mx, int my);
void aiMeerkatInit2(AIEntity *e, int mx, int my);
void aiFatFrogInit(AIEntity *e, int mx, int my);
void aiFatFrogInit2(AIEntity *e, int mx, int my);
void aiGoodFairyInit(AIEntity *e, int mx, int my);
void aiGoodFairyInit2(AIEntity *e, int mx, int my);
void aiBadFairyInit(AIEntity *e, int mx, int my);
void aiBadFairyInit2(AIEntity *e, int mx, int my);
void aiGatePuddleInit(AIEntity *e, int mx, int my);
void aiGatePuddleInit2(AIEntity *e, int mx, int my);
void aiIcePuffInit(AIEntity *e, int mx, int my);
void aiIcePuffInit2(AIEntity *e, int mx, int my);
void aiBuzzflyInit(AIEntity *e, int mx, int my);
void aiBuzzflyInit2(AIEntity *e, int mx, int my);
void aiDragonInit(AIEntity *e, int mx, int my);
void aiDragonInit2(AIEntity *e, int mx, int my);
void aiDragonWake(AIEntity *e, int mx, int my);
void aiDragonUse(AIEntity *e, int mx, int my);

void aiEnvelopeGreenInit(AIEntity *e, int mx, int my);
void aiEnvelopeGreenInit2(AIEntity *e, int mx, int my);
void aiGemBlueInit(AIEntity *e, int mx, int my);
void aiGemBlueInit2(AIEntity *e, int mx, int my);
void aiGemRedInit(AIEntity *e, int mx, int my);
void aiGemRedInit2(AIEntity *e, int mx, int my);
void aiGemGreenInit(AIEntity *e, int mx, int my);
void aiGemGreenInit2(AIEntity *e, int mx, int my);

void aiTeaCupInit(AIEntity *e, int mx, int my);
void aiTeaCupInit2(AIEntity *e, int mx, int my);
void aiCookieInit(AIEntity *e, int mx, int my);
void aiCookieInit2(AIEntity *e, int mx, int my);
void aiBurgerInit(AIEntity *e, int mx, int my);
void aiBurgerInit2(AIEntity *e, int mx, int my);
void aiBookInit(AIEntity *e, int mx, int my);
void aiBookInit2(AIEntity *e, int mx, int my);
void aiClipboardInit(AIEntity *e, int mx, int my);
void aiClipboardInit2(AIEntity *e, int mx, int my);
void aiNoteInit(AIEntity *e, int mx, int my);
void aiNoteInit2(AIEntity *e, int mx, int my);
void aiKeycardWhiteInit(AIEntity *e, int mx, int my);
void aiKeycardWhiteInit2(AIEntity *e, int mx, int my);
void aiKeycardBlueInit(AIEntity *e, int mx, int my);
void aiKeycardBlueInit2(AIEntity *e, int mx, int my);
void aiKeycardRedInit(AIEntity *e, int mx, int my);
void aiKeycardRedInit2(AIEntity *e, int mx, int my);
void aiKeycardGreenInit(AIEntity *e, int mx, int my);
void aiKeycardGreenInit2(AIEntity *e, int mx, int my);
void aiKeycardPurpleInit(AIEntity *e, int mx, int my);
void aiKeycardPurpleInit2(AIEntity *e, int mx, int my);
void aiKeycardBlackInit(AIEntity *e, int mx, int my);
void aiKeycardBlackInit2(AIEntity *e, int mx, int my);
void aiSeedInit(AIEntity *e, int mx, int my);
void aiSeedInit2(AIEntity *e, int mx, int my);
void aiSodaInit(AIEntity *e, int mx, int my);
void aiSodaInit2(AIEntity *e, int mx, int my);
void aiDollyTool1Init(AIEntity *e, int mx, int my);
void aiDollyTool1Init2(AIEntity *e, int mx, int my);
void aiDollyTool2Init(AIEntity *e, int mx, int my);
void aiDollyTool2Init2(AIEntity *e, int mx, int my);
void aiDollyTool3Init(AIEntity *e, int mx, int my);
void aiDollyTool3Init2(AIEntity *e, int mx, int my);
void aiDollyTool4Init(AIEntity *e, int mx, int my);
void aiDollyTool4Init2(AIEntity *e, int mx, int my);
void aiRouterInit(AIEntity *e, int mx, int my);
void aiRouterInit2(AIEntity *e, int mx, int my);
void aiSlicerInit(AIEntity *e, int mx, int my);
void aiSlicerInit2(AIEntity *e, int mx, int my);
void aiPackageInit(AIEntity *e, int mx, int my);
void aiPackageInit2(AIEntity *e, int mx, int my);
void aiMagicEggAction(AIEntity *e, int mx, int my);
void aiMagicEggInit(AIEntity *e, int mx, int my);
void aiMagicEggInit2(AIEntity *e, int mx, int my);
void aiMagicEggUse(AIEntity *e, int mx, int my);
void aiIceBlockAction(AIEntity *e, int mx, int my);
void aiIceBlockInit(AIEntity *e, int mx, int my);
void aiIceBlockInit2(AIEntity *e, int mx, int my);

void aiCabKeyInit(AIEntity *e, int mx, int my);
void aiCabKeyInit2(AIEntity *e, int mx, int my);
void aiItemChickenInit(AIEntity *e, int mx, int my);
void aiItemChickenInit2(AIEntity *e, int mx, int my);

void aiPdaInit(AIEntity *e, int mx, int my);
void aiPdaInit2(AIEntity *e, int mx, int my);
#if 0
void aiCellUse(AIEntity *e, int mx, int my);
#endif
void aiCellInit2(AIEntity *e, int mx, int my);
void aiCellInit(AIEntity *e, int mx, int my);
void aiEnvelopeWhiteInit(AIEntity *e, int mx, int my);
void aiEnvelopeWhiteInit2(AIEntity *e, int mx, int my);
void aiEnvelopeBlueInit(AIEntity *e, int mx, int my);
void aiEnvelopeBlueInit2(AIEntity *e, int mx, int my);
void aiEnvelopeRedInit(AIEntity *e, int mx, int my);
void aiEnvelopeRedInit2(AIEntity *e, int mx, int my);
void aiTransceiverInit(AIEntity *e, int mx, int my);
void aiTransceiverInit2(AIEntity *e, int mx, int my);
void aiTransceiverAction(AIEntity *e, int mx, int my);
#if 0
void aiTransceiverUse(AIEntity *e, int mx, int my);
#endif
void aiMonkeystoneInit(AIEntity *e, int mx, int my);
void aiMonkeystoneAction(AIEntity *e, int mx, int my);
void aiMonkeystoneInit2(AIEntity *e, int mx, int my);
void aiMonkeystoneUse(AIEntity *e, int mx, int my);
void aiGemAction(AIEntity *e, int mx, int my);
void aiGemWhiteInit(AIEntity *e, int mx, int my);
void aiGemWhiteInit2(AIEntity *e, int mx, int my);
void aiGooCupUse(AIEntity *e, int mx, int my);
void aiGooCupInit(AIEntity *e, int mx, int my);
void aiGooCupInit2(AIEntity *e, int mx, int my);

void aiVortexianAction(AIEntity *e, int mx, int my);
void aiVortexianUse(AIEntity *e, int mx, int my);
void aiVortexianInit(AIEntity *e, int mx, int my);
void aiVortexianInit2(AIEntity *e, int mx, int my);

void aiNoneInit(AIEntity *e, int mx, int my);
void aiNoneInit2(AIEntity *e, int mx, int my);

void callbackDoorOpenClose(int x, int y);
void callbackAutoDoorOpenClose(int x, int y);
void callbackAiBarrelExplosionEnd(int x, int y);

// Utility Functions
void aiAnimateStanddown(AIEntity *e, int speed);
void aiGenericAction(AIEntity *e, int mx, int my);
void aiGetItemAction(AIEntity *e, int mx, int my);

} // End of Namespace

#endif // !HDB_AI_PLAYER_H
