
#ifndef __QD_CONDITION_GROUP_H__
#define __QD_CONDITION_GROUP_H__

#include "xml_fwd.h"

//! Группа условий.
class qdConditionGroup
{
public:
	//! Режим проверки условий.
	enum conditions_mode_t {
		//! "И" - должны выполниться все условия.
		CONDITIONS_AND,
		//! "ИЛИ" - достаточно выполнения одного из условий.
		CONDITIONS_OR
	};

	explicit qdConditionGroup(conditions_mode_t md = CONDITIONS_AND);
	qdConditionGroup(const qdConditionGroup& cg);

	qdConditionGroup& operator = (const qdConditionGroup& cg);

	~qdConditionGroup();

#ifdef _QUEST_EDITOR
	typedef std::list<int> conditions_container_t;
#else
	typedef std::vector<int> conditions_container_t;
#endif
	typedef conditions_container_t::const_iterator conditions_iterator_t;

	conditions_iterator_t conditions_begin() const { return conditions_.begin(); }
	conditions_iterator_t conditions_end() const { return conditions_.end(); }

	int conditions_size() const { return conditions_.size(); }

	conditions_mode_t conditions_mode() const { return conditions_mode_; }
	void set_conditions_mode(conditions_mode_t mode){ conditions_mode_ = mode; }

	bool add_condition(int condition_id);
	bool remove_condition(int condition_id);
#ifdef _QUEST_EDITOR
	void remove_all(){
		conditions_.clear();
	}
#endif // _QUEST_EDITOR

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;
	
private:

	conditions_mode_t conditions_mode_;
	conditions_container_t conditions_;
};

#endif /* __QD_CONDITION_GROUP_H__ */

