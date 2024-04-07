#include "xglobal.h"
#include <float.h>
#include "BugslayerUtil.h"

XBuffer& assertsBuffer();

void xtSysFinit();
int xtDispatchMessage(MSG* msg);

//#define _EXCEPTION_CATCH

char *defprefix 	= "XHANDLER  INFORM";

char *ctrlMSG		= "CTRL-BREAK TERMINATION";
char *exceptMSG 	= "EXCEPTION OCCURED";
char *debugMSG		= "DEBUG PROHIBITION";
char *winopenMSG	= "WINDOW NOT OPEN";
char *winclassregMSG	= "WINDOW CLASS REGISTRATION ERROR";
char *threadcreatMSG	= "THREAD CREATION ERROR";
char *semaphcreatMSG	= "SEMAPHORE CREATION ERROR";
char *rterrorMSG	= "RUN-TIME ERROR";

#pragma warning (disable : 4073)
#pragma init_seg(lib)
XErrorHandler ErrH;

static char _ConvertBuffer[CONV_BUFFER_LEN + 1];

void win32_break(char* error,char* msg);

#define HANDLE_EXEPT(string,mask)\
	{ if (ErrH.flags & (mask))\
		strcat(msg,(string));\
	    else\
		return EXCEPTION_CONTINUE_SEARCH;\
	  break; }

char* dwtoa(unsigned int a)
{
	int len = 8 - strlen(ultoa(a,_ConvertBuffer,16));
	for(int i = 0;i < len;i++)
		_ConvertBuffer[i] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}
char* wtoa(unsigned int a)
{
	int len = 4 - strlen(ultoa(a,_ConvertBuffer,16));
	for(int i = 0;i < len;i++)
		_ConvertBuffer[i] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}
char* uctoa(unsigned char a)
{
	int len = 2 - strlen(ultoa(a,_ConvertBuffer,16));
	if(len == 1)
		_ConvertBuffer[0] = '0';
	strlen(ultoa(a,_ConvertBuffer + len,16));
	return _ConvertBuffer;
}

