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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#include "scumm/he/basketball/basketball.h"
#include "scumm/he/basketball/collision/bball_collision.h"
#include "scumm/he/basketball/geo_translations.h"

namespace Scumm {

// Opcodes
#define WORLD_TO_SCREEN_TRANSLATION           1006
#define WORLD_TO_SCREEN_TRANSLATION_PARAMS       3
#define SCREEN_TO_WORLD_TRANSLATION           1010
#define SCREEN_TO_WORLD_TRANSLATION_PARAMS       2
#define INIT_SCREEN_TRANSLATIONS              1011
#define INIT_SCREEN_TRANSLATIONS_PARAMS          0
#define GET_COURT_DIMENSIONS                  1012
#define GET_COURT_DIMENSIONS_PARAMS              0

#define COMPUTE_INITIAL_SHOT_VELOCITY         1030
#define COMPUTE_INITIAL_SHOT_VELOCITY_PARAMS     4
#define COMPUTE_TRAJECTORY_TO_TARGET          1031
#define COMPUTE_TRAJECTORY_TO_TARGET_PARAMS      7
#define COMPUTE_LAUNCH_TRAJECTORY             1032
#define COMPUTE_LAUNCH_TRAJECTORY_PARAMS         6
#define COMPUTE_ANGLE_OF_SHOT                 1033
#define COMPUTE_ANGLE_OF_SHOT_PARAMS             2
#define COMPUTE_ANGLE_OF_PASS                 1034
#define COMPUTE_ANGLE_OF_PASS_PARAMS             4
#define COMPUTE_POINTS_FOR_PIXELS             1035
#define COMPUTE_POINTS_FOR_PIXELS_PARAMS         2
#define COMPUTE_ANGLE_OF_BOUNCE_PASS          1036
#define COMPUTE_ANGLE_OF_BOUNCE_PASS_PARAMS      5
#define COMPUTE_BANK_SHOT_TARGET              1037
#define COMPUTE_BANK_SHOT_TARGET_PARAMS          4
#define COMPUTE_SWOOSH_TARGET                 1038
#define COMPUTE_SWOOSH_TARGET_PARAMS             4
#define DETECT_SHOT_MADE                      1039
#define DETECT_SHOT_MADE_PARAMS                  9
#define COMPUTE_ANGLE_BETWEEN_VECTORS         1040
#define COMPUTE_ANGLE_BETWEEN_VECTORS_PARAMS     6
#define HIT_MOVING_TARGET                     1041
#define HIT_MOVING_TARGET_PARAMS                 7
#define GET_PASS_TARGET                       1042
#define GET_PASS_TARGET_PARAMS                   3
#define DETECT_PASS_BLOCKER                   1043
#define DETECT_PASS_BLOCKER_PARAMS               3
#define GET_BALL_INTERCEPT                    1044
#define GET_BALL_INTERCEPT_PARAMS                4

#define INIT_COURT                            1050
#define INIT_COURT_PARAMS                        1
#define INIT_BALL                             1051
#define INIT_BALL_PARAMS                         8
#define INIT_PLAYER                           1052
#define INIT_PLAYER_PARAMS                       7
#define DEINIT_COURT                          1053
#define DEINIT_COURT_PARAMS                      0
#define DEINIT_BALL                           1054
#define DEINIT_BALL_PARAMS                       0
#define DEINIT_PLAYER                         1055
#define DEINIT_PLAYER_PARAMS                     1
#define DETECT_BALL_COLLISION                 1056
#define DETECT_BALL_COLLISION_PARAMS             8
#define DETECT_PLAYER_COLLISION               1057
#define DETECT_PLAYER_COLLISION_PARAMS           8
#define GET_LAST_BALL_COLLISION               1058
#define GET_LAST_BALL_COLLISION_PARAMS           1
#define GET_LAST_PLAYER_COLLISION             1059
#define GET_LAST_PLAYER_COLLISION_PARAMS         1
#define INIT_VIRTUAL_BALL                     1060
#define INIT_VIRTUAL_BALL_PARAMS                 8
#define DEINIT_VIRTUAL_BALL                   1061
#define DEINIT_VIRTUAL_BALL_PARAMS               0
#define PLAYER_OFF                            1062
#define PLAYER_OFF_PARAMS                        1
#define PLAYER_ON                             1063
#define PLAYER_ON_PARAMS                         1
#define RAISE_SHIELDS                         1064
#define RAISE_SHIELDS_PARAMS                     1
#define LOWER_SHIELDS                         1065
#define LOWER_SHIELDS_PARAMS                     1
#define FIND_PLAYER_CLOSEST_TO_BALL           1066
#define FIND_PLAYER_CLOSEST_TO_BALL_PARAMS       1
#define IS_PLAYER_IN_BOUNDS                   1067
#define IS_PLAYER_IN_BOUNDS_PARAMS               1
#define ARE_SHIELDS_CLEAR                     1068
#define ARE_SHIELDS_CLEAR_PARAMS                 0
#define SHIELD_PLAYER                         1069
#define SHIELD_PLAYER_PARAMS                     2
#define CLEAR_PLAYER_SHIELD                   1070
#define CLEAR_PLAYER_SHIELD_PARAMS               1
#define IS_BALL_IN_BOUNDS                     1071
#define IS_BALL_IN_BOUNDS_PARAMS                 0 
#define GET_AVOIDANCE_PATH                    1072
#define GET_AVOIDANCE_PATH_PARAMS                4
#define SET_BALL_LOCATION                     1073
#define SET_BALL_LOCATION_PARAMS                 4
#define GET_BALL_LOCATION                     1074
#define GET_BALL_LOCATION_PARAMS                 1
#define SET_PLAYER_LOCATION                   1075
#define SET_PLAYER_LOCATION_PARAMS               4
#define GET_PLAYER_LOCATION                   1076
#define GET_PLAYER_LOCATION_PARAMS               1
#define START_BLOCK                           1077
#define START_BLOCK_PARAMS                       3
#define HOLD_BLOCK                            1078
#define HOLD_BLOCK_PARAMS                        1
#define END_BLOCK                             1079
#define END_BLOCK_PARAMS                         1
#define IS_PLAYER_IN_GAME                     1080
#define IS_PLAYER_IN_GAME_PARAMS                 1
#define IS_BALL_IN_GAME                       1081
#define IS_BALL_IN_GAME_PARAMS                   1

#define UPDATE_CURSOR_POS                     1090
#define UPDATE_CURSOR_POS_PARAMS                 2
#define MAKE_CURSOR_STICKY                    1091
#define MAKE_CURSOR_STICKY_PARAMS                2
#define CURSOR_TRACK_MOVING_OBJECT            1092
#define CURSOR_TRACK_MOVING_OBJECT_PARAMS        2
#define GET_CURSOR_POSITION                   1093
#define GET_CURSOR_POSITION_PARAMS               0

#define AI_GET_OPEN_SPOT                      1100
#define AI_GET_OPEN_SPOT_PARAMS                 10
#define AI_GET_OPPONENTS_IN_CONE              1101
#define AI_GET_OPPONENTS_IN_CONE_PARAMS          6

#define U32_CLEAN_UP_OFF_HEAP	              1102
#define U32_CLEAN_UP_OFF_HEAP_PARAMS          0

#define DRAW_DEBUG_LINES                      1500
#define DRAW_DEBUG_LINES_PARAMS                  0

#define ADD_DEBUG_GEOM                        1501

int LogicHEBasketball::versionID() {
	return 1;
}

int32 LogicHEBasketball::dispatch(int cmdID, int paramCount, int32 *params) {
	U32FltPoint3D flt3DPoint1, flt3DPoint2;
	U32FltPoint2D flt2DPoint1, flt2DPoint2;
	U32IntVector3D int3DVector1;
	U32FltVector3D flt3DVector1, flt3DVector2;
	U32FltVector2D flt2DVector1;
	U32Sphere sphere1;

	int retValue = 0;

	switch (cmdID) {

	case INIT_SCREEN_TRANSLATIONS:
		assert(paramCount == INIT_SCREEN_TRANSLATIONS_PARAMS);

		retValue = u32_userInitScreenTranslations();
		break;

	case WORLD_TO_SCREEN_TRANSLATION:
		assert(paramCount == WORLD_TO_SCREEN_TRANSLATION_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		retValue = u32_userWorldToScreenTranslation(flt3DPoint1);
		break;

	case SCREEN_TO_WORLD_TRANSLATION:
		assert(paramCount == SCREEN_TO_WORLD_TRANSLATION_PARAMS);

		flt2DPoint1.x = (float)params[0];
		flt2DPoint1.y = (float)params[1];

		retValue = u32_userScreenToWorldTranslation(flt2DPoint1);
		break;

	case GET_COURT_DIMENSIONS:
		assert(paramCount == GET_COURT_DIMENSIONS_PARAMS);

		retValue = u32_userGetCourtDimensions();
		break;

	case COMPUTE_TRAJECTORY_TO_TARGET:
		assert(paramCount == COMPUTE_TRAJECTORY_TO_TARGET_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		flt3DPoint2.x = (float)params[3];
		flt3DPoint2.y = (float)params[4];
		flt3DPoint2.z = (float)params[5];

		retValue = u32_userComputeTrajectoryToTarget(flt3DPoint1, flt3DPoint2, params[6]);
		break;

	case COMPUTE_LAUNCH_TRAJECTORY:
		assert(paramCount == COMPUTE_LAUNCH_TRAJECTORY_PARAMS);

		flt2DPoint1.x = (float)params[0];
		flt2DPoint1.y = (float)params[1];

		flt2DPoint2.x = (float)params[2];
		flt2DPoint2.y = (float)params[3];

		retValue = u32_userComputeLaunchTrajectory(flt2DPoint1, flt2DPoint2, params[4], params[5]);
		break;

	case COMPUTE_ANGLE_BETWEEN_VECTORS:
		assert(paramCount == COMPUTE_ANGLE_BETWEEN_VECTORS_PARAMS);

		flt3DVector1.x = (float)params[0];
		flt3DVector1.y = (float)params[1];
		flt3DVector1.z = (float)params[2];

		flt3DVector2.x = (float)params[3];
		flt3DVector2.y = (float)params[4];
		flt3DVector2.z = (float)params[5];

		retValue = u32_userComputeAngleBetweenVectors(flt3DVector1, flt3DVector2);
		break;

	case COMPUTE_INITIAL_SHOT_VELOCITY:
		assert(paramCount == COMPUTE_INITIAL_SHOT_VELOCITY_PARAMS);
		retValue = u32_userComputeInitialShotVelocity(params[0], params[1], params[2], params[3]);
		break;

	case COMPUTE_ANGLE_OF_SHOT:
		assert(paramCount == COMPUTE_ANGLE_OF_SHOT_PARAMS);
		retValue = u32_userComputeAngleOfShot(params[0], params[1]);
		break;

	case COMPUTE_ANGLE_OF_PASS:
		assert(paramCount == COMPUTE_ANGLE_OF_PASS_PARAMS);
		retValue = u32_userComputeAngleOfPass(params[0], params[1], params[2], params[3]);
		break;

	case COMPUTE_ANGLE_OF_BOUNCE_PASS:
		assert(paramCount == COMPUTE_ANGLE_OF_BOUNCE_PASS_PARAMS);
		retValue = u32_userComputeAngleOfBouncePass(params[0], params[1], params[2], params[3], params[4]);
		break;

	case HIT_MOVING_TARGET:
		assert(paramCount == HIT_MOVING_TARGET_PARAMS);

		flt2DPoint1.x = (float)params[0];
		flt2DPoint1.y = (float)params[1];
		flt2DPoint2.x = (float)params[2];
		flt2DPoint2.y = (float)params[3];
		flt2DVector1.x = (float)params[4];
		flt2DVector1.y = (float)params[5];

		retValue = u32_userHitMovingTarget(flt2DPoint1, flt2DPoint2, flt2DVector1, params[6]);
		break;

	case GET_PASS_TARGET:
		assert(paramCount == GET_PASS_TARGET_PARAMS);

		flt3DVector1.x = (float)params[1];
		flt3DVector1.y = (float)params[2];

		retValue = u32_userGetPassTarget(params[0], flt3DVector1);
		break;

	case DETECT_PASS_BLOCKER:
		assert(paramCount == DETECT_PASS_BLOCKER_PARAMS);

		flt3DVector1.x = (float)params[1];
		flt3DVector1.y = (float)params[2];

		retValue = u32_userDetectPassBlocker(params[0], flt3DVector1);
		break;

	case GET_BALL_INTERCEPT:
		assert(paramCount == GET_BALL_INTERCEPT_PARAMS);

		retValue = u32_userGetBallIntercept(params[0], params[1], params[2], params[3]);
		break;

	case COMPUTE_POINTS_FOR_PIXELS:
		assert(paramCount == COMPUTE_POINTS_FOR_PIXELS_PARAMS);
		retValue = u32_userComputePointsForPixels(params[0], params[1]);
		break;

	case COMPUTE_BANK_SHOT_TARGET:
		assert(paramCount == COMPUTE_BANK_SHOT_TARGET_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		retValue = u32_userComputeBankShotTarget(flt3DPoint1, params[3]);
		break;

	case COMPUTE_SWOOSH_TARGET:
		assert(paramCount == COMPUTE_SWOOSH_TARGET_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		retValue = u32_userComputeSwooshTarget(flt3DPoint1, params[3]);
		break;

	case INIT_COURT:
		assert(paramCount == INIT_COURT_PARAMS);
		retValue = u32_userInitCourt(params[0]);
		break;

	case DEINIT_COURT:
		assert(paramCount == DEINIT_COURT_PARAMS);
		retValue = u32_userDeinitCourt();
		break;

	case INIT_BALL:
		assert(paramCount == INIT_BALL_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		flt3DVector1.x = (float)params[3];
		flt3DVector1.y = (float)params[4];
		flt3DVector1.z = (float)params[5];


		retValue = u32_userInitBall(flt3DPoint1, flt3DVector1, params[6], params[7]);
		break;

	case DEINIT_BALL:
		assert(paramCount == DEINIT_BALL_PARAMS);

		retValue = u32_userDeinitBall();
		break;

	case INIT_VIRTUAL_BALL:
		assert(paramCount == INIT_VIRTUAL_BALL_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		flt3DVector1.x = (float)params[3];
		flt3DVector1.y = (float)params[4];
		flt3DVector1.z = (float)params[5];

		retValue = u32_userInitVirtualBall(flt3DPoint1, flt3DVector1, params[6], params[7]);
		break;

	case DEINIT_VIRTUAL_BALL:
		assert(paramCount == DEINIT_VIRTUAL_BALL_PARAMS);

		retValue = u32_userDeinitVirtualBall();
		break;

	case INIT_PLAYER:
		assert(paramCount == INIT_PLAYER_PARAMS);

		flt3DPoint1.x = (float)params[1];
		flt3DPoint1.y = (float)params[2];
		flt3DPoint1.z = (float)params[3];

		retValue = u32_userInitPlayer(params[0], flt3DPoint1, params[4], params[5], (params[6] != 0));
		break;

	case DEINIT_PLAYER:
		assert(paramCount == DEINIT_PLAYER_PARAMS);
		retValue = u32_userDeinitPlayer(params[0]);
		break;

	case PLAYER_OFF:
		assert(paramCount == PLAYER_OFF_PARAMS);
		retValue = u32_userPlayerOff(params[0]);
		break;

	case PLAYER_ON:
		assert(paramCount == PLAYER_ON_PARAMS);
		retValue = u32_userPlayerOn(params[0]);
		break;

	case SET_BALL_LOCATION:
		assert(paramCount == SET_BALL_LOCATION_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];


		_vm->_basketball->_court->getBallPtr(params[3])->center = flt3DPoint1;

		retValue = 1;
		break;

	case GET_BALL_LOCATION:
		assert(paramCount == GET_BALL_LOCATION_PARAMS);

		flt3DPoint1 = _vm->_basketball->_court->getBallPtr(params[0])->center;

		writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(flt3DPoint1.x));
		writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(flt3DPoint1.y));
		writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(flt3DPoint1.z));

		retValue = 1;
		break;

	case SET_PLAYER_LOCATION:
		assert(paramCount == SET_PLAYER_LOCATION_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		flt3DPoint1.z += (_vm->_basketball->_court->getPlayerPtr(params[3])->height / 2);
		_vm->_basketball->_court->getPlayerPtr(params[3])->center = flt3DPoint1;

		retValue = 1;
		break;

	case GET_PLAYER_LOCATION:
		assert(paramCount == GET_PLAYER_LOCATION_PARAMS);

		flt3DPoint1 = _vm->_basketball->_court->getPlayerPtr(params[0])->center;
		flt3DPoint1.z -= (_vm->_basketball->_court->getPlayerPtr(params[0])->height / 2);

		writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(flt3DPoint1.x));
		writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(flt3DPoint1.y));
		writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(flt3DPoint1.z));

		retValue = 1;
		break;

	case DETECT_BALL_COLLISION:
		assert(paramCount == DETECT_BALL_COLLISION_PARAMS);

		flt3DPoint1.x = (float)params[0];
		flt3DPoint1.y = (float)params[1];
		flt3DPoint1.z = (float)params[2];

		flt3DVector1.x = (float)params[3];
		flt3DVector1.y = (float)params[4];
		flt3DVector1.z = (float)params[5];

		retValue = u32_userDetectBallCollision(flt3DPoint1, flt3DVector1, params[6], params[7]);
		break;

	case DETECT_PLAYER_COLLISION:
		assert(paramCount == DETECT_PLAYER_COLLISION_PARAMS);

		flt3DPoint1.x = (float)params[1];
		flt3DPoint1.y = (float)params[2];
		flt3DPoint1.z = (float)params[3];

		flt3DVector1.x = (float)params[4];
		flt3DVector1.y = (float)params[5];
		flt3DVector1.z = (float)params[6];

		retValue = u32_userDetectPlayerCollision(params[0], flt3DPoint1, flt3DVector1, (params[7] != 0));
		break;

	case GET_LAST_BALL_COLLISION:
		assert(paramCount == GET_LAST_BALL_COLLISION_PARAMS);

		retValue = u32_userGetLastBallCollision(params[0]);
		break;

	case GET_LAST_PLAYER_COLLISION:
		assert(paramCount == GET_LAST_PLAYER_COLLISION_PARAMS);

		retValue = u32_userGetLastPlayerCollision(params[0]);
		break;

	case DETECT_SHOT_MADE:
		assert(paramCount == DETECT_SHOT_MADE_PARAMS);

		sphere1.center.x = (float)params[0];
		sphere1.center.y = (float)params[1];
		sphere1.center.z = (float)params[2];
		sphere1.radius = (float)params[6];

		int3DVector1.x = params[3];
		int3DVector1.y = params[4];
		int3DVector1.z = params[5];

		retValue = u32_userDetectShotMade(sphere1, int3DVector1, params[7], params[8]);
		break;

	case RAISE_SHIELDS:
		assert(paramCount == RAISE_SHIELDS_PARAMS);

		retValue = u32_userRaiseShields(params[0]);
		break;

	case LOWER_SHIELDS:
		assert(paramCount == LOWER_SHIELDS_PARAMS);

		retValue = u32_userLowerShields(params[0]);
		break;

	case FIND_PLAYER_CLOSEST_TO_BALL:
		assert((paramCount == 0) || (paramCount == FIND_PLAYER_CLOSEST_TO_BALL_PARAMS));

		if (paramCount == 0) {
			retValue = u32_userGetPlayerClosestToBall();
		} else {
			retValue = u32_userGetPlayerClosestToBall(params[0]);
		}

		break;

	case IS_PLAYER_IN_BOUNDS:
		assert(paramCount == IS_PLAYER_IN_BOUNDS_PARAMS);

		retValue = u32_userIsPlayerInBounds(params[0]);
		break;

	case IS_BALL_IN_BOUNDS:
		assert(paramCount == IS_BALL_IN_BOUNDS_PARAMS);

		retValue = u32_userIsBallInBounds();
		break;

	case ARE_SHIELDS_CLEAR:
		assert(paramCount == ARE_SHIELDS_CLEAR_PARAMS);

		retValue = u32_userAreShieldsClear();
		break;

	case SHIELD_PLAYER:
		assert(paramCount == SHIELD_PLAYER_PARAMS);

		retValue = u32_userShieldPlayer(params[0], params[1]);
		break;

	case CLEAR_PLAYER_SHIELD:
		assert(paramCount == CLEAR_PLAYER_SHIELD_PARAMS);

		retValue = u32_userClearPlayerShield(params[0]);
		break;

	case GET_AVOIDANCE_PATH:
		assert(paramCount == GET_AVOIDANCE_PATH_PARAMS);

		flt2DPoint1.x = (float)params[1];
		flt2DPoint1.y = (float)params[2];

		retValue = u32_userGetAvoidancePath(params[0], flt2DPoint1, (EAvoidanceType)params[3]);
		break;

	case START_BLOCK:
		assert(paramCount == START_BLOCK_PARAMS);
		assert((FIRST_PLAYER <= params[0]) && (params[0] <= LAST_PLAYER));

		(_vm->_basketball->_court->getPlayerPtr(params[0]))->startBlocking(params[1], params[2]);

		retValue = 1;
		break;

	case HOLD_BLOCK:
		assert(paramCount == HOLD_BLOCK_PARAMS);
		assert((FIRST_PLAYER <= params[0]) && (params[0] <= LAST_PLAYER));

		(_vm->_basketball->_court->getPlayerPtr(params[0]))->holdBlocking();

		retValue = 1;
		break;

	case END_BLOCK:
		assert(paramCount == END_BLOCK_PARAMS);
		assert((FIRST_PLAYER <= params[0]) && (params[0] <= LAST_PLAYER));

		(_vm->_basketball->_court->getPlayerPtr(params[0]))->endBlocking();

		retValue = 1;
		break;

	case IS_PLAYER_IN_GAME:
		assert(paramCount == IS_PLAYER_IN_GAME_PARAMS);
		assert((FIRST_PLAYER <= params[0]) && (params[0] <= LAST_PLAYER));

		writeScummVar(_vm1->VAR_U32_USER_VAR_A, ((_vm->_basketball->_court->getPlayerPtr(params[0]))->_playerIsInGame) ? 1 : 0);

		retValue = 1;
		break;

	case IS_BALL_IN_GAME:
		assert(paramCount == IS_BALL_IN_GAME_PARAMS);

		writeScummVar(_vm1->VAR_U32_USER_VAR_A, ((_vm->_basketball->_court->getBallPtr(params[0]))->_ignore) ? 0 : 1);

		retValue = 1;
		break;

	case UPDATE_CURSOR_POS:
		assert(paramCount == UPDATE_CURSOR_POS_PARAMS);

		retValue = u32_userUpdateCursorPos(params[0], params[1]);
		break;

	case MAKE_CURSOR_STICKY:
		assert(paramCount == MAKE_CURSOR_STICKY_PARAMS);

		retValue = u32_userMakeCursorSticky(params[0], params[1]);
		break;

	case CURSOR_TRACK_MOVING_OBJECT:
		assert(paramCount == CURSOR_TRACK_MOVING_OBJECT_PARAMS);

		retValue = u32_userCursorTrackMovingObject(params[0], params[1]);
		break;

	case GET_CURSOR_POSITION:
		assert(paramCount == GET_CURSOR_POSITION_PARAMS);

		retValue = u32_userGetCursorPos();
		break;

	case AI_GET_OPEN_SPOT:
		assert(paramCount == AI_GET_OPEN_SPOT_PARAMS);
		retValue = u32_userGetOpenSpot(
			params[0],
			U32FltVector2D((float)params[1], (float)params[2]),
			U32FltVector2D((float)params[3], (float)params[4]),
			U32FltVector2D((float)params[5], (float)params[6]),
			(params[7] != 0),
			U32FltVector2D((float)params[8], (float)params[9])); 
		break;

	case AI_GET_OPPONENTS_IN_CONE:
		assert(paramCount == AI_GET_OPPONENTS_IN_CONE_PARAMS);
		retValue = _vm->_basketball->numOpponentsInCone(
			params[0],
			(((float)params[1]) / 65536),
			U32FltVector2D((float)params[2], (float)params[3]),
			U32FltVector2D((float)params[4], (float)params[5]));
		break;

	case U32_CLEAN_UP_OFF_HEAP:
		// No-op
		retValue = 1;
		break;

	default:
		break;
	}

	return retValue;
}

LogicHE *makeLogicHEbasketball(ScummEngine_v100he *vm) {
	return new LogicHEBasketball(vm);
}

} // End of namespace Scumm
