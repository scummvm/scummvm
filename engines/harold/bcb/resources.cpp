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
#include "image/pict.h"

namespace BCB {

bool BCBEngine::initializeSearchPaths() {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	_programFiles.addDirectory(gameDataDir, 0, 1, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "program", 0, 1, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "bcb/program", 0, 2, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "pict", 0, 1, false);
	_programFiles.addSubDirectoryMatching(gameDataDir, "BLUE CHICAGO BLUES/pict", 0, 1, false);
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

Common::SeekableReadStream *BCBEngine::createDiscImageReadStream(int discNumber, const char *relativePath, Common::String &resolvedPath, bool searchAllDiscs) const {
	resolvedPath = relativePath;
	if (discNumber < 1 || discNumber > 5)
		return nullptr;

	Common::String fallbackPath(relativePath);
	fallbackPath.toLowercase();
	if (!fallbackPath.hasPrefix("bmp/") || !fallbackPath.hasSuffix(".bmp")) {
		for (int offset = 0; offset < (searchAllDiscs ? 5 : 1); ++offset) {
			Common::SeekableReadStream *stream = createDiscReadStream((discNumber - 1 + offset) % 5 + 1, relativePath);
			if (stream)
				return stream;
		}
		return nullptr;
	}

	const int marker = fallbackPath.find("/16m/");
	Common::String pictPath = fallbackPath;
	if (marker >= 0)
		pictPath = pictPath.substr(0, marker) + pictPath.substr(marker + 4);
	pictPath = "pict/" + pictPath.substr(4, pictPath.size() - 8) + ".pic";
	Common::String paths[3] = {relativePath, pictPath, ""};
	const int pathCount = marker >= 0 ? 3 : 2;
	const int discCount = marker >= 0 || searchAllDiscs ? 5 : 1;
	if (marker >= 0)
		paths[2] = fallbackPath.substr(0, marker) + "/256/" + fallbackPath.substr(marker + 5);

	for (int pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		resolvedPath = paths[pathIndex];
		for (int offset = 0; offset < discCount; ++offset) {
			const int sourceDisc = (discNumber - 1 + offset) % 5 + 1;
			Common::SeekableReadStream *stream = createDiscReadStream(sourceDisc, resolvedPath.c_str());
			if (stream)
				return stream;
		}
	}
	return nullptr;
}

Graphics::Surface *BCBEngine::loadImageFromStream(Common::SeekableReadStream *stream, const char *fileName) const {
	Common::ScopedPtr<Common::SeekableReadStream> file(stream);
	if (!file)
		return nullptr;

	Common::String name(fileName);
	name.toLowercase();
	Common::ScopedPtr<Image::ImageDecoder> decoder;
	if (name.hasSuffix(".pic") || name.hasSuffix(".pict"))
		decoder.reset(new Image::PICTDecoder());
	else
		decoder.reset(new Image::BitmapDecoder());
	if (!decoder->loadStream(*file)) {
		warning("BCBEngine::loadImageFromStream(): Could not decode %s", fileName);
		return nullptr;
	}

	const Graphics::Surface *source = decoder->getSurface();
	if (!source)
		return nullptr;

	return source->convertTo(g_system->getScreenFormat(), decoder->getPalette().data(), decoder->getPalette().size());
}

Graphics::Surface *BCBEngine::loadProgramBitmap(const char *fileName) const {
	Common::SeekableReadStream *stream = createProgramReadStream(fileName);
	if (stream)
		return loadImageFromStream(stream, fileName);

	static const struct {
		const char *bitmap;
		const char *picture;
	} imageNames[] = {
		{"16mchip.bmp", "PCHIP.PIC"},
		{"16mlogo.bmp", "PRSLOGO.PIC"},
		{"16mmain.bmp", "PMAIN.PIC"},
		{"16mmainn.bmp", "PMAIN_N.PIC"},
		{"16mmap.bmp", "PMAP.PIC"},
		{"16msys.bmp", "PSYSTEM.PIC"},
		{"16mtitle.bmp", "PTITLE.PIC"},
		{"256com.bmp", "PCOM.PIC"}
	};
	const Common::String requestedName(fileName);
	for (uint i = 0; i < ARRAYSIZE(imageNames); ++i) {
		if (!requestedName.equalsIgnoreCase(imageNames[i].bitmap))
			continue;
		stream = createProgramReadStream(imageNames[i].picture);
		if (!stream && requestedName.equalsIgnoreCase("256com.bmp")) {
			for (int disc = 1; disc <= 5 && !stream; ++disc)
				stream = createDiscReadStream(disc, "pict/comp/pcom.pic");
		}
		return loadImageFromStream(stream, imageNames[i].picture);
	}
	return nullptr;
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
		bitmap = loadProgramBitmap(fileName);
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
	return loadProgramBitmap(fileName);
}

Graphics::Surface *BCBEngine::loadDiscBitmap(const char *relativePath) const {
	Common::String resolvedPath;
	Common::SeekableReadStream *stream = createDiscImageReadStream(CLIP<int>(_activeMediaDiscNumber, 1, 5), relativePath, resolvedPath);
	return loadImageFromStream(stream, resolvedPath.c_str());
}

Graphics::Surface *BCBEngine::loadDiscBitmapFromAnyDisc(const char *relativePath) const {
	Common::String resolvedPath;
	Common::SeekableReadStream *stream = createDiscImageReadStream(CLIP<int>(_activeMediaDiscNumber, 1, 5), relativePath, resolvedPath, true);
	return loadImageFromStream(stream, resolvedPath.c_str());
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