LONG APIENTRY exHandler(EXCEPTION_POINTERS *except_info)
{
	static bool handled = false;
	if(handled)
		return EXCEPTION_CONTINUE_EXECUTION;
	handled = true;

	_clearfp();
	//_controlfp( _controlfp(0,0) & ~(0),  MCW_EM ); 
	SetUnhandledExceptionFilter(NULL);

	static char msg[100000];

	strcpy(msg, "");

	switch(except_info->ExceptionRecord->ExceptionCode)
	{
	 case EXCEPTION_INT_DIVIDE_BY_ZERO:
		 HANDLE_EXEPT("INTEGER DIVIDE ERROR",XERR_MATH)
	 case EXCEPTION_INT_OVERFLOW:
		 HANDLE_EXEPT("INTEGER OVERFLOW",XERR_MATH)
	 case EXCEPTION_ACCESS_VIOLATION:
		 HANDLE_EXEPT("MEMORY ACCESS VIOLATION",XERR_ACCESSVIOLATION)
	 case EXCEPTION_BREAKPOINT:
		 HANDLE_EXEPT("A BREAKPOINT WAS ENCOUNTERED",XERR_DEBUG)
	 case EXCEPTION_DATATYPE_MISALIGNMENT:
		 HANDLE_EXEPT("DATA TYPE MISALIGMENT",XERR_ACCESSVIOLATION)
	 case EXCEPTION_SINGLE_STEP:
		 HANDLE_EXEPT("TRACE TRAP",XERR_DEBUG)
	 case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		 HANDLE_EXEPT("OUT OF BOUNDS FOR ARRAY",XERR_ACCESSVIOLATION)
	 case EXCEPTION_FLT_DENORMAL_OPERAND:
		 HANDLE_EXEPT("FLOATING POINT DENORMAL OPERAND",XERR_FLOAT)
	 case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		 HANDLE_EXEPT("FLOATING POINT DIVIDE BY ZERO",XERR_FLOAT)
	 case EXCEPTION_FLT_INEXACT_RESULT:
		 HANDLE_EXEPT("FLOATING POINT INEXACT RESULT",XERR_FLOAT)
	 case EXCEPTION_FLT_INVALID_OPERATION:
		 HANDLE_EXEPT("FLOATING POINT INVALID OPERATION",XERR_FLOAT)
	 case EXCEPTION_FLT_OVERFLOW:
		 HANDLE_EXEPT("FLOATING POINT OVERFLOW",XERR_FLOAT)
	 case EXCEPTION_FLT_STACK_CHECK:
		 HANDLE_EXEPT("FLOATING POINT STACK CHECK FAILED",XERR_FLOAT)
	 case EXCEPTION_FLT_UNDERFLOW:
		 HANDLE_EXEPT("FLOATING POINT UNDERFLOW",XERR_FLOAT)
	 case EXCEPTION_PRIV_INSTRUCTION:
		 HANDLE_EXEPT("PRIVELEGED INSTRUCTION",XERR_ACCESSVIOLATION)
	 default:
		 HANDLE_EXEPT("UNKNOWN ERROR TYPE",XERR_UNKNOWN)
	}
	strcat(msg," AT LOCATION 0x");
	strcat(msg,strupr(itoa((int)except_info->ExceptionRecord->ExceptionAddress,_ConvertBuffer,16)));

	static int attempt_to_show_context = 0;
	if(!attempt_to_show_context){
		PCONTEXT p = except_info -> ContextRecord;
		if((p -> ContextFlags & CONTEXT_INTEGER) && (p -> ContextFlags & CONTEXT_CONTROL) &&
			(p -> ContextFlags & CONTEXT_CONTROL)){
			attempt_to_show_context = 1;
			strcat(msg,"\r\n\r\nRegisters:\r\n");
			strcat(msg,"EAX="); strcat(msg, dwtoa(p -> Eax));
			strcat(msg,"  CS="); strcat(msg, wtoa(p -> SegCs));
			strcat(msg,"  EIP="); strcat(msg, dwtoa(p -> Eip));
			strcat(msg,"  EFLAGS="); strcat(msg, dwtoa(p -> EFlags));

			strcat(msg,"\r\nEBX="); strcat(msg, dwtoa(p -> Ebx));
			strcat(msg,"  SS="); strcat(msg, wtoa(p -> SegSs));
			strcat(msg,"  ESP="); strcat(msg, dwtoa(p -> Esp));
			strcat(msg,"  EBP="); strcat(msg, dwtoa(p -> Ebp));

			strcat(msg,"\r\nECX="); strcat(msg, dwtoa(p -> Ecx));
			strcat(msg,"  DS="); strcat(msg, wtoa(p -> SegDs));
			strcat(msg,"  ESI="); strcat(msg, dwtoa(p -> Esi));
			strcat(msg,"  FS="); strcat(msg, wtoa(p -> SegFs));

			strcat(msg,"\r\nEDX="); strcat(msg, dwtoa(p -> Edx));
			strcat(msg,"  ES="); strcat(msg, wtoa(p -> SegEs));
			strcat(msg,"  EDI="); strcat(msg, dwtoa(p -> Edi));
			strcat(msg,"  GS="); strcat(msg, wtoa(p -> SegGs));
			
			strcat(msg,"\r\n\r\nBytes at CS::EIP:\r\n");
			unsigned char* code = (unsigned char*)(p -> Eip);
			int i;
			for(i = 0;i < 16;i++){
				strcat(msg, uctoa(code[i]));
				strcat(msg," "); 
				}

			strcat(msg,"\r\n\r\nStack dump:\r\n");
			unsigned int* stack = (unsigned int*)(p -> Esp);
			for(i = 0;i < 32;i++){
				strcat(msg, dwtoa(stack[i]));
				strcat(msg, (i & 7) == 7 ? "\r\n" : " "); 
				}

			strcat(msg,"\r\nCall stack:\r\n");
			int dwOpts = GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
			const char * szBuff = GetFirstStackTraceString ( dwOpts, except_info) ;
			int iterations = 100;
			while(szBuff && --iterations) {
				strcat(msg, szBuff);
				strcat(msg,"\r\n");
				szBuff = GetNextStackTraceString ( dwOpts , except_info ) ;
				}
			}
		}


	ErrH.WriteLog(exceptMSG, msg);

#if defined(_DEBUG) || defined(_EXCEPTION_CATCH)
	xtSysFinit();
	xtClearMessageQueue();
	win32_break(exceptMSG,msg);
	return EXCEPTION_CONTINUE_EXECUTION;
#else
	ErrH.Abort(exceptMSG,XERR_USER,-1,msg);
	return EXCEPTION_EXECUTE_HANDLER;
#endif
}


