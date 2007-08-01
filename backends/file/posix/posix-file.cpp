#include "backends/file/posix/posix-file.h"

POSIXFile::POSIXFile() : BaseFile() {
	//
}

POSIXFile::~POSIXFile() {
	close();
}

void POSIXFile::_clearerr(FILE *stream) { 
	clearerr(stream);
}

int POSIXFile::_fclose(FILE *stream) {
	return fclose(stream);
}

int POSIXFile::_feof(FILE *stream) const {
	return feof(stream);
}
FILE *POSIXFile::_fopen(const char * filename, const char * mode) {
	printf("Opened a file!\n");
	return fopen(filename, mode);
}

int POSIXFile::_fread(void *buffer, size_t obj_size, size_t num, FILE *stream) {
	return fread(buffer, obj_size, num, stream);
}

int POSIXFile::_fseek(FILE * stream, long int offset, int origin) const {
	return fseek(stream, offset, origin);
}

long POSIXFile::_ftell(FILE *stream) const {
	return ftell(stream);
}

int POSIXFile::_fwrite(const void * ptr, size_t obj_size, size_t count, FILE * stream) {
	return fwrite(ptr, obj_size, count, stream);
}
