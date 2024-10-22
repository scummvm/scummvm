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
	TimeManager(HoldData<TimeManagerData> &data);
	~TimeManager();

	bool timeIsOut() const;
	float leftTime() const;
	float timeCost() const {
		return timeCost_;
	}

	void quant(float dt);

private:
	float gameTime_;
	float timeCost_;
	int lastEventTime_;
	mgVect3f startPos_;
	mgVect2f size_;
	Direction direction_;
	QDObject timeBar_;
};

MinigameManager::MinigameManager(MinigameConsCallback callback)
	: currentGameIndex_(-1, -1) {
	state_container_name_ = Common::String::format("%s.min", g_engine->getTargetName().c_str());

	engine_ = 0;
	scene_ = 0;

	timeManager_ = 0;
	textManager_ = 0;
	eventManager_ = 0;
	effectManager_ = 0;
	state_flag_ = 0;
	pause_flag_ = 0;
	complete_help_ = 0;
	complete_help_miniature_ = 0;
	game_help_ = 0;
	game_help_trigger_ = 0;
	game_help_enabled_ = true;
	game_ = 0;
	gameTime_ = 0;

	currentGameInfo_ = 0;

	invertMouseButtons_ = false;
	debugMode_ = false;
	seed_ = 0;

	for (int idx = 0; idx < 256; ++idx)
		lastKeyChecked_[idx] = false;

	_callback = callback;
}

MinigameManager::~MinigameManager() {
	assert(!engine_ && !scene_);

	for (auto &it : gameInfos_) {
		debugC(5, kDebugMinigames, "~MinigameManager(): free: (%d,%d)", it._key.gameLevel_, it._key.gameNum_);
		it._value.free();
	}
}

