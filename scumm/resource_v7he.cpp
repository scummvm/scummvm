/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "common/stream.h"

namespace Scumm {

/*
 * Static variables
 */
const char *res_types[] = {
	/* 0x01: */
	"cursor", "bitmap", "icon", "menu", "dialog", "string",
	"fontdir", "font", "accelerator", "rcdata", "messagelist",
	"group_cursor", NULL, "group_icon", NULL,
	/* the following are not defined in winbase.h, but found in wrc. */
	/* 0x10: */ 
	"version", "dlginclude", NULL, "plugplay", "vxd",
	"anicursor", "aniicon"
};
#define RES_TYPE_COUNT (sizeof(res_types)/sizeof(char *))

Win32ResExtractor::Win32ResExtractor(ScummEngine_v70he *scumm) {
	_vm = scumm;

	snprintf(_fileName, 256, "%s.he3", _vm->getGameName());
}

void Win32ResExtractor::setCursor(int id) {
	char buf[20];
	byte *cursorRes = 0, *cursor = 0;
	int cursorsize;
	int w = 0, h = 0, hotspot_x = 0, hotspot_y = 0, keycolor = 0;

	snprintf(buf, 20, "%d", id);

	cursorsize = extractResource("group_cursor", buf, &cursorRes);

	convertIcons(cursorRes, cursorsize, &cursor, &w, &h, &hotspot_x, &hotspot_y,
				 &keycolor);

	_vm->setCursorHotspot(hotspot_x, hotspot_y);
	_vm->setCursorFromBuffer(cursor, w, h, w);
	free(cursorRes);
	free(cursor);
}

int Win32ResExtractor::extractResource(const char *resType, char *resName, byte **data) {
	char *arg_language = NULL;
	const char *arg_type = resType;
	char *arg_name = resName;
	int arg_action = ACTION_LIST;
	int ressize = 0;

	_arg_raw = false;

	/* translate --type option from resource type string to integer */
	arg_type = res_type_string_to_id(arg_type);

	WinLibrary fi;
		
	/* initiate stuff */
	fi.memory = NULL;
	fi.file = new File;

	/* get file size */
	fi.file->open(_fileName);
	if (!fi.file->isOpen()) {
		error("Cannot open file %s", _fileName);
	}

	fi.total_size = fi.file->size();
	if (fi.total_size == -1) {
		warning("Cannot get size of file %s", fi.file->name());
		goto cleanup;
	}
	if (fi.total_size == 0) {
		warning("%s: file has a size of 0", fi.file->name());
		goto cleanup;
	}

	/* read all of file */
	fi.memory = (byte *)malloc(fi.total_size);
	if (fi.file->read(fi.memory, fi.total_size) == 0) {
		error("Cannot read from file %s", fi.file->name());
		goto cleanup;
	}

	/* identify file and find resource table */
	if (!read_library(&fi)) {
		/* error reported by read_library */
		goto cleanup;
	}

	//	verbose_printf("file is a %s\n",
	//		fi.is_PE_binary ? "Windows NT `PE' binary" : "Windows 3.1 `NE' binary");

	/* errors will be printed by the callback */
	ressize = do_resources(&fi, arg_type, arg_name, arg_language, arg_action, data);

	/* free stuff and close file */
	cleanup:
	if (fi.file != NULL)
		fi.file->close();
	if (fi.memory != NULL)
		free(fi.memory);

	return ressize;
}


/* res_type_id_to_string:
 *   Translate a numeric resource type to it's corresponding string type.
 *   (For informative-ness.)
 */
const char *Win32ResExtractor::res_type_id_to_string(int id) {
	if (id == 241)
		return "toolbar";
	if (id > 0 && id <= (int)RES_TYPE_COUNT)
		return res_types[id-1];
	return NULL;
}

/* res_type_string_to_id:
 *   Translate a resource type string to integer.
 *   (Used to convert the --type option.)
 */
const char *Win32ResExtractor::res_type_string_to_id(const char *type) {
	static const char *res_type_ids[] = {
		"-1", "-2", "-3", "-4", "-5", "-6", "-7", "-8", "-9", "-10",
		"-11", "-12", NULL, "-14", NULL, "-16", "-17", NULL, "-19",
		"-20", "-21", "-22"
	};
	int c;

	if (type == NULL)
		return NULL;

	for (c = 0 ; c < (int)RES_TYPE_COUNT ; c++) {
		if (res_types[c] != NULL && !scumm_stricmp(type, res_types[c]))
			return res_type_ids[c];
	}

	return type;
}

int Win32ResExtractor::extract_resources(WinLibrary *fi, WinResource *wr,
                            WinResource *type_wr, WinResource *name_wr,
							WinResource *lang_wr, byte **data) {
	int size;
	bool free_it;
	const char *type;
	int32 id;

	if (*data) {
		error("Win32ResExtractor::extract_resources() more than one cursor");
		return 0;
	}

	*data = extract_resource(fi, wr, &size, &free_it, type_wr->id, (lang_wr == NULL ? NULL : lang_wr->id), _arg_raw);

	if (data == NULL) {
		warning("Win32ResExtractor::extract_resources() problem with resource extraction");
		return 0;
	}

	/* get named resource type if possible */
	type = NULL;
	if ((id = strtol(type_wr->id, 0, 10)) != 0)
		type = res_type_id_to_string(id);

	debugC(DEBUG_RESOURCE, "extractCursor(). Found cursor name: %s%s%s [size=%d]",
	  get_resource_id_quoted(name_wr),
	  (lang_wr->id[0] != '\0' ? " language: " : ""),
	  get_resource_id_quoted(lang_wr), size);

	return size;
}

/* extract_resource:
 *   Extract a resource, returning pointer to data.
 */
byte *Win32ResExtractor::extract_resource(WinLibrary *fi, WinResource *wr, int *size,
                  bool *free_it, char *type, char *lang, bool raw) {
	char *str;
	int32 intval;
	
	/* just return pointer to data if raw */
	if (raw) {
		*free_it = false;
		/* get_resource_entry will print possible error */
		return get_resource_entry(fi, wr, size);
	}

	/* find out how to extract */
	str = type;
	if (str != NULL && (intval = strtol(STRIP_RES_ID_FORMAT(str), 0, 10))) {
		if (intval == (int)RT_GROUP_ICON) {
			*free_it = true;
			return extract_group_icon_cursor_resource(fi, wr, lang, size, true);
		}
		if (intval == (int)RT_GROUP_CURSOR) {
			*free_it = true;
			return extract_group_icon_cursor_resource(fi, wr, lang, size, false);
		}
	}

	return NULL;
}

/* extract_group_icon_resource:
 *   Create a complete RT_GROUP_ICON resource, that can be written to
 *   an `.ico' file without modifications. Returns an allocated
 *   memory block that should be freed with free() once used.
 *
 *   `root' is the offset in file that specifies the resource.
 *   `base' is the offset that string pointers are calculated from.
 *   `ressize' should point to an integer variable where the size of
 *   the returned memory block will be placed.
 *   `is_icon' indicates whether resource to be extracted is icon
 *   or cursor group.
 */
byte *Win32ResExtractor::extract_group_icon_cursor_resource(WinLibrary *fi, WinResource *wr, char *lang,
                                   int *ressize, bool is_icon) {
	Win32CursorIconDir *icondir;
	Win32CursorIconFileDir *fileicondir;
	byte *memory;
	int c, offset, skipped;
	int size;

	/* get resource data and size */
	icondir = (Win32CursorIconDir *)get_resource_entry(fi, wr, &size);
	if (icondir == NULL) {
		/* get_resource_entry will print error */
		return NULL;
	}

	/* calculate total size of output file */
	RETURN_IF_BAD_POINTER(NULL, icondir->count);
	skipped = 0;
	for (c = 0 ; c < icondir->count ; c++) {
		int level;
		int iconsize;
		char name[14];
		WinResource *fwr;

		RETURN_IF_BAD_POINTER(NULL, icondir->entries[c]);
		/*printf("%d. bytes_in_res=%d width=%d height=%d planes=%d bit_count=%d\n", c,
			icondir->entries[c].bytes_in_res,
			(is_icon ? icondir->entries[c].res_info.icon.width : icondir->entries[c].res_info.cursor.width),
			(is_icon ? icondir->entries[c].res_info.icon.height : icondir->entries[c].res_info.cursor.height),
			icondir->entries[c].plane_count,
			icondir->entries[c].bit_count);*/

		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", icondir->entries[c].res_id);
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, lang, &level);
		if (fwr == NULL) {
			warning("%s: could not find `%s' in `%s' resource.",
					fi->file->name(), &name[1], (is_icon ? "group_icon" : "group_cursor"));
			return NULL;
		}

		if (get_resource_entry(fi, fwr, &iconsize) != NULL) {
		    if (iconsize == 0) {
				debugC(DEBUG_RESOURCE, "%s: icon resource `%s' is empty, skipping", fi->file->name(), name);
				skipped++;
				continue;
		    }
		    if ((uint32)iconsize != icondir->entries[c].bytes_in_res) {
				debugC(DEBUG_RESOURCE, "%s: mismatch of size in icon resource `%s' and group (%d != %d)", 
					fi->file->name(), name, iconsize, icondir->entries[c].bytes_in_res);
		    }
		    size += iconsize; /* size += icondir->entries[c].bytes_in_res; */

		    /* cursor resources have two additional WORDs that contain
		     * hotspot info */
		    if (!is_icon)
			size -= sizeof(uint16)*2;
		}
	}
	offset = sizeof(Win32CursorIconFileDir) + (icondir->count-skipped) * sizeof(Win32CursorIconFileDirEntry);
	size += offset;
	*ressize = size;

