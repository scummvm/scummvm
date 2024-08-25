/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/common/ptr_util.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/movie_pc.h"

#include "common/util.h"

namespace ICB {

int32 Movie_name_to_ID(char *name);
void Init_play_movie(const char *param0, bool8 param1);

mcodeFunctionReturnCodes fn_play_movie(int32 &result, int32 *params) { return (MS->fn_play_movie(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_play_movie(int32 &, int32 *params) {
	const char *movie_name = nullptr;
	if (params && params[0]) {
		movie_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	}

	// Are we free to begin movie playback
	if (!L->looping) {
		Init_play_movie(movie_name, (bool8)params[1]);

		// Cycle calls to this function until playback has finished
		L->looping = 1;

		return (IR_REPEAT);
	} else {
		// If we're already playing a movie then loop through this function until we're done
		if (g_theSequenceManager->busy())
			return (IR_REPEAT);
		else {
			// Movie finished so continue script
			L->looping = 0;
			return (IR_CONT);
		}
	}
}

void Init_play_movie(const char *param0, bool8 param1) {
	// Stop all sounds occurring
	PauseSounds();

	const char *moviename = (const char *)param0;

	// Filename checking to help catch Jake's PSX restrictions
	if (strlen(moviename) > 8)
		Fatal_error("Movie stream name must not exceed 8 characters in length!!!");

	for (uint32 i = 0; i < strlen(moviename); i++) {
		if (!Common::isAlnum(moviename[i]))
			Fatal_error(pxVString("Can't register movie: %s as filename must consist of alpha-numerics ONLY.", moviename));
		if (!Common::isDigit(moviename[i]))
			if (!Common::isLower(moviename[i]))
				Fatal_error(pxVString("Can't register movie: %s as filename must consist of lowercase characters ONLY.", moviename));
	}

	pxString fullname;

#ifndef PC_DEMO
#if 1 // was #ifdef FROM_PC_CD
	// Non-global movies are streamed from the CD
	// char *_root = g_theClusterManager->GetCDRoot();
#endif
#endif

	// All in one directory, which is nice
	fullname.Format("movies\\%s.bik", moviename);
	fullname.ConvertPath();
	// Ensure correct CD is in the drive (can't assume this because of movie library)
	switch (moviename[2]) {
	case '1':
		g_theClusterManager->CheckDiscInserted(MISSION1);
		break;
	case '2':
		g_theClusterManager->CheckDiscInserted(MISSION2);
		break;
	case '3':
		g_theClusterManager->CheckDiscInserted(MISSION3);
		break;
	case '4':
		g_theClusterManager->CheckDiscInserted(MISSION4);
		break;
	case '5':
		g_theClusterManager->CheckDiscInserted(MISSION5);
		break;
	case '7':
		g_theClusterManager->CheckDiscInserted(MISSION7);
		break;
	case '8':
		g_theClusterManager->CheckDiscInserted(MISSION8);
		break;
	case '9':
		g_theClusterManager->CheckDiscInserted(MISSION9);
		break;
	case '0':
		g_theClusterManager->CheckDiscInserted(MISSION10);
		break;
	}

	// Did we find it in the mission directory (or possibly global directory already)
	if (!checkFileExists(fullname)) {
		// File is not present in the mission directory so check the global directory

		fullname.Format("gmovies\\%s.bik", moviename);
		fullname.ConvertPath();

		if (!checkFileExists(fullname))
			Fatal_error(pxVString("Movie %s.bik does not exist in mission or global movie directory", moviename));
	}

	if (g_theSequenceManager->registerMovie(fullname, param1, FALSE8)) {
		// Is this movie part of the title screen library
		int32 mvid = Movie_name_to_ID(const_cast<char *>(moviename));

		if (mvid != -1) {
			// We can now consider this movie viewable in the title screen library
			g_movieLibrary[mvid].visible = TRUE8;
		}

		// Bink is now active and playing

		// Successfully opened a bink sequence so set the engine to play and display it
		g_stub->Push_stub_mode(__sequence);
	} else {
		Fatal_error(pxVString("Couldn't register the movie: %s", (const char *)fullname));
	}
}

} // End of namespace ICB
