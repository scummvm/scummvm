#ifndef __QD_TRIGGER_ELEMENT_H__
#define __QD_TRIGGER_ELEMENT_H__

#include "xml_fwd.h"
#include "qd_named_object.h"
#include "qd_named_object_reference.h"
#include "qd_trigger_profiler.h"

#ifdef _QUEST_EDITOR
#include "SharedCounter.hpp"
#include <string>
#include <map>
#endif // _QUEST_EDITOR

class qdTriggerElement;
class qdTriggerLink;

#ifdef _QUEST_EDITOR
#include <boost/intrusive_ptr.hpp>
typedef boost::intrusive_ptr<qdTriggerElement> qdTriggerElementPtr;
typedef boost::intrusive_ptr<qdTriggerElement const> qdTriggerElementConstPtr;
#else
typedef qdTriggerElement* qdTriggerElementPtr;
typedef qdTriggerElement const* qdTriggerElementConstPtr;
#endif // _QUEST_EDITOR

//! Связь между двумя элементами триггера.
/**
Управляет активацией элемента триггера.
*/
class qdTriggerLink
{
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

	qdTriggerLink(qdTriggerElementPtr p,int tp = 0);
	qdTriggerLink();
	~qdTriggerLink(){ }

	//! Возвращает состояние связи.
	LinkStatus status() const { return status_; }
	//! Устанавливает состояние связи.
	void set_status(LinkStatus st){ status_ = st; }

	//! Возвращает тип связи.
	int type() const { return type_; }
	//! Устанавливает тип связи.
	void set_type(int _t) { type_ = _t; }

	//! Возвращает элемент триггера, к которому идет связь.
	qdTriggerElementPtr const& element() const { return element_; }
	//! Устанавливает элемент триггера, к которому идет связь.
	void set_element(qdTriggerElementPtr const& el){ element_ = el; }

	//! Возвращает идентификатор элемента, к которому идет связь.
	int element_ID() const { return element_ID_; }
	//! Устанавливает идентификатор элемента, к которому идет связь.
	/**
	Работает только если сам элемент нулевой.
	*/
	bool set_element_ID(int id){
		if(!element_){
			element_ID_ = id;
			return true;
		}
		return false;
	}

	bool operator == (qdTriggerElementConstPtr e) const {
		return (element_ == e);
	}

#ifdef _QUEST_EDITOR
	const SIZE& get_owner_offset() const{
		return m_owner_offset;
	}
	void set_owner_offset(const SIZE& sz){
		m_owner_offset = sz;
	}
	void set_owner_offset(int x,int y){
		m_owner_offset.cx = x;
		m_owner_offset.cy = y;
	}
	
	const SIZE& get_child_offset() const{
		return m_child_offset;
	}
	void set_child_offset(const SIZE& sz){
		m_child_offset = sz;
	}
	void set_child_offset(int x,int y){
		m_child_offset.cx = x;
		m_child_offset.cy = y;
	}
#endif
	//! Активирует связь.
	void activate();
	//! Деактивирует связь.
	void deactivate();

