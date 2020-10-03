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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/gameloader.h"
#include "ngi/objectnames.h"
#include "ngi/input.h"

#include "ngi/constants.h"

namespace NGI {

void NGIEngine::initObjectStates() {
	setLevelStates();

	setObjectState(sO_Dude, getObjectEnumState(sO_Dude, sO_NotCarryingEgg));
	setObjectState(sO_EggCracker, getObjectEnumState(sO_EggCracker, sO_DidNotCrackEgg));
	setObjectState(sO_GuvTheDrawer, getObjectEnumState(sO_GuvTheDrawer, sO_Awaken));
	setObjectState(sO_EggGulper, getObjectEnumState(sO_EggGulper, sO_First));
	setObjectState(sO_EggGulperGaveCoin, getObjectEnumState(sO_EggGulperGaveCoin, sO_No));
	setObjectState(sO_Jar_4, getObjectEnumState(sO_Jar_4, sO_OnTheSpring));
	setObjectState(sO_GulpedEggs, getObjectEnumState(sO_GulpedEgg, sO_NotPresent));

	setSwallowedEggsState();

	setObjectState(sO_WeirdWacko, getObjectEnumState(sO_WeirdWacko, sO_InGlasses));
	setObjectState(sO_TummyTrampie, getObjectEnumState(sO_TummyTrampie, sO_IsDrinking));
	setObjectState(sO_StairsUp_8, getObjectEnumState(sO_StairsUp_8, sO_NotBroken));
	setObjectState(sO_HareTheNooksiter, getObjectEnumState(sO_HareTheNooksiter, sO_WithHandle));
	setObjectState(sO_Elephantine, getObjectEnumState(sO_Elephantine, sO_WithBoot));
	setObjectState(sO_Fly_12, 0);
	setObjectState(sO_ClockAxis, getObjectEnumState(sO_ClockAxis, sO_IsNotAvailable));
	setObjectState(sO_ClockHandle, getObjectEnumState(sO_ClockHandle, sO_In_7));
	setObjectState(sO_BigMumsy, getObjectEnumState(sO_BigMumsy, sO_IsSleeping));
	setObjectState(sO_CoinSlot_1, getObjectEnumState(sO_CoinSlot_1, sO_Empty));
	setObjectState(sO_FriesPit, getObjectEnumState(sO_FriesPit, sO_WithApple));
	setObjectState(sO_Jug, getObjectEnumState(sO_Jug, sO_Blocked));
	setObjectState(sO_RightStairs_9, getObjectEnumState(sO_RightStairs_9, sO_IsClosed));
	if (isDemo() && getLanguage() == Common::RU_RUS)
		setObjectState(sO_Pipe_9, getObjectEnumState(sO_Pipe_9, sO_WithoutJug));
	else
		setObjectState(sO_Pipe_9, getObjectEnumState(sO_Pipe_9, sO_WithJug));
	setObjectState(sO_Inflater, getObjectEnumState(sO_Inflater, sO_WithGum));
	setObjectState(sO_Swingie, getObjectEnumState(sO_Swingie, sO_IsSwinging));
	setObjectState(sO_DudeHasJumped, getObjectEnumState(sO_DudeHasJumped, sO_No));
	setObjectState(sO_Bridge, getObjectEnumState(sO_Bridge, sO_Convoluted));
	setObjectState(sO_Guardian, getObjectEnumState(sO_Guardian, sO_OnRight));
	setObjectState(sO_Grandma, getObjectEnumState(sO_Grandma, sO_In_14));
	setObjectState(sO_Boot_15, getObjectEnumState(sO_Boot_15, sO_NotPresent));
	setObjectState(sO_LeftPipe_15, getObjectEnumState(sO_LeftPipe_15, sO_IsOpened));
	setObjectState(sO_Pedestal_16, getObjectEnumState(sO_Pedestal_16, sO_IsFree));
	setObjectState(sO_Cup, getObjectEnumState(sO_Cup, sO_InSmokeRoom));
	setObjectState(sO_Pedestal_17, getObjectEnumState(sO_Pedestal_17, sO_IsFree));
	setObjectState(sO_UsherHand, getObjectEnumState(sO_UsherHand, sO_WithoutCoin));
	setObjectState(sO_RightPipe_17, getObjectEnumState(sO_RightPipe_17, sO_IsClosed));
	setObjectState(sO_Fly_17, 1);
	setObjectState(sO_DudeSwinged, 0);
	setObjectState(sO_Girl, getObjectEnumState(sO_Girl, sO_IsSwinging));
	setObjectState(sO_Sugar, getObjectEnumState(sO_Sugar, sO_Present));
	setObjectState(sO_Janitors, getObjectEnumState(sO_Janitors, sO_Together));
	setObjectState(sO_Bag_22, getObjectEnumState(sO_Bag_22, sO_NotFallen));
	setObjectState(sO_Grandpa, getObjectEnumState(sO_Grandpa, sO_InSock));
	setObjectState(sO_CoinSlot_22, getObjectEnumState(sO_CoinSlot_22, sO_Empty));
	setObjectState(sO_UpperHatch_23, getObjectEnumState(sO_UpperHatch_23, sO_Closed));
	setObjectState(sO_LowerHatch_23, getObjectEnumState(sO_LowerHatch_23, sO_Closed));
	setObjectState(sO_Lever_23, getObjectEnumState(sO_Lever_23, sO_NotTaken));
	setObjectState(sO_LeverHandle_23, getObjectEnumState(sO_LeverHandle_23, sO_WithoutStool));
	setObjectState(sO_LowerPipe_21, getObjectEnumState(sO_LowerPipe_21, sO_IsClosed));
	setObjectState(sO_StairsDown_24, getObjectEnumState(sO_StairsDown_24, sO_IsOpened));
	setObjectState(sO_Hatch_26, getObjectEnumState(sO_Hatch_26, sO_Closed));
	setObjectState(sO_Sock_26, getObjectEnumState(sO_Sock_26, sO_NotHanging));
	setObjectState(sO_LeftPipe_26, getObjectEnumState(sO_LeftPipe_26, sO_IsClosed));
	setObjectState(sO_Valve1_26, getObjectEnumState(sO_Valve1_26, sO_Opened));
	setObjectState(sO_Valve2_26, getObjectEnumState(sO_Valve2_26, sO_Closed));
	setObjectState(sO_Valve3_26, getObjectEnumState(sO_Valve3_26, sO_Closed));
	setObjectState(sO_Valve4_26, getObjectEnumState(sO_Valve4_26, sO_Closed));
	setObjectState(sO_Valve5_26, getObjectEnumState(sO_Valve5_26, sO_Opened));
	setObjectState(sO_Pool, getObjectEnumState(sO_Pool, sO_Overfull));
	setObjectState(sO_Board_25, getObjectEnumState(sO_Board_25, sO_NearDudesStairs));
	setObjectState(sO_Driver, getObjectEnumState(sO_Driver, sO_WithSteering));
	setObjectState(sO_Maid, getObjectEnumState(sO_Maid, sO_WithSwab));
	setObjectState(sO_LeftPipe_29, getObjectEnumState(sO_LeftPipe_29, sO_IsClosed));
	setObjectState(sO_LeftPipe_30, getObjectEnumState(sO_LeftPipe_30, sO_IsClosed));
	setObjectState(sO_Leg, getObjectEnumState(sO_Leg, sO_ShowingHeel));
	setObjectState(sO_Tub, getObjectEnumState(sO_Tub, sO_EmptyShe));
	setObjectState(sO_Cactus, getObjectEnumState(sO_Cactus, sO_NotGrown));
	setObjectState(sO_Fireman, getObjectEnumState(sO_Fireman, sO_WithHose));
	setObjectState(sO_Cube, getObjectEnumState(sO_Cube, sO_In_33));
	setObjectState(sO_MommyOfHandle_32, getObjectEnumState(sO_MommyOfHandle_32, sO_WithoutHandle));
	setObjectState(sO_Pedestal_33, getObjectEnumState(sO_Pedestal_33, sO_IsFree));
	setObjectState(sO_Valve_34, getObjectEnumState(sO_Valve_34, sO_WithNothing));
	setObjectState(sO_Stool_34, getObjectEnumState(sO_Stool_34, sO_WithoutDrawer));
	setObjectState(sO_Plank_34, getObjectEnumState(sO_Plank_34, sO_Passive));
	setObjectState(sO_Hatch_34, getObjectEnumState(sO_Hatch_34, sO_Closed));
	setObjectState(sO_Valve_35, getObjectEnumState(sO_Valve_35, sO_TurnedOff));
	setObjectState(sO_Carpet_35, getObjectEnumState(sO_Carpet_35, sO_CannotTake));
	setObjectState(sO_CoinSlot_35, getObjectEnumState(sO_CoinSlot_35, sO_WithCoin));
	setObjectState(sO_BellyInflater, getObjectEnumState(sO_BellyInflater, sO_WithCork));
	setObjectState(sO_Jawcrucnher, getObjectEnumState(sO_Jawcrucnher, sO_WithoutCarpet));
	setObjectState(sO_Guard_1, getObjectEnumState(sO_Guard_1, sO_On));
	setObjectState(sO_Guard_2, getObjectEnumState(sO_Guard_2, sO_On));
	setObjectState(sO_Guard_3, getObjectEnumState(sO_Guard_3, sO_On));
	setObjectState(sO_Bottle_38, getObjectEnumState(sO_Bottle_38, sO_OnTheTable));
	setObjectState(sO_Boss, getObjectEnumState(sO_Boss, sO_WithHammer));
}

void NGIEngine::setLevelStates() {
	GameVar *v = _gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (v) {
		v->setSubVarAsInt(sO_Level0, 2833);
		v->setSubVarAsInt(sO_Level1, 2754);
		v->setSubVarAsInt(sO_Level2, 2757);
		v->setSubVarAsInt(sO_Level3, 2760);
		v->setSubVarAsInt(sO_Level4, 2763);
		v->setSubVarAsInt(sO_Level5, 2766);
		v->setSubVarAsInt(sO_Level6, 2769);
		v->setSubVarAsInt(sO_Level7, 2772);
		v->setSubVarAsInt(sO_Level8, 2775);
		v->setSubVarAsInt(sO_Level9, 2778);
	}
}

void NGIEngine::addCursor(CursorInfo *cursorInfo, Scene *inv, int pictureId, int hotspotX, int hotspotY, int itemPictureOffsX, int itemPictureOffsY) {
	cursorInfo->pictureId = pictureId;
	cursorInfo->picture = inv->getPictureObjectById(pictureId, 0)->_picture;
	cursorInfo->hotspotX = hotspotX;
	cursorInfo->hotspotY = hotspotY;
	cursorInfo->itemPictureOffsX = itemPictureOffsX;
	cursorInfo->itemPictureOffsY = itemPictureOffsY;

	getGameLoaderInputController()->addCursor(cursorInfo);
}

void NGIEngine::initCursors() {
	CursorInfo crs;
	Scene *inv = accessScene(SC_INV);

	addCursor(&crs, inv, PIC_CSR_DEFAULT,     15,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_DEFAULT_INV, 18, 18, 23, 23);
	addCursor(&crs, inv, PIC_CSR_ITN,         11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ITN_RED,     11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ITN_GREEN,   11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ITN_INV,     23, 17, 23, 17);
	addCursor(&crs, inv, PIC_CSR_GOU,         15, 17, 10, 10);
	addCursor(&crs, inv, PIC_CSR_GOD,         15,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_GOL,         26,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_GOR,         15,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_GOFAR_L,      1,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_GOFAR_R,     39,  1, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE1,     12, 24, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE2,     11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE2_D,   22, 15, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE3,     11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE4,     18, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE5,     23, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE6,     11, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE6_D,    0,  0, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE7,     21, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE7_D,    7, 20, 10, 10);
	addCursor(&crs, inv, PIC_CSR_ARCADE8,     23, 11, 10, 10);
	addCursor(&crs, inv, PIC_CSR_LIFT,         6, 13, 10, 10);

	getGameLoaderInputController()->setCursorMode(0);
}

void NGIEngine::initMap() {
	memset(_mapTable, 0, sizeof(_mapTable));

	updateMapPiece(PIC_MAP_S01, 1);
	updateMapPiece(PIC_MAP_A13, 1u);
}

void NGIEngine::loadAllScenes() {
	accessScene(301);
	accessScene(302);
	accessScene(303);
	accessScene(304);
	accessScene(305);
	accessScene(321);
	accessScene(635);
	accessScene(649);
	accessScene(650);
	accessScene(651);
	accessScene(652);
	accessScene(653);
	accessScene(654);
	accessScene(655);
	accessScene(726);
	accessScene(858);
	accessScene(903);
	accessScene(1137);
	accessScene(1138);
	accessScene(1139);
	accessScene(1140);
	accessScene(1141);
	accessScene(1142);
	accessScene(1143);
	accessScene(1144);
	accessScene(1546);
	accessScene(1547);
	accessScene(1548);
	accessScene(1549);
	accessScene(1550);
	accessScene(1551);
	accessScene(1552);
	accessScene(2062);
	accessScene(2063);
	accessScene(2064);
	accessScene(2065);
	accessScene(2066);
	accessScene(2067);
	accessScene(2068);
	accessScene(2069);
	accessScene(2070);
	accessScene(2071);
	accessScene(2072);
	accessScene(2460);
	accessScene(3896);
	accessScene(3907);
	accessScene(4620);
	accessScene(4999);
	accessScene(5000);
	accessScene(5001);
	accessScene(5166);
	accessScene(5222);
}

} // End of namespace NGI
