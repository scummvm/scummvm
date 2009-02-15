/***************************************************************************
 modules.c Copyright (C) 2001 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#if 0
#ifndef __BEOS__

#include <sci_memory.h>
#include <modules.h>
#include <dlfcn.h>


static sci_module_t *
_sci_try_open_module(char *filename, char *path, char *struct_name, void **handle) {
	char *fullname = sci_malloc(strlen(path) + strlen(DIR_SEPARATOR_STR)
	                            + strlen(filename));
	sci_module_t *module;
	fprintf(stderr, "Trying module %s at %s\n", filename, path);
	strcpy(fullname, path);
	strcat(fullname, DIR_SEPARATOR_STR);
	strcat(fullname, filename);

	fprintf(stderr, "Total name is %s\n", fullname);
	*handle = dlopen(fullname, RTLD_NOW);
	fprintf(stderr, "Could not open because: %s\n", dlerror());
	free(fullname);

	if (!*handle)
		return NULL;

	module = (sci_module_t *) dlsym(*handle, struct_name);
	if (!module)
		fprintf(stderr, "%s: Failed to find symbol '%s'.\n",
		        fullname, struct_name);

	return module;
}

void *
sci_find_module(char *path, char *name, char *type, char *struct_prefix,
                char *file_suffix, int magic, int version, void **handle) {
	char *module_name = sci_malloc(strlen(type) + strlen(DIR_SEPARATOR_STR)
	                               + strlen(name) + strlen(file_suffix)
	                               + strlen(MODULE_NAME_SUFFIX) + 1);
	char *struct_name = sci_malloc(strlen(struct_prefix) + strlen(name) + 1);
	char *dir_end;
	char *path_pos = path;
	char path_separator = PATH_SEPARATOR_STR[0];
	sci_module_t *module = NULL;

	strcpy(module_name, type);
	strcat(module_name, DIR_SEPARATOR_STR);
	strcat(module_name, name);
	strcat(module_name, file_suffix);
	strcat(module_name, MODULE_NAME_SUFFIX);

	strcpy(struct_name, struct_prefix);
	strcat(struct_name, name);

	do {
		dir_end = strchr(path_pos, path_separator);
		if (dir_end)
			*dir_end = 0;

		module = _sci_try_open_module(module_name, path_pos,
		                              struct_name, handle);

		if (module) {
			if (module->class_magic != magic) {
				fprintf(stderr, "%s at %s is not a %s module, skipping...\n",
				        module_name, path_pos, type);
				dlclose(*handle);
				module = NULL;
			} else if (module->class_version != version) {
				fprintf(stderr, "%s at %s has %s module version %d,"
				        " expected %d- skipping...\n",
				        module_name, path_pos, type, module->class_version,
				        version);
				dlclose(*handle);
				module = NULL;
			}
		}

		if (dir_end) {
			*dir_end = path_separator;
			path_pos = dir_end + 1;
		}

	} while (!module && dir_end);

	if (!module) {
		*handle = NULL;
		fprintf(stderr, "%s module '%s' not found in path '%s'.\n",
		        type, name, path);
	} else {
		if (dir_end)
			*dir_end = 0;

		printf("Using %s driver '%s', version %s, from '%s'.\n",
		       type, module->module_name, module->module_version,
		       path_pos);

		if (dir_end)
			*dir_end = path_separator;
	}

	free(module_name);
	free(struct_name);

	return (void *) module;
}


void
sci_close_module(void *module, char *type, char *name) {
	if (!module)
		return;

	if (dlclose(module)) {
		fprintf(stderr, "Error while closing %s module '%s': %s\n",
		        type, name, dlerror());
	}
}

#endif /* !__BEOS__ */

#endif /* 0 */
