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

#ifndef QDENGINE_MINIGAMES_ADV_RUNTIME_H
#define QDENGINE_MINIGAMES_ADV_RUNTIME_H

#include "common/hashmap.h"

#include "qdengine/minigames/adv/common.h"

namespace Common {
class SeekableReadStream;
class SeekableWriteStream;
}

namespace QDEngine {

class qdEngineInterface;
class qdMinigameSceneInterface;

class MinigameInterface;
class TextManager;
class TimeManager;
class EventManager;
class EffectManager;

struct TimeManagerData {
	mgVect3f crd;
};

struct EffectManagerData {
	mgVect3f crd;
};

struct MinigameData {
	MinigameData();
	int _sequenceIndex;
	int _lastScore;
	int _lastTime;
	int _bestTime;
	int _bestScore;

	void write(Common::WriteStream &out) const;
	void read(Common::ReadStream &out);
};

struct GameInfo {
	GameInfo();
	void persist(Common::SeekableReadStream &in);

	void write(Common::WriteStream &out) const;
	void read(Common::ReadStream &in);

	void free();
	static int version() {
		return 9;
	}
	bool empty() const {
		return _empty && _game._sequenceIndex < 0;
	}

	MinigameData _game;
	bool _empty;
	TimeManagerData _timeManagerData;
	EffectManagerData _effectManagerData;
	uint _dataSize;
	void *_gameData;
};

typedef MinigameInterface *(*MinigameConsCallback)(MinigameManager *runtime);

qdMiniGameInterface *create_adv_minigame(const char *name, MinigameConsCallback callback);

bool close_adv_minigame(qdMiniGameInterface *game);

class MinigameManager : public qdMiniGameInterface {
	friend class TempValue;
public:
	MinigameManager(MinigameConsCallback callback);
	~MinigameManager();

	// begin MiniGame virtual interface
	bool init(const qdEngineInterface *engine_interface);
	bool quant(float dt);
	bool finit();

	bool new_game(const qdEngineInterface *engine);
	int save_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, char *buffer, int buffer_size);
	int load_game(const qdEngineInterface *engine, const qdMinigameSceneInterface *scene, const char *buffer, int buffer_size);
	// finish MiniGame virtual interface

	// при необходимости заменяет на неизмененные предыдущим прохождением данные
	bool processGameData(Common::SeekableReadStream &data);

	mgVect2f mousePosition() const {
		return _mousePos;
	}
	bool mouseLeftPressed() const;
	bool mouseRightPressed() const;
	bool keyPressed(int vKey, bool once = false) const;

	mgVect2i screenSize() const {
		return _screenSize;
	}
	float getTime() const {
		return _gameTime;
	}

	const MinigameData *getScore(int level, int game) const;

	bool debugMode() const {
		return _debugMode;
	}

	TextManager &textManager() const {
		return *_textManager;
	}

	void signal(SystemEvent id);
	void event(int eventID, const mgVect2f& pos, int factor = 1);
	void event(int eventID, const mgVect2i& pos, int factor = 1) {
		event(eventID, mgVect2f(pos.x, pos.y), factor);
	}

	// указывает вариант показа информации о победе (поворот собранной картинки и т.д.)
	void setCompleteHelpVariant(int idx);
	// указывает номер подсказки для показа, -1 - спрятать подсказку
	void setGameHelpVariant(int idx);

	// Возвращает параметр из прикрепленного к игре ini файла
	const char *parameter(const char *name, bool required = true) const;
	const char *parameter(const char *name, const char *def) const;

	// Пересчитывает из экранных координат UI игры в 3D координаты R() объекта на мире
	mgVect3f game2world(const mgVect3i &coord) const;
	mgVect3f game2world(const mgVect3f &coord) const;
	mgVect3f game2world(const mgVect2i &coord, int depth = 0) const;
	mgVect3f game2world(const mgVect2f &coord, int depth = 0) const;
	// Пересчитывает из мировых координат R() в 2D UI координаты и глубину
	mgVect2f world2game(const mgVect3f& pos) const;
	mgVect3f world2game(qdMinigameObjectInterface *obj) const;
	// размер объекта
	mgVect2f getSize(qdMinigameObjectInterface *obj) const;

	// Меняет глубину объекта, не меняя его 2D положения на экране
	void setDepth(qdMinigameObjectInterface *obj, int depth) const;
	// Получает глубину объекта, чем меньше, тем ближе к игроку
	float getDepth(qdMinigameObjectInterface *obj) const;
	// Получает глубину точки, чем меньше, тем ближе к игроку
	float getDepth(const mgVect3f& pos) const;

	// получает интерфейс к динамическому игровому объекту по имени
	QDObject getObject(const char *name) const;
	// проверяет существование динамического объекта в сцене
	bool testObject(const char *name) const;
	// освобождает интерфейс
	void release(QDObject& obj);

