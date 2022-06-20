/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

//#include <vld.h>

#include "hpl1/penumbra-overture/Init.h"

#if 0//def WIN32
#include <windows.h>
#endif

int hplMain(const tString &asCommandLine) {
	cInit *pInit = hplNew(cInit, ());

	bool bRet = pInit->Init(asCommandLine);

	if (bRet == false) {
		hplDelete(pInit->mpGame);
		CreateMessageBoxW(_W("Error!"), pInit->msErrorMessage.c_str());
		OpenBrowserWindow(_W("http://support.frictionalgames.com"));
		return 1;
	}

	pInit->Run();

	pInit->Exit();

	hplDelete(pInit);

	cMemoryManager::LogResults();

	return 0;
}
