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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "common/scummsys.h"

#include "zvision/core/console.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/video/zork_avi_decoder.h"
#include "zvision/sound/zork_raw.h"
#include "zvision/graphics/cursors/cursor.h"

#include "common/system.h"
#include "common/file.h"
#include "common/bufferedstream.h"

#include "gui/debugger.h"

#include "audio/mixer.h"

namespace ZVision {

Console::Console(ZVision *engine) : GUI::Debugger(), _engine(engine) {
	registerCmd("loadvideo", WRAP_METHOD(Console, cmdLoadVideo));
	registerCmd("loadsound", WRAP_METHOD(Console, cmdLoadSound));
	registerCmd("raw2wav", WRAP_METHOD(Console, cmdRawToWav));
	registerCmd("setrenderstate", WRAP_METHOD(Console, cmdSetRenderState));
	registerCmd("generaterendertable", WRAP_METHOD(Console, cmdGenerateRenderTable));
	registerCmd("setpanoramafov", WRAP_METHOD(Console, cmdSetPanoramaFoV));
	registerCmd("setpanoramascale", WRAP_METHOD(Console, cmdSetPanoramaScale));
	registerCmd("location", WRAP_METHOD(Console, cmdLocation));
	registerCmd("dumpfile", WRAP_METHOD(Console, cmdDumpFile));
	registerCmd("dumpfiles", WRAP_METHOD(Console, cmdDumpFiles));
}

bool Console::cmdLoadVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use %s <fileName> to load a video to the screen\n", argv[0]);
		return true;
	}

	ZorkAVIDecoder videoDecoder;
	if (videoDecoder.loadFile(argv[1])) {
		_engine->playVideo(videoDecoder);
	}

	return true;
}

bool Console::cmdLoadSound(int argc, const char **argv) {
	if (!Common::File::exists(argv[1])) {
		debugPrintf("File does not exist\n");
		return true;
	}

	if (argc == 2) {
		Audio::AudioStream *soundStream = makeRawZorkStream(argv[1], _engine);
		Audio::SoundHandle handle;
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream, -1, 100, 0, DisposeAfterUse::YES, false, false);
	} else if (argc == 4) {
		int isStereo = atoi(argv[3]);

		Common::File *file = new Common::File();
		if (!_engine->getSearchManager()->openFile(*file, argv[1])) {
			warning("File not found: %s", argv[1]);
			return true;
		}

		Audio::AudioStream *soundStream = makeRawZorkStream(file, atoi(argv[2]), isStereo == 0 ? false : true);
		Audio::SoundHandle handle;
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream, -1, 100, 0, DisposeAfterUse::YES, false, false);
	} else {
		debugPrintf("Use %s <fileName> [<rate> <isStereo: 1 or 0>] to load a sound\n", argv[0]);
		return true;
	}

	return true;
}

bool Console::cmdRawToWav(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Use %s <rawFilePath> <wavFileName> to dump a .RAW file to .WAV\n", argv[0]);
		return true;
	}

	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, argv[1])) {
		warning("File not found: %s", argv[1]);
		return true;
	}

	Audio::AudioStream *audioStream = makeRawZorkStream(argv[1], _engine);

	Common::DumpFile output;
	output.open(argv[2]);

	output.writeUint32BE(MKTAG('R', 'I', 'F', 'F'));
	output.writeUint32LE(file.size() * 2 + 36);
	output.writeUint32BE(MKTAG('W', 'A', 'V', 'E'));
	output.writeUint32BE(MKTAG('f', 'm', 't', ' '));
	output.writeUint32LE(16);
	output.writeUint16LE(1);
	uint16 numChannels;
	if (audioStream->isStereo()) {
		numChannels = 2;
		output.writeUint16LE(2);
	} else {
		numChannels = 1;
		output.writeUint16LE(1);
	}
	output.writeUint32LE(audioStream->getRate());
	output.writeUint32LE(audioStream->getRate() * numChannels * 2);
	output.writeUint16LE(numChannels * 2);
	output.writeUint16LE(16);
	output.writeUint32BE(MKTAG('d', 'a', 't', 'a'));
	output.writeUint32LE(file.size() * 2);
	int16 *buffer = new int16[file.size()];
	audioStream->readBuffer(buffer, file.size());
	output.write(buffer, file.size() * 2);

	delete[] buffer;


	return true;
}

