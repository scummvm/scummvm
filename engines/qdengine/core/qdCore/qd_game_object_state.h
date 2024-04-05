#ifndef __QD_GAME_OBJECT_STATE_H__
#define __QD_GAME_OBJECT_STATE_H__

#include "xml_fwd.h"

#include "snd_sound.h"

#include "qd_contour.h"
#include "qd_sound_info.h"
#include "qd_animation_info.h"
#include "qd_animation_set_info.h"
#include "qd_conditional_object.h"
#include "qd_named_object.h"
#include "qd_coords_animation.h"
#include "qd_camera_mode.h"
#include "qd_sound_handle.h"
#include "qd_screen_text.h"
#include "qd_grid_zone.h"

class XStream;

class qdScreenTransform
{
public:
	qdScreenTransform(float angle = 0.f, const Vect2f& scale = Vect2f(1.f, 1.f)) : angle_(angle), scale_(scale) { }

	bool operator == (const qdScreenTransform& trans) const;
	bool operator != (const qdScreenTransform& trans) const { return !(*this == trans); }
	bool operator ()() const { return *this != ID; }

	qdScreenTransform operator * (float value) const { return qdScreenTransform(angle_ * value, scale_ * value); }

	qdScreenTransform& operator += (const qdScreenTransform& trans)
	{
		angle_ += trans.angle_;
		scale_ += trans.scale_;

		return *this;
	}

	bool change(float dt, const qdScreenTransform& target_trans, const qdScreenTransform& speed);

	float angle() const { return angle_; }
	void set_angle(float angle){ angle_ = angle; }

	const Vect2f& scale() const { return scale_; }
	void set_scale(const Vect2f& scale){ scale_ = scale; }
	bool has_scale() const { return fabs(scale_.x - 1.f) > FLT_EPS || fabs(scale_.y - 1.f) > FLT_EPS; }

	/// единичная трансформация
	static qdScreenTransform ID;

private:

	float angle_;
	Vect2f scale_;

	friend XStream& operator < (XStream& fh, const qdScreenTransform& trans);
	friend XStream& operator > (XStream& fh, qdScreenTransform& trans);
};

XStream& operator < (XStream& fh, const qdScreenTransform& trans);
XStream& operator > (XStream& fh, qdScreenTransform& trans);

//! Состояние динамического объекта - базовый класс.
class qdGameObjectState : public qdConditionalObject
{
public:
	static const Vect3f DEFAULT_BOUND;

