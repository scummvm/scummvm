
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"

#include "qdscr_parser.h"
#include "qd_trigger_chain.h"
#include "qd_trigger_profiler.h"

#ifdef __QD_TRIGGER_PROFILER__
#include "qd_game_dispatcher.h"
#endif

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

namespace{
	class id_equality
	{
		int id_;
	public:
		id_equality(int id):id_(id){}
		bool operator()(qdTriggerElementPtr const& element) const{
			return (element->ID() == id_);
		}
	};
	struct id_compare{
		bool operator()(qdTriggerElementPtr const& lhs, 
						int rhs) const{
			return (lhs->ID() < rhs);
		}
	};
}

qdTriggerChain::qdTriggerChain()
#ifdef _QUEST_EDITOR
:
  root_(new qdTriggerElement)
#endif // _QUEST_EDITOR
{
	root_element()->set_id(qdTriggerElement::ROOT_ID);
	root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);

#ifdef __QD_TRIGGER_PROFILER__
	root_element()->set_owner(this);
#endif

#ifdef _QUEST_EDITOR
	memset(&m_rcBound, 0, sizeof(m_rcBound));
	memset(&m_rcWorkArea, 0, sizeof(m_rcWorkArea));
	root_element()->set_title("Старт");
	m_szGenLayout.cx = m_szGenLayout.cy = -1;
#endif
}

qdTriggerChain::~qdTriggerChain()
{
#ifndef _QUEST_EDITOR
	qdTriggerElementList::iterator it;
	FOR_EACH(elements_,it)
		delete *it;
#else
	qdTriggerElementList::iterator it;
	FOR_EACH(elements_,it)
	{
		(*it)->clear_children();
		(*it)->clear_parents();

	}
#endif // _QUEST_EDITOR

	elements_.clear();
}

bool qdTriggerChain::reindex_elements()
{
	int id = 0;
	qdTriggerElementList::iterator it;
	FOR_EACH(elements_,it)
		(*it) -> set_id(id++);

	return true;
}

#ifdef _QUEST_EDITOR
qdTriggerElementPtr qdTriggerChain::search_element(const qdNamedObject* pobj) const
{
	if (!pobj)
		return NULL;
	qdTriggerElementList::const_iterator i = elements_.begin(), e = elements_.end();
	for(;i != e; ++i)
	{
		if ((*i)->object() == pobj)
			return *i;
	}
	return qdTriggerElementPtr(NULL);
}
#endif // _QUEST_EDITOR

qdTriggerElementPtr qdTriggerChain::search_element(int id)
{
	if(id == qdTriggerElement::ROOT_ID) return root_element();

	qdTriggerElementList::iterator res = 
		std::lower_bound(elements_.begin(), elements_.end(), id, id_compare());
	//		std::find_if(elements_.begin(), elements_.end(), id_equality(id));

	if (res == elements_.end()||(*res)->ID() != id)
		return 0;
	return *res;
}

#ifdef _QUEST_EDITOR
//! используется для undo/redo
bool qdTriggerChain::add_element(qdTriggerElementPtr p)
{
	assert(p);
	if(!can_add_element(p->object())) {
		assert(0);
		return 0;
	}

	p -> object() -> add_trigger_reference();
	elements_.push_back(p);

#ifdef __QD_TRIGGER_PROFILER__
	p -> set_owner(this);
#endif

	reindex_elements();

	return true;
}
#endif // _QUEST_EDITOR

qdTriggerElementPtr qdTriggerChain::add_element(qdNamedObject* p)
{
	if(!can_add_element(p)) return 0;

	qdTriggerElementPtr el = new qdTriggerElement(p);
	elements_.push_back(el);
#ifdef _QUEST_EDITOR
	p -> add_trigger_reference();
#endif // _QUEST_EDITOR
#ifdef __QD_TRIGGER_PROFILER__
	el -> set_owner(this);
#endif

	reindex_elements();

	return el;
}

