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

#include "common/debug.h"
#include "common/memstream.h"
#include "common/savefile.h"

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/qdMath.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/HoldData.h"
#include "qdengine/system/input/keyboard_input.h"
#include "qdengine/minigames/adv/TextManager.h"
#include "qdengine/minigames/adv/EventManager.h"
#include "qdengine/minigames/adv/EffectManager.h"
#include "qdengine/minigames/adv/MinigameInterface.h"

namespace QDEngine {

MinigameManager *g_runtime = 0;

class TimeManager {
	enum Direction {
		UP,
		LEFT,
		RIGHT,
		DOWN
	};
public:
	TimeManager(HoldData<TimeManagerData> &data, MinigameManager *runtime);
	~TimeManager();

	bool timeIsOut() const;
	float leftTime() const;
	float timeCost() const {
		return timeCost_;
	}

	void quant(float dt);

private:
	float _gameTime;
	float timeCost_;
	int lastEventTime_;
	mgVect3f startPos_;
	mgVect2f size_;
	Direction direction_;
	QDObject timeBar_;

	MinigameManager *_runtime;
};

MinigameManager::MinigameManager(MinigameConsCallback callback)
	: _currentGameIndex(-1, -1) {
	_state_container_name = Common::String::format("%s.min", g_engine->getTargetName().c_str());

	_engine = 0;
	_scene = 0;

	_timeManager = 0;
	_textManager = 0;
	_eventManager = 0;
	_effectManager = 0;
	_state_flag = 0;
	_pause_flag = 0;
	_complete_help = 0;
	_complete_help_miniature = 0;
	_game_help = 0;
	_game_help_trigger = 0;
	_game_help_enabled = true;
	_game = 0;
	_gameTime = 0;

	_currentGameInfo = 0;

	_invertMouseButtons = false;
	_debugMode = false;
	_seed = 0;

	for (int idx = 0; idx < 256; ++idx)
		_lastKeyChecked[idx] = false;

	_callback = callback;
}

MinigameManager::~MinigameManager() {
	assert(!_engine && !_scene);

	for (auto &it : _gameInfos) {
		debugC(5, kDebugMinigames, "~MinigameManager(): free: (%d,%d)", it._key._gameLevel, it._key._gameNum);
		it._value.free();
	}
}

bool MinigameManager::init(const qdEngineInterface *engine_interface) {
	debugC(1, kDebugMinigames, "MinigameManager::init(): init game");

	if (g_runtime != this)
		warning("MinigameManager::init(): Attempt to instantiate double minigame");

	if (g_runtime != this)
		return false;
	assert(!_engine && !_scene);

	assert(engine_interface);
	if (!engine_interface)
		return false;

	_engine = engine_interface;
	_scene = _engine->current_scene_interface();

	assert(_scene);
	if (!_scene) {
		_engine = 0;
		return false;
	}

	if (!createGame()) {
		warning("MinigameManager::init(): Game could not be initialized");
		finit();
		return false;
	}

	saveState();

	return true;
}

bool MinigameManager::createGame() {
	assert(_engine && _scene);
	assert(!_game);

	_screenSize = _engine->screen_size();

#ifdef _DEBUG
	_debugMode = getParameter("debug_mode", false);
#endif

	_seed = 0;

	if (!loadState())
		return false;

	if (_currentGameInfo) {
		debugC(2, kDebugMinigames, "MinigameManager::createGame(): level: %d, game: %d, index: %d", _currentGameIndex._gameLevel, _currentGameIndex._gameNum, _currentGameInfo->_game._sequenceIndex);
		debugC(2, kDebugMinigames, "MinigameManager::createGame(): %s", _currentGameInfo->_game._sequenceIndex == -1 ? "FIRST TIME PLAY" : "RePlay game");
	}

	int s = getParameter("random_seed", -1);
	_seed = _debugMode ? 0 : (s >= 0 ? s : _seed);

	_engine->rnd_init(_seed);
	debugC(2, kDebugMinigames, "MinigameManager::createGame(): seed = %d", _seed);

	_invertMouseButtons = getParameter("invert_mouse_buttons", false);
	_mouseAdjast = getParameter("ajast_mouse", mgVect2f());

	HoldData<TimeManagerData> timeData(_currentGameInfo ? &_currentGameInfo->_timeManagerData : 0, !_currentGameInfo || _currentGameInfo->_empty);
	_timeManager = new TimeManager(timeData, this);

	_textManager = new TextManager(this);

	_eventManager = new EventManager(this);

	HoldData<EffectManagerData> effectData(_currentGameInfo ? &_currentGameInfo->_effectManagerData : 0, !_currentGameInfo || _currentGameInfo->_empty);
	_effectManager = new EffectManager(effectData, this);

	const char *stateFlagName = parameter("_state_flagname", "state_flag");

	if ((_state_flag = _scene->object_interface(stateFlagName))) {
		if (!_state_flag->has_state("game") || !_state_flag->has_state("win") || !_state_flag->has_state("lose")) {
			warning("MinigameManager::createGame(): The object %s must have state: game, win, lose", transCyrillic(stateFlagName));
			return false;
		}
	} else {
		warning("MinigameManager::createGame(): Object '%s' for state flag is missing", transCyrillic(stateFlagName));
		return false;
	}

	const char *pauseFlagName = parameter("_pause_flagname", "BackHelp");

	if ((_pause_flag = _scene->object_interface(pauseFlagName))) {
		if (!_pause_flag->has_state("on")) {
			warning("MinigameManager::createGame(): The object %s must have state: on", transCyrillic(pauseFlagName));
			return false;
		}
	}

	_complete_help_state_name = "01";

	if (testObject(parameter("complete_help_miniatute", "miniature"))) {
		_complete_help_miniature = getObject(parameter("complete_help_miniatute", "miniature"));
		if ((_complete_help = getObject(parameter("complete_help", "complete")))) {
			if (!_complete_help->has_state("off") || !_complete_help->has_state("01")) {
				warning("MinigameManager::createGame(): The object for completed game must have state: off, 01");
				return false;
			}
		} else {
			warning("MinigameManager::createGame(): Object completed game is missing");
			return false;
		}
	}

	_game_help_state_name = "off";

	if (testObject(parameter("tips_object", "tips"))) {
		_game_help = getObject(parameter("tips_object", "tips"));
		_game_help.setState(_game_help_state_name.c_str());
	}
	if (testObject(parameter("tips_switcher", "tips_button"))) {
		_game_help_trigger = getObject(parameter("tips_switcher", "tips_button"));
		_game_help_trigger.setState(_game_help_enabled ? "01" : "02");
	}

	// Here we instantiate the specific game
	_game = _callback(this);

	if (_currentGameInfo)
		_currentGameInfo->_empty = false;

	if (_game && _game->state() != MinigameInterface::NOT_INITED) {
		_textManager->updateScore(_eventManager->score());
		_state_flag->set_state("game");
		return true;
	}

	return false;
}

#define SAFE_RELEASE(name)                      \
    if (name) {                                 \
        _scene->release_object_interface(name); \
        name = 0;                               \
    }

bool MinigameManager::finit() {
	debugC(2, kDebugMinigames, "MinigameManager::finit(): finit game");
	if (!_engine)
		return false;

	delete _game;
	_game = 0;

	delete _effectManager;
	_effectManager = 0;

	delete _eventManager;
	_eventManager = 0;

	delete _textManager;
	_textManager = 0;

	delete _timeManager;
	_timeManager = 0;

	if (_scene) {
		SAFE_RELEASE(_state_flag)
		SAFE_RELEASE(_pause_flag)

		release(_complete_help_miniature);
		release(_complete_help);

		release(_game_help);
		release(_game_help_trigger);
	}

	_game_help_enabled = true;

	_complete_help_state_name.clear();
	_game_help_state_name.clear();

	_completeCounters.clear();

	_currentGameInfo = 0;
	_currentGameIndex = GameInfoIndex(-1, -1);

	_gameInfos.clear();

	_seed = 0;
	_debugMode = false;
	_invertMouseButtons = false;
	_mouseAdjast = mgVect2f();

	if (_scene) {
		_engine->release_scene_interface(_scene);
		_scene = 0;
	}

	_gameTime = 0;

	_engine = 0;

	return true;
}
#undef SAFE_RELEASE


bool MinigameManager::new_game(const qdEngineInterface *engine_interface) {
	if (!loadState(false)) {
		debugC(2, kDebugMinigames, "MinigameManager::new_game(): new game skiped");
		return false;
	}
	debugC(2, kDebugMinigames, "MinigameManager::new_game(): new game");

	for (auto &it : _gameInfos) {
		debugC(3, kDebugMinigames, "MinigameManager::new_game(): clean game data (%d, %d)", it._key._gameLevel, it._key._gameNum);
		it._value._game = MinigameData();
	}

	saveState(true);
	return true;
}

class TempValue {
	const qdEngineInterface *_pre_engine;
	qdMinigameSceneInterface *_pre_scene;
	MinigameManager *_pre_runtime;
public:
	TempValue(MinigameManager *new_runtime, const qdEngineInterface *new_engine, qdMinigameSceneInterface *new_scene) {
		assert(new_runtime);
		_pre_runtime = g_runtime;
		g_runtime = new_runtime;

		assert(new_engine && new_scene);
		_pre_engine = g_runtime->_engine;
		_pre_scene = g_runtime->_scene;

		g_runtime->_engine = new_engine;
		g_runtime->_scene = new_scene;
	}
	~TempValue() {
		g_runtime->_engine = _pre_engine;
		g_runtime->_scene = _pre_scene;

		g_runtime = _pre_runtime;
	}
};

#define TEMP_scene_ENTER() TempValue tempSceneObject(this, engine, const_cast<qdMinigameSceneInterface*>(scene))

int MinigameManager::save_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, char *buffer, int buffer_size) {
	debugC(2, kDebugMinigames, "MinigameManager::save_game(): save game");
	TEMP_scene_ENTER();
	loadState();
	if (_currentGameInfo && !_currentGameInfo->empty()) {
		debugC(2, kDebugMinigames, "MinigameManager::save_game(): save game (%d, %d)", _currentGameIndex._gameLevel, _currentGameIndex._gameNum);

		Common::MemoryWriteStream out((byte *)buffer, buffer_size);
		out.writeUint32LE(GameInfo::version());
		_currentGameInfo->_game.write(out);
		return out.pos();
	}
	return 0;

}

