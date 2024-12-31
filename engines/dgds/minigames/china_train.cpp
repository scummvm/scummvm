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

#include "common/system.h"

#include "dgds/dgds.h"
#include "dgds/drawing.h"
#include "dgds/globals.h"
#include "dgds/game_palettes.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/font.h"
#include "dgds/scene.h"
#include "dgds/sound.h"
#include "dgds/resource.h"
#include "dgds/minigames/china_train.h"

namespace Dgds {

/*
 * Not used anywhere, but these strings are in the EXE and conveniently
 * tell us what the PlayerAction enum values are.
 *
static const char *ACTIONS[] = {
	"Stand Right",
	"Walk Right",
	"Walk Left",
	"Jump Right",
	"Jump Left",
	"Duck Right",
	"Fall Right",
	"Fall Left",
	"Stagger",
	"Death Scene",
	"Club",
	"Club Hit",
	"Swing",
	"Swing Hit",
	"Stab",
	"Stab Hit",
	"Block",
	"Heroic Jump",
	"Block Up",
	"Free6",
	"Stand Left",
	"Duck Left",
	"Free7",
	"Free8",
	"##> ERROR <##",
};
*/

static const int16 COMMAND_BUTTONS[][6] = {
	//   x     y    w    h    id  str (always 0)
	{ 0xD5, 0x96, 0xC, 0xA, 0x20, 0x0 }, // DUCK
	{ 0xE8, 0x96, 0xC, 0xA, 0x1E, 0x0 }, // ATTACK
	{ 0xD5, 0xA7, 0xC, 0xA, 0x1F, 0x0 }, // REST
	{ 0xE8, 0xA7, 0xC, 0xA, 0x13, 0x0 }, // RETREAT
};

static const int16 TRAIN[] = { 3, 2, 2, 2, 1, -1 };

int16 TrainPlayer::_blockSoundFlag = 0;

TrainPlayer::TrainPlayer(PlayerType type) : _type(type), _xpos(0), _ypos(0), _action(kActionStandRight),
_fatigue(0), _hitpoints(0), _intent(kIntentInvalid), _ferocity(0), _val7(0), _currentActionData(nullptr), _data(nullptr)
{}

void TrainPlayer::doPursue(const TrainPlayer &other) {
	if (_xpos < other._xpos)
		setAction(kActionWalkRight, false);
	else if (other._xpos < _xpos)
		setAction(kActionWalkLeft, false);
	else
		_intent = kIntentRest;
}

void TrainPlayer::doRun() {
	if (_val7 < _xpos) {
		// FIXME: Rechecking this doesn't make sense, but that's what the original does..
		if (_val7 < _xpos)
			setAction(kActionWalkLeft, false);
		else
			_intent = kIntentRest;
	} else {
		setAction(kActionWalkRight, false);
	}
}

void TrainPlayer::setAction(PlayerAction state, bool flag) {
	_action = state;
	assert((uint)state < _allData.size());
	_currentActionData = _allData.data() + (uint)state;

	if (flag)
		_data = startOfCurrentAction();
}

bool TrainPlayer::inRange(const TrainPlayer &other) {
	return abs(_xpos - other._xpos) < 30;
}

void TrainPlayer::hit(int16 damage) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int16 difficulty = engine->getDifficulty();
	if (isTong())
		damage = damage << (2 - difficulty);
	_hitpoints -= damage;
	engine->_soundPlayer->playSFX(0x88);
}

void TrainPlayer::startStagger(const TrainPlayer &other) {
	ChinaTrain *game = DgdsEngine::getInstance()->getChinaTrain();
	if (_data->_flipMode == kImageFlipNone) {
		_xpos = other._xpos - 30;
		setAction(kActionStagger, true);
		if (isTong())
			game->checkTongFall(_xpos, 4);
	} else {
		_xpos = other._xpos + 30;
		setAction(kActionStagger, true);
		if (isTong())
			game->checkTongFall(_xpos, 1);
	}
}

uint16 _effective(int32 ferocity, int32 fatigue) {
	fatigue = MIN((int32)100, fatigue);
	ferocity = MIN((int32)6, ferocity);

	if (ferocity * 10000 <= fatigue * 600)
		return 0;
	else
		// max of 6 * 10000
		return (uint16)(ferocity * 10000 - fatigue * 600);
}

int16 _getFatigue(int16 val) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int16 difficulty = engine->getDifficulty();
	return (difficulty == 2 ? val : val / 2);
}

void TrainPlayer::checkDuck(const TunnelData &currentTunnel) {
	// Check if the player should be ducking to avoid the end of the tunnel
	int32 distanceToStart = (currentTunnel._start + 2) * 8 - _xpos;
	int32 distanceToEnd = (currentTunnel._end + 6) * 8 - _xpos;

	if (!isDucking() && 0 < _xpos && _xpos < 320
		&& (currentTunnel._start < 26 || currentTunnel._end < 39)
		&& ((distanceToStart < 16 && distanceToStart >= 0) || (distanceToEnd < 16 && distanceToEnd >= 0)
		)
	) {
		if (isTong() && !isJumping()) {
			// Tong always ducks..
			setAction(kActionDuckRight, true);
		} else {
			// Lucky falls
			setAction(kActionFallLeft, true);
			DgdsEngine::getInstance()->_soundPlayer->playSFX(131);
		}
	}
}

void TrainPlayer::doAttack(TrainPlayer &other) {
	PlayerAction newAction;
	if (!isStanding() || !other.isStanding() || other.isFalling()) {
		if (_data->_flipMode == kImageFlipNone)
			newAction = kActionStandRight;
		else
			newAction = kActionStandLeft;
	} else {
		if (other._data->_flipMode != _data->_flipMode) {
			if (_xpos < other._xpos)
				_xpos = other._xpos - 26;
			else
				_xpos = other._xpos + 26;
		}

		Common::RandomSource &random = DgdsEngine::getInstance()->getRandom();
		uint16 peffective = _effective(_ferocity, _fatigue);
		uint16 randVal1 = random.getRandomNumber(UINT16_MAX);
		if (randVal1 < peffective && !other.isStaggering() && !other.isDucking()
			&& !other.isJumping() && !other.isFalling() && other._data->_flipMode != _data->_flipMode) {
			randVal1 = random.getRandomNumber(UINT16_MAX);
			int16 chance = (_fatigue >> 4) + 4;
			PlayerAction response;
			PlayerAction randActions[16];
			PlayerAction randResponse[16];

			uint16 p2effective = _effective(other._ferocity, _getFatigue(other._fatigue));
			uint16 p1effective = _effective(_ferocity, _getFatigue(_fatigue));

			if (p2effective - (p1effective >> 2) < randVal1)
				response = kActionBlock;
			else
				response = kActionStabHit;

			for (int i = 0; i < chance && i < 16; i++) {
				randActions[i] = kActionStab;
				randResponse[i] = response;
			}

			if (response != kActionBlock)
				response = kActionSwingHit;

			for (int i = chance; i < chance * 2 && i < 16; i++) {
				randActions[i] = kActionSwing;
				randResponse[i] = response;
			}

			if (response == kActionBlock)
				response = kActionBlockUp;
			else
				response = kActionClubHit;

			for (int i = chance * 2; i < 16; i++) {
				randActions[i] = kActionClub;
				randResponse[i] = response;
			}

			randVal1 = random.getRandomNumber(15);
			setAction(randActions[randVal1], true);
			other.setAction(randResponse[randVal1], true);
			if (response != kActionClubHit)
				return;

			DgdsEngine::getInstance()->getChinaTrain()->getPlayers().checkLives();
			return;
		}

		if (_data->_flipMode == kImageFlipNone)
			newAction = kActionStandRight;
		else
			newAction = kActionStandLeft;
	}
	setAction(newAction, true);
}

