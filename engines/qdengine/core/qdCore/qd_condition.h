#ifndef __QD_CONDITION_H__
#define __QD_CONDITION_H__

#include "xml_fwd.h"

#include "qd_named_object.h"
#include "qd_condition_data.h"
#include "qd_condition_object_reference.h"

//! Условие.
/**
Структура данных по типам условий.

Индексы для доступа к объектам (если они используются в условии)
совпадают с индексами для доступа к данным об именах объектов.

CONDITION_TRUE
CONDITION_FALSE
Данные: отсутствуют

CONDITION_MOUSE_CLICK
Данные:
	string
Объекты:
	qdGameObject
Индескы данных:
	OBJECT_NAME
	Имя объекта, по которому надо кликнуть.
	Если пустое - проверяется клик по объекту, которому принадлежит
	состояние, в котором выставлено это условие.

CONDITION_MOUSE_OBJECT_CLICK
Данные:
	string, string
Объекты:
	qdGameObject, qdGameObject
Индескы данных:
	OBJECT_NAME, MOUSE_OBJECT_NAME
	Имя объекта, по которому надо кликнуть
	и имя объекта, который должен быть на мыши.

CONDITION_OBJECT_IN_ZONE
Данные:
	string, string
Объекты:
	qdGameObject, qdGridZone
Индескы данных:
	OBJECT_NAME, ZONE_NAME
	Имя объекта и имя зоны на сетке,
	в которую он должен попасть.

CONDITION_PERSONAGE_WALK_DIRECTION
CONDITION_PERSONAGE_STATIC_DIRECTION
Данные:
	string, float[1]
Объекты:
	qdGameObjectMoving
Индескы данных:
	PERSONAGE_NAME, DIRECTION_ANGLE
	Имя персонажа и угол направления в радианах.
	Угол задается так же, как и направления в походках
	(т.е. от направления направо против часовой стрелки)

CONDITION_TIMER
Данные:
	float[2], int[2]
Индескы данных:
	TIMER_PERIOD, TIMER_RND
	Период и текущее время таймера в секундах,
	вероятность выполнения условия по истечении времени
	таймера в процентах и текущее состояние условия.

CONDITION_MOUSE_DIALOG_CLICK
Данные: отсутствуют

CONDITION_MINIGAME_STATE
Данные:
	string, string
Объекты:
	qdMiniGame
Индескы данных:
	MINIGAME_NAME, MINIGAME_STATE_NAME
	Имя мини-игры и имя состояния, в котором она должна находиться
	для выполнения условия.

CONDITION_OBJECT_STATE
Данные:
	string, string
Объекты:
	qdGameObjectAnimated, qdGameObjectState
Индескы данных:
	OBJECT_NAME, OBJECT_STATE_NAME
	Имя объекта и имя состояния, в котором он должен находиться
	для выполнения условия.

CONDITION_MOUSE_ZONE_CLICK
Данные:
	string
Объекты:
	qdGridZone
Индескы данных:
	CLICK_ZONE_NAME
	Имя зоны на сетке,
	в которую надо кликнуть.

CONDITION_MOUSE_OBJECT_ZONE_CLICK
Данные:
	string, string
Объекты:
	qdGridZone, qdGameObject
Индескы данных:
	CLICK_ZONE_NAME, MOUSE_OBJECT_NAME
	Имя зоны на сетке, в которую надо кликнуть
	и имя объекта, который должен быть на мыши.

CONDITION_OBJECT_STATE_WAS_ACTIVATED
CONDITION_OBJECT_STATE_WAS_NOT_ACTIVATED
Данные:
	string, string
Объекты:
	qdGameObjectAnimated, qdGameObjectState
Индескы данных:
	OBJECT_NAME, OBJECT_STATE_NAME
	Имя объекта и имя состояния.

CONDITION_OBJECT_NOT_IN_STATE
см. CONDITION_OBJECT_STATE

CONDITION_OBJECTS_DISTANCE
Данные:
	string, string, float
Объекты:
	qdGameObject, qdGameObject
Индескы данных:
	OBJECT_NAME, OBJECT2_NAME, OBJECTS_DISTANCE
	Имена объектов и расстояние по сетке.

CONDITION_PERSONAGE_ACTIVE
Данные:
	string
Объекты:
	qdGameObjectMoving
Индескы данных:
	PERSONAGE_NAME
	Имя персонажа, который должен быть активным
	для выполнения условия.

CONDITION_OBJECT_STATE_WAITING
Данные:
	string, string
Объекты:
	qdGameObjectAnimated, qdGameObjectState
Индескы данных:
	OBJECT_NAME, OBJECT_STATE_NAME
	Имя объекта и имя состояния.

CONDITION_OBJECT_STATE_ANIMATION_PHASE
Данные:
	string, string, float[2]
Объекты:
	qdGameObjectAnimated, qdGameObjectState
Индескы данных:
	OBJECT_NAME, OBJECT_STATE_NAME, ANIMATION_PHASE
	Имя объекта, имя состояния и диапазон фазы анимации.

CONDITION_OBJECT_PREV_STATE
Данные:
	string, string
Объекты:
	qdGameObjectAnimated, qdGameObjectState
Индескы данных:
	OBJECT_NAME, OBJECT_STATE_NAME
	Имя объекта и имя состояния.

CONDITION_STATE_TIME_GREATER_THAN_VALUE
Данные:
	float
	Время работы состояния.
Объекты:
	qdGameObjectState

CONDITION_STATE_TIME_GREATER_THAN_STATE_TIME
Данные: отсутствуют
Объекты:
	qdGameObjectState, qdGameObjectState
	Состояния, время работы которых сравнивается.

CONDITION_STATE_TIME_IN_INTERVAL
Данные:
	float[2]
	Интервал, в котором должно находиться ремя работы состояния.
Объекты:
	qdGameObjectState

CONDITION_COUNTER_GREATER_THAN_VALUE
CONDITION_COUNTER_LESS_THAN_VALUE
Данные:
	int
	Число, с которым сравнивается значение счетчика.
Объекты:
	qdCounter

CONDITION_COUNTER_GREATER_THAN_COUNTER
Данные: отсутствуют
Объекты:
	qdCounter, qdCounter
	Счетчики, значения которых сравнивается.

CONDITION_COUNTER_IN_INTERVAL
Данные:
	int[2]
	Интервал, в котором должно находиться значение счетчика.
Объекты:
	qdCounter

CONDITION_OBJECT_ON_PERSONAGE_WAY
Данные:
	float[1]
	Максимальное расстояние между объектам и персонажем по сетке.
Объекты:
	qdGameObjectMoving, qdGameObject

CONDITION_KEYPRESS
Данные:
	int[1]
	vkey кнопки.

CONDITION_ANY_PERSONAGE_IN_ZONE
Объекты:
	qdGridZone

CONDITION_MOUSE_RIGHT_CLICK
Объекты:
	qdGameObject
Индескы данных:
	OBJECT_NAME
	Объект, по которому надо кликнуть.
	Если нулевой - проверяется клик по объекту, которому принадлежит
	состояние, в котором выставлено это условие.

CONDITION_MOUSE_RIGHT_OBJECT_CLICK
Объекты:
	qdGameObject, qdGameObject
Индескы данных:
	OBJECT_NAME, MOUSE_OBJECT_NAME
	Объект, по которому надо кликнуть
	и объект, который должен быть на мыши.

CONDITION_MOUSE_RIGHT_ZONE_CLICK
Объекты:
	qdGridZone
Индескы данных:
	CLICK_ZONE_NAME
	Зона на сетке,
	в которую надо кликнуть.

CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK
Объекты:
	qdGridZone, qdGameObject
Индескы данных:
	CLICK_ZONE_NAME, MOUSE_OBJECT_NAME
	Зона на сетке, в которую надо кликнуть
	и объект, который должен быть на мыши.

CONDITION_OBJECT_HIDDEN
Объекты:
	qdGameObject
	Объект, который должен быть скрыт

CONDITION_MOUSE_HOVER
Объекты:
	qdGameObject
	Объект, на который наводится мышь

CONDITION_MOUSE_OBJECT_HOVER
Объекты:
	qdGameObject, qdGameObject
	Объект, на который наводится мышь,
	объект который на мыши

CONDITION_MOUSE_HOVER_ZONE
Объекты:
	qdGridZone
	Зона, на которую наводится мышь

CONDITION_MOUSE_OBJECT_HOVER_ZONE
Объекты:
	qdGridZone, qdGameObject
	Зона, на которую наводится мышь,
	объект который на мыши

CONDITION_MOUSE_CLICK_FAILED
CONDITION_MOUSE_OBJECT_CLICK_FAILED
CONDITION_MOUSE_CLICK_EVENT
//CONDITION_MOUSE_OBJECT_CLICK_EVENT
CONDITION_MOUSE_RIGHT_CLICK_EVENT
//CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT
Данных нет.

CONDITION_MOUSE_OBJECT_CLICK_EVENT
CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT
Объекты:
	qdGameObject
	Объект, которым надо кликнуть для выполнения условия.
	Если нулевй, то достаточно клика любым объектом.

CONDITION_MOUSE_STATE_PHRASE_CLICK
Объекты:
	qdGameObjecState
	Состояние, по фразе которого надо кликнуть.

CONDITION_OBJECT_IS_CLOSER
Объекты:
	qdGameObject, qdGameObject, qdGameObject
	Расстояние от первого объекта до второго должно быть меньше,
	чем расстояние от первого до третьего.

CONDITION_ANIMATED_OBJECT_IDLE_GREATER_THAN_VALUE
Объекты:
	qdGameObjectAnimated, int
	Анимированный объект, который простаивает, и порог времени,
	который должен быть превышен временем ожидания заданного объекта.

CONDITION_ANIMATED_OBJECTS_INTERSECTIONAL_BOUNDS
Объекты:
	qdGameObjectAnimated, qdGameObjectAnimated
	Два объекта, баунды которых пероверяются на пересечение.
*/