bool Console::cmdSetRenderState(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use %s <RenderState: panorama, tilt, flat> to change the current render state\n", argv[0]);
		return true;
	}

	Common::String renderState(argv[1]);

	if (renderState.matchString("panorama", true))
		_engine->getRenderManager()->getRenderTable()->setRenderState(RenderTable::PANORAMA);
	else if (renderState.matchString("tilt", true))
		_engine->getRenderManager()->getRenderTable()->setRenderState(RenderTable::TILT);
	else if (renderState.matchString("flat", true))
		_engine->getRenderManager()->getRenderTable()->setRenderState(RenderTable::FLAT);
	else
		debugPrintf("Use %s <RenderState: panorama, tilt, flat> to change the current render state\n", argv[0]);

	return true;
}

bool Console::cmdGenerateRenderTable(int argc, const char **argv) {
	_engine->getRenderManager()->getRenderTable()->generateRenderTable();

	return true;
}

bool Console::cmdSetPanoramaFoV(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use %s <fieldOfView> to change the current panorama field of view\n", argv[0]);
		return true;
	}

	_engine->getRenderManager()->getRenderTable()->setPanoramaFoV(atof(argv[1]));

	return true;
}

bool Console::cmdSetPanoramaScale(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use %s <scale> to change the current panorama scale\n", argv[0]);
		return true;
	}

	_engine->getRenderManager()->getRenderTable()->setPanoramaScale(atof(argv[1]));

	return true;
}

bool Console::cmdLocation(int argc, const char **argv) {
	Location curLocation = _engine->getScriptManager()->getCurrentLocation();
	Common::String scrFile = Common::String::format("%c%c%c%c.scr", curLocation.world, curLocation.room, curLocation.node, curLocation.view);
	debugPrintf("Current location: world '%c', room '%c', node '%c', view '%c', offset %d, script %s\n",
				curLocation.world, curLocation.room, curLocation.node, curLocation.view, curLocation.offset, scrFile.c_str());
	
	if (argc != 6) {
		debugPrintf("Use %s <char: world> <char: room> <char:node> <char:view> <int: x offset> to change your location\n", argv[0]);
		return true;
	}

	_engine->getScriptManager()->changeLocation(*(argv[1]), *(argv[2]), *(argv[3]), *(argv[4]), atoi(argv[5]));

	return true;
}

void dumpFile(Common::SeekableReadStream *s, const char *outName) {
	byte *buffer = new byte[s->size()];
	s->read(buffer, s->size());

	Common::DumpFile dumpFile;
	dumpFile.open(outName);

	dumpFile.write(buffer, s->size());
	dumpFile.flush();
	dumpFile.close();

	delete[] buffer;
}

bool Console::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use %s <fileName> to dump a file\n", argv[0]);
		return true;
	}

	Common::File f;
	if (!_engine->getSearchManager()->openFile(f, argv[1])) {
		warning("File not found: %s", argv[1]);
		return true;
	}

	dumpFile(&f, argv[1]);

	return true;
}

bool Console::cmdDumpFiles(int argc, const char **argv) {
	Common::String fileName;
	Common::SeekableReadStream *in;

	if (argc != 2) {
		debugPrintf("Use %s <file extension> to dump all files with a specific extension\n", argv[0]);
		return true;
	}

	SearchManager::MatchList fileList;
	_engine->getSearchManager()->listMembersWithExtension(fileList, argv[1]);

	for (SearchManager::MatchList::iterator iter = fileList.begin(); iter != fileList.end(); ++iter) {
		fileName = iter->_value.name;
		debugPrintf("Dumping %s\n", fileName.c_str());

		in = iter->_value.arch->createReadStreamForMember(iter->_value.name);
		dumpFile(in, fileName.c_str());
		delete in;
	}

	return true;
}

} // End of namespace ZVision
