/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"

#include "engines/engine.h"

#include "engines/myst3/myst3.h"
#include "engines/myst3/archive.h"

namespace Myst3 {


void Myst3Engine::dumpArchive(const char *fileName) {
	Common::File archiveFile;
	if (!archiveFile.open(fileName)) {
		error("Unable to open archive");
	}
	
	Archive archive;
	archive.readFromStream(archiveFile);
	archive.dumpDirectory();
	archive.dumpToFiles(archiveFile);
	
	archiveFile.close();
}

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst) {

}

Myst3Engine::~Myst3Engine() {

}

Common::Error Myst3Engine::run() {
	dumpArchive("MAISnodes.m3a");

	return Common::kNoError;
}

} // end of namespace Myst3