int MinigameManager::load_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, const char *buffer, int buffer_size) {
	assert(!_game);
	if (_game) {
		debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game skiped");
		return buffer_size;
	}
	debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game");
	TEMP_scene_ENTER();
	loadState();

	if (_currentGameInfo) {
		if (buffer_size > 0) {
			debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game (%d, %d)", _currentGameIndex._gameLevel, _currentGameIndex._gameNum);
			Common::MemoryReadStream in((const byte *)buffer, buffer_size);
			int version;
			version = in.readUint32LE();
			if (version == GameInfo::version()) {
				_currentGameInfo->_game.read(in);

				if (_currentGameInfo->_empty)
					warning("MinigameManager::load_game(): Attempt to load minigame without a scene");
				if (in.pos() != buffer_size) {
					_currentGameInfo->_game = MinigameData();
					warning("MinigameManager::load_game(): Data size mismatch");
					return 0;
				}
			} else {
				warning("MinigameManager::load_game(): Incompatible savegame version for minigame");
				return 0;
			}
		} else {
			debugC(2, kDebugMinigames, "MinigameManager::load_game(): clean game (%d, %d)", _currentGameIndex._gameLevel, _currentGameIndex._gameNum);
			_currentGameInfo->_game = MinigameData();
		}
		saveState();
	}

	return buffer_size;

}

