#include "file.h"

bool File::open(const char *filename, AccessMode mode) {
	f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb");
	return (f != NULL);
}

void File::close() {
	fclose(f);
	f = NULL;
}

int File::seek(int32 offset, int whence) {
	return fseek(f, offset, whence);
}

long File::read(void *buffer, int len) {
	return fread(buffer, 1, len, f);
}
void File::write(const void *buffer, int len) {
	fwrite(buffer, 1, len, f);
}

bool File::eof() {
	return feof(f) != 0;
}

byte File::readByte() {
	byte v;
	read(&v, sizeof(byte));
	return v;
}

uint16 File::readWord() {
	uint16 v;
	read(&v, sizeof(uint16));
	return FROM_LE_16(v);
}

uint32 File::readLong() {
	uint32 v;
	read(&v, sizeof(uint32));
	return FROM_LE_32(v);
}

void File::writeByte(byte v) {
	write(&v, sizeof(byte));
}

void File::writeWord(uint16 v) {
	uint16 vTemp = TO_LE_16(v);
	write(&vTemp, sizeof(uint16));
}

void File::writeLong(uint32 v) {
	uint32 vTemp = TO_LE_32(v);
	write(&vTemp, sizeof(uint32));
}

void File::writeString(const char *s) {
	write(s, strlen(s) + 1);
}

uint32 File::pos() {
	return ftell(f);
}
