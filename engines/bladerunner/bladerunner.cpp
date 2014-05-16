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

#include "bladerunner/bladerunner.h"

#include "bladerunner/chapters.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/image.h"
#include "bladerunner/settings.h"
#include "bladerunner/vqa_decoder.h"

#include "common/error.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/pixelformat.h"

namespace BladeRunner {

BladeRunnerEngine::BladeRunnerEngine(OSystem *syst) : Engine(syst) {
	_gameInfo = nullptr;

	_windowIsActive = true;
	_gameIsRunning  = true;

	_chapters = nullptr;
	_settings = new Settings(this);
}

bool BladeRunnerEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsRTL;
}

Common::Error BladeRunnerEngine::run() {
	initGraphics(640, 480, true, &RGB555);

	startup();

	if (!warnUserAboutUnsupportedGame())
		return Common::kNoError;

	init2();

	/* TODO: Check for save games and enter KIA */
	gameLoop();

	shutdown();

	return Common::kNoError;
}

bool BladeRunnerEngine::startup() {
	bool r;

	_surface1.create(640, 480, RGB555);

	r = openArchive("STARTUP.MIX");
	if (!r)
		return false;

	loadSplash();

	_gameInfo = new GameInfo(this);
	if (!_gameInfo)
		return false;

	r = _gameInfo->open("GAMEINFO.DAT");
	if (!r)
		return false;

	_chapters = new Chapters(this);
	if (!_chapters)
		return false;

	r = openArchive("MUSIC.MIX");
	if (!r)
		return false;

	r = openArchive("SFX.MIX");
	if (!r)
		return false;

	r = openArchive("SPCHSFX.TLK");
	if (!r)
		return false;

	initActors();

	return true;
}

void BladeRunnerEngine::initActors() {
	// TODO: Init actors...

	_settings->setChapter(1);
	_settings->setNewSetAndScene(_gameInfo->getInitialSetId(), _gameInfo->getInitialSceneId());
}

void BladeRunnerEngine::shutdown() {
	if (_chapters) {
		if (_chapters->hasOpenResources())
			_chapters->closeResources();
		delete _chapters;
		_chapters = 0;
	}

	if (isArchiveOpen("MUSIC.MIX"))
		closeArchive("MUSIC.MIX");

	if (isArchiveOpen("SFX.MIX"))
		closeArchive("SFX.MIX");

	if (isArchiveOpen("SPCHSFX.TLK"))
		closeArchive("SPCHSFX.TLK");

	if (isArchiveOpen("STARTUP.MIX"))
		closeArchive("STARTUP.MIX");
}

void BladeRunnerEngine::loadSplash() {
	Image img(this);
	if (!img.open("SPLASH.IMG"))
		return;

	img.copyToSurface(&_surface1);

	_system->copyRectToScreen(_surface1.getPixels(), _surface1.pitch, 0, 0, _surface1.w, _surface1.h);
	_system->updateScreen();
}

bool BladeRunnerEngine::init2() {
	return true;
}

void BladeRunnerEngine::gameLoop() {
	_gameIsRunning = true;
	do {
		/* TODO: check player death */
		gameTick();
	} while (_gameIsRunning && !shouldQuit());
}

void BladeRunnerEngine::gameTick() {
	handleEvents();

	if (_gameIsRunning && _windowIsActive) {
		// TODO: Only run if not in Kia, script, nor AI
		_settings->openNewScene();

		// TODO: Autosave
		// TODO: Kia
		// TODO: Spinner
		// TODO: Esper
		// TODO: VK
		// TODO: Elevators
		// TODO: Scores
		// TODO: Call Script_Player_Walked_In if applicable
		// TODO: Gun range announcements
		// TODO: ZBUF repair dirty rects
		// TODO: Tick Ambient Audio (in Replicant)

		// TODO: Advance frame (in Replicant)
		// TODO: Render overlays (mostly in Replicant)
		// TODO: Tick Actor AI and Timers (timers in Replicant)

		if (_settings->getNewScene() == -1 /* || in_script_counter || in_ai */) {

			// TODO: Tick and draw all actors in current set (drawing works in Replicant)
			// TODO: Draw items (drawing works in Replicant)
			// TODO: Draw item pickup (understood, drawing works in Replicant)
			// TODO: Draw dialogue menu
			// TODO: Draw mouse (understood)
			// TODO: Process AUD (audio in Replicant)
			// TODO: Footstep sound

			_system->updateScreen();
		}
	}
}

void BladeRunnerEngine::handleEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
	}
}

void BladeRunnerEngine::playOuttake(int id, bool no_localization) {
	Common::String name = _gameInfo->getOuttake(id);

	if (no_localization)
		name += ".VQA";
	else
		name += "_E.VQA";

	Common::SeekableReadStream *s = getResourceStream(name);
	if (!s)
		return;

	VQADecoder vqa_decoder(s);

	bool b = vqa_decoder.readHeader();
	if (!b) return;

	uint32 frame_count = 0;
	uint32 start_time = 0;
	uint32 next_frame_time = 0;

	for (;;)
	{
		handleEvents();
		// TODO: Handle skips
		if (shouldQuit())
			break;

		uint32 cur_time = next_frame_time + 1;

		if (next_frame_time <= cur_time)
		{
			int frame_number = vqa_decoder.readFrame();
			debug("frame_number: %d", frame_number);

			if (frame_number < 0)
				break;

			b = vqa_decoder.decodeFrame((uint16*)_surface1.getPixels());

			_system->copyRectToScreen(_surface1.getPixels(), _surface1.pitch, 0, 0, _surface1.w, _surface1.h);
			_system->updateScreen();

			++frame_count;

			if (!next_frame_time)
				next_frame_time = cur_time;
			next_frame_time = next_frame_time + (60 * 1000) / 15;
		}
	}
}

bool BladeRunnerEngine::openArchive(const Common::String &name) {
	uint i;

	// If archive is already open, return true
	for (i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name)
			return true;
	}

	// Find first available slot
	for (i = 0; i != kArchiveCount; ++i) {
		if (!_archives[i].isOpen())
			break;
	}
	if (i == kArchiveCount) {
		/* TODO: BLADE.EXE retires the least recently used
		 * archive when it runs out of slots. */

		error("openArchive: No more archive slots");
		return false;
	}

	_archives[i].open(name);
	return _archives[i].isOpen();
}

bool BladeRunnerEngine::closeArchive(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (_archives[i].isOpen() &&_archives[i].getName() == name) {
			_archives[i].close();
			return true;
		}
	}

	debug("closeArchive: Archive %s not open.", name.c_str());
	return false;
}

bool BladeRunnerEngine::isArchiveOpen(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (_archives[i].isOpen() &&_archives[i].getName() == name)
			return true;
	}

	return false;
}

Common::SeekableReadStream *BladeRunnerEngine::getResourceStream(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (!_archives[i].isOpen())
			continue;

		if (false)
			debug("getResource: Searching archive %s for %s.", _archives[i].getName().c_str(), name.c_str());
		Common::SeekableReadStream *stream = _archives[i].createReadStreamForMember(name);
		if (stream)
			return stream;
	}

	debug("getResource: Resource %s not found.", name.c_str());
	return 0;
}

} // End of namespace BladeRunner
