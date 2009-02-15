/***************************************************************************
  Copyright (C) 2008 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantability,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <dynlink.h>
#ifdef HAVE_DYNAMIC_LINKING
#include <dlfcn.h>

void *
dynlink_open(const char *filename)
{
	void *retval = dlopen(filename, RTLD_NOW /* Quick error handling */
#ifdef RTLD_DEEPBIND
			      | RTLD_DEEPBIND /* Enforce deep binding, if possible */
#endif
			      );
	if (retval == NULL)
		fprintf(stderr, "[dynlink] %s: %s\n", filename, dlerror());

	return retval;
}

void *
dynlink_resolve(void *handle, const char *symname)
{
	return dlsym(handle, symname);
}

void
dynlink_close(void *handle)
{
	if (dlclose(handle))
		fprintf(stderr, "[dynlink] When unloading module: %s\n", filename, dlerror());
}

char *default_paths[] = {
	NULL
};

char **
dynlink_paths(void)
{
	return default_paths;
}

#endif /* !defined(HAVE_DLFCN_H_) */
