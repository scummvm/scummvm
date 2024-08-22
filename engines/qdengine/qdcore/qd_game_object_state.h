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

#ifndef QDENGINE_QDCORE_QD_GAME_OBJECT_STATE_H
#define QDENGINE_QDCORE_QD_GAME_OBJECT_STATE_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_contour.h"
#include "qdengine/qdcore/qd_sound_info.h"
#include "qdengine/qdcore/qd_animation_info.h"
#include "qdengine/qdcore/qd_animation_set_info.h"
#include "qdengine/qdcore/qd_conditional_object.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_coords_animation.h"
#include "qdengine/qdcore/qd_camera_mode.h"
#include "qdengine/qdcore/qd_sound_handle.h"
#include "qdengine/qdcore/qd_screen_text.h"
#include "qdengine/qdcore/qd_grid_zone.h"
#include "qdengine/system/sound/snd_sound.h"

namespace QDEngine {

class qdScreenTransform {
public:
	qdScreenTransform(float angle = 0.f, const Vect2f &scale = Vect2f(1.f, 1.f)) : _angle(angle), _scale(scale) { }

	bool operator == (const qdScreenTransform &trans) const;
	bool operator != (const qdScreenTransform &trans) const {
		return !(*this == trans);
	}
	bool operator()() const {
		return *this != ID;
	}

	qdScreenTransform operator *(float value) const {
		return qdScreenTransform(_angle *value, _scale *value);
	}

	qdScreenTransform &operator += (const qdScreenTransform &trans) {
		_angle += trans._angle;
		_scale += trans._scale;

		return *this;
	}

	bool change(float dt, const qdScreenTransform &target_trans, const qdScreenTransform &speed);

	float angle() const {
		return _angle;
	}
	void set_angle(float angle) {
		_angle = angle;
	}

	const Vect2f &scale() const {
		return _scale;
	}
	void set_scale(const Vect2f &scale) {
		_scale = scale;
	}
	bool has_scale() const {
		return fabs(_scale.x - 1.f) > FLT_EPS || fabs(_scale.y - 1.f) > FLT_EPS;
	}

	/// единичная трансформация
	static qdScreenTransform ID;

private:

