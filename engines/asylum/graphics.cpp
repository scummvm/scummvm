#include "asylum/graphics.h"

#include "common/file.h"
#include "common/stream.h"

namespace Asylum {

GraphicResource::GraphicResource()
{
}

GraphicResource::~GraphicResource()
{
}

int GraphicResource::loadResource(Common::String filename)
{
    Common::File* file = new Common::File;

    if(!file || !file->open(filename)){
        printf("failed to load file\n");        
        return -1;    
    }

    _filename = filename;
    _packSize = file->size();    

    file->read( &_tagValue, 4 );
    file->read( &_flag, 4 );    
    file->read( &_contentOffset, 4 );
    file->read( &_unknown1, 4 );
    file->read( &_unknown2, 4 );
    file->read( &_unknown3, 4 );
    file->read( &_numEntries, 2 );
    file->read( &_maxWidthSize, 2 );

    file->close();
    file = NULL;    

    return 0;
}

void GraphicResource::dump()
{
    printf( _filename.c_str() );
    printf( ": PackSize %d, Tag %d, Flag %d, ConOffset %d, U1 %d, U2 %d, U3 %d, Entries %d, MaxWidthSize %d\n", _packSize, _tagValue, _flag, _contentOffset, _unknown1, _unknown2, _unknown3, _numEntries, _maxWidthSize );
}

} // end of namespace Asylum
