/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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


#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/he/resource_he.h"
#include "scumm/he/sound_he.h"

#include "sound/wave.h"
#include "graphics/cursorman.h"

#include "common/archive.h"
#include "common/stream.h"
#include "common/system.h"

namespace Scumm {

#define LE16(x)      ((x) = TO_LE_16(x))
#define LE32(x)      ((x) = TO_LE_32(x))


ResExtractor::ResExtractor(ScummEngine_v70he *scumm)
	: _vm(scumm) {

	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::~ResExtractor() {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (cc->valid) {
			free(cc->bitmap);
			free(cc->palette);
		}
	}
	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::CachedCursor *ResExtractor::findCachedCursor(int id) {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (cc->valid && cc->id == id) {
			return cc;
		}
	}
	return NULL;
}

ResExtractor::CachedCursor *ResExtractor::getCachedCursorSlot() {
	uint32 min_last_used = 0;
	CachedCursor *r = NULL;
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (!cc->valid) {
			return cc;
		} else {
			if (min_last_used == 0 || cc->last_used < min_last_used) {
				min_last_used = cc->last_used;
				r = cc;
			}
		}
	}
	assert(r);
	free(r->bitmap);
	free(r->palette);
	memset(r, 0, sizeof(CachedCursor));
	return r;
}

void ResExtractor::setCursor(int id) {
	byte *cursorRes = 0;
	int cursorsize;
	int keycolor = 0;
	CachedCursor *cc = findCachedCursor(id);
	if (cc != NULL) {
		debug(7, "Found cursor %d in cache slot %lu", id, (long)(cc - _cursorCache));
	} else {
		cc = getCachedCursorSlot();
		assert(cc && !cc->valid);
		cursorsize = extractResource(id, &cursorRes);
		convertIcons(cursorRes, cursorsize, &cc->bitmap, &cc->w, &cc->h, &cc->hotspot_x, &cc->hotspot_y, &keycolor, &cc->palette, &cc->palSize);
		debug(7, "Adding cursor %d to cache slot %lu", id, (long)(cc - _cursorCache));
		free(cursorRes);
		cc->valid = true;
		cc->id = id;
		cc->last_used = g_system->getMillis();
	}

	if (cc->palette)
		CursorMan.replaceCursorPalette(cc->palette, 0, cc->palSize);

	_vm->setCursorHotspot(cc->hotspot_x, cc->hotspot_y);
	_vm->setCursorFromBuffer(cc->bitmap, cc->w, cc->h, cc->w);
}


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

Win32ResExtractor::Win32ResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
}

int Win32ResExtractor::extractResource(int resId, byte **data) {
	char buf[20];

	snprintf(buf, sizeof(buf), "%d", resId);

	return extractResource_("group_cursor", buf, data);
}