bool MinigameManager::loadState(bool current) {
	if (_game) {
		debugC(2, kDebugMinigames, "MinigameManager::loadState(): load state skiped");
		return false;
	}
	debugC(2, kDebugMinigames, "MinigameManager::loadState(): load state");
	if (current) {
		int gameNumber = getParameter("game_number", -1);
		int gameLevel = -1;
		if (gameNumber >= 0)
			if (!getParameter("game_level", gameLevel, true))
				return false;
		_currentGameIndex = GameInfoIndex(gameNumber, gameLevel);
	} else
		_currentGameIndex = GameInfoIndex(-1, -1);

	if (!current || _currentGameIndex._gameNum >= 0) {

		if (current)
			debugC(2, kDebugMinigames, "MinigameManager::loadState(): current game: (%d,%d)", _currentGameIndex._gameLevel, _currentGameIndex._gameNum);

		Common::InSaveFile *file = g_engine->getSaveFileManager()->openForLoading(_state_container_name);
		if (file) {
			int version = file->readUint32LE();

			if (version != GameInfo::version()) {
				warning("MinigameManager::loadState(): Minigame savestate version mismatch. Remove '%s'", _state_container_name.c_str());

				delete file;
				return false;
			}
			_seed = file->readUint32LE();

			GameInfoIndex index(0, 0);

			while (!file->eos()) {
				index.read(*file);

				GameInfo data;
				data.read(*file);

				debugC(2, kDebugMinigames, "MinigameManager::loadState(): read game info: (%d,%d), index: %d, game data:%d", index._gameLevel, index._gameNum, data._game._sequenceIndex, data._empty ? 0 : 1);

				if (data._game._sequenceIndex >= 0)
					_completeCounters[index._gameLevel]++;

				_gameInfos[index] = data;
			}

			delete file;
		}

		_currentGameInfo = current ? &_gameInfos[_currentGameIndex] : 0;
	}

	return true;
}