	//! Флаги состояния.
	enum {
		//! Объект спрятан.
		QD_OBJ_STATE_FLAG_HIDDEN			= 0x01,
		//! Восстанавливать предыдущее состояние по окончании состояния.
		QD_OBJ_STATE_FLAG_RESTORE_PREV_STATE		= 0x04,
		//! Прятать объект по окончании состояния.
		QD_OBJ_STATE_FLAG_HIDE_OBJECT			= 0x08,
		//! Объект, которому принадлежит состояние - глобальный.
		QD_OBJ_STATE_FLAG_GLOBAL_OWNER			= 0x10,
		//! Состояние для инвентори.
		QD_OBJ_STATE_FLAG_INVENTORY			= 0x20,
		//! Положить объект в инвентори по окончании состояния.
		QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY		= 0x40,
		//! Положить в инвентори не удалось.
		QD_OBJ_STATE_FLAG_MOVE_TO_INVENTORY_FAILED	= 0x80,
		//! У состояния задан баунд.
		QD_OBJ_STATE_FLAG_HAS_BOUND			= 0x100,
		//! Активирована задержка перед включением состояния.
		QD_OBJ_STATE_FLAG_ACTIVATION_TIMER		= 0x200,
		//! Задержка перед включением состояния окончилась.
		QD_OBJ_STATE_FLAG_ACTIVATION_TIMER_END		= 0x400,
		//! Заданный для состояния текст - вариант фразы для диалогов.
		/**
		Означает, что одно из условий активации состояния - клик мышью по
		соответствующему тексту на экране.
		*/
		QD_OBJ_STATE_FLAG_DIALOG_PHRASE			= 0x800,
		//! Синхронизировать анимацию по звуку.
		/**
		Когда заканчивается звук, анимация останавливается.
		Работает только для qdGameObjectStateStatic без координатной анимации.
		*/
		QD_OBJ_STATE_FLAG_SOUND_SYNC			= 0x1000,
		//! Разрешить прерывать состояние.
		/**
		Если флаг установлен, то состояние может быть прервано до
		окончания его работы (например для персонажа - кликом мыши).
		*/
		QD_OBJ_STATE_FLAG_ENABLE_INTERRUPT		= 0x2000,
		//! Состояние уже было активировано.
		/**
		Показывает, что состояние уже было хотя бы один раз активировано.
		*/
		QD_OBJ_STATE_FLAG_WAS_ACTIVATED			= 0x4000,
		//! Запретить прерывать подход к стартовой точке состояния.
		/**
		Имеет смысл только если для состояний с подходом к точке запуска
		(need_to_walk() == true).
		*/
		QD_OBJ_STATE_FLAG_DISABLE_WALK_INTERRUPT	= 0x8000,
		//! Включать состояние, когда объект взят на мышь.
		QD_OBJ_STATE_FLAG_MOUSE_STATE			= 0x10000,
		//! Включать состояние, когда мышь над объектом в инвентори.
		QD_OBJ_STATE_FLAG_MOUSE_HOVER_STATE		= 0x20000,
		//! Не вытаскивать объект из инвентори, когда он взят на мышь.
		QD_OBJ_STATE_FLAG_STAY_IN_INVENTORY		= 0x40000,
		/** 
		Принудительно загружать ресурсы состояния в память при заходе на сцену,
		не выгружать их до выхода со сцены.
		*/
		QD_OBJ_STATE_FLAG_FORCED_LOAD			= 0x80000,
		//! Состояние можно скиповать кликом мыши.
		QD_OBJ_STATE_FLAG_ENABLE_SKIP			= 0x100000,
		//! При включении состояния класть объект на зону под мышью.
		QD_OBJ_STATE_FLAG_MOVE_TO_ZONE			= 0x200000,
		//! При включении состояния класть объект в центр объекта под мышью.
		QD_OBJ_STATE_FLAG_MOVE_ON_OBJECT		= 0x400000,
		//! По окончании работы состояния делать персонажа-владельца активным.
		QD_OBJ_STATE_FLAG_ACTIVATE_PERSONAGE		= 0x800000,
		//! Автоматически загружать сэйв при активации состояния.
		QD_OBJ_STATE_FLAG_AUTO_LOAD					= 0x1000000,
		//! Автоматически писать сэйв при активации состояния.
		QD_OBJ_STATE_FLAG_AUTO_SAVE					= 0x2000000,
		QD_OBJ_STATE_FLAG_FADE_IN					= 0x4000000,
		QD_OBJ_STATE_FLAG_FADE_OUT					= 0x8000000,
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
	qdGameObjectState(const qdGameObjectState& st);
	virtual ~qdGameObjectState();

	virtual qdGameObjectState& operator = (const qdGameObjectState& st);

	int named_object_type() const { return QD_NAMED_OBJECT_OBJ_STATE; }

	//! Возвращает указатель на траекторию движения объекта для состояния.
	qdCoordsAnimation* coords_animation(){ return &coords_animation_; }
	//! Возвращает указатель на траекторию движения объекта для состояния.
	const qdCoordsAnimation* coords_animation() const { return &coords_animation_; }

	//! Загрузка данных из скрипта.
	virtual bool load_script(const xml::tag* p) = 0;
	//! Запись данных в скрипт.
	virtual bool save_script(XStream& fh,int indent = 0) const = 0;

	//! Инициализация состояния, вызывается при старте и перезапуске игры.
	bool init();

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

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
	const Vect2s& center_offset() const { return center_offset_; }
	//! Устанавливает смещение центра объекта для состояния (в экранных координатах).
	void set_center_offset(const Vect2s offs){ center_offset_ = offs; }

