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

struct ArcadeNPCState {
	int16 val1;
	int16 val2;
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
	int8 byte14; /* Set to 0, 1 or 2 */
	int16 x_21;
	int16 y_21;
	int16 x_22;
	int16 y_22;
};

struct AracdeSomething {
	int16 val0;
	int16 val2;
	byte unk4[12];
	int16 val10;
	byte val12;
	byte unk13;
	byte val14;
	byte val15;
	byte unk16[8];
};

class DragonArcadeBullet {
public:
	DragonArcadeBullet() : _x(0), _y(0), _state(kBulletInactive),
		_flipMode(kImageFlipNone), _var1(0), _speed(0), _var2(0), _var3(0) {}

	int16 _x;
	int16 _y;
	DragonBulletState _state;
	ImageFlipMode _flipMode;
	int16 _var1;
	uint16 _speed;
	int16 _var2;
	int16 _var3;
};

class DragonArcade {
public:
	DragonArcade();

	void arcadeTick();

private:
	void initIfNeeded();
	void drawBackgroundAndWeapons();
	void checkToOpenMenu();
	void clearAllBulletStates();
	void clearAllNPCStates();
	void createBullet(int16 x, int16 y, ImageFlipMode flipMode, uint16 var1);
	void bladeTakeHitAndCheck();
	void enemyTakeHit() { _bossHealth--; }
	void enemyTakeHitAndCheck();
	void playSfx(int16 num) const;
	void drawBulletHitCircles(uint16 x, uint16 y, bool flag);
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
	void arcadeUpdateXScrollOffset();
	void arcade1e83();
	void arcade2445();
	void arcade2754(int16 findResult);
	void arcade3e96();
	void arcade4085();
	void arcade34b4();
	bool isNpcInsideXRange(int16 num);
	void updateBullets();
	void checkBladeFireAllStages();
	void checkEnemyFireStage0124();
	void checkBossFireStage3();
	void checkBossFireStage6();
	void updateMouseAndJoystickStates();
	int16 findValuesIn3d30Array();
	int16 checkBulletCollision(int16 num);
	void mouseUpdate();
	void keyboardUpdate();
	void limitToCenterOfScreenAndUpdateCursor();
	uint16 moveToNextStage();
	void findMatchIn3d30Array();
	void findMinIn3d30ArrayFirst();
	void findMinIn3d30ArrayLast();
	void findMatchOrMinOrMax();
	void findMaxIn3d30Array();
	void fill3d30ArrayFromLevelData();
	bool isAbsFoundValOver990();
	void playSFX55AndStuff();
	void moveBladeX();
	void handleMouseStates();
	void updateXScrollOffset();

	int16 _bladeHealth;
	int16 _bossHealth;
	int16 _lastDrawnBladeHealth;
	int16 _lastDrawnBossHealth;
	uint16 _nextRandomVal;
	int16 _loadedArcadeStage;
	int16 _nextStage;
	int16 _arcadeModeSomethingCounter;
	int16 _currentArcadeTT3Num;
	int16 _shouldUpdateState;
	int16 _finishCountdown;
	int16 _bladeState1;
	int16 _bladeStateOffset;
	uint16 _mouseButtonWentDown;
	int16 _scrollXOffset;
	int32 _nTickUpdates;
	int16 _startDifficultyMaybe;
	int16 _bossStateUpdateCounter;
	int16 _someCounter40f0;
	int16 _int0b5c;
	uint16 _uint0a17;
	int16 _int0b54;
	int16 _int0b58;
	int16 _int0b60;
	int16 _int3d18;
	uint16 _uint0be6;
	bool _flag3d14;
	int16 _scrollXIncrement;
	int16 _lMouseButtonState;
	int16 _rMouseButtonState;
	int16 _lastLMouseButtonState;
	int16 _lastRMouseButtonState;
	int16 _bladeXMove;
	int16 _bladeHorizMoveAttempt;
	int16 _currentArrowNum;
	int16 _foundValueFrom3d30Array;
	bool _foundValueFrom3d1cArray;
	int16 _2754Val;

	bool _haveBigGun;
	bool _haveBomb;
	bool _enemyHasSmallGun;
	bool _dontRedrawBgndAndWeapons;
	bool _arcadeNeedsBufferCopy;
	bool _flagInventoryOpened;
	bool _initFinished;
	bool _stillLoadingScriptsMaybe;
	bool _flag40ee;
	bool _flag40ef;
	int16 _arcadeNotMovingLeftFlag;
	bool _bladeHasFired;
	bool _mouseIsAvailable;
	bool _isMovingStage;
	DragonBladeMoveFlag _bladeMoveFlag;
	DragonBladeMoveFlag _keyStateFlags;
	DragonBladeMoveFlag _bladeMoveFlagBeforeRButton;
	DragonArcadeBullet _bullets[20];
	ArcadeNPCState _npcState[10];
	AracdeSomething _somethingArray[9];
	Common::SharedPtr<Image> _bulletImg;
	Common::SharedPtr<Image> _arrowImg;
	Common::SharedPtr<Image> _scrollImg;
	DragonArcadeTTM _arcadeTTM;
	Common::Array<int16> _array3d30;
	Common::Array<bool> _array3d1c;
};

} // end namespace Dgds

#endif // DGDS_DRAGON_ARCADE_H