void MinigameManager::saveState(bool force) {
	debugC(2, kDebugMinigames, "MinigameManager::save_state(): save state");

	if (force || _currentGameIndex._gameNum >= 0) {
		Common::OutSaveFile *file = g_engine->getSaveFileManager()->openForSaving(_state_container_name);

		if (file) {
			file->writeUint32LE(GameInfo::version());
			file->writeUint32LE(_engine ? _engine->rnd(999999) : _seed);

			for (auto &it : _gameInfos) {
				if (!it._value.empty()) {
					debugC(2, kDebugMinigames, "MinigameManager::save_state(): write game info: (%d,%d), index: %d, game data: %d", it._key._gameLevel, it._key._gameNum, it._value._game._sequenceIndex, it._value._empty ? 0 : 1);
					it._key.write(*file);
					it._value.write(*file);
				}
			}

			file->finalize();
			delete file;
		} else {
			warning("MinigameManager::saveState(): Failed to save file '%s'", _state_container_name.c_str());
		}
	}
}

bool MinigameManager::quant(float dt) {
	if (!_game)
		return false;

	if (_pause_flag && _pause_flag->is_state_active("on"))
		return true;

	_gameTime += dt;

	mgVect2i pos = _engine->mouse_cursor_position();
	_mousePos = mgVect2f(pos.x, pos.y);
	_mousePos += _mouseAdjast;

	if (_game->state() == MinigameInterface::RUNNING) {
		_timeManager->quant(dt);

		if (_complete_help_miniature) {
			assert(_complete_help);
			if (_complete_help_miniature.hit(_mousePos))
				_complete_help.setState(_complete_help_state_name.c_str());
			else
				_complete_help.setState("off");
		}

		if (_game_help_trigger) {
			if (_game_help_trigger.hit(mousePosition())) {
				_game_help_trigger.setState(_game_help_enabled ? "01_sel" : "02_sel");
				if (mouseLeftPressed())
					_game_help_enabled = !_game_help_enabled;
			} else
				_game_help_trigger.setState(_game_help_enabled ? "01" : "02");
		}

		if (_timeManager->timeIsOut()) {
			signal(EVENT_TIME_OUT);
			_game->setState(MinigameInterface::GAME_LOST);
		} else
			_game->quant(dt);

		if (_game_help)
			_game_help.setState(_game_help_enabled ? _game_help_state_name.c_str() : "off");

#ifdef _DEBUG
		if (keyPressed(VK_MULTIPLY, true))
			_game->setState(MinigameInterface::GAME_WIN);
#endif

		switch (_game->state()) {
		case MinigameInterface::GAME_LOST:
			if (!_timeManager->timeIsOut())
				signal(EVENT_GAME_LOSE);
			// fallthrough
		case MinigameInterface::NOT_INITED:
			gameLose();
			break;

		case MinigameInterface::GAME_WIN:
			signal(EVENT_GAME_WIN);
			gameWin();
			break;

		default:
			break;
		}
	}

	for (int vKey = 0; vKey < 256; ++vKey)
		if (_lastKeyChecked[vKey])
			_lastKeyChecked[vKey] = _engine->is_key_pressed(vKey);

	if (_game->state() != MinigameInterface::NOT_INITED) {
		_textManager->quant(dt);
		_effectManager->quant(dt);
		return true;
	}

	return false;
}

void MinigameManager::setCompleteHelpVariant(int idx) {
	assert(idx >= 0);
	char buf[32];
	buf[31] = 0;
	snprintf(buf, 31, "%02d", idx + 1);
	_complete_help_state_name = buf;
}

void MinigameManager::setGameHelpVariant(int idx) {
	if (idx >= 0) {
		char buf[32];
		buf[31] = 0;
		snprintf(buf, 31, "%02d", idx + 1);
		_game_help_state_name = buf;
	} else
		_game_help_state_name = "off";
}

void MinigameManager::event(int eventID, const mgVect2f& pos, int factor) {
	_eventManager->event(eventID, pos, factor);
}

void MinigameManager::signal(SystemEvent id) {
	_eventManager->sysEvent(id);
}

const MinigameData *MinigameManager::getScore(int level, int game) const {
	GameInfoMap::const_iterator it = _gameInfos.find(GameInfoIndex(game, level));
	if (it != _gameInfos.end())
		return &it->_value._game;
	return 0;
}