	//! Логический квант состояния.
	void quant(float dt);

	//! Возвращает указатель на предыдущее состояние.
	qdGameObjectState* prev_state(){ return prev_state_; }
	//! Устанавливает предыдущее состояние.
	void set_prev_state(qdGameObjectState* p){ prev_state_ = p; }

	//! Возвращает тип состояния.
	StateType state_type() const { return state_type_; }

	//! Возвращает идентификатор мышиного курсора для состояния.
	int mouse_cursor_ID() const { return mouse_cursor_id_; }
	//! Устанавливает идентификатор мышиного курсора для состояния.
	/**
	Если должен использоваться курсор по-умолчанию -
	параметр должен быть равен CURSOR_UNASSIGNED.
	*/
	void set_mouse_cursor_ID(int id){ mouse_cursor_id_ = id; }
	//! Возвращает true, если состоянию назначен особый мышиный курсор.
	bool has_mouse_cursor_ID() const { return mouse_cursor_id_ != CURSOR_UNASSIGNED; }

	//! Устанавливает имя звукового эффекта, привязанного к состоянию.
	const char* sound_name() const { return sound_info_.name(); }
	//! Возвращает имя звукового эффекта, привязанного к состоянию.
	void set_sound_name(const char* p){ sound_info_.set_name(p); }
	//! Возвращает указатель на звуковой эффект, привязанный к состоянию.
	qdSound* sound() const;
	//! Возвращает true, если для состояния задан звук.
	bool has_sound() const { if(sound_name()) return true; else return false; }
	//! Возвращает хэндл звука.
	const qdSoundHandle* sound_handle() const { return &sound_handle_; }
	//! Запускает звук, position - стартовяя позиция, от 0.0 до 1.0.
	bool play_sound(float position = 0.0f);
	//! Останавливает звук.
	bool stop_sound() const;
	//! Установка частоты звука.
	bool set_sound_frequency(float frequency_coeff) const;
	bool is_sound_finished() const;
	bool is_sound_playing() const;

	//! Устанавливает флаг для звука.
	void set_sound_flag(int fl){ sound_info_.set_flag(fl); }
	//! Скидывает флаг для звука.
	void drop_sound_flag(int fl){ sound_info_.drop_flag(fl); }
	//! Возвращает true, если установлен флаг для звука.
	bool check_sound_flag(int fl) const { return sound_info_.check_flag(fl); }

	//! Возвращает true, если состояние пустое.
	virtual bool is_state_empty() const;

	//! Возвращает true, если в данный момент состояние включено.
	bool is_active() const;
	//! Возвращает true, если состояние стартовое.
	bool is_default() const;

	//! Возвращает задержку (в секундах) перед активацией состояния.
	float activation_delay() const { return activation_delay_; }
	//! Устанавливает задержку (в секундах) перед активацией состояния.
	void set_activation_delay(float tm){ activation_delay_ = tm; }
	//! Устанавливает таймер перед активацией состояния.
	void set_activation_timer(){ activation_timer_ = activation_delay_; }

	//! Вызывается при активации состояния.
	void start(){ cur_time_ = 0.0f; is_sound_started_ = false; }

	//! Возвращает время в секундах, прошедшее с момента активации состояния.
	float cur_time() const { return cur_time_; }
	//! Возвращает длительность состояния в секундах.
	float work_time() const;
	//! Устанавливает длительность состояния в секундах.
	void set_work_time(float tm){ work_time_ = tm; }
	
	//! Возвращает true, если персонажу требуется подойти к точке включения состояния.
	bool need_to_walk() const { 
		if(!coords_animation_.is_empty() 
			&& coords_animation_.check_flag(QD_COORDS_ANM_OBJECT_START_FLAG)) 
			return true; 
		else 
			return false;
	}

	virtual qdGameObjectState* clone() const = 0;

