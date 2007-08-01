#ifdef __SYMBIAN32__
	#undef feof
	#undef clearerr
	
	#define FILE void
	
	FILE* 	symbian_fopen(const char* name, const char* mode);
	void 	symbian_fclose(FILE* handle);
	size_t 	symbian_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
	size_t 	symbian_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
	bool 	symbian_feof(FILE* handle);
	long int symbian_ftell(FILE* handle);
	int 	symbian_fseek(FILE* handle, long int offset, int whence);
	void 	symbian_clearerr(FILE* handle);

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode) 					symbian_fopen(name, mode)
	#define fclose(handle) 						symbian_fclose(handle)
	#define fread(ptr, size, items, file)		symbian_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		symbian_fwrite(ptr, size, items, file)
	#define feof(handle)						symbian_feof(handle)
	#define ftell(handle)						symbian_ftell(handle)
	#define fseek(handle, offset, whence)		symbian_fseek(handle, offset, whence)
	#define clearerr(handle)					symbian_clearerr(handle)
#endif
	
#if defined(UNIX) || defined(__SYMBIAN32__)
#include <errno.h>
#endif