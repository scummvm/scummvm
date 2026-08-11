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

#ifndef DGDS_MINIGAMES_CHINA_TRAIN_H
#define DGDS_MINIGAMES_CHINA_TRAIN_H

namespace Dgds {

enum PlayerType {
	kPlayerLucky,
	kPlayerTong,
};

enum PlayerAction {
	// The list of actions is not directly referenced in the game, but is in
	// the exe just after the "Heart of China Train Game" copyright
	kActionStandRight = 0,
	kActionWalkRight = 1,
	kActionWalkLeft = 2,
	kActionJumpRight = 3,
	kActionJumpLeft = 4,
	kActionDuckRight = 5,
	kActionFallRight = 6,
	kActionFallLeft = 7,
	kActionStagger = 8,
	kActionDeathScene = 9,
	kActionClub = 10,
	kActionClubHit = 11,
	kActionSwing = 12,
	kActionSwingHit = 13,
	kActionStab = 14,
	kActionStabHit = 15,
	kActionBlock = 16,
	kActionHeroicJump = 17,
	kActionBlockUp = 18,
	kActionFree6 = 19,
	kActionStandLeft = 20,
	kActionDuckLeft = 21,
	kActionFree7 = 22,
	kActionFree8 = 23,
	kActionERROR = 24,
};

enum PlayerIntent {
	kIntentDuck = 0,
	kIntentRest = 1,
	kIntentPursue = 2,
	kIntentAttack = 3,
	kIntentRetreat = 4,
	kIntentQ = 5,
	kIntentInvalid = 255,
};

class TrainPlayers;

struct PlayerData {
	int16 _frame;			 // field 0
	ImageFlipMode _flipMode; // field 1
	int16 _xstep;			 // field 2
	int16 _ystep;			 // field 3
	int16 _xoff;			 // field 4
	int16 _yoff;			 // field 5
	int16 _val6;			 // field 6
};

struct TunnelData {
	TunnelData() : _start(0), _end(0) {}
	int32 _start;
	int32 _end;
};

class TrainPlayer {
public:
	friend class TrainPlayers;
	TrainPlayer(PlayerType type);

	bool isBlocking() const { return _action == kActionBlock || _action == kActionBlockUp; }
	bool isDucking() const { return _action == kActionDuckRight || _action == kActionDuckLeft; }
	bool isFalling() const { return _action == kActionFallRight || _action == kActionFallLeft; }
	bool isJumping() const { return _action == kActionJumpRight || _action == kActionJumpLeft; }
	bool isStaggering() const { return _action == kActionStagger; }
	bool isStanding() const { return _action == kActionStandRight || _action == kActionStandLeft; }
	bool isWalking() const { return _action == kActionWalkLeft || _action == kActionWalkRight; }

	bool isTong() const { return _type == kPlayerTong; }
	bool isLucky() const { return _type == kPlayerLucky; }

	void checkLives();
	void doAttack(TrainPlayer &other);
	void doBlock();
	void doClub(int16 damage);
	void doPursue(const TrainPlayer &other);
	void doRun();
	void doJump();
	void setAction(PlayerAction state, bool flag);
	bool inRange(const TrainPlayer &other);
	void hit(int16 damage);
	void startStagger(const TrainPlayer &other);
	void readStuff(const Common::String &path);
	void computerDucks();
	void checkDuck(const TunnelData &currentTunnel);
	void doProcess();

	PlayerType _type;
	int16 _xpos;
	int16 _ypos;
	PlayerAction _action;
	int16 _fatigue;
	int16 _hitpoints;
	PlayerIntent _intent;
	int16 _ferocity;
	int16 _val7;
	Common::Array<Common::Array<PlayerData>> _allData;
	Common::Array<PlayerData> *_currentActionData;
	PlayerData *_data;

private:
	TrainPlayer &chooseEnemy();
	PlayerData *endOfCurrentAction();
	PlayerData *startOfCurrentAction();

	static int16 _blockSoundFlag;
};

class TrainPlayers {
public:
	TrainPlayers();

	void initPlayers();
	void checkLives();
	void readAnims();
	void freeAnims();
	void doScroll(int16 jumpOffset);

	TrainPlayer _lucky;
	TrainPlayer _tong;
};


/** Train fight mini-game for Heart of China */
class ChinaTrain {
public:
	ChinaTrain();

	void init(); 	// aka arcadeInit
	int16 tick(); 	// aka arcadeLoop
	void end();  	// aka aracdeReset

	//int16 currentCar() const { return _currentCar; }
	void checkTongFall(int16 xpos, int16 car);
	TrainPlayers &getPlayers() { return _players; }
	const TunnelData &getCurrentTunnel() const { return _currentTunnel; }
	void leaveArcade();
	bool checkGap(int16 xpos, int16 offset);

	void onKeyDown(Common::KeyState kbd);
	void onKeyUp(Common::KeyState kbd);
	void setMenuResult(bool yes);

	int16 _jumpOffset;

private:
	void arcadeFadeout();
	void getUserInput();
	void getNpcInput();
	void processInput();
	void handleVariables();
	void drawFrame();
	int16 trainArcade();
	void trainRestart();
	void lost();
	void makeNewTunnel();
	//void setupArcade(); // just sets fill=1 and _clipWin.
	void initScoreWindow();
	void drawBorder();
	void shadeLabel(int16 x, int16 y, int16 w, int16 h, const char *label);
	void shadeBox(Graphics::ManagedSurface &buf, byte backCol, byte foreCol, byte fill, int16 x, int16 y, int16 w, int16 h);

	void fixBorder();
	void drawCommandButtons();
	void drawBmps();
	void drawTunnel();
	void drawMountains(int16 num);
	void drawBlock(int16 x1, int16 x2, int16 param_3, int16 y1, int16 y2, int16 param_6, int16 param_7, int16 param_8);
	void drawButtons(int16 *buttons);
	void shadePressButton(int16 x, int16 y);
	void shadeButton(int16 x, int16 y);
	void drawScoreWindow();
	void drawScore();
	void drawSnow();
	void drawTrain();
	void drawActors();
	void drawCar(int16 xoff, int16 frame, int16 yoff);
	void fixUpTunnel();
	bool calcBounce(int16 car);
	void checkRegions(TrainPlayer &player);
	void processOrders(TrainPlayer &player, TrainPlayer &enemy);
	int16 readButtons();
	void cabooseLost();

	int16 _arcadeCount;
	int16 _arcadeFlag;
	bool _arcadeInitFlag;
	int16 _arcadeDrawFlag;

	int16 _failCounter;
	int16 _lastMaskedArcadeFlag;
	int16 _int3036;
	int16 _tongAttackCounter;
	int16 _tongInjuredCounter;
	int16 _tongRestTarget;
	int16 _lastTongHP;
	int16 _lastTongFatigue;
	TrainPlayers _players;
	int16 _currentCar;
	int32 _trackPos;
	int32 _frameCnt;
	int16 _xOffset;
	int16 _tunnelNum;
	TunnelData _currentTunnel;
	int16 _cabooseTrail;
	int16 _lastBtn;
	const Common::Rect _clipWin;
	//Common::SharedPtr<Image> _simChars;
	Common::SharedPtr<Image> _rectShape;
	Common::SharedPtr<Image> _luckyMaps;
	Common::SharedPtr<Image> _test;
	int16 _pressedCommandButton;
	bool _leftButtonDown;
	bool _rightButtonDown;
	int16 _lastKeycode;
	bool _playedTunnelSFX;
};

} // end namespace Dgds

#endif // DGDS_MINIGAMES_CHINA_TRAIN_H
