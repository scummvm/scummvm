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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/gameloader.h"
#include "ngi/objectnames.h"
#include "ngi/input.h"

#include "ngi/constants.h"

namespace NGI {

void NGIEngine::initObjectStates() {
	setLevelStates();

	setState(sO_Dude, getStateValue(sO_Dude, sO_NotCarryingEgg));
	setState(sO_EggCracker, getStateValue(sO_EggCracker, sO_DidNotCrackEgg));
	setState(sO_GuvTheDrawer, getStateValue(sO_GuvTheDrawer, sO_Awaken));
	setState(sO_EggGulper, getStateValue(sO_EggGulper, sO_First));
	setState(sO_EggGulperGaveCoin, getStateValue(sO_EggGulperGaveCoin, sO_No));
	setState(sO_Jar_4, getStateValue(sO_Jar_4, sO_OnTheSpring));
	setState(sO_GulpedEggs, getStateValue(sO_GulpedEgg, sO_NotPresent));

	setSwallowedEggsState();

	setState(sO_WeirdWacko, getStateValue(sO_WeirdWacko, sO_InGlasses));
	setState(sO_TummyTrampie, getStateValue(sO_TummyTrampie, sO_IsDrinking));
	setState(sO_StairsUp_8, getStateValue(sO_StairsUp_8, sO_NotBroken));
	setState(sO_HareTheNooksiter, getStateValue(sO_HareTheNooksiter, sO_WithHandle));
	setState(sO_Elephantine, getStateValue(sO_Elephantine, sO_WithBoot));
	setState(sO_Fly_12, 0);
	setState(sO_ClockAxis, getStateValue(sO_ClockAxis, sO_IsNotAvailable));
	setState(sO_ClockHandle, getStateValue(sO_ClockHandle, sO_In_7));
	setState(sO_BigMumsy, getStateValue(sO_BigMumsy, sO_IsSleeping));
	setState(sO_CoinSlot_1, getStateValue(sO_CoinSlot_1, sO_Empty));
	setState(sO_FriesPit, getStateValue(sO_FriesPit, sO_WithApple));
	setState(sO_Jug, getStateValue(sO_Jug, sO_Blocked));
	setState(sO_RightStairs_9, getStateValue(sO_RightStairs_9, sO_IsClosed));
	if (isDemo() && getLanguage() == Common::RU_RUS)
		setState(sO_Pipe_9, getStateValue(sO_Pipe_9, sO_WithoutJug));
	else
		setState(sO_Pipe_9, getStateValue(sO_Pipe_9, sO_WithJug));
	setState(sO_Inflater, getStateValue(sO_Inflater, sO_WithGum));
	setState(sO_Swingie, getStateValue(sO_Swingie, sO_IsSwinging));
	setState(sO_DudeHasJumped, getStateValue(sO_DudeHasJumped, sO_No));
	setState(sO_Bridge, getStateValue(sO_Bridge, sO_Convoluted));
	setState(sO_Guardian, getStateValue(sO_Guardian, sO_OnRight));
	setState(sO_Grandma, getStateValue(sO_Grandma, sO_In_14));
	setState(sO_Boot_15, getStateValue(sO_Boot_15, sO_NotPresent));
	setState(sO_LeftPipe_15, getStateValue(sO_LeftPipe_15, sO_IsOpened));
	setState(sO_Pedestal_16, getStateValue(sO_Pedestal_16, sO_IsFree));
	setState(sO_Cup, getStateValue(sO_Cup, sO_InSmokeRoom));
	setState(sO_Pedestal_17, getStateValue(sO_Pedestal_17, sO_IsFree));
	setState(sO_UsherHand, getStateValue(sO_UsherHand, sO_WithoutCoin));
	setState(sO_RightPipe_17, getStateValue(sO_RightPipe_17, sO_IsClosed));
	setState(sO_Fly_17, 1);
	setState(sO_DudeSwinged, 0);
	setState(sO_Girl, getStateValue(sO_Girl, sO_IsSwinging));
	setState(sO_Sugar, getStateValue(sO_Sugar, sO_Present));
	setState(sO_Janitors, getStateValue(sO_Janitors, sO_Together));
	setState(sO_Bag_22, getStateValue(sO_Bag_22, sO_NotFallen));
	setState(sO_Grandpa, getStateValue(sO_Grandpa, sO_InSock));
	setState(sO_CoinSlot_22, getStateValue(sO_CoinSlot_22, sO_Empty));
	setState(sO_UpperHatch_23, getStateValue(sO_UpperHatch_23, sO_Closed));
	setState(sO_LowerHatch_23, getStateValue(sO_LowerHatch_23, sO_Closed));
	setState(sO_Lever_23, getStateValue(sO_Lever_23, sO_NotTaken));
	setState(sO_LeverHandle_23, getStateValue(sO_LeverHandle_23, sO_WithoutStool));
	setState(sO_LowerPipe_21, getStateValue(sO_LowerPipe_21, sO_IsClosed));
	setState(sO_StairsDown_24, getStateValue(sO_StairsDown_24, sO_IsOpened));
	setState(sO_Hatch_26, getStateValue(sO_Hatch_26, sO_Closed));
	setState(sO_Sock_26, getStateValue(sO_Sock_26, sO_NotHanging));
	setState(sO_LeftPipe_26, getStateValue(sO_LeftPipe_26, sO_IsClosed));
	setState(sO_Valve1_26, getStateValue(sO_Valve1_26, sO_Opened));
	setState(sO_Valve2_26, getStateValue(sO_Valve2_26, sO_Closed));
	setState(sO_Valve3_26, getStateValue(sO_Valve3_26, sO_Closed));
	setState(sO_Valve4_26, getStateValue(sO_Valve4_26, sO_Closed));
	setState(sO_Valve5_26, getStateValue(sO_Valve5_26, sO_Opened));
	setState(sO_Pool, getStateValue(sO_Pool, sO_Overfull));
	setState(sO_Board_25, getStateValue(sO_Board_25, sO_NearDudesStairs));
	setState(sO_Driver, getStateValue(sO_Driver, sO_WithSteering));
	setState(sO_Maid, getStateValue(sO_Maid, sO_WithSwab));
	setState(sO_LeftPipe_29, getStateValue(sO_LeftPipe_29, sO_IsClosed));
	setState(sO_LeftPipe_30, getStateValue(sO_LeftPipe_30, sO_IsClosed));
	setState(sO_Leg, getStateValue(sO_Leg, sO_ShowingHeel));
	setState(sO_Tub, getStateValue(sO_Tub, sO_EmptyShe));
	setState(sO_Cactus, getStateValue(sO_Cactus, sO_NotGrown));
	setState(sO_Fireman, getStateValue(sO_Fireman, sO_WithHose));
	setState(sO_Cube, getStateValue(sO_Cube, sO_In_33));
	setState(sO_MommyOfHandle_32, getStateValue(sO_MommyOfHandle_32, sO_WithoutHandle));
	setState(sO_Pedestal_33, getStateValue(sO_Pedestal_33, sO_IsFree));
	setState(sO_Valve_34, getStateValue(sO_Valve_34, sO_WithNothing));
	setState(sO_Stool_34, getStateValue(sO_Stool_34, sO_WithoutDrawer));
	setState(sO_Plank_34, getStateValue(sO_Plank_34, sO_Passive));
	setState(sO_Hatch_34, getStateValue(sO_Hatch_34, sO_Closed));
	setState(sO_Valve_35, getStateValue(sO_Valve_35, sO_TurnedOff));
	setState(sO_Carpet_35, getStateValue(sO_Carpet_35, sO_CannotTake));
	setState(sO_CoinSlot_35, getStateValue(sO_CoinSlot_35, sO_WithCoin));
	setState(sO_BellyInflater, getStateValue(sO_BellyInflater, sO_WithCork));
	setState(sO_Jawcrucnher, getStateValue(sO_Jawcrucnher, sO_WithoutCarpet));
	setState(sO_Guard_1, getStateValue(sO_Guard_1, sO_On));
	setState(sO_Guard_2, getStateValue(sO_Guard_2, sO_On));
	setState(sO_Guard_3, getStateValue(sO_Guard_3, sO_On));
	setState(sO_Bottle_38, getStateValue(sO_Bottle_38, sO_OnTheTable));
	setState(sO_Boss, getStateValue(sO_Boss, sO_WithHammer));
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

void NGIEngine::addCursor(Cursor *cursorInfo, Scene *inv, int pictureId, int ofsX, int ofsY, int itemPictureOffsX, int itemPictureOffsY) {
	cursorInfo->pictureId = pictureId;
	cursorInfo->picture = inv->findPictureObjectById(pictureId, 0)->_picture;
	cursorInfo->ofsX = ofsX;
	cursorInfo->ofsY = ofsY;
	cursorInfo->itemPictureOffsX = itemPictureOffsX;
	cursorInfo->itemPictureOffsY = itemPictureOffsY;

	getGameLoaderInputController()->addCursor(cursorInfo);
}

void NGIEngine::initCursors() {
	Cursor crs;
	Scene *inv = findScene(SC_INV);

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
	findScene(301);
	findScene(302);
	findScene(303);
	findScene(304);
	findScene(305);
	findScene(321);
	findScene(635);
	findScene(649);
	findScene(650);
	findScene(651);
	findScene(652);
	findScene(653);
	findScene(654);
	findScene(655);
	findScene(726);
	findScene(858);
	findScene(903);
	findScene(1137);
	findScene(1138);
	findScene(1139);
	findScene(1140);
	findScene(1141);
	findScene(1142);
	findScene(1143);
	findScene(1144);
	findScene(1546);
	findScene(1547);
	findScene(1548);
	findScene(1549);
	findScene(1550);
	findScene(1551);
	findScene(1552);
	findScene(2062);
	findScene(2063);
	findScene(2064);
	findScene(2065);
	findScene(2066);
	findScene(2067);
	findScene(2068);
	findScene(2069);
	findScene(2070);
	findScene(2071);
	findScene(2072);
	findScene(2460);
	findScene(3896);
	findScene(3907);
	findScene(4620);
	findScene(4999);
	findScene(5000);
	findScene(5001);
	findScene(5166);
	findScene(5222);
}

} // End of namespace NGI
