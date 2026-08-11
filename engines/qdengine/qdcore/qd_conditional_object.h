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

#ifndef QDENGINE_QDCORE_CONDITIONAL_OBJECT_H
#define QDENGINE_QDCORE_CONDITIONAL_OBJECT_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_condition.h"
#include "qdengine/qdcore/qd_condition_group.h"
#include "qdengine/qdcore/qd_named_object.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

//! Объект с условиями.
class qdConditionalObject : public qdNamedObject {
public:
	qdConditionalObject();
	qdConditionalObject(const qdConditionalObject &obj);
	~qdConditionalObject();

	qdConditionalObject &operator = (const qdConditionalObject &obj);

	//! Режим проверки условий.
	enum ConditionsMode {
		//! "И" - должны выполниться все условия.
		CONDITIONS_AND,
		//! "ИЛИ" - достаточно выполнения одного из условий.
		CONDITIONS_OR
	};

	typedef Std::vector<qdCondition> conditions_container_t;
	typedef Std::vector<qdConditionGroup> condition_groups_container_t;

	//! Возвращает true, если условия выполняются.
	virtual bool check_conditions();
	//! Добавляет условие. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition(const qdCondition *p);
	//! Обновляет условие условие с порядковым номером num.
	bool update_condition(int num, const qdCondition &p);

	//! Удаляет условие.
	bool remove_conditon(int idx);

	//! Возвращает указатель на условие.
	const qdCondition *get_condition(int idx = 0) const {
		return &*(_conditions.begin() + idx);
	}

	//! Возвращает количество условий.
	int conditions_count() const {
		return _conditions.size();
	}

	//! Добавляет группу условий. Возвращает поярдковый номер,-1 в случае ошибки.
	int add_condition_group(const qdConditionGroup *p);
	//! Обновляет группу условий с порядковым номером num.
	bool update_condition_group(int num, const qdConditionGroup &p);

	//! Удаляет группу условий.
	bool remove_conditon_group(int idx);

	//! Возвращает указатель на группу условий.
	const qdConditionGroup *get_condition_group(int idx = 0) const {
		return &*(_condition_groups.begin() + idx);
	}

	//! Возвращает количество групп условий.
	int condition_groups_count() const {
		return _condition_groups.size();
	}

	//! Возвращает режим проверки условий.
	ConditionsMode conditions_mode() const {
		return _conditions_mode;
	}
	//! Устанавливает режим проверки условий.
	void set_conditions_mode(ConditionsMode m) {
		_conditions_mode = m;
	}

	//! Возвращает true, если список условий не пустой.
	bool has_conditions() const {
		return !_conditions.empty();
	}

	//! Обсчет логики условий, dt - время в секундах.
	void conditions_quant(float dt);

	//! Инициализация объекта, вызывается при старте и перезепуске игры.
	virtual bool init();

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

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
	bool load_conditions_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_conditions_script(Common::WriteStream &fh, int indent = 0) const;

private:

	//! Логика проверки условий - И/ИЛИ.
	ConditionsMode _conditions_mode;

	//! Условия.
	conditions_container_t _conditions;

	//! Группы условий.
	condition_groups_container_t _condition_groups;

	bool check_group_conditions(const qdConditionGroup &gr);
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_CONDITIONAL_OBJECT_H
