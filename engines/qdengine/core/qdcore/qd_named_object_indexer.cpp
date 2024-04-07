/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_named_object_indexer.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

#ifdef __QD_DEBUG_ENABLE__
appLog& operator << (appLog& log,const qdNamedObjectReference& obj);
#endif

/* --------------------------- DEFINITION SECTION --------------------------- */

qdNamedObjectIndexer::qdNamedObjectIndexer()
{
}

qdNamedObjectIndexer::~qdNamedObjectIndexer()
{
}

qdNamedObjectIndexer& qdNamedObjectIndexer::instance()
{
	static qdNamedObjectIndexer idx;
	return idx;
}

bool qdNamedObjectIndexer::qdNamedObjectReferenceLink::resolve()
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		object_ = dp -> get_named_object(&reference_);
		if(!object_){
#ifdef __QD_DEBUG_ENABLE__
			object_ = dp -> get_named_object(&reference_);
			appLog::default_log() << "qdNamedObjectReferenceLink::resolve() failed\r\n" << reference_ << "\r\n";
#endif
		}
		else
			return true;
	}

	return false;
}

qdNamedObjectReference& qdNamedObjectIndexer::add_reference(qdNamedObject*& p)
{
	links_.push_back(qdNamedObjectReferenceLink(p));
	return links_.back().reference();
}

void qdNamedObjectIndexer::resolve_references()
{
	for(link_container_t::iterator it = links_.begin(); it != links_.end(); ++it)
		it -> resolve();
}

void qdNamedObjectIndexer::clear()
{
#ifdef _QD_DEBUG_ENABLE_
	appLog::default_log() << "qdNamedObjectIndexer::links count - " << links_.size() << "\r\n";
#endif

	links_.clear();
}
