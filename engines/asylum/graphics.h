#ifndef ASYLUM_GRAPHICS_H
#define ASYLUM_GRAPHICS_H

#include "common/str.h"
#include "common/array.h"

#include "asylum/resource.h"

namespace Asylum {

class GraphicResource {
public:
    GraphicResource( ResourceItem item );
    ~GraphicResource();
    
	void dump();
	
private:
    Common::String _filename;    
    uint32 _packSize;
    uint32 _tagValue;
    uint32 _flag;
    uint32 _contentOffset;
    uint32 _unknown1;
    uint32 _unknown2;
    uint32 _unknown3;
    uint16 _numEntries;
    uint16 _maxWidthSize;
	
}; // end of class GraphicResource

class GraphicAsset {
public:
    GraphicAsset();
    ~GraphicAsset();
	
private:	
    uint8 _offset;
    uint8 _size;
    uint8 _flag;
    uint8 _offsetWidth;  // screenX position?
    uint8 _offsetHeight; // screenY position?
    uint8 _width;
    uint8 _height;
    uint8 _dataSize;     // data block size
    unsigned char *_data;
	
}; // end of class GraphicAsset

} // end of namespace Asylum

#endif
