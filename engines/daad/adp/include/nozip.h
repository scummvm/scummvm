/*
 * MIT License
 * 
 * Copyright (c) 2016 Ivan Vashchaev
 * Copyright (c) 2023 José Luis Cebrián
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NOZIP_H
#define NOZIP_H

#include <time.h>

#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

struct ZIP_Entry {
    uint64_t uncompressed_size;
    uint64_t compressed_size;
    uint64_t local_header_offset;
    const char *filename;
    time_t mtime;
};

extern size_t ZIP_Read     (ZIP_Entry **ptr, File *stream);
extern int    ZIP_Store    (File *stream, const char *filename, const void *data, size_t size);
extern bool   ZIP_Seek     (File *stream, const ZIP_Entry *entry);
extern bool   ZIP_Extract  (File *stream, const ZIP_Entry *entry, void *data, size_t size);
extern int    ZIP_Finalize (File *stream);

#endif // NOZIP_H

#ifdef NOZIP_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stb_inflate.h>

#pragma pack(push,1)

struct ZIP_LocalFileHeader {
    uint32_t signature;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc_32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
};

struct ZIP_CentralDirHeader {
    uint32_t signature;
    uint16_t version;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc_32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t local_header_offset;
};

struct ZIP_EndOfCentralDirRecord64 {
    uint32_t signature;
    uint64_t eocdr_size;
    uint16_t version;
    uint16_t version_needed;
    uint32_t disk_number;
    uint32_t cdr_disk_number;
    uint64_t disk_num_entries;
    uint64_t num_entries;
    uint64_t cdr_size;
    uint64_t cdr_offset;
};

struct ZIP_EndOfCenttralDirLocator64 {
    uint32_t signature;
    uint32_t eocdr_disk;
    uint64_t eocdr_offset;
    uint32_t num_disks;
};

struct ZIP_EndOfCentralDirRecord {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t cdr_disk_number;
    uint16_t disk_num_entries;
    uint16_t num_entries;
    uint32_t cdr_size;
    uint32_t cdr_offset;
    uint16_t ZIP_file_comment_length;
};

#pragma pack(pop)

size_t ZIP_Read(ZIP_Entry **ptr, File *stream) {
    // find the end of central directory record
    uint32_t signature;
    uint64_t offset;
	uint64_t size = File_GetSize(stream);
    for (offset = sizeof(ZIP_EndOfCentralDirRecord);; ++offset) {
        if ( offset > UINT16_MAX || 
			!File_Seek(stream, size-offset) || 
			 File_Read(stream, &signature, sizeof(signature)) != sizeof(signature))
            return 0;
        if (signature == 0x06054B50)
            break;
    }

    // read end of central directory record
    ZIP_EndOfCentralDirRecord eocdr;
    if (!(File_Seek(stream, size-offset) &&
          File_Read(stream, &eocdr, sizeof(eocdr)) == sizeof(eocdr) &&
          eocdr.signature == 0x06054B50 &&
          eocdr.disk_number == 0 &&
          eocdr.cdr_disk_number == 0 &&
          eocdr.disk_num_entries == eocdr.num_entries))
        return 0;

    // check for zip64
    ZIP_EndOfCentralDirRecord64 eocdr64;
    int zip64 = eocdr.num_entries == UINT16_MAX || eocdr.cdr_offset == UINT32_MAX || eocdr.cdr_size == UINT32_MAX;
    if (zip64) {
        // zip64 end of central directory locator
        ZIP_EndOfCenttralDirLocator64 eocdl64;
        if (!(File_Seek(stream, size-offset - sizeof(eocdl64)) &&
              File_Read(stream, &eocdl64, sizeof(eocdl64)) == sizeof(eocdl64) &&
              eocdl64.signature == 0x07064B50 &&
              eocdl64.eocdr_disk == 0 &&
              eocdl64.num_disks == 1))
            return 0;
        // zip64 end of central directory record
        if (!(File_Seek(stream, eocdl64.eocdr_offset) &&
              File_Read(stream, &eocdr64, sizeof(eocdr64)) == sizeof(eocdr64) &&
              eocdr64.signature == 0x06064B50 &&
              eocdr64.disk_number == 0 &&
              eocdr64.cdr_disk_number == 0 &&
              eocdr64.disk_num_entries == eocdr64.num_entries))
            return 0;
    }

    // seek to central directory record
    if (!File_Seek(stream, zip64 ? eocdr64.cdr_offset : eocdr.cdr_offset))
        return 0;

    // alloc buffer for entries array and filenames
    ZIP_Entry *entries = (ZIP_Entry *)Allocate<uint8_t>("ZIP File Entries", zip64 ? eocdr64.cdr_size : eocdr.cdr_size);
    if (!entries)
        return 0;

    // store filenames after entries array
    char *strings = (char *)(entries + (zip64 ? eocdr64.num_entries : eocdr.num_entries));

    for (size_t i = 0, i_end = zip64 ? eocdr64.num_entries : eocdr.num_entries; i < i_end; ++i) {
        // read central directory header, filename, extra field and skip comment
        ZIP_CentralDirHeader cdh;
        if (!(File_Read(stream, &cdh, sizeof(cdh)) == sizeof(cdh) &&
              cdh.signature == 0x02014B50 &&
              File_Read(stream, strings, cdh.file_name_length + cdh.extra_field_length) == cdh.file_name_length + cdh.extra_field_length &&
              File_Seek(stream, File_GetPosition(stream) + cdh.file_comment_length))) {
            Free(entries);
            return 0;
        }

        ZIP_Entry *entry = entries + i;
        entry->uncompressed_size = cdh.uncompressed_size;
        entry->compressed_size = cdh.compressed_size;
        entry->local_header_offset = cdh.local_header_offset;

        // find zip64 extended information extra field
        for (char *extra = strings + cdh.file_name_length; extra != strings + cdh.file_name_length + cdh.extra_field_length;) {
            uint16_t header_id;
            memcpy(&header_id, extra, sizeof(header_id));
            extra += sizeof(header_id);

            uint16_t data_size;
            memcpy(&data_size, extra, sizeof(data_size));
            extra += sizeof(data_size);

            switch (header_id) {
            case 0x0001:
                if (cdh.uncompressed_size == UINT32_MAX) {
                    memcpy(&entry->uncompressed_size, extra, sizeof(entry->uncompressed_size));
                    extra += sizeof(entry->uncompressed_size);
                }
                if (cdh.compressed_size == UINT32_MAX) {
                    memcpy(&entry->compressed_size, extra, sizeof(entry->compressed_size));
                    extra += sizeof(entry->compressed_size);
                }
                if (cdh.local_header_offset == UINT32_MAX) {
                    memcpy(&entry->local_header_offset, extra, sizeof(entry->local_header_offset));
                    extra += sizeof(entry->local_header_offset);
                }
                if (cdh.disk_number_start == UINT16_MAX) {
                    extra += sizeof(uint32_t);
                }
                break;
            default:
                extra += data_size;
                break;
            }
        }

        entry->filename = strings;
        strings += cdh.file_name_length;
        *strings++ = '\0';

		struct tm t  = { 0 };
		t.tm_sec     = (cdh.last_mod_file_time << 1) & 0x3F;
		t.tm_min     = (cdh.last_mod_file_time >> 5) & 0x3F;
		t.tm_hour    = (cdh.last_mod_file_time >> 11) & 0x1F;
		t.tm_mday    = cdh.last_mod_file_date & 0x1F;
		t.tm_mon     = ((cdh.last_mod_file_date >> 5) & 0xF) - 1;
		t.tm_year    = ((cdh.last_mod_file_date >> 9) & 0x7F) + 1980 - 1900;
		t.tm_isdst   = -1;
        entry->mtime = mktime(&t);
    }

    *ptr = entries;
    return zip64 ? eocdr64.num_entries : eocdr.num_entries;
}

bool ZIP_Seek(File *stream, const ZIP_Entry *entry) {
    ZIP_LocalFileHeader lfh;
    return File_Seek(stream, entry->local_header_offset) &&
           File_Read(stream, &lfh, sizeof(lfh)) == sizeof(lfh) &&
           lfh.signature == 0x04034B50 &&
           File_Seek(stream, File_GetPosition(stream) + lfh.file_name_length + lfh.extra_field_length);
}

bool ZIP_Extract(File *stream, const ZIP_Entry *entry, void *data, size_t size) {
	if (!ZIP_Seek(stream, entry))
		return false;
	uint8_t* buffer = Allocate<uint8_t>("ZIP Decompression Buffer", entry->compressed_size);
	if (!buffer)
		return false;

	bool success = false;
	if (File_Read(stream, buffer, entry->compressed_size) == entry->compressed_size)
	{
		stbi__stream stream = { 0 };
		stream.start_in  = buffer;
		stream.end_in    = buffer + entry->compressed_size;
		stream.next_in   = stream.start_in;
		stream.start_out = (uint8_t*)data;
		stream.end_out   = (uint8_t*)data + size;
		stream.next_out  = stream.start_out;
		success = stb_inflate(&stream) != 0;
	}
	Free(buffer);
	return success;
}

int ZIP_Store(File *stream, const char *filename, const void *data, size_t size) {
    uint64_t offset = File_GetPosition(stream);
    if (offset == -1)
        return 1;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    ZIP_LocalFileHeader lfh = { 0 };
	lfh.signature           = 0x04034B50;
	lfh.version_needed      = 10;
	lfh.flags               = 0;
	lfh.compression_method  = 0;
	lfh.last_mod_file_time  = tm->tm_hour << 11 | tm->tm_min << 5 | tm->tm_sec >> 1;
	lfh.last_mod_file_date  = (tm->tm_year - 80) << 9 | (tm->tm_mon + 1) << 5 | tm->tm_mday;
	lfh.crc_32              = 0;
	lfh.compressed_size     = size;
	lfh.uncompressed_size   = size;
	lfh.file_name_length    = strlen(filename);
	lfh.extra_field_length  = 0;

    File_Write(stream, &lfh, sizeof(lfh));
    File_Write(stream, filename, lfh.file_name_length);
    File_Write(stream, data, size);

    return 0;
}

int ZIP_Finalize(File *stream) {
    uint64_t offset = 0;
    ZIP_LocalFileHeader lfh;
    char filename[1024];

    ZIP_EndOfCentralDirRecord eocdr = { 0 };
	eocdr.signature                 = 0x06054B50;
	eocdr.disk_number               = 0;
	eocdr.cdr_disk_number           = 0;
	eocdr.disk_num_entries          = 0;
	eocdr.num_entries               = 0;
	eocdr.cdr_size                  = 0;
	eocdr.cdr_offset                = File_GetPosition(stream);
	eocdr.ZIP_file_comment_length   = 0;

    while (File_Seek(stream, offset) == 0 &&
           File_Read(stream, &lfh, sizeof(lfh)) == sizeof(lfh) &&
           lfh.signature == 0x04034B50 &&
           lfh.file_name_length < sizeof(filename) &&
           File_Read(stream, filename, lfh.file_name_length) == lfh.file_name_length) {

        DebugPrintf("F %.*s\n", lfh.file_name_length, filename);
        ZIP_CentralDirHeader cdh      = { 0 };
		cdh.signature                 = 0x02014B50;
		cdh.version                   = 10;
		cdh.version_needed            = lfh.version_needed;
		cdh.flags                     = lfh.flags;
		cdh.compression_method        = lfh.compression_method;
		cdh.last_mod_file_time        = lfh.last_mod_file_time;
		cdh.last_mod_file_date        = lfh.last_mod_file_date;
		cdh.crc_32                    = lfh.crc_32;
		cdh.compressed_size           = lfh.compressed_size;
		cdh.uncompressed_size         = lfh.uncompressed_size;
		cdh.file_name_length          = lfh.file_name_length;
		cdh.extra_field_length        = 0;
		cdh.file_comment_length       = 0;
		cdh.disk_number_start         = 0;
		cdh.internal_file_attributes  = 0;
		cdh.external_file_attributes  = 0;
		cdh.local_header_offset       = offset;

        File_Seek(stream, File_GetSize(stream));
        File_Write(stream, &cdh, sizeof(cdh));
        File_Write(stream, filename, lfh.file_name_length);

        ++eocdr.num_entries;
        ++eocdr.disk_num_entries;

        offset += sizeof(lfh) + lfh.file_name_length + lfh.compressed_size;
    }

    File_Seek(stream, File_GetSize(stream));
    eocdr.cdr_size = File_GetPosition(stream) - eocdr.cdr_offset;
    File_Write(stream, &eocdr, sizeof(eocdr));

    return 0;
}

#endif // NOZIP_IMPLEMENTATION