	/* allocate that much memory */
	memory = (byte *)malloc(size);
	fileicondir = (Win32CursorIconFileDir *)memory;

	/* transfer Win32CursorIconDir structure members */
	fileicondir->reserved = icondir->reserved;
	fileicondir->type = icondir->type;
	fileicondir->count = icondir->count - skipped;

	/* transfer each cursor/icon: Win32CursorIconDirEntry and data */
	skipped = 0;
	for (c = 0 ; c < icondir->count ; c++) {
		int level;
		char name[14];
		WinResource *fwr;
		byte *data;
	
		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", icondir->entries[c].res_id);
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, lang, &level);
		if (fwr == NULL) {
			warning("%s: could not find `%s' in `%s' resource.",
				fi->file->name(), &name[1], (is_icon ? "group_icon" : "group_cursor"));
			return NULL;
		}

		/* get data and size of that resource */
		data = (byte *)get_resource_entry(fi, fwr, &size);
		if (data == NULL) {
			/* get_resource_entry has printed error */
			return NULL;
		}
    	    	if (size == 0) {
		    skipped++;
		    continue;
		}

		/* copy ICONDIRENTRY (not including last dwImageOffset) */
		memcpy(&fileicondir->entries[c-skipped], &icondir->entries[c],
			sizeof(Win32CursorIconFileDirEntry)-sizeof(uint32));

