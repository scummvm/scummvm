/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_trigger_element.h"
#include "qd_trigger_chain.h"
#include "qd_video.h"
#include "qd_minigame.h"
#include "qd_music_track.h"
#include "qd_game_scene.h"
#include "qd_grid_zone.h"
#include "qd_game_object_state.h"
#include "qd_game_object_animated.h"
#include "qd_named_object_indexer.h"

#include "qd_game_dispatcher.h"

#ifdef _QUEST_EDITOR
#include <additional/qls.hpp>
#endif //#ifdef _QUEST_EDITOR

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

#ifdef __QD_DEBUG_ENABLE__
static appLog& operator << (appLog& log,const qdTriggerLink* link);
appLog& operator << (appLog& log,const qdNamedObjectReference& obj);
#endif

/* --------------------------- DEFINITION SECTION --------------------------- */

qdTriggerLink::qdTriggerLink(qdTriggerElementPtr p,int tp) 
	: element_(p),
	element_ID_(qdTriggerElement::INVALID_ID),
	type_(tp),
	auto_restart_(false)
{
	if(element_)
		element_ID_ = element_ -> ID();

	set_status(LINK_INACTIVE); 
#ifdef _QUEST_EDITOR
	m_owner_offset.cx = m_owner_offset.cy = 0;
	m_child_offset.cx = m_child_offset.cy = 0;
#endif // _QUEST_EDITOR
}

qdTriggerLink::qdTriggerLink() : element_(NULL),
	element_ID_(qdTriggerElement::INVALID_ID),
	type_(0),
	auto_restart_(false)
{
	set_status(LINK_INACTIVE); 
#ifdef _QUEST_EDITOR
	m_owner_offset.cx = m_owner_offset.cy = 0;
	m_child_offset.cx = m_child_offset.cy = 0;
#endif // _QUEST_EDITOR
}

bool qdTriggerLink::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);
		switch(it -> ID()){
		case QDSCR_ID:
			set_element_ID(buf.get_int());
			break;
		case QDSCR_TYPE:
			type_ = buf.get_int();
			break;
		case QDSCR_TRIGGER_ELEMENT_LINK_AUTO_RESTART:
			if(buf.get_int())
				auto_restart_ = true;
			else
				auto_restart_ = false;
			break;
#ifdef _QUEST_EDITOR
		case QDSCR_TRIGGER_ELEMENT_LINK_OFFSETS:
			m_owner_offset.cx = buf.get_int();
			m_owner_offset.cy = buf.get_int();
			m_child_offset.cx = buf.get_int();
			m_child_offset.cy = buf.get_int();
			break;
#endif // _QUEST_EDITOR
		}
	}
	return true;
}

bool qdTriggerLink::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i++)  fh < "\t";
	fh < "<link";

	if(element_) fh < " ID=\"" <= element_ -> ID() < "\"";

	if(type_) fh < " type=\"" <= type_ <"\"";

	if(auto_restart_)
		fh < " auto_restart=\"1\"";

#ifdef _QUEST_EDITOR
	if(m_owner_offset.cx || m_owner_offset.cy || m_child_offset.cx || m_child_offset.cy){
		fh < " offsets=\"" <= m_owner_offset.cx < " " <= m_owner_offset.cy < " ";
		fh <= m_child_offset.cx < " " <= m_child_offset.cy < "\"";
	}
#endif // _QUEST_EDITOR

	fh < "/>\r\n";

	return true;
}

qdTriggerElement::qdTriggerElement() : object_(NULL),
	ID_(0),
	is_active_(false),
#ifdef __QD_TRIGGER_PROFILER__
	owner_(NULL),
#endif
	status_(TRIGGER_EL_INACTIVE)
{
#ifdef _QUEST_EDITOR
	memset(&m_rcBound, 0, sizeof(RECT));
	m_bSelected = false;
	m_vCellNumber.x = m_vCellNumber.y = -1;
#endif
}

qdTriggerElement::qdTriggerElement(qdNamedObject* p) : object_(p),
	ID_(0),
	is_active_(false),
