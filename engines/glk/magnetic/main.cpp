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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/magnetic/defs.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Glk {
namespace Magnetic {

#define WIDTH 78

static type8 buffer[80], xpos = 0, bufpos = 0, log_on = 0, ms_gfx_enabled, filename[256];
static Common::DumpFile *log1 = 0, *log2 = 0;

type8 ms_load_file(const char *name, type8 *ptr, type16 size) {
	assert(name);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(name);
	if (!file)
		return 1;

	if (file->read(ptr, size) != size) {
		delete file;
		return 1;
	}

	delete file;
	return 0;
}

type8 ms_save_file(const char *name, type8 *ptr, type16 size) {
	assert(name);
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(name);
	if (!file)
		return 1;

	if (file->write(ptr, size) != size) {
		delete file;
		return 1;
	}

	file->finalize();
	delete file;
	return 0;
}

void script_write(type8 c) {
	if (log_on == 2)
		log1->writeByte(c);
}

void transcript_write(type8 c) {
	if (log2)
		log2->writeByte(c);
}

void ms_fatal(const char *txt) {
	error("Fatal error: %s", txt);
}

#if 0
main(int argc, char **argv) {
	type8 running, i, *gamename = 0, *gfxname = 0, *hintname = 0;
	type32 dlimit, slimit;

	if (sizeof(type8) != 1 || sizeof(type16) != 2 || sizeof(type32) != 4) {
		fprintf(stderr,
		        "You have incorrect typesizes, please edit the typedefs and recompile\n"
		        "or proceed on your own risk...\n");
		exit(1);
	}
	dlimit = slimit = 0xffffffff;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (tolower(argv[i][1])) {
			case 'd':
				if (strlen(argv[i]) > 2)
					dlimit = atoi(&argv[i][2]);
				else
					dlimit = 0;
				break;
			case 's':
				if (strlen(argv[i]) > 2)
					slimit = atoi(&argv[i][2]);
				else
					slimit = 655360;
				break;
			case 't':
				if (!(log2 = fopen(&argv[i][2], "w")))
					printf("Failed to open \"%s\" for writing.\n", &argv[i][2]);
				break;
			case 'r':
				if (log1 = fopen(&argv[i][2], "r"))
					log_on = 1;
				else
					printf("Failed to open \"%s\" for reading.\n", &argv[i][2]);
				break;
			case 'w':
				if (log1 = fopen(&argv[i][2], "w"))
					log_on = 2;
				else
					printf("Failed to open \"%s\" for writing.\n", &argv[i][2]);
				break;
			default:
				printf("Unknown option -%c, ignoring.\n", argv[i][1]);
				break;
			}
		} else if (!gamename)
			gamename = argv[i];
		else if (!gfxname)
			gfxname = argv[i];
		else if (!hintname)
			hintname = argv[i];
	}
	if (!gamename) {
		printf("Magnetic 2.3 - a Magnetic Scrolls interpreter\n\n");
		printf("Usage: %s [options] game [gfxfile] [hintfile]\n\n"
		       "Where the options are:\n"
		       " -dn    activate register dump (after n instructions)\n"
		       " -rname read script file\n"
		       " -sn    safety mode, exits automatically (after n instructions)\n"
		       " -tname write transcript file\n"
		       " -wname write script file\n\n"
		       "The interpreter commands are:\n"
		       " #undo   undo - don't use it near are_you_sure prompts\n"
		       " #logoff turn off script writing\n\n", argv[0]);
		exit(1);
	}

	if (!(ms_gfx_enabled = ms_init(gamename, gfxname, hintname, 0))) {
		printf("Couldn't start up game \"%s\".\n", gamename);
		exit(1);
	}
	ms_gfx_enabled--;
	running = 1;
	while ((ms_count() < slimit) && running) {
		if (ms_count() >= dlimit)
			ms_status();
		running = ms_rungame();
	}
	if (ms_count() == slimit) {
		printf("\n\nSafety limit (%d) reached.\n", slimit);
		ms_status();
	}
	ms_freemem();
	if (log_on)
		fclose(log1);
	if (log2)
		fclose(log2);
	printf("\nExiting.\n");
	return 0;
}
#endif

} // End of namespace Magnetic
} // End of namespace Glk
