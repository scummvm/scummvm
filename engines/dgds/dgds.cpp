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
		_gdsScene->load("DRAGON.GDS", _resource, _decompressor);

		REQFileData invRequestData;
		REQFileData vcrRequestData;
		RequestParser invRequest(_resource, _decompressor);
		RequestParser vcrRequest(_resource, _decompressor);
		invRequest.parse(&invRequestData, "DINV.REQ");
		vcrRequest.parse(&vcrRequestData, "DVCR.REQ");
		debug("Parsed DINV.REQ:\n%s", invRequestData.dump().c_str());
		debug("Parsed DVCR.REQ:\n%s", vcrRequestData.dump().c_str());

		// Load the intro and play it for now.
		interpIntro.load("TITLE1.ADS");
		//interpIntro.load("INTRO.ADS");

		_fntP = (PFont *)Font::load("DRAGON.FNT", _resource, _decompressor);
	} else if (getGameId() == GID_CHINA) {
		//_scene->load("S101.SDS", _resource, _decompressor);
		interpIntro.load("TITLE.ADS");

		_fntP = (PFont *)Font::load("HOC.FNT", _resource, _decompressor);
	} else if (getGameId() == GID_BEAMISH) {
		//_scene->load("S34.SDS", _resource, _decompressor);
		interpIntro.load("TITLE.ADS");

		_fntP = (PFont *)Font::load("WILLY.FNT", _resource, _decompressor);
	}

	bool moveToNext = false;

	while (!shouldQuit()) {
		if (eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					moveToNext = true;
				default:
					break;
				}
			}
		}

		if (getGameId() == GID_DRAGON || getGameId() == GID_CHINA) {
			if (moveToNext || !interpIntro.run()) {
				moveToNext = false;

				if (!creditsShown) {
					creditsShown = true;
					if (getGameId() == GID_DRAGON)
						_scene->load("S55.SDS", _resource, _decompressor); // FIXME: Removing this breaks the Bahumat scene dialog
					interpIntro.load("INTRO.ADS");
				} else {
					return Common::kNoError;
				}
			}
		} else if (getGameId() == GID_BEAMISH) {
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



// Parts of the old parse file code, kept temporarily for reference
#if 0
void DgdsEngine::parseFileInner(Common::Platform platform, Common::SeekableReadStream &file, const char *name) {
	const char *dot;
	DGDS_EX ex = 0;

	if ((dot = strrchr(name, '.'))) {
		ex = MKTAG24(dot[1], dot[2], dot[3]);
	}

	uint parent = 0;

	if (ex == EX_VIN || ex == EX_AMG) {
		Common::String line = file.readLine();
		while (!file.eos()) {
			if (!line.empty())
				debug("    \"%s\"", line.c_str());
			line = file.readLine();
		}
	} else {
		DgdsChunkReader chunk(&file);
		while (chunk.readNextHeader(ex, name)) {
			if (chunk.isContainer()) {
				parent = chunk.getId();
				continue;
			}

			chunk.readContent(_decompressor);
			Common::SeekableReadStream *stream = chunk.getContent();

			switch (ex) {
			case EX_TDS:
				/* Heart of China. */
				if (chunk.isSection(ID_THD)) {
					uint32 mark = stream->readUint32LE();
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
			default:
				break;
			}
		}
	}
}
#endif

} // End of namespace Dgds