bool MinigameManager::init(const qdEngineInterface *engine_interface) {
	debugC(1, kDebugMinigames, "MinigameManager::init(): init game");

	if (g_runtime != this)
		warning("MinigameManager::init(): Attempt to instantiate double minigame");

	if (g_runtime != this)
		return false;
	assert(!engine_ && !scene_);

	assert(engine_interface);
	if (!engine_interface)
		return false;

	engine_ = engine_interface;
	scene_ = engine_->current_scene_interface();

	assert(scene_);
	if (!scene_) {
		engine_ = 0;
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
	assert(engine_ && scene_);
	assert(!game_);

	screenSize_ = engine_->screen_size();

#ifdef _DEBUG
	debugMode_ = getParameter("debug_mode", false);
#endif

	seed_ = 0;

	if (!loadState())
		return false;

	if (currentGameInfo_) {
		debugC(2, kDebugMinigames, "MinigameManager::createGame(): level: %d, game: %d, index: %d", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_, currentGameInfo_->game_.sequenceIndex_);
		debugC(2, kDebugMinigames, "MinigameManager::createGame(): %s", currentGameInfo_->game_.sequenceIndex_ == -1 ? "FIRST TIME PLAY" : "RePlay game");
	}

	int s = getParameter("random_seed", -1);
	seed_ = debugMode_ ? 0 : (s >= 0 ? s : seed_);

	engine_->rnd_init(seed_);
	debugC(2, kDebugMinigames, "MinigameManager::createGame(): seed = %d", seed_);

	invertMouseButtons_ = getParameter("invert_mouse_buttons", false);
	mouseAdjast_ = getParameter("ajast_mouse", mgVect2f());

	HoldData<TimeManagerData> timeData(currentGameInfo_ ? &currentGameInfo_->timeManagerData_ : 0, !currentGameInfo_ || currentGameInfo_->empty_);
	timeManager_ = new TimeManager(timeData);

	textManager_ = new TextManager();

	eventManager_ = new EventManager();

	HoldData<EffectManagerData> effectData(currentGameInfo_ ? &currentGameInfo_->effectManagerData_ : 0, !currentGameInfo_ || currentGameInfo_->empty_);
	effectManager_ = new EffectManager(effectData);

	const char *stateFlagName = parameter("state_flag_name", "state_flag");

	if ((state_flag_ = scene_->object_interface(stateFlagName))) {
		if (!state_flag_->has_state("game") || !state_flag_->has_state("win") || !state_flag_->has_state("lose")) {
			warning("MinigameManager::createGame(): The object %s must have state: game, win, lose", transCyrillic(stateFlagName));
			return false;
		}
	} else {
		warning("MinigameManager::createGame(): Object '%s' for state flag is missing", transCyrillic(stateFlagName));
		return false;
	}

	const char *pauseFlagName = parameter("pause_flag_name", "BackHelp");

	if ((pause_flag_ = scene_->object_interface(pauseFlagName))) {
		if (!pause_flag_->has_state("on")) {
			warning("MinigameManager::createGame(): The object %s must have state: on", transCyrillic(pauseFlagName));
			return false;
		}
	}

	complete_help_state_name_ = "01";

	if (testObject(parameter("complete_help_miniatute", "miniature"))) {
		complete_help_miniature_ = getObject(parameter("complete_help_miniatute", "miniature"));
		if ((complete_help_ = getObject(parameter("complete_help", "complete")))) {
			if (!complete_help_->has_state("off") || !complete_help_->has_state("01")) {
				warning("MinigameManager::createGame(): The object for completed game must have state: off, 01");
				return false;
			}
		} else {
			warning("MinigameManager::createGame(): Object completed game is missing");
			return false;
		}
	}

	game_help_state_name_ = "off";

	if (testObject(parameter("tips_object", "tips"))) {
		game_help_ = getObject(parameter("tips_object", "tips"));
		game_help_.setState(game_help_state_name_.c_str());
	}
	if (testObject(parameter("tips_switcher", "tips_button"))) {
		game_help_trigger_ = getObject(parameter("tips_switcher", "tips_button"));
		game_help_trigger_.setState(game_help_enabled_ ? "01" : "02");
	}

	// Here we instantiate the specific game
	game_ = _callback();

	if (currentGameInfo_)
		currentGameInfo_->empty_ = false;

	if (game_ && game_->state() != MinigameInterface::NOT_INITED) {
		textManager_->updateScore(eventManager_->score());
		state_flag_->set_state("game");
		return true;
	}

	return false;
}

#define SAFE_RELEASE(name)                      \
    if (name) {                                 \
        scene_->release_object_interface(name); \
        name = 0;                               \
    }

bool MinigameManager::finit() {
	debugC(2, kDebugMinigames, "MinigameManager::finit(): finit game");
	if (!engine_)
		return false;

	delete game_;
	game_ = 0;

	delete effectManager_;
	effectManager_ = 0;

	delete eventManager_;
	eventManager_ = 0;

	delete textManager_;
	textManager_ = 0;

	delete timeManager_;
	timeManager_ = 0;

	if (scene_) {
		SAFE_RELEASE(state_flag_)
		SAFE_RELEASE(pause_flag_)

		release(complete_help_miniature_);
		release(complete_help_);

		release(game_help_);
		release(game_help_trigger_);
	}

	game_help_enabled_ = true;

	complete_help_state_name_.clear();
	game_help_state_name_.clear();

	completeCounters_.clear();

	currentGameInfo_ = 0;
	currentGameIndex_ = GameInfoIndex(-1, -1);

	gameInfos_.clear();

	seed_ = 0;
	debugMode_ = false;
	invertMouseButtons_ = false;
	mouseAdjast_ = mgVect2f();

	if (scene_) {
		engine_->release_scene_interface(scene_);
		scene_ = 0;
	}

	gameTime_ = 0;

	engine_ = 0;

	return true;
}
#undef SAFE_RELEASE


bool MinigameManager::new_game(const qdEngineInterface *engine_interface) {
	if (!loadState(false)) {
		debugC(2, kDebugMinigames, "MinigameManager::new_game(): new game skiped");
		return false;
	}
	debugC(2, kDebugMinigames, "MinigameManager::new_game(): new game");

	for (auto &it : gameInfos_) {
		debugC(3, kDebugMinigames, "MinigameManager::new_game(): clean game data (%d, %d)", it._key.gameLevel_, it._key.gameNum_);
		it._value.game_ = MinigameData();
	}

	saveState(true);
	return true;
}

class TempValue {
	const qdEngineInterface *pre_engine_;
	qdMinigameSceneInterface *pre_scene_;
	MinigameManager *pre_runtime_;
public:
	TempValue(MinigameManager *new_runtime, const qdEngineInterface *new_engine, qdMinigameSceneInterface *new_scene) {
		assert(new_runtime);
		pre_runtime_ = g_runtime;
		g_runtime = new_runtime;

		assert(new_engine && new_scene);
		pre_engine_ = g_runtime->engine_;
		pre_scene_ = g_runtime->scene_;

		g_runtime->engine_ = new_engine;
		g_runtime->scene_ = new_scene;
	}
	~TempValue() {
		g_runtime->engine_ = pre_engine_;
		g_runtime->scene_ = pre_scene_;

		g_runtime = pre_runtime_;
	}
};

#define TEMP_SCENE_ENTER() TempValue tempSceneObject(this, engine, const_cast<qdMinigameSceneInterface*>(scene))

int MinigameManager::save_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, char *buffer, int buffer_size) {
	debugC(2, kDebugMinigames, "MinigameManager::save_game(): save game");
	TEMP_SCENE_ENTER();
	loadState();
	if (currentGameInfo_ && !currentGameInfo_->empty()) {
		debugC(2, kDebugMinigames, "MinigameManager::save_game(): save game (%d, %d)", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);

		Common::MemoryWriteStream out((byte *)buffer, buffer_size);
		out.writeUint32LE(GameInfo::version());
		currentGameInfo_->game_.write(out);
		return out.pos();
	}
	return 0;

}