class qdCondition
{
public:
	//! Типы условий.
	enum ConditionType {
		//! всегда true
		CONDITION_TRUE,
		//! всегда false
		CONDITION_FALSE,

		//! клик мышью по объекту
		CONDITION_MOUSE_CLICK,
		//! клик объектом на мыши по объекту
		CONDITION_MOUSE_OBJECT_CLICK,
		//! персонаж или объект в определенном месте сетки
		CONDITION_OBJECT_IN_ZONE,
		//! персонаж движется в определенном направлении
		CONDITION_PERSONAGE_WALK_DIRECTION,
		//! персонаж стоит и ориентирован в определенном направлении
		CONDITION_PERSONAGE_STATIC_DIRECTION,
		//! условие выпоняется с некоторой периодичностью
		CONDITION_TIMER,			
		//! клик мышью по фразе в диалоге
		CONDITION_MOUSE_DIALOG_CLICK,
		//! мини-игра находится в определенном состоянии
		CONDITION_MINIGAME_STATE,
		//! объект находится в определенном состоянии
		CONDITION_OBJECT_STATE,
		//! клик мышью по зоне на сетке сцены
		CONDITION_MOUSE_ZONE_CLICK,
		//! клик объектом по зоне на сетке сцены
		CONDITION_MOUSE_OBJECT_ZONE_CLICK,
		//! состояние объекта активировалось
		CONDITION_OBJECT_STATE_WAS_ACTIVATED,
		//! состояние объекта не активировалось
		CONDITION_OBJECT_STATE_WAS_NOT_ACTIVATED,
		//! объект не находится в определенном состоянии
		CONDITION_OBJECT_NOT_IN_STATE,
		//! расстояние по сетке между двумя объектами меньше заданного
		CONDITION_OBJECTS_DISTANCE,
		//! персонаж активен
		CONDITION_PERSONAGE_ACTIVE,
		//! состояние объекта ждет активации
		CONDITION_OBJECT_STATE_WAITING,
		//! состояние в определенной фазе анимации
		CONDITION_OBJECT_STATE_ANIMATION_PHASE,
		//! предыдущее состояние объекта
		CONDITION_OBJECT_PREV_STATE,
		//! время работы состояния больше значения
		CONDITION_STATE_TIME_GREATER_THAN_VALUE,
		//! время работы одного состояния больше времени работы другого
		CONDITION_STATE_TIME_GREATER_THAN_STATE_TIME,
		//! время работы состояния в интервале
		CONDITION_STATE_TIME_IN_INTERVAL,
		//! значние счетчика больше числа
		CONDITION_COUNTER_GREATER_THAN_VALUE,
		//! значние счетчика меньше числа
		CONDITION_COUNTER_LESS_THAN_VALUE,
		//! значние счетчика больше значения другого счетчика
		CONDITION_COUNTER_GREATER_THAN_COUNTER,
		//! значние счетчика в интервале
		CONDITION_COUNTER_IN_INTERVAL,
		//! объект на пути персонажа
		CONDITION_OBJECT_ON_PERSONAGE_WAY,
		//! кнопка нажата
		CONDITION_KEYPRESS,
		//! в зоне хотя бы один персонаж
		CONDITION_ANY_PERSONAGE_IN_ZONE,
		//! правый клик мыши по объекту
		CONDITION_MOUSE_RIGHT_CLICK,
		//! правый клик объектом на мыши по объекту
		CONDITION_MOUSE_RIGHT_OBJECT_CLICK,
		//! правый клик мышью по зоне на сетке сцены
		CONDITION_MOUSE_RIGHT_ZONE_CLICK,
		//! правый клик объектом по зоне на сетке сцены
		CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK,
		//! объект скрыт
		CONDITION_OBJECT_HIDDEN,
		//! мышь наведена на объект
		CONDITION_MOUSE_HOVER,
		//! мышь наведена на объект, на мыши объект
		CONDITION_MOUSE_OBJECT_HOVER,
		//! мышь наведена на зону
		CONDITION_MOUSE_HOVER_ZONE,
		//! мышь наведена на зону, на мыши объект
		CONDITION_MOUSE_OBJECT_HOVER_ZONE,
		//! неудачный клик мыши
		CONDITION_MOUSE_CLICK_FAILED,
		//! неудачный клик объектом на мыши
		CONDITION_MOUSE_OBJECT_CLICK_FAILED,
		//! был клик мыши
		CONDITION_MOUSE_CLICK_EVENT,
		//! был клик объектом на мыши
		CONDITION_MOUSE_OBJECT_CLICK_EVENT,
		//! был клик правой кнопкой мыши
		CONDITION_MOUSE_RIGHT_CLICK_EVENT,
		//! был клик правой кнопкой объектом на мыши
		CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT,
		//! клик по фразе определённого состояния
		CONDITION_MOUSE_STATE_PHRASE_CLICK,
		//! расстояние до одного объекта меньше чем до другого
		CONDITION_OBJECT_IS_CLOSER,
		//! время простоя больше заданного порога
		CONDITION_ANIMATED_OBJECT_IDLE_GREATER_THAN_VALUE,
		//! баунды двух объектов пересекаются
		CONDITION_ANIMATED_OBJECTS_INTERSECTIONAL_BOUNDS,
	};

