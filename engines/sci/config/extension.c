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
#include "conf_extension.h"
#include "sci_memory.h"

int
conf_extension_supercedes(conf_extension_t *a, conf_extension_t *b)
{
	return 0;
}

void
conf_extension_print(FILE *file, conf_extension_t *a)
{
	switch (a->type) {

	case CONF_EXT_TYPE_GFX:
		fputs(a->data, file);
		break;

	default:
		fprintf(stderr, "[conf] Unknown config extension type %d", a->type);
	}
}

void
conf_extension_free(conf_extension_t *a)
{
	switch (a->type) {

	case CONF_EXT_TYPE_GFX:
		sci_free(a->data);
		break;

	default:
		fprintf(stderr, "[conf] Unknown config extension type %d", a->type);
	}

	a->type = CONF_EXT_TYPE_INVALID;
	a->data = NULL;
	sci_free(a);
}