int Win32ResExtractor::extractResource_(const char *resType, char *resName, byte **data) {
	char *arg_language = NULL;
	const char *arg_type = resType;
	char *arg_name = resName;
	int ressize = 0;

	_arg_raw = false;

	/* translate --type option from resource type string to integer */
	arg_type = res_type_string_to_id(arg_type);

	WinLibrary fi;

	/* initiate stuff */
	fi.memory = NULL;
	fi.file = NULL;

	if (_fileName.empty()) { // We are running for the first time
		_fileName = _vm->generateFilename(-3);
	}

	/* get file size */
	fi.file = SearchMan.createReadStreamForMember(_fileName);
	if (!fi.file) {
		error("Cannot open file %s", _fileName.c_str());
	}

	fi.total_size = fi.file->size();
	if (fi.total_size == -1) {
		error("Cannot get size of file %s", _fileName.c_str());
		goto cleanup;
	}
	if (fi.total_size == 0) {
		error("%s: file has a size of 0", _fileName.c_str());
		goto cleanup;
	}

	/* read all of file */
	fi.memory = (byte *)malloc(fi.total_size);
	if (fi.file->read(fi.memory, fi.total_size) == 0) {
		error("Cannot read from file %s", _fileName.c_str());
		goto cleanup;
	}

	/* identify file and find resource table */
	if (!read_library(&fi)) {
		/* error reported by read_library */
		goto cleanup;
	}

	/* errors will be printed by the callback */
	ressize = do_resources(&fi, arg_type, arg_name, arg_language, data);

	/* free stuff and close file */
	cleanup:
	delete fi.file;
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

/* return the resource id quoted if it's a string, otherwise just return it */
char *Win32ResExtractor::WinResource::get_resource_id_quoted() {
	// FIXME: Using a static var here is EVIL and in fact, broken when
	// used multiple times in a row, e.g. in a single call to printf()
	// or debug()... which is in fact how we use this function... :-)
	static char tmp[WINRES_ID_MAXLEN+2];

	if (numeric_id || id[0] == '\0')
		return id;

	sprintf(tmp, "'%s'", id);
	return tmp;
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
		error("Win32ResExtractor::extract_resources() problem with resource extraction");
		return 0;
	}

	/* get named resource type if possible */
	type = NULL;
	if ((id = strtol(type_wr->id, 0, 10)) != 0)
		type = res_type_id_to_string(id);

	debugC(DEBUG_RESOURCE, "extractCursor(). Found cursor name: %s%s%s [size=%d]",
	  name_wr->get_resource_id_quoted(),
	  (lang_wr->id[0] != '\0' ? " language: " : ""),
	  lang_wr->get_resource_id_quoted(), size);

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
	for (c = 0 ; c < FROM_LE_16(icondir->count) ; c++) {
		int level;
		int iconsize;
		char name[14];
		WinResource *fwr;

		RETURN_IF_BAD_POINTER(NULL, icondir->entries[c]);
		/*printf("%d. bytes_in_res=%d width=%d height=%d planes=%d bit_count=%d\n", c,
			FROM_LE_32(icondir->entries[c].bytes_in_res),
			(is_icon ? icondir->entries[c].res_info.icon.width : FROM_LE_16(icondir->entries[c].res_info.cursor.width)),
			(is_icon ? icondir->entries[c].res_info.icon.height : FROM_LE_16(icondir->entries[c].res_info.cursor.height)),
			FROM_LE_16(icondir->entries[c].plane_count),
			FROM_LE_16(icondir->entries[c].bit_count));*/

		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", FROM_LE_16(icondir->entries[c].res_id));
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, lang, &level);
		if (fwr == NULL) {
			error("%s: could not find `%s' in `%s' resource.",
					_fileName.c_str(), &name[1], (is_icon ? "group_icon" : "group_cursor"));
			return NULL;
		}

		if (get_resource_entry(fi, fwr, &iconsize) != NULL) {
		    if (iconsize == 0) {
				debugC(DEBUG_RESOURCE, "%s: icon resource `%s' is empty, skipping", _fileName.c_str(), name);
				skipped++;
				continue;
		    }
		    if ((uint32)iconsize != FROM_LE_32(icondir->entries[c].bytes_in_res)) {
				debugC(DEBUG_RESOURCE, "%s: mismatch of size in icon resource `%s' and group (%d != %d)",
					_fileName.c_str(), name, iconsize, FROM_LE_32(icondir->entries[c].bytes_in_res));
		    }
		    size += iconsize; /* size += FROM_LE_32(icondir->entries[c].bytes_in_res); */

		    /* cursor resources have two additional WORDs that contain
		     * hotspot info */
		    if (!is_icon)
			size -= sizeof(uint16)*2;
		}
	}
	offset = sizeof(Win32CursorIconFileDir) + (FROM_LE_16(icondir->count)-skipped) * sizeof(Win32CursorIconFileDirEntry);
	size += offset;
	*ressize = size;

	/* allocate that much memory */
	memory = (byte *)malloc(size);
	fileicondir = (Win32CursorIconFileDir *)memory;

	/* transfer Win32CursorIconDir structure members */
	fileicondir->reserved = icondir->reserved;
	fileicondir->type = icondir->type;
	fileicondir->count = TO_LE_16(FROM_LE_16(icondir->count) - skipped);

	/* transfer each cursor/icon: Win32CursorIconDirEntry and data */
	skipped = 0;
	for (c = 0 ; c < FROM_LE_16(icondir->count) ; c++) {
		int level;
		char name[14];
		WinResource *fwr;
		byte *data;

		/* find the corresponding icon resource */
		snprintf(name, sizeof(name)/sizeof(char), "-%d", FROM_LE_16(icondir->entries[c].res_id));
		fwr = find_resource(fi, (is_icon ? "-3" : "-1"), name, lang, &level);
		if (fwr == NULL) {
			error("%s: could not find `%s' in `%s' resource.",
				_fileName.c_str(), &name[1], (is_icon ? "group_icon" : "group_cursor"));
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
			fileicondir->entries[c-skipped].height = TO_LE_16(FROM_LE_16(icondir->entries[c].res_info.cursor.height) / 2);
			fileicondir->entries[c-skipped].color_count = 0;
			fileicondir->entries[c-skipped].reserved = 0;
		}

		/* set image offset and increase it */
		fileicondir->entries[c-skipped].dib_offset = TO_LE_32(offset);

		/* transfer resource into file memory */
		if (is_icon) {
			memcpy(&memory[offset], data, FROM_LE_32(icondir->entries[c].bytes_in_res));
		} else {
			fileicondir->entries[c-skipped].hotspot_x = ((uint16 *) data)[0];
			fileicondir->entries[c-skipped].hotspot_y = ((uint16 *) data)[1];
			memcpy(&memory[offset], data+sizeof(uint16)*2,
				   FROM_LE_32(icondir->entries[c].bytes_in_res)-sizeof(uint16)*2);
			offset -= sizeof(uint16)*2;
		}

		/* increase the offset pointer */
		offset += FROM_LE_32(icondir->entries[c].bytes_in_res);
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
		need_size, total_size, (uint)((byte *)offset - memory), size);

	if (need_size < 0 || need_size > total_size) {
		error("%s: premature end", name);
		return false;
	}

	return true;
}


