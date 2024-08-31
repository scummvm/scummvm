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

#ifndef QDENGINE_QDCORE_QD_TRIGGER_CHAIN_H
#define QDENGINE_QDCORE_QD_TRIGGER_CHAIN_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_trigger_element.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdTriggerChain : public qdNamedObject {
public:
	qdTriggerChain();
	~qdTriggerChain();

	int named_object_type() const {
		return QD_NAMED_OBJECT_TRIGGER_CHAIN;
	}

	qdTriggerElementPtr root_element() {
		return &_root;
	}
	qdTriggerElementConstPtr root_element() const {
		return &_root;
	}

	qdTriggerElementPtr add_element(qdNamedObject *p);
	bool remove_element(qdTriggerElementPtr p, bool free_mem = false, bool relink_elements = false);

	bool can_add_element(const qdNamedObject *p) const;
	bool init_elements();

	//! Инициализация для отладочной проверки цепочки.
	/**
	Помечает все ветки до стартовых элементов как выполненные, все
	ветки после стартовых элементов - как невыполненные.
	*/
	bool init_debug_check();

	const qdTriggerElementList &elements_list() const {
		return _elements;
	}
	bool is_element_in_list(const qdNamedObject *p) const;
	bool is_element_in_list(qdTriggerElementConstPtr p) const;

	bool add_link(qdTriggerElementPtr from, qdTriggerElementPtr to, int link_type = 0, bool auto_restart = false);
	bool remove_link(qdTriggerElementPtr from, qdTriggerElementPtr to);

	//! Активирует все линки, идущие от элементов, содержащих объект from.
	bool activate_links(const qdNamedObject *from);

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	void quant(float dt);

	//! Возвращает имя параметра командной строки для отладки тиггеров.
	static const char *debug_comline();

	void reset();

	//! Делает неактивными все триггера объекта.
	bool deactivate_object_triggers(const qdNamedObject *p);

	qdTriggerElementPtr search_element(int id);
private:

	qdTriggerElement _root;
	qdTriggerElementList _elements;

	bool reindex_elements();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_TRIGGER_CHAIN_H