void TrainPlayer::doClub(int16 damage) {
	if (_data->_val6 != 0) {
		TrainPlayer &enemy = chooseEnemy();
		if (inRange(enemy)) {
			if (!enemy.isBlocking())
				enemy.hit(damage);
		}
	}

	if (_data == endOfCurrentAction()) {
		if (_data->_flipMode == kImageFlipNone)
			setAction(kActionStandRight, true);
		else
			setAction(kActionStandLeft, true);
	}
}

void TrainPlayer::doJump() {
	TrainPlayer &enemy = chooseEnemy();
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (_data->_val6)
		engine->_soundPlayer->playSFX(0x87);


	if (inRange(enemy)) {
		if (_data->_flipMode == kImageFlipNone)
			enemy._xpos = _xpos + 0x1e;
		else
			enemy._xpos = _xpos - 0x1e;

		if (engine->getChinaTrain()->checkGap(enemy._xpos, 2)) {
			if (enemy._xpos < 0xa0)
				enemy._xpos = _xpos + 0x1e;
			else
				enemy._xpos = _xpos - 0x1e;
		}
	}

	if (_data == startOfCurrentAction()) {
		if (_data->_flipMode == kImageFlipNone)
			setAction(kActionStandRight, true);
		else
			setAction(kActionStandLeft, true);

		if (isLucky()) {
			if (_data->_flipMode == kImageFlipNone && 0xdc < _xpos) {
				setAction(kActionWalkRight, true);
				engine->getChinaTrain()->_jumpOffset++;
				_xpos -= 0x10;
				enemy._xpos -= 0x10;
			} else if (_data->_flipMode != kImageFlipNone && (0x5d >= _xpos)) {
				setAction(kActionWalkLeft, true);
				engine->getChinaTrain()->_jumpOffset--;
				_xpos += 0x10;
				enemy._xpos += 0x10;
			}
		}
	}
	_fatigue++;

}

void TrainPlayer::doBlock() {
	TrainPlayer &enemy = chooseEnemy();
	if (_data->_val6 == 0) {
		_blockSoundFlag = 0;
	} else {
		if (_blockSoundFlag == 0) {
			DgdsEngine *engine = DgdsEngine::getInstance();
			uint16 randval = engine->getRandom().getRandomNumber(UINT16_MAX);
			//_blockSoundFlag = randval; // bug in original? it assigns this twice
			_blockSoundFlag = -(randval >> 0xf);

			engine->_soundPlayer->playSFX(-(randval >> 0xf) + 0x80);
		}
		// TODO: What is going on here??
		//enemy._data++;
		//assert(enemy._data->_frame != -1);
		enemy._data++;
		if (enemy._data->_frame == -1)
			enemy._data = enemy.startOfCurrentAction();
	}

	if (_data == endOfCurrentAction()) {
		_fatigue++;
		if (_data->_flipMode == kImageFlipNone) {
			enemy.setAction(kActionStandLeft, true);
			setAction(kActionStandRight, true);
		} else {
			setAction(kActionStandLeft, true);
			enemy.setAction(kActionStandRight, true);
		}
	}
}

TrainPlayer &TrainPlayer::chooseEnemy() {
	ChinaTrain *game = DgdsEngine::getInstance()->getChinaTrain();
	if (isTong())
		return game->getPlayers()._lucky;
	else
		return game->getPlayers()._tong;
}


void TrainPlayer::readStuff(const Common::String &resname) {
	ResourceManager *resMan = DgdsEngine::getInstance()->getResourceManager();
	Common::SeekableReadStream *stream = resMan->getResource(resname);
	if (!stream)
		error("Couldn't open train animation %s", resname.c_str());

	_allData.clear();

	uint16 nitems = stream->readUint16LE();
	debug(10, "Reading %d items from %s", nitems, resname.c_str());

	uint16 gotitems = 0;
	Common::Array<PlayerData> currentArray;
	while (gotitems < nitems && !stream->eos()) {
		PlayerData data;
		data._frame = stream->readSint16LE();
		data._flipMode = static_cast<ImageFlipMode>(stream->readUint16LE());
		data._xstep = stream->readSint16LE();
		data._ystep = stream->readSint16LE();
		data._xoff = stream->readSint16LE();
		data._yoff = stream->readSint16LE();
		data._val6 = stream->readSint16LE();

		//
		// Frame -1 means end of current list.  Add it to the list as a sentinal
		// and manually reset to the start.
		//
		// The original uses a lopped linked list so the next frame is always
		// avaialble.
		//
		currentArray.push_back(data);
		if (data._frame == -1) {
			_allData.push_back(currentArray);
			currentArray.clear();
		} else {
			gotitems++;
		}
	}

	debug(10, "Got %d/%d items from %s", gotitems, nitems, resname.c_str());

	delete stream;
}

void TrainPlayer::computerDucks() {
	PlayerAction newState;
	ChinaTrain *game = DgdsEngine::getInstance()->getChinaTrain();
	const TunnelData &currentTunnel = game->getCurrentTunnel();
	int16 xd1 = (currentTunnel._start + 10) * 8 - _xpos;
	int16 xd2 = currentTunnel._start * 8 - _xpos;
	if (((xd1 < 0x130 && 0 < xd1) || (xd2 < 0x80 && -0x60 < xd2)) &&
			(isStanding() || isWalking() || isDucking())) {
		if (_data->_flipMode == kImageFlipNone)
			newState = kActionDuckRight;
		else
			newState = kActionDuckLeft;
	} else {
		if (!isDucking())
			return;

		if (chooseEnemy()._xpos < _xpos)
			newState = kActionStandLeft;
		else
			newState = kActionStandRight;
	}
	setAction(newState, true);
}

