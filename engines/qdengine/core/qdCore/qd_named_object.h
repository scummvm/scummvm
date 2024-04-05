#ifndef __QD_NAMED_OBJECT_H__
#define __QD_NAMED_OBJECT_H__

#include "qd_named_object_base.h"

//! Типы поименованных объектов.
enum qdNamedObjectType
{
	QD_NAMED_OBJECT_GENERIC,
	QD_NAMED_OBJECT_SCALE_INFO,
	QD_NAMED_OBJECT_TRIGGER_CHAIN,
	QD_NAMED_OBJECT_SOUND,
	QD_NAMED_OBJECT_ANIMATION,
	QD_NAMED_OBJECT_ANIMATION_INFO,
	QD_NAMED_OBJECT_COORDS_ANIMATION,
	QD_NAMED_OBJECT_OBJ_STATE,
	QD_NAMED_OBJECT_STATIC_OBJ,
	QD_NAMED_OBJECT_ANIMATED_OBJ,
	QD_NAMED_OBJECT_MOVING_OBJ,
	QD_NAMED_OBJECT_MOUSE_OBJ,
	QD_NAMED_OBJECT_SCENE,
	QD_NAMED_OBJECT_LOCATION,
	QD_NAMED_OBJECT_DISPATCHER,
	
	QD_NAMED_OBJECT_ANIMATION_SET,
	QD_NAMED_OBJECT_GRID_ZONE,
	QD_NAMED_OBJECT_VIDEO,
	QD_NAMED_OBJECT_INVENTORY,
	QD_NAMED_OBJECT_MINIGAME,
	QD_NAMED_OBJECT_MUSIC_TRACK,
	QD_NAMED_OBJECT_GRID_ZONE_STATE,
	
	QD_NAMED_OBJECT_SOUND_INFO,
	QD_NAMED_OBJECT_ANIMATION_SET_INFO,
	
	QD_NAMED_OBJECT_GAME_END,
	QD_NAMED_OBJECT_COUNTER,

	QD_NAMED_OBJECT_FONT_INFO,

	QD_NAMED_OBJECT_MAX_TYPE
};

//! Поименованный объект.
class qdNamedObject : public qdNamedObjectBase
{
public:
	qdNamedObject();
	qdNamedObject(const qdNamedObject& p);
	~qdNamedObject();

	qdNamedObject& operator = (const qdNamedObject& p);

	//! Возвращает владельца объекта.
	qdNamedObject* owner() const { return owner_; }
	//! Возвращает владельца объекта, тип которого tp.
	qdNamedObject* owner(qdNamedObjectType tp) const;

#ifndef _QUEST_EDITOR
	//! Устанавливает владельца объекта.
	void set_owner(qdNamedObject* p){ owner_ = p; }
#else
	qdNamedObject* ref_owner() const { return ref_owner_; }
	void set_ref_owner(qdNamedObject* p){ ref_owner_ = p; }
	void set_owner(qdNamedObject* p){ owner_ = ref_owner_ = p; }
#endif

	//! Устанавливает флаг.
	void set_flag(int fl){ flags_ |= fl; }
	//! Скидывает флаг.
	void drop_flag(int fl){ flags_ &= ~fl; }
	//! Возвращает true, если установлен флаг fl.
	bool check_flag(int fl) const { if(flags_ & fl) return true; return false; }
	//! Очищает флаги.
	void clear_flags(){ flags_ = 0; }
	//! Возвращает значение флагов объекта.
	int flags() const { return flags_; }

	//! Возвращает тип объекта.
	virtual int named_object_type() const = 0;

	//! Загрузка данных из сэйва.
	virtual bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	virtual bool save_data(qdSaveStream& fh) const;

	//! Добавляет ссылку из триггеров на объект.
	void add_trigger_reference(){ trigger_reference_count_++; }
	//! Удаляет ссылку из триггеров на объект.
	void remove_trigger_reference(){ if(trigger_reference_count_) trigger_reference_count_--; }
	//! Очищает счётчик ссылок из триггеров на объект.
	void clear_trigger_references(){ trigger_reference_count_ = 0; }
	//! Возвращает true, если на объект есть ссылки из триггеров.
	bool is_in_triggers() const { return (trigger_reference_count_ > 0); }

private:

	//! Некие свойства объекта.
	int flags_;

	//! Количество ссылок на объект из триггеров.
	int trigger_reference_count_;

	//! Владелец объекта.
	mutable qdNamedObject* owner_;
#ifdef _QUEST_EDITOR
	mutable qdNamedObject* ref_owner_;
#endif
};

appLog& operator << (appLog& log,const qdNamedObject* obj);

#endif /* __QD_NAMED_OBJECT_H__ */