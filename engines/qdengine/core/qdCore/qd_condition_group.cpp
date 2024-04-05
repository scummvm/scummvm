/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_condition_group.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdConditionGroup::qdConditionGroup(conditions_mode_t md) : conditions_mode_(md)
{
}

qdConditionGroup::qdConditionGroup(const qdConditionGroup& cg) : conditions_mode_(cg.conditions_mode_), conditions_(cg.conditions_)
{
}

qdConditionGroup::~qdConditionGroup()
{
}

qdConditionGroup& qdConditionGroup::operator = (const qdConditionGroup& cg)
{
	if(this == &cg) return *this;

	conditions_mode_ = cg.conditions_mode_;
	conditions_ = cg.conditions_;

	return *this;
}

bool qdConditionGroup::add_condition(int condition_id)
{
	conditions_container_t::iterator it = std::find(conditions_.begin(),conditions_.end(),condition_id);
	if(it != conditions_.end())
		return false;

	conditions_.push_back(condition_id);
	return true;
}

bool qdConditionGroup::remove_condition(int condition_id)
{
	for(conditions_container_t::iterator it = conditions_.begin(); it != conditions_.end(); ++it){
		if(*it > condition_id)
			(*it)--;
	}

	conditions_container_t::iterator it1 = std::find(conditions_.begin(),conditions_.end(),condition_id);
	if(it1 != conditions_.end())
		return false;

	conditions_.erase(it1);
	return true;
}

bool qdConditionGroup::load_script(const xml::tag* p)
{
#ifndef _QUEST_EDITOR
	conditions_.reserve(p -> data_size());
#endif

	if(const xml::tag* tp = p -> search_subtag(QDSCR_TYPE))
		conditions_mode_ = conditions_mode_t(xml::tag_buffer(*tp).get_int());

	xml::tag_buffer buf(*p);
	for(int i = 0; i < p -> data_size(); i++)
		conditions_.push_back(buf.get_int());

	return true;
}

bool qdConditionGroup::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++) fh < "\t";

	fh < "<condition_group";
	fh < " type=\"" <= (int)conditions_mode_ < "\"";
	fh < ">";

	fh <= conditions_.size();

	for(conditions_container_t::const_iterator it = conditions_.begin(); it != conditions_.end(); ++it)
		fh < " " <= *it;

	fh < "</condition_group>\r\n";

	return true;
}
