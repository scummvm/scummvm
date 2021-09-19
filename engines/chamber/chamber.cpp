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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "chamber/chamber.h"

namespace Chamber {

ChamberEngine *g_vm;

ChamberEngine::ChamberEngine(OSystem *syst)
	: Engine(syst) {
	g_vm = this;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("chamber");

	_shouldQuit = false;
	_pxiData = NULL;
}

ChamberEngine::~ChamberEngine() {
	// Dispose your resources here
	delete _rnd;
	delete[] _pxiData;
}

bool ChamberEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error ChamberEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error ChamberEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void ChamberEngine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int16 dummy = 0;
	s.syncAsUint16LE(dummy);
}

} // End of namespace Chamber