		/* special treatment for cursors */
		if (!is_icon) {
			fileicondir->entries[c-skipped].width = icondir->entries[c].res_info.cursor.width;
			fileicondir->entries[c-skipped].height = icondir->entries[c].res_info.cursor.height / 2;
			fileicondir->entries[c-skipped].color_count = 0;
			fileicondir->entries[c-skipped].reserved = 0;
		}

		/* set image offset and increase it */
		fileicondir->entries[c-skipped].dib_offset = offset;

		/* transfer resource into file memory */
		if (is_icon) {
			memcpy(&memory[offset], data, icondir->entries[c].bytes_in_res);
		} else {
			fileicondir->entries[c-skipped].hotspot_x = ((uint16 *) data)[0];
			fileicondir->entries[c-skipped].hotspot_y = ((uint16 *) data)[1];
			memcpy(&memory[offset], data+sizeof(uint16)*2,
				   icondir->entries[c].bytes_in_res-sizeof(uint16)*2);
			offset -= sizeof(uint16)*2;
		}

		/* increase the offset pointer */
		offset += icondir->entries[c].bytes_in_res;
	}

	return memory;
}

/* check_offset:
 *   Check if a chunk of data (determined by offset and size)
 *   is within the bounds of the WinLibrary file.
 *   Usually not called directly.
 */
bool Win32ResExtractor::check_offset(byte *memory, int total_size, const char *name, void *offset, int size) {
	int need_size = (int)((byte *)offset - memory + size);

	debugC(DEBUG_RESOURCE, "check_offset: size=%x vs %x offset=%x size=%x",
		need_size, total_size, (byte *)offset - memory, size);

	if (need_size < 0 || need_size > total_size) {
		warning("%s: premature end", name);
		return false;
	}

	return true;
}


/* do_resources:
 *   Do something for each resource matching type, name and lang.
 */
int Win32ResExtractor::do_resources(WinLibrary *fi, const char *type, char *name, char *lang, int action, byte **data) {
	WinResource *type_wr;
	WinResource *name_wr;
	WinResource *lang_wr;
	int size;

	type_wr = (WinResource *)calloc(sizeof(WinResource)*3, 1);
	name_wr = type_wr + 1;
	lang_wr = type_wr + 2;

	size = do_resources_recurs(fi, NULL, type_wr, name_wr, lang_wr, type, name, lang, action, data);

	free(type_wr);

	return size;
}

/* what is each entry in this directory level for? type, name or language? */
#define WINRESOURCE_BY_LEVEL(x) ((x)==0 ? type_wr : ((x)==1 ? name_wr : lang_wr))

/* does the id of this entry match the specified id? */
#define LEVEL_MATCHES(x) (x == NULL || x ## _wr->id[0] == '\0' || compare_resource_id(x ## _wr, x))

int Win32ResExtractor::do_resources_recurs(WinLibrary *fi, WinResource *base, 
		  WinResource *type_wr, WinResource *name_wr, WinResource *lang_wr,
		  const char *type, char *name, char *lang, int action, byte **data) {
	int c, rescnt;
	WinResource *wr;
	uint32 size = 0;

	/* get a list of all resources at this level */
	wr = list_resources(fi, base, &rescnt);
	if (wr == NULL)
		if (size != 0)
			return size;
		else
			return 0;

	/* process each resource listed */
	for (c = 0 ; c < rescnt ; c++) {
		/* (over)write the corresponding WinResource holder with the current */
		memcpy(WINRESOURCE_BY_LEVEL(wr[c].level), wr+c, sizeof(WinResource));

		/* go deeper unless there is something that does NOT match */
		if (LEVEL_MATCHES(type) && LEVEL_MATCHES(name) && LEVEL_MATCHES(lang)) {
			if (wr->is_directory)
				size = do_resources_recurs(fi, wr+c, type_wr, name_wr, lang_wr, type, name, lang, action, data);
			else
				size = extract_resources(fi, wr+c, type_wr, name_wr, lang_wr, data);
		}
	}

	/* since we're moving back one level after this, unset the
	 * WinResource holder used on this level */
	memset(WINRESOURCE_BY_LEVEL(wr[0].level), 0, sizeof(WinResource));

	return size;
}

/* return the resource id quoted if it's a string, otherwise just return it */
char *Win32ResExtractor::get_resource_id_quoted(WinResource *wr) {
	static char tmp[WINRES_ID_MAXLEN+2];

	if (wr->numeric_id || wr->id[0] == '\0')
		return wr->id;

	sprintf(tmp, "'%s'", wr->id);
	return tmp;
}