	float _angle;
	Vect2f _scale;

};

//! Состояние динамического объекта - базовый класс.
class qdGameObjectState : public qdConditionalObject {
public:
	//! Флаги состояния.
	enum {
		//! Объект спрятан.
		QD_OBJ_STATE_FLAG_HIDDEN            = 0x01,
		//! The state is not in triggers. Deprecated, replaced with is_in_triggers()
		QD_OBJ_STATE_FLAG_NOT_IN_TRIGGERS  = 0x02,
		//! Восстанавливать предыдущее состояние по окончании состояния.
		QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE        = 0x04,
		//! Прятать объект по окончании состояния.
		QD_OBJ_STATE_FLAG_HIDE_OBJECT           = 0x08,
		//! Объект, которому принадлежит состояние - глобальный.
		QD_OBJ_STATE_FLAG_GLOBAL_OWNER          = 0x10,
		//! Состояние для инвентори.
		QD_OBJ_STATE_FLAG_INVENTORY         = 0x20,
		//! Положить объект в инвентори по окончании состояния.
		QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY     = 0x40,
		//! Положить в инвентори не удалось.
		QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED  = 0x80,
		//! У состояния задан баунд.
		QD_OBJ_STATE_FLAG_HAS_BOUND         = 0x100,
		//! Активирована задержка перед включением состояния.
		QD_OBJ_STATE_FLAG_ACTIVATION_TIMER      = 0x200,
		//! Задержка перед включением состояния окончилась.
		QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END      = 0x400,
		//! Заданный для состояния текст - вариант фразы для диалогов.
		/**
		Означает, что одно из условий активации состояния - клик мышью по
		соответствующему тексту на экране.
		*/
		QD_OBJ_STATE_FLAG_DIALOG_PHRASE         = 0x800,
		//! Синхронизировать анимацию по звуку.
		/**
		Когда заканчивается звук, анимация останавливается.
		Работает только для qdGameObjectStateStatic без координатной анимации.
		*/
		QD_OBJ_STATE_FLAG_SOUND_SYNC            = 0x1000,
		//! Разрешить прерывать состояние.
		/**
		Если флаг установлен, то состояние может быть прервано до
		окончания его работы (например для персонажа - кликом мыши).
		*/
		QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT      = 0x2000,
		//! Состояние уже было активировано.
		/**
		Показывает, что состояние уже было хотя бы один раз активировано.
		*/
		QD_OBJ_STATE_FLAG_WAS_ACTIVATED         = 0x4000,
		//! Запретить прерывать подход к стартовой точке состояния.
		/**
		Имеет смысл только если для состояний с подходом к точке запуска
		(need_to_walk() == true).
		*/
		QD_OBJ_STATE_FLAG_DISABLE_WALK_INTERRUPT    = 0x8000,
		//! Включать состояние, когда объект взят на мышь.
		QD_OBJ_STATE_FLAG_MOUSE_STATE           = 0x10000,
		//! Включать состояние, когда мышь над объектом в инвентори.
		QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE     = 0x20000,
		//! Не вытаскивать объект из инвентори, когда он взят на мышь.
		QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY     = 0x40000,
		/**
		Принудительно загружать ресурсы состояния в память при заходе на сцену,
		не выгружать их до выхода со сцены.
		*/
		QD_OBJ_STATE_FLAG_FORCED_LOAD           = 0x80000,
		//! Состояние можно скиповать кликом мыши.
		QD_OBJ_STATE_FLAG_ENABLE_SKIP           = 0x100000,
		//! При включении состояния класть объект на зону под мышью.
		QD_OBJ_STATE_FLAG_MOVE_TO_ZONE          = 0x200000,
		//! При включении состояния класть объект в центр объекта под мышью.
		QD_OBJ_STATE_FLAG_MOVE_ON_OBJECT        = 0x400000,
		//! По окончании работы состояния делать персонажа-владельца активным.
		QD_OBJ_STATE_FLAG_ACTIVATE_PERSONAGE        = 0x800000,
		//! Автоматически загружать сэйв при активации состояния.
		QD_OBJ_STATE_FLAG_AUTO_LOAD                 = 0x1000000,
		//! Автоматически писать сэйв при активации состояния.
		QD_OBJ_STATE_FLAG_AUTO_SAVE                 = 0x2000000,
		QD_OBJ_STATE_FLAG_FADE_IN                   = 0x4000000,
		QD_OBJ_STATE_FLAG_FADE_OUT                  = 0x8000000,
	};

	//! Тип состояния.
	enum StateType {
		//! Статическая анимация.
		STATE_STATIC,
		//! Походка.
		STATE_WALK,
		//! Маска на фоне.
		STATE_MASK
	};

	enum {
		CURSOR_UNASSIGNED = -1
	};

	qdGameObjectState(StateType tp);
	qdGameObjectState(const qdGameObjectState &st);
	virtual ~qdGameObjectState();

	virtual qdGameObjectState &operator = (const qdGameObjectState &st);

	int named_object_type() const {
		return QD_NAMED_OBJECT_OBJ_STATE;
	}

	//! Возвращает указатель на траекторию движения объекта для состояния.
	qdCoordsAnimation *coords_animation() {
		return &_coords_animation;
	}
	//! Возвращает указатель на траекторию движения объекта для состояния.
	const qdCoordsAnimation *coords_animation() const {
		return &_coords_animation;
	}

	//! Загрузка данных из скрипта.
	virtual bool load_script(const xml::tag *p) = 0;
	//! Запись данных в скрипт.
	virtual bool save_script(Common::WriteStream &fh, int indent = 0) const = 0;

	//! Инициализация состояния, вызывается при старте и перезапуске игры.
	bool init();

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	//! Регистрация ресурсов состояния в диспетчере ресурсов.
	virtual bool register_resources();
	//! Отмена регистрации ресурсов состояния в диспетчере ресурсов.
	virtual bool unregister_resources();
	//! Загрузка ресурсов.
	virtual bool load_resources();
	//! Выгрузка ресурсов.
	virtual bool free_resources();

	//! Возвращает true, если условия активации состояния выполняются.
	bool check_conditions();