PlayerData *TrainPlayer::endOfCurrentAction() {
	// return the last real item in the current action
	assert(_currentActionData);
	int offset = (int)_currentActionData->size() - 2;
	assert(offset >= 0);
	return _currentActionData->data() + offset;
}

PlayerData *TrainPlayer::startOfCurrentAction() {
	assert(_currentActionData);
	return _currentActionData->data();
}

void TrainPlayer::doProcess() {
	ChinaTrain *game = DgdsEngine::getInstance()->getChinaTrain();
	assert(_data);
	_xpos += _data->_xstep;
	_ypos += _data->_ystep;

	_data++;
	if (_data->_frame == -1) // hit the end of the list.
		_data = startOfCurrentAction();

	assert(_data->_frame != -1);

	switch (_action) {
	case kActionStandRight:
	case kActionStandLeft:
		_ypos = 95;
		if (_fatigue != 0 && _data == startOfCurrentAction())
			_fatigue--;

		break;
	case kActionWalkRight:
		_ypos = 95;
		setAction(kActionStandRight, 0);
		if (_data == startOfCurrentAction())
			_fatigue++;

		break;
	case kActionWalkLeft:
		_ypos = 95;
		setAction(kActionStandLeft, 0);
		if (_data == startOfCurrentAction())
			_fatigue++;

		break;
	case kActionJumpRight:
	case kActionJumpLeft:
		doJump();
		break;
	case kActionDuckRight:
	case kActionDuckLeft:
		if (_fatigue != 0 && _data == startOfCurrentAction())
			_fatigue--;

		break;
	case kActionFallRight:
	case kActionFallLeft:
		if (0xa0 < _ypos) {
			game->leaveArcade();
		}
		_hitpoints = 0;
		break;
	case kActionStagger:
		if (_data == startOfCurrentAction()) {
			if (_data->_flipMode == kImageFlipNone)
				setAction(kActionStandRight, true);
			else
				setAction(kActionStandLeft, true);
		}
		break;
	case kActionDeathScene:
		if (isLucky()) {
			if (_data->_val6) {
				_data++;
				if (_data->_frame == -1)
					_data = endOfCurrentAction();
			}
		} else if (_data->_val6) {
			game->leaveArcade();
		}
		break;
	case kActionClub:
		doClub(5);
		_fatigue += 2;
		break;
	case kActionClubHit:
	case kActionSwingHit:
	case kActionStabHit:
		if (_data == startOfCurrentAction())
			startStagger(chooseEnemy());
		break;
	case kActionSwing:
		doClub(3);
		_fatigue++;
		break;
	case kActionStab:
		doClub(1);
		_fatigue++;
		break;
	case kActionBlock:
	case kActionBlockUp:
		doBlock();
		break;
	default:
		break;
	}

	if (100 < _fatigue)
		_fatigue = 100;
}


//////////////////////////

TrainPlayers::TrainPlayers() : _lucky(kPlayerLucky), _tong(kPlayerTong) {
}

void TrainPlayers::initPlayers() {
	_lucky._xpos = 160;
	_lucky._ypos = 95;
	_lucky.setAction(kActionStandLeft, true);
	_lucky._hitpoints = 100;
	_lucky._fatigue = 0;
	_lucky._ferocity = 0;
	_lucky._intent = kIntentRest;
	_tong._xpos = 110;
	_tong._ypos = 95;
	_tong.setAction(kActionStandRight, true);
	_tong._hitpoints = 100;
	_tong._fatigue = 0;
	_tong._ferocity = 1;
	_tong._intent = kIntentAttack;
}

void TrainPlayers::checkLives() {
	if (_lucky._hitpoints < 3) {
		_tong.setAction(kActionDeathScene, true);
		_lucky.setAction(kActionDeathScene, true);
	} else if (_tong._hitpoints < 4) {
		_tong._intent = kIntentQ;
	}
}

void TrainPlayers::readAnims() {
	_lucky.readStuff("lucky.anm");
	_tong.readStuff("tong.anm");
}

void TrainPlayers::freeAnims() {
	_lucky._allData.clear();
	_tong._allData.clear();
	_lucky._currentActionData = nullptr;
	_lucky._data = nullptr;
	_tong._currentActionData = nullptr;
	_tong._data = nullptr;
}


void TrainPlayers::doScroll(int16 jumpOffset) {
	if (!jumpOffset)
		return;

	int16 offset = (jumpOffset < 0) ? 16 : -16;
	_lucky._xpos += offset;
	_tong._xpos += offset;
	_lucky._val7 += offset;
}

//////////////////////////


ChinaTrain::ChinaTrain() : _arcadeCount(0), _arcadeFlag(0), _arcadeInitFlag(false),
_arcadeDrawFlag(0), _failCounter(0), _lastMaskedArcadeFlag(0), _int3036(0),
_tongAttackCounter(0), _tongInjuredCounter(0), _tongRestTarget(0), _lastTongHP(0),
_lastTongFatigue(0), _currentCar(0), _trackPos(0), _frameCnt(0), _xOffset(0), _tunnelNum(0),
_cabooseTrail(0), _lastBtn(0), _pressedCommandButton(-1), _leftButtonDown(false),
_rightButtonDown(false), _lastKeycode(0), _playedTunnelSFX(false), _jumpOffset(0),
_clipWin(Common::Point(8, 0), 304, 130) {
}