	// задать текст для контрола
	void setText(const char *name, const char *text) const;
	void setText(const char *name, int toText, const char *format = "%d") const;

	// спрятать объект за пределами экрана
	void hide(qdMinigameObjectInterface *obj) const;

	// случайное значение в диапазоне [min, max]
	float rnd(float min, float max) const;
	int rnd(int min, int max) const;
	// случайный диапазон, из набора вероятностей
	int rnd(const Std::vector<float> &prob) const;

	// файл со списком игр по уровням
	const char *gameListFileName() const {
		return "resource/minigames.lst";
	}

	int getParameter(const char* name, const int& defValue);
	bool getParameter(const char* name, int& out, bool obligatory);
	float getParameter(const char* name, const float &defValue);
	bool getParameter(const char* name, float &out, bool obligatory);
	mgVect2f getParameter(const char* name, const mgVect2f& defValue);
	bool getParameter(const char* name, mgVect2f& out, bool obligatory);
	mgVect2i getParameter(const char* name, const mgVect2i& defValue);
	bool getParameter(const char* name, mgVect2i& out, bool obligatory);

private:
	MinigameInterface *_game;

	// Вывод текста с помощью объектов
	TextManager *_textManager;
	// Подсчет и визуализация времени
	TimeManager *_timeManager;
	// Обработка событий игры
	EventManager *_eventManager;
	// выводимые эффекты
	EffectManager *_effectManager;

	// Время в секундах с момента стара игры
	float _gameTime;
	// кеш проверенных на нажатие клавиш, для отслеживания непосредственно нажатия
	mutable bool _lastKeyChecked[256];
	// Размер играна
	mgVect2i _screenSize;
	// текущее положение мыши
	mgVect2f _mousePos;
	// подстройка мыши
	mgVect2f _mouseAdjast;

	// объект для передачи сигнала об окончании игры в триггеры
	qdMinigameObjectInterface *_state_flag;
	// объект для получения сигнала о постановке на паузу
	qdMinigameObjectInterface *_pause_flag;
	// справка по победе
	QDObject _complete_help;
	QDObject _complete_help_miniature;
	// текущее состояние для включения справки
	Common::String _complete_help_state_name;
	// справка по игре
	QDObject _game_help;
	QDObject _game_help_trigger;
	bool _game_help_enabled;
	// текущее состояние для включения справки
	Common::String _game_help_state_name;

	// интерфейс к движку
	const qdEngineInterface *_engine;
	// интерфейс к текущей сцене
	qdMinigameSceneInterface *_scene;

	// игра запущена для отладки
	bool _debugMode;
	// rnd seed
	int _seed;

	// кнопки мыши инвертированы
	bool _invertMouseButtons;

	// имя файла и информацией о минииграх
	Common::String _state_container_name;
	// количество пройденных игр на каждом уровне
	typedef Common::HashMap<int, int> Counters;
	Counters _completeCounters;

	struct GameInfoIndex {
		GameInfoIndex(int idx, int level) : _gameNum(idx), _gameLevel(level) {}
		int _gameNum;
		int _gameLevel;

		void write(Common::WriteStream &out) const;
		void read(Common::ReadStream &in);

		bool operator< (const GameInfoIndex& rs) const {
			return _gameLevel == rs._gameLevel ? _gameNum < rs._gameNum : _gameLevel < rs._gameLevel;
		}
	};

	struct GameInfoIndex_Hash {
		uint operator()(const GameInfoIndex& x) const {
			return (x._gameNum << 16) + x._gameLevel;
		}
	};

	struct GameInfoIndex_EqualTo {
		uint operator()(const GameInfoIndex& x, const GameInfoIndex& y) const {
			return x._gameNum == y._gameNum && x._gameLevel == y._gameLevel;
		}
	};

	// информация о пройденных играх
	typedef Common::HashMap<GameInfoIndex, GameInfo, GameInfoIndex_Hash, GameInfoIndex_EqualTo> GameInfoMap;
	GameInfoMap _gameInfos;
	// Информация о текущей игре, при выходе запишется
	GameInfoIndex _currentGameIndex;
	GameInfo *_currentGameInfo;

	// проверить что все необходимые игры пройдены
	bool testAllGamesWin();
	// Непосредственно создает и инициализирует игру
	bool createGame();
	// обработка победы
	void gameWin();
	// обработка поражения
	void gameLose();
	// чтение данных об играх
	bool loadState(bool current = true);
	// сохранение данных в файл
	void saveState(bool force = false);

	// Полуить объект-счетчик
	QDCounter getCounter(const char *name);
	// Освободить счетчик
	void release(QDCounter& counter);

	MinigameConsCallback _callback = nullptr;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_RUNTIME_H
