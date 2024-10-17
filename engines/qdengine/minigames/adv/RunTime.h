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

namespace QDEngine {

class qdEngineInterface;
class qdMinigameSceneInterface;

class MinigameInterface;
class TextManager;
class TimeManager;
enum SystemEvent;
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
	int sequenceIndex_;
	int lastScore_;
	int lastTime_;
	int bestTime_;
	int bestScore_;
};

struct GameInfo {
	GameInfo();
	void write(void* data, unsigned int size);
	void free();
	static int version() {
		return 9;
	}
	bool empty() const {
		return empty_ && game_.sequenceIndex_ < 0;
	}

	MinigameData game_;
	bool empty_;
	TimeManagerData timeManagerData_;
	EffectManagerData effectManagerData_;
	unsigned int dataSize_;
	void *gameData_;
};

class XStream;
XStream &operator< (XStream& out, const GameInfo& info);
XStream &operator> (XStream& in, GameInfo& info);

class MinigameManager : public qdMiniGameInterface {
	friend class TempValue;
public:
	MinigameManager();
	~MinigameManager();

	// begin MiniGame virtual interface
	bool init(const qdEngineInterface* engine_interface);
	bool quant(float dt);
	bool finit();

	bool new_game(const qdEngineInterface* engine);
	int save_game(const qdEngineInterface* engine, const qdMinigameSceneInterface* scene, char* buffer, int buffer_size);
	int load_game(const qdEngineInterface* engine, const qdMinigameSceneInterface* scene, const char* buffer, int buffer_size);
	// finish MiniGame virtual interface

	// при необходимости заменяет на неизмененные предыдущим прохождением данные
	bool processGameData(XBuffer& data);

	mgVect2f mousePosition() const {
		return mousePos_;
	}
	bool mouseLeftPressed() const;
	bool mouseRightPressed() const;
	bool keyPressed(int vKey, bool once = false) const;

	mgVect2i screenSize() const {
		return screenSize_;
	}
	float time() const {
		return gameTime_;
	}

	const MinigameData *getScore(int level, int game) const;

	bool debugMode() const {
		return debugMode_;
	}

	TextManager &textManager() const {
		return *textManager_;
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
	const char *parameter(const char* name, bool required = true) const;
	const char *parameter(const char* name, const char* def) const;

	// Пересчитывает из экранных координат UI игры в 3D координаты R() объекта на мире
	mgVect3f game2world(const mgVect3i& coord) const;
	mgVect3f game2world(const mgVect3f& coord) const;
	mgVect3f game2world(const mgVect2i& coord, int depth = 0) const;
	mgVect3f game2world(const mgVect2f& coord, int depth = 0) const;
	// Пересчитывает из мировых координат R() в 2D UI координаты и глубину
	mgVect2f world2game(const mgVect3f& pos) const;
	mgVect3f world2game(qdMinigameObjectInterface* obj) const;
	// размер объекта
	mgVect2f getSize(qdMinigameObjectInterface* obj) const;

	// Меняет глубину объекта, не меняя его 2D положения на экране
	void setDepth(qdMinigameObjectInterface* obj, int depth) const;
	// Получает глубину объекта, чем меньше, тем ближе к игроку
	float getDepth(qdMinigameObjectInterface* obj) const;
	// Получает глубину точки, чем меньше, тем ближе к игроку
	float getDepth(const mgVect3f& pos) const;

	// получает интерфейс к динамическому игровому объекту по имени
	QDObject getObject(const char* name) const;
	// проверяет существование динамического объекта в сцене
	bool testObject(const char* name) const;
	// освобождает интерфейс
	void release(QDObject& obj);

	// задать текст для контрола
	void setText(const char* name, const char* text) const;
	void setText(const char* name, int toText, const char* format = "%d") const;

	// спрятать объект за пределами экрана
	void hide(qdMinigameObjectInterface* obj) const;

	// случайное значение в диапазоне [min, max]
	float rnd(float min, float max) const;
	int rnd(int min, int max) const;
	// случайный диапазон, из набора вероятностей
	int rnd(const vector<float> &prob) const;

	// файл со списком игр по уровням
	const char *gameListFileName() const {
		return "resource\\minigames.lst";
	}

private:
	MinigameInterface *game_;

	// Вывод текста с помощью объектов
	TextManager *textManager_;
	// Подсчет и визуализация времени
	TimeManager *timeManager_;
	// Обработка событий игры
	EventManager *eventManager_;
	// выводимые эффекты
	EffectManager *effectManager_;

	// Время в секундах с момента стара игры
	float gameTime_;
	// кеш проверенных на нажатие клавиш, для отслеживания непосредственно нажатия
	mutable bool lastKeyChecked_[256];
	// Размер играна
	mgVect2i screenSize_;
	// текущее положение мыши
	mgVect2f mousePos_;
	// подстройка мыши
	mgVect2f mouseAdjast_;

	// объект для передачи сигнала об окончании игры в триггеры
	qdMinigameObjectInterface *state_flag_;
	// объект для получения сигнала о постановке на паузу
	qdMinigameObjectInterface *pause_flag_;
	// справка по победе
	QDObject complete_help_;
	QDObject complete_help_miniature_;
	// текущее состояние для включения справки
	string complete_help_state_name_;
	// справка по игре
	QDObject game_help_;
	QDObject game_help_trigger_;
	bool game_help_enabled_;
	// текущее состояние для включения справки
	string game_help_state_name_;

	// интерфейс к движку
	const qdEngineInterface *engine_;
	// интерфейс к текущей сцене
	qdMinigameSceneInterface *scene_;

	// игра запущена для отладки
	bool debugMode_;
	// rnd seed
	int seed_;

	// кнопки мыши инвертированы
	bool invertMouseButtons_;

	// имя файла и информацией о минииграх
	const char *state_container_name_;
	// количество пройденных игр на каждом уровне
	typedef map<int, int> Counters;
	Counters completeCounters_;

	struct GameInfoIndex {
		GameInfoIndex(int idx, int level) : gameNum_(idx), gameLevel_(level) {}
		int gameNum_;
		int gameLevel_;
		bool operator< (const GameInfoIndex& rs) const {
			return gameLevel_ == rs.gameLevel_ ? gameNum_ < rs.gameNum_ : gameLevel_ < rs.gameLevel_;
		}
	};
	// информация о пройденных играх
	typedef map<GameInfoIndex, GameInfo> GameInfoMap;
	GameInfoMap gameInfos_;
	// Информация о текущей игре, при выходе запишется
	GameInfoIndex currentGameIndex_;
	GameInfo *currentGameInfo_;

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
	QDCounter getCounter(const char* name);
	// Освободить счетчик
	void release(QDCounter& counter);
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_RUNTIME_H