void ChinaTrain::init() { // aka arcadeInit
	if (_arcadeInitFlag)
		return;

	//_reshow_before_ads = _arc_refresh;
	DgdsEngine *engine = DgdsEngine::getInstance();

	engine->disableKeymapper();
	engine->getBackgroundBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	engine->getStoredAreaBuffer().fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);

	_rectShape.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_rectShape->loadBitmap("bkgnd.bmp");

	engine->getGamePals()->loadPalette("hc.pal");

	_test.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_test->loadBitmap("train.bmp");
	_luckyMaps.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_luckyMaps->loadBitmap("lucky.bmp");

	_players.readAnims();
	trainRestart();
	// engine->getFontMan()->getFont(font6);
	// _fgcolor = 0xe;

	engine->_soundPlayer->playSFX(125);
	_arcadeInitFlag = true;
	_arcadeCount = 0;
	_arcadeDrawFlag = 1;

	_leftButtonDown = false;
	_rightButtonDown = false;
	_lastKeycode = false;
	_playedTunnelSFX = false;
	_pressedCommandButton = -1;
	_lastBtn = 0;

	// setupArcade();  // this just sets clipping window which we already set.
	initScoreWindow();

	// the background buffer is now set up, copy it to the composition buffer
	engine->_compositionBuffer.blitFrom(engine->getBackgroundBuffer());

	drawFrame();

	// ensure mouse cursor is on
	engine->setMouseCursor(kDgdsMouseGameDefault);

	engine->setMenuToTrigger(kMenuSkipArcade);

	// Copy to screen now so the menu is drawn over the game background.
	g_system->copyRectToScreen(engine->_compositionBuffer.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int16 ChinaTrain::tick() { // aka arcadeLoop
	if (!trainArcade())
		return 0;

	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	int16 trainState = globals->getTrainState();

	if (trainState == 3 || trainState == 4) {
		arcadeFadeout();
		end();
		_arcadeFlag = 0;
		return 0;
	} else {
		getUserInput();
		getNpcInput();
		processInput();
		handleVariables();
		drawFrame();

		// Original has delay here to reduce the arcade speed to 15 FPS.
		DgdsEngine::getInstance()->setSkipNextFrame();

		return 1;
	}
}

void ChinaTrain::arcadeFadeout() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	for (int i = 63; i > 0; i--) {
		engine->getGamePals()->setFade(0, 255, 0, i * 4);
		g_system->updateScreen();
		g_system->delayMillis(5);
	}
	Common::Rect screen(SCREEN_WIDTH, SCREEN_HEIGHT);
	engine->_compositionBuffer.fillRect(screen, 0);
	engine->getBackgroundBuffer().fillRect(screen, 0);
	g_system->lockScreen()->fillRect(screen, 0);
	g_system->unlockScreen();
	engine->getGamePals()->setFade(0, 255, 0, 255);
}

int16 ChinaTrain::trainArcade() {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	int16 trainState = globals->getTrainState();
	if (trainState == 0) {
		globals->setTrainState(1);
		if (_int3036 != 0)
			_arcadeFlag |= 0xa5;
		else
			_arcadeFlag |= 0x25;

		_failCounter = 0;
		_lastMaskedArcadeFlag = 0;
	}
	if (_lastMaskedArcadeFlag == 0)
		_lastMaskedArcadeFlag = _arcadeFlag & 0x380;

	if (_lastMaskedArcadeFlag == 0) {
		return 1;
	} else {
		if (!(_arcadeFlag & _lastMaskedArcadeFlag)) {
			if (_arcadeFlag & 0x400 || _lastMaskedArcadeFlag == 0x80) {
				globals->setTrainState(1);
				trainRestart();
				_lastMaskedArcadeFlag = 0;
				return 1;
			}
			if (!(_lastMaskedArcadeFlag & 0x200))
				globals->setTrainState(3);
			else
				globals->setTrainState(4);

			_lastMaskedArcadeFlag = 0;
		}
		return 0;
	}
}

void ChinaTrain::setMenuResult(bool yes) {
	if (_arcadeInitFlag && _arcadeFlag)
		_arcadeFlag = (yes ? 0x400 : 0);
}

void ChinaTrain::end() { // aka arcadeReset
	if (!_arcadeInitFlag)
		return;

	//_reshow_before_ads = _game_reshow;
	//_matrix.clear();
	_rectShape.reset();
	_luckyMaps.reset();
	_test.reset();

	_players.freeAnims();
	DgdsEngine *engine = DgdsEngine::getInstance();
	engine->_soundPlayer->stopSfxByNum(125);

	engine->enableKeymapper();
	_arcadeInitFlag = false;
	_arcadeDrawFlag = 0;
	_arcadeFlag = 0;
}

void ChinaTrain::leaveArcade() {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	assert(globals);

	int16 trainState = globals->getTrainState();
	if (trainState != 4)
		lost();
}

void ChinaTrain::trainRestart() {
	_currentCar = 2;
	_trackPos = 0;
	_xOffset = 0;
	_tunnelNum = 0;
	makeNewTunnel();
	_players.initPlayers();
}

void ChinaTrain::makeNewTunnel() {
	Common::RandomSource &random = DgdsEngine::getInstance()->getRandom();
	uint16 randval = random.getRandomNumber(UINT16_MAX);
	_currentTunnel._start = (randval >> 9) + 350;
	randval = random.getRandomNumber(UINT16_MAX);
	_currentTunnel._end = _currentTunnel._start + (randval >> 10) + 100;
}

void ChinaTrain::lost() {
	_failCounter++;

	DgdsEngine *engine = DgdsEngine::getInstance();

	if (_failCounter < 5) {
		engine->setMenuToTrigger(kMenuReplayArcade);
		_arcadeFlag |= 0x100;
	} else {
		engine->setMenuToTrigger(kMenuArcadeFrustrated);
		_arcadeFlag |= 0x200;
	}

	_lastMaskedArcadeFlag = _arcadeFlag & 0x300;
}

void ChinaTrain::checkTongFall(int16 xpos, int16 car) {
	if (checkGap(xpos, -2) && _currentCar == car)
		leaveArcade();
}

bool ChinaTrain::checkGap(int16 xpos, int16 offset) {
	int16 xmin = (94 - offset) - _jumpOffset;
	int16 xmax = (offset + 220) - _jumpOffset;
	return ((xmin + offset * 2 - 48 < xpos) && xpos < xmin)
		|| (xmax < xpos && xpos < xmax - offset * 2 + 48)
		|| (_currentCar == 4 && xpos < xmin)
		|| (_currentCar == 1 && xmax < xpos);
}

void ChinaTrain::initScoreWindow() {
	Graphics::ManagedSurface &buf = DgdsEngine::getInstance()->getBackgroundBuffer();
	drawBorder();
	buf.fillRect(Common::Rect(Common::Point(8, 143), 304, 39), 126);
	shadeBox(buf, 23, 20, 0, 0xac, 0x91, 0x82, 0x24);
	shadeBox(buf, 17, 6, 0, 0xad, 0x92, 0x80, 0x22);
	shadeLabel(0xb1, 0x97, 0x1e, 9, "DUCK");
	shadeLabel(0xb1, 0xa8, 0x1e, 9, "REST");
	shadeLabel(0xfa, 0x97, 0x2d, 9, "ATTACK");
	shadeLabel(0xfa, 0xa8, 0x2d, 9, "RETREAT");
	shadeLabel(0x2a, 0x99, 0x1e, 9, "LIFE");
	shadeLabel(0x67, 0x99, 0x2d, 9, "FATIGUE");
	shadeLabel(0x42, 0xac, 0x31, 9, "FEROICTY");
	buf.fillRect(Common::Rect(Common::Point(0, 195), 320, 5), 0);
}

