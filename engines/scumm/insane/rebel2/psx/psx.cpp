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
 */

#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"
#include "scumm/insane/rebel2/psx/video.h"

namespace Scumm {

Rebel2PSX::Rebel2PSX(ScummEngine_v7 *vm) : _vm(vm) {
}

Common::SeekableReadStream *Rebel2PSX::openResource(int number) {
	const Common::Path path(Common::String::format("RESOURCE.%03d", number));
	Common::File *file = new Common::File();
	if (file->open(path))
		return file;
	delete file;
	return nullptr;
}

Common::SeekableReadStream *Rebel2PSX::openRawFile(const Common::Path &path, int discNumber) {
	Common::File *file = new Common::File();
	Common::Path extractedPath = path;
	if (discNumber == 2) {
		const Common::Path disc2Path = Common::Path("DISC2").join(path);
		if (file->open(disc2Path))
			extractedPath = disc2Path;
	}
	if (!file->isOpen() && !file->open(extractedPath)) {
		delete file;
		return nullptr;
	}
	if (file->size() % 2352 != 0) {
		delete file;
		return nullptr;
	}
	return file;
}

bool Rebel2PSX::playVideo(const Common::Path &path, int discNumber, bool version2) {
	Common::SeekableReadStream *stream = openRawFile(path, discNumber);
	if (!stream)
		return false;

	const RA2PSXStreamDecoder::FrameVersion frameVersion = version2 ?
			RA2PSXStreamDecoder::kVersion2 : RA2PSXStreamDecoder::kVersion3;
	RA2PSXStreamDecoder decoder(frameVersion);
	if (!decoder.loadStream(stream) || !decoder.setOutputPixelFormat(g_system->getScreenFormat())) {
		decoder.close();
		return false;
	}
	decoder.start();
	g_system->fillScreen(0);

	bool skipped = false;
	while (!_vm->shouldQuit() && !decoder.endOfVideo() && !skipped) {
		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				const int width = MIN<int>(frame->w, _vm->_screenWidth);
				const int height = MIN<int>(frame->h, _vm->_screenHeight);
				const int sourceX = (frame->w - width) / 2;
				const int sourceY = (frame->h - height) / 2;
				const int destX = (_vm->_screenWidth - width) / 2;
				const int destY = (_vm->_screenHeight - height) / 2;
				g_system->copyRectToScreen(frame->getBasePtr(sourceX, sourceY), frame->pitch,
						destX, destY, width, height);
				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
					(event.customType == kScummActionInsaneSkip ||
					 event.customType == kScummActionInsaneBack)) ||
					(event.type == Common::EVENT_KEYDOWN &&
					 event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
					event.type == Common::EVENT_LBUTTONUP)
				skipped = true;
			else if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				_vm->quitGame();
		}
		g_system->delayMillis(5);
	}
	decoder.close();
	return !_vm->shouldQuit();
}

bool Rebel2PSX::loadLevel1Assets(RA2PSXModel &model, RA2PSXModel &crosshair,
		RA2PSXModel &laser, RA2PSXLevel1UI &ui) {
	Common::SeekableReadStream *stream = openResource(1);
	if (!stream)
		return false;
	RA2PSXArchive archive;
	const bool loaded = archive.load(*stream);
	delete stream;
	if (!loaded)
		return false;

	Common::Array<byte> modelData;
	Common::Array<byte> crosshairData;
	Common::Array<byte> laserData;
	return archive.getMember("fOFS/TieFighter/main", modelData) && model.load(modelData) &&
			archive.getMember("fOFS/CrosshairW", crosshairData) && crosshair.load(crosshairData) &&
			archive.getMember("fOFS/WingLaser", laserData) && laser.load(laserData) &&
			ui.load(archive);
}

Common::Error Rebel2PSX::runGame() {
#ifdef USE_TINYGL
	RA2PSXModel model;
	RA2PSXModel crosshair;
	RA2PSXModel laser;
	RA2PSXLevel1UI ui;
	if (!loadLevel1Assets(model, crosshair, laser, ui))
		return Common::Error(Common::kReadingFailed,
				_("Could not load the PlayStation Level 1 resources"));

	if (!playVideo("S1/L01_INTR.STR", 1, false)) {
		if (_vm->shouldQuit())
			return Common::kNoError;
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 1 introduction"));
	}

	int lives = 3;
	int score = 0;
	while (!_vm->shouldQuit()) {
		const Level1Result result = playLevel1(model, crosshair, laser, ui, lives, score);
		if (result == kLevel1Quit)
			return Common::kNoError;
		if (result == kLevel1Error)
			return Common::Error(Common::kReadingFailed,
					_("Could not run the PlayStation Level 1 gameplay"));
		if (result == kLevel1Complete) {
			if (!playVideo("S1/L01_EXTR.STR", 1, false) && !_vm->shouldQuit())
				return Common::Error(Common::kReadingFailed,
						_("Could not play the PlayStation Level 1 ending"));
			return Common::kNoError;
		}

		if (!playVideo("S1/L01_DIE.STR", 1, false)) {
			if (_vm->shouldQuit())
				return Common::kNoError;
			return Common::Error(Common::kReadingFailed,
					_("Could not play the PlayStation Level 1 death video"));
		}
		if (--lives == 0) {
			if (!playVideo("S1/L01_OVER.STR", 1, false) && !_vm->shouldQuit())
				return Common::Error(Common::kReadingFailed,
						_("Could not play the PlayStation game-over video"));
			return Common::kNoError;
		}
	}
	return Common::kNoError;
#else
	return Common::Error(Common::kUnsupportedGameidError,
			_s("Rebel Assault II PlayStation support requires TinyGL"));
#endif
}

} // End of namespace Scumm
