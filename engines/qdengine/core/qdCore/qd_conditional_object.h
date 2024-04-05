#ifndef __CONDITIONAL_OBJECT_H__
#define __CONDITIONAL_OBJECT_H__

#include "xml_fwd.h"

#include "qd_condition.h"
#include "qd_condition_group.h"
#include "qd_named_object.h"

//! Объект с условиями.
class qdConditionalObject : public qdNamedObject
{
public:
	qdConditionalObject();
	qdConditionalObject(const qdConditionalObject& obj);
	~qdConditionalObject();

	qdConditionalObject& operator = (const qdConditionalObject& obj);

	//! Режим проверки условий.
	enum ConditionsMode {
		//! "И" - должны выполниться все условия.
		CONDITIONS_AND,
		//! "ИЛИ" - достаточно выполнения одного из условий.
		CONDITIONS_OR
	};

	typedef std::vector<qdCondition> conditions_container_t;
	typedef std::vector<qdConditionGroup> condition_groups_container_t;

	//! Возвращает true, если условия выполняются.
	virtual bool check_conditions();
	//! Добавляет условие. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition(const qdCondition* p);
	//! Обновляет условие условие с порядковым номером num.
	bool update_condition(int num,const qdCondition& p);

	//! Удаляет условие.
	bool remove_conditon(int idx);

#ifdef _QUEST_EDITOR
	//! Удаляет все условия
	void remove_all_conditions();
	//! Удаляет все группы
	void remove_all_groups();
#endif //_QUEST_EDITOR

	//! Возвращает указатель на условие.
	const qdCondition* get_condition(int idx = 0) const { return &*(conditions_.begin() + idx); }

	//! Возвращает количество условий.
	int conditions_count() const { return conditions_.size(); }

	//! Добавляет группу условий. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition_group(const qdConditionGroup* p);
	//! Обновляет группу условий с порядковым номером num.
	bool update_condition_group(int num,const qdConditionGroup& p);

	//! Удаляет группу условий.
	bool remove_conditon_group(int idx);

	//! Возвращает указатель на группу условий.
	const qdConditionGroup* get_condition_group(int idx = 0) const { return &*(condition_groups_.begin() + idx); }

	//! Возвращает количество групп условий.
	int condition_groups_count() const { return condition_groups_.size(); }

	//! Возвращает режим проверки условий.
	ConditionsMode conditions_mode() const { return conditions_mode_; }
	//! Устанавливает режим проверки условий.
	void set_conditions_mode(ConditionsMode m){ conditions_mode_ = m; }

	//! Возвращает true, если список условий не пустой.
	bool has_conditions() const { return !conditions_.empty(); }

	//! Обсчет логики условий, dt - время в секундах.
	void conditions_quant(float dt);

#ifdef _QUEST_EDITOR
	bool backup_object();
	bool restore_object();
#endif

	//! Инициализация объекта, вызывается при старте и перезепуске игры.
	virtual bool init();

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

#ifdef _QUEST_EDITOR
	bool init_objects();
	static void toggle_global_list(bool state){ 
		enable_objects_list_ = state; 
		if(!state)
			objects_list_.clear();
	}
	static void global_init();
#endif

	bool is_condition_in_group(int condition_idx) const;

	//! Варианты старта триггера.
	enum trigger_start_mode {
		//! запуск не удался
		TRIGGER_START_FAILED,
		//! включить активный режим у триггера
		TRIGGER_START_ACTIVATE,
		//! включить ждущий режим у триггера
		TRIGGER_START_WAIT
	};
	//! Активация триггера.
	virtual trigger_start_mode trigger_start() = 0;

	//! Возвращает true, если триггер может быть активирован при выполнении его внутренних условий.
	/**
	Объект должен принадлежать активной сцене (игровые объекты, зоны на сетке, музыка)
	или не зависеть от нее (сцены, мини-игры и т.д.).
	*/
	virtual bool trigger_can_start() const;

protected:

	//! Загрузка данных из скрипта.
	bool load_conditions_script(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_conditions_script(XStream& fh,int indent = 0) const;

private:

	//! Логика проверки условий - И/ИЛИ.
	ConditionsMode conditions_mode_;

	//! Условия.
	conditions_container_t conditions_;

	//! Группы условий.
	condition_groups_container_t condition_groups_;

#ifdef _QUEST_EDITOR
	static conditions_container_t backup_;
	static condition_groups_container_t groups_backup_;
	static ConditionsMode backup_mode_;

	//! Список всех объектов (для инициализации в редакторе).
	static std::list<qdConditionalObject*> objects_list_;
	static bool enable_objects_list_;
#endif

	bool check_group_conditions(const qdConditionGroup& gr);
};

#endif /* __CONDITIONAL_OBJECT_H__ */