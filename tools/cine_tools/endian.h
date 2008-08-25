#ifndef CINE_ENDIAN_H
#define CINE_ENDIAN_H

#include <stdio.h> // fread etc
#include "typedefs.h" // uint8 etc

/* NOTE: These big endian reading functions assume we're on a little endian machine. */
static uint16 READ_BE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 8) | b[1];
}

static uint32 READ_BE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

static uint16 freadUint16BE(FILE *fp) {
	char data[2];
	fread(data, 2, 1, fp);
	return READ_BE_UINT16(data);
}

static uint32 freadUint32BE(FILE *fp) {
	char data[4];
	fread(data, 4, 1, fp);
	return READ_BE_UINT32(data);
}

#endif
