#ifndef __QD_NAMED_OBJECT_INDEXER_H__
#define __QD_NAMED_OBJECT_INDEXER_H__

#include "qd_named_object_reference.h"

class qdNamedObjectIndexer
{
public:
	qdNamedObjectIndexer();
	~qdNamedObjectIndexer();

	void resolve_references();
	qdNamedObjectReference& add_reference(qdNamedObject*& p);

	void clear();

	static qdNamedObjectIndexer& instance();

private:

	class qdNamedObjectReferenceLink 
	{
	public:
		qdNamedObjectReferenceLink(qdNamedObject*& p) : object_(p) { }
		~qdNamedObjectReferenceLink(){ }

		bool resolve();

		qdNamedObjectReference& reference(){ return reference_; }

	private:

		qdNamedObjectReference reference_;
		qdNamedObject*& object_;
	};

	typedef std::list<qdNamedObjectReferenceLink> link_container_t;
	link_container_t links_;
};

#endif /* __QD_NAMED_OBJECT_INDEXER_H__ */

