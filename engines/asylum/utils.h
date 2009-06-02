#ifndef ASYLUM_UTILS_H
#define ASYLUM_UTILS_H

namespace Asylum {
	
uint32 read32( unsigned char *value, int offset )
{
	uint32 val = (byte)value[offset] | 
				 (byte)value[offset + 1] << 8 | 
				 (byte)value[offset + 2] << 16 | 
				 (byte)value[offset + 3] << 24;
	return val;
}	

uint16 read16( unsigned char *value, int offset )
{
	uint16 val = (byte)value[offset] | (byte)value[offset + 1] << 8;
	return val;
}
	
	
} // end of namespace Asylum

#endif