/* do_resources:
 *   Do something for each resource matching type, name and lang.
 */
int Win32ResExtractor::do_resources(WinLibrary *fi, const char *type, char *name, char *lang, byte **data) {
	WinResource *type_wr;
	WinResource *name_wr;
	WinResource *lang_wr;
	int size;

	type_wr = (WinResource *)calloc(sizeof(WinResource)*3, 1);
	name_wr = type_wr + 1;
	lang_wr = type_wr + 2;

	size = do_resources_recurs(fi, NULL, type_wr, name_wr, lang_wr, type, name, lang, data);

	free(type_wr);

	return size;
}

/* what is each entry in this directory level for? type, name or language? */
#define WINRESOURCE_BY_LEVEL(x) ((x)==0 ? type_wr : ((x)==1 ? name_wr : lang_wr))

/* does the id of this entry match the specified id? */
#define LEVEL_MATCHES(x) (x == NULL || x ## _wr->id[0] == '\0' || compare_resource_id(x ## _wr, x))

int Win32ResExtractor::do_resources_recurs(WinLibrary *fi, WinResource *base,
		  WinResource *type_wr, WinResource *name_wr, WinResource *lang_wr,
		  const char *type, char *name, char *lang, byte **data) {
	int c, rescnt;
	WinResource *wr;
	uint32 size = 0;

	/* get a list of all resources at this level */
	wr = list_resources(fi, base, &rescnt);
	if (wr == NULL) {
		if (size != 0)
			return size;
		else
			return 0;
	}

	/* process each resource listed */
	for (c = 0 ; c < rescnt ; c++) {
		/* (over)write the corresponding WinResource holder with the current */
		memcpy(WINRESOURCE_BY_LEVEL(wr[c].level), wr+c, sizeof(WinResource));

		/* go deeper unless there is something that does NOT match */
		if (LEVEL_MATCHES(type) && LEVEL_MATCHES(name) && LEVEL_MATCHES(lang)) {
			if (wr->is_directory)
				size = do_resources_recurs(fi, wr+c, type_wr, name_wr, lang_wr, type, name, lang, data);
			else
				size = extract_resources(fi, wr+c, type_wr, name_wr, lang_wr, data);
		}
	}

	/* since we're moving back one level after this, unset the
	 * WinResource holder used on this level */
	memset(WINRESOURCE_BY_LEVEL(wr[0].level), 0, sizeof(WinResource));

	return size;
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
		len = FROM_LE_16(mem[0]);
		RETURN_IF_BAD_OFFSET(false, &mem[1], sizeof(uint16) * len);

		len = MIN(FROM_LE_16(mem[0]), (uint16)WINRES_ID_MAXLEN);
		for (c = 0 ; c < len ; c++)
			wr->id[c] = FROM_LE_16(mem[c+1]) & 0x00FF;
		wr->id[len] = '\0';
		wr->numeric_id = false;
	} else {					/* Unicode string id */
		/* translate id into a string */
		snprintf(wr->id, WINRES_ID_MAXLEN, "%d", value);
		wr->numeric_id = true;
	}

	return true;
}

