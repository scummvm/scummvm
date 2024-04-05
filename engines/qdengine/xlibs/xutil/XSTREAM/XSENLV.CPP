#include "xglobal.h"

static char* openMSG	 = "CREATE/OPEN FAILURE";
static char* closeMSG	 = "CLOSE FAILURE";
static char* appendMSG	 = "APPENDING FAILURE";

bool XStream::open(const char* name, unsigned f)
{
	close();

	DWORD fa = 0;
	DWORD fs = 0;
	DWORD fc = 0;
	DWORD ff = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;

	if(f & XS_APPEND)
		f |= XS_OUT;

	if(f & XS_IN) 
		fa |= GENERIC_READ;
	if(f & XS_OUT) 
		fa |= GENERIC_WRITE;

	if(!(f & XS_NOSHARING)) 
		fs |= FILE_SHARE_READ | FILE_SHARE_WRITE;

	if((f & XS_IN) || (f & XS_NOREPLACE)) 
		fc = OPEN_EXISTING;
	else if(f & XS_OUT) 
		fc = f & XS_APPEND ? OPEN_ALWAYS : CREATE_ALWAYS;

	if(f & XS_NOBUFFERING) 
		ff |= FILE_FLAG_NO_BUFFERING;

	handler = CreateFile(name,fa,fs,0,fc,ff,0);
	if(handler == INVALID_HANDLE_VALUE)
		if(handleErrors_) 
			ErrH.Abort(openMSG,XERR_USER,GetLastError(),name);
		else{
			ioError_ = true;
			return false;
		}

	fname = name;
	pos = SetFilePointer(handler, 0, 0, f & XS_APPEND ? FILE_END : FILE_CURRENT);
	eofFlag = 0;
	ioError_ = false;
	return true;
}

bool XStream::open(XStream* owner,long s,long ext_sz)
{
	fname = owner -> fname;
	handler = owner -> handler;
	pos = 0;
	owner -> seek(s,XS_BEG);
	eofFlag = owner -> eof();
	extSize = ext_sz;
	extPos = s;
	ioError_ = owner->ioError();
	return ioError_;
}

void XStream::close()
{
	if(handler == INVALID_HANDLE_VALUE) 
		return;

	if(extSize == -1 && !CloseHandle(handler) && handleErrors_)
		ErrH.Abort(closeMSG,XERR_USER,GetLastError(),fname);

	handler = INVALID_HANDLE_VALUE;
	fname = NULL;
	pos = 0L;
	eofFlag = 1;
	extSize = -1;
	extPos = 0;
	ioError_ = false;
}