#ifdef _QUEST_EDITOR
	object_reference_(p),
#endif
#ifdef __QD_TRIGGER_PROFILER__
	owner_(NULL),
#endif
	status_(TRIGGER_EL_INACTIVE)
{
#ifdef _QUEST_EDITOR
	memset(&m_rcBound, 0, sizeof(RECT));
	m_bSelected = false;
	m_vCellNumber.x = m_vCellNumber.y = -1;
	update_title();
#else
	p -> add_trigger_reference();
#endif
}

#ifdef _QUEST_EDITOR
void qdTriggerElement::update_title()
{
	if (object_reference_.is_empty()) return;
	std::string res;
	int nLevels = object_reference_.num_levels()-1;
	int i = 0;
//	if (object_->ref_owner()!=object_->owner()) {
//		i = 1;
//	}
	for(; i < nLevels; ++i)
	{
		res += object_reference_.object_name(i);
		res += "::";
	}
	res += object_reference_.object_name(nLevels);

	m_strTitle = res;	
}

bool qdTriggerElement::update_object_reference()
{
	if (object_) 
	{
		object_reference_.init(object_);
		update_title();
		return true;
	}
	else
		object_reference_.clear();

	return true;
}

#endif // _QUEST_EDITOR

qdTriggerElement::~qdTriggerElement()
{
#ifndef _QUEST_EDITOR
	if(object_) object_ -> remove_trigger_reference();
#endif // _QUEST_EDITOR

	parents_.clear();
	children_.clear();
}

bool qdTriggerElement::check_external_conditions(int link_type)
{
	if(parents_.empty()) return true;

	qdTriggerLinkList::iterator it;
	FOR_EACH(parents_,it){
		if(it -> type() == link_type && it -> element() -> status() != TRIGGER_EL_DONE)
			return false;
	}

	return true;
}

bool qdTriggerElement::check_internal_conditions()
{
	if(!object_) return true;

	if(qdConditionalObject* p = dynamic_cast<qdConditionalObject*>(object_)){
		if(p -> trigger_can_start())
			return p -> check_conditions();
		else
			return false;
	}

	return true;
}

qdTriggerLink* qdTriggerElement::find_child_link(qdTriggerElementConstPtr ptrChild)
{
#ifdef _QUEST_EDITOR
	qdTriggerLinkList::iterator itr = qls::find(children_.begin(),children_.end(),ptrChild);
#else
	qdTriggerLinkList::iterator itr = std::find(children_.begin(),children_.end(),ptrChild);
#endif

	if(itr != children_.end())
		return &*itr;
	return NULL;
}

qdTriggerLink* qdTriggerElement::find_child_link(int child_id)
{
	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		if(it -> element() -> ID() == child_id)
			return &*it;
	}

	return NULL;
}

qdTriggerLink* qdTriggerElement::find_parent_link(qdTriggerElementConstPtr ptrParent)
{
#ifdef _QUEST_EDITOR
	qdTriggerLinkList::iterator itr = qls::find(parents_.begin(),parents_.end(),ptrParent);
#else
	qdTriggerLinkList::iterator itr = std::find(parents_.begin(),parents_.end(),ptrParent);
#endif

	if(itr != parents_.end())
		return &*itr;
	return NULL;
}

qdTriggerLink* qdTriggerElement::find_parent_link(int parent_id)
{
	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		if(it -> element() -> ID() == parent_id)
			return &*it;
	}

	return NULL;
}

#ifdef _QUEST_EDITOR
bool qdTriggerElement::set_parent_link_owner_offset(qdTriggerElementConstPtr el,int x,int y)
{
	qdTriggerLink* lp = find_parent_link(el);
	if(lp){
		lp -> set_owner_offset(x,y);
		return true;
	}
	return false;
}

bool qdTriggerElement::set_parent_link_child_offset(qdTriggerElementConstPtr el,int x,int y)
{
	qdTriggerLink* lp = find_parent_link(el);
	if(lp){
		lp -> set_child_offset(x,y);
		return true;
	}
	return false;
}

