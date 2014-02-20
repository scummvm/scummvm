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

#include <FBase.h>
#include <FApp.h>
#include <FSystem.h>

#include "backends/platform/tizen/application.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Collection;

/**
 * The entry function of tizen application called by the operating system.
 */
extern "C" _EXPORT_ int OspMain(int argc, char *pArgv[]) {
	result r = E_SUCCESS;

	AppLog("Application started.");
	ArrayList args(SingleObjectDeleter);
	args.Construct();
	for (int i = 0; i < argc; i++) {
		args.Add(new (std::nothrow) String(pArgv[i]));
	}

	r = Tizen::App::UiApp::Execute(TizenScummVM::createInstance, &args);
	TryLog(r == E_SUCCESS, "[%s] Application execution failed", GetErrorMessage(r));
	AppLog("Application finished.");

	return static_cast<int>(r);
}

