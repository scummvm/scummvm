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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/file.h"
#include "common/compression/unzip.h"
#include "image/bmp.h"

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::loadAssets() {
	if (isDemo())
		loadAssetsDemo();
	else
		loadAssetsFullGame();
}

void FreescapeEngine::loadAssetsFullGame() {
	Common::File file;
	if (isAmiga()) {
		loadAssetsAmigaFullGame();
	} else if (isAtariST()) {
		loadAssetsAtariFullGame();
	} else if (isSpectrum()) {
		loadAssetsZXFullGame();
	} else if (isCPC()) {
		loadAssetsCPCFullGame();
	} else if (isC64()) {
		loadAssetsC64FullGame();
	} else if (isDOS()) {
		loadAssetsDOSFullGame();
	} else
		error("Invalid or unsupported render mode %s", Common::getRenderModeDescription(_renderMode));
}

void FreescapeEngine::loadAssetsDemo() {
	Common::File file;
	if (isAmiga()) {
		loadAssetsAmigaDemo();
	} else if (isAtariST()) {
		loadAssetsAtariDemo();
	} else if (isDOS()) {
		loadAssetsDOSDemo();
	} else if (isSpectrum()) {
		loadAssetsZXDemo();
	} else if (isCPC()) {
		loadAssetsCPCDemo();
	} else
		error("Unsupported demo");
}

void FreescapeEngine::loadAssetsAtariDemo() {
}

void FreescapeEngine::loadAssetsAmigaDemo() {
}

void FreescapeEngine::loadAssetsDOSDemo() {
}

void FreescapeEngine::loadAssetsZXDemo() {
}

void FreescapeEngine::loadAssetsCPCDemo() {
}

void FreescapeEngine::loadAssetsC64Demo() {
}


void FreescapeEngine::loadAssetsAtariFullGame() {
}

void FreescapeEngine::loadAssetsAmigaFullGame() {
}

void FreescapeEngine::loadAssetsDOSFullGame() {
}

void FreescapeEngine::loadAssetsZXFullGame() {
}

void FreescapeEngine::loadAssetsCPCFullGame() {
}

void FreescapeEngine::loadAssetsC64FullGame() {
}

void FreescapeEngine::loadDataBundle() {
	_dataBundle = Common::makeZipArchive(FREESCAPE_DATA_BUNDLE);
	if (!_dataBundle) {
		error("ENGINE: Couldn't load data bundle '%s'.", FREESCAPE_DATA_BUNDLE);
	}
	Common::Path versionFilename("version");
	if (!_dataBundle->hasFile(versionFilename))
		error("No version number in %s", FREESCAPE_DATA_BUNDLE);

	Common::SeekableReadStream *versionFile = _dataBundle->createReadStreamForMember(versionFilename);
	char *versionData = (char *)malloc((versionFile->size() + 1) * sizeof(char));
	versionFile->read(versionData, versionFile->size());
	versionData[versionFile->size()] = '\0';
	Common::String expectedVersion = "3";
	if (versionData != expectedVersion)
		error("Unexpected version number for freescape.dat: expecting '%s' but found '%s'", expectedVersion.c_str(), versionData);
	free(versionData);
}

Graphics::Surface *FreescapeEngine::loadBundledImage(const Common::String &name, bool appendRenderMode) {
	Image::BitmapDecoder decoder;
	Common::Path bmpFilename(name + ".bmp");

	if (appendRenderMode)
		bmpFilename = Common::Path(name + "_" + Common::getRenderModeDescription(_renderMode) + ".bmp");

	debugC(1, kFreescapeDebugParser, "Loading %s from bundled archive", bmpFilename.toString().c_str());
	if (!_dataBundle->hasFile(bmpFilename))
		error("Failed to open file %s from bundle", bmpFilename.toString().c_str());

	Common::SeekableReadStream *bmpFile = _dataBundle->createReadStreamForMember(bmpFilename);
	if (!decoder.loadStream(*bmpFile))
		error("Failed to decode bmp file %s from bundle", bmpFilename.toString().c_str());

	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*decoder.getSurface());
	decoder.destroy();
	return surface;
}

} // End of namespace Freescape
