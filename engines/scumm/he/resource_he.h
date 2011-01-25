/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Parts of this code are heavily based on:
 * icoutils - A set of programs dealing with MS Windows icons and cursors.
 * Copyright (C) 1998-2001 Oskar Liljeblad
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_HE_RESOURCE_HE_H
#define SCUMM_HE_RESOURCE_HE_H

#include "common/macresman.h"

namespace Scumm {

#define WINRES_ID_MAXLEN (256)

/*
 * Definitions
 */

#define MZ_HEADER(x)	((DOSImageHeader *)(x))

#define STRIP_RES_ID_FORMAT(x) (x != NULL && (x[0] == '-' || x[0] == '+') ? ++x : x)

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_SIZEOF_SHORT_NAME 8

#define	IMAGE_RESOURCE_NAME_IS_STRING		0x80000000
#define	IMAGE_RESOURCE_DATA_IS_DIRECTORY	0x80000000

#define PE_HEADER(module) \
    ((Win32ImageNTHeaders*)((byte *)(module) + \
		(((DOSImageHeader*)(module))->lfanew)))

#define PE_SECTIONS(module) \
    ((Win32ImageSectionHeader *)((byte *) &PE_HEADER(module)->optional_header + \
                           PE_HEADER(module)->file_header.size_of_optional_header))

#define IMAGE_DOS_SIGNATURE    0x5A4D     /* MZ */
#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */

/* The following symbols below and another group a few lines below are defined in
 * the windows header, at least in wince and most likely in plain win32 as well.
 * Defining them out silences a redefinition warning in gcc.
 * If the same problem arises in win32 builds as well, please replace
 * _WIN32_WCE with _WIN32 which is also defined in the wince platform.
 */
#ifndef _WIN32_WCE
#define IMAGE_SCN_CNT_CODE			0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA		0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA	0x00000080
#endif

// Only IMAGE_DIRECTORY_ENTRY_RESOURCE is used:
#define	IMAGE_DIRECTORY_ENTRY_EXPORT		0
#define	IMAGE_DIRECTORY_ENTRY_IMPORT		1
#define	IMAGE_DIRECTORY_ENTRY_RESOURCE		2
#define	IMAGE_DIRECTORY_ENTRY_EXCEPTION		3
#define	IMAGE_DIRECTORY_ENTRY_SECURITY		4
#define	IMAGE_DIRECTORY_ENTRY_BASERELOC		5
#define	IMAGE_DIRECTORY_ENTRY_DEBUG			6
#define	IMAGE_DIRECTORY_ENTRY_COPYRIGHT		7
#define	IMAGE_DIRECTORY_ENTRY_GLOBALPTR		8   /* (MIPS GP) */
#define	IMAGE_DIRECTORY_ENTRY_TLS			9
#define	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10
#define	IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11
#define	IMAGE_DIRECTORY_ENTRY_IAT			12  /* Import Address Table */
#define	IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT	13
#define	IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

#ifndef _WIN32_WCE
// Only RT_GROUP_CURSOR and RT_GROUP_ICON are used
#define RT_CURSOR        1
#define RT_BITMAP        2
#define RT_ICON          3
#define RT_MENU          4
#define RT_DIALOG        5
#define RT_STRING        6
#define RT_FONTDIR       7
#define RT_FONT          8
#define RT_ACCELERATOR   9
#define RT_RCDATA        10
#define RT_MESSAGELIST   11
#define RT_GROUP_CURSOR  12
#define RT_GROUP_ICON    14
#endif

#define RETURN_IF_BAD_POINTER(r, x) \
	if (!check_offset(fi->memory, fi->total_size, _fileName.c_str(), &(x), sizeof(x))) \
		return (r);
#define RETURN_IF_BAD_OFFSET(r, x, s) \
	if (!check_offset(fi->memory, fi->total_size, _fileName.c_str(), x, s)) \
		return (r);

class ScummEngine_v70he;

class ResExtractor {
public:
	ResExtractor(ScummEngine_v70he *scumm);
	virtual ~ResExtractor();

	void setCursor(int id);

	virtual int extractResource(int id, byte **buf) { return 0; }
	virtual int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
							 int *hotspot_x, int *hotspot_y, int *keycolor,
							 byte **palette, int *palSize) { return 0; }

	enum {
		MAX_CACHED_CURSORS = 10
	};

