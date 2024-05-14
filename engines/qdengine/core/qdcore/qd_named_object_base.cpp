/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"

#include "qdengine/core/qdcore/qd_named_object_base.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdNamedObjectBase::qdNamedObjectBase() {
}

qdNamedObjectBase::qdNamedObjectBase(const qdNamedObjectBase &obj) : name_(obj.name_) {
}

qdNamedObjectBase::~qdNamedObjectBase() {
}

qdNamedObjectBase &qdNamedObjectBase::operator = (const qdNamedObjectBase &obj) {
	if (this == &obj) return *this;

	name_ = obj.name_;

	return *this;
}