bool qdTriggerElement::set_child_link_owner_offset(qdTriggerElementConstPtr el,int x,int y)
{
	qdTriggerLink* lp = find_child_link(el);
	if(lp){
		lp -> set_owner_offset(x,y);
		return true;
	}
	return false;
}

bool qdTriggerElement::set_child_link_child_offset(qdTriggerElementConstPtr el,int x,int y)
{
	qdTriggerLink* lp = find_child_link(el);
	if(lp){
		lp -> set_child_offset(x,y);
		return true;
	}
	return false;
}

void qdTriggerElement::clear_parents()
{
	parents_.clear();
}
void qdTriggerElement::clear_children()
{
	children_.clear();
}

#endif//#ifdef _QUEST_EDITOR

bool qdTriggerElement::is_parent(qdTriggerElementConstPtr p)
{
#ifdef _QUEST_EDITOR
	return (find_parent_link(p) != NULL);
#else
	qdTriggerLinkList::iterator it;
	FOR_EACH(parents_,it)
		if(it -> element() == p) return true;

	return false;
#endif
}

bool qdTriggerElement::is_child(qdTriggerElementConstPtr p)
{
#ifdef _QUEST_EDITOR
	return (find_child_link(p) != NULL);
#else
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it)
		if(it -> element() == p) return true;

	return false;
#endif
}

bool qdTriggerElement::add_parent(qdTriggerElementPtr p,int link_type)
{
	if(p == this || is_parent(p)) return false;
	parents_.push_back(qdTriggerLink(p,link_type));
	return true;
}

bool qdTriggerElement::add_child(qdTriggerElementPtr p,int link_type,bool auto_restart)
{
	if(p == this || is_child(p)) return false;
	children_.push_back(qdTriggerLink(p,link_type));
	if(auto_restart)
		children_.back().toggle_auto_restart(true);
	
	return true;
}

bool qdTriggerElement::remove_parent(qdTriggerElementPtr p)
{
#ifdef _QUEST_EDITOR
	qdTriggerLinkList::iterator res = qls::mutable_find(parents_.begin(), 
		parents_.end(), p);
	if (res != parents_.end()) {
		parents_.erase(res);
		return true;
	}
#else
	qdTriggerLinkList::iterator it;
	FOR_EACH(parents_,it){
		if(it -> element() == p){ 
			parents_.erase(it);
			return true;
		}
	}
#endif
	return false;
}

bool qdTriggerElement::remove_child(qdTriggerElementPtr p)
{
#ifdef _QUEST_EDITOR
	qdTriggerLinkList::iterator res = qls::mutable_find(
		children_.begin(), children_.end(), p);
	if (res != children_.end()) {
		children_.erase(res);
		return true;
	}
#else
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it){
		if(it -> element() == p){
			children_.erase(it);
			return true;
		}
	}
#endif//#ifdef _QUEST_EDITOR
	return false;
}

bool qdTriggerElement::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_ID:
				set_id(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_NAMED_OBJECT: {
					qdNamedObjectReference ref;
					ref.load_script(&*it);
					retrieve_object(ref);
#ifdef _QUEST_EDITOR	
					object_reference_ = ref;
#endif
				}
				break;
			case QDSCR_TRIGGER_START_ELEMENT:
				if(xml::tag_buffer(*it).get_int())
					is_active_ = true;
				else
					is_active_ = false;
				break;
			case QDSCR_TRIGGER_ELEMENT_PARENT_LINKS:
				load_links_script(&*it,true);
				break;
			case QDSCR_TRIGGER_ELEMENT_CHILD_LINKS:
				load_links_script(&*it,false);
				break;
#ifdef _QUEST_EDITOR
			case QDSCR_TRIGGER_BOUND: {
				xml::tag_buffer buf(*it);
				m_rcBound.left = buf.get_int();
				m_rcBound.top = buf.get_int();
				m_rcBound.right = buf.get_int();
				m_rcBound.bottom = buf.get_int();
				}
				break;
			case QDSCR_TRIGGER_ELEMENT_TITLE:
				m_strTitle = it -> data();
				break;
			case QDSCR_TRIGGER_ELEMENT_CELL_NUMBER: {
				xml::tag_buffer buf(*it);
				m_vCellNumber.x = buf.get_int();
				m_vCellNumber.y = buf.get_int();
				}
				break;
#endif
		}
	}

	set_status(TRIGGER_EL_INACTIVE);
	return true;
}

