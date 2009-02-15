/***************************************************************************
 scriptdump.c Copyright (C) 2001 Christoph Reichenbach


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

#include <console.h>
#include <script.h>
#include <vocabulary.h>
#include <old_objects.h>
#include <stdio.h>
#include <stdlib.h>
#include <engine.h>
#include "sciunpack.h"

int script_dump()
{
	con_passthrough = 1;

	if(loadObjects(resmgr))
	{
		fprintf(stderr, "Unable to load object hierarchy\n");
		return 1;
	}

	printObject(object_root, SCRIPT_PRINT_METHODS|SCRIPT_PRINT_CHILDREN);
	return 0;
}