	virtual float adjust_direction_angle(float angle) const { return angle; }

#ifndef _QUEST_EDITOR
	//! Возвращает координаты точки, в которой должно активироваться состояние.
	const Vect3f& start_pos() const {
		if(!coords_animation_.is_empty()){
			return coords_animation_.get_point(0) -> dest_pos();
		}
		else
			return Vect3f::ZERO;
	}
	//! Возвращает направление объекта в точке, в которой должно активироваться состояние.
	float start_direction_angle() const {
		if(!coords_animation_.is_empty()){
			return coords_animation_.get_point(0) -> direction_angle();
		}
		else
			return -1.0f;
	}
#endif	

	//! Возвращает количество ссылок на состояние.
	int reference_count() const { return reference_count_; }
	//! Инкремент количества ссылок на состояние.
	void inc_reference_count(){ reference_count_++; }
	//! Декремент количества ссылок на состояние.
	void dec_reference_count(){ if(reference_count_) reference_count_--; }

	//! Возвращает true, если у состояния задан текст субтитров.
	bool has_text() const { return (!text_ID_.empty() || !short_text_ID_.empty()); }
	//! Возвращает текст субтитров.
	const char* text() const { if(has_full_text()) return full_text(); else return short_text(); }
	//! Возвращает полный текст субтитров.
	const char* full_text() const;
	//! Возвращает сокращенный текст субтитров.
	const char* short_text() const;

	//! Возвращает true, если у состояния задан текст субтитров.
	bool has_full_text() const { return !text_ID_.empty(); }
	//! Возвращает true, если у состояния задан сокращенный текст субтитров.
	bool has_short_text() const { return !short_text_ID_.empty(); }
	//! Возвращает полный текст субтитров.
	const char* full_text_ID() const { return text_ID_.c_str(); }
	//! Устанавливает полный текст субтитров.
	/**
	Если параметр нулевой, то текст очищается.
	*/
	void set_full_text_ID(const char* p){ if(p) text_ID_ = p; else text_ID_.clear(); }
	//! Возвращает сокращенный текст субтитров.
	const char* short_text_ID() const { return short_text_ID_.c_str(); }
	//! Устанавливает сокращенный текст субтитров.
	/**
	Если параметр нулевой, то текст очищается.
	*/
	void set_short_text_ID(const char* p){ if(p) short_text_ID_ = p; else short_text_ID_.clear(); }

	//! Возвращает true, если у состояния задан баунд.
	bool has_bound() const { return check_flag(QD_OBJ_STATE_FLAG_HAS_BOUND); }
	//! Возвращает баунд состояния.
	const Vect3f& bound() const { return bound_; }
	//! Возвращает радиус состояния.
	float radius() const { return radius_; }
	//! Устанавливает баунд состояния.
	void set_bound(const Vect3f& b);
	//! Устанавливает баунд состояния по анимации.
	virtual bool auto_bound(){ return false; }

	//! Устанавливает режим работы камеры, включается при активации состояния.
	void set_camera_mode(const qdCameraMode& mode){ camera_mode_ = mode; }
	//! Режим работы камеры, включается при активации состояния.
	const qdCameraMode& camera_mode() const { return camera_mode_; }
	//! Возвращает true, если у состояния задан режим работы камеры.
	bool has_camera_mode() const { return camera_mode_.camera_mode() != qdCameraMode::MODE_UNASSIGNED; }

	float rnd_move_radius() const { return rnd_move_radius_; }
	void set_rnd_move_radius(float radius){ rnd_move_radius_ = radius; }

	float rnd_move_speed() const { return rnd_move_speed_; }
	void set_rnd_move_speed(float speed){ rnd_move_speed_ = speed; }

	qdConditionalObject::trigger_start_mode trigger_start();
	bool trigger_can_start() const;

	bool forced_load() const { return check_flag(QD_OBJ_STATE_FLAG_FORCED_LOAD); }

	float text_delay() const { return text_delay_; }
	bool has_text_delay() const { return text_delay_ > FLT_EPS; }
	void set_text_delay(float delay){ text_delay_ = delay; }

	float sound_delay() const { return sound_delay_; }
	bool has_sound_delay() const { return sound_delay_ > FLT_EPS; }
	void set_sound_delay(float delay){ sound_delay_ = delay; }