bool qdTriggerElement::load_links_script(const xml::tag* p,bool load_parents)
{
	qdTriggerLinkList& lst = (load_parents) ? parents_ : children_;

	int size = 0;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		if(it -> ID() == QDSCR_TRIGGER_ELEMENT_LINK)
			size++;
	}

	if(size)
		lst.resize(size);

	int link_idx = 0;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		if(it -> ID() == QDSCR_TRIGGER_ELEMENT_LINK){
			if(link_idx < size)
				lst[link_idx++].load_script(&*it);
		}
	}

	return true;
}

bool qdTriggerElement::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	if(ID_ == ROOT_ID)
		fh < "<trigger_chain_root";
	else
		fh < "<trigger_element ID=\"" <= ID_ < "\"";

	if(is_active_) fh < " start_element=\"1\"";

#ifdef _QUEST_EDITOR
	fh < " trigger_bound=\"" <= m_rcBound.left < " " <= m_rcBound.top < " " <= m_rcBound.right < " " <= m_rcBound.bottom < "\"";

	fh < " trigger_element_title=\"" < qdscr_XML_string(m_strTitle.c_str()) < "\"";

	fh < " trigger_element_cell_number=\"" <= m_vCellNumber.x < " " <= m_vCellNumber.y < "\"";
#endif

	fh < ">\r\n";

	if(object_){
		qdNamedObjectReference ref(object_);
		ref.save_script(fh,indent + 1);
	}
	else {
#ifdef _QUEST_EDITOR
		if(!object_reference_.is_empty())
			object_reference_.save_script(fh,indent + 1);
#endif
	}

	if(parents_.size()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<parent_links>\r\n";

		for(qdTriggerLinkList::const_iterator it = parents_.begin(); it != parents_.end(); ++it)
			it -> save_script(fh,indent + 2);

		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "</parent_links>\r\n";
	}

	if(children_.size()){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<child_links>\r\n";

		for(qdTriggerLinkList::const_iterator it = children_.begin(); it != children_.end(); ++it)
			it -> save_script(fh,indent + 2);

		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "</child_links>\r\n";
	}

	for(int i = 0; i < indent; i ++) fh < "\t";
	if(ID_ == ROOT_ID)
		fh < "</trigger_chain_root>\r\n";
	else
		fh < "</trigger_element>\r\n";

	return true;
}

bool qdTriggerElement::retrieve_object(const qdNamedObjectReference& ref)
{
	if(qdGameDispatcher* dsp = qdGameDispatcher::get_dispatcher()){
		if(object_) object_ -> remove_trigger_reference();
		object_ = dsp -> get_named_object(&ref);

		if(!object_){
#ifndef _QUEST_EDITOR
			qdNamedObjectReference& r = qdNamedObjectIndexer::instance().add_reference((qdNamedObject*&)object_);
			r = ref;
#endif
#ifdef __QD_DEBUG_ENABLE__
			appLog::default_log() << "qdTriggerElement::retrieve_object() failed:\r\n";
			appLog::default_log() << ref << "\r\n";
#endif
			return false;
		}

		object_ -> add_trigger_reference();
	}

	return false;
}

