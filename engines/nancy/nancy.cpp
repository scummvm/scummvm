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

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/installshield_cab.h"

#include "graphics/surface.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "nancy/nancy.h"
#include "nancy/resource.h"
#include "nancy/iff.h"
#include "nancy/audio.h"
#include "nancy/logo.h"

#include "engines/util.h"

namespace Nancy {

NancyEngine *NancyEngine::s_Engine = 0;

NancyEngine::NancyEngine(OSystem *syst, const NancyGameDescription *gd) :
	Engine(syst),
	_gameDescription(gd),
	_bsum(nullptr)
{
	_system = syst;

	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");
	DebugMan.addDebugChannel(kDebugObject, "Object", "Object debug level");
	DebugMan.addDebugChannel(kDebugMusic, "Music", "Music debug level");

	_console = new NancyConsole(this);
	_logoSequence = new LogoSequence(this);
	_rnd = 0;
}

NancyEngine::~NancyEngine() {

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GUI::Debugger *NancyEngine::getDebugger() {
	return _console;
}

bool NancyEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *NancyEngine::getCopyrightString() const {
	return "Copyright 1989-1997 David P Gray, All Rights Reserved.";
}

GameType NancyEngine::getGameType() const {
	return _gameType;
}

Common::Platform NancyEngine::getPlatform() const {
	return _platform;
}

Common::Error NancyEngine::run() {
	s_Engine = this;

	Graphics::PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(640, 480, &format);
	_console = new NancyConsole(this);

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "game");
	SearchMan.addSubDirectoryMatching(gameDataDir, "datafiles");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdsound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "hdvideo");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cdvideo");

	Common::SeekableReadStream *cabStream = SearchMan.createReadStreamForMember("data1.hdr");
	if (cabStream)
		SearchMan.add("data1.hdr", Common::makeInstallShieldArchive(cabStream));

//	_mouse = new MouseHandler(this);
	_res = new ResourceManager(this);
	_res->initialize();

	// Setup mixer
	syncSoundSettings();

	// Some bits and pieces of the engine in order to make something happen
	IFF *boot = new IFF(this, "boot");
	if (!boot->load())
		error("Failed to load boot script");
	preloadCals(*boot);
	readSound(*boot, "MSND", _menuSound);
	_bsum = boot->getChunkStream("BSUM");
	if (!_bsum)
		error("Failed to load BOOT BSUM");
	readBootSummary(*boot);
	delete boot;

	Common::EventManager *ev = g_system->getEventManager();
	bool quit = false;

	_gameFlow.minGameState = kLogo;

	while (!shouldQuit() && !quit) {
		switch(_gameFlow.minGameState) {
		case kLogo:
			_logoSequence->doIt();
			break;
		case kIdle:
		default:
			break;
		}

		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && (event.kbd.flags & Common::KBD_CTRL)) {
				switch(event.kbd.keycode) {
				case Common::KEYCODE_q:
					quit = true;
					break;
				case Common::KEYCODE_d:
					_console->attach();
				default:
					break;
				}
			}
		}

		_system->updateScreen();
		_system->delayMillis(16);
	}
#if 0
	// Play music
	Common::SeekableReadStream *mSnd = SearchMan.createReadStreamForMember(_menuSound.name + ".his");
	if (mSnd) {
		Audio::RewindableAudioStream *aStr = makeHISStream(mSnd, DisposeAfterUse::YES);
		if (aStr) {
			Audio::AudioStream *aStrLoop = Audio::makeLoopingAudioStream(aStr, 0);
			Audio::SoundHandle handle;
			_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &handle, aStrLoop);
		}
	}

	// Show logo
	Graphics::Surface surf;
	if (_res->loadImage("ciftree", _logos[0].name, surf)) {
		_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, surf.w, surf.h);
		surf.free();
	}
#endif

	return Common::kNoError;
}

void NancyEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_rnd = new Common::RandomSource("nancy");
	_rnd->setSeed(42);                              // Kick random number generator
}

