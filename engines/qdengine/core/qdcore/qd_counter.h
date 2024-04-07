#ifndef __QD_COUNTER_H__
#define __QD_COUNTER_H__

#include <vector>

#include "xml_fwd.h"

#include "qd_named_object_reference.h"

class qdGameObjectState;

class qdCounterElement
{
public:
	qdCounterElement();
	~qdCounterElement();
	explicit qdCounterElement(const qdGameObjectState* p,bool inc_value = true);

	bool operator == (const qdGameObjectState* p) const{ return state_ == p; }

	const qdGameObjectState* state() const { return state_; }
	bool increment_value() const { return increment_value_; }

	bool init();
	bool quant();

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;
	
private:

	qdNamedObjectReference state_reference_;
	const qdGameObjectState* state_;
	bool last_state_status_;
	bool increment_value_;
};

//! Счетчик состояний.
class qdCounter : public qdNamedObject
{
public:
	qdCounter();
	~qdCounter();

	//! Флаги счетчика.
	enum {
		//! принимает только положительные значения
		POSITIVE_VALUE			= 0x01
	};

	int named_object_type() const { return QD_NAMED_OBJECT_COUNTER; }

	int value() const { return value_; }
	void set_value(int value);
	void add_value(int value_delta);

	int value_limit() const { return value_limit_; }
	void set_value_limit(int limit){ value_limit_ = limit; }

	typedef std::vector<qdCounterElement> element_container_t;
	const element_container_t& elements() const { return elements_; }

	//! Добавление состояния. Возвращает false, если оно уже добавлено ранее.
	bool add_element(const qdGameObjectState* p,bool inc_value = true);

	bool remove_element(const qdGameObjectState* p);
	bool remove_element(int idx);
#ifdef _QUEST_EDITOR
	void remove_all_elements();
#endif // _QUEST_EDITOR

	void quant();

	void init();

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

protected:
private:

	//! Состояния, которые при включении изменяют значение счетчика.
	element_container_t elements_;

	//! Текущее значение счетчика.
	int value_;
	//! Предельное значение, по достижении которого счетчик скидывается в ноль.
	/**
	Если меньше или равно нулю - не учитывается.
	*/
	int value_limit_;
};

#endif // __QD_COUNTER_H__
