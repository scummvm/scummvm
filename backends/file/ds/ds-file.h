#ifdef __DS__

	// These functions replease the standard library functions of the same name.
	// As this header is included after the standard one, I have the chance to #define
	// all of these to my own code.
	//
	// A #define is the only way, as redefinig the functions would cause linker errors.
	
	// These functions need to be #undef'ed, as their original definition 
	// in devkitarm is done with #includes (ugh!)
	#undef feof
	#undef clearerr
	//#undef getc
	//#undef ferror
	

	//void 	std_fprintf(FILE* handle, const char* fmt, ...);	// used in common/util.cpp
	//void 	std_fflush(FILE* handle);	// used in common/util.cpp

	//char* 	std_fgets(char* str, int size, FILE* file);	// not used
	//int 	std_getc(FILE* handle);	// not used
	//char* 	std_getcwd(char* dir, int dunno);	// not used
	//void 	std_cwd(char* dir);	// not used
	//int 	std_ferror(FILE* handle);	// not used
	
	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode) 					DS::std_fopen(name, mode)
	#define fclose(handle) 						DS::std_fclose(handle)
	#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
	#define feof(handle)						DS::std_feof(handle)
	#define ftell(handle)						DS::std_ftell(handle)
	#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
	#define clearerr(handle)					DS::std_clearerr(handle)

	//#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)

	//#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
	//#define fflush(file)						DS::std_fflush(file)	// used in common/util.cpp

	//#define fgets(str, size, file)				DS::std_fgets(str, size, file)	// not used
	//#define getc(handle)						DS::std_getc(handle)	// not used
	//#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)	// not used
	//#define ferror(handle)						DS::std_ferror(handle)	// not used

#endif