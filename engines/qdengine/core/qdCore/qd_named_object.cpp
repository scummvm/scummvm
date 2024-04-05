
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_named_object.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdNamedObject::qdNamedObject() : owner_(0),
	trigger_reference_count_(0),
#ifdef _QUEST_EDITOR
	ref_owner_(0),
#endif
	flags_(0)
{
}

qdNamedObject::qdNamedObject(const qdNamedObject& obj) : qdNamedObjectBase(obj),
	owner_(obj.owner_),
#ifdef _QUEST_EDITOR
	ref_owner_(obj.ref_owner_),
#endif
	flags_(obj.flags_),
	trigger_reference_count_(0)
{
}

qdNamedObject::~qdNamedObject()
{
}

qdNamedObject& qdNamedObject::operator = (const qdNamedObject& obj)
{
	if(this == &obj) return *this;

	*static_cast<qdNamedObjectBase*>(this) = obj;

	flags_ = obj.flags_;
	owner_ = obj.owner_;
#ifdef _QUEST_EDITOR
	ref_owner_ = obj.ref_owner_;
#endif
	return *this;
}

qdNamedObject* qdNamedObject::owner(qdNamedObjectType tp) const
{
	qdNamedObject* p = owner();

	while(p){
		if(p -> named_object_type() == tp) return p;
		p = p -> owner();
	}

	return NULL;
}

bool qdNamedObject::load_data(qdSaveStream& fh,int save_version)
{
	fh > flags_;

	return true;
}

bool qdNamedObject::save_data(qdSaveStream& fh) const
{
	fh < flags_;

	return true;
}

appLog& operator << (appLog& log,const qdNamedObject* obj)
{
	int owners_count = 0;

	const qdNamedObject* p = obj -> owner();
	while(p){
		p = p -> owner();
		owners_count++;
	}

	for(int i = owners_count; i > 0; i--){
		const qdNamedObject* p = obj;
		for(int j = 0; j < i; j++)
			p = p -> owner();

		if(p -> name())
			log << p -> name() << "::";
	}

	if(obj -> name())
		log << obj -> name();

	return log;
}