	struct CachedCursor {
		bool valid;
		int id;
		byte *bitmap;
		int w, h;
		int hotspot_x, hotspot_y;
		uint32 last_used;
		byte *palette;
		int palSize;
	};

	ScummEngine_v70he *_vm;

	ResExtractor::CachedCursor *findCachedCursor(int id);
	ResExtractor::CachedCursor *getCachedCursorSlot();

	bool _arg_raw;
	Common::String _fileName;
	CachedCursor _cursorCache[MAX_CACHED_CURSORS];
};

class Win32ResExtractor : public ResExtractor {
 public:
	Win32ResExtractor(ScummEngine_v70he *scumm);
	~Win32ResExtractor() {}
	int extractResource(int id, byte **data);
	void setCursor(int id);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);

 private:
	int extractResource_(const char *resType, char *resName, byte **data);
/*
 * Structures
 */

#include "common/pack-start.h"	// START STRUCT PACKING

	struct WinLibrary {
		Common::SeekableReadStream *file;
		byte *memory;
		byte *first_resource;
		int total_size;
	} PACKED_STRUCT;

	struct WinResource {
		char id[256];
		void *this_;
		void *children;
		int level;
		bool numeric_id;
		bool is_directory;

		Common::String getQuotedResourceId() const;
	} PACKED_STRUCT;


	struct Win32IconResDir {
		byte width;
		byte height;
		byte color_count;
		byte reserved;
	} PACKED_STRUCT;

	struct Win32CursorDir {
		uint16 width;
		uint16 height;
	} PACKED_STRUCT;

	struct Win32CursorIconDirEntry {
		union {
			Win32IconResDir icon;
			Win32CursorDir cursor;
		} res_info;
		uint16 plane_count;
		uint16 bit_count;
		uint32 bytes_in_res;
		uint16 res_id;
	} PACKED_STRUCT;

	struct Win32CursorIconDir {
		uint16 reserved;
		uint16 type;
		uint16 count;
		Win32CursorIconDirEntry entries[1];
	} PACKED_STRUCT;

	struct Win32CursorIconFileDirEntry {
		byte width;
		byte height;
		byte color_count;
		byte reserved;
		uint16 hotspot_x;
		uint16 hotspot_y;
		uint32 dib_size;
		uint32 dib_offset;
	} PACKED_STRUCT;

	struct Win32CursorIconFileDir {
		uint16 reserved;
		uint16 type;
		uint16 count;
		Win32CursorIconFileDirEntry entries[1];
	} PACKED_STRUCT;

	struct Win32BitmapInfoHeader {
		uint32 size;
		int32 width;
		int32 height;
		uint16 planes;
		uint16 bit_count;
		uint32 compression;
		uint32 size_image;
		int32 x_pels_per_meter;
		int32 y_pels_per_meter;
		uint32 clr_used;
		uint32 clr_important;
	} PACKED_STRUCT;

	struct Win32RGBQuad {
		byte blue;
		byte green;
		byte red;
		byte reserved;
	} PACKED_STRUCT;

	struct Win32ImageResourceDirectoryEntry {
		uint32 name;
		uint32 offset_to_data;
	} PACKED_STRUCT;

	struct Win16NETypeInfo {
		uint16 type_id;
		uint16 count;
		uint32 resloader;     // FARPROC16 - smaller? uint16?
	} PACKED_STRUCT;

	struct DOSImageHeader {
		uint16 magic;
		uint16 cblp;
		uint16 cp;
		uint16 crlc;
		uint16 cparhdr;
		uint16 minalloc;
		uint16 maxalloc;
		uint16 ss;
		uint16 sp;
		uint16 csum;
		uint16 ip;
		uint16 cs;
		uint16 lfarlc;
		uint16 ovno;
		uint16 res[4];
		uint16 oemid;
		uint16 oeminfo;
		uint16 res2[10];
		uint32 lfanew;
	} PACKED_STRUCT;

	struct Win32ImageFileHeader {
		uint16 machine;
		uint16 number_of_sections;
		uint32 time_date_stamp;
		uint32 pointer_to_symbol_table;
		uint32 number_of_symbols;
		uint16 size_of_optional_header;
		uint16 characteristics;
	} PACKED_STRUCT;

	struct Win32ImageDataDirectory {
		uint32 virtual_address;
		uint32 size;
	} PACKED_STRUCT;

	struct Win32ImageOptionalHeader {
		uint16 magic;
		byte major_linker_version;
		byte minor_linker_version;
		uint32 size_of_code;
		uint32 size_of_initialized_data;
		uint32 size_of_uninitialized_data;
		uint32 address_of_entry_point;
		uint32 base_of_code;
		uint32 base_of_data;
		uint32 image_base;
		uint32 section_alignment;
		uint32 file_alignment;
		uint16 major_operating_system_version;
		uint16 minor_operating_system_version;
		uint16 major_image_version;
		uint16 minor_image_version;
		uint16 major_subsystem_version;
		uint16 minor_subsystem_version;
		uint32 win32_version_value;
		uint32 size_of_image;
		uint32 size_of_headers;
		uint32 checksum;
		uint16 subsystem;
		uint16 dll_characteristics;
		uint32 size_of_stack_reserve;
		uint32 size_of_stack_commit;
		uint32 size_of_heap_reserve;
		uint32 size_of_heap_commit;
		uint32 loader_flags;
		uint32 number_of_rva_and_sizes;
		Win32ImageDataDirectory data_directory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
	} PACKED_STRUCT;

	struct Win32ImageNTHeaders {
		uint32 signature;
		Win32ImageFileHeader file_header;
		Win32ImageOptionalHeader optional_header;
	} PACKED_STRUCT;

	struct Win32ImageSectionHeader {
		byte name[IMAGE_SIZEOF_SHORT_NAME];
		union {
			uint32 physical_address;
			uint32 virtual_size;
		} misc;
		uint32 virtual_address;
		uint32 size_of_raw_data;
		uint32 pointer_to_raw_data;
		uint32 pointer_to_relocations;
		uint32 pointer_to_linenumbers;
		uint16 number_of_relocations;
		uint16 number_of_linenumbers;
		uint32 characteristics;
	} PACKED_STRUCT;

	struct Win32ImageResourceDataEntry {
		uint32 offset_to_data;
		uint32 size;
		uint32 code_page;
		uint32 resource_handle;
	} PACKED_STRUCT;

	struct Win32ImageResourceDirectory {
		uint32 characteristics;
		uint32 time_date_stamp;
		uint16 major_version;
		uint16 minor_version;
		uint16 number_of_named_entries;
		uint16 number_of_id_entries;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

/*
 * Function Prototypes
 */

	WinResource *list_resources(WinLibrary *, WinResource *, int *);
	bool read_library(WinLibrary *);
	WinResource *find_resource(WinLibrary *, const char *, const char *, const char *, int *);
	byte *get_resource_entry(WinLibrary *, WinResource *, int *);
	int do_resources(WinLibrary *, const char *, char *, char *, byte **);
	bool compare_resource_id(WinResource *, const char *);
	const char *res_type_string_to_id(const char *);

	const char *res_type_id_to_string(int);
	char *get_destination_name(WinLibrary *, char *, char *, char *);

	byte *extract_resource(WinLibrary *, WinResource *, int *, bool *, char *, char *, bool);
	int extract_resources(WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, byte **);
	byte *extract_group_icon_cursor_resource(WinLibrary *, WinResource *, char *, int *, bool);

	bool decode_pe_resource_id(WinLibrary *, WinResource *, uint32);
	WinResource *list_pe_resources(WinLibrary *, Win32ImageResourceDirectory *, int, int *);
	int calc_vma_size(WinLibrary *);
	int do_resources_recurs(WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, const char *, char *, char *, byte **);
	WinResource *find_with_resource_array(WinLibrary *, WinResource *, const char *);

	bool check_offset(byte *, int, const char *, void *, int);

	uint32 simple_vec(byte *data, uint32 ofs, byte size);

	void fix_win32_cursor_icon_file_dir_endian(Win32CursorIconFileDir *obj);
	void fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj);
	void fix_win32_cursor_icon_file_dir_entry_endian(Win32CursorIconFileDirEntry *obj);
	void fix_win32_image_section_header(Win32ImageSectionHeader *obj);
	void fix_win32_image_header_endian(Win32ImageNTHeaders *obj);
	void fix_win32_image_data_directory(Win32ImageDataDirectory *obj);
};

class MacResExtractor : public ResExtractor {

public:
	MacResExtractor(ScummEngine_v70he *scumm);
	~MacResExtractor() { }

private:
	Common::MacResManager *_resMgr;

	int extractResource(int id, byte **buf);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);
};

} // End of namespace Scumm

#endif