int MinigameManager::load_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, const char *buffer, int buffer_size) {
	assert(!game_);
	if (game_) {
		debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game skiped");
		return buffer_size;
	}
	debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game");
	TEMP_SCENE_ENTER();
	loadState();

	if (currentGameInfo_) {
		if (buffer_size > 0) {
			debugC(2, kDebugMinigames, "MinigameManager::load_game(): load game (%d, %d)", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);
			Common::MemoryReadStream in((const byte *)buffer, buffer_size);
			int version;
			version = in.readUint32LE();
			if (version == GameInfo::version()) {
				currentGameInfo_->game_.read(in);

				if (currentGameInfo_->empty_)
					warning("MinigameManager::load_game(): Attempt to load minigame without a scene");
				if (in.pos() != buffer_size) {
					currentGameInfo_->game_ = MinigameData();
					warning("MinigameManager::load_game(): Data size mismatch");
					return 0;
				}
			} else {
				warning("MinigameManager::load_game(): Incompatible savegame version for minigame");
				return 0;
			}
		} else {
			debugC(2, kDebugMinigames, "MinigameManager::load_game(): clean game (%d, %d)", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);
			currentGameInfo_->game_ = MinigameData();
		}
		saveState();
	}

	return buffer_size;

}

bool MinigameManager::loadState(bool current) {
	if (game_) {
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
		currentGameIndex_ = GameInfoIndex(gameNumber, gameLevel);
	} else
		currentGameIndex_ = GameInfoIndex(-1, -1);

	if (!current || currentGameIndex_.gameNum_ >= 0) {

		if (current)
			debugC(2, kDebugMinigames, "MinigameManager::loadState(): current game: (%d,%d)", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);

		Common::InSaveFile *file = g_engine->getSaveFileManager()->openForLoading(state_container_name_);
		if (file) {
			int version = file->readUint32LE();

			if (version != GameInfo::version()) {
				warning("MinigameManager::loadState(): Minigame savestate version mismatch. Remove '%s'", state_container_name_.c_str());

				delete file;
				return false;
			}
			seed_ = file->readUint32LE();

			GameInfoIndex index(0, 0);

			while (!file->eos()) {
				index.read(*file);

				GameInfo data;
				data.read(*file);

				debugC(2, kDebugMinigames, "MinigameManager::loadState(): read game info: (%d,%d), index: %d, game data:%d", index.gameLevel_, index.gameNum_, data.game_.sequenceIndex_, data.empty_ ? 0 : 1);

				if (data.game_.sequenceIndex_ >= 0)
					completeCounters_[index.gameLevel_]++;

				gameInfos_[index] = data;
			}

			delete file;
		}

		currentGameInfo_ = current ? &gameInfos_[currentGameIndex_] : 0;
	}

	return true;
}