	int autosave_slot() const {	return autosave_slot_; }
	void set_autosave_slot(int slot){ autosave_slot_ = slot; }

	float fade_time() const { return fade_time_; }
	void set_fade_time(float time){ fade_time_ = time; }

	unsigned shadow_color() const { return shadow_color_; }
	int shadow_alpha() const { return shadow_alpha_; }

	void set_shadow(unsigned color, int alpha){ shadow_color_ = color; shadow_alpha_ = alpha; }

	const qdScreenTextFormat& text_format(bool topic_mode = false) const { 
#ifndef _QUEST_EDITOR
		if(text_format_.is_global_depend()){
			return (topic_mode && check_flag(QD_OBJ_STATE_FLAG_DIALOG_PHRASE)) ? 
				qdScreenTextFormat::global_topic_format() : qdScreenTextFormat::global_text_format();
		}
#endif
		return text_format_; 
	}
	void set_text_format(const qdScreenTextFormat& text_format) { text_format_ = text_format; }

#ifdef _QUEST_EDITOR
	//! Выдает имя курсора. Пустая строка, если установлен курсор по умолчанию
	std::string const& cursor_name() const;
	//! Установка имени курсора
	void set_cursor_name(std::string const& cursor_name);
	//! По имени курсора определяет его номер
	void update_cursor_id(qdGameDispatcher const& gameDispatcher);
#endif // _QUEST_EDITOR

	bool has_transform() const { return transform_() || transform_speed_(); }

	const qdScreenTransform& transform() const { return transform_; }
	void set_transform(const qdScreenTransform& tr){ transform_ = tr; }
	const qdScreenTransform& transform_speed() const { return transform_speed_; }
	void set_transform_speed(const qdScreenTransform& tr_speed){ transform_speed_ = tr_speed; }

protected:

	//! Загрузка данных из скрипта.
	bool load_script_body(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_script_body(XStream& fh,int indent = 0) const;

	//! Возвращает true, если надо перезапустить звук.
	virtual bool need_sound_restart() const;

private:

	//! Тип состояния.
	StateType state_type_;

	//! Смещение центра анимации относительно центра объекта в экранных координатах.
	Vect2s center_offset_;
	//! Баунд.
	Vect3f bound_;
	//! Радиус сферы, описывающей баунд.
	float radius_;

	//! Траектория движения объекта.
	qdCoordsAnimation coords_animation_;

	//! Задержка перед активацией состояния (в секундах)
	float activation_delay_;
	//! Время в секундах, оставшееся до активации состояния.
	float activation_timer_;

	//! Длительность состояния в секундах.
	/**
	Если нулевая, то состояние работает до конца анимации
	или до конца траектории движения.
	*/
	float work_time_;
	//! Время в секундах, прошедшее с момента активации состояния.
	float cur_time_;

	//! Информация о звуке, привязанном к состоянию.
	qdSoundInfo sound_info_;
	//! Хэндл для управления звуком, привязанным к состоянию.
	qdSoundHandle sound_handle_;
	//! Задержка запуска звука от старта состояния (в секундах)
	float sound_delay_;
	//! true, если звук состояния запущен
	bool is_sound_started_;

	//! Задержка перед появлением текста от старта состояния (в секундах)
	float text_delay_;
	//! true, если текст состояния появился
	bool is_text_shown_;

	//! Номер мышиного курсора, который включается, если мышь над объектом в этом состоянии.
	int mouse_cursor_id_;

	//! Текст, выводимый на экран при работе состояния (для диалогов и т.д.)
	std::string text_ID_;
	//! Короткий вариант текста, выводимого на экран при работе состояния (для диалогов и т.д.)
	std::string short_text_ID_;

	//! Режим работы камеры, включается при активации состояния.
	qdCameraMode camera_mode_;

	float rnd_move_radius_;
	float rnd_move_speed_;

	/// Номер слота автосэйва
	int autosave_slot_;

	/// Время фэйда экрана при включении состояния
	float fade_time_;

	//! Цвет затенения.
	unsigned shadow_color_;
	//! Прозрачность затенения, значения - [0, 255], если равно QD_NO_SHADOW_ALPHA, то объект не затеняется.
	int shadow_alpha_;

	/// Преобразование картинки объекта, включается при активации состояния
	qdScreenTransform transform_;
	qdScreenTransform transform_speed_;

	//! Формат текста.
	qdScreenTextFormat text_format_;

	//! Количество ссылок на состояние.
	/**
	Если объект, которому принадлежит состояние - глобальный (т.е. принадлежит
	игровому диспетрчеру, а не сцене), то состояние может одновременно находиться в более чем одном списке.
	*/
	int reference_count_;

	//! Предыдущее состояние.
	qdGameObjectState* prev_state_;

#ifdef _QUEST_EDITOR
	std::string cursor_name_;
#endif // _QUEST_EDITOR
};

typedef std::vector<qdGameObjectState*> qdGameObjectStateVector;

//! Состояние динамического объекта - статическая анимация.
class qdGameObjectStateStatic : public qdGameObjectState
{
public:
	qdGameObjectStateStatic();
	qdGameObjectStateStatic(const qdGameObjectStateStatic& st);
	~qdGameObjectStateStatic();
	