void ChinaTrain::drawBorder() {
	//
	// This is not exactly how the original does it, but should work.
	// The original uses indexes into the tiling matrix for the image.
	// We just reference the tiles we want directly.
	//
	// TL is 370, 372, 20
	// TR is 340 693, 703
	// BR is 340, 350, 360
	// BL is 0, 10, 20
	//
	// LEFT/RIGHT border is 371 (or 704?)
	// TOP/BOTTOM border is 87
	// RIGHT MIDDLE EDGE (between game and control panel) is 77
	// LEFT MIDDLE EDGE (between game and control panel) is 30
	// MIDDLE LINE (between game and control panel) is 40
	//

	Graphics::ManagedSurface &buf = DgdsEngine::getInstance()->getBackgroundBuffer();
	const Common::Rect screen(SCREEN_WIDTH, SCREEN_HEIGHT);

	for (int16 i = 1; i < 39; i++)
		_rectShape->drawBitmap(87, i * 8, 0, screen, buf);

	// top-left
	_rectShape->drawBitmap(370, 0, 0, screen, buf);
	_rectShape->drawBitmap(372, 8, 0, screen, buf);
	_rectShape->drawBitmap(20, 16, 0, screen, buf);

	// top-right
	_rectShape->drawBitmap(340, 0x128, 0, screen, buf);
	_rectShape->drawBitmap(693, 0x130, 0, screen, buf);
	_rectShape->drawBitmap(703, 0x138, 0, screen, buf);

	// bottom-left
	_rectShape->drawBitmap(0, 0, 0xb6, screen, buf);
	_rectShape->drawBitmap(10, 8, 0xb6, screen, buf);
	_rectShape->drawBitmap(20, 16, 0xb6, screen, buf);

	// bottom-right
	_rectShape->drawBitmap(340, 0x128, 0xb6, screen, buf);
	_rectShape->drawBitmap(350, 0x130, 0xb6, screen, buf);
	_rectShape->drawBitmap(360, 0x138, 0xb6, screen, buf);

	// Mid corners (between game and score area)
	_rectShape->drawBitmap(30, 0, 0x82, screen, buf);
	_rectShape->drawBitmap(40, 8, 0x82, screen, buf);
	_rectShape->drawBitmap(40, 0x130, 0x82, screen, buf);
	_rectShape->drawBitmap(77, 0x138, 0x82, screen, buf);

	for (int16 i = 2; i < 0x26; i++)
		_rectShape->drawBitmap(40, i * 8, 0x82, screen, buf);

	for (int16 i = 3; i < 0x25; i++)
		_rectShape->drawBitmap(87, i * 8, 0xb6, screen, buf);

	// Complete left/right sides
	for (int16 i = 1; i < 10; i++) {
		// Left side
		_rectShape->drawBitmap(371, 0, i * 13, screen, buf);
		// Right side
		_rectShape->drawBitmap(371, 0x138, i * 13, screen, buf);
	}

	for (int16 i = 11; i < 14; i++) {
		_rectShape->drawBitmap(371, 0, i * 13, screen, buf);
		_rectShape->drawBitmap(371, 0x138, i * 13, screen, buf);
	}
}

void ChinaTrain::drawBlock(int16 x1, int16 x2, int16 param_3, int16 y1, int16 y2, int16 stride, int16 xoffset, int16 param_8) {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	for (; y1 < y2; y1++) {
		for (int16 xx = x1; xx < x2; xx = xx + 1) {
			if (xx <= 0 || xx >= 39)
				continue;
			int16 matrixX = param_8 + ((param_3 + xx) - x1 + xoffset) % stride;
			int16 matrixY = y1;
			int16 frameno = _rectShape->getFrameFromMatrix(matrixX, matrixY);
			_rectShape->drawBitmap(frameno, xx * 8, y1 * 13, _clipWin, compBuf);
		}
	}
}

void ChinaTrain::drawMountains(int16 param) {
	static int16 lastXOffsetOdd = 0;
	static int16 numEvenXOffsets = 0;

	int16 x2;
	if (_currentTunnel._start < 40 && _currentTunnel._start >= 0)
		x2 = _currentTunnel._start;
	else
		x2 = 39;

	// Top row - mountains
	drawBlock(param, x2, param - 1, 1, 5, 40, 0, 0);

	// Mid row - moves at 1/2 speed xOffset
	if (_xOffset % 2 != lastXOffsetOdd) {
		lastXOffsetOdd = _xOffset % 2;
		if (lastXOffsetOdd == 0) {
			numEvenXOffsets++;
			if (numEvenXOffsets > 39)
				numEvenXOffsets = 0;
		}
	}

	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	for (int16 xx = param; xx < 39; xx = xx + 1) {
		//int16 frameno = 5 * 40 + ((xx + numEvenXOffsets) % 40);
		int16 frameno = _rectShape->getFrameFromMatrix((xx + numEvenXOffsets) % 40, 5);
		_rectShape->drawBitmap(frameno, xx * 8, 0x41, _clipWin, compBuf);
	}

	// 3rd row - moves at speed of xOffset
	drawBlock(param, x2, param - 1, 6, 7, 40, _xOffset, 0);
	// Bottom row - moves at 2x speed of xOffset
	drawBlock(param, x2, param - 1, 7, 10, 40, _xOffset * 2, 0);
}

void ChinaTrain::drawTunnel() {
	int16 x2;
	if (_currentTunnel._start + 13 < 40)
		x2 = _currentTunnel._start + 13;
	else
		x2 = 39;

	drawBlock(_currentTunnel._start, x2, 0, 1, 10, 13, 0, 0x29);

	if (x2 < 39 && _currentTunnel._end >= 0) {
		if (_currentTunnel._end < 40)
			x2 = _currentTunnel._end;
		else
			x2 = 39;

		drawBlock(_currentTunnel._start + 13, x2, 0, 1, 10, 12, 0, 0x43);
	}

	if (_currentTunnel._end < 39) {
		drawBlock(_currentTunnel._end, _currentTunnel._end + 13, 0, 1, 10, 13, 0, 0x36);
	}
}

int16 ChinaTrain::readButtons() {
	const Common::Point pt = DgdsEngine::getInstance()->getLastMouse();
	_pressedCommandButton = -1;
	int16 retVal = 0;
	for (int btn = 0; btn < ARRAYSIZE(COMMAND_BUTTONS); btn++) {
		const int16 *btnData = COMMAND_BUTTONS[btn];

		if (btnData[0] < pt.x && pt.x < btnData[0] + btnData[2]
			&& btnData[1] < pt.y && pt.y < btnData[1] + btnData[3]) {
			retVal = btnData[4];
			_pressedCommandButton = btn;
		}
	}
	return retVal;
}

