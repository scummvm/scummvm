#ifndef __VISTA_ENGINE_EDITABLE_CONDITION_H_INCLUDED__
#define __VISTA_ENGINE_EDITABLE_CONDITION_H_INCLUDED__
#include "Serialization\Serialization.h"
#include "TriggerEditor\TriggerExport.h"

struct EditableCondition {
	EditableCondition(Condition* _condition = 0)
	: condition(_condition)
	{

	}

	~EditableCondition()
	{

	}

	void serialize (Archive& ar) {
		ar.serialize(condition, "condition", "Condition");
	}
	ShareHandle<Condition> condition;
};

#endif // __VISTA_ENGINE_EDITABLE_CONDITION_H_INCLUDED__
