/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_interface_object_base.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdInterfaceObjectBase::qdInterfaceObjectBase() : owner_(NULL)
{
}

qdInterfaceObjectBase::qdInterfaceObjectBase(const qdInterfaceObjectBase& obj) : qdNamedObjectBase(obj),
	owner_(obj.owner_)
{
}

qdInterfaceObjectBase::~qdInterfaceObjectBase()
{
}

qdInterfaceObjectBase& qdInterfaceObjectBase::operator = (const qdInterfaceObjectBase& obj)
{
	if(this == &obj) return *this;

	*static_cast<qdNamedObjectBase*>(this) = obj;

	owner_ = obj.owner_;

	return *this;
}
