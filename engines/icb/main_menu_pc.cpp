/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/p4.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/cluster_manager_pc.h"

#include "common/system.h"

namespace ICB {

void Main_menu() {
	if (!g_theOptionsManager->HasControl()) {
		// Initialise
		g_theOptionsManager->StartMainOptions();
	} else {
		// OptionsManager is in control
		g_theOptionsManager->CycleLogic();
		stub.Update_screen();
	}
}

void Pause_menu() {
	// OptionsManager is in control
	if (gRegainedFocus)
		g_theOptionsManager->ForceInGameScreenRefresh();

	g_theOptionsManager->CycleLogic();
	stub.Update_screen();
}

void Credits() {
	uint32 t = GetMicroTimer();
	g_theOptionsManager->DoCredits();
	stub.Update_screen();
	t = GetMicroTimer() - t;

	// As the DoCredits() function takes a variable time this timing code
	// forces an constant frame rate for smoother scrolling
	int32 r = 40000 - t;
	if (t < 40000) {
		g_system->delayMillis(r / 1000);
	}
}

void InitisliaseScrollingText(const char *textFileName, const char *movieFileName, int frameStart) {
	// Movies are streamed from the CD
	char *_root;

#if 1 // was ifdef FROM_PC_CD
#ifndef PC_DEMO

	_root = g_theClusterManager->GetCDRoot();

#endif
#endif

	// Find movie to play
	pxString fullMovieName;

#ifdef PC_DEMO
	// All in one directory, which is nice
	fullMovieName.Format("gmovies\\%s.bik", movieFileName);
	fullMovieName.ConvertPath();
#else
	// All in one directory, which is nice
	fullMovieName.Format("movies\\%s.bik", movieFileName);
	fullMovieName.ConvertPath();
#endif

	// Ensure correct CD is in the drive (can't assume this because of movie library)
	switch (movieFileName[2]) {
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

	default: // Default look in the global directory
		g_theClusterManager->CheckAnyDiscInserted();
		break;
	}

	// Did we find it in the mission directory (or possibly global directory already)
	if (!checkFileExists(fullMovieName)) { // amode = 0
		// File is not present in the mission directory so check the global directory

		fullMovieName.Format("gmovies\\%s.bik", movieFileName);
		fullMovieName.ConvertPath();

		if (!checkFileExists(fullMovieName))
			Fatal_error(pxVString("Movie %s.bik does not exist in mission or global movie directory", movieFileName));
	}

	g_theOptionsManager->InitialiseScrollingText(textFileName, fullMovieName, frameStart);
}

void ScrollingText() {
	uint32 t = GetMicroTimer();
	g_theOptionsManager->DoScrollingText();
	stub.Update_screen();
	t = GetMicroTimer() - t;

#ifdef PC_DEMO
	int32 r = 80000 - t;
	if (t < 80000) {
		g_system->delayMillis(r / 1000);
	}
#else
	int32 r = 40000 - t;
	if (t < 40000) {
		g_system->delayMillis(r / 1000);
	}
#endif
}

void Gameover_menu() {
	if (!g_theOptionsManager->HasControl()) {
		// Initialise
		g_theOptionsManager->StartGameOverOptions();
	} else {
		// OptionsManager is in control
		if (gRegainedFocus)
			g_theOptionsManager->ForceInGameScreenRefresh();

		g_theOptionsManager->CycleLogic();
		stub.Update_screen();
	}
}

void GameCompleted() {
	px.game_completed = TRUE8;
	// Fill in any gaps in the movie library
	g_theOptionsManager->UnlockMovies();
}

} // End of namespace ICB