	//! Возвращет смещение центра объекта для состояния (в экранных координатах).
	const Vect2s &center_offset() const {
		return _center_offset;
	}
	//! Устанавливает смещение центра объекта для состояния (в экранных координатах).
	void set_center_offset(const Vect2s offs) {
		_center_offset = offs;
	}

	//! Логический квант состояния.
	void quant(float dt);

	//! Возвращает указатель на предыдущее состояние.
	qdGameObjectState *prev_state() {
		return _prev_state;
	}
	//! Устанавливает предыдущее состояние.
	void set_prev_state(qdGameObjectState *p) {
		_prev_state = p;
	}

	//! Возвращает тип состояния.
	StateType state_type() const {
		return _state_type;
	}

	//! Возвращает идентификатор мышиного курсора для состояния.
	int mouse_cursor_ID() const {
		return _mouse_cursor_id;
	}
	//! Устанавливает идентификатор мышиного курсора для состояния.
	/**
	Если должен использоваться курсор по-умолчанию -
	параметр должен быть равен CURSOR_UNASSIGNED.
	*/
	void set_mouse_cursor_ID(int id) {
		_mouse_cursor_id = id;
	}
	//! Возвращает true, если состоянию назначен особый мышиный курсор.
	bool has_mouse_cursor_ID() const {
		return _mouse_cursor_id != CURSOR_UNASSIGNED;
	}

	//! Устанавливает имя звукового эффекта, привязанного к состоянию.
	const char *sound_name() const {
		return _sound_info.name();
	}
	//! Возвращает имя звукового эффекта, привязанного к состоянию.
	void set_sound_name(const char *p) {
		_sound_info.set_name(p);
	}
	//! Возвращает указатель на звуковой эффект, привязанный к состоянию.
	qdSound *sound() const;
	//! Возвращает true, если для состояния задан звук.
	bool has_sound() const {
		if (sound_name()) return true;
		else return false;
	}
	//! Возвращает хэндл звука.
	const qdSoundHandle *sound_handle() const {
		return &_sound_handle;
	}
	//! Запускает звук, position - стартовяя позиция, от 0.0 до 1.0.
	bool play_sound();
	//! Останавливает звук.
	bool stop_sound() const;
	//! Установка частоты звука.
	bool set_sound_frequency(float frequency_coeff) const;
	bool is_sound_finished() const;
	bool is_sound_playing() const;

	//! Устанавливает флаг для звука.
	void set_sound_flag(int fl) {
		_sound_info.set_flag(fl);
	}
	//! Скидывает флаг для звука.
	void drop_sound_flag(int fl) {
		_sound_info.drop_flag(fl);
	}
	//! Возвращает true, если установлен флаг для звука.
	bool check_sound_flag(int fl) const {
		return _sound_info.check_flag(fl);
	}

	//! Возвращает true, если состояние пустое.
	virtual bool is_state_empty() const;

	//! Возвращает true, если в данный момент состояние включено.
	bool is_active() const;
	//! Возвращает true, если состояние стартовое.
	bool is_default() const;

	//! Возвращает задержку (в секундах) перед активацией состояния.
	float activation_delay() const {
		return _activation_delay;
	}
	//! Устанавливает задержку (в секундах) перед активацией состояния.
	void set_activation_delay(float tm) {
		_activation_delay = tm;
	}
	//! Устанавливает таймер перед активацией состояния.
	void set_activation_timer() {
		_activation_timer = _activation_delay;
	}

	//! Вызывается при активации состояния.
	void start() {
		_cur_time = 0.0f;
		_is_sound_started = false;
	}

	//! Возвращает время в секундах, прошедшее с момента активации состояния.
	float cur_time() const {
		return _cur_time;
	}
	//! Возвращает длительность состояния в секундах.
	float work_time() const;
	//! Устанавливает длительность состояния в секундах.
	void set_work_time(float tm) {
		_work_time = tm;
	}

	//! Возвращает true, если персонажу требуется подойти к точке включения состояния.
	bool need_to_walk() const {
		if (!_coords_animation.is_empty()
		        && _coords_animation.check_flag(QD_COORDS_ANM_OBJECT_START_FLAG))
			return true;
		else
			return false;
	}

	virtual qdGameObjectState *clone() const = 0;

