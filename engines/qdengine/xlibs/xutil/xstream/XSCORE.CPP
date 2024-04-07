#include "xglobal.h"

XStream::XStream(bool handleErrors)
{
	handleErrors_ = handleErrors;
	handler  = INVALID_HANDLE_VALUE;
	eofFlag  = 1;
	radix	 = XS_DEFRADIX;
	digits	 = XS_DEFDIGITS;
	extSize  = -1;
	extPos	 = 0;
	ioError_ = false;
}

XStream::~XStream()
{
	close();
}

XStream::XStream(const char* name, unsigned flags,bool handleErrors)
{
	handleErrors_ = handleErrors;
	handler  = INVALID_HANDLE_VALUE;
	eofFlag  = 1;
	radix	 = XS_DEFRADIX;
	digits	 = XS_DEFDIGITS;
	extSize  = -1;
	extPos	 = 0;
	ioError_ = false;
	open(name,flags);
}


