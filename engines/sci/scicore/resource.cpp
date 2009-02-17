/***************************************************************************
 resource.c Copyright (C) 1999 Christoph Reichenbach, TU Darmstadt


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   990327 - created (CJR)

***************************************************************************/
/* Resource library */

#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"
#include "sci/include/vocabulary.h" /* For SCI version auto-detection */

#undef SCI_REQUIRE_RESOURCE_FILES
/* #define SCI_VERBOSE_RESMGR 1 */

const char* sci_version_types[] = {
	"SCI version undetermined (Autodetect failed / not run)",
	"SCI version 0.xxx",
	"SCI version 0.xxx w/ 1.000 compression",
	"SCI version 1.000 w/ 0.xxx resource.map",
	"SCI version 1.000 w/ special resource.map",
	"SCI version 1.000 (early)",
	"SCI version 1.000 (late)",
	"SCI version 1.001",
	"SCI WIN/32"
};

const int sci_max_resource_nr[] = {65536, 1000, 2048, 2048, 2048, 8192, 8192, 65536};

const char* sci_error_types[] = {
	"No error",
	"I/O error",
	"Resource is empty (size 0)",
	"resource.map entry is invalid",
	"resource.map file not found",
	"No resource files found",
	"Unknown compression method",
	"Decompression failed: Decompression buffer overflow",
	"Decompression failed: Sanity check failed",
	"Decompression failed: Resource too big",
	"SCI version is unsupported"
};

const char* sci_resource_types[] = {"view", "pic", "script", "text", "sound",
                                    "memory", "vocab", "font", "cursor",
                                    "patch", "bitmap", "palette", "cdaudio",
                                    "audio", "sync", "message", "map", "heap"
                                   };
/* These are the 18 resource types supported by SCI1 */

const char *sci_resource_type_suffixes[] = {"v56", "p56", "scr", "tex", "snd",
        "   ", "voc", "fon", "cur", "pat",
        "bit", "pal", "cda", "aud", "syn",
        "msg", "map", "hep"
                                           };


int resourcecmp(const void *first, const void *second);


typedef int decomp_funct(resource_t *result, int resh, int sci_version);
typedef void patch_sprintf_funct(char *string, resource_t *res);

static decomp_funct *decompressors[] = {
	NULL,
	&decompress0,
	&decompress01,
	&decompress01,
	&decompress01,
	&decompress1,
	&decompress1,
	&decompress11,
	NULL
};

static patch_sprintf_funct *patch_sprintfers[] = {
	NULL,
	&sci0_sprintf_patch_file_name,
	&sci0_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name
};


int resourcecmp(const void *first, const void *second) {
	if (((resource_t *)first)->type ==
	        ((resource_t *)second)->type)
		return (((resource_t *)first)->number <
		        ((resource_t *)second)->number) ? -1 :
		       !(((resource_t *)first)->number ==
		         ((resource_t *)second)->number);
	else
		return (((resource_t *)first)->type <
		        ((resource_t *)second)->type) ? -1 : 1;
}





/*-----------------------------*/
/*-- Resmgr helper functions --*/
/*-----------------------------*/

void
_scir_add_altsource(resource_t *res, resource_source_t *source, unsigned int file_offset) {
	resource_altsource_t *rsrc = (resource_altsource_t*)sci_malloc(sizeof(resource_altsource_t));

	rsrc->next = res->alt_sources;
	rsrc->source = source;
	rsrc->file_offset = file_offset;
	res->alt_sources = rsrc;
}

resource_t *
_scir_find_resource_unsorted(resource_t *res, int res_nr, int type, int number) {
	int i;
	for (i = 0; i < res_nr; i++)
		if (res[i].number == number && res[i].type == type)
			return res + i;
	return NULL;
}

/*-----------------------------------*/
/** Resource source list management **/
/*-----------------------------------*/

resource_source_t *
scir_add_external_map(resource_mgr_t *mgr, char *file_name) {
	resource_source_t *newsrc = (resource_source_t *)
	                            malloc(sizeof(resource_source_t));

	/* Add the new source to the SLL of sources */
	newsrc->next = mgr->sources;
	mgr->sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_EXTERNAL_MAP;
	newsrc->location.file.name = strdup(file_name);
	newsrc->scanned = 0;
	newsrc->associated_map = NULL;

	return newsrc;
}

