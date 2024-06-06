
#ifndef __QD_CONDITION_OBJECT_REFERENCE_H__
#define __QD_CONDITION_OBJECT_REFERENCE_H__

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_named_object_reference.h"


namespace QDEngine {

class qdNamedObject;

class qdConditionObjectReference {
public:
	qdConditionObjectReference();
	qdConditionObjectReference(const qdConditionObjectReference &ref);
	~qdConditionObjectReference();

	qdConditionObjectReference &operator = (const qdConditionObjectReference &ref);

	void set_object(const qdNamedObject *p);
	bool find_object();

	const qdNamedObject *object() const {
		return object_;
	}

	bool is_empty() const {
		if (object_) return true;
		else return false;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(XStream &fh, int indent = 0, int id = 0) const;

private:

#ifdef _QUEST_EDITOR
	qdNamedObjectReference object_reference_;
#endif
	const qdNamedObject *object_;
};

} // namespace QDEngine

#endif /* __QD_CONDITION_OBJECT_REFERENCE_H__ */