bool MinigameManager::testAllGamesWin() {
	warning("STUB: MinigameManager::testAllGamesWin()");

#if 0
	XStream file(false);
	if (!file.open(gameListFileName(), XS_IN))
		return false;

	char read_buf[512];
	while (!file.eof()) {
		file.getline(read_buf, 512);
		XBuffer xbuf((void*)read_buf, strlen(read_buf));
		int level;
		xbuf >= level;
		unsigned char ch;
		xbuf > ch;
		if (ch != ':') {
			warning("MinigameManager::testAllGamesWin(): incorrect file format: '%s'", gameListFileName());
			return false;
		}
		while (xbuf.tell() < xbuf.size()) {
			xbuf > ch;
			if (Common::isDigit(ch)) {
				--xbuf;
				int game;
				xbuf >= game;
				const MinigameData *data = getScore(level, game);
				if (!data || data->_sequenceIndex == -1)
					return false;
			}
		}
	}
#endif

	return true;
}

void MinigameManager::gameWin() {
	debugC(2, kDebugMinigames, "MinigameManager::gameWin(): Game Win");
	_state_flag->set_state("win");

	if (debugMode() || !_currentGameInfo)
		return;

	assert(_currentGameIndex._gameNum >= 0);

	_effectManager->start(EFFECT_1);

	if (_currentGameIndex._gameNum == 0)
		return;

	int gameTime = round(getTime());
	_eventManager->addScore(round(_timeManager->leftTime() * _timeManager->timeCost()));

	_currentGameInfo->_game._lastTime = gameTime;
	_currentGameInfo->_game._lastScore = _eventManager->score();

	if (_currentGameInfo->_game._sequenceIndex >= 0) { // это переигровка
		if (_eventManager->score() > _currentGameInfo->_game._bestScore) {
			debugC(2, kDebugMinigames, "MinigameManager::gameWin(): new high score");
			_currentGameInfo->_game._bestScore = _eventManager->score();
			_currentGameInfo->_game._bestTime = gameTime;
		}
	} else {
		debugC(2, kDebugMinigames, "MinigameManager::gameWin(): adding score to the sum: %d", _eventManager->score());
		_currentGameInfo->_game._sequenceIndex = _completeCounters[_currentGameIndex._gameLevel];
		_currentGameInfo->_game._bestScore = _eventManager->score();
		_currentGameInfo->_game._bestTime = gameTime;
		if (QDCounter all_score = getCounter("all_score")) {
			all_score->add_value(_eventManager->score());
			if (testAllGamesWin()) {
				debugC(2, kDebugMinigames, "MinigameManager::gameWin(): All games are won, adding record to the score table: %d", all_score->value());
				_engine->add_hall_of_fame_entry(all_score->value());
			}
			release(all_score);
		}
		if (QDCounter all_time = getCounter("all_time")) {
			all_time->add_value(gameTime);
			release(all_time);
		}
	}

	saveState();
}

void MinigameManager::gameLose() {
	debugC(2, kDebugMinigames, "MinigameManager: Game Lose");
	_state_flag->set_state("lose");
}

const char *MinigameManager::parameter(const char *name, bool required) const {
	if (!_scene)
		error("MinigameManager::parameter(): Scene is undefined");

	const char *txt = _scene->minigame_parameter(name);

	if (required && !txt)
		warning("MinigameManager::parameter(): Required parameter '%s' is missing in the ini file", transCyrillic(name));

	return txt;
}

const char *MinigameManager::parameter(const char *name, const char *def) const {
	if (!def)
		warning("MinigameManager::parameter(): Default value for parameter '%s' is missing", transCyrillic(name));

	const char *txt = _scene->minigame_parameter(name);

	if (!def && !txt)
		warning("MinigameManager::parameter(): Required parameter '%s' is missing in the ini file", transCyrillic(name));

	return txt ? txt : (def ? def : "");
}

bool MinigameManager::mouseLeftPressed() const {
	if (_invertMouseButtons)
		return _engine->is_mouse_event_active(qdEngineInterface::MOUSE_EV_RIGHT_DOWN);
	return _engine->is_mouse_event_active(qdEngineInterface::MOUSE_EV_LEFT_DOWN);

}

bool MinigameManager::mouseRightPressed() const {
	if (_invertMouseButtons)
		return _engine->is_mouse_event_active(qdEngineInterface::MOUSE_EV_LEFT_DOWN);
	return _engine->is_mouse_event_active(qdEngineInterface::MOUSE_EV_RIGHT_DOWN);

}

