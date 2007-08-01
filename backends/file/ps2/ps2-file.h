#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fopen(a, b)			ps2_fopen(a, b)
	#define fclose(a)			ps2_fclose(a)
	#define fseek(a, b, c)			ps2_fseek(a, b, c)
	#define ftell(a)			ps2_ftell(a)
	#define feof(a)				ps2_feof(a)
	#define fread(a, b, c, d)		ps2_fread(a, b, c, d)
	#define fwrite(a, b, c, d)		ps2_fwrite(a, b, c, d)

	//#define fprintf				ps2_fprintf	// used in common/util.cpp
	//#define fflush(a)			ps2_fflush(a)	// used in common/util.cpp

	//#define fgetc(a)			ps2_fgetc(a)	// not used
	//#define fgets(a, b, c)			ps2_fgets(a, b, c)	// not used
	//#define fputc(a, b)			ps2_fputc(a, b)	// not used
	//#define fputs(a, b)			ps2_fputs(a, b)	// not used

	//#define fsize(a)			ps2_fsize(a)	// not used -- and it is not a standard function either
#endif