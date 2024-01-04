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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/platform.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/system.h"

#include "common/formats/iff_container.h"

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "dgds/console.h"
#include "dgds/decompress.h"
#include "dgds/scene.h"
#include "dgds/detection_tables.h"
#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/parser.h"
#include "dgds/request.h"
#include "dgds/resource.h"
#include "dgds/scripts.h"
#include "dgds/sound.h"

namespace Dgds {

//#define DUMP_ALL_CHUNKS 1

DgdsEngine::DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _image(nullptr), /*_fntF(nullptr),*/ _fntP(nullptr), _console(nullptr),
	_soundPlayer(nullptr), _decompressor(nullptr), _scene(nullptr), _gdsScene(nullptr),
	_resource(nullptr) {
	syncSoundSettings();

	_platform = gameDesc->platform;

	if (!strcmp(gameDesc->gameId, "rise"))
		_gameId = GID_DRAGON;
	else if (!strcmp(gameDesc->gameId, "china"))
		_gameId = GID_CHINA;
	else if (!strcmp(gameDesc->gameId, "beamish"))
		_gameId = GID_BEAMISH;
	else
		error("Unknown game ID");

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "patches");
}

DgdsEngine::~DgdsEngine() {
	DebugMan.removeAllDebugChannels();

	delete _image;
	delete _decompressor;
	delete _resource;
	delete _scene;
	delete _gdsScene;
	delete _soundPlayer;
}

void readStrings(Common::SeekableReadStream *stream) {
	uint16 count = stream->readUint16LE();
	debug("        %u:", count);

	for (uint16 k = 0; k < count; k++) {
		byte ch;
		uint16 idx = stream->readUint16LE();

		Common::String str;
		while ((ch = stream->readByte()))
			str += ch;
		debug("        %2u: %2u, \"%s\"", k, idx, str.c_str());
	}
}

void DgdsEngine::parseRstChunk(Common::SeekableReadStream &file) {
	uint32 mark = file.readUint32LE();
	debug("    0x%X", mark);

	// elaborate guesswork. who knows it might be true.
	while (!file.eos()) {
		uint16 vals[7];
		uint16 idx = file.readUint16LE();

		debugN("  #%u:\t", idx);

		if (idx == 0)
			break;

		for (int i = 0; i < ARRAYSIZE(vals); i++) {
			vals[i] = file.readUint16LE();
			if (i != 0)
				debugN(", ");
			debugN("%u", vals[i]);
		}
		
		debug(".");
	}

	debug("-");

	while (!file.eos()) {
		uint16 vals[2];
		uint16 idx = file.readUint16LE();
		
		debugN("  #%u:\t", idx);

		for (int i = 0; i < ARRAYSIZE(vals); i++) {
			vals[i] = file.readUint16LE();
			if (i != 0)
				debugN(", ");
			debugN("%u", vals[i]);
		}

		debug(".");

		if (idx == 0)
			break;
	}
	debug("-");
}

void DgdsEngine::parseAmigaChunks(Common::SeekableReadStream &file, DGDS_EX ex) {
	Common::String line;

	switch (ex) {
	case EX_SCR: {
		/* Unknown image format (Amiga). */
		byte tag[5];
		file.read(tag, 4); /* maybe */
		tag[4] = '\0';

		uint16 pitch, planes;
		pitch = file.readUint16BE();  /* always 200 (320x200 screen). */
		planes = file.readUint16BE(); /* always 5 (32 color). */

		debug("    \"%s\" pitch:%u bpp:%u size: %u bytes",
			  tag, pitch, planes,
			  SCREEN_WIDTH * planes * SCREEN_HEIGHT / 8);
	} break;
	case EX_BMP: {
		/* Unknown image format (Amiga). */
		uint16 tcount = file.readUint16BE();
		uint16 *tw = new uint16[tcount];
		uint16 *th = new uint16[tcount];

		uint32 packedSize;
		uint32 unpackedSize = file.readUint32BE();	// TODO: this is wrong - it's re-read below
		debug("        [%u] %u =", tcount, unpackedSize);

		uint32 sz = 0;
		uint32 *toffset = new uint32[tcount];
		for (uint16 k = 0; k < tcount; k++) {
			tw[k] = file.readUint16BE();
			th[k] = file.readUint16BE();
			debug("        %ux%u ~@%u", tw[k], th[k], sz);

			toffset[k] = sz;
			sz += uint(tw[k] + 15) / 16 * th[k] * 5;
		}
		debug("    ~= [%u]", sz);

		/* this is a wild guess. */
		byte version[13];
		file.read(version, 12);
		version[12] = '\0';
		debug("    %s", version);

		unpackedSize = file.readUint32BE();
		packedSize = file.readUint32BE();
		debug("        %u -> %u",
			  packedSize, unpackedSize);
		delete[] toffset;
		delete[] tw;
		delete[] th;
	} break;
	case EX_SNG:
		/* IFF-SMUS music (Amiga). */
		break;
	case EX_AMG:
		/* (Amiga). */
		line = file.readLine();
		while (!file.eos() && !line.empty()) {
			debug("    \"%s\"", line.c_str());
			line = file.readLine();
		}
		break;
	case EX_INS: // Handled in Sound::playAmigaSfx
		error("Should not be here");
	}
}

