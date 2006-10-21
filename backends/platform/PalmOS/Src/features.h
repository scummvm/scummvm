#ifndef __FEATURES_H__
#define __FEATURES_H__

enum {
	ftrBufferOverlay	= 1000,
	ftrBufferBackup,
	ftrBufferHotSwap
};

#define FREE_FTR(num)	\
	{					\
		UInt32 ptr;		\
		FtrGet(appFileCreator, num, &ptr);			\
		if (ptr) FtrPtrFree(appFileCreator, num);	\
	}

#endif
