#include "xglobal.h"

static char* readMSG	= "WRONG READING";
static char* writeMSG	= "WRONG WRITING";

unsigned xsReadBytes = 0;
unsigned xsReadBytesDelta = 0;
unsigned xsWriteBytes = 0;
unsigned xsWriteBytesDelta = 0;

unsigned xsRdWrResolution = 500000U;

void (*xsReadHandler)(unsigned) = NULL;
void (*xsWriteHandler)(unsigned) = NULL;

void xsSetReadHandler(void (*fp)(unsigned),unsigned res)
{
	xsReadHandler = fp;
	xsRdWrResolution = res;
	xsReadBytes = 0;
}

void xsSetWriteHandler(void (*fp)(unsigned),unsigned res)
{
	xsWriteHandler = fp;
	xsRdWrResolution = res;
	xsWriteBytes = 0;
}

unsigned long XStream::read(void* buf, unsigned long len)
{
	unsigned long ret;
	if(!ReadFile(handler,buf,len,&ret,0))
		if(handleErrors_) 
			ErrH.Abort(readMSG,XERR_USER,GetLastError(),fname);
		else{
			ioError_ = true;
			return 0U;
		}

	if(ret < len) 
		eofFlag = 1;
	pos += ret;
	if(extSize != -1 && pos >= extSize) 
		eofFlag = 1;

	if(xsReadHandler){
		xsReadBytesDelta += ret;
		if(xsReadBytesDelta >= xsRdWrResolution){
			xsReadBytes += xsReadBytesDelta;
			xsReadBytesDelta = 0;
			(*xsReadHandler)(xsReadBytes);
		}
	}
	return ret;
}

unsigned long XStream::write(const void* buf, unsigned long len)
{
	unsigned long ret;
	if(!WriteFile(handler,buf,len,&ret,0))
		if(handleErrors_) 
			ErrH.Abort(writeMSG,XERR_USER,GetLastError(),fname);
		else{
			ioError_ = true;
			return 0U;
		}
	pos += ret;

	if(xsWriteHandler){
		xsWriteBytesDelta += ret;
		if(xsWriteBytesDelta >= xsRdWrResolution){
			xsWriteBytes += xsWriteBytesDelta;
			xsWriteBytesDelta = 0;
			(*xsWriteHandler)(xsWriteBytes);
		}
	}
	return ret;
}