	virtual float adjust_direction_angle(float angle) const {
		return angle;
	}

	//! Возвращает координаты точки, в которой должно активироваться состояние.
	const Vect3f start_pos() const {
		if (!_coords_animation.is_empty()) {
			return _coords_animation.get_point(0)->dest_pos();
		} else
			return Vect3f(0, 0, 0);
	}
	//! Возвращает направление объекта в точке, в которой должно активироваться состояние.
	float start_direction_angle() const {
		if (!_coords_animation.is_empty()) {
			return _coords_animation.get_point(0)->direction_angle();
		} else
			return -1.0f;
	}

	//! Возвращает количество ссылок на состояние.
	int reference_count() const {
		return _reference_count;
	}
	//! Инкремент количества ссылок на состояние.
	void inc_reference_count() {
		_reference_count++;
	}
	//! Декремент количества ссылок на состояние.
	void dec_reference_count() {
		if (_reference_count) _reference_count--;
	}

	//! Возвращает true, если у состояния задан текст субтитров.
	bool has_text() const {
		return (!_text_ID.empty() || !_short_text_ID.empty());
	}
	//! Возвращает текст субтитров.
	const char *text() const {
		if (has_full_text()) return full_text();
		else return short_text();
	}
	//! Возвращает полный текст субтитров.
	const char *full_text() const;
	//! Возвращает сокращенный текст субтитров.
	const char *short_text() const;

	//! Возвращает true, если у состояния задан текст субтитров.
	bool has_full_text() const {
		return !_text_ID.empty();
	}
	//! Возвращает true, если у состояния задан сокращенный текст субтитров.
	bool has_short_text() const {
		return !_short_text_ID.empty();
	}
	//! Возвращает полный текст субтитров.
	const char *full_text_ID() const {
		return _text_ID.c_str();
	}
	//! Устанавливает полный текст субтитров.
	/**
	Если параметр нулевой, то текст очищается.
	*/
	void set_full_text_ID(const char *p) {
		if (p) _text_ID = p;
		else _text_ID.clear();
	}
	//! Возвращает сокращенный текст субтитров.
	const char *short_text_ID() const {
		return _short_text_ID.c_str();
	}
	//! Устанавливает сокращенный текст субтитров.
	/**
	Если параметр нулевой, то текст очищается.
	*/
	void set_short_text_ID(const char *p) {
		if (p) _short_text_ID = p;
		else _short_text_ID.clear();
	}

	//! Возвращает true, если у состояния задан баунд.
	bool has_bound() const {
		return check_flag(QD_OBJ_STATE_FLAG_HAS_BOUND);
	}
	//! Возвращает баунд состояния.
	const Vect3f &bound() const {
		return _bound;
	}
	//! Возвращает радиус состояния.
	float radius() const {
		return _radius;
	}
	//! Устанавливает баунд состояния.
	void set_bound(const Vect3f &b);
	//! Устанавливает баунд состояния по анимации.
	virtual bool auto_bound() {
		return false;
	}

	//! Устанавливает режим работы камеры, включается при активации состояния.
	void set_camera_mode(const qdCameraMode &mode) {
		_camera_mode = mode;
	}
	//! Режим работы камеры, включается при активации состояния.
	const qdCameraMode &camera_mode() const {
		return _camera_mode;
	}
	//! Возвращает true, если у состояния задан режим работы камеры.
	bool has_camera_mode() const {
		return _camera_mode.camera_mode() != qdCameraMode::MODE_UNASSIGNED;
	}

	float rnd_move_radius() const {
		return _rnd_move_radius;
	}
	void set_rnd_move_radius(float radius) {
		_rnd_move_radius = radius;
	}

	float rnd_move_speed() const {
		return _rnd_move_speed;
	}
	void set_rnd_move_speed(float speed) {
		_rnd_move_speed = speed;
	}

	qdConditionalObject::trigger_start_mode trigger_start();
	bool trigger_can_start() const;

	bool forced_load() const {
		return check_flag(QD_OBJ_STATE_FLAG_FORCED_LOAD);
	}

	float text_delay() const {
		return _text_delay;
	}
	bool has_text_delay() const {
		return _text_delay > FLT_EPS;
	}
	void set_text_delay(float delay) {
		_text_delay = delay;
	}