bool qdTriggerElement::retrieve_link_elements(qdTriggerChain* p)
{
	bool result = true;

	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		if(!it -> element()){
			if(qdTriggerElementPtr el = p -> search_element(it -> element_ID())){
				it -> set_element(el);
			}
			else {
				appLog::default_log() << "parent link element not found: " << ID() << " -> " << it -> element_ID() << "\r\n";
				result = false;
			}
		}
	}

	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		if(!it -> element()){
			if(qdTriggerElementPtr el = p -> search_element(it -> element_ID())){
				it -> set_element(el);
			}
			else {
				appLog::default_log() << "child link element not found: " << ID() << " -> " << it -> element_ID() << "\r\n";
				result = false;
			}
		}
	}

	return result;
}

bool qdTriggerElement::quant(float dt)
{
	bool ret = false;
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it){
		if(it -> status() == qdTriggerLink::LINK_ACTIVE){
			if(it -> element() -> conditions_quant(it -> type())){
				if(!it -> auto_restart())
					it -> set_status(qdTriggerLink::LINK_DONE);
#ifdef __QD_TRIGGER_PROFILER__
				if(!qdTriggerProfiler::instance().is_read_only()){
					qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,it -> element() -> ID(),it -> status());
					qdTriggerProfiler::instance().add_record(rec);
				}
#endif
				ret = true;
			}
		}
	}

	return ret;
}

void qdTriggerElement::start()
{
	if(qdConditionalObject* p = dynamic_cast<qdConditionalObject*>(object_)){
		switch(p -> trigger_start()){
		case qdConditionalObject::TRIGGER_START_ACTIVATE:
			set_status(TRIGGER_EL_WORKING);

			for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
				if(qdTriggerLink* lp = it -> element() -> find_child_link(this)){
					if(lp -> status() == qdTriggerLink::LINK_ACTIVE){
						it -> element() -> activate_links(this);
						it -> element() -> deactivate_links(this);
					}
				}
			}
			break;
		case qdConditionalObject::TRIGGER_START_WAIT:
			set_status(TRIGGER_EL_WAITING);
		}
	}
	else
		set_status(TRIGGER_EL_WAITING);
}

bool qdTriggerElement::conditions_quant(int link_type)
{
	switch(status_){
		case TRIGGER_EL_DONE:
		case TRIGGER_EL_INACTIVE:
			if(check_external_conditions(link_type) && check_internal_conditions() && object_){
#ifndef _QUEST_EDITOR
				if(qdCondition::successful_click())
					qdGameDispatcher::get_dispatcher() -> drop_flag(qdGameDispatcher::CLICK_FAILED_FLAG);
				if(qdCondition::successful_object_click())
					qdGameDispatcher::get_dispatcher() -> drop_flag(qdGameDispatcher::OBJECT_CLICK_FAILED_FLAG);
#endif
				start();
			}
			else {
				if(object_ && object_ -> named_object_type() == QD_NAMED_OBJECT_OBJ_STATE){
					qdGameObjectState* p = static_cast<qdGameObjectState*>(object_);
					if(p -> has_text() && p -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DIALOG_PHRASE)){
						if(qdGameDispatcher* gp = qd_get_game_dispatcher())
							gp -> add_dialog_state(p);
					}
				}
			}
			break;
		case TRIGGER_EL_WAITING:
			if(object_ && object_ -> named_object_type() == QD_NAMED_OBJECT_OBJ_STATE){
				qdGameObjectState* p = static_cast<qdGameObjectState*>(object_);
				qdGameObjectAnimated* obj = static_cast<qdGameObjectAnimated*>(object_ -> owner());

				switch(obj -> state_status(p)){
				case qdGameObjectAnimated::STATE_INACTIVE:
					set_status(TRIGGER_EL_INACTIVE);
					break;
				case qdGameObjectAnimated::STATE_ACTIVE:
				case qdGameObjectAnimated::STATE_DONE:
					set_status(TRIGGER_EL_WORKING);
					break;
				}
			}
			else
				set_status(TRIGGER_EL_WORKING);

			if(status() == TRIGGER_EL_WORKING){
				for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
					if(qdTriggerLink* lp = it -> element() -> find_child_link(this)){
						if(lp -> status() != qdTriggerLink::LINK_INACTIVE){
							it -> element() -> activate_links(this);
							it -> element() -> deactivate_links(this);
						}
					}
				}
			}
			break;
		case TRIGGER_EL_WORKING:
			if(object_ && object_ -> named_object_type() == QD_NAMED_OBJECT_OBJ_STATE){
				qdGameObjectState* p = static_cast<qdGameObjectState*>(object_);
				qdGameObjectAnimated* obj = static_cast<qdGameObjectAnimated*>(object_ -> owner());

				switch(obj -> state_status(p)){
				case qdGameObjectAnimated::STATE_INACTIVE:
					set_status(TRIGGER_EL_DONE);
					break;
				case qdGameObjectAnimated::STATE_ACTIVE:
					break;
				case qdGameObjectAnimated::STATE_DONE:
					obj -> handle_state_end();
					set_status(TRIGGER_EL_DONE);
					break;
				}
			}
			else 
				set_status(TRIGGER_EL_DONE);

			if(status_ == TRIGGER_EL_DONE){
				qdTriggerLinkList::iterator it;
				FOR_EACH(parents_,it)
					it -> element() -> deactivate_link(this);
				FOR_EACH(children_,it){
					it -> activate();
#ifdef __QD_TRIGGER_PROFILER__
					if(!qdTriggerProfiler::instance().is_read_only()){
						qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,it -> element() -> ID(),it -> status());
						qdTriggerProfiler::instance().add_record(rec);
					}
#endif
				}

				return true;
			}
			break;