bool qdTriggerChain::remove_element(qdTriggerElementPtr p,bool free_mem,bool relink_elements)
{
	qdTriggerElementList::iterator it;
	FOR_EACH(elements_,it){
		if(*it == p){
			if(relink_elements){
				qdTriggerLinkList::iterator it_c;
				FOR_EACH((*it) -> children(),it_c)
					it_c -> element() -> remove_parent(*it);

				qdTriggerLinkList::iterator it_p;
				FOR_EACH((*it) -> parents(),it_p)
					it_p -> element() -> remove_child(*it);

				FOR_EACH((*it) -> children(),it_c){
					FOR_EACH((*it) -> parents(),it_p){
						it_c -> element() -> add_parent(it_p -> element());
						it_p -> element() -> add_child(it_c -> element());
					}
				}
			}

#ifndef _QUEST_EDITOR
			if(free_mem)
				delete *it;
#else
			(*it)->object()->remove_trigger_reference();
#endif // _QUEST_EDITOR

			elements_.erase(it);
			reindex_elements();

			return true;
		}
	}

	return false;
}

bool qdTriggerChain::can_add_element(const qdNamedObject* p) const
{
	if(is_element_in_list(p) && p -> named_object_type() != QD_NAMED_OBJECT_SCENE) return false;

	return true;
}

bool qdTriggerChain::init_elements()
{
	qdTriggerElementList::iterator it;

	FOR_EACH(elements_,it)
		(*it) -> clear_object_trigger_references();

	FOR_EACH(elements_,it)
		(*it) -> add_object_trigger_reference();

	return true;
}

bool qdTriggerChain::is_element_in_list(qdNamedObject const* p) const
{
	qdTriggerElementList::const_iterator it;
	FOR_EACH(elements_,it){
		if((*it) -> object() == p)
			return true;
	}
	return false;
}

bool qdTriggerChain::is_element_in_list(qdTriggerElementConstPtr p) const
{
	qdTriggerElementList::const_iterator it;
	FOR_EACH(elements_,it){
		if(*it == p || ((*it) -> object() && (*it) -> object() == p -> object()))
			return true;
	}
	return false;
}

bool qdTriggerChain::add_link(qdTriggerElementPtr from,qdTriggerElementPtr to,int link_type,bool auto_restart)
{
	if(!from -> add_child(to,link_type,auto_restart)) return false;
	if(!to -> add_parent(from,link_type)) return false;

	return true;
}

bool qdTriggerChain::remove_link(qdTriggerElementPtr from,qdTriggerElementPtr to)
{
	if(!from -> remove_child(to)) return false;
	if(!to -> remove_parent(from)) return false;

	return true;
}

bool qdTriggerChain::load_script(const xml::tag* p)
{
	int id0,id1,tp0,tp1 = 0;
	qdTriggerElementPtr el,el1;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_TRIGGER_CHAIN_ROOT:
			root_element()->load_script(&*it);
			root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);
			break;
		case QDSCR_TRIGGER_ELEMENT:
			el = qdTriggerElementPtr(new qdTriggerElement);
			el -> load_script(&*it);
#ifdef __QD_TRIGGER_PROFILER__
			el -> set_owner(this);
#endif
			elements_.push_back(el);
			break;
		case QDSCR_TRIGGER_ELEMENT_LINK:
			xml::tag_buffer(*it) > id0 > id1 > tp0;

			el = search_element(id0);
			el1 = search_element(id1);

			if(el && el1){
				bool auto_restart = false;
				if(const xml::tag* tp = it -> search_subtag(QDSCR_TRIGGER_ELEMENT_LINK_AUTO_RESTART)){
					if(xml::tag_buffer(*tp).get_int())
						auto_restart = true;
					else
						auto_restart = false;
				}
				add_link(el,el1,tp0,auto_restart);
			}
			break;
