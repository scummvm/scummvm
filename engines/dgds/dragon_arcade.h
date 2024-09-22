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

#ifndef DGDS_DRAGON_ARCADE_H
#define DGDS_DRAGON_ARCADE_H

#include "common/types.h"
#include "engines/dgds/dragon_arcade_ttm.h"

namespace Dgds {

enum DragonBulletState {
	kBulletInactive = 0,
	kBulletFlying = 1,
	kBulletHittingBlade = 2,
	kBulletHittingEnemy = 3,
};

enum DragonBladeMoveFlag {
	kBladeMoveNone = 0,
	kBladeMoveUp = 1,
	kBladeMoveDown = 2,
	kBladeMoveRight = 4,
	kBladeMoveLeft = 8,
};

class ArcadeNPCState {
public:
	ArcadeNPCState() : xx(0), yy(0), x(0), y(0), x_11(0), y_11(0), x_12(0), y_12(0),
		ttmPage(0), byte12(0), byte13(0), health(0), ttmNum(0), x_21(0), y_21(0),
		x_22(0), y_22(0) {}
	int16 xx;
	int16 yy;
	int16 x;
	int16 y;
	int16 x_11;
	int16 y_11;
	int16 x_12;
	int16 y_12;
	int16 ttmPage;
	int8 byte12;
	int8 byte13;
	int8 health;
	int8 ttmNum; /* Set to 0, 1 or 2 */
	int16 x_21;
	int16 y_21;
	int16 x_22;
	int16 y_22;
};

class DragonArcadeBullet {
public:
	DragonArcadeBullet() : _x(0), _y(0), _state(kBulletInactive),
		_flipMode(kImageFlipNone), _bulletType(0), _ySpeed(0) {}

	int16 _x;
	int16 _y;
	DragonBulletState _state;
	ImageFlipMode _flipMode;
	int16 _bulletType;
	uint16 _ySpeed;
	// these fields are in the original but seem to not be used.
	// int16 _var2;
	// int16 _var3;
};

class DragonArcade {
public:
	DragonArcade();

	void arcadeTick();
	void onKeyDown(Common::KeyState kbd);
	void onKeyUp(Common::KeyState kbd);

private:
	void initIfNeeded();
	void drawBackgroundAndWeapons();
	void checkToOpenMenu();
	void clearAllBulletStates();
	void clearAllNPCStates();
	void createBullet(int16 x, int16 y, ImageFlipMode flipMode, int16 bulletType);
	void bladeTakeHitAndCheck();
	void enemyTakeHit() { _npcState[1].health--; }
	void enemyTakeHitAndCheck();
	void playSfx(int16 num) const;
	void drawBulletHitCircles(uint16 x, uint16 y, bool colorFlag);
	void drawHealthBars();
	void runThenDrawBulletsInFlight();
	void redraw();
	void finish();
	void loadTTMScriptsForStage(uint16 stage);
	void fadeInAndClearScreen();
	bool doTickUpdate();
	void resetStageState();
	void initValuesForStage();
	void initValuesForStage0();
	void initValuesForStage2();
	void initValuesForStage3();
	void initValuesForStage4();
	void initValuesForStage6();
	void setFinishCountdownIfLessThan0(int16 val);
	void updateBladeWithInputs();
	void updateBlade();
	void updateBoss();
	void updateBoss2();
	void decBossHealth();
	void decBossHealthAndCheck();
	void bladeTakeHit();
	void arcade16bc();
	void arcade16de(int16 param);
	void arcade1e83();
	void arcade2445();
	void arcade2754(int16 findResult);
	void arcade34b4();
	void arcade3e96();
	void arcade4085();
	void updateXScrollOffset();
	bool isNpcInsideXRange(int16 num);
	void updateBullets();
	void checkBladeFireAllStages();
	void checkEnemyFireStage0124();
	void checkBossFireStage3();
	void checkBossFireStage6();
	void updateMouseAndJoystickStates();
	int16 findFloorUnderBlade();
	int16 checkBulletCollision(int16 num);
	void mouseUpdate();
	void keyboardUpdate();
	void limitToCenterOfScreenAndUpdateCursor();
	uint16 moveToNextStage();
	void findFloorMatch();
	void findFloorMinGT();
	void findFloorMinGE();
	void findFloorMatchOrMinOrMax();
	void findFloorMax();
	void updateFloorsUnderBlade();
	bool isFloorNotFound();
	void playSFX55AndStuff();
	void moveBladeX();
	void handleMouseStates();
	void drawScrollBmp();

	int16 _lastDrawnBladeHealth;
	int16 _lastDrawnBossHealth;
	uint16 _nextRandomVal;
	int16 _loadedArcadeStage;
	int16 _nextStage;
	int16 _attemptCounter;
	int16 _shouldUpdateState;
	int16 _finishCountdown;
	int16 _bladeState1;
	int16 _bladePageOffset;
	uint16 _mouseButtonWentDown;
	int16 _scrollXOffset;
	int32 _nTickUpdates;
	int16 _startDifficultyMaybe;
	int16 _bossStateUpdateCounter;
	int16 _npcStateResetCounter;
	int16 _scrollVelocityX;
	uint16 _uint0a17;
	int16 _currentYOffset;
	int16 _int0b58;
	int16 _int0b5a;
	int16 _int0b60;
	int16 _ttmYAdjust;
	uint16 _uint0be6;
	bool _dontMoveBladeFlag;
	int16 _scrollXIncrement;
	bool _lMouseButtonState;
	bool _rMouseButtonState;
	bool _lastLMouseButtonState;
	bool _lastRMouseButtonState;
	int16 _bladeXMove;
	int16 _currentArrowNum;
	int16 _foundFloorY;
	bool _foundFloorFlag;
	int16 _lastFloorY;

	bool _haveBigGun;
	bool _haveBomb;
	bool _enemyHasSmallGun;
	bool _dontRedrawBgndAndWeapons;
	// maybe don't need these
	//bool _arcadeNeedsBufferCopy;
	//bool _flagInventoryOpened;
	bool _initFinished;
	bool _stillLoadingScriptsMaybe;
	bool _flag40ee;
	bool _flag40ef;
	int16 _someMoveDirection;
	bool _bladeHasFired;
	bool _mouseIsAvailable;
	bool _isMovingStage;
	DragonBladeMoveFlag _bladeMoveFlag;
	DragonBladeMoveFlag _keyStateFlags;
	DragonBladeMoveFlag _bladeMoveFlagBeforeRButton;
	DragonBladeMoveFlag _bladeHorizMoveAttempt;
	DragonArcadeBullet _bullets[20];
	ArcadeNPCState _npcState[20];
	Common::SharedPtr<Image> _bulletImg;
	Common::SharedPtr<Image> _arrowImg;
	Common::SharedPtr<Image> _scrollImg;
	DragonArcadeTTM _arcadeTTM;
	Common::Array<int16> _floorY;
	Common::Array<bool> _floorFlag;
};

} // end namespace Dgds

#endif // DGDS_DRAGON_ARCADE_H