void ChinaTrain::getUserInput() {
	/*
	TODO: original gets joystick values here.
	djoy(0);
	joyButton(0);
	joyButton(1);
	*/
	_rightButtonDown = DgdsEngine::getInstance()->getScene()->isRButtonDown();
	_leftButtonDown = DgdsEngine::getInstance()->getScene()->isLButtonDown();

	uint16 btn = _lastKeycode;
	if (btn == 0 && _leftButtonDown) {
		btn = readButtons();
	}

	if (btn == _lastBtn) {
		btn = 0;
	} else {
		_lastBtn = btn;
	}

	switch (btn) {
	case 0x1e: // ATTACK (A)
		if (_players._lucky._intent == kIntentAttack || _players._lucky._intent == kIntentPursue) {
			_players._lucky._ferocity++;
			if (6 < _players._lucky._ferocity) {
				_players._lucky._ferocity = 6;
			}
		} else {
			_players._lucky._ferocity = 1;
			_players._lucky._intent = kIntentAttack;
		}
		break;
	case 0x10: // (Q)
		_players._tong._intent = kIntentQ;
		break;
	case 0x13: // RETREAT (R)
		if (!_players._lucky.isJumping()) {
			_players._lucky._intent = kIntentRetreat;
			int16 newVal7 = _players._lucky._xpos;
			if (_players._tong._xpos < newVal7)
				newVal7 = newVal7 + 0x90;
			else
				newVal7 = _players._lucky._xpos - 0x90;

			_players._lucky._val7 = newVal7;

			if (_currentCar == 1) {
				if (_players._lucky._xpos < 0xad)
					_players._lucky._val7 = 0xac;
				else
					_players._lucky._val7 = _players._lucky._xpos;

			} else if (_currentCar == 4 && (_players._lucky._xpos < _players._tong._xpos)) {
				_players._lucky._val7 = 0x8e;
			}

			if (checkGap(_players._lucky._val7, 0) != 0) {
				_players._lucky._val7 = 0xa0;
			}
		}
		break;
	case 0x1f: // REST (S)
		_players._lucky._intent = kIntentRest;
		break;
	case 0x20: // DUCK (D)
		_players._lucky._intent = kIntentDuck;
		break;
	default:
		break;
	}

	PlayerAction newAction;
	if (!_rightButtonDown || _players._lucky.isJumping() || _players._lucky.isFalling()) {
		if (!_players._lucky.isDucking())
			return;

		// Stand up if there is no longer a signal to duck.
		if (_players._tong._xpos < _players._lucky._xpos)
			newAction = kActionStandLeft;
		else
			newAction = kActionStandRight;
	} else {
		if (_players._lucky._data->_flipMode == kImageFlipNone)
			newAction = kActionDuckRight;
		else
			newAction = kActionDuckLeft;
	}
	_players._lucky.setAction(newAction, true);
}

void ChinaTrain::getNpcInput() {
	_players._tong.computerDucks();
	if (DgdsEngine::getInstance()->getDifficulty() == 0)
		_players._lucky.computerDucks();

	if (_players._tong.isStanding()) {
		if ( _players._lucky.isFalling() && !_players._tong.isDucking()) {
			_players._tong._intent = kIntentRest;
			if (_players._tong._xpos < _players._lucky._xpos)
				_players._tong.setAction(kActionStandRight, true);
			else
				_players._tong.setAction(kActionStandLeft, true);
		}
		if (_players._tong._intent == kIntentAttack || _players._tong._intent == kIntentPursue) {
			_tongAttackCounter++;
		} else {
			_tongAttackCounter = 0;
		}

		if (20 < _tongAttackCounter) {
			_tongAttackCounter = 0;
			if (_players._tong.inRange(_players._lucky)) {
				_players._tong._ferocity++;
				if (_players._tong._ferocity > 6)
					_players._tong._ferocity = 6;
			}
		}

		if (_players._tong._hitpoints == _lastTongHP) {
			_tongInjuredCounter--;
			if (_tongInjuredCounter < 0)
				_tongInjuredCounter = 0;
		} else {
			_tongInjuredCounter++;
		}

		if (5 < _tongInjuredCounter && _currentCar != 4) {
			_players._tong._intent = kIntentRetreat;
			_players._tong._ferocity = 1;
			_tongInjuredCounter = 0;
		}

		if (_players._tong._intent == kIntentRest && _players._tong._fatigue == _lastTongFatigue) {
			_tongRestTarget = _players._tong._fatigue - 20;
			if (_tongRestTarget < 0)
				_tongRestTarget = 0;
		}

		if (_players._tong._intent == kIntentRest && _players._tong._fatigue == _tongRestTarget) {
			_players._tong._intent = kIntentAttack;
		}

		if (0x4b < _players._tong._fatigue) {
			_players._tong._ferocity--;
			if (_players._tong._ferocity < 1)
				_players._tong._ferocity = 1;
		}

		_lastTongHP = _players._tong._hitpoints;
		_lastTongFatigue = _players._tong._fatigue;
	}
}

void ChinaTrain::handleVariables() {
	_frameCnt++;
	_trackPos++;
	_xOffset = _trackPos % 40;

	_currentTunnel._start -= 2;
	_currentTunnel._end -= 2;

	// TODO: Simplify this logic
	if (_jumpOffset < 1 || (_jumpOffset += 16, _jumpOffset < 0xb1)) {
		if (_jumpOffset < 0 && (_jumpOffset -= 16, _jumpOffset < -0xb0)) {
			_jumpOffset = 0;
			_currentCar++;
		}
	} else {
		_jumpOffset = 0;
		_currentCar--;
	}

	if (_cabooseTrail) {
		_cabooseTrail += 2;
		if (_cabooseTrail > 0x40)
			cabooseLost();
	}

	if (_currentTunnel._start < 0x32 && _playedTunnelSFX == 0) {
		DgdsEngine::getInstance()->_soundPlayer->playSFX(0x85);
		_playedTunnelSFX = 1;
	}

	if (_currentTunnel._end < -12) {
		makeNewTunnel();
		_playedTunnelSFX = 0;
	}
}

void ChinaTrain::cabooseLost() {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	assert(globals);
	globals->setTrainState(4);
	leaveArcade();
}