	qdGameObjectState& operator = (const qdGameObjectState& st);
	qdGameObjectStateStatic& operator = (const qdGameObjectStateStatic& st);
	
	qdAnimation* animation(){ return animation_info_.animation(); }
	const qdAnimation* animation() const { return animation_info_.animation(); }
	qdAnimationInfo* animation_info(){ return &animation_info_; }
#ifdef _QUEST_EDITOR
	qdAnimationInfo const* animation_info() const{ return &animation_info_; }
#endif // _QUEST_EDITOR
	
	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Регистрация ресурсов состояния в диспетчере ресурсов.
	bool register_resources();
	//! Отмена регистрации ресурсов состояния в диспетчере ресурсов.
	bool unregister_resources();
	bool load_resources();
	bool free_resources();
	
	bool is_state_empty() const;

	qdGameObjectState* clone() const {
		return new qdGameObjectStateStatic(*this);
	}

	//! Устанавливает баунд состояния по анимации.
	bool auto_bound();

private:
	qdAnimationInfo animation_info_;
};

//! Состояние динамического объекта - походка.
class qdGameObjectStateWalk : public qdGameObjectState
{
public:
	qdGameObjectStateWalk();
	qdGameObjectStateWalk(const qdGameObjectStateWalk& st);
	~qdGameObjectStateWalk();
	
	qdGameObjectState& operator = (const qdGameObjectState& st);
	qdGameObjectStateWalk& operator = (const qdGameObjectStateWalk& st);

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

	enum OffsetType
	{
		OFFSET_STATIC = 0,
		OFFSET_WALK,
		OFFSET_START,
		OFFSET_END
	};

	qdAnimationSet* animation_set() const;

	qdAnimation* animation(float direction_angle);
	qdAnimation* static_animation(float direction_angle);

	qdAnimationInfo* animation_info(float direction_angle);
	qdAnimationInfo* static_animation_info(float direction_angle);

	const Vect2i& center_offset(int direction_index, OffsetType offset_type = OFFSET_WALK) const;
	const Vect2i& center_offset(float direction_angle, OffsetType offset_type = OFFSET_WALK) const;
	void set_center_offset(int direction_index,const Vect2i& offs, OffsetType offset_type = OFFSET_WALK);

	const float walk_sound_frequency(int direction_index) const;
	const float walk_sound_frequency(float direction_angle) const;
	void set_walk_sound_frequency(int direction_index,float freq);

	//! Установка частоты звука.
	bool update_sound_frequency(float direction_angle) const;

	qdAnimationSetInfo* animation_set_info(){ return &animation_set_info_; }

	float adjust_direction_angle(float angle) const;