	//! Индексы для обращения к данным в условиях.
	enum ConditionDataIndex {
		OBJECT_NAME = 0,
		MOUSE_OBJECT_NAME = 1,
		ZONE_NAME = 1,
		DIRECTION_ANGLE = 1,
		PERSONAGE_NAME = 0,
		TIMER_PERIOD = 0,
		TIMER_RND = 1,
		MINIGAME_NAME = 0,
		MINIGAME_STATE_NAME = 1,
		OBJECT_STATE_NAME = 1,
		CLICK_ZONE_NAME = 0,
		OBJECT2_NAME = 1,
		OBJECTS_DISTANCE = 2,
		ANIMATION_PHASE = 2,
		STATE_TIME = 2
	};

	qdCondition();
	qdCondition(ConditionType tp);
	qdCondition(const qdCondition& cnd);
	~qdCondition();

	qdCondition& operator = (const qdCondition& cnd);

	ConditionType type() const { return type_; }
	void set_type(ConditionType tp);

	bool put_value(int idx,const char* str);

	bool is_click_condition() const { 
		return (type_ == CONDITION_MOUSE_CLICK || type_ == CONDITION_MOUSE_DIALOG_CLICK || type_ == CONDITION_MOUSE_ZONE_CLICK);
	}
	bool is_object_click_condition() const {
		return (type_ == CONDITION_MOUSE_OBJECT_CLICK || type_ == CONDITION_MOUSE_OBJECT_ZONE_CLICK);
	}