//		case TRIGGER_EL_DONE:
//			break;
	}

	return false;
}

bool qdTriggerElement::activate_links(qdTriggerElementPtr child)
{
	int link_type = -1;
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it){
		if(it -> element() == child){
			link_type = it -> type();
			break;
		}
	}

	if(link_type == -1) return false;

	FOR_EACH(children_,it){
		if(it -> type() == link_type){
			if(it -> element() != child && it -> status() == qdTriggerLink::LINK_INACTIVE){
				it -> activate();
#ifdef __QD_TRIGGER_PROFILER__
				if(!qdTriggerProfiler::instance().is_read_only()){
					qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,it -> element() -> ID(),it -> status());
					qdTriggerProfiler::instance().add_record(rec);
				}
#endif
			}
		}
	}

	return true;
}

bool qdTriggerElement::deactivate_links(qdTriggerElementPtr child)
{
	int link_type = -1;
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it){
		if(it -> element() == child){
			link_type = it -> type();
			break;
		}
	}
	
	if(link_type == -1) return false;
	
	FOR_EACH(children_,it){
		if(it -> type() != link_type){
			it -> deactivate();
//			it -> element() -> set_status(qdTriggerElement::TRIGGER_EL_INACTIVE);
#ifdef __QD_TRIGGER_PROFILER__
			if(!qdTriggerProfiler::instance().is_read_only()){
				qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,it -> element() -> ID(),it -> status());
				qdTriggerProfiler::instance().add_record(rec);
			}
#endif
		}
	}
	
	return true;
}

bool qdTriggerElement::deactivate_link(qdTriggerElementPtr child)
{
	qdTriggerLinkList::iterator it;
	FOR_EACH(children_,it){
		if(it -> element() == child){
			it -> deactivate();
#ifdef __QD_TRIGGER_PROFILER__
			if(!qdTriggerProfiler::instance().is_read_only()){
				qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,it -> element() -> ID(),it -> status());
				qdTriggerProfiler::instance().add_record(rec);
			}
#endif
			return true;
		}
	}

	return false;
}

bool qdTriggerElement::debug_set_active()
{
	set_status(TRIGGER_EL_INACTIVE);

	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		if(it -> element() -> status() == TRIGGER_EL_DONE && !it -> element() -> is_active())
			it -> element() -> debug_set_inactive();
	}

#ifdef __QD_TRIGGER_PROFILER__
	if(qdTriggerProfiler::instance().is_logging_enabled())
		qdTriggerProfiler::instance().set_read_only(false);