void ChinaTrain::fixUpTunnel() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	if (_currentTunnel._start < 0x25 && -12 < _currentTunnel._start)
		_test->drawBitmap(4, _currentTunnel._start * 8 + 0x18, 0x5e, _clipWin, compBuf, kImageFlipNone);

	if (_currentTunnel._end < 0x1e)
		_test->drawBitmap(4, _currentTunnel._end * 8 + 0x47, 0x5e, _clipWin, compBuf, kImageFlipH);
}

void ChinaTrain::drawFrame() {
	//setupArcade();
	if (2 < _currentTunnel._start)
		drawMountains(1);

	if (_currentTunnel._end < 0x1a)
		drawMountains(_currentTunnel._end + 13);

	if (_currentTunnel._start < 40)
		drawTunnel();

	fixBorder();
	drawScoreWindow();
	drawCommandButtons();
	drawBmps();
	//fullScreen(); // set clip window to full screen
}

void ChinaTrain::shadeBox(Graphics::ManagedSurface &buf, byte tlCol, byte brCol, byte fill, int16 x, int16 y, int16 w, int16 h) {
	// Button background
	buf.fillRect(Common::Rect(Common::Point(x + 1, y + 1), w - 2, h - 2), fill);
	// Button top/right color
	buf.vLine(x + w - 1, y, y + h - 1, tlCol);
	buf.hLine(x, y, x + w - 1, tlCol);
	// Button bottom/left color
	buf.vLine(x, y + 1, y + h - 1, brCol);
	buf.hLine(x, y + h - 1, x + w - 1, brCol);
}

void ChinaTrain::shadeLabel(int16 x, int16 y, int16 w, int16 h, const char *label) {
	Graphics::ManagedSurface &buf = DgdsEngine::getInstance()->getBackgroundBuffer();
	shadeBox(buf, 0x10, 0x14, 0x16, x, y, w, h);
	const DgdsFont *font = DgdsEngine::getInstance()->getFontMan()->getFont(FontManager::kGameFont);
	int16 len = font->getStringWidth(label);
	font->drawString(&buf, label, x + (w / 2) - (len / 2), y + 2, len, 0x1a);
}

void ChinaTrain::drawCommandButtons() {
	// The original has code to draw strings in the buttons, but in practice that was deleted
	// so we can just do the simpler thing.
	for (int i = 0; i < ARRAYSIZE(COMMAND_BUTTONS); i++) {
		const int16 *btn = COMMAND_BUTTONS[i];
		if (_pressedCommandButton == i)
			shadePressButton(btn[0], btn[1]);
		else
			shadeButton(btn[0], btn[1]);
	}
	_pressedCommandButton = -1;
}

void ChinaTrain::shadePressButton(int16 x, int16 y) {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	shadeBox(compBuf, 16, 20, 0, x, y, 12, 10);
	shadeBox(compBuf, 20, 17, 0, x + 1, y + 1, 10, 8);
	shadeBox(compBuf, 6, 16, 24, x + 2, y + 2, 8, 6);
}

void ChinaTrain::shadeButton(int16 x, int16 y) {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	shadeBox(compBuf, 16, 20, 0, x, y, 12, 10);
	shadeBox(compBuf, 17, 6, 23, x + 1, y + 1, 10, 8);
}

void ChinaTrain::drawScoreWindow() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	shadeBox(compBuf, 17, 6, 16, 0x18, 0x90, 0x41, 8);
	shadeBox(compBuf, 6, 17, 13, 0x1b, 0x92, 0x3c, 4);
	shadeBox(compBuf, 17, 6, 16, 0x5d, 0x90, 0x41, 8);
	shadeBox(compBuf, 6, 17, 13, 0x60, 0x92, 0x3c, 4);
	shadeBox(compBuf, 17, 6, 16, 0x3a, 0xa3, 0x41, 8);
	shadeBox(compBuf, 6, 17, 13, 0x3d, 0xa5, 0x3c, 4);
	drawScore();
}

void ChinaTrain::drawScore() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;

	if (1 < _players._lucky._hitpoints) {
		int16 width = _players._lucky._hitpoints / 2;
		if (25 < width) {
			width += 8;
		}

		compBuf.fillRect(Common::Rect(Common::Point(0x1c, 0x93), width, 2), 10);
	}

	if (1 < _players._lucky._fatigue) {
		int16 width = _players._lucky._fatigue / 2;
		if (30 < width)
			width += 8;

		if (58 < width)
			width = 58;

		compBuf.fillRect(Common::Rect(Common::Point(0x61, 0x93), width, 2), 12);
	}

	if (0 < _players._lucky._ferocity) {
		int16 width = _players._lucky._ferocity * 10;
		if (58 < width)
			width = 58;

		compBuf.fillRect(Common::Rect(Common::Point(0x3e, 0xa6), width, 2), 0xe);
	}
}

bool ChinaTrain::calcBounce(int16 car) {
	return (_xOffset % 10 < car * 2 + 5 && car * 2 < _xOffset % 10);
}

void ChinaTrain::drawBmps() {
	drawSnow();
	drawTrain();
	drawActors();
	fixUpTunnel();
}

void ChinaTrain::drawActors() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	int16 bounce = calcBounce(_currentCar);

	const PlayerData *luckyData = _players._lucky._data;
	_luckyMaps->drawBitmap(luckyData->_frame, _players._lucky._xpos + luckyData->_xoff,
			   _players._lucky._ypos + bounce + luckyData->_yoff, _clipWin, compBuf, luckyData->_flipMode);

	if (_players._tong._xpos < 0x5e)
		bounce = calcBounce(_currentCar + 1);
	else if (_players._tong._xpos >= 0xdd)
		bounce = calcBounce(_currentCar - 1);


	const PlayerData *tongData = _players._tong._data;
	_luckyMaps->drawBitmap(tongData->_frame, _players._tong._xpos + tongData->_xoff,
			   _players._tong._ypos + bounce + tongData->_yoff, _clipWin, compBuf, tongData->_flipMode);

	_players._lucky.checkDuck(_currentTunnel);
	_players._tong.checkDuck(_currentTunnel);
}

void ChinaTrain::drawTrain() {
	int16 xoff = -268;
	for (int carNum = _currentCar + 2; carNum > -1; carNum--) {
		int16 yoff = calcBounce(carNum);
		if (carNum < 5 && TRAIN[carNum] != -1)
			drawCar(xoff, TRAIN[carNum], yoff);

		xoff += 176;
	}
}

