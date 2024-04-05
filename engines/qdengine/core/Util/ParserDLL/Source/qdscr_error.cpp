/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_error.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void (*qdscr_ErrFnc)(char*) = 0;

int qdscr_ErrMode = 1;

void qdscr_SetErrorOutHandler(void (*hfnc)(char*))
{
	qdscr_ErrFnc = hfnc;
}

void qdscr_SetErrorMode(int mode)
{
	qdscr_ErrMode = mode;
}

void qdscr_Error(char* msg)
{
	if(qdscr_ErrFnc)
		(*qdscr_ErrFnc)(msg);
	else
		ErrH.Abort(msg);
//		printf(msg);

	if(qdscr_ErrMode)
		exit(1);
}

void qdscr_Error(char* msg,int code,char* text)
{
	static char msg_buf[1024];
	if(text)
		sprintf(msg_buf,msg,code,text);
	else
		sprintf(msg_buf,msg,code);

	qdscr_Error(msg_buf);
}
