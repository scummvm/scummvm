#include "unistd.h"
#include "extend.h"	// for SCUMMVM_SAVEPATH


// currently used only to retreive savepath
Char *getcwd(Char *buf, UInt32 size) {
	Char *copy = buf;
	
	if (!copy)
		copy = (Char *)MemPtrNew(StrLen(SCUMMVM_SAVEPATH)); // this may never occured
	
	StrCopy(copy, SCUMMVM_SAVEPATH);
	return copy;
}