bool Win32ResExtractor::compare_resource_id(WinResource *wr, const char *id) {
	if (wr->numeric_id) {
		int32 cmp1, cmp2;
		if (id[0] == '+')
			return false;
		if (id[0] == '-')
			id++;
		if (!(cmp1 = strtol(wr->id, 0, 10)) || !(cmp2 = strtol(id, 0, 10)) || cmp1 != cmp2)
			return false;
	} else {
		if (id[0] == '-')
			return false;
		if (id[0] == '+')
			id++;
		if (strcmp(wr->id, id))
			return false;
	}

	return true;
}

bool Win32ResExtractor::decode_pe_resource_id(WinLibrary *fi, WinResource *wr, uint32 value) {
	if (value & IMAGE_RESOURCE_NAME_IS_STRING) {	/* numeric id */
		int c, len;
		uint16 *mem = (uint16 *)
		  (fi->first_resource + (value & ~IMAGE_RESOURCE_NAME_IS_STRING));

		/* copy each char of the string, and terminate it */
		RETURN_IF_BAD_POINTER(false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(false, &mem[1], sizeof(uint16) * len);

		len = MIN(mem[0], (uint16)WINRES_ID_MAXLEN);
		for (c = 0 ; c < len ; c++)
			wr->id[c] = mem[c+1] & 0x00FF;
		wr->id[len] = '\0';
	} else {					/* Unicode string id */
		/* translate id into a string */
		snprintf(wr->id, WINRES_ID_MAXLEN, "%d", value);
	}

	wr->numeric_id = (value & IMAGE_RESOURCE_NAME_IS_STRING ? false:true);
	return true;
}
 
byte *Win32ResExtractor::get_resource_entry(WinLibrary *fi, WinResource *wr, int *size) {
	if (fi->is_PE_binary) {
		Win32ImageResourceDataEntry *dataent;

		dataent = (Win32ImageResourceDataEntry *) wr->children;
		RETURN_IF_BAD_POINTER(NULL, *dataent);
		*size = dataent->size;
		RETURN_IF_BAD_OFFSET(NULL, fi->memory + dataent->offset_to_data, *size);

		return fi->memory + dataent->offset_to_data;
	} else {
		Win16NENameInfo *nameinfo;
		int sizeshift;

		nameinfo = (Win16NENameInfo *) wr->children;
		sizeshift = *((uint16 *) fi->first_resource - 1);
		*size = nameinfo->length << sizeshift;
		RETURN_IF_BAD_OFFSET(NULL, fi->memory + (nameinfo->offset << sizeshift), *size);

		return fi->memory + (nameinfo->offset << sizeshift);
	}
}

bool Win32ResExtractor::decode_ne_resource_id(WinLibrary *fi, WinResource *wr, uint16 value) {
	if (value & NE_RESOURCE_NAME_IS_NUMERIC) {		/* numeric id */
		/* translate id into a string */
		snprintf(wr->id, WINRES_ID_MAXLEN, "%d", value & ~NE_RESOURCE_NAME_IS_NUMERIC);
	} else {					/* ASCII string id */
		int len;
		char *mem = (char *)NE_HEADER(fi->memory)
		                     + NE_HEADER(fi->memory)->rsrctab
		                     + value;

		/* copy each char of the string, and terminate it */
		RETURN_IF_BAD_POINTER(false, *mem);
		len = mem[0];
		RETURN_IF_BAD_OFFSET(false, &mem[1], sizeof(char) * len);
		memcpy(wr->id, &mem[1], len);
		wr->id[len] = '\0';
	}

	wr->numeric_id = (value & NE_RESOURCE_NAME_IS_NUMERIC ? true:false);
	return true;
}

Win32ResExtractor::WinResource *Win32ResExtractor::list_pe_resources(WinLibrary *fi, Win32ImageResourceDirectory *pe_res, int level, int *count) {
	WinResource *wr;
	int c, rescnt;
	Win32ImageResourceDirectoryEntry *dirent
	  = (Win32ImageResourceDirectoryEntry *)(pe_res + 1);

	/* count number of `type' resources */
	RETURN_IF_BAD_POINTER(NULL, *dirent);
	rescnt = pe_res->number_of_named_entries + pe_res->number_of_id_entries;
	*count = rescnt;

	/* allocate WinResource's */
	wr = (WinResource *)malloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RETURN_IF_BAD_POINTER(NULL, dirent[c]);
		wr[c].this_ = pe_res;
		wr[c].level = level;
		wr[c].is_directory = (dirent[c].u2.s.data_is_directory);
		wr[c].children = fi->first_resource + dirent[c].u2.s.offset_to_directory;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_pe_resource_id (fi, wr + c, dirent[c].u1.name))
			return NULL;
	}

	return wr;
}

Win32ResExtractor::WinResource *Win32ResExtractor::list_ne_name_resources(WinLibrary *fi, WinResource *typeres, int *count) {
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo = (Win16NETypeInfo *) typeres->this_;
	Win16NENameInfo *nameinfo = (Win16NENameInfo *) typeres->children;

	/* count number of `type' resources */
	RETURN_IF_BAD_POINTER(NULL, typeinfo->count);
	*count = rescnt = typeinfo->count;

	/* allocate WinResource's */
	wr = (WinResource *)malloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RETURN_IF_BAD_POINTER(NULL, nameinfo[c]);
		wr[c].this_ = nameinfo+c;
		wr[c].is_directory = false;
		wr[c].children = nameinfo+c;
		wr[c].level = 1;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_ne_resource_id(fi, wr + c, (nameinfo+c)->id))
			return NULL;
	}

	return wr;
}

