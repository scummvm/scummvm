#ifndef PNODEFS_H
#define PNODEFS_H

#include "PNOLoader.h"

// Defined in PnoLoader, used to preinit PnoCall
typedef struct PnoEntryHeader {
	UInt32 r10Value;
	UInt32 pnoMainAddress;
	UInt32 userDataP;
} PnoEntryHeader;

#define ALIGN_4BYTE(addr) (((UInt32)(addr) + 3) & 0xFFFFFFFC)

enum {
	ARM_COMMON = 0,
	ARM_ENGINE,
	ARM_COUNT
};

enum {
	RSC_ARMCOMMON = 100,
	RSC_ARMENGINE = 101
};

#endif
