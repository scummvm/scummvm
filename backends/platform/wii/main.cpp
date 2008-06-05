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
 */

#include <fat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "osystem.h"

#ifdef DEBUG_WII
#include <debug.h>
#include <gecko_console.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char *argv[]) {
	s32 res;

	VIDEO_Init();
	PAD_Init();
	AUDIO_Init(NULL);

#ifdef DEBUG_WII
	gecko_console_init(0);
	//DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
#endif

	printf("startup\n");

	if (!fatInitDefault()) {
		printf("fatInitDefault failed\n");
	} else {
		// set the default path if libfat couldnt set it
		// this allows loading over tcp/usbgecko
		char buf[MAXPATHLEN];

		getcwd(buf, MAXPATHLEN);
		if (!strcmp(buf, "fat:/"))
			chdir("/apps/scummvm");

		//fatEnableReadAhead(PI_DEFAULT, 32, 128);
	}

	g_system = new OSystem_Wii();
	assert(g_system);

	res = scummvm_main(argc, argv);
	g_system->quit();

	printf("shutdown\n");

	if (!fatUnmount(PI_DEFAULT)) {
		printf("fatUnmount failed\n");
		fatUnsafeUnmount(PI_DEFAULT);
	}

	printf("reloading\n");

	return res;
}

#ifdef __cplusplus
}
#endif