#endif

	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		it -> element() -> set_child_link_status(this,qdTriggerLink::LINK_ACTIVE);
		it -> element() -> set_status(TRIGGER_EL_DONE);
	}

#ifdef __QD_TRIGGER_PROFILER__
	if(qdTriggerProfiler::instance().is_logging_enabled())
		qdTriggerProfiler::instance().set_read_only(true);
#endif

	return true;
}

bool qdTriggerElement::debug_set_inactive()
{
	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it)
		it -> element() -> set_child_link_status(this,qdTriggerLink::LINK_INACTIVE);

	set_status(TRIGGER_EL_INACTIVE);

	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		if(it -> element() -> status() == TRIGGER_EL_DONE && !it -> element() -> is_active())
			it -> element() -> debug_set_inactive();
	}

	return true;
}

bool qdTriggerElement::debug_set_done()
{
	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it)
		it -> element() -> set_child_link_status(this,qdTriggerLink::LINK_DONE);

	set_status(TRIGGER_EL_DONE);

	return true;
}

bool qdTriggerElement::set_child_link_status(qdTriggerElementConstPtr child,qdTriggerLink::LinkStatus st)
{
	if(qdTriggerLink* p = find_child_link(child)){
		if(!p -> auto_restart() || st == qdTriggerLink::LINK_ACTIVE){
			p -> set_status(st);
#ifdef __QD_TRIGGER_PROFILER__
			if(!qdTriggerProfiler::instance().is_read_only()){
				qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,owner_,ID_,p -> element() -> ID(),p -> status());
				qdTriggerProfiler::instance().add_record(rec);
			}
#endif
		}
		return true;
	}

	return false;
}

bool qdTriggerElement::set_parent_link_status(qdTriggerElementConstPtr parent,qdTriggerLink::LinkStatus st)
{
	if(qdTriggerLink* p = find_parent_link(parent)){
		p -> set_status(st);
		return true;
	}

	return false;
}

bool qdTriggerElement::load_data(qdSaveStream& fh,int save_version)
{
	char st;
	fh > st;

/*	switch(st){
	case TRIGGER_EL_INACTIVE_ALL:
		set_status(TRIGGER_EL_INACTIVE);
		for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it)
			it -> set_status(qdTriggerLink::LINK_INACTIVE);
		for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it)
			it -> set_status(qdTriggerLink::LINK_INACTIVE);
		return true;
	case TRIGGER_EL_DONE_ALL:
		set_status(TRIGGER_EL_DONE);
		for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it)
			it -> set_status(qdTriggerLink::LINK_DONE);
		for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it)
			it -> set_status(qdTriggerLink::LINK_DONE);
		return true;
	}*/

	set_status(ElementStatus(st));

	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		char st;
		fh > st;

		it -> set_status(qdTriggerLink::LinkStatus(st));
	}

	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		char st;
		fh > st;

		it -> set_status(qdTriggerLink::LinkStatus(st));
	}

	return true;
}

bool qdTriggerElement::save_data(qdSaveStream& fh) const
{
/*	switch(status_){
	case TRIGGER_EL_INACTIVE:
		for(qdTriggerLinkList::const_iterator it = parents_.begin(); it != parents_.end(); ++it){
			if(it -> status() != qdTriggerLink::LINK_INACTIVE) 
				break;
		}
		for(qdTriggerLinkList::const_iterator it = children_.begin(); it != children_.end(); ++it){
			if(it -> status() != qdTriggerLink::LINK_INACTIVE) 
				break;
		}
		fh < char(TRIGGER_EL_INACTIVE_ALL);
		return true;
	case TRIGGER_EL_DONE:
		for(qdTriggerLinkList::const_iterator it = parents_.begin(); it != parents_.end(); ++it){
			if(it -> status() != qdTriggerLink::LINK_DONE) 
				break;
		}
		for(qdTriggerLinkList::const_iterator it = children_.begin(); it != children_.end(); ++it){
			if(it -> status() != qdTriggerLink::LINK_DONE) 
				break;
		}
		fh < char(TRIGGER_EL_DONE_ALL);
		return true;
	}*/

	fh < char(status_);

	for(qdTriggerLinkList::const_iterator it = parents_.begin(); it != parents_.end(); ++it)
		fh < char(it -> status());

	for(qdTriggerLinkList::const_iterator it = children_.begin(); it != children_.end(); ++it)
		fh < char(it -> status());

	return true;
}