Win32ResExtractor::WinResource *Win32ResExtractor::list_ne_type_resources(WinLibrary *fi, int *count) {
	int c, rescnt;
	WinResource *wr;
	Win16NETypeInfo *typeinfo;

	/* count number of `type' resources */
	typeinfo = (Win16NETypeInfo *) fi->first_resource;
	RETURN_IF_BAD_POINTER(NULL, *typeinfo);
	for (rescnt = 0 ; typeinfo->type_id != 0 ; rescnt++) {
		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
		RETURN_IF_BAD_POINTER(NULL, *typeinfo);
	}
	*count = rescnt;

	/* allocate WinResource's */
	wr = (WinResource *)malloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	typeinfo = (Win16NETypeInfo *) fi->first_resource;
	for (c = 0 ; c < rescnt ; c++) {
		wr[c].this_ = typeinfo;
		wr[c].is_directory = (typeinfo->count != 0);
		wr[c].children = typeinfo+1;
		wr[c].level = 0;

		/* fill in wr->id, wr->numeric_id */
		if (!decode_ne_resource_id(fi, wr + c, typeinfo->type_id))
			return NULL;

		typeinfo = NE_TYPEINFO_NEXT(typeinfo);
	}

	return wr;
}

/* list_resources:
 *   Return an array of WinResource's in the current
 *   resource level specified by res.
 */
Win32ResExtractor::WinResource *Win32ResExtractor::list_resources(WinLibrary *fi, WinResource *res, int *count) {
	if (res != NULL && !res->is_directory)
		return NULL;

	if (fi->is_PE_binary) {
		return list_pe_resources(fi, (Win32ImageResourceDirectory *)
				 (res == NULL ? fi->first_resource : res->children),
				 (res == NULL ? 0 : res->level+1),
				 count);
	} else {
		return (res == NULL
				? list_ne_type_resources(fi, count)
				: list_ne_name_resources(fi, res, count));
	}
}

/* read_library:
 *   Read header and get resource directory offset in a Windows library
 *    (AKA module).
 *
 */
bool Win32ResExtractor::read_library(WinLibrary *fi) {
	/* check for DOS header signature `MZ' */
	RETURN_IF_BAD_POINTER(false, MZ_HEADER(fi->memory)->magic);
	if (MZ_HEADER(fi->memory)->magic == IMAGE_DOS_SIGNATURE) {
		DOSImageHeader *mz_header = MZ_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(false, mz_header->lfanew);
		if (mz_header->lfanew < sizeof(DOSImageHeader)) {
			warning("%s: not a Windows library", fi->file->name());
			return false;
		}
	}

	/* check for OS2 (Win16) header signature `NE' */
	RETURN_IF_BAD_POINTER(false, NE_HEADER(fi->memory)->magic);
	if (NE_HEADER(fi->memory)->magic == IMAGE_OS2_SIGNATURE) {
		OS2ImageHeader *header = NE_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(false, header->rsrctab);
		RETURN_IF_BAD_POINTER(false, header->restab);
		if (header->rsrctab >= header->restab) {
			warning("%s: no resource directory found", fi->file->name());
			return false;
		}

		fi->is_PE_binary = false;
		fi->first_resource = (byte *) NE_HEADER(fi->memory)
		  + header->rsrctab + sizeof(uint16);
		RETURN_IF_BAD_POINTER(false, *(Win16NETypeInfo *) fi->first_resource);

		return true;
	}

	/* check for NT header signature `PE' */
	RETURN_IF_BAD_POINTER(false, PE_HEADER(fi->memory)->signature);
	if (PE_HEADER(fi->memory)->signature == IMAGE_NT_SIGNATURE) {
		Win32ImageSectionHeader *pe_sec;
		Win32ImageDataDirectory *dir;
		Win32ImageNTHeaders *pe_header;
		int d;

		/* allocate new memory */
		fi->total_size = calc_vma_size(fi);
		if (fi->total_size == 0) {
			/* calc_vma_size has reported error */
			return false;
		}
		fi->memory = (byte *)realloc(fi->memory, fi->total_size);

		/* relocate memory, start from last section */
		pe_header = PE_HEADER(fi->memory);
		RETURN_IF_BAD_POINTER(false, pe_header->file_header.number_of_sections);

		/* we don't need to do OFFSET checking for the sections.
		 * calc_vma_size has already done that */
		for (d = pe_header->file_header.number_of_sections - 1; d >= 0 ; d--) {
			pe_sec = PE_SECTIONS(fi->memory) + d;

	            	if (pe_sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
			    continue;

    	    	    	//if (pe_sec->virtual_address + pe_sec->size_of_raw_data > fi->total_size)

    	    	    	RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->virtual_address, pe_sec->size_of_raw_data);
    	    	    	RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->pointer_to_raw_data, pe_sec->size_of_raw_data);
    	    	    	if (pe_sec->virtual_address != pe_sec->pointer_to_raw_data) {
    	    	    	    memmove(fi->memory + pe_sec->virtual_address,
				    fi->memory + pe_sec->pointer_to_raw_data,
				    pe_sec->size_of_raw_data);
			}
		}

		/* find resource directory */
		RETURN_IF_BAD_POINTER(false, pe_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
		dir = pe_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
		if (dir->size == 0) {
			warning("%s: file contains no resources", fi->file->name());
			return false;
		}

		fi->first_resource = fi->memory + dir->virtual_address;
		fi->is_PE_binary = true;
		return true;
	}

	/* other (unknown) header signature was found */
	warning("%s: not a Windows library", fi->file->name());
	return false;
}