resource_source_t *
scir_add_volume(resource_mgr_t *mgr, resource_source_t *map, char *filename,
                int number, int extended_addressing) {
	resource_source_t *newsrc = (resource_source_t *)
	                            malloc(sizeof(resource_source_t));

	/* Add the new source to the SLL of sources */
	newsrc->next = mgr->sources;
	mgr->sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_VOLUME;
	newsrc->scanned = 0;
	newsrc->location.file.name = strdup(filename);
	newsrc->location.file.volume_number = number;
	newsrc->associated_map = map;
	return 0;
}

resource_source_t *
scir_add_patch_dir(resource_mgr_t *mgr, int type, char *dirname) {
	resource_source_t *newsrc = (resource_source_t *)
	                            malloc(sizeof(resource_source_t));

	/* Add the new source to the SLL of sources */
	newsrc->next = mgr->sources;
	mgr->sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_DIRECTORY;
	newsrc->scanned = 0;
	newsrc->location.dir.name = strdup(dirname);
	return 0;
}

resource_source_t *
scir_get_volume(resource_mgr_t *mgr, resource_source_t *map, int volume_nr) {
	resource_source_t *seeker = mgr->sources;

	while (seeker) {
		if (seeker->source_type == RESSOURCE_TYPE_VOLUME &&
		        seeker->associated_map == map &&
		        seeker->location.file.volume_number == volume_nr)
			return seeker;
		seeker = seeker->next;
	}

	return NULL;
}

/*------------------------------------------------*/
/** Resource manager constructors and operations **/
/*------------------------------------------------*/

static void
_scir_init_trivial(resource_mgr_t *mgr) {
	mgr->resources_nr = 0;
	mgr->resources = (resource_t*)sci_malloc(1);
}


static void
_scir_load_from_patch_file(int fh, resource_t *res, char *filename) {
	unsigned int really_read;

	res->data = (unsigned char*)sci_malloc(res->size);
	really_read = read(fh, res->data, res->size);

	if (really_read < res->size) {
		sciprintf("Error: Read %d bytes from %s but expected %d!\n",
		          really_read, filename, res->size);
		exit(1);
	}

	res->status = SCI_STATUS_ALLOCATED;
}

static void
_scir_load_resource(resource_mgr_t *mgr, resource_t *res, int protect) {
	char filename[MAXPATHLEN];
	int fh;
	resource_t backup;
	char *save_cwd = sci_getcwd();

	memcpy(&backup, res, sizeof(resource_t));

	/* First try lower-case name */
	if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY) {

		if (!patch_sprintfers[mgr->sci_version]) {
			sciprintf("Resource manager's SCI version (%d) has no patch file name printers -> internal error!\n",
			          mgr->sci_version);
			exit(1);
		}

		/* Get patch file name */
		patch_sprintfers[mgr->sci_version](filename, res);
		chdir(res->source->location.dir.name);
	} else
		strcpy(filename, res->source->location.file.name);

	fh = open(filename, O_RDONLY | O_BINARY);


	if (!IS_VALID_FD(fh)) {
		char *raiser = filename;
		while (*raiser) {
			*raiser = toupper(*raiser); /* Uppercasify */
			++raiser;
		}
		fh = sci_open(filename, O_RDONLY | O_BINARY);
	}    /* Try case-insensitively name */

	if (!IS_VALID_FD(fh)) {
		sciprintf("Failed to open %s!\n", filename);
		res->data = NULL;
		res->status = SCI_STATUS_NOMALLOC;
		res->size = 0;
		chdir(save_cwd);
		free(save_cwd);
		return;
	}


	lseek(fh, res->file_offset, SEEK_SET);

	if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY ||
	        res->source->source_type == RESSOURCE_TYPE_AUDIO_DIRECTORY)
		_scir_load_from_patch_file(fh, res, filename);
	else if (!decompressors[mgr->sci_version]) {
		/* Check whether we support this at all */
		sciprintf("Resource manager's SCI version (%d) is invalid!\n",
		          mgr->sci_version);
		exit(1);
	} else {
		int error = /* Decompress from regular resource file */
		    decompressors[mgr->sci_version](res, fh, mgr->sci_version);

		if (error) {
			sciprintf("Error %d occured while reading %s.%03d"
			          " from resource file: %s\n",
			          error, sci_resource_types[res->type], res->number,
			          sci_error_types[error]);

			if (protect)
				memcpy(res, &backup, sizeof(resource_t));

			res->data = NULL;
			res->status = SCI_STATUS_NOMALLOC;
			res->size = 0;
			chdir(save_cwd);
			free(save_cwd);
			return;
		}
	}

	close(fh);
}