void MinigameManager::saveState(bool force) {
	debugC(2, kDebugMinigames, "MinigameManager::save_state(): save state");

	if (force || currentGameIndex_.gameNum_ >= 0) {
		Common::OutSaveFile *file = g_engine->getSaveFileManager()->openForSaving(state_container_name_);

		if (file) {
			file->writeUint32LE(GameInfo::version());
			file->writeUint32LE(engine_ ? engine_->rnd(999999) : seed_);

			for (auto &it : gameInfos_) {
				if (!it._value.empty()) {
					debugC(2, kDebugMinigames, "MinigameManager::save_state(): write game info: (%d,%d), index: %d, game data: %d", it._key.gameLevel_, it._key.gameNum_, it._value.game_.sequenceIndex_, it._value.empty_ ? 0 : 1);
					it._key.write(*file);
					it._value.write(*file);
				}
			}

			file->finalize();
			delete file;
		} else {
			warning("MinigameManager::saveState(): Failed to save file '%s'", state_container_name_.c_str());
		}
	}
}

bool MinigameManager::quant(float dt) {
	if (!game_)
		return false;

	if (pause_flag_ && pause_flag_->is_state_active("on"))
		return true;

	gameTime_ += dt;

	mgVect2i pos = engine_->mouse_cursor_position();
	mousePos_ = mgVect2f(pos.x, pos.y);
	mousePos_ += mouseAdjast_;

	if (game_->state() == MinigameInterface::RUNNING) {
		timeManager_->quant(dt);

		if (complete_help_miniature_) {
			assert(complete_help_);
			if (complete_help_miniature_.hit(mousePos_))
				complete_help_.setState(complete_help_state_name_.c_str());
			else
				complete_help_.setState("off");
		}

		if (game_help_trigger_) {
			if (game_help_trigger_.hit(mousePosition())) {
				game_help_trigger_.setState(game_help_enabled_ ? "01_sel" : "02_sel");
				if (mouseLeftPressed())
					game_help_enabled_ = !game_help_enabled_;
			} else
				game_help_trigger_.setState(game_help_enabled_ ? "01" : "02");
		}

		if (timeManager_->timeIsOut()) {
			signal(EVENT_TIME_OUT);
			game_->setState(MinigameInterface::GAME_LOST);
		} else
			game_->quant(dt);

		if (game_help_)
			game_help_.setState(game_help_enabled_ ? game_help_state_name_.c_str() : "off");

#ifdef _DEBUG
		if (keyPressed(VK_MULTIPLY, true))
			game_->setState(MinigameInterface::GAME_WIN);
#endif

		switch (game_->state()) {
		case MinigameInterface::GAME_LOST:
			if (!timeManager_->timeIsOut())
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
		if (lastKeyChecked_[vKey])
			lastKeyChecked_[vKey] = engine_->is_key_pressed(vKey);

	if (game_->state() != MinigameInterface::NOT_INITED) {
		textManager_->quant(dt);
		effectManager_->quant(dt);
		return true;
	}

	return false;
}

void MinigameManager::setCompleteHelpVariant(int idx) {
	assert(idx >= 0);
	char buf[32];
	buf[31] = 0;
	snprintf(buf, 31, "%02d", idx + 1);
	complete_help_state_name_ = buf;
}

void MinigameManager::setGameHelpVariant(int idx) {
	if (idx >= 0) {
		char buf[32];
		buf[31] = 0;
		snprintf(buf, 31, "%02d", idx + 1);
		game_help_state_name_ = buf;
	} else
		game_help_state_name_ = "off";
}

void MinigameManager::event(int eventID, const mgVect2f& pos, int factor) {
	eventManager_->event(eventID, pos, factor);
}

void MinigameManager::signal(SystemEvent id) {
	eventManager_->sysEvent(id);
}

const MinigameData *MinigameManager::getScore(int level, int game) const {
	GameInfoMap::const_iterator it = gameInfos_.find(GameInfoIndex(game, level));
	if (it != gameInfos_.end())
		return &it->_value.game_;
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
				if (!data || data->sequenceIndex_ == -1)
					return false;
			}
		}
	}
#endif

	return true;
}

