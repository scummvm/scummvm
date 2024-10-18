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

MinigameManager *runtime = 0;
// createGame() должна реализоваться непосредственно в КАЖДОМ проекте игры
MinigameInterface *createGame();

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

MinigameManager::MinigameManager()
	: currentGameIndex_(-1, -1) {
	state_container_name_ = "Saves\\minigames.dat";

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
}

MinigameManager::~MinigameManager() {
	xassert(!engine_ && !scene_);

	GameInfoMap::iterator it;
	FOR_EACH(gameInfos_, it) {
		//dprintf("free: (%d,%d)\n", it->first.gameLevel_, it->first.gameNum_);
		it->second.free();
	}
}

bool MinigameManager::init(const qdEngineInterface* engine_interface) {
	dprintf("init game\n");

	xxassert(runtime == this, "Попытка одновременного запуска дубля миниигры");
	if (runtime != this)
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
	dprintf("%s", debugMode_ ? "DEBUG MODE\n" : "");
	#endif

	seed_ = 0;

	if (!loadState())
		return false;

	if (currentGameInfo_) {
		dprintf("level: %d, game: %d, index: %d\n", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_, currentGameInfo_->game_.sequenceIndex_);
		dprintf("%s\n", currentGameInfo_->game_.sequenceIndex_ == -1 ? "FIRST TIME PLAY" : "RePlay game");
	}

	int s = getParameter("random_seed", -1);
	seed_ = debugMode_ ? 0 : (s >= 0 ? s : seed_);

	engine_->rnd_init(seed_);
	dprintf("seed = %d\n", seed_);

	invertMouseButtons_ = getParameter("invert_mouse_buttons", false);
	mouseAdjast_ = getParameter("ajast_mouse", mgVect2f());

	HoldData<TimeManagerData> timeData(currentGameInfo_ ? &currentGameInfo_->timeManagerData_ : 0, !currentGameInfo_ || currentGameInfo_->empty_);
	timeManager_ = new TimeManager(timeData);

	textManager_ = new TextManager();

	eventManager_ = new EventManager();

	HoldData<EffectManagerData> effectData(currentGameInfo_ ? &currentGameInfo_->effectManagerData_ : 0, !currentGameInfo_ || currentGameInfo_->empty_);
	effectManager_ = new EffectManager(effectData);

	const char *stateFlagName = parameter("state_flag_name", "state_flag");

	if (state_flag_ = scene_->object_interface(stateFlagName)) {
		if (!state_flag_->has_state("game") || !state_flag_->has_state("win") || !state_flag_->has_state("lose")) {
			warning("MinigameManager::createGame(): The object %s must have state: game, win, lose", transCyrillic(stateFlagName));
			return false;
		}
	} else {
		warning("MinigameManager::createGame(): Object '%s' for state flag is missing", transCyrillic(stateFlagName));
		return false;
	}

	const char *pauseFlagName = parameter("pause_flag_name", "BackHelp");

	if (pause_flag_ = scene_->object_interface(pauseFlagName)) {
		if (!pause_flag_->has_state("on")) {
			warning("MinigameManager::createGame(): The object %s must have state: on", transCyrillic(pauseFlagName));
			return false;
		}
	}

	complete_help_state_name_ = "01";

	if (testObject(parameter("complete_help_miniatute", "miniature"))) {
		complete_help_miniature_ = getObject(parameter("complete_help_miniatute", "miniature"));
		if (complete_help_ = getObject(parameter("complete_help", "complete"))) {
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

	game_ = ::createGame();

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
	if(name){                                   \
		scene_->release_object_interface(name); \
		name = 0;                               \
	}

bool MinigameManager::finit() {
	dprintf("finit game\n");
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

	SAFE_RELEASE(state_flag_)
	SAFE_RELEASE(pause_flag_)

	release(complete_help_miniature_);
	release(complete_help_);

	release(game_help_);
	release(game_help_trigger_);
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


bool MinigameManager::new_game(const qdEngineInterface* engine_interface) {
	if (!loadState(false)) {
		dprintf("new game skiped\n");
		return false;
	}
	dprintf("new game\n");

	GameInfoMap::iterator it;
	FOR_EACH(gameInfos_, it) {
		dprintf("clean game data (%d, %d)\n", it->first.gameLevel_, it->first.gameNum_);
		it->second.game_ = MinigameData();
	}

	saveState(true);
	return true;

}

class TempValue {
	const qdEngineInterface *pre_engine_;
	qdMinigameSceneInterface *pre_scene_;
	MinigameManager *pre_runtime_;
public:
	TempValue(MinigameManager* new_runtime, const qdEngineInterface* new_engine, qdMinigameSceneInterface* new_scene) {
		assert(new_runtime);
		pre_runtime_ = runtime;
		runtime = new_runtime;

		assert(new_engine && new_scene);
		pre_engine_ = runtime->engine_;
		pre_scene_ = runtime->scene_;

		runtime->engine_ = new_engine;
		runtime->scene_ = new_scene;
	}
	~TempValue() {
		runtime->engine_ = pre_engine_;
		runtime->scene_ = pre_scene_;

		runtime = pre_runtime_;
	}
};

#define TEMP_SCENE_ENTER() TempValue tempSceneObject(this, engine, const_cast<qdMinigameSceneInterface*>(scene))

int MinigameManager::save_game(const qdEngineInterface* engine, const qdMinigameSceneInterface* scene, char* buffer, int buffer_size) {
	dprintf("save game\n");
	TEMP_SCENE_ENTER();
	loadState();
	if (currentGameInfo_ && !currentGameInfo_->empty()) {
		dprintf("save game (%d, %d)\n", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);
		XBuffer out((void*)buffer, buffer_size);
		out.write(GameInfo::version());
		out.write(currentGameInfo_->game_);
		return out.tell();
	}
	return 0;

}

int MinigameManager::load_game(const qdEngineInterface* engine, const qdMinigameSceneInterface* scene, const char* buffer, int buffer_size) {
	assert(!game_);
	if (game_) {
		dprintf("load game skiped\n");
		return buffer_size;
	}
	dprintf("load game\n");
	TEMP_SCENE_ENTER();
	loadState();
	if (currentGameInfo_) {
		if (buffer_size > 0) {
			dprintf("load game (%d, %d)\n", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);
			XBuffer in((void*)buffer, buffer_size);
			int version;
			in.read(version);
			if (version == GameInfo::version()) {
				in.read(currentGameInfo_->game_);
				xxassert(!currentGameInfo_->empty_, "Загрузка данных по миниигре без данных о сцене. Рекомендуется удалить все сохранения.");
				if (in.tell() != buffer_size) {
					currentGameInfo_->game_ = MinigameData();
					xxassert(0, "Не совпадает размер данных в сохранении и в миниигре.");
					return 0;
				}
			} else {
				xxassert(0, "Несовместимая версия сохранения для миниигры.");
				return 0;
			}
		} else {
			dprintf("clean game (%d, %d)\n", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);
			currentGameInfo_->game_ = MinigameData();
		}
		saveState();
	}
	return buffer_size;

}

bool MinigameManager::loadState(bool current) {
	if (game_) {
		dprintf("load state skiped\n");
		return false;
	}
	dprintf("load state\n");
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
			dprintf("current game: (%d,%d)\n", currentGameIndex_.gameLevel_, currentGameIndex_.gameNum_);

		XStream file(false);
		if (file.open(state_container_name_, XS_IN)) {
			int version;
			file > version;
			if (version != GameInfo::version()) {
				xxassert(0, (XBuffer() < "Не совпадает версия сохранения состояния миниигры. Удалите " < state_container_name_).c_str());
				return false;
			}
			file > seed_;
			GameInfoIndex index(0, 0);
			while (!file.eof()) {
				file.read(index);
				xassert(gameInfos_.find(index) == gameInfos_.end());
				if (file.eof())
					return false;
				{
					GameInfo data;
					file > data;
					dprintf("read game info: (%d,%d), index: %d, game data:%d\n", index.gameLevel_, index.gameNum_, data.game_.sequenceIndex_, data.empty_ ? 0 : 1);
					if (data.game_.sequenceIndex_ >= 0)
						completeCounters_[index.gameLevel_]++;
					gameInfos_[index] = data;
				}
			}
		}

		currentGameInfo_ = current ? &gameInfos_[currentGameIndex_] : 0;
	}
	return true;
}

extern bool createDirForFile(const char* partialPath);
void MinigameManager::saveState(bool force) {
	dprintf("save state\n");
	if (force || currentGameIndex_.gameNum_ >= 0) {
		XStream file(false);
		if (createDirForFile(state_container_name_) && file.open(state_container_name_, XS_OUT)) {
			file < GameInfo::version();
			file < (engine_ ? engine_->rnd(999999) : seed_);
			GameInfoMap::const_iterator it;
			FOR_EACH(gameInfos_, it)
			if (!it->second.empty()) {
				dprintf("write game info: (%d,%d), index: %d, game data:%d\n", it->first.gameLevel_, it->first.gameNum_, it->second.game_.sequenceIndex_, it->second.empty_ ? 0 : 1);
				file.write(it->first);
				file < it->second;
			}
		} else {
			xxassert(0, (XBuffer() < "Не удалось сохранить прогресс в файл: \"" < state_container_name_ < "\"").c_str());
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
			xassert(complete_help_);
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
		case MinigameInterface::NOT_INITED:
			gameLose();
			break;

		case MinigameInterface::GAME_WIN:
			signal(EVENT_GAME_WIN);
			gameWin();
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
	xassert(idx >= 0);
	char buf[32];
	buf[31] = 0;
	_snprintf(buf, 31, "%02d", idx + 1);
	complete_help_state_name_ = buf;
}

void MinigameManager::setGameHelpVariant(int idx) {
	if (idx >= 0) {
		char buf[32];
		buf[31] = 0;
		_snprintf(buf, 31, "%02d", idx + 1);
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
		return &it->second.game_;
	return 0;
}

bool MinigameManager::testAllGamesWin() {
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
			xxassert(ch != ':', (XBuffer() < "Неправильный формат файла \"" < gameListFileName() < "\"").c_str());
			return false;
		}
		while (xbuf.tell() < xbuf.size()) {
			xbuf > ch;
			if (isdigit(ch)) {
				--xbuf;
				int game;
				xbuf >= game;
				const MinigameData* data = getScore(level, game);
				if (!data || data->sequenceIndex_ == -1)
					return false;
			}
		}
	}

	return true;
}

void MinigameManager::gameWin() {
	dprintf("Game Win\n");
	state_flag_->set_state("win");

	if (debugMode() || !currentGameInfo_)
		return;

	xassert(currentGameIndex_.gameNum_ >= 0);

	effectManager_->start(EFFECT_1);

	if (currentGameIndex_.gameNum_ == 0)
		return;

	int gameTime = round(getTime());
	eventManager_->addScore(round(timeManager_->leftTime() * timeManager_->timeCost()));

	currentGameInfo_->game_.lastTime_ = gameTime;
	currentGameInfo_->game_.lastScore_ = eventManager_->score();

	if (currentGameInfo_->game_.sequenceIndex_ >= 0) { // это переигровка
		if (eventManager_->score() > currentGameInfo_->game_.bestScore_) {
			dprintf("установлен новый рекорд очков.\n");
			currentGameInfo_->game_.bestScore_ = eventManager_->score();
			currentGameInfo_->game_.bestTime_ = gameTime;
		}
	} else {
		dprintf("добавляем очки к сумме прохождения: %d\n", eventManager_->score());
		currentGameInfo_->game_.sequenceIndex_ = completeCounters_[currentGameIndex_.gameLevel_];
		currentGameInfo_->game_.bestScore_ = eventManager_->score();
		currentGameInfo_->game_.bestTime_ = gameTime;
		if (QDCounter all_score = getCounter("all_score")) {
			all_score->add_value(eventManager_->score());
			if (testAllGamesWin()) {
				dprintf("Все игры пройдены, добавлена запись в таблицу рекордов: %d\n", all_score->value());
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
	dprintf("Game Lose\n");
	state_flag_->set_state("lose");
}

const char *MinigameManager::parameter(const char* name, bool required) const {
	xxassert(scene_, "Сцена не определена");
	const char *txt = scene_->minigame_parameter(name);
	xxassert(!required || txt, (XBuffer() < "Не задан обязательный параметр [" < name < "] в ini файле").c_str());
	return txt;
}

const char *MinigameManager::parameter(const char* name, const char* def) const {
	xxassert(def, (XBuffer() < "Не задано значение по умолчанию для параметра [" < name < "]").c_str());
	const char *txt = scene_->minigame_parameter(name);
	xxassert(def || txt, (XBuffer() < "Не задан обязательный параметр [" < name < "] в ini файле").c_str());
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
	xassert(vKey >= 0 && vKey <= 255);
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

mgVect3f MinigameManager::world2game(qdMinigameObjectInterface* obj) const {
	mgVect2i scr = obj->screen_R();
	return mgVect3f(scr.x, scr.y, round(getDepth(obj)));
}

mgVect2f MinigameManager::getSize(qdMinigameObjectInterface* obj) const {
	if (obj) {
		mgVect2i size = obj->screen_size();
		return mgVect2f(size.x, size.y);
	}
	return mgVect2f();
}

void MinigameManager::setDepth(qdMinigameObjectInterface* obj, int depth) const {
	mgVect2i scr = obj->screen_R();
	obj->set_R(scene_->screen2world_coords(scr, depth));
}

float MinigameManager::getDepth(qdMinigameObjectInterface* obj) const {
	return scene_->screen_depth(obj->R());
}

float MinigameManager::getDepth(const mgVect3f& pos) const {
	return scene_->screen_depth(pos);
}

QDObject MinigameManager::getObject(const char* name) const {
	xxassert(name && *name, "Нулевое имя для получение объекта");
	if (!name || !*name)
		return QDObject::ZERO;
	qdMinigameObjectInterface* obj = scene_->object_interface(name);
	xxassert(obj, (XBuffer() < "Не найден объект: \"" < name < "\"").c_str());
	if (obj)
		return QDObject(obj, name);
	return QDObject::ZERO;
}

bool MinigameManager::testObject(const char* name) const {
	if (qdMinigameObjectInterface * obj = scene_->object_interface(name)) {
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

QDCounter MinigameManager::getCounter(const char* name) {
	qdMinigameCounterInterface* counter = engine_->counter_interface(name);
	xxassert(counter, (XBuffer() < "Не найден счетчик: \"" < name < "\"").c_str());
	return counter;
}

void MinigameManager::release(QDCounter& counter) {
	xxassert(counter, "Передан нулевой счетчик для освобождения");
	engine_->release_counter_interface(counter);
	counter = 0;
}

void MinigameManager::setText(const char* name, const char* text) const {
	engine_->set_interface_text(0, name, text);
}

void MinigameManager::setText(const char* name, int toText, const char* format) const {
	char text[16];
	text[15] = 0;
	_snprintf(text, 15, format, toText);
	setText(name, text);
}

void MinigameManager::hide(qdMinigameObjectInterface* obj) const {
	obj->set_R(scene_->screen2world_coords(mgVect2i(-10000, -10000), getDepth(obj)));
}

float MinigameManager::rnd(float min, float max) const {
	return min + engine_->fabs_rnd(max - min);
}

int MinigameManager::rnd(int min, int max) const {
	return min + round(engine_->fabs_rnd(max - min));
}

int MinigameManager::rnd(const vector<float> &prob) const {
	float rnd = runtime->rnd(0.f, .9999f);
	float accum = 0.f;
	int idx = 0;
	int size = prob.size();
	for (; idx < size; ++idx) {
		accum += prob[idx];
		if (rnd <= accum)
			break;
	}
	xassert(idx >= 0 && idx < prob.size());
	#ifdef _DEBUG
	float sum = 0.f;
	vector<float>::const_iterator pit;
	FOR_EACH(prob, pit)
	sum += *pit;
	xassert(abs(sum - 1.f) < 0.0001f);
	#endif
	return idx;
}


//========================================================================================================================


// если данные еще ни разу не сохранялись - запоминаем
// если уже есть запомненные, то заменяем на них
bool MinigameManager::processGameData(XBuffer& data) {
	if (currentGameInfo_) {
		if (currentGameInfo_->empty_) {
			currentGameInfo_->empty_ = false;
			xassert(data.tell());
			currentGameInfo_->write(data.buffer(), data.tell());
		} else {
			xxassert(data.tell() == currentGameInfo_->dataSize_, (XBuffer() < "Сильно устаревшее сохранение состояния миниигры. Удалите " < state_container_name_).c_str());
			if (data.tell() == currentGameInfo_->dataSize_) {
				data.set(0);
				data.write(currentGameInfo_->gameData_, currentGameInfo_->dataSize_, true);
			} else {
				data.set(0);
				return false;
			}
		}
	}
	data.set(0);
	return true;
}

MinigameData::MinigameData() {
	sequenceIndex_ = -1;
	lastScore_ = 0;
	lastTime_ = 0;
	bestTime_ = 0;
	bestScore_ = 0;
}

GameInfo::GameInfo() {
	empty_ = true;
	dataSize_ = 0;
	gameData_ = 0;
}

void GameInfo::free() {
	if (gameData_) {
		xassert(dataSize_ > 0);
		//dprintf("memory free: %#x\n", gameData_);
		::free(gameData_);
		gameData_ = 0;
	}
	dataSize_ = 0;
}

void GameInfo::write(void* data, unsigned int size) {
	if (dataSize_ != size) {
		free();
		if (size > 0) {
			gameData_ = malloc(size);
			dataSize_ = size;
			//dprintf("memory alloc: %#x, %d bytes\n", gameData_, size);
		}
	}
	if (dataSize_ > 0)
		memcpy(gameData_, data, dataSize_);
}

XStream &operator< (XStream& out, const GameInfo& info) {
	out.write(info.game_);
	out.write(info.empty_);
	if (!info.empty_) {
		out.write(info.timeManagerData_);
		out.write(info.effectManagerData_);
		out < info.dataSize_;
		if (info.dataSize_ > 0)
			out.write(info.gameData_, info.dataSize_);
	}
	return out;
}

XStream &operator> (XStream& in, GameInfo& info) {
	in.read(info.game_);
	in.read(info.empty_);
	if (!info.empty_) {
		in.read(info.timeManagerData_);
		in.read(info.effectManagerData_);
		unsigned int size;
		in > size;
		XBuffer buf(size);
		in.read(buf.buffer(), size);
		info.write(buf.buffer(), size);
	}
	return in;
}


//========================================================================================================================


TimeManager::TimeManager(HoldData<TimeManagerData> &data) {
	if (const char * data = runtime->parameter("game_time", false)) {
		if (sscanf(data, "%f", &gameTime_) != 1)
			gameTime_ = -1.f;
	} else
		gameTime_ = -1.f;

	timeCost_ = 0.f;

	if (gameTime_ > 0) {
		if (const char * data = runtime->parameter("time_bar"))
			timeBar_ = runtime->getObject(data);

		if (const char * data = runtime->parameter("time_cost"))
			sscanf(data, "%f", &timeCost_);
	}

	if (timeBar_) {
		TimeManagerData myData;
		myData.crd = runtime->world2game(timeBar_);

		data.process(myData);

		startPos_ = myData.crd;
		size_ = runtime->getSize(timeBar_);

		if (const char * data = runtime->parameter("time_bar_direction")) {
			int dir;
			if (sscanf(data, "%d", &dir) == 1) {
				xassert(dir >= 0 && dir <= 3);
				direction_ = Direction(dir);
			} else
				direction_ = DOWN;
		} else
			direction_ = DOWN;
	} else
		size_ = mgVect2f(-1.f, -1.f);

	xassert(runtime->getTime() == 0.f);

	lastEventTime_ = 0;

}

TimeManager::~TimeManager() {
	if (timeBar_)
		runtime->release(timeBar_);

}

bool TimeManager::timeIsOut() const {
	if (gameTime_ > 0.f)
		return runtime->getTime() > gameTime_;
	return false;

}

float TimeManager::leftTime() const {
	if (gameTime_ <= 0.f)
		return 0;
	return runtime->getTime() > gameTime_ ? 0 : gameTime_ - runtime->getTime();

}

void TimeManager::quant(float dt) {
	int seconds = round(runtime->getTime());
	if (seconds != lastEventTime_) {
		lastEventTime_ = seconds;
		runtime->textManager().updateTime(seconds);
		int amountSeconds = round(leftTime());
		if (gameTime_ < 0.f || amountSeconds > 10)
			if (seconds % 60 == 0)
				runtime->signal(EVENT_TIME_60_SECOND_TICK);
			else if (seconds % 10 == 0)
				runtime->signal(EVENT_TIME_10_SECOND_TICK);
			else
				runtime->signal(EVENT_TIME_1_SECOND_TICK);
		else if (amountSeconds == 10)
			runtime->signal(EVENT_TIME_10_SECOND_LEFT);
		else
			runtime->signal(EVENT_TIME_LESS_10_SECOND_LEFT_SECOND_TICK);
	}

	if (gameTime_ <= 0.f || !timeBar_)
		return;

	float phase = clamp(runtime->getTime() / gameTime_, 0.f, 1.f);
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

	timeBar_->set_R(runtime->game2world(pos));
}

} // namespace QDEngine