byte *Win32ResExtractor::get_resource_entry(WinLibrary *fi, WinResource *wr, int *size) {
	byte *result;

	Win32ImageResourceDataEntry *dataent;

	dataent = (Win32ImageResourceDataEntry *) wr->children;
	RETURN_IF_BAD_POINTER(NULL, *dataent);
	*size = FROM_LE_32(dataent->size);

	result = fi->memory + FROM_LE_32(dataent->offset_to_data);

	RETURN_IF_BAD_OFFSET(NULL, result, *size);

	return result;
}

Win32ResExtractor::WinResource *Win32ResExtractor::list_pe_resources(WinLibrary *fi, Win32ImageResourceDirectory *pe_res, int level, int *count) {
	WinResource *wr;
	int c, rescnt;
	Win32ImageResourceDirectoryEntry *dirent
	  = (Win32ImageResourceDirectoryEntry *)(pe_res + 1);

	/* count number of `type' resources */
	RETURN_IF_BAD_POINTER(NULL, *dirent);
	rescnt = FROM_LE_16(pe_res->number_of_named_entries) + FROM_LE_16(pe_res->number_of_id_entries);
	*count = rescnt;

	/* allocate WinResource's */
	wr = (WinResource *)malloc(sizeof(WinResource) * rescnt);

	/* fill in the WinResource's */
	for (c = 0 ; c < rescnt ; c++) {
		RETURN_IF_BAD_POINTER(NULL, dirent[c]);
		wr[c].this_ = pe_res;
		wr[c].level = level;
		wr[c].is_directory = ((FROM_LE_32(dirent[c].offset_to_data) & IMAGE_RESOURCE_DATA_IS_DIRECTORY) != 0);
		wr[c].children = fi->first_resource + (FROM_LE_32(dirent[c].offset_to_data) & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);

		/* fill in wr->id, wr->numeric_id */
		if (!decode_pe_resource_id(fi, wr + c, FROM_LE_32(dirent[c].name)))
			return NULL;
	}

	return wr;
}


/* list_resources:
 *   Return an array of WinResource's in the current
 *   resource level specified by _res->
 */
Win32ResExtractor::WinResource *Win32ResExtractor::list_resources(WinLibrary *fi, WinResource *res, int *count) {
	if (res != NULL && !res->is_directory)
		return NULL;

	return list_pe_resources(fi, (Win32ImageResourceDirectory *)
			 (res == NULL ? fi->first_resource : res->children),
			 (res == NULL ? 0 : res->level+1),
			 count);
}

/* read_library:
 *   Read header and get resource directory offset in a Windows library
 *    (AKA module).
 *
 */
