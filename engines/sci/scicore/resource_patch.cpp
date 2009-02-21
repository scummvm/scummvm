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

#include "common/archive.h"
#include "common/file.h"

#include "sci/include/sciresource.h"
#include "sci/include/sci_memory.h"

namespace Sci {

void sci0_sprintf_patch_file_name(char *string, resource_t *res) {
	sprintf(string, "%s.%03i", sci_resource_types[res->type], res->number);
}

void sci1_sprintf_patch_file_name(char *string, resource_t *res) {
	sprintf(string, "%d.%s", res->number, sci_resource_type_suffixes[res->type]);
}

/* version-agnostic patch application */
static void process_patch(ResourceSource *source,
              Common::ArchiveMember &member, int restype, int resnumber, resource_t **resource_p, int *resource_nr_p) {
	Common::File file;

	if (restype == sci_invalid_resource)
		return;

	printf("Patching \"%s\": ", member.getName().c_str());
	if (!file.open(member.createReadStream(), member.getName()))
		perror("""__FILE__"": (""__LINE__""): failed to open");
	else {
		guint8 filehdr[2];
		resource_t *newrsc = _scir_find_resource_unsorted(*resource_p,
		                     *resource_nr_p,
		                     restype,
		                     resnumber);
		int fsize = file.size();
		if (fsize < 3) {
			printf("File too small\n");
			return;
		}

		int patch_data_offset;

		file.read(filehdr, 2);

		patch_data_offset = filehdr[1];

		if ((filehdr[0] & 0x7f) != restype) {
			printf("Failed; resource type mismatch\n");
		} else if (patch_data_offset + 2 >= fsize) {
			printf("Failed; patch starting at offset %d can't be in file of size %d\n", filehdr[1] + 2, fsize);
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

			printf("OK\n");

		}
	}
}


int sci0_read_resource_patches(ResourceSource *source, resource_t **resource_p, int *resource_nr_p) {
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.???");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		int restype = sci_invalid_resource;
		int resnumber = -1;
		int i;
		unsigned int resname_len;
		char *endptr;

		for (i = sci_view; i < sci_invalid_resource; i++)
			if (scumm_strnicmp(sci_resource_types[i], name.c_str(),
			                strlen(sci_resource_types[i])) == 0)
				restype = i;

		if (restype != sci_invalid_resource) {

			resname_len = strlen(sci_resource_types[restype]);
			if (name[resname_len] != '.')
				restype = sci_invalid_resource;
			else {
				resnumber = strtol(name.c_str() + 1 + resname_len,
				                   &endptr, 10); /* Get resource number */
				if ((*endptr != '\0') || (resname_len + 1 == name.size()))
					restype = sci_invalid_resource;

				if ((resnumber < 0) || (resnumber > 1000))
					restype = sci_invalid_resource;
			}
		}

		process_patch(source, **x, restype, resnumber, resource_p, resource_nr_p);
	}

	return 0;
}

int sci1_read_resource_patches(ResourceSource *source, resource_t **resource_p, int *resource_nr_p) {
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.*");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		int restype = sci_invalid_resource;
		int resnumber = -1;
		int i;
		char *endptr;
		const char *dot = strchr(name.c_str(), '.');

		for (i = sci_view; i < sci_invalid_resource; i++) {
			if (dot != NULL) {
				if (scumm_strnicmp(sci_resource_type_suffixes[i], dot + 1, 3) == 0) {
					restype = i;
				}
			}
		}

		if (restype != sci_invalid_resource) {

			resnumber = strtol(name.c_str(),
			                   &endptr, 10); /* Get resource number */

			if (endptr != dot)
				restype = sci_invalid_resource;

			if (*(dot + 4) != '\0')
				restype = sci_invalid_resource;

			if ((resnumber < 0) || (resnumber > 8192))
				restype = sci_invalid_resource;
		}

		process_patch(source, **x, restype, resnumber, resource_p, resource_nr_p);
	}

	return 0;
}

} // End of namespace Sci
