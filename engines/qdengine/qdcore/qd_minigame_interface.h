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

#ifndef QDENGINE_QDCORE_QD_MINIGAME_INTERFACE_H
#define QDENGINE_QDCORE_QD_MINIGAME_INTERFACE_H


namespace QDEngine {

class qdEngineInterface;

//! Интерфейсный класс для подключения мини-игр через dll.
/**
В dll с игрой должны быть две функции:
qdMiniGameInterface* open_game_interface(const char* game_name); - открывает интерфейс игры,
bool close_game_interface(qdMiniGameInterface* p); - закрывает интерфейс игры.

Формат конфигурационного .ini файла (этот файл указывается в редакторе в параметрах миниигры):

Имя секции - название параметра,
ключи:

1. type = "string" / "file" / "object" - тип данных (строка, файл, объект)

string: просто некие текстовые данные для миниигры.

file: данные - имя файла, который нужен для работы миниигы.
Если есть такие файлы, то их обязательно надо заносить в параметры, чтобы
они корректно копировались при финальной сборке игры.

object: данные - имя объекта из сцены для миниигры
Используется в ситуации, когда миниигре требуется большое количество одинаковых объектов из сцены.
При загрузке игры в сцене создаётся заказанное количество копий объекта.

2. count - количество копий объекта, для строк и файлов игнорируется
3. value - строка данных по-умолчанию (в зависимости от типа данных - просто строка, имя файла или имя объекта,
для копий объекта к этому имени добавляется черырёхзначный индекс, отсчитывающийся от нуля)

В редакторе эту строку можно менять, при этом изменения пишутся в скрипт игры.

4. comment - комментарий

Если тип данных не указан, то считается равным "string".

Пример:

[ParameterExample]
type = "object"
count = 2
value = "Object0"
comment = "пример параметра"
*/

class qdMinigameSceneInterface;

class qdMiniGameInterface {
public:
	//! Возвращает указатель на интерфейс к игре, параметр - имя конфигурационного файла.
	typedef qdMiniGameInterface *(*interface_open_proc)(const char *game_name);
	//! Очищает интерфейс к игре.
	typedef bool (*interface_close_proc)(qdMiniGameInterface *p);

	virtual ~qdMiniGameInterface() { };

	//! Инициализация игры.
	virtual bool init(const qdEngineInterface *engine_interface) = 0;

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	virtual bool quant(float dt) = 0;

	//! Деинициализация игры.
	virtual bool finit() = 0;

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	virtual bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	virtual int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	virtual int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	// maski, 20060129, no version yet
	// version 100, 20060414, qdMinigameCounterInterface added, version added, no other changes
	// version 101, 20060607
	// version 102, 20060629
	// version 103, 20060715
	// version 104, never released
	// version 105, 20061026, b945e65
	// version 106, 20070214
	// version 107, 20070404 09:13
	// version 108, 20070404 10:12
	// version 109, 20070416
	// version 110, 20070419
	// version 111, 20070420
	// version 112, 20070507
	enum { INTERFACE_VERSION = 112 };
	virtual int version() const {
		return INTERFACE_VERSION;
	}
};

template<class T>
class mgVect2 {
public:
	explicit mgVect2(T value = (T)0) : x(value), y(value) { }
	mgVect2(T xx, T yy) : x(xx), y(yy) { }

