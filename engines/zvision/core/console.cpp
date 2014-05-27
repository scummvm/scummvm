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

#include "common/scummsys.h"

#include "zvision/core/console.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/strings/string_manager.h"
#include "zvision/video/zork_avi_decoder.h"
#include "zvision/sound/zork_raw.h"
#include "zvision/utility/utility.h"
#include "zvision/cursors/cursor.h"

#include "common/system.h"
#include "common/file.h"
#include "common/bufferedstream.h"

#include "gui/debugger.h"

#include "audio/mixer.h"


namespace ZVision {

Console::Console(ZVision *engine) : GUI::Debugger(), _engine(engine) {
	registerCmd("loadimage", WRAP_METHOD(Console, cmdLoadImage));
	registerCmd("loadvideo", WRAP_METHOD(Console, cmdLoadVideo));
	registerCmd("loadsound", WRAP_METHOD(Console, cmdLoadSound));
	registerCmd("raw2wav", WRAP_METHOD(Console, cmdRawToWav));
	registerCmd("setrenderstate", WRAP_METHOD(Console, cmdSetRenderState));
	registerCmd("generaterendertable", WRAP_METHOD(Console, cmdGenerateRenderTable));
	registerCmd("setpanoramafov", WRAP_METHOD(Console, cmdSetPanoramaFoV));
	registerCmd("setpanoramascale", WRAP_METHOD(Console, cmdSetPanoramaScale));
	registerCmd("changelocation", WRAP_METHOD(Console, cmdChangeLocation));
	registerCmd("dumpfile", WRAP_METHOD(Console, cmdDumpFile));
	registerCmd("parseallscrfiles", WRAP_METHOD(Console, cmdParseAllScrFiles));
	registerCmd("rendertext", WRAP_METHOD(Console, cmdRenderText));
}

bool Console::cmdLoadImage(int argc, const char **argv) {
	if (argc == 4)
		_engine->getRenderManager()->renderImageToScreen(argv[1], atoi(argv[2]), atoi(argv[3]));
	else {
		debugPrintf("Use loadimage <fileName> <destinationX> <destinationY> to load an image to the screen\n");
		return true;
	}

	return true;
}

bool Console::cmdLoadVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use loadvideo <fileName> to load a video to the screen\n");
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
		file->open(argv[1]);

		Audio::AudioStream *soundStream = makeRawZorkStream(file, atoi(argv[2]), isStereo == 0 ? false : true);
		Audio::SoundHandle handle;
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream, -1, 100, 0, DisposeAfterUse::YES, false, false);
	} else {
		debugPrintf("Use loadsound <fileName> [<rate> <isStereo: 1 or 0>] to load a sound\n");
		return true;
	}

	return true;
}

bool Console::cmdRawToWav(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Use raw2wav <rawFilePath> <wavFileName> to dump a .RAW file to .WAV\n");
		return true;
	}

	convertRawToWav(argv[1], _engine, argv[2]);
	return true;
}

bool Console::cmdSetRenderState(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use setrenderstate <RenderState: panorama, tilt, flat> to change the current render state\n");
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
		debugPrintf("Use setrenderstate <RenderState: panorama, tilt, flat> to change the current render state\n");

	return true;
}

bool Console::cmdGenerateRenderTable(int argc, const char **argv) {
	_engine->getRenderManager()->getRenderTable()->generateRenderTable();

	return true;
}

bool Console::cmdSetPanoramaFoV(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use setpanoramafov <fieldOfView> to change the current panorama field of view\n");
		return true;
	}

	_engine->getRenderManager()->getRenderTable()->setPanoramaFoV(atof(argv[1]));

	return true;
}

bool Console::cmdSetPanoramaScale(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use setpanoramascale <scale> to change the current panorama scale\n");
		return true;
	}

	_engine->getRenderManager()->getRenderTable()->setPanoramaScale(atof(argv[1]));

	return true;
}

bool Console::cmdChangeLocation(int argc, const char **argv) {
	if (argc != 6) {
		debugPrintf("Use changelocation <char: world> <char: room> <char:node> <char:view> <int: x position> to change your location\n");
		return true;
	}

	_engine->getScriptManager()->changeLocation(*(argv[1]), *(argv[2]), *(argv[3]), *(argv[4]), atoi(argv[5]));

	return true;
}

bool Console::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Use dumpfile <fileName> to dump a file\n");
		return true;
	}

	writeFileContentsToFile(argv[1], argv[1]);

	return true;
}

bool Console::cmdParseAllScrFiles(int argc, const char **argv) {
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.scr");

	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		_engine->getScriptManager()->parseScrFile((*iter)->getName());
	}

	return true;
}

bool Console::cmdRenderText(int argc, const char **argv) {
	if (argc != 7) {
		debugPrintf("Use rendertext <text> <fontNumber> <destX> <destY> <maxWidth> <1 or 0: wrap> to render text\n");
		return true;
	}

	StringManager::TextStyle style = _engine->getStringManager()->getTextStyle(atoi(argv[2]));
	_engine->getRenderManager()->renderTextToWorkingWindow(333, Common::String(argv[1]), style.font, atoi(argv[3]), atoi(argv[4]), style.color, atoi(argv[5]), -1, Graphics::kTextAlignLeft, atoi(argv[6]) == 0 ? false : true);

	return true;
}

} // End of namespace ZVision
