/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_error_handler.h"
#include "qd_resource_file.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool qd_open_resource(const char* file_name,class XStream& fh,bool readonly,bool err_message)
{
	if(!readonly){
		fh.open(file_name,XS_OUT);
		return true;
	}

//	int err = fh.ErrHUsed;
//	fh.ErrHUsed = 0;

	int result = fh.open(file_name,XS_IN);
	if(!result){
		if(err_message){
			while(!result){
				switch(app_errH.message_box(file_name,appErrorHandler::ERR_FILE_NOT_FOUND,appErrorHandler::ERR_MB_ABORTRETRYIGNORE)){
					case appErrorHandler::ERR_RETRY:
						result = fh.open(file_name,XS_IN);
						break;
					case appErrorHandler::ERR_IGNORE:
//						fh.ErrHUsed = err;
						return false;
					case appErrorHandler::ERR_ABORT:
						#ifndef _QUEST_EDITOR
						exit(1);
						#else	
//						fh.ErrHUsed = err;
						return false;
						#endif
				}
			}
		}

//		fh.ErrHUsed = err;
		return false;
	}
	else {
//		fh.ErrHUsed = err;
		return true;
	}
}
