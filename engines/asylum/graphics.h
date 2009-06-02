#ifndef ASYLUM_GRAPHICS_H
#define ASYLUM_GRAPHICS_H

#include "common/str.h"

namespace Asylum {

class GraphicResource {
public:
    GraphicResource();
    ~GraphicResource();
    
    int loadResource(Common::String Filename);
    void dump();
	
private:
    Common::String _filename;    
    uint8 _packSize;
    uint8 _tagValue;
    uint8 _flag;
    uint8 _contentOffset;
    uint8 _unknown1;
    uint8 _unknown2;
    uint8 _unknown3;
    uint8 _numEntries;
    uint8 _maxWidthSize;
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