	float sound_delay() const {
		return _sound_delay;
	}
	bool has_sound_delay() const {
		return _sound_delay > FLT_EPS;
	}
	void set_sound_delay(float delay) {
		_sound_delay = delay;
	}

	int autosave_slot() const {
		return _autosave_slot;
	}
	void set_autosave_slot(int slot) {
		_autosave_slot = slot;
	}

	float fade_time() const {
		return _fade_time;
	}
	void set_fade_time(float time) {
		_fade_time = time;
	}

	uint32 shadow_color() const {
		return _shadow_color;
	}
	int shadow_alpha() const {
		return _shadow_alpha;
	}

	void set_shadow(uint32 color, int alpha) {
		_shadow_color = color;
		_shadow_alpha = alpha;
	}

	const qdScreenTextFormat &text_format(bool topic_mode = false) const {
		if (_text_format.is_global_depend()) {
			return (topic_mode && check_flag(QD_OBJ_STATE_FLAG_DIALOG_PHRASE)) ?
			       qdScreenTextFormat::global_topic_format() : qdScreenTextFormat::global_text_format();
		}
		return _text_format;
	}
	void set_text_format(const qdScreenTextFormat &text_format) {
		_text_format = text_format;
	}

	bool has_transform() const {
		return _transform() || _transform_speed();
	}

	const qdScreenTransform &transform() const {
		return _transform;
	}
	void set_transform(const qdScreenTransform &tr) {
		_transform = tr;
	}
	const qdScreenTransform &transform_speed() const {
		return _transform_speed;
	}
	void set_transform_speed(const qdScreenTransform &tr_speed) {
		_transform_speed = tr_speed;
	}

protected:

	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script_body(Common::WriteStream &fh, int indent = 0) const;

	//! Возвращает true, если надо перезапустить звук.
	virtual bool need_sound_restart() const;

	Common::String flag2str(int fl) const;

private:

	//! Тип состояния.
	StateType _state_type;

	//! Смещение центра анимации относительно центра объекта в экранных координатах.
	Vect2s _center_offset;
	//! Баунд.
	Vect3f _bound;
	//! Радиус сферы, описывающей баунд.
	float _radius;

	//! Траектория движения объекта.
	qdCoordsAnimation _coords_animation;

	//! Задержка перед активацией состояния (в секундах)
	float _activation_delay;
	//! Время в секундах, оставшееся до активации состояния.
	float _activation_timer;

	//! Длительность состояния в секундах.
	/**
	Если нулевая, то состояние работает до конца анимации
	или до конца траектории движения.
	*/
	float _work_time;
	//! Время в секундах, прошедшее с момента активации состояния.
	float _cur_time;

	//! Информация о звуке, привязанном к состоянию.
	qdSoundInfo _sound_info;
	//! Хэндл для управления звуком, привязанным к состоянию.
	qdSoundHandle _sound_handle;
	//! Задержка запуска звука от старта состояния (в секундах)
	float _sound_delay;
	//! true, если звук состояния запущен
	bool _is_sound_started;

	//! Задержка перед появлением текста от старта состояния (в секундах)
	float _text_delay;
	//! true, если текст состояния появился
	bool _is_text_shown;

	//! Номер мышиного курсора, который включается, если мышь над объектом в этом состоянии.
	int _mouse_cursor_id;

	//! Текст, выводимый на экран при работе состояния (для диалогов и т.д.)
	Common::String _text_ID;
	//! Короткий вариант текста, выводимого на экран при работе состояния (для диалогов и т.д.)
	Common::String _short_text_ID;

	//! Режим работы камеры, включается при активации состояния.
	qdCameraMode _camera_mode;

	float _rnd_move_radius;
	float _rnd_move_speed;

	/// Номер слота автосэйва
	int _autosave_slot;

	/// Время фэйда экрана при включении состояния
	float _fade_time;

	//! Цвет затенения.
	uint32 _shadow_color;
	//! Прозрачность затенения, значения - [0, 255], если равно QD_NO_SHADOW_ALPHA, то объект не затеняется.
	int _shadow_alpha;

	/// Преобразование картинки объекта, включается при активации состояния
	qdScreenTransform _transform;
	qdScreenTransform _transform_speed;

