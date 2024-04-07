#ifndef __QD_TRIGGER_CHAIN_H__
#define __QD_TRIGGER_CHAIN_H__

#include "xml_fwd.h"
#include "qd_trigger_element.h"

class qdTriggerChain : public qdNamedObject
{
public:
	qdTriggerChain();
	~qdTriggerChain();

#ifdef _QUEST_EDITOR
	const RECT& bounding_rect() const{
		return m_rcBound;
	}
	void set_bounding_rect(const RECT& r) const{
		m_rcBound = r;
	}
	const SIZE& gen_layout() const{ 
		return m_szGenLayout;
	}
	const RECT gen_layout_rect() const{
		RECT r = bounding_rect();
		OffsetRect(&r, m_szGenLayout.cx - r.left, m_szGenLayout.cy - r.top);
		return r;
	}

	void set_gen_layout(SIZE sz) const{
		m_szGenLayout = sz;
	}
	const RECT& work_area() const{
		return m_rcWorkArea;
	}
	void set_work_area(const RECT& r) const{
		m_rcWorkArea = r;
	}
#endif
	int named_object_type() const { return QD_NAMED_OBJECT_TRIGGER_CHAIN; }

	qdTriggerElementPtr root_element(){ 
#ifdef _QUEST_EDITOR
		return root_; 
#else
		return &root_; 
#endif // _QUEST_EDITOR
	}
	qdTriggerElementConstPtr root_element() const{ 
#ifdef _QUEST_EDITOR
		return root_; 
#else
		return &root_; 
#endif // _QUEST_EDITOR
	}

	qdTriggerElementPtr add_element(qdNamedObject* p);
	bool remove_element(qdTriggerElementPtr p,bool free_mem = false,bool relink_elements = false);

	bool can_add_element(const qdNamedObject* p) const;
#ifdef _QUEST_EDITOR
	//! используется для undo/redo
	bool add_element(qdTriggerElementPtr p);
#endif // _QUEST_EDITOR
	bool init_elements();

	//! Инициализация для отладочной проверки цепочки.
	/**
	Помечает все ветки до стартовых элементов как выполненные, все
	ветки после стартовых элементов - как невыполненные.
	*/
	bool init_debug_check();

	const qdTriggerElementList& elements_list() const { return elements_; }
	bool is_element_in_list(const qdNamedObject* p) const;
	bool is_element_in_list(qdTriggerElementConstPtr p) const;

	bool add_link(qdTriggerElementPtr from,qdTriggerElementPtr to,int link_type = 0,bool auto_restart = false);
	bool remove_link(qdTriggerElementPtr from,qdTriggerElementPtr to);

	//! Активирует все линки, идущие от элементов, содержащих объект from.
	bool activate_links(const qdNamedObject* from);

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	void quant(float dt);

	//! Возвращает имя параметра командной строки для отладки тиггеров.
	static const char* const debug_comline();

	void reset();

	//! Делает неактивными все триггера объекта.
	bool deactivate_object_triggers(const qdNamedObject* p);

	qdTriggerElementPtr search_element(int id);
#ifdef _QUEST_EDITOR
	qdTriggerElementPtr search_element(const qdNamedObject* pobj) const;
#endif // _QUEST_EDITOR
private:

#ifdef _QUEST_EDITOR
	qdTriggerElementPtr root_;
#else
	qdTriggerElement root_;
#endif // _QUEST_EDITOR
	qdTriggerElementList elements_;

	bool reindex_elements();
#ifdef _QUEST_EDITOR
	//объединение прямоугольников 
	//всех элементов цепочки
	mutable RECT m_rcBound;
	//рабочая область цепочки в окне
	//
	mutable RECT m_rcWorkArea;

	//определяет положение цепочки, когда выводится 
	//несколько цепочек одновременно
	mutable SIZE m_szGenLayout;
#endif
};

#endif __QD_TRIGGER_CHAIN_H__