bool Win32ResExtractor::read_library(WinLibrary *fi) {
	/* check for DOS header signature `MZ' */
	RETURN_IF_BAD_POINTER(false, MZ_HEADER(fi->memory)->magic);
	if (FROM_LE_16(MZ_HEADER(fi->memory)->magic) == IMAGE_DOS_SIGNATURE) {
		DOSImageHeader *mz_header = MZ_HEADER(fi->memory);

		RETURN_IF_BAD_POINTER(false, mz_header->lfanew);

		// Apply endian fix (currently only lfanew is used from the DOSImageHeader,
		// so we don't bother to 'fix' the rest).
		LE32(mz_header->lfanew);

		if (mz_header->lfanew < sizeof(DOSImageHeader)) {
			error("%s: not a Windows library", _fileName.c_str());
			return false;
		}
	}

	/* check for NT header signature `PE' */
	RETURN_IF_BAD_POINTER(false, PE_HEADER(fi->memory)->signature);
	if (FROM_LE_32(PE_HEADER(fi->memory)->signature) == IMAGE_NT_SIGNATURE) {
		Win32ImageNTHeaders *pe_header;
		int d;

		// Fix image header endianess
		fix_win32_image_header_endian(PE_HEADER(fi->memory));

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
			Win32ImageSectionHeader *pe_sec = PE_SECTIONS(fi->memory) + d;

			if (pe_sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
				continue;

			//if (pe_sec->virtual_address + pe_sec->size_of_raw_data > fi->total_size)

			RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->virtual_address, pe_sec->size_of_raw_data);
			RETURN_IF_BAD_OFFSET(0, fi->memory + pe_sec->pointer_to_raw_data, pe_sec->size_of_raw_data);
			if (FROM_LE_32(pe_sec->virtual_address) != pe_sec->pointer_to_raw_data) {
				memmove(fi->memory + pe_sec->virtual_address,
						fi->memory + pe_sec->pointer_to_raw_data,
						pe_sec->size_of_raw_data);
			}
		}

		/* find resource directory */
		RETURN_IF_BAD_POINTER(false, pe_header->optional_header.data_directory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
		Win32ImageDataDirectory *dir = pe_header->optional_header.data_directory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
		if (dir->size == 0) {
			error("%s: file contains no resources", _fileName.c_str());
			return false;
		}

		fix_win32_image_data_directory(dir);

		fi->first_resource = fi->memory + dir->virtual_address;
		return true;
	}

	/* other (unknown) header signature was found */
	error("%s: not a Windows library", _fileName.c_str());
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
		fix_win32_image_section_header(seg);

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
			int *hotspot_x, int *hotspot_y, int *keycolor, byte **pal, int *palSize) {
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
		error("not an icon or cursor file (reserved non-zero)");
		goto cleanup;
	}
	if (dir.type != 1 && dir.type != 2) {
		error("not an icon or cursor file (wrong type)");
		goto cleanup;
	}

	entries = (Win32CursorIconFileDirEntry *)malloc(dir.count * sizeof(Win32CursorIconFileDirEntry));
	for (c = 0; c < dir.count; c++) {
		if (!in->read(&entries[c], sizeof(Win32CursorIconFileDirEntry)))
			goto cleanup;
		fix_win32_cursor_icon_file_dir_entry_endian(&entries[c]);
		if (entries[c].reserved != 0)
			error("reserved is not zero");
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
					error("bitmap header is too short");
					goto local_cleanup;
				}
				if (bitmap.compression != 0) {
					error("compressed image data not supported");
					goto local_cleanup;
				}
				if (bitmap.x_pels_per_meter != 0)
					error("x_pels_per_meter field in bitmap should be zero");
				if (bitmap.y_pels_per_meter != 0)
					error("y_pels_per_meter field in bitmap should be zero");
				if (bitmap.clr_important != 0)
					error("clr_important field in bitmap should be zero");
				if (bitmap.planes != 1)
					error("planes field in bitmap should be one");
				if (bitmap.size != sizeof(Win32BitmapInfoHeader)) {
					uint32 skip = bitmap.size - sizeof(Win32BitmapInfoHeader);
					error("skipping %d bytes of extended bitmap header", skip);
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
					    (int)(bitmap.size + image_size + mask_size + palette_count * sizeof(Win32RGBQuad))
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
					uint32 mmod = y * (mask_size / height) * 8;

					for (x = 0; x < width; x++) {

						uint32 color = simple_vec(image_data, x + imod, bitmap.bit_count);

						// We set up cursor palette for default cursor, so use it
						if (!simple_vec(mask_data, x + mmod, 1)) {
							if (color) {
								cursor[0][width * d + x] = 254; // white
							} else {
								cursor[0][width * d + x] = 253; // black
							}
						} else {
							cursor[0][width * d + x] = 255; // transparent
						}
						/*

						if (bitmap.bit_count <= 16) {
							if (color >= palette_count) {
								error("color out of range in image data");
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
				error("offset of bitmap header incorrect (too low)");
				goto cleanup;
			}
			assert(min_offset != 0x7fffffff);
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

	// FIXME: Does this assert ever trigger? If so, we should modify this function
	// to properly deal with it.
	assert(obj->file_header.size_of_optional_header >= sizeof(obj->optional_header));
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


MacResExtractor::MacResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
	_resOffset = -1;
}

int MacResExtractor::extractResource(int id, byte **buf) {
	Common::File in;
	int size;

	if (_fileName.empty()) { // We are running for the first time
		_fileName = _vm->generateFilename(-3);

		// Some programs write it as .bin. Try that too
		if (!in.open(_fileName)) {
			Common::String tmp(_fileName);

			_fileName += ".bin";

			if (!in.open(_fileName)) {
				// And finally check if we have dumped resource fork
				_fileName = tmp;
				_fileName += ".bin";
				if (!in.open(_fileName)) {
					error("Cannot open file any of files '%s', '%s.bin', '%s.rsrc",
						  tmp.c_str(), tmp.c_str(), tmp.c_str());
				}
			}
		}
	} else
		in.open(_fileName);

	if (!in.isOpen()) {
		error("Cannot open file %s", _fileName.c_str());
	}

	// we haven't calculated it
	if (_resOffset == -1) {
		if (!init(in))
			error("Resource fork is missing in file '%s'", _fileName.c_str());
		in.close();
		in.open(_fileName);
	}

	*buf = getResource(in, "crsr", 1000 + id, &size);

	in.close();

	if (*buf == NULL)
		error("There is no cursor ID #%d", 1000 + id);

	return size;
}

#define MBI_INFOHDR 128
#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

bool MacResExtractor::init(Common::File &in) {
	byte infoHeader[MBI_INFOHDR];
	int32 data_size, rsrc_size;
	int32 data_size_pad, rsrc_size_pad;
	int filelen;

	filelen = in.size();
	in.read(infoHeader, MBI_INFOHDR);

	// Maybe we have MacBinary?
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		data_size = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		rsrc_size = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		data_size_pad = (((data_size + 127) >> 7) << 7);
		rsrc_size_pad = (((rsrc_size + 127) >> 7) << 7);

		// Length check
		int sumlen =  MBI_INFOHDR + data_size_pad + rsrc_size_pad;

		if (sumlen == filelen)
			_resOffset = MBI_INFOHDR + data_size_pad;
	}

	if (_resOffset == -1) // MacBinary check is failed
		_resOffset = 0; // Maybe we have dumped fork?

	in.seek(_resOffset);

	_dataOffset = in.readUint32BE() + _resOffset;
	_mapOffset = in.readUint32BE() + _resOffset;
	_dataLength = in.readUint32BE();
	_mapLength = in.readUint32BE();

	// do sanity check
	if (_dataOffset >= filelen || _mapOffset >= filelen ||
		_dataLength + _mapLength  > filelen) {
		_resOffset = -1;
		return false;
	}

	debug(7, "got header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	readMap(in);

	return true;
}

byte *MacResExtractor::getResource(Common::File &in, const char *typeID, int16 resID, int *size) {
	int i;
	int typeNum = -1;
	int resNum = -1;
	byte *buf;
	int len;

	for (i = 0; i < _resMap.numTypes; i++)
		if (strcmp(_resTypes[i].id, typeID) == 0) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return NULL;

	for (i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID) {
			resNum = i;
			break;
		}

	if (resNum == -1)
		return NULL;

	in.seek(_dataOffset + _resLists[typeNum][resNum].dataOffset);

	len = in.readUint32BE();
	buf = (byte *)malloc(len);

	in.read(buf, len);

	*size = len;

	return buf;
}

void MacResExtractor::readMap(Common::File &in) {
	int	i, j, len;

	in.seek(_mapOffset + 22);

	_resMap.resAttr = in.readUint16BE();
	_resMap.typeOffset = in.readUint16BE();
	_resMap.nameOffset = in.readUint16BE();
	_resMap.numTypes = in.readUint16BE();
	_resMap.numTypes++;

	in.seek(_mapOffset + _resMap.typeOffset + 2);
	_resTypes = new ResType[_resMap.numTypes];

	for (i = 0; i < _resMap.numTypes; i++) {
		in.read(_resTypes[i].id, 4);
		_resTypes[i].id[4] = 0;
		_resTypes[i].items = in.readUint16BE();
		_resTypes[i].offset = in.readUint16BE();
		_resTypes[i].items++;
	}

	_resLists = new ResPtr[_resMap.numTypes];

	for (i = 0; i < _resMap.numTypes; i++) {
		_resLists[i] = new Resource[_resTypes[i].items];
		in.seek(_resTypes[i].offset + _mapOffset + _resMap.typeOffset);

		for (j = 0; j < _resTypes[i].items; j++) {
			ResPtr resPtr = _resLists[i] + j;

			resPtr->id = in.readUint16BE();
			resPtr->nameOffset = in.readUint16BE();
			resPtr->dataOffset = in.readUint32BE();
			in.readUint32BE();
			resPtr->name = 0;

			resPtr->attr = resPtr->dataOffset >> 24;
			resPtr->dataOffset &= 0xFFFFFF;
		}

		for (j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1) {
				in.seek(_resLists[i][j].nameOffset + _mapOffset + _resMap.nameOffset);

				len = in.readByte();
				_resLists[i][j].name = new byte[len + 1];
				_resLists[i][j].name[len] = 0;
				in.read(_resLists[i][j].name, len);
			}
		}
	}
}

int MacResExtractor::convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			  int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize) {
	Common::MemoryReadStream dis(data, datasize);
	int i, b;
	byte imageByte;
	byte *iconData;
	int numBytes;
	int pixelsPerByte, bpp;
	int ctSize;
	byte bitmask;
	int iconRowBytes, iconBounds[4];
	int ignored;
	int iconDataSize;

	dis.readUint16BE(); // type
	dis.readUint32BE(); // offset to pixel map
	dis.readUint32BE(); // offset to pixel data
	dis.readUint32BE(); // expanded cursor data
	dis.readUint16BE(); // expanded data depth
	dis.readUint32BE(); // reserved

	// Grab B/W icon data
	*cursor = (byte *)malloc(16 * 16);
	for (i = 0; i < 32; i++) {
		imageByte = dis.readByte();
		for (b = 0; b < 8; b++)
			cursor[0][i*8+b] = (byte)((imageByte &
									  (0x80 >> b)) > 0? 0x0F: 0x00);
	}

	// Apply mask data
	for (i = 0; i < 32; i++) {
		imageByte = dis.readByte();
		for (b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				cursor[0][i*8+b] = 0xff;
	}

	*hotspot_y = dis.readUint16BE();
	*hotspot_x = dis.readUint16BE();
	*w = *h = 16;

	// Use b/w cursor on backends which don't support cursor palettes
	if (!_vm->_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return 1;

	dis.readUint32BE(); // reserved
	dis.readUint32BE(); // cursorID

	// Color version of cursor
	dis.readUint32BE(); // baseAddr

	// Keep only lowbyte for now
	dis.readByte();
	iconRowBytes = dis.readByte();

	if (!iconRowBytes)
		return 1;

	iconBounds[0] = dis.readUint16BE();
	iconBounds[1] = dis.readUint16BE();
	iconBounds[2] = dis.readUint16BE();
	iconBounds[3] = dis.readUint16BE();

	dis.readUint16BE(); // pmVersion
	dis.readUint16BE(); // packType
	dis.readUint32BE(); // packSize

	dis.readUint32BE(); // hRes
	dis.readUint32BE(); // vRes

	dis.readUint16BE(); // pixelType
	dis.readUint16BE(); // pixelSize
	dis.readUint16BE(); // cmpCount
	dis.readUint16BE(); // cmpSize

	dis.readUint32BE(); // planeByte
	dis.readUint32BE(); // pmTable
	dis.readUint32BE(); // reserved

	// Pixel data for cursor
	iconDataSize =  iconRowBytes * (iconBounds[3] - iconBounds[1]);
	iconData = (byte *)malloc(iconDataSize);
	dis.read(iconData, iconDataSize);

	// Color table
	dis.readUint32BE(); // ctSeed
	dis.readUint16BE(); // ctFlag
	ctSize = dis.readUint16BE() + 1;

	*palette = (byte *)malloc(ctSize * 4);

	// Read just high byte of 16-bit color
	for (int c = 0; c < ctSize; c++) {
		// We just use indices 0..ctSize, so ignore color ID
		dis.readUint16BE(); // colorID[c]

		palette[0][c * 4 + 0] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 1] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 2] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 3] = 0;
	}

	*palSize = ctSize;

	numBytes =
         (iconBounds[2] - iconBounds[0]) *
         (iconBounds[3] - iconBounds[1]);

	pixelsPerByte = (iconBounds[2] - iconBounds[0]) / iconRowBytes;
	bpp           = 8 / pixelsPerByte;

	// build a mask to make sure the pixels are properly shifted out
	bitmask = 0;
	for (int m = 0; m < bpp; m++) {
		bitmask <<= 1;
		bitmask  |= 1;
	}

	// Extract pixels from bytes
	for (int j = 0; j < iconDataSize; j++)
		for (b = 0; b < pixelsPerByte; b++) {
			int idx = j * pixelsPerByte + (pixelsPerByte - 1 - b);

			if (cursor[0][idx] != 0xff) // if mask is not there
				cursor[0][idx] = (byte)((iconData[j] >> (b * bpp)) & bitmask);
		}

	free(iconData);

	assert(datasize - dis.pos() == 0);

	return 1;
}



void ScummEngine_v70he::readRoomsOffsets() {
	int num, i;
	byte *ptr;

	debug(9, "readRoomOffsets()");

	num = READ_LE_UINT16(_heV7RoomOffsets);
	ptr = _heV7RoomOffsets + 2;
	for (i = 0; i < num; i++) {
		_res->roomoffs[rtRoom][i] = READ_LE_UINT32(ptr);
		ptr += 4;
	}
}

void ScummEngine_v70he::readGlobalObjects() {
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectOwnerTable, num);
	_fileHandle->read(_objectRoomTable, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (int i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

#ifdef ENABLE_HE
void ScummEngine_v99he::readMAXS(int blockSize) {
	if (blockSize == 52) {
		debug(0, "ScummEngine_v99he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numPalettes = _fileHandle->readUint16LE();
		_numUnk = _fileHandle->readUint16LE();
		_numTalkies = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2048;
	} else
		ScummEngine_v90he::readMAXS(blockSize);
}

void ScummEngine_v90he::readMAXS(int blockSize) {
	if (blockSize == 46) {
		debug(0, "ScummEngine_v90he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		if (_game.features & GF_HE_985)
			_numGlobalScripts = 2048;
		else
			_numGlobalScripts = 200;
	} else
		ScummEngine_v72he::readMAXS(blockSize);
}

void ScummEngine_v72he::readMAXS(int blockSize) {
	if (blockSize == 40) {
		debug(0, "ScummEngine_v72he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 200;
	} else
		ScummEngine_v6::readMAXS(blockSize);
}

byte *ScummEngine_v72he::getStringAddress(int i) {
	byte *addr = getResourceAddress(rtString, i);
	if (addr == NULL)
		return NULL;
	return ((ScummEngine_v72he::ArrayHeader *)addr)->data;
}

int ScummEngine_v72he::getSoundResourceSize(int id) {
	const byte *ptr;
	int offs, size;

	if (id > _numSounds) {
		if (!((SoundHE *)_sound)->getHEMusicDetails(id, offs, size)) {
			debug(0, "getSoundResourceSize: musicID %d not found", id);
			return 0;
		}
	} else {
		ptr = getResourceAddress(rtSound, id);
		if (!ptr)
			return 0;

		if (READ_BE_UINT32(ptr) == MKID_BE('RIFF')) {
			byte flags;
			int rate;

			size = READ_BE_UINT32(ptr + 4);
			Common::MemoryReadStream stream(ptr, size);

			if (!Audio::loadWAVFromStream(stream, size, rate, flags)) {
				error("getSoundResourceSize: Not a valid WAV file");
			}
		} else {
			ptr += 8 + READ_BE_UINT32(ptr + 12);
			if (READ_BE_UINT32(ptr) == MKID_BE('SBNG')) {
				ptr += READ_BE_UINT32(ptr + 4);
			}

			assert(READ_BE_UINT32(ptr) == MKID_BE('SDAT'));
			size = READ_BE_UINT32(ptr + 4) - 8;
		}
	}

	return size;
}

#endif

} // End of namespace Scumm