/* calc_vma_size:
 *   Calculate the total amount of memory needed for a 32-bit Windows
 *   module. Returns -1 if file was too small.
 */
int Win32ResExtractor::calc_vma_size(WinLibrary *fi) {
    Win32ImageSectionHeader *seg;
    int c, segcount, size;

	size = 0;
	RETURN_IF_BAD_POINTER(-1, PE_HEADER(fi->memory)->file_header.number_of_sections);
	segcount = PE_HEADER(fi->memory)->file_header.number_of_sections;

	/* If there are no segments, just process file like it is.
	 * This is (probably) not the right thing to do, but problems
	 * will be delt with later anyway.
	 */
	if (segcount == 0)
		return fi->total_size;

	seg = PE_SECTIONS(fi->memory);
	RETURN_IF_BAD_POINTER(-1, *seg);
    for (c = 0 ; c < segcount ; c++) {
		RETURN_IF_BAD_POINTER(0, *seg);

        size = MAX((uint32)size, seg->virtual_address + seg->size_of_raw_data);
		/* I have no idea what misc.virtual_size is for... */
        size = MAX((uint32)size, seg->virtual_address + seg->misc.virtual_size);
        seg++;
    }

    return size;
}

Win32ResExtractor::WinResource *Win32ResExtractor::find_with_resource_array(WinLibrary *fi, WinResource *wr, const char *id) {
	int c, rescnt;
	WinResource *return_wr;

	wr = list_resources(fi, wr, &rescnt);
	if (wr == NULL)
		return NULL;

	for (c = 0 ; c < rescnt ; c++) {
		if (compare_resource_id(&wr[c], id)) {
			/* duplicate WinResource and return it */
			return_wr = (WinResource *)malloc(sizeof(WinResource));
			memcpy(return_wr, &wr[c], sizeof(WinResource));

			/* free old WinResource */
			free(wr);
			return return_wr;
		}
	}

	return NULL;
}

Win32ResExtractor::WinResource *Win32ResExtractor::find_resource(WinLibrary *fi, const char *type, const char *name, const char *language, int *level) {
	WinResource *wr;

	*level = 0;
	if (type == NULL)
		return NULL;
	wr = find_with_resource_array(fi, NULL, type);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 1;
	if (name == NULL)
		return wr;
	wr = find_with_resource_array(fi, wr, name);
	if (wr == NULL || !wr->is_directory)
		return wr;

	*level = 2;
	if (language == NULL)
		return wr;
	wr = find_with_resource_array(fi, wr, language);
	return wr;
}

#define ROW_BYTES(bits) ((((bits) + 31) >> 5) << 2)