resource_t *
scir_test_resource(resource_mgr_t *mgr, int type, int number) {
	resource_t binseeker;
	binseeker.type = type;
	binseeker.number = number;
	return (resource_t *)
	       bsearch(&binseeker, mgr->resources, mgr->resources_nr,
	               sizeof(resource_t), resourcecmp);
}

int sci0_get_compression_method(int resh);

int
sci_test_view_type(resource_mgr_t *mgr) {
	int fh;
	char filename[MAXPATHLEN];
	int compression;
	resource_t *res;
	int i;

	mgr->sci_version = SCI_VERSION_AUTODETECT;

	for (i = 0;i < 1000;i++) {
		res = scir_test_resource(mgr, sci_view, i);

		if (!res) continue;

		if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY ||
		        res->source->source_type == RESSOURCE_TYPE_AUDIO_DIRECTORY)
			continue;

		strcpy(filename, res->source->location.file.name);
		fh = open(filename, O_RDONLY | O_BINARY);

		if (!IS_VALID_FD(fh)) {
			char *raiser = filename;
			while (*raiser) {
				*raiser = toupper(*raiser); /* Uppercasify */
				++raiser;
			}
			fh = sci_open(filename, O_RDONLY | O_BINARY);
		}    /* Try case-insensitively name */

		if (!IS_VALID_FD(fh)) continue;
		lseek(fh, res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(fh);
		close(fh);

		if (compression == 3)
			return (mgr->sci_version = SCI_VERSION_01_VGA);
	}

	/* Try the same thing with pics */
	for (i = 0;i < 1000;i++) {
		res = scir_test_resource(mgr, sci_pic, i);

		if (!res) continue;

		if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY ||
		        res->source->source_type == RESSOURCE_TYPE_AUDIO_DIRECTORY)
			continue;

		strcpy(filename, res->source->location.file.name);
		fh = open(filename, O_RDONLY | O_BINARY);


		if (!IS_VALID_FD(fh)) {
			char *raiser = filename;
			while (*raiser) {
				*raiser = toupper(*raiser); /* Uppercasify */
				++raiser;
			}
			fh = sci_open(filename, O_RDONLY | O_BINARY);
		}    /* Try case-insensitively name */

		if (!IS_VALID_FD(fh)) continue;
		lseek(fh, res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(fh);
		close(fh);

		if (compression == 3)
			return (mgr->sci_version = SCI_VERSION_01_VGA);
	}

	return mgr->sci_version;
}



int
scir_add_appropriate_sources(resource_mgr_t *mgr,
                             int allow_patches,
                             char *dir) {
	const char *trailing_slash = "";
	//char path_separator;
	sci_dir_t dirent;
	char *name;
	resource_source_t *map;
	int fd;
	char fullname[MAXPATHLEN];

	if (dir[strlen(dir)-1] != G_DIR_SEPARATOR) {
		trailing_slash = G_DIR_SEPARATOR_S;
	}

	name = (char *)malloc(strlen(dir) + 1 +
	                      strlen("RESOURCE.MAP") + 1);

	sprintf(fullname, "%s%s%s", dir, trailing_slash, "RESOURCE.MAP");
	fd = sci_open("RESOURCE.MAP", O_RDONLY | O_BINARY);
	if (!IS_VALID_FD(fd)) return 0;
	close(fd);
	map = scir_add_external_map(mgr, fullname);
	free(name);
	sci_init_dir(&dirent);
	name = sci_find_first(&dirent, "RESOURCE.0??");
	while (name != NULL) {
		char *dot = strrchr(name, '.');
		int number = atoi(dot + 1);

		sprintf(fullname, "%s%s%s", dir, G_DIR_SEPARATOR_S, name);
		scir_add_volume(mgr, map, fullname, number, 0);
		name = sci_find_next(&dirent);
	}
	sci_finish_find(&dirent);

	sci_finish_find(&dirent);
	sprintf(fullname, "%s%s", dir, G_DIR_SEPARATOR_S);
	scir_add_patch_dir(mgr, RESSOURCE_TYPE_DIRECTORY, fullname);

	return 1;
}