	// val_index - индекс, по которому пропишется значение 
	// в qdConditionDataInt::data_ или qdConditionDataFloat::data_
	bool put_value(int idx,int val,int val_index = 0);
	bool put_value(int idx,float val,int val_index = 0);

	bool get_value(int idx,const char*& str) const;
	bool get_value(int idx,int& val,int val_index = 0) const;
	bool get_value(int idx,float& val,int val_index = 0) const;

	bool put_object(int idx,const qdNamedObject* obj);
	const qdNamedObject* get_object(int idx) ;

	const qdNamedObject* owner() const { return owner_; }
	void set_owner(const qdNamedObject* p){ owner_ = p; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	void quant(float dt);

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	//! Инициализация условия, вызывается при старте и перезапуске игры.
	bool init();

	bool is_inversed() const { return is_inversed_; }
	void inverse(bool inverse_mode = true){ is_inversed_ = inverse_mode; }

	bool check();

#ifndef _QUEST_EDITOR
	bool is_in_group() const { return is_in_group_; }
	void add_group_reference(){ is_in_group_ = true; }
	void remove_group_reference(){ is_in_group_ = false; }

	static bool successful_click(){ return successful_click_; }
	static bool successful_object_click(){ return successful_object_click_; }
	static void clear_successful_clicks(){ successful_click_ = successful_object_click_ = false; }
#else
	bool init_objects();
#endif

private:

	ConditionType type_;
	const qdNamedObject* owner_;

	typedef std::vector<qdConditionData> data_container_t;
	data_container_t data_;

	typedef std::vector<qdConditionObjectReference> objects_container_t;
	objects_container_t objects_;

	bool is_inversed_;

#ifndef _QUEST_EDITOR
	bool is_in_group_;

	static bool successful_click_;
	static bool successful_object_click_;
#endif

	bool init_data(int data_index,qdConditionData::data_t data_type,int data_size = 0){
		assert(data_index >= 0 && data_index < data_.size());

		data_[data_index].set_type(data_type);
		if(data_size)
			data_[data_index].alloc_data(data_size);

		return true;
	}
};

#endif /* __QD_CONDITION_H__ */