void qdTriggerLink::activate()
{
	set_status(LINK_ACTIVE);

#ifndef _QUEST_EDITOR
//	if(element_ && element_ -> status() != qdTriggerElement::TRIGGER_EL_INACTIVE)
//		element_ -> set_status(qdTriggerElement::TRIGGER_EL_INACTIVE);
#endif

//	appLog::default_log() << this;
}

void qdTriggerLink::deactivate()
{
	if(status_ == LINK_ACTIVE){
		if(!auto_restart())
			set_status(LINK_INACTIVE); 
#ifdef __QD_DEBUG_ENABLE__
		else
			appLog::default_log() << this;
#endif
	}
}

#ifdef __QD_DEBUG_ENABLE__
static appLog& operator << (appLog& log,const qdTriggerLink* link)
{
	if(link -> element()){
		if(qdNamedObject* p = link -> element() -> object()){
			log << " -> тип " << link -> type();

			switch(link -> status()){
			case qdTriggerLink::LINK_ACTIVE:
				log << " вкл. ";
				break;
			case qdTriggerLink::LINK_INACTIVE:
				log << " выкл. ";
				break;
			case qdTriggerLink::LINK_DONE:
				log << " отр. ";
				break;
			}

			if(p -> named_object_type() == QD_NAMED_OBJECT_OBJ_STATE){
				if(p -> owner() && p -> owner() -> name())
					log << p -> owner() -> name() << "/";
			}
			log << p -> name() << "\r\n";
		}
	}

	return log;
}

appLog& operator << (appLog& log,const qdNamedObjectReference& obj)
{
	for(int i = 0; i < obj.num_levels(); i++){
		if(i) log << "::";
		log << obj.object_name(obj.num_levels() - i - 1);
	}

	return log;
}
#endif

void qdTriggerElement::set_status(ElementStatus st)
{ 
	status_ = st;

#ifdef __QD_TRIGGER_PROFILER__
	if(!qdTriggerProfiler::instance().is_read_only()){
		qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::ELEMENT_STATUS_UPDATE,owner_,ID_,0,st);
		qdTriggerProfiler::instance().add_record(rec);
	}
#endif
}

void qdTriggerElement::reset()
{
	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it)
		it -> set_status(qdTriggerLink::LINK_INACTIVE);
	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it)
		it -> set_status(qdTriggerLink::LINK_INACTIVE);

	set_status(TRIGGER_EL_INACTIVE);
}

void qdTriggerElement::deactivate(const qdNamedObject* ignore_object)
{
	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		if(!ignore_object || it -> element() -> object() != ignore_object){
			if(!it -> auto_restart())
				it -> set_status(qdTriggerLink::LINK_INACTIVE);
		}
	}
	for(qdTriggerLinkList::iterator it = children_.begin(); it != children_.end(); ++it){
		if(!ignore_object || it -> element() -> object() != ignore_object){
			if(!it -> auto_restart())
				it -> set_status(qdTriggerLink::LINK_INACTIVE);
		}
	}

	set_status(TRIGGER_EL_INACTIVE);

	for(qdTriggerLinkList::iterator it = parents_.begin(); it != parents_.end(); ++it){
		if(!ignore_object || it -> element() -> object() != ignore_object)
			it -> element() -> set_child_link_status(this,qdTriggerLink::LINK_INACTIVE);
	}
}

bool qdTriggerElement::add_object_trigger_reference()
{
	if(object_){
		object_ -> add_trigger_reference();
		return true;
	}

	return false;
}

bool qdTriggerElement::clear_object_trigger_references()
{
	if(object_){
		object_ -> clear_trigger_references();
		return true;
	}

	return false;
}