#ifdef _QUEST_EDITOR
		case QDSCR_TRIGGER_PARENT_LINK_CHILD_OFFSET:
			xml::tag_buffer(*it) > id0 > id1 > tp0 > tp1;

			el = search_element(id0);
			el1 = search_element(id1);
			if(el && el1)
				el -> set_parent_link_child_offset(el1,tp0,tp1);
			break;
		case QDSCR_TRIGGER_PARENT_LINK_OWNER_OFFSET:
			xml::tag_buffer(*it) > id0 > id1 > tp0 > tp1;

			el = search_element(id0);
			el1 = search_element(id1);
			if(el && el1)
				el -> set_parent_link_owner_offset(el1,tp0,tp1);
			break;
		case QDSCR_TRIGGER_CHILD_LINK_CHILD_OFFSET:
			xml::tag_buffer(*it) > id0 > id1 > tp0 > tp1;

			el = search_element(id0);
			el1 = search_element(id1);
			if(el && el1)
				el -> set_child_link_child_offset(el1,tp0,tp1);
			break;
		case QDSCR_TRIGGER_CHILD_LINK_OWNER_OFFSET:
			xml::tag_buffer(*it) > id0 > id1 > tp0 > tp1;

			el = search_element(id0);
			el1 = search_element(id1);
			if(el && el1)
				el -> set_child_link_owner_offset(el1,tp0,tp1);
			break;
		case QDSCR_TRIGGER_BOUND: {
			xml::tag_buffer buf(*it);
			m_rcBound.left = buf.get_int();
			m_rcBound.top = buf.get_int();
			m_rcBound.right = buf.get_int();
			m_rcBound.bottom = buf.get_int();
			}
			break;
		case QDSCR_TRIGGER_CHAIN_WORK_AREA: {
			xml::tag_buffer buf(*it);
			m_rcWorkArea.left = buf.get_int();
			m_rcWorkArea.top = buf.get_int();
			m_rcWorkArea.right = buf.get_int();
			m_rcWorkArea.bottom = buf.get_int();
			}
			break;
		case QDSCR_TRIGGER_CHAIN_LAYOUT: {
			xml::tag_buffer buf(*it);
			m_szGenLayout.cx = buf.get_int();
			m_szGenLayout.cy = buf.get_int();
			}
			break;
#endif
		}
	}

	root_element()->retrieve_link_elements(this);
	for(qdTriggerElementList::iterator it = elements_.begin(); it != elements_.end(); ++it)
		(*it) -> retrieve_link_elements(this);

	for(qdTriggerLinkList::iterator itl = root_element()->children().begin(); 
		itl != root_element()->children().end(); 
		++itl)
		itl -> activate();

	return true;
}

bool qdTriggerChain::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<trigger_chain name=";

	if(name())
		fh < "\"" < qdscr_XML_string(name()) < "\"";
	else
		fh < "\" \"";

#ifdef _QUEST_EDITOR
	fh < " trigger_bound=\"" <= m_rcBound.left < " " <= m_rcBound.top < " " <= m_rcBound.right < " " <= m_rcBound.bottom < "\"";
	fh < " work_area=\"" <= m_rcWorkArea.left < " " <= m_rcWorkArea.top < " " <= m_rcWorkArea.right < " " <= m_rcWorkArea.bottom < "\"";
	fh < " layout=\"" <= m_szGenLayout.cx < " " <= m_szGenLayout.cy < "\"";
#endif

	fh < ">\r\n";

	root_element()->save_script(fh,indent + 1);

	qdTriggerElementList::const_iterator it;
	FOR_EACH(elements_,it)
		(*it) -> save_script(fh,indent + 1);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</trigger_chain>\r\n";

	return true;
}

void qdTriggerChain::quant(float dt)
{
#ifndef _QUEST_EDITOR
	root_element()->quant(dt);

	qdTriggerElementList::iterator it;
	FOR_EACH(elements_,it)
		(*it) -> quant(dt);
#endif
}

