/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * Parts of code heavily based on:
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

#if !defined(RESOURCE_HE_H) && !defined(DISABLE_HE)
#define RESOURCE_HE_H

namespace Scumm {

#define WINRES_ID_MAXLEN (256)

/*
 * Definitions
 */

#define ACTION_LIST 				1	/* command: list resources */
#define ACTION_EXTRACT				2	/* command: extract resources */
#define CALLBACK_STOP				0	/* results of ResourceCallback */
#define CALLBACK_CONTINUE			1
#define CALLBACK_CONTINUE_RECURS	2

#define MZ_HEADER(x)	((DOSImageHeader *)(x))
#define NE_HEADER(x)	((OS2ImageHeader *)PE_HEADER(x))
#define NE_TYPEINFO_NEXT(x) ((Win16NETypeInfo *)((byte *)(x) + sizeof(Win16NETypeInfo) + \
						    ((Win16NETypeInfo *)x)->count * sizeof(Win16NENameInfo)))
#define NE_RESOURCE_NAME_IS_NUMERIC (0x8000)

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
#define IMAGE_OS2_SIGNATURE    0x454E     /* NE */
#define IMAGE_OS2_SIGNATURE_LE 0x454C     /* LE */
#define IMAGE_OS2_SIGNATURE_LX 0x584C     /* LX */
#define IMAGE_VXD_SIGNATURE    0x454C     /* LE */
#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */

#if !defined (WIN32)
#define IMAGE_SCN_CNT_CODE			0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA		0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA	0x00000080
#endif

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

#if !defined (WIN32)
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
	if (!check_offset(fi->memory, fi->total_size, fi->file->name(), &(x), sizeof(x))) \
		return (r);
#define RETURN_IF_BAD_OFFSET(r, x, s) \
	if (!check_offset(fi->memory, fi->total_size, fi->file->name(), x, s)) \
		return (r);

class ScummEngine_v70he;

class ResExtractor {
public:
	ResExtractor(ScummEngine_v70he *scumm);
	virtual ~ResExtractor();

	void setCursor(int id);

	virtual int extractResource(int id, byte **buf) { return 0; };
	virtual int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
							 int *hotspot_x, int *hotspot_y, int *keycolor,
							 byte **palette, int *palSize) { return 0; };

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

	typedef Common::MemoryReadStream MemoryReadStream;

};

class Win32ResExtractor : public ResExtractor {
 public:
	Win32ResExtractor(ScummEngine_v70he *scumm);
	~Win32ResExtractor() {};
	int extractResource(int id, byte **data);
	void setCursor(int id);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);

 private:
	int extractResource_(const char *resType, char *resName, byte **data);
/*
 * Structures
 */

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

	struct WinLibrary {
		Common::File *file;
		byte *memory;
		byte *first_resource;
		bool is_PE_binary;
		int total_size;
	};

	struct WinResource {
		char id[256];
		void *this_;
		void *children;
		int level;
		bool numeric_id;
		bool is_directory;
	};


	struct Win32IconResDir {
		byte width;
		byte height;
		byte color_count;
		byte reserved;
	};

	struct Win32CursorDir {
		uint16 width;
		uint16 height;
	};

	struct Win32CursorIconDirEntry {
		union {
			Win32IconResDir icon;
			Win32CursorDir cursor;
		} res_info;
		uint16 plane_count;
		uint16 bit_count;
		uint32 bytes_in_res;
		uint16 res_id;
	};

	struct Win32CursorIconDir {
		uint16 reserved;
		uint16 type;
		uint16 count;
		Win32CursorIconDirEntry entries[1] GCC_PACK;
	};

	struct Win32CursorIconFileDirEntry {
		byte width;
		byte height;
		byte color_count;
		byte reserved;
		uint16 hotspot_x;
		uint16 hotspot_y;
		uint32 dib_size;
		uint32 dib_offset;
	};

	struct Win32CursorIconFileDir {
		uint16 reserved;
		uint16 type;
		uint16 count;
		Win32CursorIconFileDirEntry entries[1];
	};

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
	};

	struct Win32RGBQuad {
		byte blue;
		byte green;
		byte red;
		byte reserved;
	};

	struct Win32ImageResourceDirectoryEntry {
		union {
			struct {
				#ifdef SCUMM_BIGENDIAN
				unsigned name_is_string:1;
				unsigned name_offset:31;
	    	    #else
				unsigned name_offset:31;
				unsigned name_is_string:1;
	    	    #endif
			} s1;
			uint32 name;
			struct {
	    	    #ifdef SCUMM_BIG_ENDIAN
				uint16 __pad;
				uint16 id;
	    	    #else
				uint16 id;
				uint16 __pad;
	    	    #endif
			} s2;
		} u1;
		union {
			uint32 offset_to_data;
			struct {
	    	    #ifdef SCUMM_BIG_ENDIAN
				unsigned data_is_directory:1;
				unsigned offset_to_directory:31;
	    	    #else
				unsigned offset_to_directory:31;
				unsigned data_is_directory:1;
	    	    #endif
			} s;
		} u2;
	};

	struct Win16NETypeInfo {
		uint16 type_id;
		uint16 count;
		uint32 resloader;     // FARPROC16 - smaller? uint16?
	};

	struct Win16NENameInfo {
		uint16 offset;
		uint16 length;
		uint16 flags;
		uint16 id;
		uint16 handle;
		uint16 usage;
	};

	struct OS2ImageHeader {
		uint16 magic;
		byte ver;
		byte rev;
		uint16 enttab;
		uint16 cbenttab;
		int32 crc;
		uint16 flags;
		uint16 autodata;
		uint16 heap;
		uint16 stack;
		uint32 csip;
		uint32 sssp;
		uint16 cseg;
		uint16 cmod;
		uint16 cbnrestab;
		uint16 segtab;
		uint16 rsrctab;
		uint16 restab;
		uint16 modtab;
		uint16 imptab;
		uint32 nrestab;
		uint16 cmovent;
		uint16 align;
		uint16 cres;
		byte exetyp;
		byte flagsothers;
		uint16 fastload_offset;
		uint16 fastload_length;
		uint16 swaparea;
		uint16 expver;
	};

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
	};

	struct Win32ImageFileHeader {
		uint16 machine;
		uint16 number_of_sections;
		uint32 time_date_stamp;
		uint32 pointer_to_symbol_table;
		uint32 number_of_symbols;
		uint16 size_of_optional_header;
		uint16 characteristics;
	};

	struct Win32ImageDataDirectory {
		uint32 virtual_address;
		uint32 size;
	};

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
	};

	struct Win32ImageNTHeaders {
		uint32 signature;
		Win32ImageFileHeader file_header;
		Win32ImageOptionalHeader optional_header;
	};

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
	};

	struct Win32ImageResourceDataEntry {
		uint32 offset_to_data;
		uint32 size;
		uint32 code_page;
		uint32 resource_handle;
	};

	struct Win32ImageResourceDirectory {
		uint32 characteristics;
		uint32 time_date_stamp;
		uint16 major_version;
		uint16 minor_version;
		uint16 number_of_named_entries;
		uint16 number_of_id_entries;
	};

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