	float direction_angle() const { return direction_angle_; }
	void set_direction_angle(float ang){ direction_angle_ = ang; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Регистрация ресурсов состояния в диспетчере ресурсов.
	bool register_resources();
	//! Отмена регистрации ресурсов состояния в диспетчере ресурсов.
	bool unregister_resources();
	bool load_resources();
	bool free_resources();

	bool is_state_empty() const;

	qdGameObjectState* clone() const {
		return new qdGameObjectStateWalk(*this);
	}
	
	//! Устанавливает баунд состояния по анимации.
	bool auto_bound();

	float acceleration() const { return acceleration_; }
	void set_acceleration(float acc){ acceleration_ = acc; }

	float max_speed() const { return max_speed_; }
	void set_max_speed(float max_sp){ max_speed_ = max_sp; }

	void set_movement_type(movement_type_t type){ movement_type_ = type; }
	movement_type_t movement_type() const { return movement_type_; }

protected:

	//! Возвращает true, если надо перезапустить звук.
	bool need_sound_restart() const;

private:

	float direction_angle_;
	qdAnimationSetInfo animation_set_info_;

	//! Ускорение - насколько увеличивается скорость передвижения за секунду.
	float acceleration_;
	//! Максимальная для походки скорость передвижения.
	/**
	Если нулевая - ограничения нет.
	*/
	float max_speed_;

	//! Режим передвижения персонажа.
	movement_type_t movement_type_;

	//! Смещения центров анимаций походок.
	std::vector<Vect2i> center_offsets_;
	//! Смещения центров статических анимаций.
	std::vector<Vect2i> static_center_offsets_;
	//! Смещения центров анимаций стартов.
	std::vector<Vect2i> start_center_offsets_;
	//! Смещения центров анимаций стопов.
	std::vector<Vect2i> stop_center_offsets_;

	//! Коэффициенты для частоты звука походки.
	std::vector<float> walk_sound_frequency_;
};

//! Состояние динамического объекта - маска на статическом объекте.
class qdGameObjectStateMask : public qdGameObjectState, public qdContour
{
public:
	qdGameObjectStateMask();
	qdGameObjectStateMask(const qdGameObjectStateMask& st);
	~qdGameObjectStateMask();

	qdGameObjectState& operator = (const qdGameObjectState& st);
	qdGameObjectStateMask& operator = (const qdGameObjectStateMask& st);

	const char* parent_name() const { return parent_name_.c_str(); }
	void set_parent_name(const char* p){ parent_name_ = p; parent_ = 0; }

	qdGameObject* parent();
	const qdGameObject* parent() const;

	bool hit(int x,int y) const;
	bool draw_mask(unsigned color) const;

	bool can_be_closed() const { return (contour_size() > 2); }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	bool is_state_empty() const;

	bool load_resources();

	qdGameObjectState* clone() const {
		return new qdGameObjectStateMask(*this);
	}
	
#ifdef _QUEST_EDITOR
	void copy_contour(qdGameObjectStateMask const* source);
#endif // _QUEST_EDITOR

private:

	//! Имя объекта, к которому привязана маска.
	std::string parent_name_;

	//! Указатель на объект, к которому привязана маска.
	qdGameObject* parent_;
};

#ifdef __QD_DEBUG_ENABLE__
inline bool qdbg_is_object_state(const qdNamedObject* obj,const char* scene_name,const char* object_name,const char* state_name)
{
	if(dynamic_cast<const qdGameObjectState*>(obj)){
		if(obj -> name() && !strcmp(state_name,obj -> name())){
			if(!object_name || (obj -> owner() && obj -> owner() -> name() && !strcmp(object_name,obj -> owner() -> name()))){
				if(!scene_name || (obj -> owner() -> owner() && obj -> owner() -> owner() -> name() && !strcmp(obj -> owner() -> owner() -> name(),scene_name)))
					return true;
			}
		}
	}
	return false;
}
#else
inline bool qdbg_is_object_state(const qdNamedObject* obj,const char* scene_name,const char* object_name,const char* state_name){ return false; }
#endif

#endif /* __QD_GAME_OBJECT_STATE_H__ */

