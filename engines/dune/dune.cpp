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

#include "dune/dune.h"

#include "dune/hsq.h"
#include "dune/intro.h"
#include "dune/room.h"
#include "dune/resources.h"
#include "dune/sprite_sheet.h"
#include "dune/video.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/fs.h"
#include "common/scummsys.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "gui/EventRecorder.h"

namespace Dune {

DuneEngine::DuneEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _graphics(this)
{
	_timerTicks = 0;
	_video = new HnmPlayer(this);
	_rnd = new Common::RandomSource("dune_randomseed");

	_currentBankId   = 0xffff;
	_currentBankData = nullptr;
}

DuneEngine::~DuneEngine() {
	delete _rnd;
}

bool DuneEngine::loadExecutableData() {
	Common::File file;
	if (!file.open("DNCDPRG.EXE"))
		return false;

	// The engine uses the layout of the detected CD edition. Check the
	// complete file before using offsets beyond the detection checksum.
	if (file.size() != 78700 ||
			Common::computeStreamMD5AsString(file) != "52a219e5cd7721bc645b54353844010b")
		return false;

	byte header[0x1c];
	if (!file.seek(0) || file.read(header, sizeof(header)) != sizeof(header) ||
			header[0] != 'M' || header[1] != 'Z')
		return false;

	const uint32 imageOffset = 16u * READ_LE_UINT16(header + 8);
	// This edition starts its stack in the data segment, immediately
	// after the initialized data. The MZ header gives both boundaries.
	const uint32 codeSize = 16u * READ_LE_UINT16(header + 0x0e);
	const uint16 dataSize = READ_LE_UINT16(header + 0x10);
	if (imageOffset < sizeof(header) || !codeSize || !dataSize ||
			imageOffset + codeSize + dataSize != file.size())
		return false;

	Common::Array<byte> data;
	data.resize(0x10000);
	memset(data.begin(), 0, data.size());
	if (!file.seek(imageOffset + codeSize) ||
			file.read(data.begin(), dataSize) != dataSize)
		return false;

	_staticState.swap(data);
	return true;
}

Common::Error DuneEngine::run() {
	debug("run");
	_graphics.getInfo(&_screen, &_screenBufferSize);
	_graphics.setMode();

	if (!_archive.openArchive("DUNE.DAT")) {
		debug("Failed to open DUNE.DAT");
	}

	if (!loadExecutableData())
		return Common::Error(Common::kReadingFailed,
				"Dune requires DNCDPRG.EXE from the supported DOS CD release");

	_framebuffer1 = new byte[320*200]{};
	_framebuffer2 = new byte[320*200]{};

	_graphics.setFrameBuffer(_framebuffer1);

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	if (ConfMan.hasKey("dune_playvideo")) {
		playVideo(ConfMan.getInt("dune_playvideo"));
		return Common::kNoError;
	}

	openSpriteSheet("POR.HSQ");

	Room room(this, _archive.openMember("PALACE.SAL"));
	room.draw(_framebuffer1, 0);

	// The original panel table contains 14-byte records at DS:1AE6.
	openSpriteSheet("ICONES.HSQ");
	const byte *panel = _staticState.begin() + 0x1ae6;
	for (int i = 0; i != 22; i++, panel += 14) {
		const uint16 sprite = READ_LE_UINT16(panel + 10);
		if (sprite != 0xffff)
			drawSprite(sprite, READ_LE_INT16(panel), READ_LE_INT16(panel + 2), 0);
	}

	_system->copyRectToScreen(_framebuffer1, 320, 0, 0, 320, 200);
	_system->updateScreen();

	if (ConfMan.hasKey("dune_dump")) {
		dumpScreenPPM(ConfMan.get("dune_dump"));
		return Common::kNoError;
	}

	while (!shouldQuit()) {
		while (eventMan->pollEvent(event)) {
		}
		_system->delayMillis(10);
	}

	return Common::kNoError;
}


// A development aid: writes what is on screen out as a PPM so that rendering
// can be checked without a display.  Set DUNE_DUMP to a file name to use it.
void DuneEngine::dumpScreenPPM(const Common::String &path, const byte *buffer) {
	if (!buffer)
		buffer = _framebuffer1;

	byte palette[3 * 256];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);

	Common::DumpFile out;
	if (!out.open(Common::Path(path))) {
		warning("Could not write %s", path.c_str());
		return;
	}

	int nonZeroPixels = 0;
	for (int i = 0; i != 320 * 200; ++i)
		if (buffer[i])
			nonZeroPixels++;
	int nonZeroPalette = 0;
	for (int i = 0; i != 3 * 256; ++i)
		if (palette[i])
			nonZeroPalette++;
	debug("dump %s: %d non zero pixels, %d non zero palette bytes", path.c_str(), nonZeroPixels, nonZeroPalette);

