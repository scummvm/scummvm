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

#if !defined(SCUMM_HE_LOGIC_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_LOGIC_HE_H

#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/wiz_he.h"
#include "scumm/he/basketball/basketball.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/obstacle_avoidance.h"

namespace Scumm {

class ScummEngine_v90he;
class ScummEngine_v100he;
class ResourceManager;

class LogicHE {
public:
	static LogicHE *makeLogicHE(ScummEngine_v90he *vm);

	virtual ~LogicHE();

	virtual void beforeBootScript() {}
	virtual void initOnce() {}
	virtual int startOfFrame() { return 1; }
	void endOfFrame() {}
	void processKeyStroke(int keyPressed) {}

	virtual int versionID();
	virtual int32 dispatch(int op, int numArgs, int32 *args);
	virtual bool userCodeProcessWizImageCmd(const WizImageCommand *icmdPtr) { return false; }
	virtual bool overrideImageHitTest(int *outValue, int globNum, int state, int x, int y, int32 flags) { return false; }
	virtual bool overrideImagePixelHitTest(int *outValue, int globNum, int state, int x, int y, int32 flags) { return false; }
	virtual bool getSpriteProperty(int sprite, int property, int *outValue) { return false; }
	virtual bool setSpriteProperty(int sprite, int property, int value) { return false; }
	virtual bool getGroupProperty(int group, int property, int *outValue) { return false; }
	virtual void spriteNewHook(int sprite) {}
	virtual void groupNewHook(int group) {}
	

protected:
	// Only to be used from makeLogicHE()
	LogicHE(ScummEngine_v90he *vm);

	ScummEngine_v90he *_vm;

	void writeScummVar(int var, int32 value);
	int getFromArray(int arg0, int idx2, int idx1);
	void putInArray(int arg0, int idx2, int idx1, int val);
	int32 scummRound(double arg) { return (int32)(arg + 0.5); }

	#define RAD2DEG (180 / M_PI)
	#define DEG2RAD (M_PI / 180)
};

// Logic declarations
LogicHE *makeLogicHErace(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfunshop(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfootball(ScummEngine_v90he *vm);
LogicHE *makeLogicHEfootball2002(ScummEngine_v90he *vm);
LogicHE *makeLogicHEsoccer(ScummEngine_v90he *vm);
LogicHE *makeLogicHEbaseball2001(ScummEngine_v90he *vm);
LogicHE *makeLogicHEbasketball(ScummEngine_v100he *vm);
LogicHE *makeLogicHEmoonbase(ScummEngine_v100he *vm);


// Declarations for Basketball logic live here
// instead of in basketball_logic.cpp since
// they are defined in multiple files...
class LogicHEBasketball : public LogicHE {
public:
	LogicHEBasketball(ScummEngine_v100he *vm) : LogicHE(vm) { _vm1 = vm; }

	int versionID() override;
	int32 dispatch(int op, int numArgs, int32 *args) override;

	float _courtAngle = 0.0F;
	float _yTranslationA = 0.0F;
	float _yTranslationB = 0.0F;
	float _yTranslationC = 0.0F;
	float _yRevTranslationA = 0.0F;
	float _yRevTranslationB = 0.0F;
	float _yRevTranslationC = 0.0F;
	int _topScalingPointCutoff = 0;
	int _bottomScalingPointCutoff = 0;

protected:
	int u32_userInitCourt(int courtID);
	int u32_userInitBall(U32FltPoint3D &ballLocation, U32FltVector3D &bellVelocity, int radius, int ballID);
	int u32_userInitVirtualBall(U32FltPoint3D &ballLocation, U32FltVector3D &bellVelocity, int radius, int ballID);
	int u32_userInitPlayer(int playerID, U32FltPoint3D &playerLocation, int height, int radius, bool bPlayerIsInGame);
	int u32_userPlayerOff(int playerID);
	int u32_userPlayerOn(int playerID);
	int u32_userDetectBallCollision(U32FltPoint3D &ballLocation, U32FltVector3D &ballVector, int recordCollision, int ballID);
	int u32_userDetectPlayerCollision(int playerID, U32FltPoint3D &playerLocation, U32FltVector3D &playerVector, bool bPlayerHasBall);
	int u32_userGetLastBallCollision(int ballID);
	int u32_userGetLastPlayerCollision(int playerID);
	int u32_userDeinitCourt();
	int u32_userDeinitBall();
	int u32_userDeinitVirtualBall();
	int u32_userDeinitPlayer(int playerID);

	int u32_userInitScreenTranslations(void);
	int u32_userWorldToScreenTranslation(const U32FltPoint3D &worldPoint);
	int u32_userScreenToWorldTranslation(const U32FltPoint2D &screenPoint);
	int u32_userGetCourtDimensions();
	int u32_userComputePointsForPixels(int pixels, int yPos);

	int u32_userComputeTrajectoryToTarget(const U32FltPoint3D &sourcePoint, const U32FltPoint3D &targetPoint, int speed);
	int u32_userComputeLaunchTrajectory(const U32FltPoint2D &sourcePoint, const U32FltPoint2D &targetPoint, int launchAngle, int iVelocity);
	int u32_userComputeAngleBetweenVectors(const U32FltVector3D &vector1, const U32FltVector3D &vector2);

	// Court shields
	int u32_userRaiseShields(int shieldID);
	int u32_userLowerShields(int shieldID);
	int u32_userAreShieldsClear();

	// Player shields
	int u32_userShieldPlayer(int playerID, int shieldRadius);
	int u32_userClearPlayerShield(int playerID);

	int u32_userComputeAngleOfShot(int hDist, int vDist);
	int u32_userComputeBankShotTarget(U32FltPoint3D basketLoc, int ballRadius);
	int u32_userComputeSwooshTarget(const U32FltPoint3D &basketLoc, int ballRadius);
	int u32_userComputeInitialShotVelocity(int theta, int hDist, int vDist, int gravity);
	int u32_userDetectShotMade(const U32Sphere &basketball, const U32IntVector3D &ballVector, int gravity, int whichBasket);
	int u32_userComputeAngleOfPass(int iVelocity, int hDist, int vDist, int gravity);
	int u32_userComputeAngleOfBouncePass(int iVelocity, int hDist, int currentZ, int destZ, int gravity);
	int u32_userHitMovingTarget(U32FltPoint2D sourcePlayer, U32FltPoint2D targetPlayer, U32FltVector2D targetVelocity, int passSpeed);
	int u32_userGetPassTarget(int playerID, const U32FltVector3D &aimVector);
	int u32_userDetectPassBlocker(int playerID, const U32FltPoint3D &targetPoint);
	int u32_userGetBallIntercept(int playerID, int ballID, int playerSpeed, int gravity);
	int u32_userGetAvoidancePath(int playerID, const U32FltPoint2D &targetLocation, EAvoidanceType type);

	int u32_userGetPlayerClosestToBall(int teamIndex);
	int u32_userGetPlayerClosestToBall();
	int u32_userIsPlayerInBounds(int playerID);
	int u32_userIsBallInBounds();
	int u32_userGetOpenSpot(int whichPlayer, U32FltVector2D upperLeft, U32FltVector2D lowerRight, U32FltVector2D from, bool attract, U32FltVector2D attract_point);

	int u32_userUpdateCursorPos(int xScrollVal, int yScrollVal);
	int u32_userMakeCursorSticky(int lastCursorX, int lastCursorY);
	int u32_userCursorTrackMovingObject(int xChange, int yChange);
	int u32_userGetCursorPos();

private:
	ScummEngine_v100he *_vm1;
};

} // End of namespace Scumm

#endif