void DgdsEngine::parseFile(const Common::String &filename) {
       Common::SeekableReadStream *stream = _resource->getResource(filename);
       if (!stream)
               error("Couldn't get resource file %s", filename.c_str());
       parseFileInner(_platform, *stream, filename.c_str());
       delete stream;
}


void DgdsEngine::parseFileInner(Common::Platform platform, Common::SeekableReadStream &file, const char *name) {
	const char *dot;
	DGDS_EX ex = 0;

	if ((dot = strrchr(name, '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	}

	uint parent = 0;

	if (platform == Common::kPlatformAmiga) {
		parseAmigaChunks(file, ex);
	}

	if (DgdsChunkReader::isFlatfile(ex)) {
		Common::String line;

		switch (ex) {
		case EX_RST:
			parseRstChunk(file);
			break;
		case EX_VIN:
			line = file.readLine();
			while (!file.eos()) {
				if (!line.empty())
					debug("    \"%s\"", line.c_str());
				line = file.readLine();
			}
			break;
		case EX_DAT: {
			// TODO
			int leftover = file.size() - file.pos();
			file.hexdump(leftover);
			file.skip(leftover);
			} break;
		default:
			break;
		}
	} else {
		DgdsChunkReader chunk(&file);
		int chunkno = 0;
		while (chunk.readNextHeader(ex, name)) {
			if (chunk.isContainer()) {
				parent = chunk.getId();
				continue;
			}

			chunk.readContent(_decompressor);
			Common::SeekableReadStream *stream = chunk.getContent();

#ifdef DUMP_ALL_CHUNKS
			{
				Common::DumpFile out;
				int64 start = stream->pos();
				out.open(Common::Path(Common::String::format("tmp/dgds_%s_%02d_%.3s.dump", name, chunkno, chunk.getIdStr())));
				out.writeStream(stream);
				out.close();
				stream->seek(start);
			}

#endif
			chunkno++;

			switch (ex) {
			case EX_TDS:
				/* Heart of China. */
				if (chunk.isSection(ID_THD)) {
					uint32 mark;

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String bmpName;
					while ((ch = stream->readByte()))
						bmpName += ch;
					debug("    \"%s\"", bmpName.c_str());

					Common::String personName;
					while ((ch = stream->readByte()))
						personName += ch;
					debug("    \"%s\"", personName.c_str());
				}
				break;
			case EX_DDS:
				/* Heart of China. */
				if (chunk.isSection(ID_DDS)) {
					uint32 mark;

					mark = stream->readUint32LE();
					debug("    0x%X", mark);

					char version[7];
					stream->read(version, sizeof(version));
					debug("    \"%s\"", version);

					byte ch;
					Common::String tag;
					while ((ch = stream->readByte()))
						tag += ch;
					debug("    \"%s\"", tag.c_str());
				}
				break;
			case EX_SDS:
				if (chunk.isSection(ID_SDS)) {
					_scene->parse(stream);
				}
				break;
			case EX_GDS:
				if (chunk.isSection(ID_GDS)) {
					// do nothing, this is the container.
					assert(chunk.isContainer());
				} else if (chunk.isSection(ID_INF)) {
					_gdsScene->parseInf(stream);
				} else if (chunk.isSection(ID_SDS)) {
					_gdsScene->parse(stream);
				}
				break;
			case EX_SNG:	// Handled in Sound::playMusic
			case EX_SX:     // Handled in Sound::playMacMusic
			case EX_PAL:	// Handled in Image::setPalette
			case EX_SCR:    // Handled in Image::loadScreen
			case EX_BMP:    // Handled in Image::loadBitmap
			case EX_TTM:	// Handled by TTMParser
			case EX_REQ:    // Handled by Request
			case EX_ADS:    // Handled by ADSParser
			case EX_ADL:    // Handled by ADSParser
			case EX_ADH:    // Handled by ADSParser
			case EX_FNT:	// Handled by Font::load
				error("Should not be here");
			default:
				break;
			}
			int leftover = stream->size() - stream->pos();
			//stream->hexdump(leftover);
			stream->skip(leftover);
		}
	}

	debug("  [%u] --", (uint)file.pos());
}

Common::Error DgdsEngine::run() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	_console = new Console(this);
	_resource = new ResourceManager();
	_decompressor = new Decompressor();
	_image = new Image(_resource, _decompressor);
	_soundPlayer = new Sound(_mixer, _resource, _decompressor);
	_scene = new SDSScene();
	_gdsScene = new GDSScene();

	setDebugger(_console);

	_bottomBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_topBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_resData.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	g_system->fillScreen(0);

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event ev;

	ADSInterpreter interpIntro(this);
	bool creditsShown = false;

	if (getGameId() == GID_DRAGON) {
		// Test parsing some things..
		parseFile("DRAGON.GDS");

		RequestData invRequestData;
		RequestData vcrRequestData;
		Request invRequest(_resource, _decompressor);
		Request vcrRequest(_resource, _decompressor);
		invRequest.parse(&invRequestData, "DINV.REQ");
		vcrRequest.parse(&vcrRequestData, "DVCR.REQ");

		// Load the intro and play it for now.
		interpIntro.load("TITLE1.ADS");
		//interpIntro.load("INTRO.ADS");

		_fntP = (PFont *)Font::load("DRAGON.FNT", _resource, _decompressor);
	} else if (getGameId() == GID_CHINA) {
		interpIntro.load("TITLE.ADS");

		_fntP = (PFont *)Font::load("HOC.FNT", _resource, _decompressor);
	} else if (getGameId() == GID_BEAMISH) {
		interpIntro.load("TITLE.ADS");

		//_fntP = (PFont *)Font::load("HOC.FNT", _resource, _decompressor);
	}

	//_console->attach();

	while (!shouldQuit()) {
		if (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
					/*
					case Common::KEYCODE_TAB:	sid++;					break;
					case Common::KEYCODE_UP:	if (id > 0) id--; bk=0;			break;
					case Common::KEYCODE_DOWN:	if (id < BMPs.size()) id++; bk=0;	break;
					case Common::KEYCODE_LEFT:	if (bk > 0) bk--;			break;
					case Common::KEYCODE_RIGHT:	if (bk < (_tcount-1)) bk++;		break;
					*/
				case Common::KEYCODE_ESCAPE:
					return Common::kNoError;
				default:
					break;
				}
			}
		}

		if (getGameId() == GID_DRAGON) {
			if (!interpIntro.run()) {
				if (!creditsShown) {
					creditsShown = true;
					parseFile("S55.SDS"); // FIXME: Removing this breaks the Bahumat scene dialog
					interpIntro.load("INTRO.ADS");
				} else {
					return Common::kNoError;
				}
			}
		} else if (getGameId() == GID_CHINA || getGameId() == GID_BEAMISH) {
			if (!interpIntro.run())
				return Common::kNoError;
		}

		g_system->delayMillis(40);
	}
	return Common::kNoError;
}

Common::SeekableReadStream *DgdsEngine::getResource(const Common::String &name, bool ignorePatches) {
	return _resource->getResource(name, ignorePatches);
}

} // End of namespace Dgds
