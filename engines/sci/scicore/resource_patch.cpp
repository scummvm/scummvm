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


#include "sci/include/sciresource.h"
#include "sci/include/sci_memory.h"

void
sci0_sprintf_patch_file_name(char *string, resource_t *res) {
	sprintf(string, "%s.%03i", sci_resource_types[res->type], res->number);
}

void
sci1_sprintf_patch_file_name(char *string, resource_t *res) {
	sprintf(string, "%d.%s", res->number, sci_resource_type_suffixes[res->type]);
}

/* version-agnostic patch application */
static void
process_patch(resource_source_t *source,
              char *entry, int restype, int resnumber, resource_t **resource_p, int *resource_nr_p) {
	int fsize;
	char filename[MAXPATHLEN];

	if (restype == sci_invalid_resource)
		return;

	printf("Patching \"%s\": ", entry);

	sprintf(filename, "%s%s", source->location.dir.name, entry);
	fsize = sci_file_size(filename);
	if (fsize < 0)
		perror("""__FILE__"": (""__LINE__""): sci_file_size()");
	else {
		int file;
		guint8 filehdr[2];
		resource_t *newrsc = _scir_find_resource_unsorted(*resource_p,
		                     *resource_nr_p,
		                     restype,
		                     resnumber);

		if (fsize < 3) {
			printf("File too small\n");
			return;
		}

		file = open(entry, O_RDONLY);
		if (!IS_VALID_FD(file))
			perror("""__FILE__"": (""__LINE__""): open()");
		else {
			int patch_data_offset;

			read(file, filehdr, 2);

			patch_data_offset = filehdr[1];

			if ((filehdr[0] & 0x7f) != restype) {
				printf("Failed; resource type mismatch\n");
				close(file);
			} else if (patch_data_offset + 2 >= fsize) {
				printf("Failed; patch starting at offset %d can't be in file of size %d\n", filehdr[1] + 2, fsize);
				close(file);
			} else {
				/* Adjust for file offset */
				fsize -= patch_data_offset;

				/* Prepare destination, if neccessary */
				if (!newrsc) {
					/* Completely new resource! */
					++(*resource_nr_p);
					*resource_p = (resource_t*)sci_realloc(*resource_p,
					                                       *resource_nr_p
					                                       * sizeof(resource_t));
					newrsc = (*resource_p - 1) + *resource_nr_p;
					newrsc->alt_sources = NULL;
				}

				/* Overwrite everything, because we're patching */
				newrsc->size = fsize - 2;
				newrsc->id = restype << 11 | resnumber;
				newrsc->number = resnumber;
				newrsc->status = SCI_STATUS_NOMALLOC;
				newrsc->type = restype;
				newrsc->source = source;
				newrsc->file_offset = 2 + patch_data_offset;

				_scir_add_altsource(newrsc, source, 2);

				close(file);

				printf("OK\n");

			}
		}
	}
}


int
sci0_read_resource_patches(resource_source_t *source, resource_t **resource_p, int *resource_nr_p) {
	sci_dir_t dir;
	char *entry;
	char *caller_cwd = sci_getcwd();

	chdir(source->location.dir.name);
	sci_init_dir(&dir);
	entry = sci_find_first(&dir, "*.???");
	while (entry) {
		int restype = sci_invalid_resource;
		int resnumber = -1;
		int i;
		unsigned int resname_len;
		char *endptr;

		for (i = sci_view; i < sci_invalid_resource; i++)
			if (scumm_strnicmp(sci_resource_types[i], entry,
			                strlen(sci_resource_types[i])) == 0)
				restype = i;

		if (restype != sci_invalid_resource) {

			resname_len = strlen(sci_resource_types[restype]);
			if (entry[resname_len] != '.')
				restype = sci_invalid_resource;
			else {
				resnumber = strtol(entry + 1 + resname_len,
				                   &endptr, 10); /* Get resource number */
				if ((*endptr != '\0') || (resname_len + 1 == strlen(entry)))
					restype = sci_invalid_resource;

				if ((resnumber < 0) || (resnumber > 1000))
					restype = sci_invalid_resource;
			}
		}

		process_patch(source, entry, restype, resnumber, resource_p, resource_nr_p);

		entry = sci_find_next(&dir);
	}

	chdir(caller_cwd);
	free(caller_cwd);
	return 0;
}

int
sci1_read_resource_patches(resource_source_t *source, resource_t **resource_p, int *resource_nr_p) {
	sci_dir_t dir;
	char *entry;
	char *caller_cwd = sci_getcwd();

	chdir(source->location.dir.name);
	sci_init_dir(&dir);
	entry = sci_find_first(&dir, "*.*");
	while (entry) {
		int restype = sci_invalid_resource;
		int resnumber = -1;
		int i;
		char *endptr;
		char *dot = strchr(entry, '.');

		for (i = sci_view; i < sci_invalid_resource; i++) {
			if (dot != NULL) {
				if (scumm_strnicmp(sci_resource_type_suffixes[i], dot + 1, 3) == 0) {
					restype = i;
				}
			}
		}

		if (restype != sci_invalid_resource) {

			resnumber = strtol(entry,
			                   &endptr, 10); /* Get resource number */

			if (endptr != dot)
				restype = sci_invalid_resource;

			if (*(dot + 4) != '\0')
				restype = sci_invalid_resource;

			if ((resnumber < 0) || (resnumber > 8192))
				restype = sci_invalid_resource;
		}

		process_patch(source, entry, restype, resnumber, resource_p, resource_nr_p);

		entry = sci_find_next(&dir);
	}

	chdir(caller_cwd);
	free(caller_cwd);
	return 0;
}