bool MinigameManager::keyPressed(int vKey, bool once) const {
	assert(vKey >= 0 && vKey <= 255);
	if (_engine->is_key_pressed(vKey)) {
		if (once && _lastKeyChecked[vKey])
			return false;
		return _lastKeyChecked[vKey] = true;
	}
	return _lastKeyChecked[vKey] = false;
}

mgVect3f MinigameManager::game2world(const mgVect3i& coord) const {
	return _scene->screen2world_coords(reinterpret_cast<const mgVect2i &>(coord), coord.z);
}

mgVect3f MinigameManager::game2world(const mgVect3f& coord) const {
	return _scene->screen2world_coords(mgVect2i(round(coord.x), round(coord.y)), round(coord.z));
}

mgVect3f MinigameManager::game2world(const mgVect2i& coord, int depth) const {
	return _scene->screen2world_coords(coord, depth);
}

mgVect3f MinigameManager::game2world(const mgVect2f& coord, int depth) const {
	return _scene->screen2world_coords(mgVect2i(round(coord.x), round(coord.y)), depth);
}

mgVect2f MinigameManager::world2game(const mgVect3f& pos) const {
	mgVect2i scr = _scene->world2screen_coords(pos);
	return mgVect2f(scr.x, scr.y);
}

mgVect3f MinigameManager::world2game(qdMinigameObjectInterface *obj) const {
	mgVect2i scr = obj->screen_R();
	return mgVect3f(scr.x, scr.y, round(getDepth(obj)));
}

mgVect2f MinigameManager::getSize(qdMinigameObjectInterface *obj) const {
	if (obj) {
		mgVect2i size = obj->screen_size();
		return mgVect2f(size.x, size.y);
	}
	return mgVect2f();
}

void MinigameManager::setDepth(qdMinigameObjectInterface *obj, int depth) const {
	mgVect2i scr = obj->screen_R();
	obj->set_R(_scene->screen2world_coords(scr, depth));
}

float MinigameManager::getDepth(qdMinigameObjectInterface *obj) const {
	return _scene->screen_depth(obj->R());
}

float MinigameManager::getDepth(const mgVect3f& pos) const {
	return _scene->screen_depth(pos);
}

QDObject MinigameManager::getObject(const char *name) const {
	if (!name || !*name) {
		warning("MinigameManager::getObject(): null name");
		return QDObject(0, "ZERO OBJECT");
	}

	qdMinigameObjectInterface *obj = _scene->object_interface(name);
	if (!obj)
		warning("MinigameManager::getObject(): Object '%s' not found", transCyrillic(name));

	if (obj)
		return QDObject(obj, name);
	return QDObject(0, "ZERO OBJECT");
}

bool MinigameManager::testObject(const char *name) const {
	if (qdMinigameObjectInterface *obj = _scene->object_interface(name)) {
		_scene->release_object_interface(obj);
		return true;
	}
	return false;
}

void MinigameManager::release(QDObject& obj) {
	if (obj) {
		_scene->release_object_interface(obj);
		obj = 0;
	}
}

QDCounter MinigameManager::getCounter(const char *name) {
	qdMinigameCounterInterface *counter = _engine->counter_interface(name);

	if (!counter)
		warning("MinigameManager::getCounter(): Counter '%s' not found", transCyrillic(name));

	return counter;
}

void MinigameManager::release(QDCounter& counter) {
	if (!counter)
		warning("MinigameManager::release(): Null counter");

	_engine->release_counter_interface(counter);
	counter = 0;
}

void MinigameManager::setText(const char *name, const char *text) const {
	_engine->set_interface_text(0, name, text);
}

void MinigameManager::setText(const char *name, int toText, const char *format) const {
	char text[16];
	text[15] = 0;
	snprintf(text, 15, format, toText);
	setText(name, text);
}

void MinigameManager::hide(qdMinigameObjectInterface *obj) const {
	obj->set_R(_scene->screen2world_coords(mgVect2i(-10000, -10000), getDepth(obj)));
}

float MinigameManager::rnd(float min, float max) const {
	return min + _engine->fabs_rnd(max - min);
}

int MinigameManager::rnd(int min, int max) const {
	return min + round(_engine->fabs_rnd(max - min));
}

int MinigameManager::rnd(const Std::vector<float> &prob) const {
	float rnd = this->rnd(0.f, .9999f);
	float accum = 0.f;
	int idx = 0;
	int size = prob.size();
	for (; idx < size; ++idx) {
		accum += prob[idx];
		if (rnd <= accum)
			break;
	}
	assert(idx >= 0 && idx < (int)prob.size());
#ifdef _DEBUG
	float sum = 0.f;

	for (auto &pit : prob)
		sum += pit;

	assert(abs(sum - 1.f) < 0.0001f);
#endif
	return idx;
}