	out.writeString(Common::String::format("P6\n%d %d\n255\n", 320, 200));
	for (int i = 0; i != 320 * 200; ++i)
		out.write(palette + 3 * buffer[i], 3);

	out.finalize();
}

void DuneEngine::runIntro() {
	Intro intro(this);
	intro.run();
};

bool DuneEngine::isCD() {
	return _gameDescription->flags & ADGF_CD;
}

void DuneEngine::loadBank(uint16 index) {
	if (index == _currentBankId) {
		return;
	}
}

void DuneEngine::applyPaletteBlock(Common::ReadStream *reader) {
	uint8 buf[3 * 256];
	PaletteManager *paletteManager = _system->getPaletteManager();

	for (;;) {
		uint16 h = reader->readUint16LE();
		if (h == 0x100) {
			// Skip 3 bytes
			reader->readByte();
			reader->readByte();
			reader->readByte();
			continue;
		}
		if (h == 0xffff) {
			break;
		}
		int count = (h >> 8);
		int index = h & 0xff;

		if (count == 0) {
			count = 256;
		}

		reader->read(buf, 3 * count);

		// Convert R6G6B6 to R8G8B8
		for (int i = 0; i != 3 * count; ++i) {
			buf[i] = (buf[i] * 255 + 31) / 63;
		}

		paletteManager->setPalette(buf, index, count);
	}
}

void DuneEngine::dumpResource(const char *filename) {
	Resource r = _archive.openMember(filename);
	if (!r) {
		return;
	}

	Common::DumpFile f;

	Common::String fn = filename;
	fn.replace(6, 3, "BIN");
	f.open(Common::Path(fn));

	int size = r->size();
	byte *buf = new byte[size];
	r->read(buf, size);

	f.write(buf, size);
	r->seek(0);
	delete[] buf;
}

void DuneEngine::loadVideo(uint16 videoId) {
	const char *filename = getResourceFilenameByIndex(videoId + INDEX_FIRST_HNM);
	debug("openMember(\"%s\")\n", filename);
	Resource r = _archive.openMember(filename);
	if (!r) {
		return;
	}
	_video->setReader(r);
}

void DuneEngine::playVideo(uint16 videoId) {
	const char *filename = getResourceFilenameByIndex(videoId + INDEX_FIRST_HNM);
	debug("openMember(\"%s\")\n", filename);
	Resource r = _archive.openMember(filename);
	if (!r) {
		return;
	}
	_video->setReader(r);

	if (videoId == HNM_IRULAN) {
		_video->setSubtitles(_staticState.begin() + 0x35a8, _archive.openMember("IRUL1.HSQ"));
	}

	_video->setInterlace(videoId >= HNM_IRULAN);
	_video->start();

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	float nextFrameTime = _system->getMillis() + (1000.0/12.0);
	while (!_video->done() && !shouldQuit()) {
		_video->decodeAVFrame();

		_system->copyRectToScreen(_video->_frameBuffer, 320, 0, 0, 320, 200);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-%04d.ppm", ConfMan.get("dune_dump").c_str(),
												 _video->_currentFrame), _video->_frameBuffer);

		while (eventMan->pollEvent(event)) {
		}

		float now = _system->getMillis();
		if (now < nextFrameTime) {
			_system->delayMillis((int)floor(nextFrameTime - now));
		}
		nextFrameTime += (1000.0/12.0);
	}

}

void DuneEngine::dumpSAL(const char *filename) {
	Resource r = _archive.openMember(filename);

	Common::Array<uint32_t> roomOffsets;

	uint16_t offset_0 = r->readUint16LE();
	roomOffsets.push_back(offset_0);

	for (int i = 1; i != offset_0 / 2; ++i) {
		uint16_t offset = r->readUint16LE();
		roomOffsets.push_back(offset);
	}

	for (int i = 0; i != roomOffsets.size(); ++i) {
		debug("%04x", roomOffsets[i]);
	}
}

static void readFrameHeader(Common::ReadStream *r, int *w, int *h, byte *flags, byte *mode) {
	byte b[4];
	r->read(b, 4);

	*w     = ((0x1 & b[1]) << 8) | b[0];
	*flags = b[1] & 0xfe;
	*h     = b[2];
	*mode  = b[3];
}

void DuneEngine::openSpriteSheet(const char *filename) {
	Resource r = _archive.openMember(filename);

	debug("Opening sprite sheet %s", filename);
	currentSpriteSheet = new SpriteSheet(this, r);
}

void DuneEngine::drawSprite(int index, int dst_x, int dst_y, byte draw_flags) {
	assert(currentSpriteSheet);

	currentSpriteSheet->draw(index, dst_x, dst_y, draw_flags);
}

} // End of namespace Dune
