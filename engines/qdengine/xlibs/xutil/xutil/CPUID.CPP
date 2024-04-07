/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#define CPUINFODLL "cpuinf32.dll"

unsigned int xt_processor_id = 0;
unsigned int xt_processor_type = 0;
unsigned int xt_processor_family = 0;
unsigned int xt_processor_model = 0;
unsigned int xt_processor_revision = 0;
unsigned int xt_processor_features = 0;

int xt_mmxUse;

unsigned int xt_get_cpuid(){
	unsigned int type,family,model,revision,id,clone;
	type = family = model = revision = id = clone = 0;

	HINSTANCE hLibrary = LoadLibrary(CPUINFODLL);
	if(!hLibrary){
/*
		char buf[256];
		sprintf(buf, "Error loading library: %s\n", CPUINFODLL);
		ErrH.Abort(buf);
*/
		return 0;
	}
	typedef WORD (FAR *WORDvoid)();
	typedef DWORD (FAR *DWORDvoid)();

	WORD (FAR *lpfnwincpuidsupport)();
	WORD (FAR *lpfnwincpuidext)();
	DWORD (FAR *lpfnwincpufeatures)();

	lpfnwincpuidsupport = (WORDvoid )GetProcAddress(hLibrary,"wincpuidsupport");
	lpfnwincpuidext = (WORDvoid )GetProcAddress(hLibrary,"wincpuidext");
	lpfnwincpufeatures = (DWORDvoid )GetProcAddress(hLibrary,"wincpufeatures");

	if(lpfnwincpuidsupport()){
		id = lpfnwincpuidext();
		xt_processor_type = (id & ((1 << 16)) - 1) >> 12;
		xt_processor_family = (id & ((1 << 12)) - 1) >> 8;
		xt_processor_model = (id & ((1 << 8)) - 1) >> 4;
		xt_processor_revision = id & ((1 << 4)) - 1;
		xt_processor_features = lpfnwincpufeatures() & 0xFFFF0000;
		return 1;
	}
	else return 0;
}

char* xt_getMMXstatus()
{
	static char message[32];
	strcpy(message,"MMX ");

	if(xt_get_cpuid())
		if(xt_processor_features & INTEL_MMX){
			xt_mmxUse = 1;
			strcat(message,"detected...");
			}

	if(!xt_mmxUse) strcat(message,"is absent...");

	return message;
}
