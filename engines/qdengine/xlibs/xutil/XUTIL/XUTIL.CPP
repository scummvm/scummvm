
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

char *memoutMSG = "OUT OF MEMORY";

static unsigned int XRndValue = 83838383;

unsigned int XRnd(unsigned int m)
{ 
	XRndValue = XRndValue*214013L + 2531011L;
	if(!m)
		return 0;
	return ((XRndValue>> 16) & 0x7fff) % m; 
}

void XRndSet(unsigned int m)
{
	XRndValue = m;
}

unsigned int XRndGet()
{
	return XRndValue;
}


void* xalloc(unsigned sz)
{
	if(!sz) return NULL;
	void* p = GlobalAlloc(GMEM_FIXED,sz);
	if(!p) ErrH.Abort(memoutMSG,XERR_USER,sz);
	return p;
}

void* xrealloc(void* p0,unsigned sz)
{
	if(!sz) return NULL;
	void* p = GlobalReAlloc(p0,sz,NULL);
	if(!p) ErrH.Abort(memoutMSG,-1,GetLastError());
	return p;
}

void xfree(void* p)
{
	GlobalFree(p);
}

static WIN32_FIND_DATA FFdata;
static HANDLE FFh;

char* XFindNext()
{
	if(FindNextFile(FFh,&FFdata) == TRUE){
		if(FFdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return XFindNext();
		return FFdata.cFileName;
		}
	else {
		FindClose(FFh);
		return NULL;
		}
}

char* XFindFirst(char* mask)
{
	FFh = FindFirstFile(mask,&FFdata);
	if(FFh == INVALID_HANDLE_VALUE) return NULL;
	if(FFdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return XFindNext();
	return FFdata.cFileName;
}

void xtDeleteFile(char* fname)
{
	OFSTRUCT p;
	OpenFile(fname,&p,OF_DELETE);
}

