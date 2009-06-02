#ifndef ASYLUM_RESOURCE_H
#define ASYLUM_RESOURCE_H

#include "common/str.h"
#include "common/array.h"

namespace Asylum {

class ResourceItem;

class Resource {
public:    
	Resource();
    ~Resource();
    
	int load( Common::String filename );
	void dump();
	
	uint32 getSize() { return _size; }
	uint32 getNumEntries() { return _numEntries; }
	
private:
	uint32 getNextValidOffset( uint8 startPos );

    Common::String _filename;
	bool _loaded;
	uint32 _size;
	uint32 _numEntries;
	Common::Array<ResourceItem> _items;

}; // end of class Resource

class ResourceItem {
public:
	ResourceItem();
	~ResourceItem();
	
	void dump();		
	int save( Common::String filename );
	
	uint32 offset;
	uint32 size; 		
	unsigned char *data;

}; // end of class ResourceItem


} // end of namespace Asylum

#endif