	//! Формат текста.
	qdScreenTextFormat _text_format;

	//! Количество ссылок на состояние.
	/**
	Если объект, которому принадлежит состояние - глобальный (т.е. принадлежит
	игровому диспетрчеру, а не сцене), то состояние может одновременно находиться в более чем одном списке.
	*/
	int _reference_count;

	//! Предыдущее состояние.
	qdGameObjectState *_prev_state;

};

typedef Std::vector<qdGameObjectState *> qdGameObjectStateVector;

//! Состояние динамического объекта - статическая анимация.
class qdGameObjectStateStatic : public qdGameObjectState {
public:
	qdGameObjectStateStatic();
	qdGameObjectStateStatic(const qdGameObjectStateStatic &st);
	~qdGameObjectStateStatic();

	qdGameObjectState &operator = (const qdGameObjectState &st);
	qdGameObjectStateStatic &operator = (const qdGameObjectStateStatic &st);

	qdAnimation *animation() {
		return _animation_info.animation();
	}
	const qdAnimation *animation() const {
		return _animation_info.animation();
	}
	qdAnimationInfo *animation_info() {
		return &_animation_info;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Регистрация ресурсов состояния в диспетчере ресурсов.
	bool register_resources();
	//! Отмена регистрации ресурсов состояния в диспетчере ресурсов.
	bool unregister_resources();
	bool load_resources();
	bool free_resources();

	bool is_state_empty() const;

	qdGameObjectState *clone() const {
		return new qdGameObjectStateStatic(*this);
	}

	//! Устанавливает баунд состояния по анимации.
	bool auto_bound();

private:
	qdAnimationInfo _animation_info;
};

//! Состояние динамического объекта - походка.
class qdGameObjectStateWalk : public qdGameObjectState {
public:
	qdGameObjectStateWalk();
	qdGameObjectStateWalk(const qdGameObjectStateWalk &st);
	~qdGameObjectStateWalk();

	qdGameObjectState &operator = (const qdGameObjectState &st);
	qdGameObjectStateWalk &operator = (const qdGameObjectStateWalk &st);

	//! режимы передвижения для персонажа
	enum movement_type_t {
		//! ходит только влево
		MOVEMENT_LEFT,
		//! ходит только вверх
		MOVEMENT_UP,
		//! ходит только вправо
		MOVEMENT_RIGHT,
		//! ходит только вниз
		MOVEMENT_DOWN,
		//! ходит только по горизонтали
		MOVEMENT_HORIZONTAL,
		//! ходит только по вертикали
		MOVEMENT_VERTICAL,
		//! ходит по четырем направлениям
		MOVEMENT_FOUR_DIRS,
		//! ходит по восьми направлениям
		MOVEMENT_EIGHT_DIRS,
		//! ходит по восьми и больше направлениям со сглаживанием
		MOVEMENT_SMOOTH,

		//! ходит только влево-вверх
		MOVEMENT_UP_LEFT,
		//! ходит только вправо-вверх
		MOVEMENT_UP_RIGHT,
		//! ходит только вправо-вниз
		MOVEMENT_DOWN_RIGHT,
		//! ходит только влево-вниз
		MOVEMENT_DOWN_LEFT
	};

	enum OffsetType {
		OFFSET_STATIC = 0,
		OFFSET_WALK,
		OFFSET_START,
		OFFSET_END
	};

	qdAnimationSet *animation_set() const;

	qdAnimation *animation(float direction_angle);
	qdAnimation *static_animation(float direction_angle);

	qdAnimationInfo *animation_info(float direction_angle);
	qdAnimationInfo *static_animation_info(float direction_angle);

	const Vect2i &center_offset(int direction_index, OffsetType offset_type = OFFSET_WALK) const;
	const Vect2i &center_offset(float direction_angle, OffsetType offset_type = OFFSET_WALK) const;
	void set_center_offset(int direction_index, const Vect2i &offs, OffsetType offset_type = OFFSET_WALK);

	float walk_sound_frequency(int direction_index) const;
	float walk_sound_frequency(float direction_angle) const;
	void set_walk_sound_frequency(int direction_index, float freq);