bool qdTriggerChain::init_debug_check()
{
	root_element() -> debug_set_done();
	for(qdTriggerElementList::iterator it = elements_.begin(); it != elements_.end(); ++it)
		(*it) -> debug_set_done();

#ifdef __QD_TRIGGER_PROFILER__
	if(qdTriggerProfiler::instance().is_logging_enabled())
		qdTriggerProfiler::instance().set_read_only(false);
#endif

	if(root_element() -> is_active()){
		root_element() -> debug_set_active();
		root_element() -> set_status(qdTriggerElement::TRIGGER_EL_DONE);
		
		for(qdTriggerLinkList::iterator itl = root_element() -> children().begin(); itl != root_element() -> children().end(); ++itl){
			itl -> activate();
#ifdef __QD_TRIGGER_PROFILER__
			if(!qdTriggerProfiler::instance().is_read_only()){
				qdTriggerProfilerRecord rec(qdGameDispatcher::get_dispatcher() -> time(),qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE,this,root_element() -> ID(),itl -> element() -> ID(),itl -> status());
				qdTriggerProfiler::instance().add_record(rec);
			}
#endif
		}

#ifdef __QD_TRIGGER_PROFILER__
	if(qdTriggerProfiler::instance().is_logging_enabled())
		qdTriggerProfiler::instance().set_read_only(true);
#endif
	}

	for(qdTriggerElementList::iterator it = elements_.begin(); it != elements_.end(); ++it){
		if((*it) -> is_active())
			(*it) -> debug_set_active();
	}

	return true;
}

const char* const qdTriggerChain::debug_comline()
{
	static const char* const arg = "triggers_debug";
	return arg;
}

bool qdTriggerChain::load_data(qdSaveStream& fh,int save_version)
{
	int size;
	fh > size;

	if(size != elements_.size()) return false;

	if(!root_element()->load_data(fh,save_version)) return false;

	for(qdTriggerElementList::iterator it = elements_.begin(); it != elements_.end(); ++it){
		if(!(*it) -> load_data(fh,save_version))
			return false;
	}

	return true;
}

bool qdTriggerChain::save_data(qdSaveStream& fh) const
{
	fh < elements_.size();

	if(!root_element() -> save_data(fh)) return false;

	for(qdTriggerElementList::const_iterator it = elements_.begin(); it != elements_.end(); ++it){
		if(!(*it) -> save_data(fh))
			return false;
	}

	return true;
}

void qdTriggerChain::reset()
{
#ifdef __QD_TRIGGER_PROFILER__
	qdTriggerProfiler::instance().set_read_only(true);
#endif

	root_element()->reset();

	for(qdTriggerElementList::iterator it = elements_.begin(); it != elements_.end(); ++it)
		(*it) -> reset();

	root_element() -> set_status(qdTriggerElement::TRIGGER_EL_DONE);

	for(qdTriggerLinkList::iterator itl = root_element() -> children().begin(); itl != root_element() -> children().end(); ++itl)
		itl -> activate();
	
#ifdef __QD_TRIGGER_PROFILER__
	qdTriggerProfiler::instance().set_read_only(false);
#endif
}

bool qdTriggerChain::activate_links(const qdNamedObject* from)
{
	bool ret = false;

	for(qdTriggerElementList::const_iterator it = elements_.begin(); it != elements_.end(); ++it){
		if((*it) -> object() == from){
			for(qdTriggerLinkList::iterator itl = (*it) -> children().begin(); itl != (*it) -> children().end(); ++itl)
				itl -> activate();

			ret = true;
		}
	}

	return ret;
}

bool qdTriggerChain::deactivate_object_triggers(const qdNamedObject* p)
{
	bool ret = false;

	for(qdTriggerElementList::const_iterator it = elements_.begin(); it != elements_.end(); ++it){
		if((*it) -> object()){
			const qdNamedObject* obj = (*it) -> object() -> owner(qdNamedObjectType(p -> named_object_type()));
			if(obj == p){
				(*it) -> deactivate(p);
				ret = true;
			}
		}
	}

	return ret;
}
