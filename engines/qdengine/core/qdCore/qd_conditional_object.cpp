
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_conditional_object.h"
#include "qd_game_scene.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#ifdef _QUEST_EDITOR
qdConditionalObject::ConditionsMode qdConditionalObject::backup_mode_ = qdConditionalObject::CONDITIONS_OR;
qdConditionalObject::condition_groups_container_t qdConditionalObject::groups_backup_;
qdConditionalObject::conditions_container_t qdConditionalObject::backup_;

std::list<qdConditionalObject*> qdConditionalObject::objects_list_;
bool qdConditionalObject::enable_objects_list_ = false;
#endif

qdConditionalObject::qdConditionalObject() : conditions_mode_(CONDITIONS_OR)
{
#ifdef _QUEST_EDITOR
	if(enable_objects_list_)
		objects_list_.push_back(this);
#endif
}

qdConditionalObject::qdConditionalObject(const qdConditionalObject& obj) : qdNamedObject(obj),
	conditions_mode_(obj.conditions_mode_),
	conditions_(obj.conditions_),
	condition_groups_(obj.condition_groups_)
{
#ifdef _QUEST_EDITOR
	if(enable_objects_list_)
		objects_list_.push_back(this);
#endif
}

qdConditionalObject::~qdConditionalObject()
{
#ifdef _QUEST_EDITOR
	if(enable_objects_list_){
		std::list<qdConditionalObject*>::iterator it = std::find(objects_list_.begin(),objects_list_.end(),this);
		if(it != objects_list_.end())
			objects_list_.erase(it);
	}
#endif
}

qdConditionalObject& qdConditionalObject::operator = (const qdConditionalObject& obj)
{
	if(this == &obj) return *this;

	*static_cast<qdNamedObject*>(this) = obj;

	conditions_mode_ = obj.conditions_mode_;
	conditions_ = obj.conditions_;
	condition_groups_ = obj.condition_groups_;

	return *this;
}

int qdConditionalObject::add_condition(const qdCondition* p)
{
	conditions_.push_back(*p);
	conditions_.back().set_owner(this);

	return conditions_.size() - 1;
}

bool qdConditionalObject::update_condition(int num,const qdCondition& p)
{
	assert(num >= 0 && num < conditions_.size());

	qdCondition& cond = conditions_[num];
	cond = p;
	cond.set_owner(this);

	return true;
}

bool qdConditionalObject::check_conditions()
{
#ifndef _QUEST_EDITOR
	qdCondition::clear_successful_clicks();

	if(!conditions_.empty()){
		switch(conditions_mode()){
		case CONDITIONS_AND:
			for(conditions_container_t::iterator it = conditions_.begin(); it != conditions_.end(); ++it){
				if(!it -> is_in_group()){
					if(!it -> check())
						return false;
				}
			}
			for(condition_groups_container_t::iterator it = condition_groups_.begin(); it != condition_groups_.end(); ++it){
				if(!check_group_conditions(*it))
					return false;
			}
			return true;
		case CONDITIONS_OR:
			for(conditions_container_t::iterator it = conditions_.begin(); it != conditions_.end(); ++it){
				if(!it -> is_in_group()){
					if(it -> check())
						return true;
				}
			}
			for(condition_groups_container_t::iterator it = condition_groups_.begin(); it != condition_groups_.end(); ++it){
				if(check_group_conditions(*it))
					return true;
			}
			return false;
		}
	}
#endif // _QUEST_EDITOR

	return true;
}

bool qdConditionalObject::remove_conditon(int idx)
{
	assert(idx >= 0 && idx < conditions_.size());

	conditions_.erase(conditions_.begin() + idx);

	for(condition_groups_container_t::iterator it = condition_groups_.begin(); it != condition_groups_.end(); ++it)
		it -> remove_condition(idx);

	return true;
}

#ifdef _QUEST_EDITOR
void qdConditionalObject::remove_all_conditions()
{
	conditions_.clear();
}
void qdConditionalObject::remove_all_groups()
{
	condition_groups_.clear();
}
#endif

bool qdConditionalObject::load_conditions_script(const xml::tag* p)
{
	int count = 0;
	int gr_count = 0;

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_CONDITION:
			count++;
			break;
		case QDSCR_CONDITION_GROUP:
			gr_count++;
			break;
		}
	}

	if(count) conditions_.resize(count);
	conditions_container_t::iterator ict = conditions_.begin();

	if(gr_count) condition_groups_.resize(gr_count);
	condition_groups_container_t::iterator igt = condition_groups_.begin();

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_CONDITION:
			if(const xml::tag* tp = it -> search_subtag(QDSCR_TYPE)){
				qdCondition* p = &*ict;
				p -> set_type(qdCondition::ConditionType(xml::tag_buffer(*tp).get_int()));
				p -> load_script(&*it);
				p -> set_owner(this);
			}
			++ict;
			break;
		case QDSCR_CONDITION_GROUP:
			if(const xml::tag* tp = it -> search_subtag(QDSCR_TYPE))
				igt -> set_conditions_mode(qdConditionGroup::conditions_mode_t(xml::tag_buffer(*tp).get_int()));
			igt -> load_script(&*it);
			++igt;
			break;
		case QDSCR_CONDITIONS_MODE:
			set_conditions_mode(ConditionsMode((xml::tag_buffer(*it).get_int())));
			break;
		}
	}

