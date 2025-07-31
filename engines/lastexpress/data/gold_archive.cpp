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

#include "lastexpress/lastexpress.h"
#include "lastexpress/data/gold_archive.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/compression/unzip.h"
#include "image/jpeg.h"

namespace LastExpress {

GoldArchiveManager::GoldArchiveManager(LastExpressEngine *engine) : ArchiveManager(engine) {
}

GoldArchiveManager::~GoldArchiveManager() {
}

bool GoldArchiveManager::lockCD(int32 index) {
	return true;
}

bool GoldArchiveManager::isCDAvailable(int cdNum, char *outPath, int pathSize) {
	return true;
}

bool GoldArchiveManager::lockCache(char *filename) {
	return true;
}

void GoldArchiveManager::initHPFS() {
	// This will act as a cache of files
	_hdArchiveNumFiles = 6000;
	_hdArchive = new HPF[_hdArchiveNumFiles];
	
	Common::StringArray resCategories = {
		"BG", "DATA", "LNK", "NIS",
		"SBE", "SEQ", "SND", "TGA"
	};

	for (uint i = 0; i < resCategories.size(); i++) {
		Common::Archive *tmpArch = Common::makeZipArchive(Common::Path(Common::String::format("%s.zip", resCategories[i].c_str())), false);

		if (tmpArch) {
			SearchMan.add(resCategories[i], tmpArch);
		} else {
			warning("GoldArchiveManager::initHPFS(): %s.zip could't be loaded, the game might not function properly", resCategories[i].c_str());
		}
	}
}

void GoldArchiveManager::shutDownHPFS() {
	delete[] _hdArchive;
}

HPF *GoldArchiveManager::openHPF(const char *filename) {
	HPF *result = nullptr;
	for (int32 i = 0; i < _hdArchiveNumFiles; i++) {
		if ((_hdArchive[i].status & kHPFFileIsLoaded) == 0) {
			result = &_hdArchive[i];
			break;
		}
	}

	if (!result) {
		error("GoldArchiveManager::openHPF(): Couldn't allocate cache for file %s", filename);
	}
	
	Common::String filenameStr(filename);
	filenameStr.toUppercase();

	Common::ArchiveMemberPtr archiveItem = SearchMan.getMember(Common::Path(filenameStr));

	if (!archiveItem.get()) {
		archiveItem = SearchMan.getMember(Common::Path(Common::Path(Common::String::format("COMMON/%s", filenameStr.c_str()))));
	}

	if (!archiveItem.get()) {
		archiveItem = SearchMan.getMember(Common::Path(Common::Path(Common::String::format("%s/%s", _languagePrefix.c_str(), filenameStr.c_str()))));
	}

	if (!archiveItem.get()) {
		for (int i = 1; i <= 3; i++) {
			archiveItem = SearchMan.getMember(Common::Path(Common::Path(Common::String::format("%s/CD%d/%s", _languagePrefix.c_str(), i, filenameStr.c_str()))));
			if (archiveItem.get())
				break;

			archiveItem = SearchMan.getMember(Common::Path(Common::Path(Common::String::format("CD%d/%s", i, filenameStr.c_str()))));
			if (archiveItem.get())
				break;
		}
	}

	if (!archiveItem.get()) {
		warning("GoldArchiveManager::openHPF(): Can't find file %s", filename);
		return nullptr;
	}

	Common::strcpy_s(result->name, filename);
	int64 archiveSize = archiveItem.get()->createReadStream()->size();
	result->size = (uint16)((archiveSize / MEM_PAGE_SIZE) + ((archiveSize % MEM_PAGE_SIZE) > 0 ? 1 : 0));
	result->status |= kHPFFileIsLoaded;
	result->currentPos = 0;
	result->archiveName = archiveItem.get()->getPathInArchive().toString();
	result->archiveRef = archiveItem;

	return result;
}

void GoldArchiveManager::readHPF(HPF *archive, void *dstBuf, uint32 size) {
	if (!archive)
		return;

	uint32 effSize;

	if ((archive->status & kHPFFileIsLoaded) != 0) {
		if (archive->size > archive->currentPos) {
			effSize = size;
			if (archive->currentPos + size > archive->size)
				effSize = archive->size - archive->currentPos;

			Common::ArchiveMember *archivePtr = archive->archiveRef.get();
			assert(archivePtr);

			Common::SeekableReadStream *readStream = archivePtr->createReadStream();

			readStream->seek((archive->currentPos + archive->offset) * MEM_PAGE_SIZE, SEEK_SET);
			readStream->read(dstBuf, effSize * MEM_PAGE_SIZE);

			delete readStream;

			archive->currentPos += effSize;
		}
	}
}

int GoldArchiveManager::loadBG(const char *filename) {
	TBM tbm;
	char bgFilename[84];

	memset(bgFilename, 0, sizeof(bgFilename));

	tbm.x = _engine->getGraphicsManager()->_renderBox1.x;
	tbm.y = _engine->getGraphicsManager()->_renderBox1.y;
	tbm.width = _engine->getGraphicsManager()->_renderBox1.width;
	tbm.height = _engine->getGraphicsManager()->_renderBox1.height;

	PixMap *bgSurface = _engine->getGraphicsManager()->_frontBuffer;

	if (_engine->getLogicManager()->_doubleClickFlag &&
		(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
		_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
		return -1;
	}

	Common::strcpy_s(bgFilename, filename);
	Common::strcat_s(bgFilename, ".bg");

	HPF *archive = openHPF(bgFilename);

	if (!archive) {
		Common::strcpy_s(bgFilename, "DEFAULT.BG");
		archive = openHPF(bgFilename);
	}

	if (archive) {
		// The background format in the Gold Edition is basically JPEG but with an header on top, containing:
		// - X coordinate
		// - Y coordinate
		// - Width
		// - Height
		//
		// All these fields are 32-bit LE.

		Image::JPEGDecoder *dec = new Image::JPEGDecoder();
		byte *backgroundCompBuffer = (byte *)malloc(archive->size * MEM_PAGE_SIZE);
		assert(backgroundCompBuffer);
		readHPF(archive, backgroundCompBuffer, archive->size);
		Common::SeekableReadStream *seqDataStream = new Common::MemoryReadStream(backgroundCompBuffer, MEM_PAGE_SIZE * archive->size, DisposeAfterUse::YES);

		_engine->getGraphicsManager()->_renderBox1.x = seqDataStream->readUint32LE();
		_engine->getGraphicsManager()->_renderBox1.y = seqDataStream->readUint32LE();
		_engine->getGraphicsManager()->_renderBox1.width = seqDataStream->readUint32LE();
		_engine->getGraphicsManager()->_renderBox1.height = seqDataStream->readUint32LE();

		dec->setOutputPixelFormat(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		const Graphics::Surface *decodedSurf = dec->decodeFrame(*seqDataStream);

		memcpy(bgSurface, decodedSurf->getPixels(), _engine->getGraphicsManager()->_renderBox1.width * _engine->getGraphicsManager()->_renderBox1.height * sizeof(PixMap));

		_engine->getGraphicsManager()->modifyPalette(bgSurface, 640 * 480);
		closeHPF(archive);

		delete dec;

		if (_engine->getLogicManager()->_doubleClickFlag &&
			(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
			_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
			return -1;
		} else {
			for (int32 i = _engine->getGraphicsManager()->_renderBox1.y - 1 + _engine->getGraphicsManager()->_renderBox1.height; i >= _engine->getGraphicsManager()->_renderBox1.y; i--) {
				memmove(
					&bgSurface[640 * i + _engine->getGraphicsManager()->_renderBox1.x],
					&bgSurface[_engine->getGraphicsManager()->_renderBox1.width * (i - _engine->getGraphicsManager()->_renderBox1.y)],
					2 * _engine->getGraphicsManager()->_renderBox1.width
				);
			}

			if (_engine->getGraphicsManager()->_renderBox1.x) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, 0, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, 640 - _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
			}

			if (_engine->getGraphicsManager()->_renderBox1.y) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.width, _engine->getGraphicsManager()->_renderBox1.y);
				_engine->getGraphicsManager()->clear(
					_engine->getGraphicsManager()->_frontBuffer,
					_engine->getGraphicsManager()->_renderBox1.x,
					480 - _engine->getGraphicsManager()->_renderBox1.y,
					_engine->getGraphicsManager()->_renderBox1.width,
					_engine->getGraphicsManager()->_renderBox1.y
				);
			}

			if (_engine->getLogicManager()->_doubleClickFlag &&
				(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
				_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
				return -1;

			} else {
				_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_frontBuffer, _engine->getGraphicsManager()->_backBuffer, 0, 0, 640, 480);

				if (tbm.x      != _engine->getGraphicsManager()->_renderBox1.x     ||
					tbm.y      != _engine->getGraphicsManager()->_renderBox1.y     ||
					tbm.width  != _engine->getGraphicsManager()->_renderBox1.width ||
					tbm.height != _engine->getGraphicsManager()->_renderBox1.height) {

					_engine->getGraphicsManager()->_renderBox2.x = tbm.x;
					_engine->getGraphicsManager()->_renderBox2.y = tbm.y;
					_engine->getGraphicsManager()->_renderBox2.width = tbm.width;
					_engine->getGraphicsManager()->_renderBox2.height = tbm.height;

					if (_engine->getGraphicsManager()->_renderBox2.x > _engine->getGraphicsManager()->_renderBox1.x)
						_engine->getGraphicsManager()->_renderBox2.x = _engine->getGraphicsManager()->_renderBox1.x;

					if (_engine->getGraphicsManager()->_renderBox2.y > _engine->getGraphicsManager()->_renderBox1.y)
						_engine->getGraphicsManager()->_renderBox2.y = _engine->getGraphicsManager()->_renderBox1.y;

					if (_engine->getGraphicsManager()->_renderBox1.height + _engine->getGraphicsManager()->_renderBox1.y > (_engine->getGraphicsManager()->_renderBox2.height + _engine->getGraphicsManager()->_renderBox2.y))
						_engine->getGraphicsManager()->_renderBox2.height = _engine->getGraphicsManager()->_renderBox1.height + _engine->getGraphicsManager()->_renderBox1.y - _engine->getGraphicsManager()->_renderBox2.y;

					if (_engine->getGraphicsManager()->_renderBox1.x + _engine->getGraphicsManager()->_renderBox1.width > (_engine->getGraphicsManager()->_renderBox2.x + _engine->getGraphicsManager()->_renderBox2.width))
						_engine->getGraphicsManager()->_renderBox2.width = _engine->getGraphicsManager()->_renderBox1.x + _engine->getGraphicsManager()->_renderBox1.width - _engine->getGraphicsManager()->_renderBox2.x;

					return 1;
				}

				return 0;
			}
		}
	} else {
		memset(_engine->getGraphicsManager()->_backBuffer, 0, (640 * 480 * sizeof(PixMap)));

		_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backBuffer, _engine->getGraphicsManager()->_frontBuffer, 0, 0, 640, 480);
		_engine->getGraphicsManager()->_renderBox1.x = 0;
		_engine->getGraphicsManager()->_renderBox1.y = 0;
		_engine->getGraphicsManager()->_renderBox1.width = 640;
		_engine->getGraphicsManager()->_renderBox1.height = 480;

		_engine->getGraphicsManager()->_renderBox2.x = 0;
		_engine->getGraphicsManager()->_renderBox2.y = _engine->getGraphicsManager()->_renderBox1.y;
		_engine->getGraphicsManager()->_renderBox2.width = _engine->getGraphicsManager()->_renderBox1.width;
		_engine->getGraphicsManager()->_renderBox2.height = _engine->getGraphicsManager()->_renderBox1.height;

		return 1;
	}
}

} // End of namespace LastExpress