void MinigameManager::gameWin() {
	debugC(2, kDebugMinigames, "MinigameManager::gameWin(): Game Win");
	state_flag_->set_state("win");

	if (debugMode() || !currentGameInfo_)
		return;

	assert(currentGameIndex_.gameNum_ >= 0);

	effectManager_->start(EFFECT_1);

	if (currentGameIndex_.gameNum_ == 0)
		return;

	int gameTime = round(getTime());
	eventManager_->addScore(round(timeManager_->leftTime() * timeManager_->timeCost()));

	currentGameInfo_->game_.lastTime_ = gameTime;
	currentGameInfo_->game_.lastScore_ = eventManager_->score();

	if (currentGameInfo_->game_.sequenceIndex_ >= 0) { // это переигровка
		if (eventManager_->score() > currentGameInfo_->game_.bestScore_) {
			debugC(2, kDebugMinigames, "MinigameManager::gameWin(): new high score");
			currentGameInfo_->game_.bestScore_ = eventManager_->score();
			currentGameInfo_->game_.bestTime_ = gameTime;
		}
	} else {
		debugC(2, kDebugMinigames, "MinigameManager::gameWin(): adding score to the sum: %d", eventManager_->score());
		currentGameInfo_->game_.sequenceIndex_ = completeCounters_[currentGameIndex_.gameLevel_];
		currentGameInfo_->game_.bestScore_ = eventManager_->score();
		currentGameInfo_->game_.bestTime_ = gameTime;
		if (QDCounter all_score = getCounter("all_score")) {
			all_score->add_value(eventManager_->score());
			if (testAllGamesWin()) {
				debugC(2, kDebugMinigames, "MinigameManager::gameWin(): All games are won, adding record to the score table: %d", all_score->value());
				engine_->add_hall_of_fame_entry(all_score->value());
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
	state_flag_->set_state("lose");
}

const char *MinigameManager::parameter(const char *name, bool required) const {
	if (!scene_)
		error("MinigameManager::parameter(): Scene is undefined");

	const char *txt = scene_->minigame_parameter(name);

	if (required && !txt)
		warning("MinigameManager::parameter(): Required parameter '%s' is missing in the ini file", transCyrillic(name));

	return txt;
}

const char *MinigameManager::parameter(const char *name, const char *def) const {
	if (!def)
		warning("MinigameManager::parameter(): Default value for parameter '%s' is missing", transCyrillic(name));

	const char *txt = scene_->minigame_parameter(name);

	if (!def && !txt)
		warning("MinigameManager::parameter(): Required parameter '%s' is missing in the ini file", transCyrillic(name));

	return txt ? txt : (def ? def : "");
}

bool MinigameManager::mouseLeftPressed() const {
	if (invertMouseButtons_)
		return engine_->is_mouse_event_active(qdEngineInterface::MOUSE_EV_RIGHT_DOWN);
	return engine_->is_mouse_event_active(qdEngineInterface::MOUSE_EV_LEFT_DOWN);

}

bool MinigameManager::mouseRightPressed() const {
	if (invertMouseButtons_)
		return engine_->is_mouse_event_active(qdEngineInterface::MOUSE_EV_LEFT_DOWN);
	return engine_->is_mouse_event_active(qdEngineInterface::MOUSE_EV_RIGHT_DOWN);

}

bool MinigameManager::keyPressed(int vKey, bool once) const {
	assert(vKey >= 0 && vKey <= 255);
	if (engine_->is_key_pressed(vKey)) {
		if (once && lastKeyChecked_[vKey])
			return false;
		return lastKeyChecked_[vKey] = true;
	}
	return lastKeyChecked_[vKey] = false;
}

mgVect3f MinigameManager::game2world(const mgVect3i& coord) const {
	return scene_->screen2world_coords(reinterpret_cast<const mgVect2i &>(coord), coord.z);
}

mgVect3f MinigameManager::game2world(const mgVect3f& coord) const {
	return scene_->screen2world_coords(mgVect2i(round(coord.x), round(coord.y)), round(coord.z));
}

mgVect3f MinigameManager::game2world(const mgVect2i& coord, int depth) const {
	return scene_->screen2world_coords(coord, depth);
}

mgVect3f MinigameManager::game2world(const mgVect2f& coord, int depth) const {
	return scene_->screen2world_coords(mgVect2i(round(coord.x), round(coord.y)), depth);
}

mgVect2f MinigameManager::world2game(const mgVect3f& pos) const {
	mgVect2i scr = scene_->world2screen_coords(pos);
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
	obj->set_R(scene_->screen2world_coords(scr, depth));
}

float MinigameManager::getDepth(qdMinigameObjectInterface *obj) const {
	return scene_->screen_depth(obj->R());
}

float MinigameManager::getDepth(const mgVect3f& pos) const {
	return scene_->screen_depth(pos);
}

QDObject MinigameManager::getObject(const char *name) const {
	if (!name || !*name) {
		warning("MinigameManager::getObject(): null name");
		return QDObject::ZERO;
	}

	qdMinigameObjectInterface *obj = scene_->object_interface(name);
	if (!obj)
		warning("MinigameManager::getObject(): Object '%s' not found", transCyrillic(name));

	if (obj)
		return QDObject(obj, name);
	return QDObject::ZERO;
}

bool MinigameManager::testObject(const char *name) const {
	if (qdMinigameObjectInterface *obj = scene_->object_interface(name)) {
		scene_->release_object_interface(obj);
		return true;
	}
	return false;
}

void MinigameManager::release(QDObject& obj) {
	if (obj) {
		scene_->release_object_interface(obj);
		obj = 0;
	}
}

QDCounter MinigameManager::getCounter(const char *name) {
	qdMinigameCounterInterface *counter = engine_->counter_interface(name);

	if (!counter)
		warning("MinigameManager::getCounter(): Counter '%s' not found", transCyrillic(name));

	return counter;
}

void MinigameManager::release(QDCounter& counter) {
	if (!counter)
		warning("MinigameManager::release(): Null counter");

	engine_->release_counter_interface(counter);
	counter = 0;
}

void MinigameManager::setText(const char *name, const char *text) const {
	engine_->set_interface_text(0, name, text);
}

void MinigameManager::setText(const char *name, int toText, const char *format) const {
	char text[16];
	text[15] = 0;
	snprintf(text, 15, format, toText);
	setText(name, text);
}

void MinigameManager::hide(qdMinigameObjectInterface *obj) const {
	obj->set_R(scene_->screen2world_coords(mgVect2i(-10000, -10000), getDepth(obj)));
}

float MinigameManager::rnd(float min, float max) const {
	return min + engine_->fabs_rnd(max - min);
}

int MinigameManager::rnd(int min, int max) const {
	return min + round(engine_->fabs_rnd(max - min));
}

int MinigameManager::rnd(const Std::vector<float> &prob) const {
	float rnd = g_runtime->rnd(0.f, .9999f);
	float accum = 0.f;
	int idx = 0;
	int size = prob.size();
	for (; idx < size; ++idx) {
		accum += prob[idx];
		if (rnd <= accum)
			break;
	}
	assert(idx >= 0 && idx < prob.size());
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

	if (currentGameInfo_) {
		if (currentGameInfo_->empty_) {
			currentGameInfo_->empty_ = false;
			assert(data.pos());
			currentGameInfo_->persist(data);
		} else {
			if (data.pos() != currentGameInfo_->dataSize_)
				warning("MinigameManager::processGameData(): Old minigame save detected. Remove '%s'", state_container_name_.c_str());

			if (data.pos() == currentGameInfo_->dataSize_) {
				currentGameInfo_->persist(data);
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
	sequenceIndex_ = -1;
	lastScore_ = 0;
	lastTime_ = 0;
	bestTime_ = 0;
	bestScore_ = 0;
}

void MinigameData::write(Common::WriteStream &out) const {
	out.writeSint32LE(sequenceIndex_);
	out.writeSint32LE(lastScore_);
	out.writeSint32LE(lastTime_);
	out.writeSint32LE(bestTime_);
	out.writeSint32LE(bestScore_);
}

void MinigameData::read(Common::ReadStream &out) {
	sequenceIndex_ = out.readSint32LE();
	lastScore_ =     out.readSint32LE();
	lastTime_ =      out.readSint32LE();
	bestTime_ =      out.readSint32LE();
	bestScore_ =     out.readSint32LE();
}

GameInfo::GameInfo() {
	empty_ = true;
	dataSize_ = 0;
	gameData_ = 0;
}

void GameInfo::free() {
	if (gameData_) {
		assert(dataSize_ > 0);

		::free(gameData_);
		gameData_ = 0;
	}
	dataSize_ = 0;
}

void GameInfo::persist(Common::SeekableReadStream &in) {
	if (dataSize_ != in.size()) {
		free();
		if (in.size() > 0) {
			dataSize_ = in.size();
			gameData_ = malloc(dataSize_);
		}
	}
	if (dataSize_ > 0)
		in.read(gameData_, dataSize_);
}

void GameInfo::write(Common::WriteStream &out) const {
	game_.write(out);
	out.writeByte(empty_);

	if (!empty_) {
		timeManagerData_.crd.write(out);
		effectManagerData_.crd.write(out);

		out.writeUint32LE(dataSize_);
		if (dataSize_ > 0)
			out.write(gameData_, dataSize_);
	}
}

void GameInfo::read(Common::ReadStream &in) {
	game_.read(in);
	empty_ = in.readByte();

	if (!empty_) {
		timeManagerData_.crd.read(in);
		effectManagerData_.crd.read(in);

		free();

		dataSize_ = in.readUint32LE();

		if (dataSize_) {
			gameData_ = malloc(dataSize_);
			in.read(gameData_, dataSize_);
		}
	}
}

void MinigameManager::GameInfoIndex::write(Common::WriteStream &out) const {
	out.writeUint32LE(gameNum_);
	out.writeUint32LE(gameLevel_);
}

void MinigameManager::GameInfoIndex::read(Common::ReadStream &in) {
	gameNum_ = in.readUint32LE();
	gameLevel_ = in.readUint32LE();
}

//========================================================================================================================


TimeManager::TimeManager(HoldData<TimeManagerData> &data_) {
	if (const char *data = g_runtime->parameter("game_time", false)) {
		if (sscanf(data, "%f", &gameTime_) != 1)
			gameTime_ = -1.f;
	} else
		gameTime_ = -1.f;

	timeCost_ = 0.f;

	if (gameTime_ > 0) {
		if (const char *data = g_runtime->parameter("time_bar"))
			timeBar_ = g_runtime->getObject(data);

		if (const char *data = g_runtime->parameter("time_cost"))
			sscanf(data, "%f", &timeCost_);
	}

	direction_ = DOWN; // Default value

	if (timeBar_) {
		TimeManagerData myData;
		myData.crd = g_runtime->world2game(timeBar_);

		data_.process(myData);

		startPos_ = myData.crd;
		size_ = g_runtime->getSize(timeBar_);

		if (const char *data = g_runtime->parameter("time_bar_direction")) {
			int dir;
			if (sscanf(data, "%d", &dir) == 1) {
				assert(dir >= 0 && dir <= 3);
				direction_ = Direction(dir);
			}
		}
	} else
		size_ = mgVect2f(-1.f, -1.f);

	assert(g_runtime->getTime() == 0.f);

	lastEventTime_ = 0;
}

TimeManager::~TimeManager() {
	if (timeBar_)
		g_runtime->release(timeBar_);

}

bool TimeManager::timeIsOut() const {
	if (gameTime_ > 0.f)
		return g_runtime->getTime() > gameTime_;
	return false;

}

float TimeManager::leftTime() const {
	if (gameTime_ <= 0.f)
		return 0;
	return g_runtime->getTime() > gameTime_ ? 0 : gameTime_ - g_runtime->getTime();

}

void TimeManager::quant(float dt) {
	int seconds = round(g_runtime->getTime());
	if (seconds != lastEventTime_) {
		lastEventTime_ = seconds;
		g_runtime->textManager().updateTime(seconds);
		int amountSeconds = round(leftTime());
		if (gameTime_ < 0.f || amountSeconds > 10)
			if (seconds % 60 == 0)
				g_runtime->signal(EVENT_TIME_60_SECOND_TICK);
			else if (seconds % 10 == 0)
				g_runtime->signal(EVENT_TIME_10_SECOND_TICK);
			else
				g_runtime->signal(EVENT_TIME_1_SECOND_TICK);
		else if (amountSeconds == 10)
			g_runtime->signal(EVENT_TIME_10_SECOND_LEFT);
		else
			g_runtime->signal(EVENT_TIME_LESS_10_SECOND_LEFT_SECOND_TICK);
	}

	if (gameTime_ <= 0.f || !timeBar_)
		return;

	float phase = clamp(g_runtime->getTime() / gameTime_, 0.f, 1.f);
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

	timeBar_->set_R(g_runtime->game2world(pos));
}

} // namespace QDEngine