/*
 * Function Prototypes
 */

	WinResource *list_resources(WinLibrary *, WinResource *, int *);
	bool read_library(WinLibrary *);
	WinResource *find_resource(WinLibrary *, const char *, const char *, const char *, int *);
	byte *get_resource_entry(WinLibrary *, WinResource *, int *);
	int do_resources(WinLibrary *, const char *, char *, char *, int, byte **);
	bool compare_resource_id(WinResource *, const char *);
	const char *res_type_string_to_id(const char *);

	const char *res_type_id_to_string(int);
	char *get_destination_name(WinLibrary *, char *, char *, char *);

	byte *extract_resource(WinLibrary *, WinResource *, int *, bool *, char *, char *, bool);
	int extract_resources(WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, byte **);
	byte *extract_group_icon_cursor_resource(WinLibrary *, WinResource *, char *, int *, bool);

	bool decode_pe_resource_id(WinLibrary *, WinResource *, uint32);
	bool decode_ne_resource_id(WinLibrary *, WinResource *, uint16);
	WinResource *list_ne_type_resources(WinLibrary *, int *);
	WinResource *list_ne_name_resources(WinLibrary *, WinResource *, int *);
	WinResource *list_pe_resources(WinLibrary *, Win32ImageResourceDirectory *, int, int *);
	int calc_vma_size(WinLibrary *);
	int do_resources_recurs(WinLibrary *, WinResource *, WinResource *, WinResource *, WinResource *, const char *, char *, char *, int, byte **);
	char *get_resource_id_quoted(WinResource *);
	WinResource *find_with_resource_array(WinLibrary *, WinResource *, const char *);

	bool check_offset(byte *, int, const char *, void *, int);

	uint32 simple_vec(byte *data, uint32 ofs, byte size);

	void fix_win32_cursor_icon_file_dir_endian(Win32CursorIconFileDir *obj);
	void fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj);
	void fix_win32_cursor_icon_file_dir_entry_endian(Win32CursorIconFileDirEntry *obj);
	void fix_win32_image_section_header(Win32ImageSectionHeader *obj);
	void fix_os2_image_header_endian(OS2ImageHeader *obj);
	void fix_win32_image_header_endian(Win32ImageNTHeaders *obj);
	void fix_win32_image_data_directory(Win32ImageDataDirectory *obj);
};

class MacResExtractor : public ResExtractor {

public:
	MacResExtractor(ScummEngine_v70he *scumm);
	~MacResExtractor() { }
	void setCursor(int id) ;

private:
	int extractResource(int id, byte **buf);
	bool init(Common::File *in);
	void readMap(Common::File *in);
	byte *getResource(Common::File *in, const char *typeID, int16 resID, int *size);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);

	struct ResMap {
		int16 resAttr;
		int16 typeOffset;
		int16 nameOffset;
		int16 numTypes;
	};

	struct ResType {
		char  id[5];
		int16 items;
		int16 offset;
	};

	struct Resource {
		int16 id;
		int16 nameOffset;
		byte  attr;
		int32 dataOffset;
		byte  *name;
	};

	typedef Resource *ResPtr;

private:
	int _resOffset;
	int32 _dataOffset;
	int32 _dataLength;
	int32 _mapOffset;
	int32 _mapLength;
	ResMap _resMap;
	ResType *_resTypes;
	ResPtr  *_resLists;
};

} // End of namespace Scumm

#endif
