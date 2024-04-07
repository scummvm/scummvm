#include "xglobal.h"

static char *seekMSG	= "BAD SEEK";
static char *flushMSG	= "FLUSH FILE BUFFERS ERROR";
static char *sizeMSG	= "FILE SIZE CALCULATION ERROR";

long XStream::seek(long offset, int dir)
{
	long ret;

	if(extSize != -1){
		switch(dir){
			case XS_BEG:
				ret = SetFilePointer(handler,extPos + offset,0,dir) - extPos;
				break;
			case XS_END:
				ret = SetFilePointer(handler,extPos + extSize - offset - 1,0,XS_BEG) - extPos;
				break;
			case XS_CUR:
				ret = SetFilePointer(handler,extPos + pos + offset,0,XS_BEG) - extPos;
				break;
		}
	}
	else
		ret = SetFilePointer(handler,offset,0,dir);

	if (ret == -1L)
		if (handleErrors_) ErrH.Abort(seekMSG,XERR_USER,GetLastError(),fname);
			 else return -1L;
	if (ret >= size() - 1) eofFlag = 1;  else eofFlag = 0;
	return pos = ret;
}

void XStream::flush()
{
       if (!FlushFileBuffers(handler) && handleErrors_)
		ErrH.Abort(flushMSG,XERR_USER,GetLastError(),fname);
}

long XStream::size() const
{
	long tmp = extSize;
	
	if(tmp == -1){
		tmp=GetFileSize(handler,0);
		if (tmp == -1L)
			if (handleErrors_) ErrH.Abort(sizeMSG,XERR_USER,GetLastError(),fname);
				 else return -1;
	}
	return tmp;
}