//========================================================================================================================


// если данные еще ни разу не сохранялись - запоминаем
// если уже есть запомненные, то заменяем на них
bool MinigameManager::processGameData(Common::SeekableReadStream &data) {
	data.seek(0);

	if (_currentGameInfo) {
		if (_currentGameInfo->_empty) {
			_currentGameInfo->_empty = false;
			assert(data.pos());
			_currentGameInfo->persist(data);
		} else {
			if (data.pos() != _currentGameInfo->_dataSize)
				warning("MinigameManager::processGameData(): Old minigame save detected. Remove '%s'", _state_container_name.c_str());

			if (data.pos() == _currentGameInfo->_dataSize) {
				_currentGameInfo->persist(data);
			} else {
				data.seek(0);
				return false;
			}
		}
	}
	data.seek(0);

	return true;
}

MinigameData::MinigameData() {
	_sequenceIndex = -1;
	_lastScore = 0;
	_lastTime = 0;
	_bestTime = 0;
	_bestScore = 0;
}

void MinigameData::write(Common::WriteStream &out) const {
	out.writeSint32LE(_sequenceIndex);
	out.writeSint32LE(_lastScore);
	out.writeSint32LE(_lastTime);
	out.writeSint32LE(_bestTime);
	out.writeSint32LE(_bestScore);
}

void MinigameData::read(Common::ReadStream &out) {
	_sequenceIndex = out.readSint32LE();
	_lastScore =     out.readSint32LE();
	_lastTime =      out.readSint32LE();
	_bestTime =      out.readSint32LE();
	_bestScore =     out.readSint32LE();
}

GameInfo::GameInfo() {
	_empty = true;
	_dataSize = 0;
	_gameData = 0;
}

void GameInfo::free() {
	if (_gameData) {
		assert(_dataSize > 0);

		::free(_gameData);
		_gameData = 0;
	}
	_dataSize = 0;
}

void GameInfo::persist(Common::SeekableReadStream &in) {
	if (_dataSize != in.size()) {
		free();
		if (in.size() > 0) {
			_dataSize = in.size();
			_gameData = malloc(_dataSize);
		}
	}
	if (_dataSize > 0)
		in.read(_gameData, _dataSize);
}

void GameInfo::write(Common::WriteStream &out) const {
	_game.write(out);
	out.writeByte(_empty);

	if (!_empty) {
		_timeManagerData.crd.write(out);
		_effectManagerData.crd.write(out);

		out.writeUint32LE(_dataSize);
		if (_dataSize > 0)
			out.write(_gameData, _dataSize);
	}
}

void GameInfo::read(Common::ReadStream &in) {
	_game.read(in);
	_empty = in.readByte();

	if (!_empty) {
		_timeManagerData.crd.read(in);
		_effectManagerData.crd.read(in);

		free();

		_dataSize = in.readUint32LE();

		if (_dataSize) {
			_gameData = malloc(_dataSize);
			in.read(_gameData, _dataSize);
		}
	}
}

void MinigameManager::GameInfoIndex::write(Common::WriteStream &out) const {
	out.writeUint32LE(_gameNum);
	out.writeUint32LE(_gameLevel);
}

void MinigameManager::GameInfoIndex::read(Common::ReadStream &in) {
	_gameNum = in.readUint32LE();
	_gameLevel = in.readUint32LE();
}

int MinigameManager::getParameter(const char* name, const int& defValue) {
	return round(getParameter(name, (float)defValue));
}

bool MinigameManager::getParameter(const char* name, int& out, bool obligatory) {
	float retValue = out;
	if (getParameter(name, retValue, obligatory)) {
		out = round(retValue);
		return true;
	}
	return false;
}

float MinigameManager::getParameter(const char* name, const float &defValue) {
	if (const char *data = parameter(name, false)) {
		float retValue = defValue;
		if (sscanf(data, "%f", &retValue) == 1)
			return retValue;
		error("The parameter [%s] contains wrong data type. It must be a number", name);
	}
	return defValue;

}

bool MinigameManager::getParameter(const char* name, float &out, bool obligatory) {
	if (const char * data = parameter(name, obligatory)) {
		float retValue = out;
		if (sscanf(data, "%f", &retValue) == 1) {
			out = retValue;
			return true;
		}
		error("The parameter [%s] contains wrong data type. It must be a number", name);
	}
	return false;

}

