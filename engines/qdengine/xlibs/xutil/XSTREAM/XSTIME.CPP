#include "xglobal.h"

static char *timeMSG="FILE GET TIME ERROR";

void XStream::gettime(unsigned& fdate,unsigned& ftime)
{
	fdate=0; ftime=0;
	unsigned short dt,tm;


	FILETIME ft;
	if (!GetFileTime(handler,0,0,&ft))
		if (handleErrors_) ErrH.Abort(timeMSG,XERR_USER,GetLastError(),fname);
			 else
			      return;
	if (!FileTimeToDosDateTime(&ft,&dt,&tm))
		if (handleErrors_) ErrH.Abort(timeMSG,XERR_USER,GetLastError(),fname);
			 else
			      return;

	fdate = dt;
	ftime = tm;

}

