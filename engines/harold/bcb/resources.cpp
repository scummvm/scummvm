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

#include "common/config-manager.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "image/bmp.h"

namespace BCB {

bool BCBEngine::initializeSearchPaths() {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	_programFiles.addDirectory(gameDataDir, 0, 1, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "program", 0, 1, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "bcb/program", 0, 2, false);
	for (int i = 0; i < 5; ++i)
		_discFiles[i].addSubDirectoryMatching(gameDataDir, Common::String::format("bcb%d", i + 1), 0, 8);

	return _programFiles.hasFile("check.bin");
}

Common::SeekableReadStream *BCBEngine::createProgramReadStream(const char *relativePath) const {
	return _programFiles.createReadStreamForMember(Common::Path(relativePath, '/'));
}

Common::SeekableReadStream *BCBEngine::createDiscReadStream(int discNumber, const char *relativePath) const {
	if (discNumber < 1 || discNumber > 5)
		return nullptr;

	return _discFiles[discNumber - 1].createReadStreamForMember(Common::Path(relativePath, '/'));
}

Common::SeekableReadStream *BCBEngine::createActiveDiscReadStream(const char *relativePath) const {
	return createDiscReadStream(CLIP<int>(_activeMediaDiscNumber, 1, 5), relativePath);
}

Common::SeekableReadStream *BCBEngine::createActiveDiscHighColorReadStream(const char *relativePath) const {
	Common::SeekableReadStream *stream = createActiveDiscReadStream(relativePath);
	if (stream)
		return stream;

	Common::String fallbackPath(relativePath);
	int marker = fallbackPath.find("/16m/");
	if (marker < 0)
		return nullptr;

	fallbackPath = fallbackPath.substr(0, marker) + "/256/" + fallbackPath.substr(marker + 5);
	return createActiveDiscReadStream(fallbackPath.c_str());
}

Graphics::Surface *BCBEngine::loadBitmapFromStream(Common::SeekableReadStream *stream, const char *debugName) const {
	Common::ScopedPtr<Common::SeekableReadStream> file(stream);
	if (!file)
		return nullptr;

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*file)) {
		warning("BCBEngine::loadBitmapFromStream(): Could not decode %s", debugName);
		return nullptr;
	}

	const Graphics::Surface *source = decoder.getSurface();
	if (!source)
		return nullptr;

	return source->convertTo(g_system->getScreenFormat(), decoder.getPalette().data(), decoder.getPalette().size());
}

const Graphics::Surface *BCBEngine::getUiBitmap(const char *fileName) {
	const bool isChip = !strcmp(fileName, "16mchip.bmp");
	Graphics::Surface *&bitmap = isChip ? _chipBitmap : _uiBitmap;
	if (bitmap && ((!isChip && _uiBitmapName != fileName) || bitmap->format != g_system->getScreenFormat())) {
		bitmap->free();
		delete bitmap;
		bitmap = nullptr;
	}
	if (!bitmap) {
		bitmap = loadBitmapFromStream(createProgramReadStream(fileName), fileName);
		if (!isChip)
			_uiBitmapName = fileName;
	}
	return bitmap;
}

Graphics::Surface *BCBEngine::loadBitmap(const char *fileName) {
	if (!strcmp(fileName, "16mmain.bmp") || !strcmp(fileName, "16mmainn.bmp") ||
		!strcmp(fileName, "16mmap.bmp") || !strcmp(fileName, "16msys.bmp")) {
		const Graphics::Surface *source = getUiBitmap(fileName);
		if (!source)
			return nullptr;
		Graphics::Surface *copy = new Graphics::Surface();
		copy->copyFrom(*source);
		return copy;
	}
	return loadBitmapFromStream(createProgramReadStream(fileName), fileName);
}

Graphics::Surface *BCBEngine::loadDiscBitmap(const char *relativePath) const {
	return loadBitmapFromStream(createActiveDiscHighColorReadStream(relativePath), relativePath);
}

Graphics::Surface *BCBEngine::loadDiscBitmapFromAnyDisc(const char *relativePath) const {
	for (int discNumber = 1; discNumber <= 5; ++discNumber) {
		Common::SeekableReadStream *stream = createDiscReadStream(discNumber, relativePath);
		if (stream)
			return loadBitmapFromStream(stream, relativePath);

		Common::String fallbackPath(relativePath);
		const int marker = fallbackPath.find("/16m/");
		if (marker >= 0) {
			fallbackPath = fallbackPath.substr(0, marker) + "/256/" + fallbackPath.substr(marker + 5);
			stream = createDiscReadStream(discNumber, fallbackPath.c_str());
			if (stream)
				return loadBitmapFromStream(stream, fallbackPath.c_str());
		}
	}

	return nullptr;
}

Common::SeekableReadStream *BCBEngine::createMovieReadStream(int movieId) const {
	const Common::String movieName = Common::String::format("mov%04d.avi", movieId);
	const Common::String moviePath = Common::String::format("movie/%s", movieName.c_str());
	return createActiveDiscReadStream(moviePath.c_str());
}

bool BCBEngine::switchActiveMediaDisc(int discNumber) {
	if (discNumber < 1 || discNumber > 5)
		return false;

	Common::ArchiveMemberList discMembers;
	if (_discFiles[discNumber - 1].listMembers(discMembers) == 0)
		return false;

	_activeMediaDiscNumber = discNumber;
	return true;
}

void BCBEngine::checkCurrentMediaPathReady() {
	if (stateDword(kStateClearDisplayBeforeMediaCheckFlag) != 0)
		clearDisplayAndResetState();

	const int requiredDiscNumber = stateDword(kStateRequiredDiscIndex) + 1;
	if (requiredDiscNumber == _activeMediaDiscNumber)
		return;

	if (requiredDiscNumber < 1 || requiredDiscNumber > 5) {
		warning("BCBEngine::checkCurrentMediaPathReady(): Invalid required media index %d from script state %u",
				requiredDiscNumber, stateDword(kStateRequiredDiscIndex));
		_scriptStopFlag = true;
		return;
	}

	Common::ArchiveMemberList requiredDiscMembers;
	if (_discFiles[requiredDiscNumber - 1].listMembers(requiredDiscMembers) == 0) {
		warning("BCBEngine::checkCurrentMediaPathReady(): Missing installed media directory bcb%d",
				requiredDiscNumber);
		debugC(1, kBCBDebugGeneral,
			   "BCB required media dump: active=bcb%d required=bcb%d scriptRequiredDiscIndex=%u",
			   _activeMediaDiscNumber, requiredDiscNumber, stateDword(kStateRequiredDiscIndex));
		_scriptStopFlag = true;
		return;
	}

	_currentDisplayState = 0;
	stopSceneIdleMovie();
	restoreActiveCaptionBackingRect();
	freeSceneChoiceBacking();
	_sceneChoiceActive = false;

	Graphics::Surface blackScreen;
	blackScreen.create(640, 480, g_system->getScreenFormat());
	blackScreen.fillRect(Common::Rect(0, 0, 640, 480), blackScreen.format.RGBToColor(0, 0, 0));
	blitScreenWithLineReveal(blackScreen);
	blackScreen.free();

	if (!switchActiveMediaDisc(requiredDiscNumber)) {
		warning("BCBEngine::checkCurrentMediaPathReady(): Failed to switch to installed media directory bcb%d", requiredDiscNumber);
		_scriptStopFlag = true;
		return;
	}

	stateDword(kStateMovieHistoryCount) = 0;
}

} // End of namespace BCB