mgVect2f MinigameManager::getParameter(const char* name, const mgVect2f& defValue) {
	if (const char * data = parameter(name, false)) {
		mgVect2f retValue = defValue;
		if (sscanf(data, "%f %f", &retValue.x, &retValue.y) == 2)
			return retValue;
		error("The parameter [%s] contains wrong data type. It must be a pair of numbers", name);
	}
	return defValue;

}

bool MinigameManager::getParameter(const char* name, mgVect2f& out, bool obligatory) {
	if (const char * data = parameter(name, obligatory)) {
		mgVect2f retValue = out;
		if (sscanf(data, "%f %f", &retValue.x, &retValue.y) == 2) {
			out = retValue;
			return true;
		}
		error("The parameter [%s] contains wrong data type. It must be a pair of numbers", name);
	}
	return false;

}

mgVect2i MinigameManager::getParameter(const char* name, const mgVect2i& defValue) {
	mgVect2f retValue = getParameter(name, mgVect2f(defValue.x, defValue.y));
	return mgVect2i(round(retValue.x), round(retValue.y));

}

bool MinigameManager::getParameter(const char* name, mgVect2i& out, bool obligatory) {
	mgVect2f retValue = mgVect2f(out.x, out.y);
	if (getParameter(name, retValue, obligatory)) {
		out = mgVect2i(round(retValue.x), round(retValue.y));
		return true;
	}
	return false;
}

//========================================================================================================================


TimeManager::TimeManager(HoldData<TimeManagerData> &data_, MinigameManager *runtime) {
	_runtime = runtime;

	if (const char *data = _runtime->parameter("game_time", false)) {
		if (sscanf(data, "%f", &_gameTime) != 1)
			_gameTime = -1.f;
	} else
		_gameTime = -1.f;

	timeCost_ = 0.f;

	if (_gameTime > 0) {
		if (const char *data = _runtime->parameter("time_bar"))
			timeBar_ = _runtime->getObject(data);

		if (const char *data = _runtime->parameter("time_cost"))
			sscanf(data, "%f", &timeCost_);
	}

	direction_ = DOWN; // Default value

	if (timeBar_) {
		TimeManagerData myData;
		myData.crd = _runtime->world2game(timeBar_);

		data_.process(myData);

		startPos_ = myData.crd;
		size_ = _runtime->getSize(timeBar_);

		if (const char *data = _runtime->parameter("time_bar_direction")) {
			int dir;
			if (sscanf(data, "%d", &dir) == 1) {
				assert(dir >= 0 && dir <= 3);
				direction_ = Direction(dir);
			}
		}
	} else
		size_ = mgVect2f(-1.f, -1.f);

	assert(_runtime->getTime() == 0.f);

	lastEventTime_ = 0;
}

TimeManager::~TimeManager() {
	if (timeBar_)
		_runtime->release(timeBar_);

}

bool TimeManager::timeIsOut() const {
	if (_gameTime > 0.f)
		return _runtime->getTime() > _gameTime;
	return false;

}

float TimeManager::leftTime() const {
	if (_gameTime <= 0.f)
		return 0;
	return _runtime->getTime() > _gameTime ? 0 : _gameTime - _runtime->getTime();

}

void TimeManager::quant(float dt) {
	int seconds = round(_runtime->getTime());
	if (seconds != lastEventTime_) {
		lastEventTime_ = seconds;
		_runtime->textManager().updateTime(seconds);
		int amountSeconds = round(leftTime());
		if (_gameTime < 0.f || amountSeconds > 10)
			if (seconds % 60 == 0)
				_runtime->signal(EVENT_TIME_60_SECOND_TICK);
			else if (seconds % 10 == 0)
				_runtime->signal(EVENT_TIME_10_SECOND_TICK);
			else
				_runtime->signal(EVENT_TIME_1_SECOND_TICK);
		else if (amountSeconds == 10)
			_runtime->signal(EVENT_TIME_10_SECOND_LEFT);
		else
			_runtime->signal(EVENT_TIME_LESS_10_SECOND_LEFT_SECOND_TICK);
	}

	if (_gameTime <= 0.f || !timeBar_)
		return;

	float phase = clamp(_runtime->getTime() / _gameTime, 0.f, 1.f);
	mgVect3f pos;
	switch (direction_) {
	case UP:
		pos.y = -size_.y * phase;
		break;
	case DOWN:
		pos.y = size_.y * phase;
		break;
	case LEFT:
		pos.x = -size_.x * phase;
		break;
	case RIGHT:
		pos.x = size_.x * phase;
		break;
	}

	pos += startPos_;

	timeBar_->set_R(_runtime->game2world(pos));
}

} // namespace QDEngine
