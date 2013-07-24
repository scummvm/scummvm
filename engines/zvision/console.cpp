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

#include "gui/debugger.h"
#include "common/file.h"
#include "common/bufferedstream.h"
#include "audio/mixer.h"

#include "zvision/console.h"
#include "zvision/zvision.h"
#include "zvision/render_manager.h"
#include "zvision/zork_avi_decoder.h"
#include "zvision/zork_raw.h"
#include "zvision/utility.h"

namespace ZVision {

Console::Console(ZVision *engine) : GUI::Debugger(), _engine(engine) {
	DCmd_Register("loadimage", WRAP_METHOD(Console, cmdLoadImage));
	DCmd_Register("loadvideo", WRAP_METHOD(Console, cmdLoadVideo));
	DCmd_Register("loadsound", WRAP_METHOD(Console, cmdLoadSound));
	DCmd_Register("raw2wav", WRAP_METHOD(Console, cmdRawToWav));
	DCmd_Register("setrenderstate", WRAP_METHOD(Console, cmdSetRenderState));
}

bool Console::cmdLoadImage(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Use loadimage <fileName> <x> <y> to load an image to the screen\n");
		return true;
	}
	_engine->getRenderManager()->renderImageToScreen(argv[1], atoi(argv[2]), atoi(argv[3]));

	return true;
}

bool Console::cmdLoadVideo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Use loadvideo <fileName> to load a video to the screen\n");
		return true;
	}

	Video::VideoDecoder *videoDecoder = new ZorkAVIDecoder();
	if (videoDecoder && videoDecoder->loadFile(argv[1])) {
		_engine->getRenderManager()->startVideo(videoDecoder);
	}

	return true;
}

bool Console::cmdLoadSound(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Use loadsound <fileName> to load a sound\n");
		return true;
	}

	if (!Common::File::exists(argv[1])) {
		DebugPrintf("File does not exist\n");
		return true;
	}

	Audio::AudioStream *soundStream = makeRawZorkStream(argv[1], _engine);
	Audio::SoundHandle handle;
	_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream, -1, 100, 0, DisposeAfterUse::YES, false, false);

	return true;
}

bool Console::cmdRawToWav(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Use raw2wav <rawFilePath> <wavFileName> to dump a .RAW file to .WAV\n");
		return true;
	}

	convertRawToWav(argv[1], _engine, argv[2]);
	return true;
}

bool Console::cmdSetRenderState(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Use setrenderstate <RenderState: panorama, tilt, flat> to change the current render state\n");
		return true;
	}

	Common::String renderState(argv[1]);

	if (renderState.matchString("panorama", true))
		_engine->getRenderManager()->setRenderState(RenderTable::PANORAMA);
	else if (renderState.matchString("tilt", true))
		_engine->getRenderManager()->setRenderState(RenderTable::TILT);
	else if (renderState.matchString("flat", true))
		_engine->getRenderManager()->setRenderState(RenderTable::FLAT);
	else
		DebugPrintf("Use setrenderstate <RenderState: panorama, tilt, flat> to change the current render state\n");

	return true;
}

} // End of namespace ZVision
