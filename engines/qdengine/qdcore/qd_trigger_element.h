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

#ifndef QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_H
#define QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_named_object_reference.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

class qdTriggerElement;
class qdTriggerLink;

typedef qdTriggerElement *qdTriggerElementPtr;
typedef qdTriggerElement const *qdTriggerElementConstPtr;

//! Связь между двумя элементами триггера.
/**
Управляет активацией элемента триггера.
*/
class qdTriggerLink {
public:
	//! Состояние связи.
	enum LinkStatus {
		//! Cвязь выключена.
		LINK_INACTIVE,
		//! Связь включена.
		/**
		    Элемент триггера проверяет условия и активируется, если они выполняются.
		*/
		LINK_ACTIVE,
		//! Связь отработана.
		/**
		    Элемент триггера активировался и завершил работу.
		*/
		LINK_DONE
	};

	qdTriggerLink(qdTriggerElementPtr p, int tp = 0);
	qdTriggerLink();
	~qdTriggerLink() { }

	//! Возвращает состояние связи.
	LinkStatus status() const {
		return _status;
	}
	//! Устанавливает состояние связи.
	void set_status(LinkStatus st) {
		_status = st;
	}

	//! Возвращает тип связи.
	int type() const {
		return _type;
	}
	//! Устанавливает тип связи.
	void set_type(int _t) {
		_type = _t;
	}

	//! Возвращает элемент триггера, к которому идет связь.
	qdTriggerElementPtr const &element() const {
		return _element;
	}
	//! Устанавливает элемент триггера, к которому идет связь.
	void set_element(qdTriggerElementPtr const &el) {
		_element = el;
	}

	//! Возвращает идентификатор элемента, к которому идет связь.
	int element_ID() const {
		return _element_ID;
	}
	//! Устанавливает идентификатор элемента, к которому идет связь.
	/**
	Работает только если сам элемент нулевой.
	*/
	bool set_element_ID(int id) {
		if (!_element) {
			_element_ID = id;
			return true;
		}
		return false;
	}

	bool operator == (qdTriggerElementConstPtr e) const {
		return (_element == e);
	}

	//! Активирует связь.
	void activate();
	//! Деактивирует связь.
	void deactivate();

	//! Устанавливает, надо ли автоматом перезапускать линк.
	void toggle_auto_restart(bool state) {
		_auto_restart = state;
	}
	//! Возвращает true, если надо автоматом перезапускать линк.
	bool auto_restart() const {
		return _auto_restart;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	Common::String toString();

private:
	//! Тип связи.
	int _type;
	//! Элемент, к которому направлена связь.
	qdTriggerElementPtr _element;
	//! Идентификатор элемента, к которому направлена связь.
	int _element_ID;

	//! Состояние связи.
	LinkStatus _status;

	//! Если true, линк автоматом активируется после выключения.
	bool _auto_restart;

};

typedef Std::vector<qdTriggerLink> qdTriggerLinkList;

//! Элемент триггера.
class qdTriggerElement {
public:
	qdTriggerElement();
	qdTriggerElement(qdNamedObject *p);
	~qdTriggerElement();

	enum {
		ROOT_ID = -1,
		INVALID_ID = -2
	};

	enum ElementStatus {
		TRIGGER_EL_INACTIVE,
		TRIGGER_EL_WAITING,
		TRIGGER_EL_WORKING,
		TRIGGER_EL_DONE
	};


	qdTriggerLink *find_child_link(qdTriggerElementConstPtr ptrChild);
	qdTriggerLink *find_child_link(int child_id);

	qdTriggerLink *find_parent_link(qdTriggerElementConstPtr ptrParent);
	qdTriggerLink *find_parent_link(int parent_id);

	//! Возвращает true, если элемент помечен как активный для отладочной проверки.
	bool is_active() const {
		return _is_active;
	}
	//! Помечает элемент как активный (или неактивный) для отладочной проверки.
	void make_active(bool v) {
		_is_active = v;
	}

	//! Инициализация для отладочной проверки.
	/**
	Включает связи, идущие к элементу, и
	помечает как невыполненные все ветки после элемента.
	*/
	bool debug_set_active();
	//! Инициализация для отладочной проверки.
	/**
	Помечает сам элемент и связи, идущие к нему, как выполненные.
	*/
	bool debug_set_done();
	//! Инициализация для отладочной проверки.
	/**
	Помечает сам элемент и связи, идущие к нему, как невыполненные.
	*/
	bool debug_set_inactive();

	bool check_external_conditions(int link_type);
	bool check_internal_conditions();

	ElementStatus status() const {
		return _status;
	}
	void set_status(ElementStatus st);
//	void set_status(ElementStatus st){ _status = st; }

	int ID() const {
		return _ID;
	}
	void set_id(int id) {
		_ID = id;
	}

	qdNamedObject *object() const {
		return _object;
	}
	bool add_object_trigger_reference();
	bool clear_object_trigger_references();

	bool retrieve_object(const qdNamedObjectReference &ref);
	bool retrieve_link_elements(qdTriggerChain *p);

	qdTriggerLinkList &parents()  {
		return _parents;
	}
	qdTriggerLinkList &children() {
		return _children;
	}

	const qdTriggerLinkList &parents()  const {
		return _parents;
	}
	const qdTriggerLinkList &children() const {
		return _children;
	}

	bool is_parent(qdTriggerElementConstPtr p);
	bool is_child(qdTriggerElementConstPtr p);

	bool add_parent(qdTriggerElementPtr p, int link_type = 0);
	bool add_child(qdTriggerElementPtr p, int link_type = 0, bool auto_restart = false);

	bool remove_parent(qdTriggerElementPtr p);
	bool remove_child(qdTriggerElementPtr p);

	bool set_child_link_status(qdTriggerElementConstPtr child, qdTriggerLink::LinkStatus st);
	bool set_parent_link_status(qdTriggerElementConstPtr parent, qdTriggerLink::LinkStatus st);

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(Common::SeekableReadStream &fh, int save_version);
	//! Запись данных в сэйв.
	bool save_data(Common::WriteStream &fh) const;

	bool quant(float dt);

	void reset();
	void deactivate(const qdNamedObject *ignore_object = NULL);

private:

	//! Специальные состояния - используются только в сэйве.
	enum ElementStatusSpecial {
		//! неактивен сам триггер и все связи от него и к нему
		TRIGGER_EL_INACTIVE_ALL,
		//! выполнен сам триггер и все связи от него и к нему
		TRIGGER_EL_DONE_ALL
	};

	int _ID;

	ElementStatus _status;

	//! Является ли элемент активным (для отладки, при проверке триггеров).
	bool _is_active;

	qdNamedObject *_object;

	qdTriggerLinkList _parents;
	qdTriggerLinkList _children;

	bool load_links_script(const xml::tag *p, bool load_parents);

	bool activate_links(qdTriggerElementPtr child);
	bool deactivate_links(qdTriggerElementPtr child);

	bool deactivate_link(qdTriggerElementPtr child);
	bool conditions_quant(int link_type);
	void start();
};

typedef Std::vector<qdTriggerElementPtr> qdTriggerElementList;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_TRIGGER_ELEMENT_H