	//! Устанавливает, надо ли автоматом перезапускать линк.
	void toggle_auto_restart(bool state){ auto_restart_ = state; }
	//! Возвращает true, если надо автоматом перезапускать линк.
	bool auto_restart() const { return auto_restart_; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

private:
	//! Тип связи.
	int type_;
	//! Элемент, к которому направлена связь.
	qdTriggerElementPtr element_;
	//! Идентификатор элемента, к которому направлена связь.
	int element_ID_;

	//! Состояние связи.
	LinkStatus status_;

	//! Если true, линк автоматом активируется после выключения.
	bool auto_restart_;

#ifdef _QUEST_EDITOR
	//отступ от центра прямоугольника в родительском элементе
	SIZE m_owner_offset;
	//отступ от центра прямоугольника в дочернем элементе
	SIZE m_child_offset;
#endif // _QUEST_EDITOR
};

typedef std::vector<qdTriggerLink> qdTriggerLinkList;

//! Элемент триггера.
class qdTriggerElement
#ifdef _QUEST_EDITOR
	: public SharedCounter
#endif // _QUEST_EDITOR
{
public:
	qdTriggerElement();
	qdTriggerElement(qdNamedObject* p);
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

#ifdef _QUEST_EDITOR
	const std::string& title() const {
		return m_strTitle;
	}
	void set_title(const std::string& strTitle){
		m_strTitle = strTitle;
	}
	
	const RECT& bounding_rect() const{
		return m_rcBound;
	}
	void set_bounding_rect(const RECT& r){
		memcpy(&m_rcBound, &r, sizeof(r));
	}
	
	bool in_bound(const POINT& p){
		return static_cast<bool>(PtInRect(&m_rcBound, p));
	}
	
	const POINT left_top() const{
		POINT p = {m_rcBound.left, m_rcBound.top};
		return p;
	}
	void set_cell_number(int x, int y){
		m_vCellNumber.x = x;
		m_vCellNumber.y = y;
	}

	const POINT& cell_number() const {
		return m_vCellNumber;
	}

	void select(bool bs){
		m_bSelected = bs;
	}
	bool selected() const{
		return m_bSelected;
	}
	bool update_object_reference();
	void update_title();
#endif //_QUEST_EDITOR

	qdTriggerLink* find_child_link(qdTriggerElementConstPtr ptrChild);
	qdTriggerLink* find_child_link(int child_id);

	qdTriggerLink* find_parent_link(qdTriggerElementConstPtr ptrParent);
	qdTriggerLink* find_parent_link(int parent_id);

	//! Возвращает true, если элемент помечен как активный для отладочной проверки.
	bool is_active() const { return is_active_; }
	//! Помечает элемент как активный (или неактивный) для отладочной проверки.
	void make_active(bool v){ is_active_ = v; }

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

	ElementStatus status() const { return status_; }
	void set_status(ElementStatus st);
//	void set_status(ElementStatus st){ status_ = st; }

	int ID() const { return ID_; }
	void set_id(int id){ ID_ = id; }

	qdNamedObject* object() const { return object_; }
	bool add_object_trigger_reference();
	bool clear_object_trigger_references();

#ifdef _QUEST_EDITOR
	qdNamedObjectReference* object_reference(){ return &object_reference_; }
#endif

	bool retrieve_object(const qdNamedObjectReference& ref);
	bool retrieve_link_elements(qdTriggerChain* p);

	qdTriggerLinkList& parents()  { return parents_; }
	qdTriggerLinkList& children() { return children_; }

	const qdTriggerLinkList& parents()  const { return parents_; }
	const qdTriggerLinkList& children() const { return children_; }

	bool is_parent(qdTriggerElementConstPtr p);
	bool is_child(qdTriggerElementConstPtr p);

	bool add_parent(qdTriggerElementPtr p,int link_type = 0);
	bool add_child(qdTriggerElementPtr p,int link_type = 0,bool auto_restart = false);

	bool remove_parent(qdTriggerElementPtr p);
	bool remove_child(qdTriggerElementPtr p);

	bool set_child_link_status(qdTriggerElementConstPtr child,qdTriggerLink::LinkStatus st);
	bool set_parent_link_status(qdTriggerElementConstPtr parent,qdTriggerLink::LinkStatus st);

#ifdef _QUEST_EDITOR
	bool set_parent_link_owner_offset(qdTriggerElementConstPtr el,int x,int y);
	bool set_parent_link_child_offset(qdTriggerElementConstPtr el,int x,int y);
	bool set_child_link_owner_offset(qdTriggerElementConstPtr el,int x,int y);
	bool set_child_link_child_offset(qdTriggerElementConstPtr el,int x,int y);

	void clear_parents();
	void clear_children();
#endif

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool quant(float dt);

	void reset();
	void deactivate(const qdNamedObject* ignore_object = NULL);

#ifdef __QD_TRIGGER_PROFILER__
	void set_owner(const qdTriggerChain* p){ owner_ = p; }
#endif

private:
	
	//! Специальные состояния - используются только в сэйве.
	enum ElementStatusSpecial {
		//! неактивен сам триггер и все связи от него и к нему
		TRIGGER_EL_INACTIVE_ALL,
		//! выполнен сам триггер и все связи от него и к нему
		TRIGGER_EL_DONE_ALL
	};
	
	int ID_;

	ElementStatus status_;

	//! Является ли элемент активным (для отладки, при проверке триггеров).
	bool is_active_;
	
	qdNamedObject* object_;

#ifdef _QUEST_EDITOR
	qdNamedObjectReference object_reference_;
#endif
	
	qdTriggerLinkList parents_;
	qdTriggerLinkList children_;

#ifdef __QD_TRIGGER_PROFILER__
	const qdTriggerChain* owner_;
#endif

#ifdef _QUEST_EDITOR

	//передаем элемент, на который указывает линк
	//потому что он его однозначно идентифицирует

	RECT m_rcBound;		// прямоугольник, охватывающий текст
	std::string m_strTitle;	// то что отображаем на экране
	bool m_bSelected;
	POINT m_vCellNumber;	//номер ячейки по х и по у.
#endif //_QUEST_EDITOR

	bool load_links_script(const xml::tag* p,bool load_parents);
	
	bool activate_links(qdTriggerElementPtr child);
	bool deactivate_links(qdTriggerElementPtr child);

	bool deactivate_link(qdTriggerElementPtr child);
	bool conditions_quant(int link_type);
	void start();
};

#include "qd_trigger_element_boost.h"


typedef std::vector<qdTriggerElementPtr> qdTriggerElementList;

#endif /* __QD_TRIGGER_ELEMENT_H__ */
