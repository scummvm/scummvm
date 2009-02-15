/***************************************************************************
  Copyright (C) 2007 Christoph Reichenbach


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

#include <stdio.h>
#include "conf_parse.h"

int
main(int argc, char **argv)
{
	int i;

	if (argc < 2) {
		printf("Usage: %s <conf-filename>+\n", argv[0]);
		return 1;
	}

	conf_parse_t *conf = NULL;

	for (i = 1; i < argc; i++) {
		conf_parse_t *newconf = conf_read_file(argv[i], 1, conf);
		if (newconf == NULL) {
			fprintf(stderr, "Couldn't parse ``%s''\n", argv[i]);
			conf_free_parse(conf);
			return 1;
		} else
			conf = newconf;
	}

	printf (">>> Parsing complete.  Re-emitting parsed files...\n");
	conf_write_parse(conf);
	printf (">>> Cleanup...\n");
	conf_free_parse(conf);

	return 0;
}