static int
_scir_scan_new_sources(resource_mgr_t *mgr, int *detected_version, resource_source_t *source) {
	int preset_version = mgr->sci_version;
	int resource_error = 0;
	int dummy = mgr->sci_version;
//	resource_t **concat_ptr = &(mgr->resources[mgr->resources_nr-1].next);

	if (detected_version == NULL)
		detected_version = &dummy;

	*detected_version = mgr->sci_version;
	if (source->next)
		_scir_scan_new_sources(mgr, detected_version, source->next);

	if (!source->scanned) {
		source->scanned = 1;
		switch (source->source_type) {
		case RESSOURCE_TYPE_DIRECTORY:
			if (mgr->sci_version <= SCI_VERSION_01)
				sci0_read_resource_patches(source,
				                           &mgr->resources,
				                           &mgr->resources_nr);
			else
				sci1_read_resource_patches(source,
				                           &mgr->resources,
				                           &mgr->resources_nr);
			break;
		case RESSOURCE_TYPE_EXTERNAL_MAP:
			if (preset_version <= SCI_VERSION_01_VGA_ODD
			        /* || preset_version == SCI_VERSION_AUTODETECT -- subsumed by the above line */) {
				resource_error =
				    sci0_read_resource_map(mgr,
				                           source,
				                           &mgr->resources,
				                           &mgr->resources_nr,
				                           detected_version);

#if 0
				if (resource_error >= SCI_ERROR_CRITICAL) {
					sciprintf("Resmgr: Error while loading resource map: %s\n",
					          sci_error_types[resource_error]);
					if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND)
						sciprintf("Running SCI games without a resource map is not supported ATM\n");
					sci_free(mgr);
					chdir(caller_cwd);
					free(caller_cwd);
					return NULL;
				}
				if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
					/* fixme: Try reading w/o resource.map */
					resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}

				if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
					/* Initialize empty resource manager */
					_scir_init_trivial(mgr);
					resource_error = 0;
				}
#endif
			}

			if ((preset_version == SCI_VERSION_1_EARLY) ||
			        (preset_version == SCI_VERSION_1_LATE) ||
			        (preset_version == SCI_VERSION_1_1) ||
			        ((*detected_version == SCI_VERSION_AUTODETECT) && (preset_version == SCI_VERSION_AUTODETECT))) {
				resource_error =
				    sci1_read_resource_map(mgr,
				                           source,
				                           scir_get_volume(mgr, source, 0),
				                           &mgr->resources,
				                           &mgr->resources_nr,
				                           detected_version);

				if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
					/* fixme: Try reading w/o resource.map */
					resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}

				if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
					/* Initialize empty resource manager */
					_scir_init_trivial(mgr);
					resource_error = 0;
				}

				*detected_version = SCI_VERSION_1;
			}

			mgr->sci_version = *detected_version;
			break;
		}
		qsort(mgr->resources, mgr->resources_nr, sizeof(resource_t),
		      resourcecmp); /* Sort resources */
	}
	return resource_error;
}

int
scir_scan_new_sources(resource_mgr_t *mgr, int *detected_version) {
	_scir_scan_new_sources(mgr, detected_version, mgr->sources);
	return 0;
}

static void
_scir_free_resource_sources(resource_source_t *rss) {
	if (rss) {
		_scir_free_resource_sources(rss->next);
		free(rss);
	}
}