void ChinaTrain::drawCar(int16 xoff, int16 frame, int16 yoff) {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	int16 cabooseX = 0;
	if (_cabooseTrail) {
		if (TRAIN[_currentCar] == 1 || frame != 1) {
			if (TRAIN[_currentCar] == 1 && frame != 1)
				cabooseX = _cabooseTrail;
		} else {
			cabooseX = -_cabooseTrail;
		}
	}

	if (frame == 1)
		yoff -= 5;

	_test->drawBitmap(frame, xoff - _jumpOffset + cabooseX, yoff + 117, _clipWin, compBuf);
}

void ChinaTrain::drawSnow() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	if (_currentTunnel._start < 39 && _currentTunnel._start > 0)
		_test->drawBitmap(0, _currentTunnel._start * 8 - 32, 84, _clipWin, compBuf, kImageFlipNone);

	if (_currentTunnel._end < 29)
		_test->drawBitmap(0, _currentTunnel._end * 8 + 0x68, 84, _clipWin, compBuf, kImageFlipH);
}

void ChinaTrain::fixBorder() {
	Graphics::ManagedSurface &compBuf = DgdsEngine::getInstance()->_compositionBuffer;
	const Common::Rect screenRect(SCREEN_WIDTH, SCREEN_HEIGHT);
	_rectShape->drawBitmap(371, 0, 65, screenRect, compBuf);
	_rectShape->drawBitmap(371, 312, 65, screenRect, compBuf);
}

void ChinaTrain::processInput() {
	processOrders(_players._tong, _players._lucky);
	processOrders(_players._lucky, _players._tong);
	checkRegions(_players._lucky);
	_players._lucky.doProcess();
	checkRegions(_players._tong);
	_players._tong.doProcess();
	_players.doScroll(_jumpOffset);
}

void ChinaTrain::checkRegions(TrainPlayer &player) {
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (checkGap(player._xpos, 0)) {
		int16 nextX = player._xpos + _jumpOffset;
		int16 mode;
		if (nextX < 68)
			mode = 0;
		else if (nextX < 160)
			mode = 1;
		else if (nextX < 244)
			mode = 2;
		else
			mode = 3;

		if (player._action == kActionWalkRight) {
			if (mode == 0 || (mode == 2 && _cabooseTrail == 0)) {
				player.setAction(kActionJumpRight, true);
				engine->_soundPlayer->playSFX(134);
			}
			//
			// The original also has this code, but it can never execute because
			// mode == 0 would have taken the above branch.
			// There's also no code to handle kActionHeroicJump in doProcess,
			// so it was probably vestigal.
			//
			// } else if (mode == 0 && _cabooseTrail != 0) {
			//    player.setAction(kActionHeroicJump, true);
			//    engine->_soundPlayer->playSFX(134);
			// }
		} else if (player._action == kActionWalkLeft) {
			if ((mode == 1 || mode == 3) && (_players._tong._intent != 5 || _currentCar != 3 || &player == &_players._tong)) {
				player.setAction(kActionJumpLeft, true);
				engine->_soundPlayer->playSFX(134);
				player._xpos = 95;
			} else {
				player.setAction(kActionStandLeft, true);
			}
		} else if (player._action == kActionStagger) {
			if (&player == &_players._tong || engine->getDifficulty() == 0) {
				if (mode == 1) {
					player.setAction(kActionJumpLeft, true);
					engine->_soundPlayer->playSFX(134);
					player._xpos = 95;
				} else if (_currentCar == 1) {
					player.setAction(kActionFallRight, true);
					engine->_soundPlayer->playSFX(131);
				} else {
					player.setAction(kActionJumpRight, true);
					engine->_soundPlayer->playSFX(134);
				}
			} else if (mode == 1) {
				player.setAction(kActionFallLeft, true);
				engine->_soundPlayer->playSFX(131);
			} else {
				player.setAction(kActionFallRight, true);
				engine->_soundPlayer->playSFX(131);
			}
		}
	}
}

void ChinaTrain::processOrders(TrainPlayer &player, TrainPlayer &enemy) {
	if (!player.isStanding())
		return;

	switch (player._intent) {
	case kIntentDuck:
		if (_players._lucky._data->_flipMode == kImageFlipNone)
			_players._lucky.setAction(kActionDuckRight, true);
		else
			_players._lucky.setAction(kActionDuckLeft, true);

		break;
	case kIntentRest:
		if (player._xpos < enemy._xpos)
			player.setAction(kActionStandRight, true);
		else
			player.setAction(kActionStandLeft, true);

		player._ferocity = 1;
		if (player._fatigue)
			player._fatigue--;

		break;
	case kIntentPursue:
		if (!player.inRange(enemy)) {
			if (enemy._intent == kIntentPursue && enemy.isJumping()) {
				if (player._data->_flipMode == kImageFlipNone)
					player.setAction(kActionStandRight, true);
				else
					player.setAction(kActionStandLeft, true);
			} else {
				player.doPursue(enemy);
			}
		} else {
			player._intent = kIntentAttack;
		}
		break;
	case kIntentAttack:
		if (!player.inRange(enemy))
			player._intent = kIntentPursue;
		else
			player.doAttack(enemy);
		break;
	case kIntentRetreat:
		if (abs(player._xpos - player._val7) < 5)
			player._intent = kIntentRest;
		else
			player.doRun();
		break;
	case kIntentQ:
		if ((160 - _jumpOffset) - (4 - _currentCar) * 160 < player._xpos) {
			player.setAction(kActionWalkLeft, false);
		} else {
			player.setAction(kActionStandRight, true);
			if (_cabooseTrail == 0)
				_cabooseTrail = 1;
			else if (_currentCar != 4)
				_players._tong._xpos--;
		}
		break;
	default:
		error("Unexpected player intent value %d", (int)player._intent);
	}
}


void ChinaTrain::onKeyDown(Common::KeyState kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_a: _lastKeycode = 0x1e; break;
	case Common::KEYCODE_q: _lastKeycode = 0x10; break;
	case Common::KEYCODE_r: _lastKeycode = 0x13; break;
	case Common::KEYCODE_s: _lastKeycode = 0x1f; break;
	case Common::KEYCODE_d: _lastKeycode = 0x20; break;
	default: break;
	}
}


void ChinaTrain::onKeyUp(Common::KeyState kbd) {
	byte code_to_clear = 0;
	switch (kbd.keycode) {
	case Common::KEYCODE_a: code_to_clear = 0x1e; break;
	case Common::KEYCODE_q: code_to_clear = 0x10; break;
	case Common::KEYCODE_r: code_to_clear = 0x13; break;
	case Common::KEYCODE_s: code_to_clear = 0x1f; break;
	case Common::KEYCODE_d: code_to_clear = 0x20; break;
	default: break;
	}
	if (_lastKeycode == code_to_clear)
		_lastKeycode = 0;
}



} // end namespace Dgds