int Win32ResExtractor::convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
							int *hotspot_x, int *hotspot_y, int *keycolor) {
	Win32CursorIconFileDir dir;
	Win32CursorIconFileDirEntry *entries = NULL;
	uint32 offset;
	uint32 c, d;
	int completed;
	int matched = 0;
	MemoryReadStream *in = new MemoryReadStream(data, datasize);

	if (!in->read(&dir, sizeof(Win32CursorIconFileDir)- sizeof(Win32CursorIconFileDirEntry)))
		goto cleanup;
	fix_win32_cursor_icon_file_dir_endian(&dir);

	if (dir.reserved != 0) {
		warning("not an icon or cursor file (reserved non-zero)");
		goto cleanup;
	}
	if (dir.type != 1 && dir.type != 2) {
		warning("not an icon or cursor file (wrong type)");
		goto cleanup;
	}

	entries = (Win32CursorIconFileDirEntry *)malloc(dir.count * sizeof(Win32CursorIconFileDirEntry));
	for (c = 0; c < dir.count; c++) {
		if (!in->read(&entries[c], sizeof(Win32CursorIconFileDirEntry)))
			goto cleanup;
		fix_win32_cursor_icon_file_dir_entry_endian(&entries[c]);
		if (entries[c].reserved != 0)
			warning("reserved is not zero");
	}

	offset = sizeof(Win32CursorIconFileDir) + (dir.count - 1) * (sizeof(Win32CursorIconFileDirEntry));

	for (completed = 0; completed < dir.count; ) {
		uint32 min_offset = 0x7fffffff;
		int previous = completed;

		for (c = 0; c < dir.count; c++) {
			if (entries[c].dib_offset == offset) {
				Win32BitmapInfoHeader bitmap;
				Win32RGBQuad *palette = NULL;
				uint32 palette_count = 0;
				uint32 image_size, mask_size;
				uint32 width, height;
				byte *image_data = NULL, *mask_data = NULL;
				byte *row = NULL;

				if (!in->read(&bitmap, sizeof(Win32BitmapInfoHeader)))
					goto local_cleanup;

				fix_win32_bitmap_info_header_endian(&bitmap);
				if (bitmap.size < sizeof(Win32BitmapInfoHeader)) {
					warning("bitmap header is too short");
					goto local_cleanup;
				}
				if (bitmap.compression != 0) {
					warning("compressed image data not supported");
					goto local_cleanup;
				}
				if (bitmap.x_pels_per_meter != 0)
					warning("x_pels_per_meter field in bitmap should be zero");
				if (bitmap.y_pels_per_meter != 0)
					warning("y_pels_per_meter field in bitmap should be zero");
				if (bitmap.clr_important != 0)
					warning("clr_important field in bitmap should be zero");
				if (bitmap.planes != 1)
					warning("planes field in bitmap should be one");
				if (bitmap.size != sizeof(Win32BitmapInfoHeader)) {
					uint32 skip = bitmap.size - sizeof(Win32BitmapInfoHeader);
					warning("skipping %d bytes of extended bitmap header", skip);
					in->seek(skip, SEEK_CUR);
				}
				offset += bitmap.size;

				if (bitmap.clr_used != 0 || bitmap.bit_count < 24) {
					palette_count = (bitmap.clr_used != 0 ? bitmap.clr_used : 1 << bitmap.bit_count);
					palette = (Win32RGBQuad *)malloc(sizeof(Win32RGBQuad) * palette_count);
					if (!in->read(palette, sizeof(Win32RGBQuad) * palette_count))
						goto local_cleanup;
					offset += sizeof(Win32RGBQuad) * palette_count;
				}

				width = bitmap.width;
				height = ABS(bitmap.height)/2;
				
				image_size = height * ROW_BYTES(width * bitmap.bit_count);
				mask_size = height * ROW_BYTES(width);

				if (entries[c].dib_size	!= bitmap.size + image_size + mask_size + palette_count * sizeof(Win32RGBQuad))
					debugC(DEBUG_RESOURCE, "incorrect total size of bitmap (%d specified; %d real)",
					    entries[c].dib_size,
					    bitmap.size + image_size + mask_size + palette_count * sizeof(Win32RGBQuad)
					);

				image_data = (byte *)malloc(image_size);
				if (!in->read(image_data, image_size))
					goto local_cleanup;

				mask_data = (byte *)malloc(mask_size);
				if (!in->read(mask_data, mask_size))
					goto local_cleanup;

				offset += image_size;
				offset += mask_size;
				completed++;
				matched++;

				*hotspot_x = entries[c].hotspot_x;
				*hotspot_y = entries[c].hotspot_y;
				*w = width;
				*h = height;
				*keycolor = 0;
				*cursor = (byte *)malloc(width * height);

				row = (byte *)malloc(width * 4);

				for (d = 0; d < height; d++) {
					uint32 x;
					uint32 y = (bitmap.height < 0 ? d : height - d - 1);
					uint32 imod = y * (image_size / height) * 8 / bitmap.bit_count;
					//uint32 mmod = y * (mask_size / height) * 8;

					for (x = 0; x < width; x++) {

						uint32 color = simple_vec(image_data, x + imod, bitmap.bit_count);

						// FIXME?: This works only with b/w cursors and white index may be
						// different. But now it's enough.
						if (color) {
							cursor[0][width * d + x] = 15; // white in SCUMM
						} else {
							cursor[0][width * d + x] = 255; // transparent
						}
						/*

						if (bitmap.bit_count <= 16) {
							if (color >= palette_count) {
								warning("color out of range in image data");
								goto local_cleanup;
							}
							row[4*x+0] = palette[color].red;
							row[4*x+1] = palette[color].green;
							row[4*x+2] = palette[color].blue;

						} else {
							row[4*x+0] = (color >> 16) & 0xFF;
							row[4*x+1] = (color >>  8) & 0xFF;
							row[4*x+2] = (color >>  0) & 0xFF;
						}
						if (bitmap.bit_count == 32)
						    row[4*x+3] = (color >> 24) & 0xFF;
						else
						    row[4*x+3] = simple_vec(mask_data, x + mmod, 1) ? 0 : 0xFF;
						*/
					}

				}

				if (row != NULL)
					free(row);
				if (palette != NULL)
					free(palette);
				if (image_data != NULL) {
					free(image_data);
					free(mask_data);
				}
				continue;

			local_cleanup:

				if (row != NULL)
					free(row);
				if (palette != NULL)
					free(palette);
				if (image_data != NULL) {
					free(image_data);
					free(mask_data);
				}
				goto cleanup;
			} else {
				if (entries[c].dib_offset > offset)
						min_offset = MIN(min_offset, entries[c].dib_offset);
			}
		}

		if (previous == completed) {
			if (min_offset < offset) {
				warning("offset of bitmap header incorrect (too low)");
				goto cleanup;
			}
			debugC(DEBUG_RESOURCE, "skipping %d bytes of garbage at %d", min_offset-offset, offset);
			in->seek(min_offset - offset, SEEK_CUR);
			offset = min_offset;
		}
	}

	free(entries);
	return matched;

cleanup:

	free(entries);
	return -1;
}