void NancyEngine::preloadCals(const IFF &boot) {
	const byte *buf;
	uint size;
	buf = boot.getChunk(ID_PCAL, size);

	if (buf) {
		Common::MemoryReadStream stream(buf, size);
		uint16 count = stream.readUint16LE();
		debugC(1, kDebugEngine, "Preloading %d CALs", count);
		int nameLen = size / count;

		char *name = new char[nameLen];

		for (uint i = 0; i < count; i++) {
			stream.read(name, nameLen);
			name[nameLen - 1] = 0;
			debugC(1, kDebugEngine, "Preloading CAL '%s'", name);
			if (!_res->loadCifTree(name, "cal"))
				error("Failed to preload CAL '%s'", name);
		}

		delete[] name;

		if (stream.err())
			error("Error reading PCAL chunk");
	} else
		debugC(1, kDebugEngine, "No PCAL chunk found");
}

void NancyEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String NancyEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}

Common::String NancyEngine::readFilename(Common::ReadStream *stream) const {
	char buf[kMaxFilenameLen + 1];
	int read = stream->read(buf, getFilenameLen());
	buf[read] = 0;
	return Common::String(buf);
}

void NancyEngine::readImageList(const IFF &boot, const Common::String &prefix, ImageList &list) {
	byte count = _bsum->readByte();
	debugC(1, kDebugEngine, "Found %i %s images", count, prefix.c_str());

	for (int i = 0; i < count; ++i) {
		Common::String chunkName = Common::String::format("%s%d", prefix.c_str(), i);
		Common::SeekableReadStream *chunkStream = boot.getChunkStream(chunkName);

		if (!chunkStream)
			error("Failed to read BOOT %s", chunkName.c_str());

		Image image;
		image.name = readFilename(chunkStream);
		chunkStream->skip(1);
		image.width = chunkStream->readUint16LE();
		image.height = chunkStream->readUint16LE();

		if (chunkStream->err())
			error("Error reading %s%d", prefix.c_str(), i);

		debugC(1, kDebugEngine, "Adding %s (%dx%d)", image.name.c_str(), image.width, image.height);
		list.push_back(image);

		delete chunkStream;
	}
}

void NancyEngine::readSound(const IFF &boot, const Common::String &name, NancyEngine::Sound &sound) {
	Common::SeekableReadStream *stream = boot.getChunkStream(name);

	if (!stream)
		error("Failed to read BOOT %s", name.c_str());

	sound.name = readFilename(stream);
}

class NancyEngine_v0 : public NancyEngine {
public:
	NancyEngine_v0(OSystem *syst, const NancyGameDescription *gd) : NancyEngine(syst, gd) { }

private:
	virtual uint getFilenameLen() const { return 9; }
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v0::readBootSummary(const IFF &boot) {
	_bsum->seek(0x151);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
}

class NancyEngine_v1 : public NancyEngine_v0 {
public:
	NancyEngine_v1(OSystem *syst, const NancyGameDescription *gd) : NancyEngine_v0(syst, gd) { }

private:
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v1::readBootSummary(const IFF &boot) {
	_bsum->seek(0x14b);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
}

class NancyEngine_v2 : public NancyEngine_v1 {
public:
	NancyEngine_v2(OSystem *syst, const NancyGameDescription *gd) : NancyEngine_v1(syst, gd) { }

private:
	virtual uint getFilenameLen() const { return 32; }
	virtual void readBootSummary(const IFF &boot);
};

void NancyEngine_v2::readBootSummary(const IFF &boot) {
	_bsum->seek(0xa7);
	readImageList(boot, "FR", _frames);
	readImageList(boot, "LG", _logos);
}

NancyEngine *NancyEngine::create(GameType type, OSystem *syst, const NancyGameDescription *gd) {
	switch(type) {
	case kGameTypeNancy1:
		return new NancyEngine_v0(syst, gd);
	case kGameTypeNancy2:
		return new NancyEngine_v1(syst, gd);
	case kGameTypeNancy3:
		return new NancyEngine_v2(syst, gd);
	default:
		error("Unknown GameType");
	}
}

} // End of namespace Nancy