resource_mgr_t *
scir_new_resource_manager(char *dir, int version,
                          char allow_patches, int max_memory) {
	int resource_error = 0;
	resource_mgr_t *mgr = (resource_mgr_t*)sci_malloc(sizeof(resource_mgr_t));
	char *caller_cwd = sci_getcwd();
	int resmap_version = version;

	if (chdir(dir)) {
		sciprintf("Resmgr: Directory '%s' is invalid!\n", dir);
		free(caller_cwd);
		return NULL;
	}

	mgr->max_memory = max_memory;

	mgr->memory_locked = 0;
	mgr->memory_lru = 0;

	mgr->resource_path = dir;

	mgr->resources = NULL;
	mgr->resources_nr = 0;
	mgr->sources = NULL;
	mgr->sci_version = version;

	scir_add_appropriate_sources(mgr, allow_patches, dir);
	scir_scan_new_sources(mgr, &resmap_version);

	if (!mgr->resources || !mgr->resources_nr) {
		if (mgr->resources) {
			free(mgr->resources);
			mgr->resources = NULL;
		}
		sciprintf("Resmgr: Could not retrieve a resource list!\n");
		_scir_free_resource_sources(mgr->sources);
		free(mgr);
		chdir(caller_cwd);
		free(caller_cwd);
		return NULL;
	}

	mgr->lru_first = NULL;
	mgr->lru_last = NULL;

	mgr->allow_patches = allow_patches;

	qsort(mgr->resources, mgr->resources_nr, sizeof(resource_t),
	      resourcecmp); /* Sort resources */

	if (version == SCI_VERSION_AUTODETECT)
		switch (resmap_version) {
		case SCI_VERSION_0:
			if (scir_test_resource(mgr, sci_vocab,
			                       VOCAB_RESOURCE_SCI0_MAIN_VOCAB)) {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					sciprintf("Resmgr: Detected SCI0\n");
					version = SCI_VERSION_0;
				}
			} else if (scir_test_resource(mgr, sci_vocab,
			                              VOCAB_RESOURCE_SCI1_MAIN_VOCAB)) {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					if (scir_test_resource(mgr, sci_vocab, 912)) {
						sciprintf("Resmgr: Running KQ1 or similar, using SCI0 resource encoding\n");
						version = SCI_VERSION_0;
					} else {
						version = SCI_VERSION_01;
						sciprintf("Resmgr: Detected SCI01\n");
					}
				}
			} else {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					sciprintf("Resmgr: Warning: Could not find vocabulary; assuming SCI0 w/o parser\n");
					version = SCI_VERSION_0;
				}
			}
			break;
		case SCI_VERSION_01_VGA_ODD:
			version = resmap_version;
			sciprintf("Resmgr: Detected Jones/CD or similar\n");
			break;
		case SCI_VERSION_1: {
			resource_t *res = scir_test_resource(mgr, sci_script, 0);

			mgr->sci_version = version = SCI_VERSION_1_EARLY;
			_scir_load_resource(mgr, res, 1);

			if (res->status == SCI_STATUS_NOMALLOC)
				mgr->sci_version = version = SCI_VERSION_1_LATE;

			/* No need to handle SCI 1.1 here - it was done in resource_map.c */
			break;
		}
		default:
			sciprintf("Resmgr: Warning: While autodetecting: Couldn't"
			          " determine SCI version!\n");
		}

	if (!resource_error) {
#if 0
		if (version <= SCI_VERSION_01)
			sci0_read_resource_patches(dir,
			                           &mgr->resources,
			                           &mgr->resources_nr);
		else
			sci1_read_resource_patches(dir,
			                           &mgr->resources,
			                           &mgr->resources_nr);
#endif

		qsort(mgr->resources, mgr->resources_nr, sizeof(resource_t),
		      resourcecmp); /* Sort resources */
	}

	mgr->sci_version = version;

	chdir(caller_cwd);
	free(caller_cwd);

	return mgr;
}

static void
_scir_free_altsources(resource_altsource_t *dynressrc) {
	if (dynressrc) {
		_scir_free_altsources(dynressrc->next);
		free(dynressrc);
	}
}

void
_scir_free_resources(resource_t *resources, int resources_nr) {
	int i;

	for (i = 0; i < resources_nr; i++) {
		resource_t *res = resources + i;

		_scir_free_altsources(res->alt_sources);

		if (res->status != SCI_STATUS_NOMALLOC)
			free(res->data);
	}

	free(resources);
}

void
scir_free_resource_manager(resource_mgr_t *mgr) {
	_scir_free_resources(mgr->resources, mgr->resources_nr);
	_scir_free_resource_sources(mgr->sources);
	mgr->resources = NULL;

	free(mgr);
}


static void
_scir_unalloc(resource_t *res) {
	free(res->data);
	res->data = NULL;
	res->status = SCI_STATUS_NOMALLOC;
}


static void
_scir_remove_from_lru(resource_mgr_t *mgr, resource_t *res) {
	if (res->status != SCI_STATUS_ENQUEUED) {
		sciprintf("Resmgr: Oops: trying to remove resource that isn't"
		          " enqueued\n");
		return;
	}

	if (res->next)
		res->next->prev = res->prev;
	if (res->prev)
		res->prev->next = res->next;
	if (mgr->lru_first == res)
		mgr->lru_first = res->next;
	if (mgr->lru_last == res)
		mgr->lru_last = res->prev;

	mgr->memory_lru -= res->size;

	res->status = SCI_STATUS_ALLOCATED;
}