uint32 Win32ResExtractor::simple_vec(byte *data, uint32 ofs, byte size) {
	switch (size) {
	case 1:
		return (data[ofs/8] >> (7 - ofs%8)) & 1;
	case 2:
		return (data[ofs/4] >> ((3 - ofs%4) << 1)) & 3;
	case 4:
		return (data[ofs/2] >> ((1 - ofs%2) << 2)) & 15;
	case 8:
		return data[ofs];
	case 16:
		return data[2*ofs] | data[2*ofs+1] << 8;
	case 24:
		return data[3*ofs] | data[3*ofs+1] << 8 | data[3*ofs+2] << 16;
	case 32:
		return data[4*ofs] | data[4*ofs+1] << 8 | data[4*ofs+2] << 16 | data[4*ofs+3] << 24;
	}

	return 0;
}

#define LE16(x)      ((x) = TO_LE_16(x))
#define LE32(x)      ((x) = TO_LE_32(x))

void Win32ResExtractor::fix_win32_cursor_icon_file_dir_endian(Win32CursorIconFileDir *obj) {
    LE16(obj->reserved);
	LE16(obj->type);
    LE16(obj->count);
}

void Win32ResExtractor::fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj) {
    LE32(obj->size);
    LE32(obj->width);
    LE32(obj->height);
    LE16(obj->planes);
    LE16(obj->bit_count);
    LE32(obj->compression);
    LE32(obj->size_image);
    LE32(obj->x_pels_per_meter);
    LE32(obj->y_pels_per_meter);
    LE32(obj->clr_used);
    LE32(obj->clr_important);
}

void Win32ResExtractor::fix_win32_cursor_icon_file_dir_entry_endian(Win32CursorIconFileDirEntry *obj) {
    LE16(obj->hotspot_x);
    LE16(obj->hotspot_y);
    LE32(obj->dib_size);
    LE32(obj->dib_offset);
}

void Win32ResExtractor::fix_win32_image_section_header(Win32ImageSectionHeader *obj) {
    LE32(obj->misc.physical_address);
    LE32(obj->virtual_address);
    LE32(obj->size_of_raw_data);
    LE32(obj->pointer_to_raw_data);
    LE32(obj->pointer_to_relocations);
    LE32(obj->pointer_to_linenumbers);
    LE16(obj->number_of_relocations);
    LE16(obj->number_of_linenumbers);
    LE32(obj->characteristics);
}

void Win32ResExtractor::fix_os2_image_header_endian(OS2ImageHeader *obj) {
    LE16(obj->magic);
    LE16(obj->enttab);
    LE16(obj->cbenttab);
    LE32(obj->crc);
    LE16(obj->flags);
    LE16(obj->autodata);
    LE16(obj->heap);
    LE16(obj->stack);
    LE32(obj->csip);
    LE32(obj->sssp);
    LE16(obj->cseg);
    LE16(obj->cmod);
    LE16(obj->cbnrestab);
    LE16(obj->segtab);
    LE16(obj->rsrctab);
    LE16(obj->restab);
    LE16(obj->modtab);
    LE16(obj->imptab);
    LE32(obj->nrestab);
    LE16(obj->cmovent);
    LE16(obj->align);
    LE16(obj->cres);
    LE16(obj->fastload_offset);
    LE16(obj->fastload_length);
    LE16(obj->swaparea);
    LE16(obj->expver);
}

/* fix_win32_image_header_endian:
 * NOTE: This assumes that the optional header is always available.
 */
void Win32ResExtractor::fix_win32_image_header_endian(Win32ImageNTHeaders *obj) {
    LE32(obj->signature);
    LE16(obj->file_header.machine);
    LE16(obj->file_header.number_of_sections);
    LE32(obj->file_header.time_date_stamp);
    LE32(obj->file_header.pointer_to_symbol_table);
    LE32(obj->file_header.number_of_symbols);
    LE16(obj->file_header.size_of_optional_header);
    LE16(obj->file_header.characteristics);
    LE16(obj->optional_header.magic);
    LE32(obj->optional_header.size_of_code);
    LE32(obj->optional_header.size_of_initialized_data);
    LE32(obj->optional_header.size_of_uninitialized_data);
    LE32(obj->optional_header.address_of_entry_point);
    LE32(obj->optional_header.base_of_code);
    LE32(obj->optional_header.base_of_data);
    LE32(obj->optional_header.image_base);
    LE32(obj->optional_header.section_alignment);
    LE32(obj->optional_header.file_alignment);
    LE16(obj->optional_header.major_operating_system_version);
    LE16(obj->optional_header.minor_operating_system_version);
    LE16(obj->optional_header.major_image_version);
    LE16(obj->optional_header.minor_image_version);
    LE16(obj->optional_header.major_subsystem_version);
    LE16(obj->optional_header.minor_subsystem_version);
    LE32(obj->optional_header.win32_version_value);
    LE32(obj->optional_header.size_of_image);
    LE32(obj->optional_header.size_of_headers);
    LE32(obj->optional_header.checksum);
    LE16(obj->optional_header.subsystem);
    LE16(obj->optional_header.dll_characteristics);
    LE32(obj->optional_header.size_of_stack_reserve);
    LE32(obj->optional_header.size_of_stack_commit);
    LE32(obj->optional_header.size_of_heap_reserve);
    LE32(obj->optional_header.size_of_heap_commit);
    LE32(obj->optional_header.loader_flags);
    LE32(obj->optional_header.number_of_rva_and_sizes);
}

void Win32ResExtractor::fix_win32_image_data_directory(Win32ImageDataDirectory *obj) {
    LE32(obj->virtual_address);
    LE32(obj->size);
}


} // End of namespace Scumm