	mgVect2 &operator += (const mgVect2 &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	mgVect2 &operator -= (const mgVect2 &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	mgVect2 &operator *= (T v) {
		x *= v;
		y *= v;
		return *this;
	}
	mgVect2 &operator /= (T v) {
		x /= v;
		y /= v;
		return *this;
	}

	mgVect2 operator + (const mgVect2 &v) {
		return mgVect2(*this) += v;
	}
	mgVect2 operator - (const mgVect2 &v) {
		return mgVect2(*this) -= v;
	}
	mgVect2 operator / (T v) {
		return mgVect2(*this) /= v;
	}
	mgVect2 operator * (T v) {
		return mgVect2(*this) *= v;
	}

	T x;
	T y;
};

template<class T>
class mgVect3 {
public:
	explicit mgVect3(T value = (T)0) : x(value), y(value), z(value) { }
	mgVect3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) { }

	mgVect3 &operator += (const mgVect3 &v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	mgVect3 &operator -= (const mgVect3 &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	mgVect3 &operator *= (T v) {
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}
	mgVect3 &operator /= (T v) {
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	mgVect3 operator + (const mgVect3 &v) {
		return mgVect3(*this) += v;
	}
	mgVect3 operator - (const mgVect3 &v) {
		return mgVect3(*this) -= v;
	}
	mgVect3 operator / (T v) {
		return mgVect3(*this) /= v;
	}
	mgVect3 operator * (T v) {
		return mgVect3(*this) *= v;
	}

	T x;
	T y;
	T z;
};

typedef mgVect3<float> mgVect3f;
typedef mgVect2<float> mgVect2f;
typedef mgVect2<int> mgVect2i;

//! Интерфейс к динамическому объекту.
class qdMinigameObjectInterface {
public:
	virtual ~qdMinigameObjectInterface() { };

	/// Имя объекта.
	virtual const char *name() const = 0;

	//! Возвращает true, если у объекта есть состояние с именем state_name.
	virtual bool has_state(const char *state_name) const = 0;
	//! Возвращает имя активного в данный момент состояния.
	//! Вернёт 0, если активного состояния нету.
	virtual const char *current_state_name() const = 0;
	//! Возвращает true, если состояние с именем state_name включено в данный момент.
	virtual bool is_state_active(const char *state_name) const = 0;
	//! Возвращает true, если состояние с именем state_name в данный момент ожидает активации.
	virtual bool is_state_waiting_activation(const char *state_name) const = 0;
	//! Возвращает номер включенного в данный момент состояния.
	virtual int current_state_index() const = 0;
	//! Включает состояние с именем state_name.
	virtual bool set_state(const char *state_name) = 0;
	//! Включает состояние номер state_index (отсчитывается от нуля).
	virtual bool set_state(int state_index) = 0;
	//! Возвращает номер состояния с именем state_name.
	/**
	Отсчитывается от нуля, если такого состояния нет, то
	возвращает -1.
	*/
	virtual int state_index(const char *state_name) const = 0;

	//! Возвращает координаты объекта в мировой системе координат.
	virtual mgVect3f R() const = 0;
	//! Устанавливает координаты объекта в мировой системе координат.
	virtual void set_R(const mgVect3f &r) = 0;

	/// Проверка, попадает ли точка с экранными координатами pos в объект
	virtual bool hit_test(const mgVect2i &pos) const = 0;

	//! Возвращает координаты объекта в экранной системе координат.
	virtual mgVect2i screen_R() const = 0;
	//! Обновляет координаты объекта в экранной системе координат.
	virtual bool update_screen_R() = 0;
	//! Возвращает текущие экранные размеры объекта в пикселах.
	virtual mgVect2i screen_size() const = 0;

	//! Устанавливает поворот картинки объекта.
	//! angle - угол, на который должна быть повёрнута картинка, в радианах
	//! speed - скорость поворота, в радианах в секунду
	virtual void set_screen_rotation(float angle, float speed) = 0;
	//! Возвращает поворот картинки объекта в радианах.
	virtual float screen_rotation() const = 0;
	//! Устанавливает масштабирование картинки объекта.
	virtual void set_screen_scale(const mgVect2f &scale, const mgVect2f &speed) = 0;
	//! Возвращает масштаб картинки объекта.
	virtual mgVect2f screen_scale() const = 0;

	//! Возвращает цвет затенения.
	virtual int shadow_color() const = 0;
	//! Возвращает прозрачность затенения, значения - [0, 255], если равно -1, то затенения нет.
	virtual int shadow_alpha() const = 0;
	//! Устанавливает затенение.
	virtual bool set_shadow(int shadow_color, int shadow_alpha) = 0;

	//! Возвращает true, если объект не спрятан.
	virtual bool is_visible() const = 0;

	//! Возвращает размеры объекта в мировой системе координат.
	virtual mgVect3f bound() const = 0;

	//! Команда персонажу идти к точке target_position.
	/**
	 Если второй параметр равен false, то если target_position непроходима
	 персонаж идёт к ближайшей от target_position проходимой точке.
	 Для обычных объектов - всегда возвращает false
	*/
	virtual bool move(const mgVect3f &target_position, bool disable_target_change = false) = 0;

	//! Возвращает направление, в котором смотрит персонаж.
	//! Значение в радианах, 0 соотвествует направлению вправо, поворот по часовой стрелке.
	//! Для обычных объектов всегда возвращает 0.
	virtual float direction_angle() const = 0;
	//! Устанавливает направление персонажа.
	virtual bool set_direction_angle(float direction) = 0;
};

//! Интерфейс к сцене.
class qdMinigameSceneInterface {
public:
	virtual ~qdMinigameSceneInterface() { }

	/// Имя сцены.
	virtual const char *name() const = 0;

	//! Создаёт интерфейс к объекту с именем object_name.
	virtual qdMinigameObjectInterface *object_interface(const char *object_name) = 0;
	//! Создаёт интерфейс к персонажу с именем personage_name.
	virtual qdMinigameObjectInterface *personage_interface(const char *personage_name) = 0;
	//! Активация персонажа с именем personage_name.
	virtual bool activate_personage(const char *personage_name) = 0;

	//! Преобразование из экранных координат в мировые.
	virtual mgVect3f screen2world_coords(const mgVect2i &screen_pos, float screen_depth = 0) const = 0;
	//! Преобразование из мировых координат в экранные.
	virtual mgVect2i world2screen_coords(const mgVect3f &world_pos) const = 0;
	//! Возвращает "глубину" точки с координатами pos в мировой системе координат
	virtual float screen_depth(const mgVect3f &pos) const = 0;
	//! Возвращает мировые координаты точки на сетке по её экранным координатам.
	virtual mgVect3f screen2grid_coords(const mgVect2i &screen_pos) const = 0;

	//! Создаёт интерфейс к объекту, который взят мышью в данный момент.
	virtual qdMinigameObjectInterface *mouse_object_interface() const = 0;
	//! Создаёт интерфейс к объекту, по которому кликнули мышью.
	virtual qdMinigameObjectInterface *mouse_click_object_interface() const = 0;
	//! Создаёт интерфейс к объекту, по которому кликнули правой кнопкой мыши.
	virtual qdMinigameObjectInterface *mouse_right_click_object_interface() const = 0;
	//! Создаёт интерфейс к объекту, над которым находится мышиный курсор.
	virtual qdMinigameObjectInterface *mouse_hover_object_interface() const = 0;

	virtual const char *minigame_parameter(const char *parameter_name) const = 0;

	virtual void release_object_interface(qdMinigameObjectInterface *p) const = 0;
};

/// Интерфейс к счётчику.
class qdMinigameCounterInterface {
public:
	virtual ~qdMinigameCounterInterface() { }

	/// возвращает текущее значение счётчика
	virtual int value() const = 0;
	/// устанавливает текущее значение счётчика
	virtual void set_value(int value) = 0;
	/// добавляет к текущему значению счётчика value_delta
	virtual void add_value(int value_delta) = 0;
};

class qdEngineInterface {
public:
	virtual ~qdEngineInterface() { }

	//! Идентификаторы событий для работы с мышью.
	enum qdMinigameMouseEvent {
		//! Нажатие левой кнопки.
		MOUSE_EV_LEFT_DOWN = 0,
		//! Нажатие правой кнопки.
		MOUSE_EV_RIGHT_DOWN,
		//! Двойное нажатие левой кнопки.
		MOUSE_EV_LEFT_DBLCLICK,
		//! Двойное нажатие правой кнопки.
		MOUSE_EV_RIGHT_DBLCLICK,
		//! Отжатие левой кнопки.
		MOUSE_EV_LEFT_UP,
		//! Отжатие правой кнопки.
		MOUSE_EV_RIGHT_UP,
		//! Перемещение мыши.
		MOUSE_EV_MOUSE_MOVE
	};

	virtual qdMinigameSceneInterface *current_scene_interface() const = 0;
	virtual void release_scene_interface(qdMinigameSceneInterface *p) const = 0;

	virtual qdMinigameCounterInterface *counter_interface(const char *counter_name) const = 0;
	virtual void release_counter_interface(qdMinigameCounterInterface *p) const = 0;

	//! Возвращает размер экрана в пикселах.
	virtual mgVect2i screen_size() const = 0;

	//! Возвращает true, если на клавиатуре нажата кнопка vkey.
	virtual bool is_key_pressed(int vkey) const = 0;

	//! Возвращает true, если в данный момент произошло событие event_id.
	virtual bool is_mouse_event_active(qdMinigameMouseEvent event_id) const = 0;

	//! Возвращает текущие координаты мышиного курсора.
	virtual mgVect2i mouse_cursor_position() const = 0;

	/// Добавляет набранные очки в таблицу рекордов.
	/// Если очков оказалось достаточно, возвращает true.
	virtual bool add_hall_of_fame_entry(int score) const = 0;

	/// Устанавливает текст в интрефейсном контроле.
	/// Если имя экрана нулевое, контрол ищется в текущем эеране.
	virtual bool set_interface_text(const char *screen_name, const char *control_name, const char *text) const = 0;

	/// Инициализация rnd
	virtual void rnd_init(int seed) const = 0;
	/// Возвращает случайное значение в интервале [0, m-1].
	virtual uint32 rnd(uint32 m) const = 0;
	/// Возвращает случайное значение в интервале [-x, x].
	virtual float frnd(float x) const = 0;
	/// Возвращает случайное значение в интервале [0, x].
	virtual float fabs_rnd(float x) const = 0;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_MINIGAME_INTERFACE_H
