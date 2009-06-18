/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

/* Patch extractor
 * (C) 2008 Andrea Corna
 * 
 * This source code is adopted and striped for Residual project.
 *
 * res_system functions are taken from system.c written by Stuart Caie
 * from libmspack (http://www.cabextract.org.uk/libmspack/).
 *
 * Patch Extractor is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "tools/patchex/mspack.h"

// Command line actions
enum act { UNKNOWN_ACTION, CABINET_ACTION, ALL_LANGUAGES_ACTION, LOCALISED_ACTION};

// Languages codes
#define LANG_ALL "@@"
const char *kLanguages_ext[] = { "English", "French", "German", "Italian", "Portuguese", "Spanish", NULL};
const char *kLanguages_code[] = {  "US", "FR", "GE", "IT", "PT", "SP",  NULL };

// Extraction constans
#define RAND_A			(0x343FD)
#define RAND_B			(0x269EC3)
#define CODE_TABLE_SIZE		(0x100)
#define CONTAINER_OFFSET	(0x1C000)
#define CABINET_OFFSET		(CONTAINER_OFFSET + 8)
#define CONTAINER_MAGIC		"1CNT"

#define BUFFER_SIZE 		102400
char lang[3];

// Some useful type and function
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

uint32 READ_LE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
}

struct mspack_file_p {
	FILE *fh;
	const char *name;
	uint16 *CodeTable;
};

uint16 *create_dec_table(uint32 key) {
	uint32 value;
	uint16 *dectable;
	unsigned int i;

	value = key;
	dectable = (uint16 *)malloc( CODE_TABLE_SIZE * 2);

	for (i = 0; i < CODE_TABLE_SIZE; i++) {
		value = RAND_A * value + RAND_B;
		dectable[i] = (uint16)((value >> 16) & 0x7FFF);
	}

	return dectable;
}

static struct mspack_file *res_open(struct mspack_system *handle, const char *filename, int mode) {
	struct mspack_file_p *fh;
	const char *fmode;
	uint32 magic, key;
	
	switch (mode) {
		case MSPACK_SYS_OPEN_READ:   fmode = "rb";  break;
		case MSPACK_SYS_OPEN_WRITE:  fmode = "wb";  break;
		case MSPACK_SYS_OPEN_UPDATE: fmode = "r+b"; break;
		case MSPACK_SYS_OPEN_APPEND: fmode = "ab";  break;
		default: return NULL;
	}
	
	fh = (mspack_file_p *)malloc(sizeof(struct mspack_file_p));

	fh->name = filename;
	if (!(fh->fh = fopen(filename, fmode))) {
		free(fh);
		return NULL;
	}
	
	fh->CodeTable = NULL;
	
	if (mode != MSPACK_SYS_OPEN_READ)
		return (struct mspack_file *)fh;

	handle->seek((struct mspack_file *)fh, (off_t)CONTAINER_OFFSET, MSPACK_SYS_SEEK_START);
	uint8 count = handle->read((struct mspack_file *) fh, &magic, 4);
	
	if (count == 4 && memcmp(&magic, CONTAINER_MAGIC, 4) == 0) {
		handle->read((struct mspack_file *)fh, &key, 4);
		key = READ_LE_UINT32(&key);
		fh->CodeTable = create_dec_table(key);
	}

	handle->seek((struct mspack_file *)fh, (off_t) 0, MSPACK_SYS_SEEK_START);

	return (struct mspack_file *)fh;
}

static void res_close(struct mspack_file *file) {
	struct mspack_file_p *handle = (struct mspack_file_p *)file;

	if (handle) {
		if (handle->CodeTable)
			free(handle->CodeTable);
		fclose(handle->fh);
		free(handle);
	}
}

static int res_seek(struct mspack_file *file, off_t offset, int mode) {
	struct mspack_file_p *handle = (struct mspack_file_p *)file;

	if (handle) {
		switch (mode) {
		case MSPACK_SYS_SEEK_START:
			mode = SEEK_SET;
			if (handle->CodeTable)
				offset += (CONTAINER_OFFSET + 8);
			break;
		case MSPACK_SYS_SEEK_CUR:   mode = SEEK_CUR; break;
		case MSPACK_SYS_SEEK_END:   mode = SEEK_END; break;
		default: return -1;
		}
		return fseek(handle->fh, (int)offset, mode);
	}
	return -1;
}

static off_t res_tell(struct mspack_file *file) {
	struct mspack_file_p *handle = (struct mspack_file_p *)file;

	if (handle) {
		off_t offset = ftell(handle->fh);
		if (handle->CodeTable)
			offset -= CABINET_OFFSET;
		return offset;
	} else
		return 0;
}

void decode(uint8 *data, unsigned int size, uint16 *dectable, unsigned int start_point) {
	unsigned int i;
	for (i = 0; i < size; i++)
		data[i] = (data[i] ^ (uint8) dectable[(i + start_point) % CODE_TABLE_SIZE]) - (uint8)(dectable[(i + start_point) % CODE_TABLE_SIZE] >> 8);
}

static int res_read(struct mspack_file *file, void *buffer, int bytes) {
	struct mspack_file_p *handle = (struct mspack_file_p *)file;

	if (handle) {
		unsigned int start_point = (unsigned int)res_tell(file);
		size_t count = fread(buffer, 1, (size_t) bytes, handle->fh);

		if (!ferror(handle->fh)) {
			if (handle->CodeTable)
				decode((uint8*)buffer, count, handle->CodeTable, start_point);
			return (int) count;
		}
	}
	return -1;
}

static int res_write(struct mspack_file *file, void *buffer, int bytes) {
	struct mspack_file_p *handle = (struct mspack_file_p *)file;

	if (handle) {
		if (handle->CodeTable)
			return -1;
		size_t count = fwrite(buffer, 1, (size_t)bytes, handle->fh);
		if (!ferror(handle->fh)) return (int) count;
	}
	return -1;
}

static void res_msg(struct mspack_file *file, char *format, ...) {
	va_list ap;

	if (file)
		fprintf(stderr, "%s: ", ((struct mspack_file_p *)file)->name);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc((int) '\n', stderr);
	fflush(stderr);
}

static void *res_alloc(struct mspack_system *, size_t bytes) {
	void *memory;
	memory = malloc(bytes);
	if (memory == NULL) {
		printf("Insufficient memory!\n");
		exit(1);
	} else
		return memory;
}

static void res_free(void *buffer) {
	free(buffer);
}

static void res_copy(void *src, void *dest, size_t bytes) {
	memcpy(dest, src, bytes);
}

static struct mspack_system res_system = {
	&res_open, &res_close, &res_read,  &res_write, &res_seek,
	&res_tell, &res_msg, &res_alloc, &res_free, &res_copy, NULL
};

void extract_cabinet(char *filename, unsigned int lenght) {
	struct mspack_file *original_executable, *destination_cabinet;
	void *buffer;
	unsigned int copied_bytes;
	int count;

	original_executable = res_open(&res_system, filename, MSPACK_SYS_OPEN_READ);
	destination_cabinet = res_open(&res_system, "gfupd101.cab", MSPACK_SYS_OPEN_WRITE);
	
	buffer = res_alloc(NULL, BUFFER_SIZE);
	copied_bytes = 0;

	while (copied_bytes < lenght) {
		count = res_read(original_executable, buffer, BUFFER_SIZE);
		res_write(destination_cabinet, buffer, count);
		copied_bytes  += count;
	}
	printf("Update cabinet extracted as gfupd101.cab.\n");

	res_free(buffer);
	res_close(original_executable);
	res_close(destination_cabinet);
}

char *filter_none(struct mscabd_file *file) {
	char *filename;

	filename = (char *)malloc(strlen(file->filename) + 1);
	strcpy(filename, file->filename);
	return filename;
}

char *filter_localised(struct mscabd_file *file) {
	char *filename, file_lang[3];

	filename = (char *)malloc(strlen(file->filename) + 1);
	sscanf(file->filename, "%2s_%s",file_lang, filename);
	if (strcmp(file_lang, lang) == 0 || strcmp(file_lang, LANG_ALL) == 0)
		return filename;
	else
		return NULL;
}

void extract_files(struct mscab_decompressor *cabd, struct mscabd_cabinet *cab, char *(* filter) (struct mscabd_file *)) {
	unsigned int files_extracted = 0;
	struct mscabd_file *file;
	char *filename;

	for (file = cab->files; file; file = file->next) {
		if ((filename = filter(file))) {
			if (cabd->extract(cabd, file, filename) != MSPACK_ERR_OK) {
				printf("Extract error on %s!\n", file->filename);
				continue;
			}
			printf("%s extracted as %s\n", file->filename, filename);
			++files_extracted;
			free(filename);
		}
	}

	printf("%d file extracted.\n", files_extracted);
}

int main(int argc, char *argv[]) {
	struct mscab_decompressor *cabd;
	struct mscabd_cabinet *cab;
	int i;
	enum act action;
	char *(*filter) (struct mscabd_file *);

	action = UNKNOWN_ACTION;

	// Argument checks and usage display
	if (argc != 3) {
		printf("Usage: patchex gfupd101.exe LanguageCode/SpecialAction\n");
		printf("Available languages:\n");
		for (i = 0; kLanguages_code[i]; i++)
			printf("-%s (%s)\n", kLanguages_ext[i], kLanguages_code[i]);
		printf("Special actions:\n");
		printf("- ALL: Extracts all files with their original names.\n");
		printf("- CABINET: Extracts only the cabinet\n");
		exit(1);
	}

	// Actions check
	// Cabinet check
	if (strcasecmp("CABINET", argv[2]) == 0) {
		printf("Cabinet extraction selected\n");
		action = CABINET_ACTION;
	}

	// All languages check
	if (strcasecmp("ALL", argv[2]) == 0) {
		printf("All languages selected\n");
		action = ALL_LANGUAGES_ACTION;
	}

	// Language check
	for(i = 0; kLanguages_code[i]; i++)
		if (strcasecmp(kLanguages_code[i], argv[2]) == 0 || strcasecmp(kLanguages_ext[i], argv[2]) == 0) {
			printf("%s selected.\n", kLanguages_ext[i]);
			strcpy(lang, kLanguages_code[i]);
			action = LOCALISED_ACTION;
			break;
		}

	// Unknown action
	if (action == UNKNOWN_ACTION) {
		printf("Unknown language or action!\n");
		exit(1);
	}


	// Extraction !
	if ((cabd = mspack_create_cab_decompressor(&res_system)) != MSPACK_ERR_OK) {
		if ((cab = cabd->open(cabd, argv[1])) != MSPACK_ERR_OK) {
			if (action == CABINET_ACTION)
				extract_cabinet(argv[1], cab->length);
			else {
				switch (action) {
					case ALL_LANGUAGES_ACTION:	filter = &filter_none;		break;
					case LOCALISED_ACTION:		filter = &filter_localised;	break;
					default: printf("Internal error!\n"); exit(1);			break;
				}
				extract_files(cabd, cab, filter);
			}
			cabd->close(cabd, cab);
		} else
			printf("Unable to open %s!\n", argv[1]);
		mspack_destroy_cab_decompressor(cabd);
	} else {
		printf("Internal error!\n");
		exit(1);
	}

	return 0;
}
