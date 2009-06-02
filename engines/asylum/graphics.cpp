#include "asylum/graphics.h"
#include "asylum/utils.h"

#include "common/file.h"
#include "common/stream.h"

namespace Asylum {

GraphicResource::GraphicResource( ResourceItem item )
{
	printf( "%d %d %d %d\n", (char)item.data[0], (byte)item.data[1], (byte)item.data[2], (byte)item.data[3] );
	int pos = 0;
	_tagValue      = read32(item.data, pos);	
	_flag          = read32(item.data, pos+=4);
	_contentOffset = read32(item.data, pos+=4);
	_unknown1      = read32(item.data, pos+=4);
	_unknown2      = read32(item.data, pos+=4);
	_unknown3      = read32(item.data, pos+=4);
	_numEntries    = read32(item.data, pos+=4);
	_maxWidthSize  = read16(item.data, pos+=2);
}

GraphicResource::~GraphicResource()
{
}

void GraphicResource::dump()
{   
    printf( "Tag %d, Flag %d, ConOffset %d, U1 %d, U2 %d, U3 %d, Entries %d, MaxWidthSize %d\n", _tagValue, _flag, _contentOffset, _unknown1, _unknown2, _unknown3, _numEntries, _maxWidthSize );
}

} // end of namespace Asylum