#ifndef _QUEST_EDITOR
	for(int i = 0; i < conditions_.size(); i++){
		if(is_condition_in_group(i))
			conditions_[i].add_group_reference();
	}
#endif

	return true;
}

bool qdConditionalObject::save_conditions_script(XStream& fh,int indent) const
{
	if(conditions_.size()){
		conditions_container_t::const_iterator it;
		FOR_EACH(conditions_,it)
			it -> save_script(fh,indent + 1);

		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<conditions_mode>" <= conditions_mode_ < "</conditions_mode>\r\n";
	}

	for(condition_groups_container_t::const_iterator it = condition_groups_.begin(); it != condition_groups_.end(); ++it)
		it -> save_script(fh,indent);

	return true;
}

void qdConditionalObject::conditions_quant(float dt)
{
	conditions_container_t::iterator it;
	FOR_EACH(conditions_,it)
		it -> quant(dt);
}

#ifdef _QUEST_EDITOR
bool qdConditionalObject::backup_object()
{
	backup_ = conditions_;
	groups_backup_ = condition_groups_;
	backup_mode_ = conditions_mode_;
	return true;
}

bool qdConditionalObject::restore_object()
{
	conditions_ = backup_;
	condition_groups_ = groups_backup_;
	conditions_mode_ = backup_mode_;
	return true;
}
#endif

bool qdConditionalObject::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdNamedObject::load_data(fh,save_version)) return false;

	for(conditions_container_t::iterator it = conditions_.begin(); it != conditions_.end(); ++it)
		it -> load_data(fh,save_version);

	return true;
}

bool qdConditionalObject::save_data(qdSaveStream& fh) const
{
	if(!qdNamedObject::save_data(fh)) return false;

	for(conditions_container_t::const_iterator it = conditions_.begin(); it != conditions_.end(); ++it)
		it -> save_data(fh);

	return true;
}

bool qdConditionalObject::check_group_conditions(const qdConditionGroup& gr)
{
	switch(gr.conditions_mode()){
	case qdConditionGroup::CONDITIONS_AND:
		for(qdConditionGroup::conditions_iterator_t it = gr.conditions_begin(); it != gr.conditions_end(); ++it){
			if(!conditions_[*it].check())
				return false;
		}
		return true;
	case qdConditionGroup::CONDITIONS_OR:
		for(qdConditionGroup::conditions_iterator_t it = gr.conditions_begin(); it != gr.conditions_end(); ++it){
			if(conditions_[*it].check())
				return true;
		}
		return false;
	}

	return true;
}

bool qdConditionalObject::is_condition_in_group(int condition_idx) const
{
	for(condition_groups_container_t::const_iterator it = condition_groups_.begin(); it != condition_groups_.end(); ++it){
		if(std::find(it -> conditions_begin(),it -> conditions_end(),condition_idx) != it -> conditions_end()) 
			return true;
	}

	return false;
}

int qdConditionalObject::add_condition_group(const qdConditionGroup* p)
{
	condition_groups_.push_back(*p);
	return condition_groups_.size() - 1;
}

bool qdConditionalObject::update_condition_group(int num,const qdConditionGroup& p)
{
	assert(num >= 0 && num < condition_groups_.size());

	qdConditionGroup& gr = condition_groups_[num];
	gr = p;

#ifndef _QUEST_EDITOR
	for(int i = 0; i < conditions_.size(); i++){
		if(is_condition_in_group(i))
			conditions_[i].add_group_reference();
		else
			conditions_[i].remove_group_reference();
	}
#endif

	return true;
}

bool qdConditionalObject::remove_conditon_group(int idx)
{
	assert(idx >= 0 && idx < condition_groups_.size());

	condition_groups_.erase(condition_groups_.begin() + idx);

#ifndef _QUEST_EDITOR
	for(int i = 0; i < conditions_.size(); i++){
		if(is_condition_in_group(i))
			conditions_[i].add_group_reference();
		else
			conditions_[i].remove_group_reference();
	}
#endif

	return true;
}

#ifdef _QUEST_EDITOR
bool qdConditionalObject::init_objects()
{
	bool result = true;

	for(int i = 0; i < conditions_.size(); i++){
		if(!conditions_[i].init_objects())
			result = false;
	}

	return result;
}

void qdConditionalObject::global_init()
{
	for(std::list<qdConditionalObject*>::const_iterator it = objects_list_.begin(); it != objects_list_.end(); ++it)
		(*it) -> init_objects();
}
#endif /* _QUEST_EDITOR */

bool qdConditionalObject::init()
{
	bool result = true;

	for(int i = 0; i < conditions_.size(); i++){
		if(!conditions_[i].init())
			result = false;
	}

	return result;
}

bool qdConditionalObject::trigger_can_start() const
{
	if(const qdGameScene* p = static_cast<const qdGameScene*>(owner(QD_NAMED_OBJECT_SCENE)))
		return p -> is_active();

	return true;
}
