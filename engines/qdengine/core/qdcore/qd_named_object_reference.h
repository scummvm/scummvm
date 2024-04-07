#ifndef __QD_NAMED_OBJECT_REFERENCE_H__
#define __QD_NAMED_OBJECT_REFERENCE_H__

#include "xml_fwd.h"
#include "qd_named_object.h"

class qdNamedObjectReference
{
public:
	qdNamedObjectReference();
	explicit qdNamedObjectReference(const qdNamedObject* p);
	qdNamedObjectReference(int levels,const int* types,const char* const* names);
	qdNamedObjectReference(const qdNamedObjectReference& ref);
	~qdNamedObjectReference();

	qdNamedObjectReference& operator = (const qdNamedObjectReference& ref);

	int num_levels()			const { return object_types_.size(); }
	int object_type(int level = 0)		const { return object_types_[level]; }
	const char* object_name(int level = 0)  const { return object_names_[level].c_str(); }
	bool is_empty()				const { return object_types_.empty(); }

	bool init(const qdNamedObject* p);

	void clear(){
		object_types_.clear();
		object_names_.clear();
	}

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	qdNamedObject* object() const;

private:

	std::vector<int> object_types_;
	std::vector<std::string> object_names_;

	static int objects_counter_;
};

#endif /* __QD_NAMED_OBJECT_REFERENCE_H__ */

