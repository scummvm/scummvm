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

#include "conf_parse.h"
#include "conf_subsystems.h"


int error_count = 0;

#define ASSERT(x) if (!(x)) { ++error_count; fprintf(stderr, "[error] Assertion failed at line %d: " #x "\n", __LINE__);}

char ** dynamic_driver_locations = {
	NULL
};

char *fsci_driver_magic = CONF_DRIVER_MAGIC;

static conf_main_t main_drv = {
	.header = {
		.magic = CONF_DRIVER_MAGIC;
		.freesci_version = CONF_DRIVER_VERSION;
		.subsystem_id = 0;
		.subsystem_version = 0;
		.name = "config-test";
		.version = "1.0";
		.dependencies = CONF_DRIVER_DEPENDENCY_GFX; /* Get SFX dependency transitively from GFX */
		.options = main_options;
		.set_option = 
	};
	.subsystems = { &sub_gfx, &sub_sound,
			&sub_ignore, &sub_ignore, &sub_ignore,
			&sub_ignore, &sub_ignore };
	.dynamic_driver_locations = dynamic_driver_locations;
	.init = init_main;
	.exit = exit_main;
};

void
test_validation(conf_parse_t *parse)
{
	ASSERT (NULL == conf_validate (&main_drv, parse));
}

void
test_

int
main(int argc, char **argv)
{
	int i;

	conf_parse_t *conf = = conf_read_file("option-test.conf", 1, conf);

	printf (">>> Parsing complete.\n");
	ASSERT(conf != NULL);
	if (!conf)
		return 1;
	conf_write_parse(conf);

	test_validation(parse);
	test_setting(pase);

	printf (">>> Cleanup...\n");
	conf_free_parse(conf);

	return error_count != 0;
}