static void
_scir_add_to_lru(resource_mgr_t *mgr, resource_t *res) {
	if (res->status != SCI_STATUS_ALLOCATED) {
		sciprintf("Resmgr: Oops: trying to enqueue resource with state"
		          " %d\n", res->status);
		return;
	}

	res->prev = NULL;
	res->next = mgr->lru_first;
	mgr->lru_first = res;
	if (!mgr->lru_last)
		mgr->lru_last = res;
	if (res->next)
		res->next->prev = res;

	mgr->memory_lru += res->size;
#if (SCI_VERBOSE_RESMGR > 1)
	fprintf(stderr, "Adding %s.%03d (%d bytes) to lru control: %d bytes total\n",
	        sci_resource_types[res->type], res->number, res->size,
	        mgr->memory_lru);

#endif

	res->status = SCI_STATUS_ENQUEUED;
}

static void
_scir_print_lru_list(resource_mgr_t *mgr) {
	int mem = 0;
	int entries = 0;
	resource_t *res = mgr->lru_first;

	while (res) {
		fprintf(stderr, "\t%s.%03d: %d bytes\n",
		        sci_resource_types[res->type], res->number,
		        res->size);
		mem += res->size;
		++entries;
		res = res->next;
	}

	fprintf(stderr, "Total: %d entries, %d bytes (mgr says %d)\n",
	        entries, mem, mgr->memory_lru);
}

static void
_scir_free_old_resources(resource_mgr_t *mgr, int last_invulnerable) {
	while (mgr->max_memory < mgr->memory_lru
	        && (!last_invulnerable || mgr->lru_first != mgr->lru_last)) {
		resource_t *goner = mgr->lru_last;
		if (!goner) {
			fprintf(stderr, "Internal error: mgr->lru_last is NULL!\n");
			fprintf(stderr, "LRU-mem= %d\n", mgr->memory_lru);
			fprintf(stderr, "lru_first = %p\n", (void *)mgr->lru_first);
			_scir_print_lru_list(mgr);
		}

		_scir_remove_from_lru(mgr, goner);
		_scir_unalloc(goner);
#ifdef SCI_VERBOSE_RESMGR
		sciprintf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n",
		          sci_resource_types[goner->type], goner->number,
		          goner->size);
#endif
	}
}

resource_t *
scir_find_resource(resource_mgr_t *mgr, int type, int number, int lock) {
	resource_t *retval;

	if (number >= sci_max_resource_nr[mgr->sci_version]) {
		int modded_number = number % sci_max_resource_nr[mgr->sci_version];
		sciprintf("[resmgr] Requested invalid resource %s.%d, mapped to %s.%d\n",
		          sci_resource_types[type], number,
		          sci_resource_types[type], modded_number);
		number = modded_number;
	}

	retval = scir_test_resource(mgr, type, number);

	if (!retval)
		return NULL;

	if (!retval->status)
		_scir_load_resource(mgr, retval, 0);

	else if (retval->status == SCI_STATUS_ENQUEUED)
		_scir_remove_from_lru(mgr, retval);
	/* Unless an error occured, the resource is now either
	** locked or allocated, but never queued or freed.  */

	if (lock) {
		if (retval->status == SCI_STATUS_ALLOCATED) {
			retval->status = SCI_STATUS_LOCKED;
			retval->lockers = 0;
			mgr->memory_locked += retval->size;
		}

		++retval->lockers;

	} else if (retval->status != SCI_STATUS_LOCKED) { /* Don't lock it */
		if (retval->status == SCI_STATUS_ALLOCATED)
			_scir_add_to_lru(mgr, retval);
	}

	_scir_free_old_resources(mgr, retval->status == SCI_STATUS_ALLOCATED);

	if (retval->data)
		return retval;
	else {
		sciprintf("Resmgr: Failed to read %s.%03d\n",
		          sci_resource_types[retval->type], retval->number);
		return NULL;
	}
}

void
scir_unlock_resource(resource_mgr_t *mgr, resource_t *res, int resnum, int restype) {
	if (!res) {
		sciprintf("Resmgr: Warning: Attempt to unlock non-existant"
		          " resource %s.%03d!\n",
		          sci_resource_types[restype], resnum);
		return;
	}

	if (res->status != SCI_STATUS_LOCKED) {
		sciprintf("Resmgr: Warning: Attempt to unlock unlocked"
		          " resource %s.%03d\n",
		          sci_resource_types[res->type], res->number);
		return;
	}

	if (!--res->lockers) { /* No more lockers? */
		res->status = SCI_STATUS_ALLOCATED;
		mgr->memory_locked -= res->size;
		_scir_add_to_lru(mgr, res);
	}

	_scir_free_old_resources(mgr, 0);
}