XErrorHandler::XErrorHandler()
{
	prefix		= defprefix;
	postfix 	= NULL;
	restore_func	= 0;
	state		= 0;
	flags		= XERR_ALL;

	flag_errorOrAssertHandling=false;

	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exHandler);
}

XErrorHandler::~XErrorHandler()
{
	if ((!state) && postfix)
		MessageBox(0,postfix,prefix,MB_OK);
}

void XErrorHandler::SetFlags(unsigned f)
{
	flags |= f;
}

void XErrorHandler::ClearFlags(unsigned f)
{
	flags ^= f;
}

void XErrorHandler::Abort(const char* message, int code, int val, const char* subj)
{
	flag_errorOrAssertHandling=true;
	static char outmsg[10000]="";
	UINT attr=MB_OK;

	if(restore_func){
		restore_func();
		restore_func = 0;
	}

	xtSysFinit();
	xtClearMessageQueue();

	strcpy(outmsg,message);
	if(subj){
		strcat(outmsg,"\r");
		strcat(outmsg,subj);
	}
	if(val){
		attr|=MB_ICONSTOP;
		if(val != -1){
			strcat(outmsg,"\r");
			strcat(outmsg,"CODE: 0x");
			strcat(outmsg,ultoa(val,_ConvertBuffer,16));
		}
	}

	state = 1;

	MessageBox(NULL,outmsg,prefix,attr | MB_TOPMOST | MB_SYSTEMMODAL);
	exit(code);
	flag_errorOrAssertHandling=false;
}

void XErrorHandler::Exit()
{
	xtSysFinit();

	if(restore_func){
		restore_func();
		restore_func = 0;
	}
	exit(0);
}

unsigned XErrorHandler::InfoFlags(unsigned f)
{
	return flags & f;
}

void XErrorHandler::SetPrefix(const char* s)
{
	prefix = s;
}

void XErrorHandler::SetPostfix(const char* s)
{
	postfix = s;
}

void XErrorHandler::SetRestore(void (*rf)())
{
	restore_func = rf;
}

void XErrorHandler::RTC(const char *file, unsigned int line, const char *expr)
{
	char msg[256];
	strcpy(msg, file);
	strcat(msg,", LINE: ");
	strcat(msg,itoa(line,_ConvertBuffer,10));
	strcat(msg,"\r\n");
	strcat(msg,expr);
	Abort(rterrorMSG,XERR_USER,-1,msg);
}

void XErrorHandler::WriteLog(char* error, char* msg)
{
	static XStream ErrHStream(0);
	if(ErrHStream.open("XErrH.log", XS_OUT)){
		ErrHStream < "--------------------------------\r\n";
		ErrHStream < prefix < "\r\n\r\n\r\n";

		if(assertsBuffer().tell())
			ErrHStream < assertsBuffer() < "\r\n";

		ErrHStream < error < "\r\n";
		ErrHStream < msg < "\r\n";
		ErrHStream < "--------------------------------\r\n";
		ErrHStream.close();
	}
}