	//! Установка частоты звука.
	bool update_sound_frequency(float direction_angle) const;

	qdAnimationSetInfo *animation_set_info() {
		return &_animation_set_info;
	}

	float adjust_direction_angle(float angle) const;

	float direction_angle() const {
		return _direction_angle;
	}
	void set_direction_angle(float ang) {
		_direction_angle = ang;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Регистрация ресурсов состояния в диспетчере ресурсов.
	bool register_resources();
	//! Отмена регистрации ресурсов состояния в диспетчере ресурсов.
	bool unregister_resources();
	bool load_resources();
	bool free_resources();

	bool is_state_empty() const;

	qdGameObjectState *clone() const {
		return new qdGameObjectStateWalk(*this);
	}

	//! Устанавливает баунд состояния по анимации.
	bool auto_bound();

	float acceleration() const {
		return _acceleration;
	}
	void set_acceleration(float acc) {
		_acceleration = acc;
	}

	float max_speed() const {
		return _max_speed;
	}
	void set_max_speed(float max_sp) {
		_max_speed = max_sp;
	}

	void set_movement_type(movement_type_t type) {
		_movement_type = type;
	}
	movement_type_t movement_type() const {
		return _movement_type;
	}

protected:

	//! Возвращает true, если надо перезапустить звук.
	bool need_sound_restart() const;

private:

	float _direction_angle;
	qdAnimationSetInfo _animation_set_info;

	//! Ускорение - насколько увеличивается скорость передвижения за секунду.
	float _acceleration;
	//! Максимальная для походки скорость передвижения.
	/**
	Если нулевая - ограничения нет.
	*/
	float _max_speed;

	//! Режим передвижения персонажа.
	movement_type_t _movement_type;

	//! Смещения центров анимаций походок.
	Std::vector<Vect2i> _center_offsets;
	//! Смещения центров статических анимаций.
	Std::vector<Vect2i> _static_center_offsets;
	//! Смещения центров анимаций стартов.
	Std::vector<Vect2i> _start_center_offsets;
	//! Смещения центров анимаций стопов.
	Std::vector<Vect2i> _stop_center_offsets;

	//! Коэффициенты для частоты звука походки.
	Std::vector<float> _walk_sound_frequency;
};

//! Состояние динамического объекта - маска на статическом объекте.
class qdGameObjectStateMask : public qdGameObjectState, public qdContour {
public:
	qdGameObjectStateMask();
	qdGameObjectStateMask(const qdGameObjectStateMask &st);
	~qdGameObjectStateMask();

	qdGameObjectState &operator = (const qdGameObjectState &st);
	qdGameObjectStateMask &operator = (const qdGameObjectStateMask &st);

	const char *parent_name() const {
		return _parent_name.c_str();
	}
	void set_parent_name(const char *p) {
		_parent_name = p;
		_parent = 0;
	}

	qdGameObject *parent();
	const qdGameObject *parent() const;

	bool hit(int x, int y) const;
	bool draw_mask(uint32 color) const;

	bool can_be_closed() const {
		return (contour_size() > 2);
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool is_state_empty() const;

	bool load_resources();

	qdGameObjectState *clone() const {
		return new qdGameObjectStateMask(*this);
	}

private:

	//! Имя объекта, к которому привязана маска.
	Common::String _parent_name;

	//! Указатель на объект, к которому привязана маска.
	qdGameObject *_parent;
};

#ifdef __QD_DEBUG_ENABLE__
inline bool qdbg_is_object_state(const qdNamedObject *obj, const char *scene_name, const char *object_name, const char *state_name) {
	if (dynamic_cast<const qdGameObjectState *>(obj)) {
		if (obj->name() && !strcmp(state_name, obj->name())) {
			if (!object_name || (obj->owner() && obj->owner()->name() && !strcmp(object_name, obj->owner()->name()))) {
				if (!scene_name || (obj->owner()->owner() && obj->owner()->owner()->name() && !strcmp(obj->owner()->owner()->name(), scene_name)))
					return true;
			}
		}
	}
	return false;
}
#else
inline bool qdbg_is_object_state(const qdNamedObject *obj, const char *scene_name, const char *object_name, const char *state_name) {
	return false;
}
#endif

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_OBJECT_STATE_H
