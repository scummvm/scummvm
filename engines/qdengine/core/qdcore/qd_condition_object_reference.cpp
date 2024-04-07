/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag.h"
#include "qdscr_parser.h"

#include "qd_game_dispatcher.h"
#include "qd_named_object_indexer.h"
#include "qd_condition_object_reference.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdConditionObjectReference::qdConditionObjectReference() : object_(NULL)
{
}

qdConditionObjectReference::qdConditionObjectReference(const qdConditionObjectReference& ref) : 
#ifdef _QUEST_EDITOR
	object_reference_(ref.object_reference_),
#endif
	object_(ref.object_)
{
}

qdConditionObjectReference::~qdConditionObjectReference()
{
}

qdConditionObjectReference& qdConditionObjectReference::operator = (const qdConditionObjectReference& ref)
{
	if(this == &ref) return *this;

#ifdef _QUEST_EDITOR
	object_reference_ = ref.object_reference_;
#endif

	object_ = ref.object_;

	return *this;
}

void qdConditionObjectReference::set_object(const qdNamedObject* p)
{
	object_ = p;
}

bool qdConditionObjectReference::find_object()
{
#ifdef _QUEST_EDITOR
	if(object_reference_.is_empty())
		return false;

	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		object_ = dp -> get_named_object(&object_reference_);

		if(object_)
			return true;
	}
#endif
	return false;
}

bool qdConditionObjectReference::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAMED_OBJECT: {
#ifndef _QUEST_EDITOR
			qdNamedObjectReference& ref = qdNamedObjectIndexer::instance().add_reference((qdNamedObject*&)object_);
			ref.load_script(&*it);
#else
			object_reference_.load_script(&*it);
#endif
			}
			break;
		}
	}
	
	return true;	
}

bool qdConditionObjectReference::save_script(XStream& fh,int indent,int id) const
{
	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "<condition_object ID=\"" <= id < "\">\r\n";

	if(object_){
		qdNamedObjectReference ref(object_);
		ref.save_script(fh,indent + 1);
	}

	for(int i = 0; i < indent; i++) fh < "\t";
	fh < "</condition_object>\r\n";
	
	return true;	
}

