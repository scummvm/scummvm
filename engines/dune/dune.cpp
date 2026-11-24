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

#include "dune/combat.h"
#include "dune/console.h"
#include "dune/dune.h"

#include "dune/hsq.h"
#include "dune/intro.h"
#include "dune/intro_palette.h"
#include "dune/map_mode.h"
#include "dune/music.h"
#include "dune/room.h"
#include "dune/sound_effect.h"
#include "dune/speech_recording.h"
#include "dune/clock.h"
#include "dune/resources.h"
#include "dune/sprite_sheet.h"
#include "dune/video.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"
#include "common/stream.h"
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
	_music = new MusicPlayer(this);
	_console = nullptr;
	_lastHourMined = 0;
	_rnd = new Common::RandomSource("dune_randomseed");
	_troopRoutes.resize(67 * 3);
	memset(_troopRoutes.begin(), 0, _troopRoutes.size());

	_currentBankId   = 0xffff;
	_currentBankData = nullptr;
	_scene           = 0;
}

DuneEngine::~DuneEngine() {
	stopSoundEffect();
	delete _music;
	delete _video;
	delete _rnd;
}

Common::Error DuneEngine::run() {
	_graphics.getInfo(&_screen, &_screenBufferSize);
	initGraphics(kScreenWidth, kScreenHeight);

	if (!_archive.openArchive("DUNE.DAT"))
		return Common::Error(Common::kReadingFailed, "Could not open DUNE.DAT");

	_font.load(_archive, "DNCHAR.BIN");
	_dialogue.load(_archive);
	_condit.load(_archive);
	_map.load(_archive);
	_spiceMap.load(_archive, "MAP2.HSQ");

	_console = new Console(this);
	setDebugger(_console);

	if (!loadStaticData())
		return Common::Error(Common::kReadingFailed,
							 "Dune requires DNCDPRG.EXE from the supported DOS CD release");

	_framebuffer1 = new byte[320*200]{};
	_spokenLine = nullptr;
	_openMovie = 0xffff;
	_pageBefore = nullptr;
	_speakerPerson = -1;
	_framebuffer2 = new byte[320*200]{};

	// The panel is drawn by the intro as well as by the game, so its table
	// has to hold the data's own copy before either of them runs.
	resetPanel();

	_graphics.setFrameBuffer(_framebuffer1);

	// Draws one of the speakers, big, on an empty screen.
	// Lays a run of one bank's sprites out on the screen, so that what a
	// number draws can be seen: dune_sheet names the bank and dune_sprites the
	// first of the run and how many, as "16,24".
	//
	// dune_scene needs the bank as well, to draw the scene's sprites with, so
	// the grid gives way to it rather than taking the screen for its own.
	if (ConfMan.hasKey("dune_sheet") && !ConfMan.hasKey("dune_scene")) {
		int first = 0, many = 24;

		if (ConfMan.hasKey("dune_sprites")) {
			const Common::String run = ConfMan.get("dune_sprites");
			const uint comma = run.find(',');

			first = atoi(Common::String(run.c_str(), comma).c_str());
			if (comma != Common::String::npos)
				many = atoi(run.c_str() + comma + 1);
		}

		memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
		_graphics.setFrameBuffer(_framebuffer1);

		// Several banks - ICONES.HSQ and the desert's pictures among them -
		// hold no colours of their own, so the bank to take them from can be
		// named separately.
		if (ConfMan.hasKey("dune_sky"))
			applySkyPalette(ConfMan.getInt("dune_sky") != 0);
		else if (ConfMan.hasKey("dune_palette"))
			openSpriteSheet(ConfMan.get("dune_palette").c_str());

		openSpriteSheet(ConfMan.get("dune_sheet").c_str(),
						!ConfMan.hasKey("dune_palette") && !ConfMan.hasKey("dune_sky"));

		int x = 2, y = 2, tallest = 0;

		for (int i = first; i != first + many; i++) {
			int wide = 0, high = 0;
			if (!currentSpriteSheet->size(i, &wide, &high))
				continue;

			// A sprite as wide or as tall as the screen only fits at the
			// corner, and the blitters refuse one that hangs over an edge.
			if (x + wide > kScreenWidth) {
				x = 0;
				y += tallest + 2;
				tallest = 0;
			}
			if (wide >= kScreenWidth)
				x = 0;
			if (y + high > kScreenHeight && high >= kScreenHeight)
				y = 0;
			if (y + high >= kScreenHeight)
				break;

			drawSprite(i, x, y);
			debug("sprite %3d at %3d,%3d is %d by %d", i, x, y, wide, high);

			x += wide + 2;
			tallest = MAX(tallest, high);
		}

		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_figure")) {
		const int who = ConfMan.getInt("dune_figure");
		const int frame = ConfMan.hasKey("dune_frame") ? ConfMan.getInt("dune_frame") : 0;
		const int animation = ConfMan.hasKey("dune_animation")
			? ConfMan.getInt("dune_animation") : 0;

		memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
		_graphics.setFrameBuffer(_framebuffer1);

		// dune_frames walks every frame up to the one asked for, the way a
		// scene does, which is how a frame that goes wrong is found.
		if (ConfMan.hasKey("dune_frames")) {
			for (int i = 0; i <= ConfMan.getInt("dune_frames"); i++)
				drawSpeakerFigure(who, i, animation);
		}

		drawSpeakerFigure(who, frame, animation);

		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_playvideo")) {
		playVideo(ConfMan.getInt("dune_playvideo"));
		return Common::kNoError;
	}

	// A movie named by its resource, for the looping ones the desert is made
	// of, which are not in the list of the thirty-four the intro names.
	if (ConfMan.hasKey("dune_playres")) {
		// A looping movie carries no palette of its own; out in the desert it
		// takes the hour's sky.
		if (ConfMan.hasKey("dune_sky"))
			applySkyPalette(ConfMan.getInt("dune_sky") != 0);

		playMovieResource(ConfMan.getInt("dune_playres"), false);
		return Common::kNoError;
	}

	// Write one unpacked archive member for comparing data-driven interface
	// tables with the original executable during reference playthroughs.
	if (ConfMan.hasKey("dune_dumpres")) {
		dumpResource(ConfMan.get("dune_dumpres").c_str());
		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_phrase")) {
		memset(_framebuffer1, 0, 320 * 200);

		int y = 4;
		for (uint i = ConfMan.getInt("dune_phrase"); i != ConfMan.getInt("dune_phrase") + 8u; i++) {
			_font.drawString(_framebuffer1, 4, y, phraseByIndex(i), 15);
			y += _font.glyphHeight() + 3;
		}

		byte palette[3 * 256];
		memset(palette, 0, sizeof(palette));
		for (int i = 0; i != 16; i++)
			palette[3 * i] = palette[3 * i + 1] = palette[3 * i + 2] = 17 * i;
		_system->getPaletteManager()->setPalette(palette, 0, 256);

		_system->copyRectToScreen(_framebuffer1, 320, 0, 0, 320, 200);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	// Locate a resource line by a fragment of its translated text.  This is a
	// diagnostic counterpart to dune_phrase and is useful when matching an
	// automatic observation from a recording whose dialogue record is not yet
	// known.
	if (ConfMan.hasKey("dune_findphrase")) {
		Common::String wanted = ConfMan.get("dune_findphrase");
		wanted.toLowercase();
		for (uint file = 0; file != 2; file++) {
			for (uint line = 0; line != 512; line++) {
				Common::String said = phraseByIndex(line, file != 0);
				if (said.empty())
					continue;
				Common::String folded = said;
				folded.toLowercase();
				if (folded.contains(wanted))
					debug("PHRASE1%u line %u: %s", file + 1, line + 1,
						  said.c_str());
			}
		}
		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_finddialogue")) {
		const uint wanted = (uint)ConfMan.getInt("dune_finddialogue");
		for (uint part = 0; part != _dialogue.count(); part++) {
			const Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint record = 0; record != records.size(); record++) {
				if (records[record].line == wanted)
					debug("dialogue part %u record %u: flags %02x, condition %u (%s)",
						  part, record, records[record].flags, records[record].condition,
						  _condit.describe(records[record].condition).c_str());
			}
		}
		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_world")) {
		// The world laid out flat, in the colours of the map screen.
		openSpriteSheet("ONMAP.HSQ");
		memset(_framebuffer1, 0, 320 * 200);
		_map.drawFlat(_framebuffer1, 320, 200);

		if (ConfMan.hasKey("dune_worldshift")) {
			const byte shift = (byte)ConfMan.getInt("dune_worldshift");
			for (int i = 0; i != 320 * 200; i++)
				if (_framebuffer1[i])
					_framebuffer1[i] += shift;
		}

		_system->copyRectToScreen(_framebuffer1, 320, 0, 0, 320, 200);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_sheetgrid")) {
		// Every sprite of a bank, laid out in rows, to see what is in it.
		openSpriteSheet(ConfMan.get("dune_sheetgrid").c_str());

		const int first = ConfMan.hasKey("dune_first") ? ConfMan.getInt("dune_first") : 0;
		int x = 4, y = 90;

		for (int i = first; i != first + 8; i++) {
			drawSprite(i, x, y);
			x += 40;
		}

		_system->copyRectToScreen(_framebuffer1, 320, 0, 0, 320, 200);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_scene")) {
		openSpriteSheet(ConfMan.get("dune_sheet").c_str());

		Room scenes(this, _archive.openMember(ConfMan.get("dune_sal")));
		debug("%s: %d scenes", ConfMan.get("dune_sal").c_str(), scenes.count());

		const int index = ConfMan.getInt("dune_scene");
		if (index < scenes.count())
			scenes.draw(_framebuffer1, index);

		// Put someone on each of the scene's marks, to see where a scene
		// expects the people in it to stand.
		if (ConfMan.hasKey("dune_people")) {
			openSpriteSheet(ConfMan.get("dune_people").c_str());

			for (const auto &marker : scenes.markers()) {
				const int who = ConfMan.getInt("dune_person") >= 0
								? ConfMan.getInt("dune_person") : (marker.flags >> 9);
				debug("marker at %d,%d flags %04x -> sprite %d", marker.x, marker.y, marker.flags, who);
				drawSprite(who, marker.x, marker.y, marker.flags & ~0x0200, marker.palOffset);
			}
		}

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	// Starting a save from the launcher must not play the whole introduction
	// before that save is restored.  runGame performs the load once all of the
	// game-side objects and frame buffers are ready.
	const bool startingSaved = ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0;
	if (!ConfMan.getBool("dune_nointro") && !startingSaved) {
		runIntro();

		// The intro works on the game's own memory - it goes to places and
		// says who is standing in them - so it leaves it somewhere other than
		// where the game begins.  The game is started from the memory as it
		// came out of the file, which is the palace under the tenth kind.
		// DOS 0009/00BF seeds once before the intro. Its return at0016
		// carries the generators forward, including intro consumption.
		// Restoring scene setup must not draw a second fresh-game seed.
		loadStaticData(false);
	}

	return runGame();
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

	// The colours a picture is drawn in are not always set - a bank of sprites
	// need carry none of its own - so what was drawn cannot always be seen in
	// the dump.  Alongside it goes the screen as it stands, one byte to the
	// pixel, which says what was drawn whatever the colours are.
	if (!ConfMan.getBool("dune_dumpindex"))
		return;

	Common::DumpFile raw;
	if (raw.open(Common::Path(path + ".idx"))) {
		raw.write(buffer, 320 * 200);
		raw.finalize();
	}
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

	Common::Array<byte> code;
	Common::Array<byte> data;
	code.resize(codeSize);
	data.resize(0x10000);
	memset(data.begin(), 0, data.size());
	if (!file.seek(imageOffset) || file.read(code.begin(), codeSize) != codeSize ||
			file.read(data.begin(), dataSize) != dataSize)
		return false;

	_dosCode.swap(code);
	_staticState.swap(data);
	return true;
}

bool DuneEngine::loadStaticData(bool newCampaign) {
	if (_staticState.empty() && !loadExecutableData())
		return false;

	uint16 generators[3] = { 0, 0, 0 };
	byte selectedSky = 0, wideSky = 0;
	if (!newCampaign) {
		if (_state.size() < 0xd82a)
			return false;
		for (uint i = 0; i < ARRAYSIZE(generators); ++i)
			generators[i] = READ_LE_UINT16(_state.begin() + 0xd824 + 2 * i);
		selectedSky = _state[0x46d6];
		wideSky = _state[0x22e3];
	}

	_conversationParentMenu.clear();
	_conversationParentParts.clear();
	_conversationParentFlags.clear();
	_dialogueActionPending = false;
	_localRoomRequestPending = false;
	_pendingRoomExit = -1;
	_pendingRoomCompass = -1;
	_pendingRoomLead = 0;
	_pendingBoardingPlace = -1;
	_pendingBoardingGuided = false;
	_state = _staticState;
	stopSkyFade(); // Startup00B6/DA53 also applies when beginning a new campaign.
	_skyFadeTime = _system->getMillis() / 5;
	_bookEntries.clear();
	_bookPage = -1;
	// DOS startup 00BF..00CD copies one BIOS timer word into all three
	// persistent generators. The captured static segment ends before these
	// words; seed them once for a new game, then let saves carry their states.
	if (newCampaign) {
		const uint16 timerSeed = (uint16)_rnd->getRandomNumber(0xffff);
		WRITE_LE_UINT16(_state.begin() + 0xd824, timerSeed);
		WRITE_LE_UINT16(_state.begin() + 0xd826, timerSeed);
		WRITE_LE_UINT16(_state.begin() + 0xd828, timerSeed);
	} else {
		for (uint i = 0; i < ARRAYSIZE(generators); ++i)
			WRITE_LE_UINT16(_state.begin() + 0xd824 + 2 * i, generators[i]);
		// 0016 starts play without reinitializing the palette selected by
		// narration. Its 3950 cancellation retained that selection and width.
		_state[0x46d6] = selectedSky;
		_state[0x22e3] = wideSky;
	}

	// The captured DOS startup data still holds B2B9's input/modal lock.
	// Native views own their modal state; no DOS startup stack will pop it.
	_state[0x2788] = 0;
	_noticeWarningActive = false;
	_noticeTransitionActive = false;
	stopNoticeCloud();
	_noticeCloudRow = 0;
	_noticeCloudPrevious = 5;

	// 0x029D clears this as the game starts, and the game counts it up and
	// down again around whatever stands over the whole screen; the data it
	// begins with has it at one, which would keep the panel out of every
	// palette the rooms lay down.
	if (_state.size() > 0x227d)
		_state[0x227d] = 0;

	// DOS 0198/B5C5 snaps place longitudes onto their stored terrain point.
	for (uint32 place = 0x100; place + 28 <= _state.size() && _state[place] != 0xff; place += 28)
		WRITE_LE_UINT16(_state.begin() + place + 2,
				_map.snapLongitude(READ_LE_UINT16(_state.begin() + place + 2),
						(int16)READ_LE_UINT16(_state.begin() + place + 4)));
	initialisePlaceSpiceData();
	_map.resetTerrain(_state.begin(), _state.size());
	const uint16 currentPlace = _state.size() > 0x114f
		? READ_LE_UINT16(_state.begin() + 0x114e) : 0;
	if (currentPlace >= 0x100 && currentPlace + 1 < _state.size() && _state.size() > 0x4f)
		WRITE_LE_UINT16(_state.begin() + 0x4e,
						(uint16)((_state[currentPlace] << 8) | _state[currentPlace + 1]));
	// The initial data precedes 0x101CB, which walks each place's
	// linked troop list.  Put the locations into the troop records before the
	// gameplay view reads them.
	_map.restoreTroopLocations(_state.begin(), _state.size());
	_map.readTroops(_state.begin(), _state.size());
	return true;
}

/**
 * Builds the two place bytes which 0x0169 derives from MAP2.HSQ at startup.
 * The density byte under a place is kept at +0x10.  The frequency of that
 * exact byte over the whole density map, divided by sixteen, is kept at
 * +0x11 and later used as the divisor which exhausts a spice deposit.
 *
 * Static data is captured before this pass.  Rebuilding the two immutable
 * bytes after loading also repairs older engine checkpoints which otherwise
 * have zero divisors and therefore inexhaustible spice.
 */
void DuneEngine::initialisePlaceSpiceData() {
	for (uint32 place = 0x100; place + 28 <= _state.size(); place += 28) {
		if (_state[place] == 0xff)
			break;

		const uint16 longitude = READ_LE_UINT16(_state.begin() + place + 2);
		const int16 latitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
		const byte density = _spiceMap.groundAt(longitude, latitude);

		const int32 point = _map.groundOffset(longitude, latitude);
		WRITE_LE_UINT16(_state.begin() + place + 6, point >= 0 ? point : 0);
		_state[place + 0x10] = density;
		_state[place + 0x11] = (byte)((_spiceMap.frequency(density) + 7) >> 4);
	}
}

/**
 * Moves the story on to a stage, which the game only ever does forwards.
 * The original then runs the routine that stage keeps in the table at 0x11E7
 * - which sets up whatever is to happen next - and those are still to be
 * written; what is here is the counting itself, which is what the conditions
 * are asked about.
 */
uint16 DuneEngine::hour() const {
	return _state.size() > 3 ? READ_LE_UINT16(_state.begin() + 2) : 0;
}

/**
 * Moves the hour on when its time has come.  What the original does in its
 * timer interrupt: count down from 12000 ticks, and on becoming negative count
 * the word at DS:0x0002 up by one and the byte at 0x46DD with it.
 */
bool DuneEngine::tick() {
	return tick(true);
}

bool DuneEngine::tick(bool dispatchPending) {
	if (_state.size() <= 0x46de)
		return false;

	const uint32 now = _system->getMillis();
	if (_lastClockTime == 0) {
		_lastClockTime = now;
		// A loaded game can carry an interrupt's pending hour. Initialize the
		// host-clock anchor without postponing that DOS dispatch another frame.
		return advanceClockElapsed(0, false, dispatchPending);
	}

	const uint32 elapsed = now - _lastClockTime;
	_lastClockTime = now;
	// The timer can interrupt more than once before D83E regains control. That
	// is still one 1B0D poll of the pending-hour latch, not a troop round for
	// every interrupt that happened during a stalled frame.
	return advanceClockElapsed(elapsed, false, dispatchPending);
}

/**
 * Adds elapsed real time to the game's clock.  The DOS interrupt makes one
 * game hour when the decremented DS:46DB becomes negative. A reset of 12000
 * therefore lasts 12001 ticks, and the fresh zero counter expires on the
 * first tick. Save both that counter and the fractional five-ms tick.
 */
bool DuneEngine::advanceClock(uint32 elapsedMs) {
	// Explicit scripted fast-forwards represent successive ordinary polls,
	// one per elapsed hour. The real event loop's tick() coalesces a stall.
	return advanceClockElapsed(elapsedMs, true);
}

// The timer interrupt advances the hour counter during speech, but a modal
// lock freezes its countdown. DOS 1B0D leaves 46DD pending at ABCC/2788
// gates and performs one round on the first eligible poll, using the then-
// current hour. Several elapsed hours behind a gate still produce one
// round, not one catch-up round for each value of the pending byte.
bool DuneEngine::dispatchPendingHour() {
	return dispatchPendingHour(false);
}

bool DuneEngine::dispatchPendingHour(bool localNoticeVoiceStarted) {
	// A remote report's DDB0 reading window is a nested wait. Its DD63
	// polls service input/animation but do not re-enter 1B0D, even after
	// the voice ends; the opening/closing pixelation is nested as well.
	// Leave the interrupt's pending hour for the next ordinary poll.
	// The resident's departure paragraph/ABD5 wait similarly retains its
	// caller until 3FC8 moves Paul; expired audio alone cannot release work.
	if (_state.size() <= 0x46de || _state[0x46d9] || !_state[0x46dd] || _state[0x2788] ||
			_pendingRoomExit >= 0 || _pendingRoomCompass >= 0 || _pendingBoardingPlace >= 0 ||
			_menuKind == kMenuTroopNotice || _noticeTransitionActive ||
			(!localNoticeVoiceStarted && _mixer->isSoundHandleActive(_speechHandle)) || storyIsOver())
		return false;
	return dispatchHour();
}

/** DOS 1B23: the hour body, also called directly by FD9's scripted waits. */
bool DuneEngine::dispatchHour() {
	if (_state.size() <= 0x46de || !_state[0x46dd])
		return false;

	_state[0x46dd] = 0;
	// 1B56 updates the day-change byte only after the clock/sky services.
	// Retain the previous hour's value at those callback boundaries.

	// 1B2F recovers one walking tick per processed hour. The comparison is
	// signed after the byte decrement (including zero -> FF); below sixteen
	// it clears both fatigue and Jessica's already-given walking warning.
	byte fatigue = (byte)(_state[0xf4] - 1);
	if ((int8)fatigue < 16) {
		fatigue = 0;
		_state[0xf5] = 0;
	}
	_state[0xf4] = fatigue;
	refreshPanelClock(); // 1B40/1A0F presents the clock before the hour's work.
	retargetSkyFade(); // 1B43/38E1 precedes daily accounting and troop work.

	// 0x1B46 compares the high nibble of this hour with the one kept at
	// DS:0x1174. Crossing a sixteen-hour boundary closes the day before
	// the troops do this hour's work.
	if (_state.size() > 0x1175) {
		const uint16 previous = READ_LE_UINT16(_state.begin() + 0x1174);
		WRITE_LE_UINT16(_state.begin() + 0x1174, hour());
		_state[0x46de] = (byte)((previous & 0xf0) - (hour() & 0xf0));
		if (_state[0x46de])
			dailyRound();
	}

	// DOS 1B5E still performs daily accounting in late war, but skips
	// hourly troop, ecology, cure and battle work from war state seven.
	if (_state[0xc2] < 7) {
		letChaniCure(); // 1B65 precedes the troop round and timed callbacks.
		// 1B68 saves the caller's context across the troop and timed-event
		// callbacks. 1B86 expands that same place again for the next map or
		// conversation; raid target selection must not become the new context.
		const uint16 contextPlace = READ_LE_UINT16(_state.begin() + 0x11ce);
		const uint16 contextPeople = READ_LE_UINT16(_state.begin() + 0x11f7);
		hourlyRound();
		updateVegetationWater();
		if ((hour() & 0xf) == 3)
			updateEmperorSchedule();
		if ((hour() & 0xf) == 4)
			launchHarkonnenRaid();
		// 1DDA/29EE reminds Paul about an active Emperor message when he
		// is away from communications and Duncan is not accompanying him.
		if ((hour() & 0xf) == 8 && (_state[0xbf] & 0x10) &&
			!(READ_LE_UINT16(_state.begin() + 0x10) & 8) &&
			_state[0xb] != 8 && _state[0xc2] == 0)
			queueTroopNotice(0x030b, 0);
		if ((hour() & 0xf) == 15)
			growHarkonnenTroops();
		refreshMapInformation(); // 1B82/1C18, after work and before restoring context.
		if (contextPlace >= 0x100 && (uint32)contextPlace + 28 <= _state.size())
			expandPlaceContext(contextPlace);
		WRITE_LE_UINT16(_state.begin() + 0x11f7, contextPeople);
		finishBattleRound();
	}
	// 1B90/1BA7 discard room-change flags after defeat or when Paul has
	// left the place. A battle ending behind the riding map must not leave
	// a room refresh pending for a later arrival.
	// 1BB8/1BDC consume room-change flags without painting during FD9.
	// The wait's final room restore owns that repaint. Map waits retain
	// their separate 5D6D refresh path rather than discarding the change.
	if (_state[0x46d9] || !READ_LE_UINT16(_state.begin() + 0x114e) ||
			(_state[0x46da] && (int8)_state[0xfb] >= 0))
		_state[0x473b] = 0;
	return true;
}

/** FD9: drain a pending hour, then perform every requested hour's work. */
void DuneEngine::advanceWaitHours(int16 hours) {
	if (_state.size() <= 0x46de)
		return;
	_state[0x46da] = 1;
	_state[0x2788] = 0; // B2BE, including the zero-hour path.
	while (hours > 0) {
		WRITE_LE_UINT16(_state.begin() + 0x46db,
			READ_LE_UINT16(_state.begin() + 0x146e));
		dispatchHour();
		WRITE_LE_UINT16(_state.begin() + 2, hour() + 1);
		_state[0x46dd] = 1;
		// This calls 1B23, not 1B0D: speech/story/defeat gates belong to
		// ordinary polls. FD9 finishes the requested wait even after defeat.
		dispatchHour();
		_state[0x2788] = 0;
		--hours;
	}
	_state[0x46da] = 0;
}

bool DuneEngine::advanceClockElapsed(uint32 elapsedMs, bool dispatchEachHour, bool dispatchPending) {
	if (_state.size() <= 0xceea)
		return false;

	const uint64 elapsed = (uint64)_hourElapsedMs + elapsedMs;
	uint32 ticks = elapsed / 5;
	_hourElapsedMs = elapsed % 5;
	// EF6A freezes the countdown under the modal lock, and positive CEEA
	// freezes the interrupt clock itself. Do not catch these ticks up later.
	if (_state[0x2788] || (int8)_state[0xceea] > 0)
		ticks = 0;
	bool turned = false;

	// EF6A does not test46D9: defeat stops1B0D's campaign dispatch, not
	// the timer interrupt. The countdown still runs during the death movie
	// and final menu; their16C5 clock reset is separate from this gate.
	while (ticks) {
		const uint16 countdown = READ_LE_UINT16(_state.begin() + 0x46db);
		const uint32 remaining = timerTicksUntilHour(countdown);
		if (ticks < remaining) {
			WRITE_LE_UINT16(_state.begin() + 0x46db, countdown - ticks);
			break;
		}
		ticks -= remaining;
		WRITE_LE_UINT16(_state.begin() + 0x46db, READ_LE_UINT16(_state.begin() + 0x146e));

		WRITE_LE_UINT16(_state.begin() + 2, hour() + 1);
		_state[0x46dd]++;
		turned = true;

		debugC(1, kDebugScript, "the hour is now %d", hour());

		// DOS 1B19 returns before consuming the pending hour or updating
		// the daily timestamp once the story is finished.
		if (storyIsOver()) {
			debugC(1, kDebugScript, "the story is over; the world stands still");
			continue;
		}
		if (dispatchEachHour && dispatchPending)
			dispatchPendingHour();
	}
	// This also drains an hour that accumulated while speech was playing,
	// without waiting for another whole game hour to pass.
	if (dispatchPending && dispatchPendingHour())
		turned = true;

	return turned;
}

/**
 * Closes the game's sixteen-hour day.  This is the accounting at 0x1C46:
 * the spice at the beginning of the day is kept at 0x1170, adjustments at
 * 0x1172 are folded in, and today's gain is compared with the previous one
 * at 0xAE.  Duncan's first production report selects its next line from the
 * positive and negative differences at 0xB0 and 0xB2.
 */
/**
 * The illness which puts a sietch in Chani's hands, which the daily accounts
 * reach at 0x1C5F.  0x1E43 asks three things of the day before it strikes:
 * that it has come round to the one kept at DS:0x1156, that the story stands
 * at exactly ninety-two, and that Paul is not standing in Kynes' own sietch
 * while it happens.  It then takes the place its Fremen are tending most and
 * puts 0x400 in every troop record standing there, which is the bit DS:0x5E
 * gathers and the whole of Chani's chapter reads.
 *
 * The outbreak leaves the story at ninety-two. Chani's agreement to stay
 * and treat the ill troops advances it to ninety-three through her dialogue.
 */
void DuneEngine::strikeASietchIll() {
	enum {
		kWhenAtEarliest = 0x1156, kStruckPlace = 0x11db, kStruckCount = 0xf8,
		kKynes = 0x7c8, kNotThisOne = 0x2c0,
		kTroops = 0x8aa, kTroopSize = 27, kIll = 0x0400
	};

	if (_state.size() <= kStruckPlace + 1 || storyStage() != 92)
		return;
	if ((hour() >> 4) < READ_LE_UINT16(_state.begin() + kWhenAtEarliest))
		return;
	if (READ_LE_UINT16(_state.begin() + 0x114e) == kKynes)
		return;

	// 0x1E60 passes over a place of the eighth kind and over one already
	// marked at +0x0A, and 0x1E6C leaves one place alone by name.
	uint32 struck = 0;
	uint most = 0;

	for (uint i = 0; i != _map.places().size(); i++) {
		const uint32 place = 0x100 + 28 * i;

		if (place + 27 > _state.size() || _state[place] == 0xff)
			break;
		if (_state[place + 8] >= 0x28 || (_state[place + 0xa] & 0x80) != 0 ||
				place == kNotThisOne)
			continue;

		// 0x1EA1 counts the troops standing here whose task byte is under
		// eight, which is every one of them at work rather than in transit.
		uint working = 0;

		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		for (uint j = 0; j < standing.size(); j++) {
			const uint32 record = kTroops + standing[j] * kTroopSize;
			if (record + kTroopSize > _state.size())
				break;
			if (_state[record + 3] < 8)
				working++;
		}

		if (working > most) {
			most = working;
			struck = place;
		}
	}

	if (most == 0)
		return;

	WRITE_LE_UINT16(_state.begin() + kStruckPlace, (uint16)struck);
	_state[kStruckCount]++;

	// 1EA9 is called through 661D: the unmarked standing chain only.
	const Common::Array<uint> standing = _map.troopsAt(struck, _state.begin(), _state.size());
	for (uint j = 0; j < standing.size(); j++) {
		const uint32 record = kTroops + standing[j] * kTroopSize;
		if (record + kTroopSize > _state.size())
			break;
		if (_state[record + 3] < 0x80) {
			WRITE_LE_UINT16(_state.begin() + record + 0x12,
				(uint16)(READ_LE_UINT16(_state.begin() + record + 0x12) | kIll));
			_state[record + 3] |= 0x10; // 1EA9 falls through 7085: stop work.
		}
	}

	_map.readTroops(_state.begin(), _state.size());

	debugC(1, kDebugScript, "a strange illness takes the sietch at %04x", (uint)struck);

	// 1E9C/71B2 retain DI, the affected place, not one of its troops.
	queueTroopNotice(0x0f08, (uint16)struck);

	// 1E43/71B2 infects the standing chain and queues its report without
	// advancing the story. Chani's STAY HERE reply (part 62, line 84) runs
	// A219 and advances 92 to 93 when she accepts the cure. Advancing here
	// instead made that reply reach 94, disabling the stage-93 cure routine.
}

/** 1E01: shared by the hourly cure and an accepted STAY request at954C. */
uint16 DuneEngine::chaniCurePlace(uint32 personRecord) {
	if (personRecord + 16 > _state.size() || storyStage() != 93 ||
			_state[personRecord + 14] != 7)
		return 0;
	const uint16 hers = READ_LE_UINT16(_state.begin() + personRecord + 2);
	if ((hers & 0xff) != 0x80 || (hers >> 8) == 0)
		return 0;

	// 0x1E16 settles her in the second room of it while she works.
	_state[personRecord] = 2;
	const uint32 place = 0x100 + 28 * ((hers >> 8) - 1);
	// 0x1E2F asks whether the troop standing there is still ill.
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint j = 0; j < standing.size(); j++) {
		const uint32 record = 0x8aa + standing[j] * 27;
		if (record + 27 > _state.size())
			break;
		if (READ_LE_UINT16(_state.begin() + record + 0x12) & 0x0400)
			return (uint16)place;
	}
	return 0;
}

/**
 * 1B65/1D9F skips cure work while Chani is physically present with Paul.
 * 1EDA adds eight to F9 per processed hour and acts when the byte wraps,
 * clearing0400 illness, setting0800 cured and queuing her spoken report.
 * A fresh counter needs32 calls; accepted STAY's9551 credit reduces it to30.
 */
void DuneEngine::letChaniCure() {
	enum {
		kIll = 0x0400, kCured = 0x0800,
		kTicks = 0xf9, kStruck = 0xf8, kTroops = 0x8aa, kTroopSize = 27
	};
	// 1D9F tests physical presence, unlike954C's accepted STAY request.
	if (_state.size() <= 0x11db || (READ_LE_UINT16(_state.begin() + 0x12) & 0x80))
		return;
	const uint16 place = chaniCurePlace(0x1048);
	if (!place)
		return;

	_state[kTicks] = (byte)(_state[kTicks] + 8);
	if (_state[kTicks] != 0)
		return;

	// 1EE6 uses 6603, which reaches every record on this standing chain.
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint j = 0; j < standing.size(); j++) {
		const uint32 record = kTroops + standing[j] * kTroopSize;
		if (record + kTroopSize > _state.size())
			break;

		const uint16 was = READ_LE_UINT16(_state.begin() + record + 0x12);
		WRITE_LE_UINT16(_state.begin() + record + 0x12,
						(uint16)((was & (uint16)~kIll) | kCured));
	}

	if (_state[kStruck] != 0)
		_state[kStruck]--;

	_map.readTroops(_state.begin(), _state.size());
	debugC(1, kDebugScript, "Chani has cured the sietch at %04x", (uint)place);

	// 0x1EE9 asks for the ninth notice, which is her own word that it is done.
	queueTroopNotice(0x0709, (uint16)place);

	// 0x1EF3 then walks the places over again looking for another still ill,
	// and 0x1F05 keeps what it finds at DS:0x11DB.  Finding none is what ends
	// the chapter: 0x1F0D falls into stage ninety-six's own setup.
	uint32 stillAiling = 0;

	for (uint i = 0; i != _map.places().size() && stillAiling == 0; i++) {
		const uint32 other = 0x100 + 28 * i;

		if (other + 27 > _state.size() || _state[other] == 0xff)
			break;

		const Common::Array<uint> others = _map.troopsAt(other, _state.begin(), _state.size());
		for (uint j = 0; j < others.size(); j++) {
			const uint32 record = kTroops + others[j] * kTroopSize;
			if (record + kTroopSize > _state.size())
				break;
			if ((READ_LE_UINT16(_state.begin() + record + 0x12) & kIll) != 0) {
				stillAiling = other;
				break;
			}
		}
	}

	WRITE_LE_UINT16(_state.begin() + 0x11db, (uint16)stillAiling);

	if (stillAiling == 0)
		runStageSetup(96); // 1F0D calls 11CB directly, not 121F/B17A.
}

/** DOS 1D66: refresh the room assigned to each resident at a known place. */
void DuneEngine::refreshPeopleAtPlaces() {
	// CS:1D35 gives the last room number for each place kind. A resident
	// whose room has passed that limit, or whose place changed kind after a
	// capture, starts again in room one with the new kind.
	const byte *lastRoomByKind = _dosCode.begin() + 0x1d35;
	enum { kPeople = 0xfd8, kPersonSize = 16, kResidents = 12, kPlaceKinds = 49 };
	if (_state.size() < kPeople + kResidents * kPersonSize)
		return;

	for (uint i = 0; i < kResidents; i++) {
		byte *person = _state.begin() + kPeople + i * kPersonSize;
		const uint16 where = READ_LE_UINT16(person + 2);
		if ((where & 0xff) != 0x80 || (where >> 8) == 0xff)
			continue;
		const uint place = 0xe4 + 0x1c * (where >> 8);
		if (place + 8 >= _state.size())
			continue;
		const byte kind = _state[place + 8];
		if (kind >= kPlaceKinds)
			continue;
		if (person[1] != kind || person[0] > lastRoomByKind[kind]) {
			person[0] = 1;
			person[1] = kind;
		}
	}
}

/** DOS 1CDA: the daily Harkonnen spice estimate from uncaptured forts. */
void DuneEngine::updateHarkonnenProduction() {
	uint16 production = 0;
	for (uint place = 0x100; place + 28 <= _state.size() && _state[place] != 0xff;
			place += 28) {
		if (_state[place + 8] >= 0x28 && !(_state[place + 0xa] & 8))
			production += _state[place + 0x12] >> 3;
	}
	// E3DF uses the third saved generator and rejection sampling. A zero
	// bound returns zero without consuming the stream.
	const uint16 limit = production >> 4;
	if (limit) {
		uint16 mask = 1;
		while (mask < limit)
			mask = (mask << 1) | 1;
		uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828), value;
		do {
			value = combatRandom(seed, 0xcbd1) & mask;
		} while (value > limit);
		WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
		production += value;
	}
	WRITE_LE_UINT16(_state.begin() + 0xa8, production);
}

/** DOS C04E: rebuild the two troop-population totals used by dialogue and results. */
void DuneEngine::updateTroopPopulations() {
	if (_state.size() <= 0xad)
		return;
	uint16 alliedPopulation = 0, hostilePopulation = 0;
	for (uint i = 0; i < _map.troops().size(); i++) {
		const uint32 record = 0x8aa + i * 27;
		if (record + 27 > _state.size() || _state[record] == 0)
			break;
		if (_state[record + 3] & 0x20)
			continue;
		const byte population = _state[record + 0x1a];
		if (_state[record + 0x10] & 0x80)
			hostilePopulation += population;
		else if (!(_state[record + 3] & 0x80))
			alliedPopulation += population;
	}
	WRITE_LE_UINT16(_state.begin() + 0xaa, alliedPopulation);
	WRITE_LE_UINT16(_state.begin() + 0xac, hostilePopulation);
}

/** DOS C02E/BFE3: refresh the territory, production and army results together. */
void DuneEngine::updateResultsTotals() {
	if (_state.size() <= 0x1173)
		return;
	uint16 harkShare, ourShare;
	_map.resultsShares(harkShare, ourShare);
	WRITE_LE_UINT16(_state.begin() + 0xa2, ourShare);
	WRITE_LE_UINT16(_state.begin() + 0xa4, harkShare);

	// C02E performs the addition in a word before subtracting the saved
	// beginning-of-day stock. Its result can raise, but not lower, A6.
	const uint16 current = (uint16)(spice() + READ_LE_UINT16(_state.begin() + 0x1172));
	const uint16 before = READ_LE_UINT16(_state.begin() + 0x1170);
	const uint16 produced = current >= before ? current - before : 0;
	if (produced >= READ_LE_UINT16(_state.begin() + 0xa6))
		WRITE_LE_UINT16(_state.begin() + 0xa6, produced);
	updateTroopPopulations();
}

/** DOS BF26/BF7D: retain six previous values and their direction symbols. */
void DuneEngine::updateResultsTrends() {
	if (_state.size() <= 0x116f)
		return;
	static const uint kCurrent[6] = { 0xa4, 0xa2, 0xa8, 0xa6, 0xac, 0xaa };
	const bool sameDay = READ_LE_UINT16(_state.begin() + 0x115c) == (hour() & 0xfff0);
	for (uint i = 0; i < ARRAYSIZE(kCurrent); i++) {
		const uint16 current = READ_LE_UINT16(_state.begin() + kCurrent[i]);
		const uint16 previous = READ_LE_UINT16(_state.begin() + 0x115e + i * 2);
		WRITE_LE_UINT16(_state.begin() + 0x115e + i * 2, current);
		if (current != previous)
			_state[0x116a + i] = current < previous ? 2 : 1;
		else if (!sameDay)
			_state[0x116a + i] = 3;
	}
}

void DuneEngine::dailyRound() {
	if (_state.size() <= 0x1175)
		return;

	if (_state[0xfe] != _state[0x2a])
		_state[0xff] = 0;
	_state[0xfe] = _state[0x2a];
	_state[0xff]++;

	refreshPeopleAtPlaces();
	strikeASietchIll();

	// 0x1C62 advances the contact-range dialogue latch once per Dune day.  Zero
	// and 0xFF are terminal values (their wrapped successors are not stored);
	// every other value climbs by one.  Leaving this out kept Jessica's
	// post-worm powers conversation permanently below condition 60 even after
	// the several days which separate its two appearances in the recording.
	const byte nextContactLatch = (byte)(_state[0xd5] + 1);
	if (nextContactLatch >= 2)
		_state[0xd5] = nextContactLatch;

	const uint16 adjustments = READ_LE_UINT16(_state.begin() + 0x1172);
	WRITE_LE_UINT16(_state.begin() + 0x1172, 0);

	const uint16 now = spice();
	const uint16 before = READ_LE_UINT16(_state.begin() + 0x1170);
	WRITE_LE_UINT16(_state.begin() + 0x1170, now);

	// 1C78 adds into AX: retain its sixteen-bit wrap before 1C7E's
	// unsigned subtraction, rather than saturating a wider production total.
	const uint16 withAdjustments = (uint16)(adjustments + now);
	const uint16 produced = withAdjustments >= before
		? withAdjustments - before : 0;
	WRITE_LE_UINT16(_state.begin() + 0xa6, produced);

	const uint16 previous = READ_LE_UINT16(_state.begin() + 0xae);
	WRITE_LE_UINT16(_state.begin() + 0xae, produced);

	if (previous < produced) {
		WRITE_LE_UINT16(_state.begin() + 0xb0, produced - previous);
		WRITE_LE_UINT16(_state.begin() + 0xb2, 0);
	} else {
		WRITE_LE_UINT16(_state.begin() + 0xb0, 0);
		WRITE_LE_UINT16(_state.begin() + 0xb2, previous - produced);
	}
	updateHarkonnenProduction();
	updateResultsTotals();
	updateResultsTrends();

	// 1CA6 calls E3CC once on every Dune day, even if no village can restock.
	// This is the same saved D826 generator used by mining hazards; drawing from
	// an unrelated source changed the next hazard after every day boundary.
	// BX then rotates twice for each eligible empty stock slot, descending from
	// item four to zero in every visited smuggler account.
	if (_state.size() > 0x1141) {
		uint16 random = troopRandom();
		for (uint32 account = 0x10d8;
				account + 17 <= _state.size() && _state[account] < 20;
				account += 17) {
			if ((_state[account + 2] & 8) == 0)
				continue;
			for (int item = 4; item >= 0; item--) {
				if (_state[account + 4 + item] != 0 ||
						(int8)_state[account + 9 + item] >= 0)
					continue;
				random = (uint16)((random << 2) | (random >> 14));
				_state[account + 4 + item] = (byte)(random & 3);
			}
		}
	}

	debugC(1, kDebugScript, "the day closes with %u spice made (%+d from the day before)",
		   produced, (int)produced - previous);
}

/**
 * DOS 0x6EFD: effective work rate, also shown in troop report context. Bonuses
 * use byte arithmetic. Ecology tasks 8/9 use 100 regardless of stored morale;
 * other tasks are capped at 100 except an attack away from Paul. The fear
 * penalty at stages 100..103 is applied last, using a signed byte comparison.
 * None of these adjustments changes the troop's stored motivation.
 */
byte DuneEngine::troopRate(const Troop &troop) const {
	if (_state.size() <= 0x114f)
		return 0;

	byte rate = troop.rate;
	const byte order = troop.order();

	if (_state[0xfa])
		rate += 20;

	bool capped = true;

	if (order == 6) {
		if (troop.place == READ_LE_UINT16(_state.begin() + 0x114e))
			rate += 30;
		else
			capped = false;
	} else if ((order & 0xfe) == 8) {
		rate = 100;
	}

	if (capped)
		rate = MIN<byte>(rate, 100);

	const byte stage = _state[0x2a];
	if (stage >= 100 && stage < 104) {
		rate -= 40;
		if ((int8)rate < 10)
			rate = 10;
	}

	return rate;
}

/**
 * Adds to what Paul is thought of, the way the routine at 0x6F78 does: never
 * past 200, and something happens each time it crosses a fourth of the way.
 */
void DuneEngine::addCharisma(byte much) {
	if (_state.size() <= 0x29)
		return;

	const byte was = _state[0x29];
	_state[0x29] = MIN<byte>((byte)(was + much), 200);

	// 0x6F87 turns each four points Paul gains into one point of motivation
	// for every troop already sworn to him.  Omitting this made Tuono's troop
	// stay at 30% after the later rallies where the reference has reached 31%.
	const byte raised = (byte)((_state[0x29] & 0xfc) - (was & 0xfc)) >> 2;
	if (raised != 0) {
		// Only the sixty-seven troop records occupy DS:0x8AA..0xFBB.  Walking
		// in 27-byte strides to the end of the whole 64K segment treated room
		// pointers and every later table as troops whenever charisma crossed a
		// four-point boundary.  One such rally changed place-kind seven's room
		// table pointer from 0x12B1 to 0x1264, so leaving Habbanya-Tuek returned
		// to a black self-looping room.
		for (uint i = 0; i < _map.troops().size(); i++) {
			const uint32 at = 0x8aa + i * 27;
			if (at + 27 > _state.size())
				break;
			if ((_state[at + 3] & 0xa0) == 0)
				_state[at + 0x15] = MIN<byte>((byte)(_state[at + 0x15] + raised), 100);
		}
		_map.readTroops(_state.begin(), _state.size());
	}

	if ((_state[0x29] & 0xfc) != (was & 0xfc))
		debugC(1, kDebugScript, "Paul is thought better of: %d", _state[0x29]);
}

uint16 DuneEngine::spice() const {
	return _state.size() > 0xa1 ? READ_LE_UINT16(_state.begin() + 0xa0) : 0;
}

/**
 * An hour of spice mining, which is what order 0 sets a troop to.  The
 * arithmetic is the routine at 0x708A and the one that calls it at 0x6FE5,
 * followed as they stand:
 *
 *   the rate, plus the high nibble of the troop's skill, times how many of
 *   them there are, quartered unless the flag at +0x19 says otherwise;
 *   then taken against the high nibble of what the place still holds, the
 *   high byte kept and turned about, doubled, and held to nine bits.
 *
 * What comes of it is added to the troop's own running total, a tenth of it
 * goes to the player's spice with the remainder kept for the next hour, and
 * the place is drawn down by as much as its ground will give up.
 */
uint16 DuneEngine::spiceProduction(uint index) const {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return 0;

	const uint32 at = 0x8aa + index * 27;
	if (at + 27 > _state.size())
		return 0;
	// 708A reads SI after 714C's hazard and the daily troop update. A casualty
	// can reduce +1A before this hour's multiplication, so the map's cached
	// Troop (refreshed only after the full dispatch) is not authoritative here.
	Troop troop = troops[index];
	troop.state = _state[at + 3];
	troop.place = READ_LE_UINT16(_state.begin() + at + 4);
	troop.rate = _state[at + 0x15];
	troop.skill = _state[at + 0x16];
	troop.size = _state[at + 0x1a];

	// The place the troop is at, as an offset into the game's memory.
	const uint32 place = troop.place;
	if (place < 0x100 || place + 0x14 > _state.size())
		return 0;

	byte al = (byte)(troopRate(troop) + (troop.skill & 0xf0));
	uint16 ax = (uint16)al * troop.size;

	if (!(_state[at + 0x19] & 0x80))
		ax >>= 2;

	const byte high = ax >> 8;
	ax = (uint16)(((_state[place + 0x12] & 0xf0) + 1) & 0xff) * high;
	ax = (ax >> 8) | (ax << 8);
	ax = (ax << 1) | (ax >> 15);
	ax &= 0x1ff;

	return ax;
}

void DuneEngine::mineSpice(uint index) {
	dailyTroopUpdate(index); // 6FE5, before checking whether mining can work.
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const Troop &troop = troops[index];
	const uint32 place = troop.place;
	if (place < 0x100 || place + 0x14 > _state.size())
		return;
	const uint32 at = 0x8aa + index * 27;
	if (at + 27 > _state.size())
		return;

	// 705C/7074 retries a worm-stopped miner only when the low four bits of
	// the hour match its id. Between those attempts DOS reports zero current
	// yield while keeping the cumulative tally. A successful retry joins
	// 6FFA after 714C, so it must not draw another hazard in the same hour.
	bool retryWaiting = false;
	if (READ_LE_UINT16(_state.begin() + at + 0x10) & 0x0200) {
		if (((hour() ^ _state[at]) & 0x0f) != 0) {
			WRITE_LE_UINT16(_state.begin() + at + 0xc, 0);
			_state[at + 3] |= 0x10;
			return;
		}
		WRITE_LE_UINT16(_state.begin() + at + 0x10,
			READ_LE_UINT16(_state.begin() + at + 0x10) & ~0x0200);
		retryWaiting = true;
	}

	if (!updateTroopWorkState(index)) {
		// Ordinary 6FEF failure enters 707B: the blocked miner reports no
		// current output and loses its cumulative work tally. The 705C retry
		// path instead returns at 705B and preserves both fields.
		if (!retryWaiting) {
			WRITE_LE_UINT16(_state.begin() + at + 0xc, 0);
			WRITE_LE_UINT16(_state.begin() + at + 0xe, 0);
			_state[at + 3] |= 0x10;
		}
		debugC(2, kDebugScript, "troop %u cannot mine at %04x yet",
			   index, place);
		return;
	}

	if (!retryWaiting)
		miningHazard(index);
	const uint16 ax = spiceProduction(index);

	if (ax == 0) {
		debugC(2, kDebugScript, "troop %u mines nothing: %04x holds %u",
			   index, place, _state[place + 0x12]);
	}
	// DOS 6FFA/708A still marks the job active and exchanges the current
	// yield with zero. Returning here leaves yesterday's production in the
	// troop report and misses the falling-yield flag when a deposit runs low.

	// 6FFA marks active extraction even when the computed yield is zero. The
	// job's start hour and tally are reset by 6B25 when it is assigned or
	// released from waiting; 6FE5 does not reset them just because this bit
	// was absent before the current hour.
	WRITE_LE_UINT16(_state.begin() + at + 0x10,
					READ_LE_UINT16(_state.begin() + at + 0x10) | 0x0100);
	// The separate +0x13 bit 0x20 is set by 6B06 when the mining order is
	// assigned. DOS 6FE5 does not recreate it every productive hour.

	// 0x708A keeps the current hourly production at +0x0C and marks whether it
	// has risen (8) or fallen (4).  Those two bits select the chief's matching
	// report; without them the dialogue jumped straight from troop statistics
	// to the cumulative total.
	const uint16 previousProduction = READ_LE_UINT16(_state.begin() + at + 0xc);
	WRITE_LE_UINT16(_state.begin() + at + 0xc, ax);
	if (previousProduction != ax) {
		byte &flags = _state[at + 0x10];
		flags = (byte)((flags & 0xf3) | (previousProduction < ax ? 8 : 4));
	}

	// The troop's own tally.  Each time its upper nine bits change, 0x7012
	// advances its mining skill by one; the first 128-kilo boundary is what
	// leaves Tuono's troop just below the next experience band in the video.
	const uint16 oldTally = READ_LE_UINT16(_state.begin() + at + 0xe);
	const uint16 newTally = (uint16)(oldTally + ax);
	WRITE_LE_UINT16(_state.begin() + at + 0xe, newTally);
	if (((oldTally ^ newTally) & 0xff80) != 0)
		raiseSkill(index, 0, 1);

	// A tenth of it reaches the player, the rest waits for the next hour.
	const uint16 carried = READ_LE_UINT16(_state.begin() + 0x46e1) + ax;
	WRITE_LE_UINT16(_state.begin() + 0x46e1, carried % 10);
	WRITE_LE_UINT16(_state.begin() + 0xa0, spice() + carried / 10);

	// And the place gives up what its ground allows, keeping the remainder.
	const byte ground = _state[place + 0x11];
	if (ground) {
		const uint16 taken = ax + _state[place + 0x13];
		_state[place + 0x13] = (byte)(taken % ground);

		const uint16 depletion = taken / ground;
		// 7043..704E requests a repaint when the old density's low nibble
		// is exhausted. Keep the unbounded quotient for this test, before
		// saturating the remaining deposit; INC byte also wraps FF to zero.
		if (depletion > (_state[place + 0x12] & 0x0f) && (_state[0x46eb] & 0x40))
			_state[0x46ec]++;
		const byte given = (byte)MIN<uint16>(depletion, _state[place + 0x12]);
		_state[place + 0x12] -= given;
	}

	_lastHourMined += ax;

	debugC(2, kDebugScript, "troop %u mined %d, the player has %d spice", index, ax, spice());
}

/**
 * Risks the equipment of a spice-mining troop once every sixteen hours. This
 * is 0x714C..0x71BB: only a troop whose id shares the hour's low nibble is
 * tested, an ornithopter protects its harvester, and the DOS random stream
 * chooses between casualties, an interrupted job and a swallowed harvester.
 */
void DuneEngine::miningHazard(uint index) {
	enum {
		kTroops = 0x8aa, kTroopSize = 27, kHarvester = 0x80,
		kOrnithopter = 0x40, kRandomSeed = 0xd826
	};

	if (index >= _map.troops().size() || _state.size() <= kRandomSeed + 1 ||
		!(_state[0xa] & 1))
		return;

	const uint32 record = kTroops + index * kTroopSize;
	const Troop &troop = _map.troops()[index];
	if (record + kTroopSize > _state.size() || troop.place < 0x100 ||
		troop.place + 0x14 >= _state.size() ||
		!(_state[record + 0x19] & kHarvester) ||
		((hour() ^ _state[record]) & 0x0f) != 0)
		return;

	// 71BC precedes E3CC's ordinary hazard draw. After stage 53 a miner
	// carrying the +12 bit 0x40 rotates DS:0 three times; one phase in eight
	// calls 719C and marks both the troop and its sietch for a worm incident.
	// Travel uses this same word, so skipping the check changes later routes as
	// well as losing the notice. Its event is 0F03, not the lost-harvester 0F06.
	if (storyStage() >= 53 &&
		(READ_LE_UINT16(_state.begin() + record + 0x12) & 0x40)) {
		const uint16 bits = READ_LE_UINT16(_state.begin());
		const uint16 rotated = (uint16)((bits << 3) | (bits >> 13));
		WRITE_LE_UINT16(_state.begin(), rotated);
		if ((rotated & 7) == 0) {
			WRITE_LE_UINT16(_state.begin() + record + 0x10,
				READ_LE_UINT16(_state.begin() + record + 0x10) | 0x8200);
			_state[record + 3] |= 0x10;
			_state[troop.place + 0xa] |= 4;
			queueTroopNotice(0x0f03, troop.place);
		}
	}

	// 0xE3CC multiplies the word at D826 by CBD1, increments its low word,
	// stores that word, and returns bits 16..23 in AH.
	const uint16 seed = READ_LE_UINT16(_state.begin() + kRandomSeed);
	const uint32 product = (uint32)seed * 0xcbd1;
	WRITE_LE_UINT16(_state.begin() + kRandomSeed, (uint16)(product + 1));
	const byte random = (byte)(product >> 16);

	const byte region = _state[troop.place];
	const byte chance = 0x1141 + region < _state.size() ? _state[0x1141 + region] : 0;
	if (chance < random)
		return;

	WRITE_LE_UINT16(_state.begin() + record + 0x10,
					READ_LE_UINT16(_state.begin() + record + 0x10) | 0x4000);
	if (_state[record + 0x19] & kOrnithopter)
		return;

	switch (random & 3) {
	case 1:
		WRITE_LE_UINT16(_state.begin() + record + 0x10,
					READ_LE_UINT16(_state.begin() + record + 0x10) | 0x2000);
		if (_state[record + 0x1a] > 2)
			_state[record + 0x1a] -= 2;
		break;
	case 2:
		WRITE_LE_UINT16(_state.begin() + record + 0x10,
					READ_LE_UINT16(_state.begin() + record + 0x10) | 0x0200);
		_state[record + 3] |= 0x10;
		break;
	case 3:
		WRITE_LE_UINT16(_state.begin() + record + 0x10,
					READ_LE_UINT16(_state.begin() + record + 0x10) | 0x1000);
		_state[record + 0x19] &= (byte)~kHarvester;
		// 71AD decrements the sietch cache unconditionally. A zero cache
		// wraps just as the DOS byte decrement does.
		_state[troop.place + 0x14]--;
		queueTroopNotice(0x0f06, troop.place);
		debugC(1, kDebugScript, "a worm swallows troop %u's harvester", index);
		break;
	default:
		break;
	}
}

/** Keeps an automatic event in the original four-byte queue at 0x1190. */
void DuneEngine::queueTroopNotice(uint16 what, uint16 place) {
	enum { kQueue = 0x1190, kDepth = 10, kEntrySize = 4 };
	if (_state.size() <= kQueue + 1 + kDepth * kEntrySize || !(_state[0xa] & 1))
		return;

	uint count = MIN<uint>(_state[kQueue], kDepth);
	for (uint i = 0; i != count; i++) {
		const uint32 at = kQueue + 1 + i * kEntrySize;
		if (READ_LE_UINT16(_state.begin() + at) == what &&
			READ_LE_UINT16(_state.begin() + at + 2) == place)
			return;
	}

	if (count == kDepth) {
		consumeTroopNotice();
		count--;
	}

	const uint32 at = kQueue + 1 + count * kEntrySize;
	WRITE_LE_UINT16(_state.begin() + at, what);
	WRITE_LE_UINT16(_state.begin() + at + 2, place);
	_state[kQueue] = count + 1;
}

/** 2A34 moves the entire fixed-depth buffer and clears its vacated tail. */
void DuneEngine::consumeTroopNotice() {
	enum { kQueue = 0x1190, kDepth = 10, kEntrySize = 4 };
	const uint count = MIN<uint>(_state[kQueue], kDepth);
	if (!count)
		return;
	_state[kQueue] = count - 1;
	if (count == 1)
		_state[kQueue - 1] = 0;
	memmove(_state.begin() + kQueue + 1, _state.begin() + kQueue + 1 + kEntrySize,
		(kDepth - 1) * kEntrySize);
	memset(_state.begin() + kQueue + 1 + (kDepth - 1) * kEntrySize, 0, kEntrySize);
}

/**
 * An hour of prospecting, which is order 1.  The routine at 0x70CC works out
 * how long the ground will take - sixteen times what the place stands on,
 * divided by the troop's rate and skill together - remembers it at +0x0C, and
 * counts the hours since the troop set to work, which it keeps at +0x0A.
 * While it is not done it puts how far along it is, in hundredths, at +0x0E;
 * when the hours have run it marks the place with 0x40 and there is no more
 * prospecting to do there.
 */
void DuneEngine::prospect(uint index) {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 place = troops[index].place;
	const uint32 at = 0x8aa + index * 27;

	if (place < 0x100 || place + 0x12 > _state.size())
		return;

	if (_state[place + 0xa] & 2)
		return;
	if (!(_state[place + 0xa] & 0x40)) {
		if (!updateTroopWorkState(index))
			return;
		uint16 needed = READ_LE_UINT16(_state.begin() + at + 0xc);
		if (!needed) {
			const byte worth = (byte)(_state[at + 0x15] + _state[at + 0x16]);
			if (!worth)
				return;
			needed = (uint16)((_state[place + 0x11] << 4) / worth);
			WRITE_LE_UINT16(_state.begin() + at + 0xc, needed);
		}
		const uint16 since = hour() - READ_LE_UINT16(_state.begin() + at + 0xa);
		if (needed > since) {
			if (since)
				WRITE_LE_UINT16(_state.begin() + at + 0xe, (uint16)((uint32)since * 100 / needed));
			return;
		}
		// 7110 awards TWO spice-skill points, then completes the same hourly
		// call through 7126. Waiting for another hour delays the next journey.
		raiseSkill(index, 0, 2);
		_state[place + 0xa] |= 0x40;
		if (_state[0x46eb] & 0x40)
			_state[0x46ec]++;
		_map.readPlaces(_state.begin(), _state.size());
		debugC(1, kDebugScript, "troop %u has prospected the place at %04x", index, place);
	}
	const byte previousTask = _state[at + 3];
	updateTroopWorkState(index);
	WRITE_LE_UINT16(_state.begin() + at + 0xe, 100);
	if (READ_LE_UINT16(_state.begin() + 0x46ef) == at)
		return; // The contacted chief finishes the survey but waits for Paul.
	if (READ_LE_UINT16(_state.begin() + 0x11d3)) {
		if (index != 2) {
			// 7149 retains DI for an ordinary surveyor; only record 08E0
			// substitutes the specialist's global destination in 84A6.
			_troopRoutes[index * 3] = (place - 0x100) / 28 + 1;
		}
		beginTroopRoute(index);
	} else if (!(previousTask & 0x10) && (_state[0xa] & 1)) {
		queueTroopNotice(0x0f0e, place);
	}
}

/**
 * Makes a troop better at something, as the routine at 0x6EDD does: the
 * skills sit one after another from +0x16, none of them going past 0x5F, and
 * when one crosses into a new sixteenth the troop remembers which it was in
 * the word at +0x10 so that it can be spoken of.
 */
void DuneEngine::raiseSkill(uint index, uint which, byte by) {
	const uint32 at = 0x8aa + index * 27 + 0x16 + which;
	if (at >= _state.size())
		return;

	const byte was = _state[at];
	_state[at] = MIN<byte>((byte)(was + by), 0x5f);

	if (((_state[at] ^ was) & 0xf0) == 0)
		return;

	const uint32 flags = 0x8aa + index * 27 + 0x10;
	if (flags + 1 < _state.size())
		_state[flags] = (byte)((_state[flags] & 0xfc) | ((which + 1) & 3));

	debugC(1, kDebugScript, "troop %u is better at its %u%s work: %d", index, which,
		   which == 0 ? "st" : "th", _state[at]);
}

/**
 * An hour of military training, which is order 4.  The routine at 0x71EF
 * counts down the hours at +0x0C, and when they run out the troop is a little
 * better at soldiering and the next stretch is worked out: how far it still
 * has to go, doubled, plus its rate but never under thirty, divided into 200,
 * 250, 300 or 400 according to what it has been equipped with at +0x19.
 *
 * Gurney's location supplies a target of 160; otherwise 0x7298 averages only
 * the active training troops in this place's chain. A sabotaged location
 * instead runs the recovery countdown at 0x725F, without increasing skill.
 */
void DuneEngine::train(uint index) {
	dailyTroopUpdate(index); // 71EF, including troops with no training progress.
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 at = 0x8aa + index * 27;
	const uint32 place = troops[index].place;
	if (at + 27 > _state.size() || place < 0x100 || place + 28 > _state.size())
		return;

	_state[at + 0x11] &= 0xfd; // 71F2: training clears interrupted-mining repair.
	if (_state[place + 0xa] & 4) {
		// FF in the high byte identifies an already-running recovery. Only
		// the low byte counts down, with DOS byte wrap and a signed end test.
		uint16 recovery = READ_LE_UINT16(_state.begin() + at + 0xe);
		if ((recovery >> 8) != 0xff)
			recovery = 0xff00 | (byte)(0x40 - _state[at + 0x17]);
		const byte remaining = (byte)(recovery - 1);
		if ((int8)remaining >= 0) {
			WRITE_LE_UINT16(_state.begin() + at + 0xe, 0xff00 | remaining);
			return;
		}
		// 727D/7289 removes the sabotage and exposes any marked, visible
		// traitors at this place. Only the recovering troop receives the report.
		_state[place + 0xa] &= 0xfb;
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		bool foundTraitor = false;
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			if (_state[other + 3] < 0x80 && (_state[other + 0x12] & 0x40)) {
				_state[other + 0x12] &= 0xbf;
				foundTraitor = true;
			}
		}
		if (foundTraitor)
			_state[at + 0x13] |= 1;
		WRITE_LE_UINT16(_state.begin() + at + 0xe, 0);
		_map.readPlaces(_state.begin(), _state.size());
		return;
	}

	// 71FD tests the sign AFTER decrementing the word, including wrap.
	const uint16 left = READ_LE_UINT16(_state.begin() + at + 0xc) - 1;
	WRITE_LE_UINT16(_state.begin() + at + 0xc, left);
	if ((int16)left >= 0)
		return;

	int target = 0;

	if (place == READ_LE_UINT16(_state.begin() + 0x473c)) {
		_state[at + 0x11] |= 8; // 7209: report Gurney's help after progress.
		target = 0xa0;
	} else {
		uint count = 0, total = 0;
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			// The exact task byte excludes waiting, travelling and hidden
			// trainers; masking to the occupation would count them as teachers.
			if (_state[other + 3] != 4)
				continue;

			count++;
			total += _state[other + 0x17];
		}

		target = count ? (int)(total / count) : 0;
	}

	int togo = target - _state[at + 0x17];
	if (togo < 0)
		togo = 0;

	int worth = MAX<int>(troopRate(troops[index]), 30);
	int over = 2 * togo + worth;

	const byte kit = _state[at + 0x19];
	int span = 200;
	if (!(kit & 0x0c)) {
		span = 250;
		if (!(kit & 0x10)) {
			span = 300;
			if (!(kit & 0x20))
				span = 400;
		}
	}

	WRITE_LE_UINT16(_state.begin() + at + 0xc, over ? (uint16)(span / over) : 0);
	raiseSkill(index, 1, 1);
}

/** DOS 72B0: staged espionage reports followed by an independent capture check. */
void DuneEngine::spy(uint index) {
	const uint at = 0x8aa + index * 27;
	if (index >= _map.troops().size() || at + 27 > _state.size())
		return;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place < 0x100 || place + 28 > _state.size())
		return;

	// SUB AL,skill; CBW; two arithmetic shifts. Report comparisons are
	// signed, whereas the later exposure check compares elapsed time unsigned.
	// Discovery is an arrival effect at 83A0, not a side effect of this work.
	const uint16 gone = hour() - READ_LE_UINT16(_state.begin() + at + 0xa);
	const byte skill = _state[at + 0x17];
	const int16 halfDelay = (int8)(0x50 - skill) >> 1;
	if (!(_state[at + 0x10] & 0x40) && (halfDelay >> 1) < (int16)gone) {
		uint16 counted = READ_LE_UINT16(_state.begin() + at + 0xc);
		if (!counted) {
			// 68D2 counts and clears unseen flags in the actual standing
			// chain, including hidden/broken groups, not just enemy records.
			const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
			for (uint i = 0; i < standing.size(); i++) {
				const uint other = 0x8aa + standing[i] * 27;
				if (_state[other + 0x10] & 0x10) {
					_state[other + 0x10] &= 0xef;
					counted++;
					registerMapUnit(standing[i]); // 68E6/846C paints the discovered group immediately.
				}
			}
			WRITE_LE_UINT16(_state.begin() + at + 0xc, counted);
		}
		if (!counted) {
			_state[at + 0x10] |= 0x40; // 72F3: an empty force is a complete report.
		} else if (halfDelay < (int16)gone) {
			gatherTroopStrengths(place);
			WRITE_LE_UINT16(_state.begin() + at + 0xe,
					READ_LE_UINT16(_state.begin() + 0x94) / counted);
			_state[at + 0x10] |= 0x40;
		}
	}

	// 72F7/7313: the chain head is safe; any other spy can be captured after
	// its grace period, even before a report or after reporting is finished.
	if (_state[at] != _state[place + 9] && (skill >> 1) < gone &&
			(troopRandom() & 0x3f) >= skill)
		defeatTroop(index);
	_map.readTroops(_state.begin(), _state.size());
}

/**
 * An hour of irrigation and tree care, which is order 8.  The routine at
 * 0x7693 sets a place to it the first time: it marks the place with 1 in the
 * byte at +0x0A, empties what it held at +0x12, copies where it stands into
 * +0x0C and +0x0E and puts 4 at +0x0B.  Every hour after that a quarter of
 * the troop's skill at +0x18, and never less than one, is added to what is
 * growing there at +0x0F.
 */
void DuneEngine::irrigate(uint index) {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 at = 0x8aa + index * 27;
	const uint32 place = troops[index].place;

	if (place < 0x100 || place + 28 > _state.size() || at + 27 > _state.size())
		return;

	// 7693 does not automatically resume an order that has stopped for water
	// or equipment. A new order/equipment transition must recheck it first.
	if ((_state[at + 3] & 0x10) || !updateTroopWorkState(index))
		return;
	_state[0x4737] = 0xff;
	_state[at + 0x11] |= 1; // 0x76A3: vegetation work has begun.

	if (!(_state[place + 0xa] & 1)) {
		_state[place + 0xa] |= 1;
		_state[place + 0x12] = 0;

		WRITE_LE_UINT16(_state.begin() + place + 0xc, READ_LE_UINT16(_state.begin() + place + 2));
		WRITE_LE_UINT16(_state.begin() + place + 0xe,
						READ_LE_UINT16(_state.begin() + place + 4) & 0xff);
		_state[place + 0xb] = 4;

		plantVegetation(place);
		debugC(1, kDebugScript, "the place at %04x is being planted", place);
		return;
	}

	dailyTroopUpdate(index); // 76CB: only after the first planting hour.
	const byte grown = MAX<byte>(_state[at + 0x18] >> 2, 1);
	const uint16 was = _state[place + 0xf];

	_state[place + 0xf] = (byte)(was + grown);

	if (was + grown <= 0xff)
		return;

	raiseSkill(index, 2, 1);
	const byte water = _state[place + 0x1b];
	_state[place + 0x1b] = water >= 12 ? water - 12 : 0;
	if (water < 12) {
		_state[at + 3] |= 0x10; // 7085, without clearing work/progress flags.
		replaceMapUnit(index); // 770E replaces the active actor as work stops.
		return;
	}
	if (_state[place + 0xb] < 12)
		_state[place + 0xb]++;
	const int north = (int8)_state[place + 0xe] - 2;
	if (north >= -82)
		_state[place + 0xe] = (byte)north;
	plantVegetation(place);
}

void DuneEngine::plantVegetation(uint32 place) {
	requestPlaceMapRefresh(place); // 6515 calls 5D50 before painting/capture.
	_map.paintRegion(READ_LE_UINT16(_state.begin() + place + 0xc),
			(int8)_state[place + 0xe], _state[place + 0xb], 0,
			[](uint offset, void *context) {
				DuneEngine *engine = static_cast<DuneEngine *>(context);
				Common::Array<byte> &state = engine->_state;
				// 656E/409A: vegetation destroys the deposit at each encountered
				// place, not just the sietch where planting began.
				for (uint p = 0x100; p + 28 <= state.size() && state[p] != 0xff; p += 28) {
					if (READ_LE_UINT16(state.begin() + p + 6) == offset) {
						state[p + 0x12] = 0;
						// 657D/5D36: only an uncaptured Harkonnen fort,
						// never either palace. Vegetation reveals the fort
						// and uses 7443's full capture/recruitment transition
						// without queuing a military victory transmission.
						if (p >= 0x138 && state[p + 8] >= 0x28 && !(state[p + 0xa] & 8)) {
							state[p + 0xa] &= 0x7f;
							engine->capturePlace(p);
						}
						break;
					}
				}
			}, this);
	_map.readPlaces(_state.begin(), _state.size());
}

void DuneEngine::updateVegetationWater() {
	if (_state.size() <= 0x46de || !_state[0x46de])
		return;
	const Common::Array<byte> &terrain = _map.terrain();
	for (uint p = 0x100; p + 28 <= _state.size() && _state[p] != 0xff; p += 28) {
		if (!(_state[p + 0xa] & 0x20) || _state[p + 0x1b] >= 250)
			continue;
		// 642E counts six consecutive stored bytes around the place's point.
		const int point = READ_LE_UINT16(_state.begin() + p + 6);
		uint green = 0;
		for (int at = point - 1; at < point + 5; at++) {
			if (at >= 0 && (uint)at < terrain.size() && (terrain[at] & 0x30) == 0x10)
				green++;
		}
		_state[p + 0x1b] = MIN<byte>((byte)(_state[p + 0x1b] + (green >> 1) + 1), 250);
	}
	// The water count precedes the thinning pass in DOS 63F0.
	if (_map.decayVegetation() && (_state[0x46eb] & 0x80))
		_state[0x46ec]++; // 65FE: once for any thinned cell, even offscreen.
}

/**
 * What a troop puts into an hour of building, worked out at 0x348A and the
 * tail it jumps to at 0x347A: twice its rate plus its skill, held to a byte,
 * times how many of them there are; a sixteenth of that, and then the high
 * byte of what is left - so a great deal of work goes into very little. When
 * nothing at all comes of it a nonempty troop still manages one.
 */
byte DuneEngine::hourlyEffort(uint index) const {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return 0;

	const uint32 at = 0x8aa + index * 27;
	if (at + 0x1b > _state.size())
		return 0;

	// 0x348F doubles AL first; only the following skill addition saturates.
	uint16 worth = (byte)(2 * troopRate(troops[index])) + _state[at + 0x16];
	if (worth > 0xff)
		worth = 0xff;

	const uint32 done = ((uint32)worth * _state[at + 0x1a]) >> 4;
	const byte effort = (byte)((done >> 8) & 0xff);

	if (effort == 0 && _state[at + 0x1a] != 0)
		return 1;

	return effort;
}

/**
 * An hour of wind-trap assembly, which is order 9.  The routine at 0x7711
 * returns to planting if one is already built - flag 0x20 at +0x0A - and
 * otherwise raises the troop's own rate by one and adds its hour's effort to
 * what has been put up at +0x1B.  When that runs past a byte the trap stands:
 * the place takes the flag at +0x0A and another at +8, and +0x1B is set to
 * five. Completion goes through the common task transition at 0x6AC5.
 */
void DuneEngine::buildWindTrap(uint index) {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 at = 0x8aa + index * 27;
	const uint32 place = troops[index].place;

	if (at + 27 > _state.size() || place < 0x100 || place + 0x1c > _state.size())
		return;

	if (!(_state[place + 0xa] & 0x20)) {
		// 0x6F48 adds in AL before applying the unsigned hundred-point cap.
		_state[at + 0x15] = MIN<byte>((byte)(_state[at + 0x15] + 1), 100);

		const uint16 put = _state[place + 0x1b] + hourlyEffort(index);
		_state[place + 0x1b] = (byte)put;

		if (put <= 0xff)
			return;

		_state[place + 0xa] |= 0x20;
		_state[place + 8] |= 0x08;
		_state[place + 0x1b] = 5;

		debugC(1, kDebugScript, "a wind trap stands at the place at %04x", place);
	}

	// 0x7731 also takes this path when the trap was already present. Keep
	// the upper task bits, reset accounting and recheck planting prerequisites.
	if (setTroopOrder(index, _state[at + 3] & 0xfc))
		_map.readTroops(_state.begin(), _state.size());
}

/**
 * An hour of bulb growing, which is order 10 - the bulbs the botanist speaks
 * of in the greenhouse.  The routine at 0x767D counts a byte at DS:0x00EC up
 * for as long as the place has nothing at +0x1A, and only when that count
 * comes round is the place marked with 0x10 there.  Either way it then asks
 * for the troop to be set to irrigation, the eighth order, which is what
 * 0x6ACB does with the 8 it is handed. Planting can still wait for equipment
 * and suitable ground; stored bulbs are not automatically carried.
 */
void DuneEngine::growBulbs(uint index) {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 place = troops[index].place;
	if (place < 0x100 || place + 0x1b > _state.size() || _state.size() <= 0xec)
		return;

	if (_state[place + 0x1a] == 0) {
		_state[0xec]++;

		if (_state[0xec] != 0)
			return;

		_state[place + 0x1a] = 0x10;
		debugC(1, kDebugScript, "the bulbs at the place at %04x are ready", place);
	}

	// 0x768F goes through the same assignment helper as a requested order.
	// This resets the new work clock and checks wind/water/carried bulbs;
	// having bulbs in the sietch store does not equip the troop automatically.
	if (setTroopOrder(index, 8)) {
		_map.readTroops(_state.begin(), _state.size());

		debugC(1, kDebugScript, "troop %u turns from the bulbs to the planting", index);
	}
}

/** 8347: consume one of the specialist prospectors' three global stops. */
void DuneEngine::shiftProspectorRoute() {
	memmove(_state.begin() + 0x11d3, _state.begin() + 0x11d5, 6);
	syncProspectorRoute();
}

/** Keep the old native route cache in step with the authoritative DOS words. */
void DuneEngine::syncProspectorRoute() {
	for (uint i = 0; i < 3; i++) {
		const uint place = READ_LE_UINT16(_state.begin() + 0x11d3 + 2 * i);
		_troopRoutes[6 + i] = place >= 0x100 && (place - 0x100) % 28 == 0 &&
			(place - 0x100) / 28 < _map.places().size() ? (place - 0x100) / 28 + 1 : 0;
	}
}

/** Remove a troop from its present sietch chain and put it on its queued route. */
void DuneEngine::beginTroopRoute(uint index) {
	if (index >= 67 || _troopRoutes.size() < 67 * 3)
		return;
	const uint32 at = 0x8aa + index * 27;
	if (index == 2) {
		// 84A6/848F skips stops at which the specialist is already standing.
		// A matching in-transit destination must stay in the queue until arrival.
		for (uint i = 0; i < 3 && !(_state[at + 3] & 0x40); i++) {
			const uint next = READ_LE_UINT16(_state.begin() + 0x11d3);
			if (!next || next != READ_LE_UINT16(_state.begin() + at + 4))
				break;
			shiftProspectorRoute();
		}
		syncProspectorRoute();
	}
	if (!_troopRoutes[index * 3] || _troopRoutes[index * 3] - 1 >= _map.places().size())
		return;
	const uint destination = 0x100 + (_troopRoutes[index * 3] - 1) * 28;
	beginTroopRoute(index, destination);
}

/** DOS 84A6: depart toward an explicit destination, including scripted raids. */
void DuneEngine::beginTroopRoute(uint index, uint destination) {
	const uint32 at = 0x8aa + index * 27;
	if (index >= 67 || at + 27 > _state.size() || destination + 28 > _state.size())
		return;
	if (_state[at + 3] & 0x40) {
		// 84B8: redirect in transit, without taking equipment a second time.
		WRITE_LE_UINT16(_state.begin() + at + 4, destination);
		if ((_state[at + 3] & 3) == 3)
			_state[at + 3] &= 0xfc;
		replaceMapUnit(index); // 84C8 resets the travelling actor.
		_map.readTroops(_state.begin(), _state.size());
		return;
	}

	const uint16 oldPlace = READ_LE_UINT16(_state.begin() + at + 4);
	if (oldPlace < 0x100 || (uint)oldPlace + 28 > _state.size())
		return;
	endTroopStrike(oldPlace); // 84CA/6EBF, before unlinking the moving troop.
	unlinkTroop(index);
	if (_state[at + 3] == 6) {
		if (_state[oldPlace + 0xa] & 2) {
			uint enemies = 0, attackers = 0;
			const Common::Array<uint> standing = _map.troopsAt(oldPlace, _state.begin(), _state.size());
			for (uint i = 0; i < standing.size(); i++) {
				const uint other = 0x8aa + standing[i] * 27;
				if (_state[other + 3] & 0x20)
					continue;
				if (_state[other + 0x10] & 0x80)
					enemies++;
				else if (_state[other + 3] == 6)
					attackers++;
			}
			// 84E7 decrements the count after the departing record was
			// unlinked, so even one remaining attacker loses the place.
			if (enemies && attackers <= 1)
				loseBattlePlace(oldPlace);
		}
		if (_state[destination + 8] < 0x28) {
			if (_state[destination + 0xa] & 2)
				return; // 84FA branches straight to 851E.
			const byte morale = _state[at + 0x15];
			_state[at + 0x15] = morale >= 3 ? morale - 3 : 0;
			if (_state[at + 0x15] < 5) {
				_state[at + 0x15] = 4;
				_state[at + 3] |= 0x10;
				_state[at + 0x12] |= 0x20;
			}
		}
	}
	WRITE_LE_UINT16(_state.begin() + at + 4, destination);
	_state[at + 3] |= 0x40;
	_state[at + 2] = 0;
	for (uint item = 0; item < 7; item++) {
		if ((_state[at + 0x19] & (0x80 >> item)) && _state[oldPlace + 0x14 + item])
			_state[oldPlace + 0x14 + item]--;
	}
	replaceMapUnit(index); // 850F, before the first seven movement substeps.
	// DOS 8518 starts visible groups with seven movement substeps now,
	// independently of the four/eight substeps of each later hourly tick.
	if (!(_state[at + 0x10] & 0x10))
		advanceTroopTravel(index, 7);
	_map.readTroops(_state.begin(), _state.size());
	debugC(1, kDebugScript, "troop %u sets out for place %u", index,
		   (destination - 0x100) / 28);
}

/**
 * One hour of travel towards the first queued place.  The original routine
 * at 0x8308 makes four calls to 0x8604 (eight only for the vehicle bit at
 * troop +0x19).  Longitude is measured in the latitude row's TABLAT scale,
 * while latitude is measured in whole rows.  The less important axis moves
 * on alternating bits of the word at DS:0, which spreads a diagonal over
 * successive substeps.  Both axes within six scaled points means arrival.
 *
 * The former fixed 1024/2 aggregate was only an approximation.  Besides
 * bending the route on the map it made the prospectors reach Tuono-Tuek too
 * late to finish their work in the reference playthrough.
 */
bool DuneEngine::advanceTroopTravel(uint index, uint substeps) {
	if (index >= 67)
		return false;
	const uint32 at = 0x8aa + index * 27;
	if (at + 27 > _state.size() || (_state[at + 3] & 0x40) == 0)
		return false;
	// 8604 takes the current destination from the travelling record. The
	// optional route queue also holds later stops; a missing/stale copy of this
	// destination must not stop the troop or send it somewhere else.
	const uint16 place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place < 0x100 || (place - 0x100) % 28 != 0 ||
			(uint)place + 28 > _state.size() || (uint)(place - 0x100) / 28 >= _map.places().size())
		return true;
	const uint targetIndex = (place - 0x100) / 28;
	const uint16 targetLongitude = READ_LE_UINT16(_state.begin() + place + 2);
	const int16 targetLatitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
	uint16 longitude = READ_LE_UINT16(_state.begin() + at + 6);
	int16 latitude = (int16)READ_LE_UINT16(_state.begin() + at + 8);
	if (!substeps)
		substeps = (_state[at + 0x19] & 0x40) != 0 ? 8 : 4;
	bool arrived = false;

	for (uint moved = 0; moved != substeps; moved++) {
		const int longitudeDelta = (int16)(targetLongitude - longitude);
		const int latitudeDelta = (int)targetLatitude - latitude;
		int longitudeStep = (int)_map.step(ABS((int)latitude));
		const uint scaledLongitude = ABS(longitudeDelta) / longitudeStep;
		const uint latitudeDistance = ABS(latitudeDelta);

		if (MAX(scaledLongitude, latitudeDistance) < 7) {
			arrived = true;
			break;
		}

		int dx = longitudeDelta;
		int dy = latitudeDelta < 0 ? -1 : latitudeDelta > 0 ? 1 : 0;
		if (longitudeDelta < 0)
			longitudeStep = -longitudeStep;

		if (scaledLongitude >= latitudeDistance) {
			dx = longitudeStep;
			if (latitudeDistance == 0)
				dy = 0;
		} else if (scaledLongitude != 0) {
			dx = longitudeStep;
		} // With less than one longitude step left, 0x8604 uses the remainder.

		// ROL word ptr DS:0,1: carry is the old high bit.  The minor axis
		// advances only on that carry, exactly as both branches of 0x8604 do.
		const bool rotates = scaledLongitude >= latitudeDistance || scaledLongitude != 0;
		const uint16 bits = READ_LE_UINT16(_state.begin());
		if (rotates)
			WRITE_LE_UINT16(_state.begin(), (uint16)((bits << 1) | (bits >> 15)));
		const bool carry = rotates && (bits & 0x8000) != 0;
		if (scaledLongitude >= latitudeDistance) {
			if (!carry)
				dy = 0;
		} else if (scaledLongitude != 0 && !carry) {
			dx = 0;
		}

		longitude = (uint16)(longitude + dx);
		latitude = (int16)(latitude + dy);
	}

	WRITE_LE_UINT16(_state.begin() + at + 6, longitude);
	WRITE_LE_UINT16(_state.begin() + at + 8, (uint16)latitude);

	if (!arrived) {
		updateTravellingMapUnit(index); // 8326: translate, remove or register the live map figure.
		debugC(2, kDebugScript, "troop %u travels towards place %u at %u,%d", index,
			   targetIndex, longitude, latitude);
		return true;
	}

	finishTroopArrival(index);
	if (_troopRoutes.size() >= 67 * 3 && index != 2) {
		if (_state[at + 3] & 0x40) {
			// 841F: an equipment collector turns home without joining this
			// place's chain or losing the return address in its work counters.
			_troopRoutes[index * 3] = (READ_LE_UINT16(_state.begin() + at + 4) - 0x100) / 28 + 1;
		} else {
			_troopRoutes[index * 3] = _troopRoutes[index * 3 + 1];
			_troopRoutes[index * 3 + 1] = _troopRoutes[index * 3 + 2];
			_troopRoutes[index * 3 + 2] = 0;
		}
	}

	debugC(1, kDebugScript, "troop %u arrives at place %u", index, targetIndex);
	return true;
}

/** DOS 8368: arriving, discovering a fort, joining battle or fetching equipment. */
void DuneEngine::finishTroopArrival(uint index) {
	const uint at = 0x8aa + index * 27;
	if (index >= 67 || at + 27 > _state.size())
		return;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place < 0x100 || place + 28 > _state.size())
		return;
	// 8357 consumes a queued stop on arrival, before settling or collecting.
	if (index == 2 && place == READ_LE_UINT16(_state.begin() + 0x11d3))
		shiftProspectorRoute();
	const byte order = _state[at + 3] & 0xf;
	WRITE_LE_UINT16(_state.begin() + at + 6, READ_LE_UINT16(_state.begin() + place + 2));
	WRITE_LE_UINT16(_state.begin() + at + 8, READ_LE_UINT16(_state.begin() + place + 4));
	const bool hostile = (_state[place + 0xa] & 2) ||
		(_state[place + 8] >= 0x28 && !(_state[place + 0xa] & 8));
	if (!hostile) {
		if (order == 5 || order == 6) {
			setTroopOrder(index, _state[at + 3] & 0xfc);
		} else if ((order & 3) == 3) {
			if (place != READ_LE_UINT16(_state.begin() + at + 0xc)) {
				for (uint item = 0; item < 7; item++)
					_state[0x46fe + item] = equipmentSpare(place, item);
				const byte item = _state[at + 0xe];
				if (item < 7 && _state[0x46fe + item]) {
					_state[place + 0x14 + item]--;
					_state[at + 0x19] |= _state[at + 0xf];
					_state[at + 0xf] = 0;
				}
				WRITE_LE_UINT16(_state.begin() + at + 4, READ_LE_UINT16(_state.begin() + at + 0xc));
				replaceMapUnit(index); // 844B immediately turns the collector homeward.
				_map.readTroops(_state.begin(), _state.size());
				return;
			}
			_state[at + 3] &= 0xfc;
		}
	}

	// 83BC/85CC: arrivals join the tail. Only the existing head is safe
	// from espionage capture. A ninth Fremen may replace a prisoner to free
	// a standing slot, but must never displace an active group.
	appendTroop(index, place);
	_state[at + 3] &= 0xbf;
	if (!(_state[at + 0x10] & 0x80) && _state[at + 2] > 8) {
		unlinkTroop(index);
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			if (!(_state[other + 0x10] & 0x80) && (_state[other + 3] & 0x20)) {
				removeTroop(standing[i]);
				break;
			}
		}
		appendTroop(index, place);
	}
	for (uint item = 0; item < 7; item++) {
		if (_state[at + 0x19] & (0x80 >> item))
			_state[place + 0x14 + item]++;
	}
	if (_state[at + 3] < 0x80) {
		setTroopOrder(index, _state[at + 3] & 0xf, true);
		// 83D6/D41B: a settled troop changes the people in Paul's room.
		// Only the ordinary room menu requests this refresh: conversations,
		// maps and other modals retain their current display. With a battle
		// active it is room one; otherwise the chiefs stand in room two.
		if (place == READ_LE_UINT16(_state.begin() + 0x114e) &&
				_menuKind == kMenuRoom && _speakerPerson < 0 &&
				!_onWorld && !_onGlobe && !_onMixer && !_inCabin && !_inDesert &&
				_state[0xb] == (_state[0x2b] ? 1 : 2))
			_state[0x473b] |= 1;
	}

	if (hostile) {
		if (order != 5 && _state[at] == _state[place + 9]) {
			// 8397 enters 7429 directly, not the palace surrender at 73A9.
			winBattlePlace(place);
		} else if (order == 5 && (_state[place + 0xa] & 0x80)) {
			_state[place + 0xa] &= 0x7f;
			if (_state[0x11c9] & 3)
				_state[0x4728] |= 1;
			requestPlaceMapRefresh(place); // 83A4/5D44 also refreshes a newly revealed fort.
		} else {
			mobilizeBattleTroops(place);
		}
	}
	_map.readPlaces(_state.begin(), _state.size());
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 83FD/661D: mobilize recruited standing troops and suspend prospecting. */
void DuneEngine::mobilizeBattleTroops(uint place) {
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 3] & 0xa0)
			continue; // The walk excludes unrecruited troops and 8403 excludes prisoners.
		const byte task = _state[at + 3] & 0xf;
		if (task == 1)
			_state[at + 3] |= 0x10;
		else if (task != 6)
			setTroopOrder(standing[i], 6);
	}
}

/**
 * What a troop is worth in a fight, worked out at 0x342D: twice its rate plus
 * its soldiering at +0x17, held to a byte, times how many of them there are,
 * and a sixteenth of that, raised by its four weapon bits at +0x19. The
 * final high-byte extraction at 0x347A gives a value from 0 to 255, with
 * one as the minimum for a nonempty troop.
 */
uint16 DuneEngine::troopStrength(uint index) const {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return 0;

	const uint32 at = 0x8aa + index * 27;
	if (at + 0x1b > _state.size())
		return 0;

	return combatStrength(troopRate(troops[index]), _state[at + 0x17],
			_state[at + 0x19], _state[at + 0x1a]);
}

/** DOS 33BE: the standing chain, not troops travelling toward this place. */
void DuneEngine::gatherTroopStrengths(uint32 place) {
	if (_state.size() <= 0x9c)
		return;
	uint16 allies = 0, enemies = 0, flags = 0, status = 0;
	const Common::Array<uint> standing = place >= 0x100 && place + 28 <= _state.size()
		? _map.troopsAt(place, _state.begin(), _state.size()) : Common::Array<uint>();
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 3] & 0x20)
			continue;
		if (_state[at + 0x10] & 0x80) {
			enemies += troopStrength(standing[i]);
		} else {
			allies += troopStrength(standing[i]);
			flags |= READ_LE_UINT16(_state.begin() + at + 0x10);
			status |= READ_LE_UINT16(_state.begin() + at + 0x12);
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0x94, enemies);
	WRITE_LE_UINT16(_state.begin() + 0x96, allies);
	WRITE_LE_UINT16(_state.begin() + 0x5c, flags);
	WRITE_LE_UINT16(_state.begin() + 0x5e, status);
	_state[0x9c] = combatOdds(allies, enemies);
}

/** DOS 34A5/6639: standing troops, then arrivals (collectors count at home). */
void DuneEngine::gatherTroopCounts(uint32 place) {
	if (_state.size() <= 0x92)
		return;
	memset(_state.begin() + 0x60, 0, 0x33);
	if (place < 0x100 || place + 28 > _state.size())
		return;

	// 34D0 counts prisoners neither as troops nor by occupation/region. An
	// unrecruited Fremen (exactly 80, not every hidden troop) has its own
	// counter instead of contributing to the allied totals.
	auto count = [this](uint at) {
		const byte state = _state[at + 3];
		if (state & 0x20)
			return;
		const bool enemy = (_state[at + 0x10] & 0x80) != 0;
		if (!enemy && state == 0x80) {
			_state[0x90]++;
			return;
		}
		const uint base = state & 0x40 ? 0x7e : 0x60;
		_state[base + (enemy ? 1 : 0)]++;
		// Equipment searches retain their occupation: 3/7/11/15 count as
		// 0/4/8/12. Flags for travel, visibility and waiting do not change it.
		const byte task = (state & 3) == 3 ? state & 0x0c : state & 0x0f;
		_state[base + 2 + task]++;
		if (!(state & 0x40))
			_state[0x71 + (_state[at + 0x12] & 0x0f)]++;
	};

	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++)
		count(0x8aa + standing[i] * 27);

	for (uint i = 0; i < _map.troops().size(); i++) {
		const uint at = 0x8aa + i * 27;
		if (at + 27 > _state.size())
			break;
		const byte state = _state[at + 3];
		if (!(state & 0x40))
			continue;
		const uint target = READ_LE_UINT16(_state.begin() + at + ((state & 3) == 3 ? 0x0c : 4));
		if (target == place)
			count(at);
	}

	_state[0x91] = (byte)(_state[0x60] + _state[0x7e]);
	_state[0x92] = (byte)(_state[0x61] + _state[0x7f]);
}

uint16 DuneEngine::troopRandom(bool secondary) {
	const uint offset = secondary ? 0xd824 : 0xd826;
	if (_state.size() < offset + 2)
		return 0;
	uint16 seed = READ_LE_UINT16(_state.begin() + offset);
	const uint16 value = combatRandom(seed, secondary ? 0xe56d : 0xcbd1);
	WRITE_LE_UINT16(_state.begin() + offset, seed);
	return value;
}

/** The battle-presence branch of DOS 503C, shared by local battle controls. */
bool DuneEngine::battleHere() const {
	if (_state.size() <= 0x114f || _state[6] != 0x80)
		return false;
	const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
	return place && battleAt(place);
}

/** DOS 627E/503C: determine battle presence using the caller's place offset. */
bool DuneEngine::battleAt(uint place) const {
	if ((place && place < 0x100) || place + 28 > _state.size())
		return false;
	if (_state[place + 0xa] & 2)
		return true;
	// 5D36: friendly sietches and already captured forts need the battle
	// flag above. An uncaptured fort also recognizes standing attackers.
	if (_state[place + 8] < 0x28 || (_state[place + 0xa] & 8))
		return false;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + 27 * standing[i];
		if (_state[at + 3] == 6 && !(_state[at + 0x10] & 0x80))
			return true;
	}
	return false;
}

/** DOS 60F8: relative surviving fractions, not weapon-weighted combat odds. */
byte DuneEngine::battleProgress(uint place) {
	if (_state.size() <= 0xd81d || (place && place < 0x100) || place + 28 > _state.size())
		return 0;
	uint16 losses = 0, kills = 0, allies = 0, enemies = 0;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + 27 * standing[i];
		if (_state[at + 0x10] & 0x80) {
			// 6155 includes enemy prisoners and hidden troops in this total.
			enemies += _state[at + 0x1a];
		} else if (_state[at + 3] == 6) {
			allies += _state[at + 0x1a];
			kills += READ_LE_UINT16(_state.begin() + at + 0xc);
			losses += READ_LE_UINT16(_state.begin() + at + 0xe);
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0xd81c, enemies);
	// The additions wrap before division. Keep the original 8-bit fixed
	// point fractions. CWD/XCHG at 6138 leaves FF, not zero, in the low
	// byte for negative differences; then IDIV truncates and SAR rounds down.
	const uint16 allyTotal = losses + allies, enemyTotal = kills + enemies;
	const uint16 allyFraction = allyTotal ? ((uint32)allies << 8) / allyTotal : 0;
	const uint16 enemyFraction = enemyTotal ? ((uint32)enemies << 8) / enemyTotal : 0;
	const uint16 larger = MAX(allyFraction, enemyFraction);
	int difference = (int16)(allyFraction - enemyFraction);
	if (larger)
		difference = (difference * 256 + (difference < 0 ? 255 : 0)) / (int16)larger;
	const int half = difference >= 0 ? difference / 2 : -((-difference + 1) / 2);
	return (byte)(half + 0x80);
}

/** DOS 503C: battle status, report progress, landscape and unescorted defeat. */
void DuneEngine::updateBattleContext() {
	if (_state.size() <= 0xd81d)
		return;
	_state[0xfd] = _state[0x2b] = 0;
	const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
	// 402E invokes 503C before storing the arrival coordinates. Unlike the
	// ordinary local-control gate, it must work with Paul's old world point.
	// 1BEC also calls 503C after worm departure has cleared DS:114E. In
	// that case 503C reads the place fields at offset zero in the data segment.
	if ((place && place < 0x100) || place + 28 > _state.size())
		return;
	if (battleAt(place)) {
		_state[0x2b] = 1;
		_state[0xfd] = battleProgress(place) | 1;
		const byte kind = _state[place + 8];
		_state[0x11dd] = kind < 0x20 ? 0x2f : (kind < 0x28 || kind == 0x30) ? 0x30 : 0x33;
	} else if (_state[place + 8] >= 0x28 && !(_state[place + 0xa] & 8)) {
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		for (uint i = 0; i < standing.size(); i++) {
			const uint at = 0x8aa + 27 * standing[i];
			if (!(_state[at + 3] & 0x20) && (_state[at + 0x10] & 0x80)) {
				_state[0x46d9] = 4;
				break;
			}
		}
	}
}

/** 1BEC refreshes an existing battle even while its room is behind a map. */
void DuneEngine::finishBattleRound() {
	if (_state.size() <= 0x473b || !_state[0x2b])
		return;
	updateBattleContext();
	if (_state[0x46d9])
		_state[0x46d9] = 6;
	if (!_state[0x2b]) {
		// 1C0D removes B45 immediately, even when a map or another modal
		// hides the battlefield. Waiting for the next drawScene/poll leaves
		// the old battle callback and actor pool alive after combat ends.
		stopBattleAnimation();
		_state[0x473b] |= 1; // The room must be rebuilt after the battle ends.
	}
}

/** DOS 7317..7370: one direction for up to sixteen standing-chain passes. */
void DuneEngine::resolveMassiveAttack() {
	if (_state.size() <= 0x485d)
		return;
	const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
	if (place < 0x100 || place + 28 > _state.size())
		return;
	_state[0x473a] = 1;
	gatherTroopStrengths(place);
	const uint16 enemies = READ_LE_UINT16(_state.begin() + 0x94);
	const uint16 allies = READ_LE_UINT16(_state.begin() + 0x96);
	WRITE_LE_UINT16(_state.begin() + 0x98, enemies);
	WRITE_LE_UINT16(_state.begin() + 0x9a, allies);
	const bool offensive = (byte)troopRandom() < _state[0x9c];
	for (uint pass = 0; pass < 16; pass++) {
		gatherTroopStrengths(place);
		// 6603 follows the link AFTER each callback. Capture can alter this
		// chain, so a snapshot array would visit troops DOS has just removed.
		byte id = _state[place + 9];
		for (uint visited = 0; id && id <= _map.troops().size() && visited < 67; visited++) {
			const uint at = 0x8aa + 27 * (id - 1);
			if (_state[at + 3] == 6) {
				if (!offensive)
					counterattackTroop(id - 1);
				else if (READ_LE_UINT16(_state.begin() + 0x94) == 0)
					winBattlePlace(place);
				else
					attackEnemies(id - 1);
			}
			id = _state[at + 1];
		}
		// These are the last gathered strengths, not fresh totals after each
		// troop. In particular an eliminated side may need one final pass.
		if (!READ_LE_UINT16(_state.begin() + 0x94) || !READ_LE_UINT16(_state.begin() + 0x96))
			break;
	}
	gatherTroopStrengths(place);
	WRITE_LE_UINT16(_state.begin() + 0x98, enemies - READ_LE_UINT16(_state.begin() + 0x94));
	WRITE_LE_UINT16(_state.begin() + 0x9a, allies - READ_LE_UINT16(_state.begin() + 0x96));
}

/** DOS 739E: local strength and the saved random stream choose who takes losses. */
void DuneEngine::attack(uint index) {
	if (index >= _map.troops().size() || _state.size() <= 0x11bc)
		return;
	const uint at = 0x8aa + index * 27;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place < 0x100 || place + 28 > _state.size())
		return;
	_state[0x11bc] |= 1; // 739E: a battle took place during this update.
	if (place == 0x11c) {
		winBattle(index);
		return;
	}
	gatherTroopStrengths(place);
	if (READ_LE_UINT16(_state.begin() + 0x94) == 0) {
		winBattle(index);
		return;
	}
	// 73E3 consumes AL from the persistent E3CC stream, not ScummVM's RNG.
	if ((byte)troopRandom() < _state[0x9c])
		attackEnemies(index);
	else
		counterattackTroop(index);
}

/** DOS 73EF/7552: one offensive exchange against the standing enemies. */
void DuneEngine::attackEnemies(uint index) {
	const Common::Array<Troop> &troops = _map.troops();
	if (index >= troops.size())
		return;

	const uint32 place = troops[index].place;
	if (place < 0x100 || place + 0xa > _state.size())
		return;

	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());

	uint16 defenders = 0;
	for (uint i = 0; i != standing.size(); i++) {
		const uint32 other = 0x8aa + standing[i] * 27;
		if (other + 0x11 > _state.size() || (_state[other + 3] & 0x20))
			continue;

		if (_state[other + 0x10] & 0x80)
			defenders++;
	}

	uint16 strength = troopStrength(index);
	if (defenders)
		strength /= defenders;

	const byte share = (byte)MIN<uint16>(strength + 1, 255);
	uint16 losses = 0, survivors = 0;

	for (uint i = 0; i != standing.size(); i++) {
		const uint32 other = 0x8aa + standing[i] * 27;
		if (other + 0x1b > _state.size() || !(_state[other + 0x10] & 0x80))
			continue;

		const byte lost = combatLosses(share, _state[other + 0x17], _state[other + 0x1a]);

		_state[other + 0x1a] -= lost;
		losses += lost;

		if (_state[other + 0x1a] == 0) {
			_state[other + 3] |= 0x20;
			_state[other + 0x10] |= 0x10;
			// 756F/7F75: one in four defeated enemies loses its carried
			// equipment from the place inventory. Atomics are always retained.
			if ((troopRandom(true) & 3) == 0) {
				_state[other + 0x19] &= 0xfb;
				for (uint kit = 0; kit < 7; kit++) {
					if ((_state[other + 0x19] & (0x80 >> kit)) && _state[place + 0x14 + kit])
						_state[place + 0x14 + kit]--;
				}
			}
			// 7581 calls 6917/C58A immediately. A 6B34 map-animation
			// repaint must not keep the defeated defender's old figure alive.
			replaceMapUnit(standing[i]);
			if (standing[i] < _troopRects.size())
				_troopRects[standing[i]] = Common::Rect();

			debugC(1, kDebugScript, "troop %u is broken", standing[i]);
		} else
			survivors++;
	}

	const uint32 at = 0x8aa + index * 27;
	WRITE_LE_UINT16(_state.begin() + at + 0xc, READ_LE_UINT16(_state.begin() + at + 0xc) + losses);

	if (losses)
		debugC(1, kDebugScript, "troop %u attacks at %04x and takes %d from them", index,
			   place, losses);

	_map.readTroops(_state.begin(), _state.size());
	if (survivors == 0)
		winBattlePlace(place); // 7416 goes straight to 7429, even at the palace.
}

/** DOS 668F/6FB0: the defeated group survives as prisoners, losing its kit. */
void DuneEngine::defeatTroop(uint index) {
	const uint at = 0x8aa + index * 27;
	if ((_state[at + 3] & 0x20) || (_state[at + 0x10] & 0x80))
		return;
	_state[at + 3] |= 0x20;
	_state[at + 0x19] = 0;
	WRITE_LE_UINT16(_state.begin() + at + 0xa, hour());
	const byte was = _state[0x29];
	_state[0x29] = was > 4 ? was - 4 : 1;
	const byte moraleLoss = (byte)((was & 0xfc) - (_state[0x29] & 0xfc)) >> 2;
	if (moraleLoss) {
		for (uint i = 0; i < 67; i++) {
			const uint other = 0x8aa + i * 27;
			if (_state[other + 3] & 0xa0)
				continue;
			const byte morale = _state[other + 0x15];
			_state[other + 0x15] = morale >= moraleLoss ? morale - moraleLoss : 0;
			if (_state[other + 0x15] < 5) {
				_state[other + 0x15] = 4;
				_state[other + 3] |= 0x10;
				_state[other + 0x12] |= 0x20;
			}
		}
	}
}

/** DOS 751D: share enemy strength, record our losses, and test defeat. */
void DuneEngine::counterattackTroop(uint index) {
	const uint at = 0x8aa + index * 27;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	uint16 share = READ_LE_UINT16(_state.begin() + 0x94);
	if (_state[0x60])
		share /= _state[0x60];
	const byte lost = combatLosses(share, _state[at + 0x17], _state[at + 0x1a]);
	WRITE_LE_UINT16(_state.begin() + at + 0xe, READ_LE_UINT16(_state.begin() + at + 0xe) + lost);
	_state[at + 0x1a] -= lost;
	if (_state[at + 0x1a] == 0) {
		_state[at + 0x1a] = (byte)((troopRandom(true) & 0x7f) + 30);
		defeatTroop(index);
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		bool fighting = false;
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			if (_state[other + 3] == 6 && !(_state[other + 0x10] & 0x80))
				fighting = true;
		}
		if (!fighting)
			loseBattlePlace(place);
	}
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 74B6: the last attacker has fallen; hand the place to the enemy. */
void DuneEngine::loseBattlePlace(uint32 place) {
	_state[place + 0xa] &= 0xfd;
	if (place == READ_LE_UINT16(_state.begin() + 0x114e)) {
		_state[0x46d9] = 6;
		return;
	}
	if (_state[place + 8] < 0x28) {
		_state[place + 8] = (_state[place + 8] & 7) + 0x28;
		_state[0x27]--;
		const uint16 location = (((place - 0x100) / 28 + 1) << 8) | 0x80;
		// 40AE supplies the new place kind in DH; 74D6 changes only DL
		// to room three. Keep both bytes when moving captured companions.
		const uint16 room = (_state[place + 8] << 8) | 3;
		for (uint who = 0xfd8; who < 0xfd8 + 9 * 16; who += 16) {
			if (READ_LE_UINT16(_state.begin() + who + 2) == location)
				WRITE_LE_UINT16(_state.begin() + who, room);
		}
	}
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 0x10] & 0x80)
			_state[at + 0x10] |= 0x10;
		else
			_state[at + 3] |= 0x20;
	}
	_map.paintRegion(READ_LE_UINT16(_state.begin() + place + 2),
			(int16)READ_LE_UINT16(_state.begin() + place + 4), 5, 0x30);
	_state[place + 0xa] &= 0xf6;
	if (_state[0x11c9] & 3)
		_state[0x4728] |= 1;
	_map.readPlaces(_state.begin(), _state.size());
	requestPlaceMapRefresh(place); // 74FD/5D44 falls through to 5D50.
}

/** DOS 73A9/7429: the palace has its own surrender path; other places settle. */
void DuneEngine::winBattle(uint index) {
	const uint at = 0x8aa + index * 27;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place == 0x11c) {
		_state[0xc2]++;
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			if (_state[other + 3] < 0x80)
				_state[other + 3] = 4;
			else if (_state[other + 0x10] & 0x80)
				removeTroop(standing[i]);
		}
		_map.conquerEnemyTerritory();
		if (_state[0xa] & 1)
			queueTroopNotice(0x0f0a, place);
		_map.readTroops(_state.begin(), _state.size());
		return;
	}
	winBattlePlace(place);
}

/** DOS 7429: settle an ordinary victory, also used by unopposed arrivals. */
void DuneEngine::winBattlePlace(uint32 place) {
	if (place != READ_LE_UINT16(_state.begin() + 0x114e) && (_state[0xa] & 1))
		queueTroopNotice(0x0f07, place);
	if (_state[place + 8] >= 0x28) {
		capturePlace(place);
		return;
	}
	_state[place + 0xa] &= 0xfd;
	// 743A clears the battle layout before 75AF/6B00 replaces the victors'
	// actors. Refresh the cached standing layout now, not at the queued repaint.
	_map.readPlaces(_state.begin(), _state.size());
	settleBattleTroops(place, false);
	finishBattleControl(place);
}

/** DOS 7443: shared by military victory and vegetation reaching a fort. */
void DuneEngine::capturePlace(uint32 place) {
	_map.paintRegion(READ_LE_UINT16(_state.begin() + place + 2),
			(int16)READ_LE_UINT16(_state.begin() + place + 4), 5, 0x20);
	_state[place + 0xb] = (byte)((hour() >> 4) + 2);
	addCharisma(4);
	for (uint i = 0; i < 67; i++) {
		const uint at = 0x8aa + i * 27;
		if (!(_state[at + 3] & 0xa0))
			_state[at + 0x15] = MIN<byte>((byte)(_state[at + 0x15] + 1), 100);
	}
	_state[place + 0xa] |= 8;
	const uint region = _state[place] & 15;
	WRITE_LE_UINT16(_state.begin() + 0x115a,
			(uint16)((0x8000u << region) | (0x8000u >> (16 - region))));
	settleBattleTroops(place, true);
	finishBattleControl(place);
}

/** DOS 75AF/75EA: release ours and turn one enemy group into recruitable Fremen. */
void DuneEngine::settleBattleTroops(uint32 place, bool convertEnemies) {
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 3] >= 0x80)
			continue;
		if (standing[i] == 2) {
			// The prospectors keep their job, with 6B8A's survey prerequisite
			// even if their saved task is not currently the usual order one.
			_state[at + 3] &= 0xdf;
			const byte previousState = _state[at + 3];
			if (_state[place + 0xa] & 0x41)
				_state[at + 3] |= 0x10;
			else
				_state[at + 3] &= 0xef;
			if ((_state[at + 3] ^ previousState) & 0x10) {
				// 75E7/6BB6 replaces the actor before 6B25 resets resumed work.
				replaceMapUnit(standing[i]);
				if (previousState & 0x10) {
					WRITE_LE_UINT16(_state.begin() + at + 0xa, hour());
					WRITE_LE_UINT16(_state.begin() + at + 0xc, 0);
					WRITE_LE_UINT16(_state.begin() + at + 0xe, 0);
				}
			}
		} else if (_state[at + 3] & 0x20) {
			_state[at + 3] = 0x22;
		} else {
			_state[at + 0x11] |= 4;
			if (_state[place + 8] >= 0x28)
				_state[at + 0x10] |= 0x20;
			_state[at + 0x15] = MIN<byte>((byte)(_state[at + 0x15] + 4), 100);
			raiseSkill(standing[i], 1, 3);
			setTroopOrder(standing[i], 4);
		}
	}
	if (convertEnemies) {
		// 6603 reads the current record's NEXT link after the callback.
		// Unlink/append clears that link, ending this pass at its first
		// converted enemy; iterating a snapshot would convert too many.
		for (uint i = 0; i < standing.size(); i++) {
			const uint at = 0x8aa + standing[i] * 27;
			if (_state[at + 3] < 0x80 || !(_state[at + 0x10] & 0x80))
				continue;
			unlinkTroop(standing[i]);
			_state[at + 0x10] &= 0x7f;
			if (appendTroop(standing[i], place) >= 8) {
				removeTroop(standing[i]);
			} else {
				_state[at + 3] = 0xa0;
				const uint16 population = (troopRandom(true) & 0x0f7f) + 0x1464;
				_state[at + 0x1a] = (byte)population;
				_state[at + 0x15] = population >> 8;
				const uint16 skills = (troopRandom(true) & 0x1f1f) + 0x0a0a;
				_state[at + 0x16] = (byte)skills;
				_state[at + 0x17] = skills >> 8;
				_state[at + 0x19] = 0;
				_state[at + 0x10] &= 0xef;
				// 7627 jumps to 68E0/846C: the converted group becomes
				// visible immediately, without waiting for a map rebuild.
				registerMapUnit(standing[i]);
			}
			break;
		}
	}
}

/** DOS 747C: retain at most one hostile remnant, then update war statistics. */
void DuneEngine::finishBattleControl(uint32 place) {
	bool keepEnemy = (READ_LE_UINT16(_state.begin()) & 3) == 0;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 3] < 0x80 || !(_state[at + 0x10] & 0x80))
			continue;
		if (keepEnemy) {
			_state[at + 3] = 0xac;
			_state[at + 0x10] |= 0x10;
			_state[at + 0x1a] = 0;
			WRITE_LE_UINT16(_state.begin() + at + 0xc, 0);
			_state[at + 0x19] = 0;
			keepEnemy = false;
		} else {
			removeTroop(standing[i]);
		}
	}
	uint16 forts = 0;
	byte atomics = 0;
	for (uint p = 0x100; p + 28 <= _state.size() && _state[p] != 0xff; p += 28) {
		atomics += _state[p + 0x19];
		if (_state[p + 8] >= 0x28 && !(_state[p + 0xa] & 8))
			forts++;
	}
	updateHarkonnenProduction();
	if (forts <= 1) {
		_state[0xc2] = 1;
		_state[0xff7] &= 0xfd;
		_state[0x1007] &= 0xfd;
		if (atomics >= 10)
			_state[place + 0x19] += atomics - 10;
	}
	_map.readTroops(_state.begin(), _state.size());
	_map.readPlaces(_state.begin(), _state.size());
	requestPlaceMapRefresh(place); // 74B3 jumps to 5D50 after victory.
}

/** DOS 858C: detach without discarding the troop's population or allegiance. */
void DuneEngine::unlinkTroop(uint index) {
	const uint at = 0x8aa + index * 27;
	if (index >= 67 || at + 27 > _state.size())
		return;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (!(_state[at + 3] & 0x40) && place >= 0x100 && place + 28 <= _state.size()) {
		uint link = place + 9;
		for (uint visited = 0; visited < 67 && _state[link]; visited++) {
			const byte id = _state[link];
			if (id == _state[at]) {
				_state[link] = _state[at + 1];
				break;
			}
			if (id > 67)
				break;
			link = 0x8aa + (id - 1) * 27 + 1;
		}
	}
	_state[at + 1] = 0;
}

/** DOS 851F: append at the tail and choose the first free standing slot. */
byte DuneEngine::appendTroop(uint index, uint32 place) {
	const uint at = 0x8aa + index * 27;
	bool used[30] = {};
	uint tail = place + 9;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint other = 0x8aa + standing[i] * 27;
		const byte slot = _state[other + 2];
		if (slot && slot <= ARRAYSIZE(used))
			used[slot - 1] = true;
		tail = other + 1;
	}
	_state[tail] = _state[at];
	byte slot = (_state[at + 0x10] & 0x80) ? 9 : 1;
	while (slot < ARRAYSIZE(used) && used[slot - 1])
		slot++;
	_state[at + 2] = slot;
	return slot;
}

/** DOS 66B1: unlink the record and remove its live actor. */
void DuneEngine::removeTroop(uint index) {
	const uint at = 0x8aa + index * 27;
	if (index >= 67 || at + 27 > _state.size())
		return;
	unlinkTroop(index);
	removeMapUnit(index); // 66B6/66BB: 6917/C58A remove its live actor.
	WRITE_LE_UINT16(_state.begin() + at + 4, 0xfbc);
	_state[at + 3] = 0xa0;
	_state[at + 0x1a] = 0;
	if (index < _troopRects.size())
		_troopRects[index] = Common::Rect();
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 6DBB: finish a fort's two-day conversion, including its occupants. */
void DuneEngine::convertCapturedFort(uint32 place) {
	if (place < 0x100 || place + 28 > _state.size())
		return;
	_state[place + 0xa] &= 0xf7;
	_state[place + 8] &= 7;
	_state[0x27]++;
	const byte kind = _state[place + 8];
	const uint16 location = (((place - 0x100) / 28 + 1) << 8) | 0x80;
	for (uint person = 0xfd8; person < 0xfd8 + 12 * 16; person += 16) {
		if (READ_LE_UINT16(_state.begin() + person + 2) != location)
			continue;
		WRITE_LE_UINT16(_state.begin() + person,
				(kind << 8) | (_state[person] == 1 ? 1 : 2));
	}
	if (READ_LE_UINT16(_state.begin() + 6) == location) {
		_state[0x473b] |= 0x80;
		_state[4] = _state[4] == 1 ? 1 : 2;
		_state[5] = kind;
		_state[0xb] = _state[4];
		_state[8] = kind;
		roomFor((place - 0x100) / 28);
	}
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 0x10] & 0x20) {
			_state[at + 0x10] &= 0xdf;
			_state[at + 0x13] |= 0x10;
		}
		// 764D removes unrecruited Harkonnen remnants, not converted Fremen.
		if (_state[at + 3] >= 0x80 && (_state[at + 0x10] & 0x80))
			removeTroop(standing[i]);
	}
	_state[place + 0xb] = 5;
}

/** DOS 6E20, called by mining, training and established planting only. */
void DuneEngine::dailyTroopUpdate(uint index) {
	const uint at = 0x8aa + index * 27;
	if (_state.size() <= 0x46de || !_state[0x46de] || index >= 67 || at + 27 > _state.size())
		return;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	if (place < 0x100 || place + 28 > _state.size())
		return;
	const byte day = hour() >> 4;
	if (_state[place + 0xa] & 8) {
		// FE/FF are the two days before the deadline, also across byte wrap.
		if ((byte)(day - _state[place + 0xb]) >= 0xfe)
			return;
		convertCapturedFort(place);
	}
	if (_state[place + 8] < 0x20 && _state[place + 0xb] < 12 && !(_state[place + 0xa] & 1)) {
		_state[place + 0xb]++;
		_map.paintRegion(READ_LE_UINT16(_state.begin() + place + 2),
				(int16)READ_LE_UINT16(_state.begin() + place + 4), _state[place + 0xb], 0x20);
	}
	if ((byte)(day - _state[at + 0x14]) > 8) {
		// 6F93 clamps exhausted morale to four and marks refusal to work.
		if (_state[at + 0x15])
			_state[at + 0x15]--;
		if (_state[at + 0x15] < 5) {
			_state[at + 0x15] = 4;
			_state[at + 3] |= 0x10;
			_state[at + 0x12] |= 0x20;
		}
	}
	if (_state[at] == _state[place + 9] && place != READ_LE_UINT16(_state.begin() + 0x114e) &&
			_state[place + 8] < 0x21) {
		// A strike needs low-morale miners from both regional groups. Once
		// started, low-morale soldiers at the same place join it as well.
		const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
		byte groups = 0;
		for (uint i = 0; i < standing.size(); i++) {
			const uint other = 0x8aa + standing[i] * 27;
			if (_state[other + 3] < 0x80 && !(_state[other + 3] & 0x2f) && _state[other + 0x15] < 40)
				groups |= (_state[other + 0x12] & 0x80) ? 2 : 1;
		}
		if (groups == 3) {
			for (uint i = 0; i < standing.size(); i++) {
				const uint other = 0x8aa + standing[i] * 27;
				if (_state[other + 3] < 0x80 && !(_state[other + 3] & 0x2b) && _state[other + 0x15] < 40) {
					_state[other + 3] |= 0x10;
					_state[other + 0x12] |= 0x10;
				}
			}
			if (_state[0xa] & 1)
				queueTroopNotice(0x0302, place);
		}
	}
	_map.readPlaces(_state.begin(), _state.size());
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 6D19: small working groups merge into the smallest fitting ally. */
bool DuneEngine::mergeSmallTroop(uint index) {
	if (index >= _map.troops().size())
		return false;
	const uint at = 0x8aa + index * 27;
	if (at + 27 > _state.size())
		return false;
	if (_state[at + 0x1a] >= 20 || (_state[at + 3] & 0xe3) ||
			(_state[at + 0x10] & 0x80) || index == 2)
		return false;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	int recipient = -1;
	byte room = 255 - _state[at + 0x1a];
	for (uint i = 0; i < standing.size(); i++) {
		const uint other = 0x8aa + standing[i] * 27;
		if (standing[i] == index || standing[i] == 2 || (_state[other + 3] & 0xa0) ||
				_state[other + 0x1a] > room)
			continue;
		recipient = standing[i];
		room = _state[other + 0x1a];
	}
	if (recipient < 0)
		return false;
	const uint other = 0x8aa + recipient * 27;
	_state[other + 0x1a] += _state[at + 0x1a];
	const byte kit = _state[at + 0x19];
	_state[at + 0x19] &= _state[other + 0x19];
	_state[other + 0x19] |= kit;
	_state[other + 0x13] |= 2;
	removeTroop(index);
	return true;
}

void DuneEngine::decayTroopSkills(uint index) {
	if ((hour() & 0x3f) || index >= _map.troops().size())
		return;
	const uint at = 0x8aa + index * 27;
	if (at + 27 > _state.size())
		return;
	const uint category = (_state[at + 3] & 0xf) >> 2;
	const uint16 unused = (uint16)((0xc000u << category) | (0xc000u >> (16 - category)));
	const uint16 history = READ_LE_UINT16(_state.begin() + at + 0x12) & unused;
	for (uint skill = 0; skill < 3; skill++) {
		if ((history & (0x2000 << skill)) && _state[at + 0x16 + skill])
			_state[at + 0x16 + skill]--;
	}
}

/** DOS 6EBF: changing a place's troop mix restarts its striking workers. */
void DuneEngine::endTroopStrike(uint32 place) {
	if (place < 0x100 || place + 28 > _state.size())
		return;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint at = 0x8aa + standing[i] * 27;
		if (_state[at + 3] < 0x80 && (_state[at + 0x12] & 0x10))
			setTroopOrder(standing[i], _state[at + 3] & 0xf, true);
	}
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 1D10: the last hour of a day can reinforce every active enemy troop. */
void DuneEngine::growHarkonnenTroops() {
	if (_state.size() <= 0xfbb)
		return;
	const uint16 bits = READ_LE_UINT16(_state.begin());
	WRITE_LE_UINT16(_state.begin(), (uint16)((bits << 1) | (bits >> 15)));
	if (!(bits & 0x8000))
		return;
	// 1D16 visits records 0..65, stopping before the spare final record.
	for (uint at = 0x8aa; at < 0xfa0; at += 27) {
		byte &men = _state[at + 0x1a];
		if ((_state[at + 0x10] & 0x80) && men != 0 && men < 200)
			men++;
	}
}

/** DOS 331E: expand a place for the next order, map action or conversation. */
/** 2E98: retain the report place and its one-based COMMAND name words. */
void DuneEngine::preparePlaceNameContext(uint16 place) {
	if (place < 0x100 || place >= 0x8aa || (place - 0x100) % 28 ||
			(uint)place + 1 >= _state.size() || _state.size() <= 0x47e7)
		return;
	WRITE_LE_UINT16(_state.begin() + 0x47e6, place);
	WRITE_LE_UINT16(_state.begin() + 0x11ed, _state[place]);
	WRITE_LE_UINT16(_state.begin() + 0x11ef, _state[place + 1] + 12);
}

void DuneEngine::expandPlaceContext(uint32 place) {
	if (place < 0x100 || place + 28 > _state.size())
		return;
	WRITE_LE_UINT16(_state.begin() + 0x11ce, place);
	_state[0x4d] = _state[place + 8];
	WRITE_LE_UINT16(_state.begin() + 0x4e,
			(uint16)((_state[place] << 8) | _state[place + 1]));
	_state[0x50] = _state[0x1141 + _state[place + 1]];
	_state[0x51] = _state[place + 0xa];
	_state[0x52] = _state[place + 0x12];
	_state[0x54] = _state[place + 0x1b];
	_state[0x53] = 0;
	for (uint item = 0; item < 7; item++) {
		_state[0x55 + item] = _state[place + 0x14 + item];
		_state[0x46fe + item] = equipmentSpare(place, item);
		if (_state[0x46fe + item])
			_state[0x53] |= 0x80 >> item;
	}
	_state[0xf7] = 0;
	if (place != READ_LE_UINT16(_state.begin() + 0x1150)) {
		const uint16 where = (uint16)((((place - 0x100) / 28 + 1) << 8) | 0x80);
		static const uint companions[] = { 0x1018, 0x1028, 0x1048 };
		for (uint i = 0; i < ARRAYSIZE(companions); i++) {
			const uint at = companions[i];
			if (READ_LE_UINT16(_state.begin() + at + 2) == where && _state[at + 0xe] < 8)
				_state[0xf7] |= 1 << _state[at + 0xe];
		}
	}
	gatherTroopStrengths(place);
	gatherTroopCounts(place);
	nearestPlaces(place);
}

/** DOS 1F64/2017: the every-other-day Harkonnen strike on a vulnerable sietch. */
void DuneEngine::launchHarkonnenRaid() {
	if (_state.size() <= 0xd819)
		return;
	if (storyStage() < 60) {
		const uint16 deadline = READ_LE_UINT16(_state.begin() + 0x1154);
		if (hour() < deadline || hour() - deadline < 112)
			return;
	}
	if (hour() & 0x10)
		return;
	const byte battleThisRound = _state[0x11bc];
	_state[0x11bc] = 0;
	if (battleThisRound)
		return;
	const uint16 bits = READ_LE_UINT16(_state.begin());
	WRITE_LE_UINT16(_state.begin(), (uint16)((bits << 1) | (bits >> 15)));
	if (!(bits & 0x8000))
		return;

	// 2017 scans only the initial, ordinary-sietch portion of the place table.
	// It keeps the northernmost eligible sietch, not simply the nearest one.
	uint16 target = 0, source = 0;
	int16 latitude = 100;
	WRITE_LE_UINT16(_state.begin() + 0xd816, 0);
	for (uint32 place = 0x100; place + 28 <= _state.size() && _state[place] < 8; place += 28) {
		if (_state[place + 8] >= 0x20 || (_state[place + 0xa] & 0x82) ||
			(int16)READ_LE_UINT16(_state.begin() + place + 4) >= latitude)
			continue;
		const Common::Array<uint> residents = _map.troopsAt(place, _state.begin(), _state.size());
		bool defended = false;
		for (uint i = 0; i < residents.size(); i++) {
			if (READ_LE_UINT16(_state.begin() + 0x8aa + residents[i] * 27 + 0x12) & 0x400) {
				defended = true;
				break;
			}
		}
		if (defended)
			continue;
		expandPlaceContext(place); // 331E, before the troop and fort tests.
		if (_state[0x60] == _state[0x63])
			continue;
		uint16 fort = 0;
		if (READ_LE_UINT16(_state.begin() + 0xe2) < 30)
			fort = READ_LE_UINT16(_state.begin() + 0xe4);
		else if (READ_LE_UINT16(_state.begin() + 0xdc) < 30)
			fort = READ_LE_UINT16(_state.begin() + 0xde);
		if (fort == 0x11c || fort < 0x100 || fort + 28 > _state.size())
			continue;
		uint enemies = 0, attackers = 0;
		const Common::Array<uint> garrison = _map.troopsAt(fort, _state.begin(), _state.size());
		for (uint i = 0; i < garrison.size(); i++) {
			const uint at = 0x8aa + garrison[i] * 27;
			if (_state[at + 3] & 0x20)
				continue;
			if (_state[at + 0x10] & 0x80)
				enemies++;
			else if (_state[at + 3] == 6)
				attackers++;
		}
		if (!enemies || attackers)
			continue;
		latitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
		target = place;
		source = fort;
		WRITE_LE_UINT16(_state.begin() + 0xd816, target);
		WRITE_LE_UINT16(_state.begin() + 0xd818, source);
	}
	if (!target)
		return;

	_state[0xc4]++;
	for (uint moved = 0; moved < 2; moved++) {
		const Common::Array<uint> garrison = _map.troopsAt(source, _state.begin(), _state.size());
		uint index = 67;
		for (uint i = 0; i < garrison.size(); i++) {
			const uint at = 0x8aa + garrison[i] * 27;
			if (_state[at + 0x10] & 0x80) {
				index = garrison[i];
				break;
			}
		}
		if (index == 67)
			break;
		const uint at = 0x8aa + index * 27;
		_state[at + 3] = 0x8d;
		// 1FB9 runs the normal departure with the hidden flag intact.
		// Visible raiders take the initial seven steps before 1FC3 settles them.
		beginTroopRoute(index, target);
		_state[at + 0x10] &= (byte)~0x10;
		// A nearby visible troop can have settled during the initial steps.
		if (_state[at + 3] & 0x40)
			finishTroopArrival(index);
	}
	_state[target + 0xa] |= 2;
	// 6883 reads the new battle flag during resident actor replacement.
	_map.readPlaces(_state.begin(), _state.size());
	mobilizeBattleTroops(target);
	const uint16 location = (((target - 0x100) / 28 + 1) << 8) | 0x80;
	const uint16 entrance = (_state[target + 8] << 8) | 1;
	// 1FD5 moves the first nine named companions outside, including those
	// left here rather than following Paul. Later person records stay put.
	for (uint person = 0xfd8; person < 0xfd8 + 9 * 16; person += 16) {
		if (READ_LE_UINT16(_state.begin() + person + 2) == location)
			WRITE_LE_UINT16(_state.begin() + person, entrance);
	}
	queueTroopNotice(READ_LE_UINT16(_state.begin() + 0x8e0 + 4) == target ? 0x0f0d : 0x0f0c, target);
	_map.readTroops(_state.begin(), _state.size());
	_map.readPlaces(_state.begin(), _state.size());
	if (location == READ_LE_UINT16(_state.begin() + 6)) {
		// 2000/6144: a local raid interrupts Paul's current room before
		// 1BEC refreshes the landscape and the normal presentation gates.
		WRITE_LE_UINT16(_state.begin() + 4, entrance);
		_state[0xb] = 1;
		_state[0x473b] |= 1;
		_state[0x2b] = 1;
		_state[0xfd] = battleProgress(target) | 1;
		roomFor((target - 0x100) / 28); // Keep the cached SAL room at the new entrance.
		return;
	}
	// 2014/5D50 requests a map rebuild for a visible, known remote target.
	// Arrival alone cannot introduce raiders from an unseen fort: 8461 only
	// replaces an existing actor. The rebuild registers the new battle mix.
	requestPlaceMapRefresh(target);
}

/** DOS 5D50: request a rebuild only for a known place in the active viewport. */
void DuneEngine::requestPlaceMapRefresh(uint32 place) {
	if (_state[0x46eb] && !(_state[place + 0xa] & 0x80)) {
		const uint16 longitude = READ_LE_UINT16(_state.begin() + place + 2);
		const int latitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
		const bool strategic = (_state[0x46eb] & 0x80) != 0;
		const Common::Rect view = strategic ? Common::Rect(4, 4, 316, 148)
			: Common::Rect(81, 45, 241, 134);
		const Common::Point at = strategic
			? _map.strategicPosition(_worldLongitude, _worldLatitude, longitude, latitude)
			: _map.surveyPosition(_worldLongitude, _worldLatitude, longitude, latitude, view);
		if (view.contains(at))
			_state[0x46ec]++; // INC byte wraps, even when a refresh is pending.
	}
}

/** Optional binary diagnostics; no configuration means no files or messages. */
void DuneEngine::hourlyRound() {
	// The completed-story gate is at 1B19, before ordinary hour dispatch.
	// Direct FD9/1B23 waits enter this body without that gate.
	// Where Gurney is, worked out as the round does at 0x6C6F: 0x80 in
	// the low byte of the word at 0x101A means the high byte is a place, and
	// 0x1C * (n - 1) + 0x100 is where its record lies.
	if (_state.size() > 0x473d) {
		const uint16 where = READ_LE_UINT16(_state.begin() + 0x101a);
		const uint16 at = (where & 0xff) == 0x80 ? 0x1c * ((where >> 8) - 1) + 0x100 : 0;

		WRITE_LE_UINT16(_state.begin() + 0x473c, at);

		// 0x6C46 is the rest of that round's work, and nothing was doing it.
		// While the story stands between forty-five and forty-seven and
		// Gurney is no longer at Paul's side, the place he was left at is
		// expanded and its count of allied troops in military training read
		// out of DS:0x66.  One troop drilling under him is what moves the
		// story on to forty-eight - the stage which puts Leto's palace
		// warning in the queue, and the vision the player is told to expect.
		// Without it Gurney went on asking to be taken to a sietch however
		// many Fremen were training around him, and the story stopped dead
		// at forty-six with every later chapter out of reach.
		if (storyStage() >= 45 && storyStage() < 48 && _state.size() > 0x114f &&
				(READ_LE_UINT16(_state.begin() + 0x10) & 0x10) == 0 && at != 0) {
			// 6C5F expands all of Gurney's place context through 331E,
			// including equipment, companions and nearest sites, not just
			// its troop counts. 1B68/1B6C retain the caller's context and
			// 1B86 restores it after troop work and timed callbacks. Do not
			// move Paul or restore the scratch context prematurely here.
			expandPlaceContext(at);
			const byte drilling = _state[0x66];
			if (drilling != 0) {
				debugC(1, kDebugScript, "Gurney has %u troop(s) in training", drilling);
				advanceStory(48);
			}
		}
	}

	const Common::Array<Troop> &troops = _map.troops();
	uint working = 0;

	_lastHourMined = 0;
	_state[0x4737] = 0;

	for (uint i = 0; i != troops.size(); i++) {
		const uint32 record = 0x8aa + i * 27;
		if (record + 27 > _state.size())
			break;
		const bool travelling = (_state[record + 3] & 0x40) != 0;
		const bool held = (READ_LE_UINT16(_state.begin() + record + 0x12) & 0x430) != 0;
		if (held && !travelling) {
			if (!_state[0xfa])
				continue;
			_state[record + 0x12] &= 0xcf;
			if (_state[record + 0x13] & 4)
				continue;
		}
		// 6CD3 sends held travellers straight to movement, before the usual
		// small-troop merge and hidden/prisoner checks at 6C99/6CA4.
		if (!(held && travelling)) {
			if (mergeSmallTroop(i) || (_state[record + 3] & 0xa0))
				continue;
		}
		working++;
		if (travelling) {
			// DOS 6CED pauses only the selected traveller (DS:1954).
			// Standing work and other troops continue during its contact.
			if (_state[record] != _state[0x1954]) {
				advanceTroopTravel(i);
			}
			continue;
		}
		debugC(2, kDebugScript, "troop %u is set to %s, worth %d this hour", i,
			   orderName(troops[i].order()).c_str(), troopRate(troops[i]));
		if (ConfMan.hasKey("dune_kit"))
			debugC(1, kDebugScript, "  troop %u kit %02x men %u at %04x", i,
				   _state[record + 0x19], _state[record + 0x1a],
				   READ_LE_UINT16(_state.begin() + record + 4));

		switch (_state[record + 3] & 0xf) {
		case 0:
			mineSpice(i);
			break;
		case 1:
			prospect(i);
			break;
		case 4:
			train(i);
			break;
		case 5:
			spy(i);
			break;
		case 8:
			irrigate(i);
			break;
		case 9:
			buildWindTrap(i);
			break;
		case 6:
			attack(i);
			break;
		case 10:
			growBulbs(i);
			break;
		default:
			// 6C26 maps tasks 2, 3, 7 and 11..15 to 0F66 (RET).
			// Equipment searches depart through the menu, not hourly work.
			break;
		}
		decayTroopSkills(i);
	}
	_state[0xfa] = _state[0x4737]; // 6CCC: next hour's ecology motivation bonus.
	_map.readTroops(_state.begin(), _state.size());
	// 6C6F does not refresh the results population words at AA/AC. C02E does
	// that at the day close and again when SEE RESULTS opens. Updating them
	// here made troop losses visible in the saved results before DOS would.

	if (working)
		debugC(1, kDebugScript, "the hour turns with %u troops about their work, %d spice",
			   working, spice());
}

/**
 * Makes the Emperor's next demand, as 0x20D2 does.  C3 numbers the demands;
 * their base grows by 150 each time and a small random factor is then
 * corrected by the satisfaction ratio from the preceding shipment.  A
 * negative ratio selects edition two, while an ordinary ratio selects the
 * stricter edition three.
 */
void DuneEngine::queueEmperorDemand() {
	if (_state.size() <= 0x118e)
		return;

	const byte demandNumber = _state[0xc3]++;
	const uint32 scale = (uint32)demandNumber * 150 + 100;
	// 0x20E9 calls E3B7, the secondary persistent generator at DS:D824,
	// and masks its return with BX=0x3F. The old host RNG plus a fixed first
	// demand matched one recording but changed every other saved seed's terms.
	const uint factor = 224 + (troopRandom(true) & 0x3f);

	uint32 demand = factor * scale / 256;
	if ((int8)_state[0xbe] >= 0) {
		const byte adjustment = (byte)~(byte)(_state[0xbe] << 1);
		demand = demand * (uint32)(adjustment + 0x100) / 0x100;
	}

	WRITE_LE_UINT16(_state.begin() + 0xbc,
		(uint16)MIN<uint32>(demand, 0xffff));
	_state[0xcf] = 0;
	_state[0xbf] |= 0x90;
	const byte edition = (int8)_state[0xbe] < 0 ? 2 : 3;
	queueEvent((uint16)((edition << 8) | 11));

	debugC(1, kDebugScript, "the Emperor demands %u tenths of spice in edition %u",
		   READ_LE_UINT16(_state.begin() + 0xbc), edition);
}

/**
 * The once-per-day Emperor pass at hour three (0x20A4).  0x118D is an absolute
 * Dune day, not an hour countdown. Before a shipment it produces harsher
 * editions four to six on the three late days; after one it creates the next
 * demand when its promised day arrives.
 */
void DuneEngine::updateEmperorSchedule() {
	if (_state.size() <= 0x46d9 || _state.size() <= 0x11bb ||
			!(_state[0xbf] & 0x80))
		return;

	const uint16 today = hour() >> 4;
	const uint16 due = READ_LE_UINT16(_state.begin() + 0x118d);

	// Once C2's late-war progression is active, the original only keeps CF,
	// the displayed days remaining, fresh.
	if (_state[0xc2] != 0) {
		if (today != due)
			_state[0xcf] = (byte)(due - today);
		return;
	}

	if (_state[0xbf] & 0x10) {
		// 2131 subtracts unsigned days. Equality leaves CF untouched; a due
		// day in the future wraps and takes the loss branch, not the countdown
		// path used before a demand has been issued.
		const uint16 late = (uint16)(today - due);
		if (late == 0)
			return;
		if (late >= 4) {
			_state[0x46d9] = 7;
			return;
		}

		// 0x2161 deliberately overlaps the end of the preceding instruction in
		// the DOS image.  Read as data it is this three-by-three reminder table.
		static const byte kLateEdition[3][3] = {
			{ 4, 5, 6 }, { 5, 6, 0 }, { 6, 0, 0 }
		};
		uint category = 0;
		static const uint16 thresholds[] = { 1, 0x40, 0x80, 0x90, 0xff };
		for (uint i = 0; i != ARRAYSIZE(thresholds); i++) {
			if (_state[0xbe] < thresholds[i])
				category++;
		}
		category = MIN<uint>(category, 2);
		const byte edition = kLateEdition[late - 1][category];
		if (edition != 0)
			queueEvent((uint16)((edition << 8) | 11));
		else
			_state[0x46d9] = 7;
		return;
	}

	if (_state[0x11bb] != 0) {
		_state[0x46d9] = 7;
		return;
	}

	// 20C6 tests equality alone. An already-past date does not generate a
	// retroactive proposal; CF receives the wrapped negative day difference.
	if (today != due) {
		_state[0xcf] = (byte)(due - today);
		return;
	}
	queueEmperorDemand();
}

/**
 * Whether the story has run out.  The game leaves DS:0x2A at two hundred when
 * there is no more of it to tell, which the round at 0x1B19 reads to stop the
 * world turning and which the condition the dialogue calls 103 tests for.
 */
bool DuneEngine::storyIsOver() const {
	return _state.size() > 0x2a && _state[0x2a] >= 200;
}

/** DOS 0x16FC/0x1771: stop the campaign clock and start the ending script. */
void DuneEngine::endStory() {
	if (_state.size() <= 0x477b)
		return;

	debugC(1, kDebugScript, "the story is brought to its end");

	_state[0x2a] = 200;
	beginEventScript(0x128f);
}

/**
 * Arriving at a place, as the routine at 0x4020 has it.  The place is kept at
 * DS:0x114E and DS:0x1150, the pair of words at DS:0x98 is emptied, the player
 * is let in on the place, and then 0x40AE turns the place into the two words
 * the game carries about where the player is: the place's own byte at +8 with
 * the kind under it, and the place's number with 0x80 under that.  A place
 * whose +8 byte is 0x20 or over is marked at +0x0A as it is reached.
 *
 * The bytes this leaves at DS:0x8, DS:0x9 and DS:0xB are read by a great many
 * of the conditions that decide what people have to say.
 */
/**
 * What is standing where the player is, which 0x7F27 copies out of the place's
 * record: seven bytes from its +0x14, one to each kind of thing the game knows
 * - harvesters, ornithopters, krys, laser guns, weirding modules, atomics and
 * bulbs, which are COMMAND1.HSQ's own words 231 to 237.  Nothing was copying
 * them, so the ornithopters standing outside a place were never counted, never
 * drawn, and never offered to be flown.
 *
 * The original counts them again every time it builds the list the panel shows
 * - 0x2F42 for a room and 0x5FDD for the maps - because what the troops at the
 * place are carrying comes off the count, and troops come and go.
 */
void DuneEngine::countThingsHere() {
	countThingsHere(true);
}

void DuneEngine::countThingsHere(bool refreshTroopContext) {
	enum { kThingsHere = 0x46fe, kKinds = 7 };

	if (_state.size() <= 0x114f || _state.size() <= kThingsHere + kKinds)
		return;

	const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
	if (refreshTroopContext) {
		gatherTroopStrengths(place);
		gatherTroopCounts(place);
	}

	for (uint item = 0; item < kKinds; item++)
		_state[kThingsHere + item] = equipmentSpare(place, item);
}

void DuneEngine::arriveAt(uint index) {
	const uint32 place = 0x100 + 28 * index;
	if (_state.size() <= place + 27 || _state.size() <= 0x1151)
		return;
	_state[0x4735] = 0; // 401F: reaching a place ends the outdoor walking run.

	WRITE_LE_UINT16(_state.begin() + 0x114e, place);
	WRITE_LE_UINT16(_state.begin() + 0x1150, place);
	WRITE_LE_UINT16(_state.begin() + 0x98, 0);
	WRITE_LE_UINT16(_state.begin() + 0x9a, 0);
	// Dialogue compares this packed type/id with the place just reached.  It
	// is not chief-only scratch: Gurney's Tuek instruction remains available
	// while it differs from 0x0303, the hidden stillsuit sietch, and his quiet
	// "impressive" observation replaces it once those bytes are current.
	if (_state.size() > 0x4f)
		WRITE_LE_UINT16(_state.begin() + 0x4e,
						(uint16)((_state[place] << 8) | _state[place + 1]));

	revealPlace(place);
	countThingsHere();

	const byte what = _state[place + 8];
	const byte which = (byte)(index + 1);

	_state[0x8] = what;
	_state[0x9] = which;

	if (what >= 0x20)
		_state[place + 0xa] |= 0x10;

	// 40AE returns outside room one. 2170 settles named residents, preserving
	// this coordinate pair through E270/E283 before 40C3 stores Paul's words.
	const uint16 where = (uint16)((what << 8) | 1);
	const uint16 whose = (uint16)((which << 8) | 0x80);

	settleArrivalResidents(); // 4054/2170 precedes the matching-follower move.
	moveMatchingFollowers(where, whose); // 4057, before Paul's old coordinates change.
	WRITE_LE_UINT16(_state.begin() + 4, where);
	_state[0xd] = _state[0xb];
	_state[0xb] = (byte)(where & 0xff);
	WRITE_LE_UINT16(_state.begin() + 6, whose);
	updateBattleContext(); // 402E, before displaying the newly reached place.

	debugC(1, kDebugScript, "arrived at place %u: 0x%04x, 0x%04x", index, where, whose);

	endStoryOnReaching(where);
}

/**
 * DOS 0x4070/0x4072: entering room 0x3002 starts the ending unless the
 * transition-suppression byte has its high bit set. DS:0x1048 is Chani's
 * person-record location, not the ending destination: it changes when Feyd
 * takes her elsewhere, but the room comparison in 0x4072 remains literal.
 */
void DuneEngine::endStoryOnReaching(uint16 where) {
	enum { kThroneRoom = 0x3002, kSuppressed = 0x46eb };

	if (_state.size() <= kSuppressed || (_state[kSuppressed] & 0x80) != 0)
		return;
	if (where != kThroneRoom)
		return;

	endStory();
}

/**
 * One of the scenes the intro puts between its movies, which the steps at
 * 0x0972, 0x098A and 0x0995 set up.  Each of them goes to a place the way
 * 0x08F0 does - the pair of words at DS:0x4 and DS:0x6, the place's own byte
 * at DS:0x8, and the place itself at DS:0x114E, worked out as 28 times the
 * place's number plus 228 - having first said who is standing there by
 * writing the word at DS:0x12, and clearing the flag at DS:0x47A4 that keeps
 * everybody off the floor.
 *
 * The kind is the low byte of the first word: two, four and ten for the three
 * scenes, which is what a great many of the conditions read out of DS:0xB.
 */
/** 099D/09C7/978E: prepare a named intro portrait over a retained page. */
void DuneEngine::prepareIntroSpeaker(int speaker, int xOffset) {
	_introSpeakerX = xOffset;
	_speakerPerson = speaker;
	_speakerFrame = 0;
	_nextMouthTime = 0;
	WRITE_LE_UINT16(_state.begin() + 0x47c4, (uint16)speaker);
	_speaker = Common::String(getResourceFilenameByIndex(speaker + 2));
	_onWorld = false;
	_speech.clear();
	_state[0x478c] = 30; // 09D0 supplies the intro's idle block length.
	prepareLocalSpeakerIdle();
}

void DuneEngine::introScene(uint place, byte kind, uint16 present, int speaker, uint16 line, bool show) {
	if (_state.size() <= 0x1151)
		return;

	_state[0x47a4] = 0;
	_state[0x46df] = 0;

	// Whoever is speaking is drawn over the room by drawScene, out of their
	// own bank and from whatever frame is current, so both must be set before
	// a scene is put up.  Leaving the last scene's speaker behind drew one
	// person's parts out of another's bank, and out of ICONES.HSQ where the
	// scene had nobody speaking at all.
	_speakerPerson = speaker;
	_speakerFrame = 0;
	_nextMouthTime = 0;

	WRITE_LE_UINT16(_state.begin() + 0x12, present);

	const uint at = 0x100 + 28 * place;
	const byte what = _state[at + 8];

	const uint16 where = (uint16)((what << 8) | kind);
	const uint16 whose = (uint16)(((place + 1) << 8) | 0x80);

	WRITE_LE_UINT16(_state.begin() + 4, where);
	WRITE_LE_UINT16(_state.begin() + 6, whose);
	_state[0x8] = what;
	_state[0xb] = kind;
	WRITE_LE_UINT16(_state.begin() + 0x114e, at);
	countThingsHere();

	_map.readPlaces(_state.begin(), _state.size());

	if (speaker < 0)
		_speaker.clear();

	roomFor(place);

	debugC(1, kDebugScript, "intro scene at place %u, kind %d, present 0x%04x, speaker %d, line 0x%04x",
		   place, kind, present, speaker, line);

	_onWorld = false;
	// 07C6 draws its explicit roster before clearing12 and entering978E.
	// Keep that room page beneath Stilgar's portrait instead of treating it
	// as the ordinary conversational view of somebody standing there.
	const bool retainedRoster = !_introPage.empty() && speaker >= 0 && present;
	if (retainedRoster) {
		_speakerPerson = -1;
		_speaker.clear();
		drawScene(false);
		WRITE_LE_UINT16(_state.begin() + 0x12, 0);
	}
	// 0798/9EFD selects a dedicated recording without PHRASE captions.
	if (speaker >= 0)
		prepareIntroSpeaker(speaker);
	if (retainedRoster) {
		drawCurrentSpeakerFigure();
		if (show) {
			copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
	} else {
		drawScene(show);
	}

	// Whoever is speaking stands over the room, in the rectangle their own
	// record gives them.
	if (speaker >= 0) {
		// Intro recordings are already relative to the speaker's voice bank.
		// The intro's 227D latch makes A701 bypass the dialogue-base subtraction.
		if (line != 0xffff)
			playSpeech(line, speaker, false, false);
	}

	if (ConfMan.hasKey("dune_dump")) {
		static int shown = 0;
		dumpScreenPPM(Common::String::format("%s-scene%02d-k%d.ppm", ConfMan.get("dune_dump").c_str(),
											 shown++, kind));
	}
}

/**
 * Puts one of the intro's pictures on the screen.  The steps that show them
 * end at 0xC2F2, which loads the resource its number names and draws it with
 * everything at nought - the whole picture at the top left corner.  They are
 * INT02 and INT04 to INT15, and the palace.
 */
const char *DuneEngine::nameOfStep(int kind, uint16 what) const {
	// A movie, whether played straight through or a stage at a time.
	if (kind == 1 || kind == 6)
		return getResourceFilenameByIndex(what + INDEX_FIRST_HNM);

	if (kind == 3 || kind == 4 || kind == 5)
		return getResourceFilenameByIndex(what);

	return "";
}

/** Present a logical page through the intro's 152-line, centered viewport. */
const byte *DuneEngine::introPresentationPage(const byte *page, Common::Array<byte> &composed) const {
	if (_introPage.empty())
		return page;
	composed.resize(kScreenWidth * kScreenHeight);
	memset(composed.begin(), 0, composed.size());
	const int height = _introYOffset ? kSceneHeight : kScreenHeight;
	memcpy(composed.begin() + _introYOffset * kScreenWidth, page, height * kScreenWidth);
	return composed.begin();
}

void DuneEngine::copyPresentedPageToScreen(const byte *page) {
	if (!_introPage.empty() && page != _introPage.begin())
		memcpy(_introPage.begin(), page, _introPage.size());
	_system->copyRectToScreen(page, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
}

void DuneEngine::copyFrameRectToScreen(const byte *pixels, int pitch, int x, int y, int w, int h) {
	if (_introPage.empty()) {
		_system->copyRectToScreen(pixels, pitch, x, y, w, h);
		return;
	}
	const bool fullPage = x == 0 && y == 0 && w == kScreenWidth && h == kScreenHeight;
	if (fullPage)
		memset(_introPage.begin(), 0, _introPage.size());
	Common::Rect bounds(x, y, x + w, y + h);
	bounds.clip(Common::Rect(0, 0, kScreenWidth, _introYOffset ? kSceneHeight : kScreenHeight));
	if (bounds.isEmpty())
		return;
	pixels += (bounds.top - y) * pitch + bounds.left - x;
	for (int row = bounds.top; row < bounds.bottom; row++)
		memcpy(_introPage.begin() + (row + _introYOffset) * kScreenWidth + bounds.left,
				pixels + (row - bounds.top) * pitch, bounds.width());
	if (fullPage)
		copyPresentedPageToScreen(_introPage.begin());
	else
		_system->copyRectToScreen(_introPage.begin() + (bounds.top + _introYOffset) * kScreenWidth + bounds.left,
				kScreenWidth, bounds.left, bounds.top + _introYOffset, bounds.width(), bounds.height());
}

/**
 * Keeps what is on the screen, so that a step which draws over it can be
 * brought in by one of the transitions rather than cut to.
 */
void DuneEngine::keepScreen() {
	if (!_pageBefore)
		_pageBefore = new byte[kScreenWidth * kScreenHeight]{};

	memcpy(_pageBefore, _introPage.empty() ? _framebuffer1 : _introPage.begin(), kScreenWidth * kScreenHeight);
	_system->getPaletteManager()->grabPalette(_paletteBefore, 0, 256);
}

/**
 * Brings in what has just been drawn the way the step's fourth word asks for.
 * Effect 10 is DNVGA's ordered 4x4 dissolve. 36 and 38 fade in and
 * out separately; 3A fades out, copies the new page, then fades in.
 */
PlaybackResult DuneEngine::applyWipe(int kind) {
	if (!_pageBefore)
		return kPlaybackFinished;
	if (kind == 16)
		return dissolveSceneFrom(_pageBefore);
	if (kind == 0x36 || kind == 0x38) {
		byte next[768], previousDAC[768], nextDAC[768], current[768], rgb[768];
		_system->getPaletteManager()->grabPalette(next, 0, 256);
		for (uint i = 0; i < 768; i++) {
			previousDAC[i] = ((uint)_paletteBefore[i] * 63 + 127) / 255;
			nextDAC[i] = ((uint)next[i] * 63 + 127) / 255;
		}
		Common::Array<byte> composed;
		const byte *page = introPresentationPage(_framebuffer1, composed);
		const uint frames = introPaletteFrameCount(kind);
		const bool fast = ConfMan.getBool("dune_nodelay");
		copyPresentedPageToScreen(kind == 0x36 ? page : _pageBefore);
		PlaybackResult asked = kPlaybackFinished;
		uint32 deadline = _system->getMillis();
		for (uint frame = fast ? frames - 1 : 0; frame < frames; frame++) {
			introPaletteFrame(previousDAC, nextDAC, current, kind, frame);
			for (uint i = 0; i < 768; i++)
				rgb[i] = ((uint)current[i] * 255 + 31) / 63;
			_system->getPaletteManager()->setPalette(rgb, 0, 256);
			_system->updateScreen();
			updateGameMusic();
			asked = pollSkip();
			if (asked != kPlaybackFinished || fast)
				break;
			deadline += 15; // 261D waits three 200-Hz ticks per DAC chunk.
			const uint32 now = _system->getMillis();
			if (now < deadline)
				_system->delayMillis(deadline - now);
		}
		if (asked != kPlaybackFinished) {
			introPaletteFrame(previousDAC, nextDAC, current, kind, frames - 1);
			for (uint i = 0; i < 768; i++)
				rgb[i] = ((uint)current[i] * 255 + 31) / 63;
			_system->getPaletteManager()->setPalette(rgb, 0, 256);
		}
		// 36 copies before fading in; 38 copies only after fading out.
		// Leave 38's DAC black until the following step brings its page in.
		copyPresentedPageToScreen(page);
		_system->updateScreen();
		return asked;
	}
	if (kind == 0x3a) {
		byte next[256 * 3];
		_system->getPaletteManager()->grabPalette(next, 0, 256);
		return fadePage(_paletteBefore, next, _pageBefore);
	}
	return kPlaybackFinished;
}

PlaybackResult DuneEngine::fadePage(const byte *previous, const byte *next,
		const byte *before, bool skippable) {
	Common::Array<byte> composed;
	const byte *page = introPresentationPage(_framebuffer1, composed);
	if (ConfMan.getBool("dune_nodelay")) {
		_system->getPaletteManager()->setPalette(next, 0, 256);
		copyPresentedPageToScreen(page);
		_system->updateScreen();
		return kPlaybackFinished;
	}
	byte old[256 * 3], target[256 * 3], current[256 * 3], rgb[256 * 3];
	for (uint i = 0; i < ARRAYSIZE(old); i++) {
		old[i] = ((uint)previous[i] * 63 + 127) / 255;
		target[i] = ((uint)next[i] * 63 + 127) / 255;
	}
	_system->getPaletteManager()->setPalette(previous, 0, 256);
	if (before)
		copyPresentedPageToScreen(before);
	uint32 deadline = _system->getMillis();
	for (uint frame = 0; frame < 132; frame++) {
		// DNVGA 26BC/264D copy the page after the 66 old-palette writes.
		if (frame == 66)
			copyPresentedPageToScreen(page);
		Graphics::fadePaletteFrame(old, target, current, frame);
		for (uint i = 0; i < ARRAYSIZE(rgb); i++)
			rgb[i] = ((uint)current[i] * 255 + 31) / 63;
		_system->getPaletteManager()->setPalette(rgb, 0, 256);
		_system->updateScreen();
		updateGameMusic();
		const PlaybackResult asked = skippable ? pollSkip() :
				(shouldQuit() ? kPlaybackAborted : kPlaybackFinished);
		if (asked != kPlaybackFinished) {
			// A skip must not leave the next scene black or partly faded.
			_system->getPaletteManager()->setPalette(next, 0, 256);
			copyPresentedPageToScreen(page);
			_system->updateScreen();
			return asked;
		}
		// 261D/257A wait three 200-Hz ticks after each 85-colour chunk.
		deadline += 15;
		const uint32 now = _system->getMillis();
		if (now < deadline)
			_system->delayMillis(deadline - now);
	}
	return kPlaybackFinished;
}

/** C108/2DC3: sixteen scene clears, palette switch, sixteen scene copies. */
PlaybackResult DuneEngine::dissolveSceneFrom(const byte *before) {
	Common::Array<byte> composed;
	const byte *page = introPresentationPage(_framebuffer1, composed);
	const int origin = _introPage.empty() ? 0 : _introYOffset * kScreenWidth;
	byte targetPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(targetPalette, 0, 256);
	_system->getPaletteManager()->setPalette(_paletteBefore, 0, 256);
	byte *work = _framebuffer2;
	memcpy(work, before, kScreenWidth * kScreenHeight);
	const bool fast = ConfMan.getBool("dune_nodelay");
	PlaybackResult result = kPlaybackFinished;
	uint32 deadline = _system->getMillis();
	for (uint phase = 0; phase < 32 && !shouldQuit(); phase++) {
		if (phase == 16)
			_system->getPaletteManager()->setPalette(targetPalette, 0, 256);
		Graphics::dissolveScenePage(page + origin, work + origin, phase);
		copyPresentedPageToScreen(work);
		_system->updateScreen();
		updateGameMusic();
		result = pollSkip();
		if (result != kPlaybackFinished)
			break;
		deadline += 15; // 2572: three 200-Hz ticks after each ordered pass.
		const uint32 now = _system->getMillis();
		if (!fast && now < deadline)
			_system->delayMillis(deadline - now);
	}
	// C4CD's caller copy includes the panel, unlike the scene-only effect.
	// A skip/abort must also leave the next step's complete page and palette.
	if (result != kPlaybackFinished || shouldQuit())
		_system->getPaletteManager()->setPalette(targetPalette, 0, 256);
	copyPresentedPageToScreen(page);
	_system->updateScreen();
	return shouldQuit() ? kPlaybackAborted : result;
}

void DuneEngine::showPicture(uint index, bool present) {
	const char *name = getResourceFilenameByIndex(index);
	if (!name)
		return;

	debugC(1, kDebugGraphics, "the intro shows %s", name);

	// 0xC2F2 loads the resource and draws it with everything at nought - the
	// whole picture at the top left corner - and clears nothing first.
	_graphics.setFrameBuffer(_framebuffer1);

	openSpriteSheet(name);
	drawSprite(0, 0, 0);

	if (present) {
		copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}

	if (ConfMan.hasKey("dune_dump"))
		dumpScreenPPM(Common::String::format("%s-pic%u.ppm", ConfMan.get("dune_dump").c_str(), index));
}

/** 0886: fill the scene, draw BACK's pillars, then its clipped part list. */
void DuneEngine::drawIntroFeydBackground() {
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet(getResourceFilenameByIndex(48));
	_graphics.fillRect(0, 0, kScreenWidth, kSceneHeight, 0xde);
	drawList(48, 0x154e, -1, 0, 0, false);
	// C32F uses C30D's rectangle-clipped painter, not C21B's ordinary one.
	const Common::Rect clip(0, 0, kScreenWidth, kSceneHeight);
	for (uint at = 0x155c; at + 6 <= _state.size(); at += 6) {
		const uint16 sprite = READ_LE_UINT16(_state.begin() + at);
		if (sprite == 0xffff)
			break;
		currentSpriteSheet->drawMapSprite(sprite,
			(int16)READ_LE_UINT16(_state.begin() + at + 2),
			(int16)READ_LE_UINT16(_state.begin() + at + 4), clip);
	}
}

/**
 * Draws one of the scenes the intro builds out of a bank, which is what
 * 0xC21B does: it walks a list of three words at a time - which sprite, and
 * where in x and y - until a first word of 0xFFFF ends it.  The low nine bits
 * of the word are the sprite and the rest are how to draw it, the same as a
 * scene file has it.  The sunrise is four bands stacked down the screen.
 */
void DuneEngine::drawList(uint resource, uint32 at, int over, int overX, int overY, bool present) {
	const char *name = getResourceFilenameByIndex(resource);
	if (!name)
		return;

	debugC(1, kDebugGraphics, "the intro builds a scene out of %s", name);

	// 0xC21B draws its list and nothing else: it does not clear what is on the
	// screen, and several of the steps count on that - they lay a thing or two
	// over what the step before them put up.  Clearing first left those steps
	// with a sprite or two on an empty screen.
	_graphics.setFrameBuffer(_framebuffer1);

	openSpriteSheet(name);

	while (at + 6 <= _state.size()) {
		const uint16 what = READ_LE_UINT16(_state.begin() + at);
		if (what == 0xffff)
			break;

		const int16 x = (int16)READ_LE_UINT16(_state.begin() + at + 2);
		const int16 y = (int16)READ_LE_UINT16(_state.begin() + at + 4);
		at += 6;

		drawSprite(what & 0x1ff, x, y, what);
	}

	// One more sprite of the same bank on top, which some of the steps add.
	if (over >= 0)
		drawSprite(over, overX, overY);

	if (present) {
		copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}

	if (ConfMan.hasKey("dune_dump"))
		dumpScreenPPM(Common::String::format("%s-list%04x.ppm", ConfMan.get("dune_dump").c_str(), (uint)at));
}

/**
 * Draws the person who is speaking, big, over the scene.
 *
 * The last sprite of a speaker's bank is not a picture but a record, and 0x91A0
 * reads it: the first four words are the rectangle the person fills, which for
 * Leto is nought to a hundred and forty-eight across and nought to a hundred
 * and fifty-two down; the fifth says how far on the animations lie; and the
 * words after it are where each part of the person is kept.
 *
 * An animation is frame after frame, and a frame is a list of part numbers
 * ending at nought, which 0x9BEE walks: a one means the number is in the byte
 * after it and two hundred and fifty-six more, and every other byte is a part.
 * Each part is a run of three bytes at a time - which sprite, and where - laid
 * end to end into the list at DS:0x4608, which 0x9D16 then draws, each piece
 * moved by the rectangle's own corner.
 *
 * Leto's frames are the parts two, three and four, then two, three and five:
 * the head and the body stay and only the last piece changes, which is the
 * mouth.
 */
int DuneEngine::fremenPicture(int person, uint chief) const {
	if (person == 15 && storyStage() == 200)
		return 15; // 9143: the final audience uses FRM2 without a chief variant.
	if ((person != 14 && person != 15) || _state.size() <= 0x4768)
		return person;

	const uint16 troop = person == 14
		? READ_LE_UINT16(_state.begin() + 0x4756)
		: READ_LE_UINT16(_state.begin() + 0x4758 + 2 * (chief & 7));

	// 0x1913B chooses the same one of three Fremen for PERS.HSQ's standing
	// figure and for the matching FRM1/FRM2/FRM3 close-up.
	if (troop >= 0x8aa && troop < 0xfbb && troop < _state.size())
		return 14 + _state[troop] % 3;

	return person;
}

int DuneEngine::fremenAnimation(int person, uint chief) const {
	if (person == 15 && storyStage() == 200)
		return 0;
	if ((person != 14 && person != 15) || _state.size() <= 0x4768)
		return 0;

	const uint16 troop = person == 14
		? READ_LE_UINT16(_state.begin() + 0x4756)
		: READ_LE_UINT16(_state.begin() + 0x4758 + 2 * (chief & 7));

	// A troop's first byte selects a face in a two-dimensional table: its
	// remainder chooses FRM1/FRM2/FRM3 and its quotient chooses one of that
	// bank's head-and-mouth animations.  Selecting only the bank made
	// troop five wear FRM3's bearded first face instead of the clean-shaven
	// stillsuit maker seen both in the room and on his map contact card.
	if (troop >= 0x8aa && troop < 0xfbb && troop < _state.size()) {
		// 0x915C wraps the quotient at fifteen for FRM1 and seventeen for
		// FRM2/3.  Later troop ids reuse these faces, not the animation bank's
		// final (mouth-parts) table.
		const uint variants = _state[troop] % 3 == 0 ? 15 : 17;
		return (_state[troop] / 3) % variants;
	}

	return 0;
}

int DuneEngine::paulAnimation() const {
	// 917A doubles AX twice before reading AH, so the sixteen-bit shift
	// wraps at hour 16384. Each of nine stages has normal and tired versions;
	// 918A selects the latter once walking fatigue reaches sixteen.
	const uint age = MIN<uint>((uint16)(hour() << 2) >> 8, 8);
	return 2 * age + (_state[0xf4] >= 16);
}

/** 912F/9143/915C: non-random portrait tables for merchants and chiefs. */
int DuneEngine::localSpeakerPicture() const {
	if (!_onWorld && (_speakerPerson == 14 || _speakerPerson == 15) &&
			_speakerIdlePerson == _speakerPerson && _speakerIdlePicture >= 14 && _speakerIdleVariant >= 0)
		return _speakerIdlePicture;
	return fremenPicture(_speakerPerson, _speakerPerson == 15 ? _state[0x476c] & 7 : 0);
}

int DuneEngine::localSpeakerVariant() const {
	if (_speakerPerson == 13) {
		const uint here = READ_LE_UINT16(_state.begin() + 0x114e);
		return here >= 0x100 && here < _state.size() ? _state[here] >> 1 : 0;
	}
	if (_speakerPerson == 15 && storyStage() == 200)
		return -1; // 9143 clears 47D0: the final audience uses named-pose choices.
	if (_speakerPerson == 14 || _speakerPerson == 15) {
		if (!_onWorld && _speakerIdlePerson == _speakerPerson && _speakerIdleVariant >= 0)
			return _speakerIdleVariant;
		return fremenAnimation(_speakerPerson, _speakerPerson == 15 ? _state[0x476c] & 7 : 0);
	}
	return -1;
}

void DuneEngine::collectSpeakerSprites(int person, int frame, int animation, int mapMouth,
		Common::Array<SpeakerSprite> &sprites, uint *frameCountOut,
		bool localMouth, Common::Rect *mouthWindowOut, Common::Rect *pictureWindowOut) {
	sprites.clear();
	if (mouthWindowOut)
		*mouthWindowOut = Common::Rect();
	if (pictureWindowOut)
		*pictureWindowOut = Common::Rect();
	if (frameCountOut)
		*frameCountOut = 0;
	if (person < 0)
		return;

	const uint selected = person == 15 && _state.size() > 0x476c
		? _state[0x476c] & 7 : 0;
	// 9123 maps every synthetic owner from seventeen onward to Paul.
	// 961B's presence refresh can move4756 to another unmet group. 91A0
	// prepared this portrait before the oath; 99BE/9DE3 retain its bank and
	// variant until another paragraph explicitly prepares a new one.
	const bool retainedFremen = !_onWorld && person == _speakerPerson &&
		person == _speakerIdlePerson && (person == 14 || person == 15) &&
		_speakerIdlePicture >= 14 && _speakerIdleVariant >= 0;
	const int pictured = retainedFremen ? localSpeakerPicture() :
		(person >= 17 ? 45 : fremenPicture(person, selected));
	if (animation == 0)
		animation = retainedFremen ? _speakerIdleVariant :
			(person >= 17 ? paulAnimation() : fremenAnimation(person, selected));
	// SMUG.HSQ contains a different merchant for each pair of regions.  The
	// original derives this one-based variant from the current place's first
	// byte at 0x912F; animation zero otherwise always drew the first, horned
	// merchant, including at Oxtyn where the recording shows the square-faced
	// third merchant.
	if (person == 13 && animation == 0 && _state.size() > 0x114f) {
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here >= 0x100 && here < _state.size())
			animation = _state[here] >> 1;
	}

	const char *name = getResourceFilenameByIndex(pictured + 2);
	if (!name)
		return;

	Resource bank = _archive.openMember(name);
	if (!bank)
		return;

	const uint32 size = bank->size();
	Common::Array<byte> b;
	b.resize(size);
	bank->read(b.begin(), size);

	if (size < 8)
		return;

	const uint32 index = READ_LE_UINT16(b.begin());
	if (index + 2 > size)
		return;

	const uint32 count = READ_LE_UINT16(b.begin() + index) / 2;
	if (count == 0 || index + 2 * count > size)
		return;

	const uint32 last = index + READ_LE_UINT16(b.begin() + index + 2 * (count - 1));
	const uint32 record = last + 4;
	if (record + 12 > size)
		return;

	const int16 bankLeft = (int16)READ_LE_UINT16(b.begin() + record);
	// 09C7 loads the bank first, then shifts its window when its left is
	// below the requested intro origin. The bank and part tables stay intact.
	const int shift = !_introPage.empty() && person == _speakerPerson &&
		(uint16)bankLeft < _introSpeakerX ? _introSpeakerX : 0;
	const int16 left = bankLeft + shift;
	int16 right = (int16)READ_LE_UINT16(b.begin() + record + 4);
	if (shift)
		right = MIN<int>(kScreenWidth, right + shift);
	const int16 top = (int16)READ_LE_UINT16(b.begin() + record + 2);
	// 9123 registers the person bank's first four words at 1BF0. The
	// retained portrait remains clickable after its caption has been closed.
	if (pictureWindowOut)
		*pictureWindowOut = Common::Rect(left, top, right,
			(int16)READ_LE_UINT16(b.begin() + record + 6));
	if (mouthWindowOut && pictured >= 0 && pictured < 17) {
		const byte *window = _state.begin() + 0x27fa + 8 * pictured;
		*mouthWindowOut = Common::Rect(left + READ_LE_UINT16(window),
			top + READ_LE_UINT16(window + 2), left + READ_LE_UINT16(window + 4),
			top + READ_LE_UINT16(window + 6));
		mouthWindowOut->clip(Common::Rect(0, 0, kScreenWidth, kSceneHeight));
	}

	const uint32 parts = record + 10;
	const uint32 animations = record + 8 + READ_LE_UINT16(b.begin() + record + 8);
	if (animations + 2 > size)
		return;

	// How many animations there are, and where the first one begins and ends.
	// The table opens with the offset of the first, so its own length says how
	// many it holds.
	const uint howMany = READ_LE_UINT16(b.begin() + animations) / 2;
	if (howMany == 0 || animations + 2 * howMany > size)
		return;

	if (animation < 0 || (uint)animation >= howMany)
		animation = 0;

	const uint32 begins = animations + READ_LE_UINT16(b.begin() + animations + 2 * animation);
	const uint32 ends = (uint)animation + 1 < howMany
		? animations + READ_LE_UINT16(b.begin() + animations + 2 * (animation + 1))
									: size;

	if (begins >= size || ends > size || begins >= ends)
		return;

	// Bound the lookup to one cycle instead of scanning every elapsed frame
	// on every redraw. Map contacts use frame zero plus their voice's mouth.
	uint frameCount = 0;
	for (uint32 p = begins; p < ends && b[p] != 0xff;) {
		if (b[p] == 0)
			frameCount++;
		p += b[p] == 1 ? 2 : 1;
	}
	if (frameCount == 0)
		return;
	if (frameCountOut)
		*frameCountOut = frameCount;
	frame = MAX(frame, 0) % frameCount;

	// Walk to the frame asked for, going back to the first when the animation
	// runs out.  Without that end the walk carries on into whatever follows
	// and asks for sprites the bank does not have.
	// An animation is frame after frame, each a list of parts ending at nought,
	// and the whole of it ends at 0xFF: Leto's first is twenty frames of four
	// bytes and then that one byte.  Walking past it reads whatever follows as
	// parts and asks for sprites the bank does not hold.
	uint32 at = begins;
	for (int i = 0; i != frame; i++) {
		while (at < ends && b[at] != 0 && b[at] != 0xff)
			at += (b[at] == 1) ? 2 : 1;

		if (at >= ends || b[at] == 0xff) {
			at = begins;
			continue;
		}

		at++;

		if (at >= ends || b[at] == 0xff)
			at = begins;
	}

	// 9DE3 uses the named bank's complete cue table, not four generic
	// mouths. Merchants have five entries per variant; local chiefs have four.
	// The map branch 9E75 does not apply the merchant's fifth-entry stride.
	if (mapMouth >= 0 && (localMouth || person == 12 ||
			(mapMouth < 4 && (person == 14 || person == 15)))) {
		const uint32 mouths = animations + READ_LE_UINT16(b.begin() + animations + 2 * (howMany - 1));
		const uint stride = person == 13 && localMouth ? 5 : (person >= 14 ? 4 : 0);
		at = mouths + 2 * (stride * animation + mapMouth);
		if (at >= size || b[at] == 0xff)
			return;
	}
	openSpriteSheet(name);
	Common::Array<uint16> selectedParts;
	const uint32 partsEnd = mapMouth >= 0 ? size : ends;
	while (at < partsEnd && b[at] != 0 && b[at] != 0xff) {
		uint16 which = b[at++];
		if (which == 1 && at < partsEnd)
			which = 0x100 + b[at++];

		if (which < 2)
			break;

		selectedParts.push_back(which);
	}
	for (uint i = 0; i < selectedParts.size(); i++) {
		const uint16 which = selectedParts[i];
		if (which < 2 || parts + 2 * (which - 2) + 2 > size)
			continue;
		const uint32 where = parts + READ_LE_UINT16(b.begin() + parts + 2 * (which - 2));
		// 9D45/9DC4 use one-based sprite ids and unsigned local coordinates.
		for (uint32 p = where; p + 2 < size && b[p] != 0; p += 3) {
			SpeakerSprite sprite = { b[p] - 1, left + b[p + 1], top + b[p + 2] };
			// 9B09 replaces the first five sprite-table offsets, not the
			// positions in the moving card's part lists. Keep the archive
			// immutable and select those digit sprites here instead. The
			// trailing zero/unit label is already in the card artwork.
			if (mapMouth < 0 && person == _speakerPerson && sprite.index < 5 &&
					((person == 3 && (_speakerReport == 6 || _speakerCardValueValid)) ||
					 (person == 13 && (_speakerReport == 5 || _speakerCardValueValid)))) {
				const uint value = _speakerCardValueValid ? _speakerCardValue :
					(person == 3 ? READ_LE_UINT16(_state.begin() + 0x20) : _state[0x9d]);
				static const uint divisors[] = { 10000, 1000, 100, 10, 1 };
				const uint divisor = divisors[sprite.index];
				// Sprite five is the blank used for leading zeros. Once
				// a nonzero digit was encountered, even zero uses six+digit.
				sprite.index = value < divisor ? 5 : 6 + (value / divisor) % 10;
			}
			sprites.push_back(sprite);
		}
	}
}

void DuneEngine::drawSpeakerFigure(int person, int frame, int animation) {
	Common::Array<SpeakerSprite> sprites;
	collectSpeakerSprites(person, frame, animation, -1, sprites);
	for (uint i = 0; i < sprites.size(); i++)
		drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
}

void DuneEngine::collectCurrentSpeakerSprites(Common::Array<SpeakerSprite> &sprites) {
	const bool duncanGesturePending = _speakerPerson == 3 &&
		((_speakerReport >= 1 && _speakerReport <= 3) || _speakerReport == 6) &&
		_speakerAnimation <= 3;
	const bool merchantGesturePending = _speakerPerson == 13 && _speakerReport == 5 &&
		_speakerAnimation < 6;
	const bool emperorGesturePending = _speakerPerson == 11 && _speakerReport == 4 &&
		_speakerAnimation <= 3;
	if (!_speakerReportLowered && _speakerVoiceActive && ((_speakerReport == 0 &&
			(_speakerAnimation <= 3 || localSpeakerVariant() >= 0)) ||
			duncanGesturePending || merchantGesturePending || emperorGesturePending))
		collectSpeakerSprites(_speakerPerson, _speakerVoiceFrame, _speakerVoiceAnimation,
				-1, sprites);
	else
		collectSpeakerSprites(_speakerPerson, _speakerFrame, _speakerAnimation,
				-1, sprites);
}

void DuneEngine::drawCurrentSpeakerFigure() {
	if (_speakerPerson < 0)
		return;
	// 88E7 paints a balloon before A0AA/978E paints the speaker. Unlike
	// 908C's transparent subtitles, the box is part of the restoration
	// backdrop: animated hands can be drawn OVER it by later 9D16 calls.
	_localSpeakerBalloonText.clear();
	if (retainedLocalBalloonCaption(_speech) && drawLocalBalloonCaption(_speech, _framebuffer1))
		_localSpeakerBalloonText = _speech;
	// The original local portrait window ends at row152. Its initial
	// paint needs the same bound as9C90's later dirty-rectangle updates;
	// a part crossing that edge must not leak through transparent panel art.
	byte panelBacking[kScreenWidth * (kScreenHeight - kSceneHeight)];
	memcpy(panelBacking, _framebuffer1 + kSceneHeight * kScreenWidth, sizeof(panelBacking));
	Common::Array<SpeakerSprite> sprites;
	collectCurrentSpeakerSprites(sprites);
	_localSpeakerPerson = -1;
	if (!_onWorld && !_onGlobe && !_inCabin && !_onMixer && !_takingOff &&
			_speakerPerson < 16 && (int8)_state[0xea] <= 0) {
		_localSpeakerBackdrop.resize(kScreenWidth * kScreenHeight);
		memcpy(_localSpeakerBackdrop.begin(), _framebuffer1, _localSpeakerBackdrop.size());
		_localSpeakerSprites = sprites;
		_localSpeakerPerson = _speakerPerson;
	}
	const int opening = _state[0x47e2] / 2;
	const bool heldReport = _localSpeakerPerson == _speakerPerson && _state[0x47e1] == 0x80 &&
		!_speakerReportLowered && (_speakerPerson == 3 ? opening == 5 || opening == 8 :
		_speakerPerson == 11 ? opening == 5 :
		_speakerPerson == 13 && opening == 6 + 2 * localSpeakerVariant());
	if (heldReport) {
		// 9CA6 leaves the completed card below row 76 while later body
		// choices repaint only above it. A rebuilt room has no retained page:
		// recover that band from the twentieth opening pose, not the current
		// body pose or a replay of A25B/9AB4 and their mutable ledger values.
		memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
		_graphics.setFrameBuffer(_framebuffer2);
		for (uint i = 0; i < sprites.size(); i++)
			drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
		memcpy(_framebuffer1, _framebuffer2, 76 * kScreenWidth);
		memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
		drawSpeakerFigure(_speakerPerson, 19, opening);
		drawSpeakerReport(_framebuffer2);
		memcpy(_framebuffer1 + 76 * kScreenWidth, _framebuffer2 + 76 * kScreenWidth,
				(kSceneHeight - 76) * kScreenWidth);
		_graphics.setFrameBuffer(_framebuffer1);
	} else {
		for (uint i = 0; i < sprites.size(); i++)
			drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
	}
	memcpy(_framebuffer1 + kSceneHeight * kScreenWidth, panelBacking, sizeof(panelBacking));
	rememberLocalSpeechPage();
}

/** C4DD/C4F0: finish a rebuilt local scene after its caption, not before it. */
void DuneEngine::finishLocalSpeakerScene() {
	if (_speakerPerson < 0 || _speakerPerson >= 16 || _localSpeechPerson != _speakerPerson ||
			_localSpeechClean.size() != kScreenWidth * kScreenHeight ||
			_onWorld || _onGlobe || _inCabin || _onMixer || _takingOff || (int8)_state[0xea] > 0 ||
			_state[0x227d])
		return;
	// The clean page retains the full crest. Present only its scene portion,
	// leaving the already drawn menu/panel intact below row 152.
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_localSpeechClean.begin());
	drawFace();
	_graphics.setFrameBuffer(_framebuffer2);
	drawFace();
	memcpy(_framebuffer1, _framebuffer2, kScreenWidth * kSceneHeight);
	_graphics.setFrameBuffer(_framebuffer1);
	const int pictured = localSpeakerPicture();
	const char *name = getResourceFilenameByIndex(pictured + 2);
	if (name)
		openSpriteSheet(name); // 1797 restores the caller's portrait bank.
}

/** 9BB1/9C2D/9D16: unchanged parts do not rebuild or repaint the room. */
bool DuneEngine::repaintLocalSpeakerFigure() {
	const bool reportGesture = (_speakerPerson == 3 &&
		((_speakerReport >= 1 && _speakerReport <= 4) || _speakerReport == 6)) ||
		(_speakerPerson == 11 && _speakerReport == 4) ||
		(_speakerPerson == 13 && _speakerReport == 5);
	if (_onWorld || _onGlobe || _inCabin || _onMixer || _takingOff ||
			_speakerPerson < 0 || _speakerPerson >= 16 || (_speakerReport && !reportGesture) ||
			(int8)_state[0xea] > 0 || _localSpeakerPerson != _speakerPerson ||
			_localSpeakerBackdrop.size() != kScreenWidth * kScreenHeight)
		return false;
	Common::Array<SpeakerSprite> sprites;
	collectCurrentSpeakerSprites(sprites);
	int left = 319, top = 199, right = 0, bottom = 0;
	auto changedParts = [&](const Common::Array<SpeakerSprite> &from,
			const Common::Array<SpeakerSprite> &to) {
		for (uint i = 0; i < from.size(); i++) {
			bool found = false;
			for (uint j = 0; j < to.size(); j++) {
				if (from[i].index == to[j].index && from[i].x == to[j].x && from[i].y == to[j].y) {
					found = true;
					break;
				}
			}
			int width, height;
			if (!found && currentSpriteSheet && currentSpriteSheet->size(from[i].index, &width, &height)) {
				left = MIN(left, from[i].x);
				top = MIN(top, from[i].y);
				right = MAX(right, from[i].x + width);
				bottom = MAX(bottom, from[i].y + height);
			}
		}
	};
	// 9C90 clips the entire pose update above the active mouth, not just
	// the mouth's opaque pixels. Baron and guard deliberately skip this bound.
	// A73C retains the unshifted bank descriptor for this bound; 9DE3 adds
	// the portrait origin only to the separate mouth-paint rectangle.
	const int pictured = localSpeakerPicture();
	auto clipChangedParts = [&]() {
		if (_speakerPerson != 9 && _speakerPerson != 12 && _state[0x47e1] != 0x81 &&
				_mixer->isSoundHandleActive(_speechHandle) && pictured >= 0 && pictured < 17) {
			const int mouthTop = READ_LE_UINT16(_state.begin() + 0x27fc + 8 * pictured);
			if (mouthTop < bottom) {
				bottom = mouthTop;
				if (bottom <= top)
					left = 319;
			}
		}
		// 9CA6 protects the completed report independently of playback.
		if (_state[0x47e1] == 0x80 && bottom > 76) {
			bottom = 76;
			if (bottom <= top)
				left = 319;
		}
	};
	// Each 9C54 pass includes the clipping tail. A fully clipped old-only
	// part resets the left sentinel before new-only parts extend the bounds.
	changedParts(_localSpeakerSprites, sprites);
	clipChangedParts();
	changedParts(sprites, _localSpeakerSprites);
	clipChangedParts();
	if (left == 319 || top >= bottom || left >= right)
		return true; // 9BD1/9A05: no copy, and no new retained part list.
	Common::Rect bounds(left, top, right, bottom);
	bounds.clip(Common::Rect(0, 0, kScreenWidth, kSceneHeight));
	if (bounds.isEmpty())
		return true;
	memcpy(_framebuffer2, _localSpeakerBackdrop.begin(), _localSpeakerBackdrop.size());
	_graphics.setFrameBuffer(_framebuffer2);
	for (uint i = 0; i < sprites.size(); i++)
		drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
	// 9D16 commits the expanded list only after a changed region was painted.
	_localSpeakerSprites = sprites;
	// 908C restores the retained caption on the CLEAN drawing page, clipped
	// to this dirty rectangle, before C4F0 copies it. Save the composed region
	// too: subsequent compound mouths and reports use this page as their base.
	if (!_speech.empty() && !retainedLocalBalloonCaption(_speech))
		drawSpeech(_speech, kSpeechColour, -1, _framebuffer2);
	for (int y = bounds.top; y < bounds.bottom; y++) {
		const int at = y * kScreenWidth + bounds.left;
		if (_localSpeechPerson == _speakerPerson && _localSpeechClean.size() == kScreenWidth * kScreenHeight)
			memcpy(_localSpeechClean.begin() + at, _framebuffer2 + at, bounds.width());
	}
	// C4F0/1797 follows 908C: the raised crest goes over intersecting
	// subtitles too. Keep its opaque pixels in the clean page without
	// replacing unrelated panel pixels outside the portrait's dirty bounds.
	if (!_state[0x227d] && bounds.bottom >= 137 && bounds.right >= 126 && bounds.left < 194) {
		drawFace();
		if (_localSpeechPerson == _speakerPerson && _localSpeechClean.size() == kScreenWidth * kScreenHeight) {
			_graphics.setFrameBuffer(_localSpeechClean.begin());
			drawFace();
		}
		const char *name = getResourceFilenameByIndex(pictured + 2);
		if (name)
			openSpriteSheet(name); // 1797 restores the selected portrait bank.
		_graphics.setFrameBuffer(_framebuffer2);
	}
	for (int y = bounds.top; y < bounds.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + bounds.left,
				_framebuffer2 + y * kScreenWidth + bounds.left, bounds.width());
	_graphics.setFrameBuffer(_framebuffer1);
	copyFrameRectToScreen(_framebuffer1 + bounds.top * kScreenWidth + bounds.left,
			kScreenWidth, bounds.left, bounds.top, bounds.width(), bounds.height());
	_system->updateScreen();
	return true;
}

void DuneEngine::rememberLocalSpeechPage() {
	const bool flightTalk = _inDesert && _flyingTo >= 0 &&
		_flightDiscoveryPrompt && _menuKind == kMenuDiscovery;
	if (_onWorld || _onGlobe || _inCabin || _onMixer || _takingOff ||
			(_inDesert && !flightTalk && (_flyingTo >= 0 ||
				(_menuKind != kMenuTalk && _menuKind != kMenuPalaceGathering))) || _speakerPerson < 0 ||
			_speakerPerson >= 16 || _menuKind == kMenuTroopNotice || (int8)_state[0xea] > 0) {
		_localSpeechClean.clear();
		_localSpeechVisible.clear();
		_localSpeechPerson = -1;
		return;
	}
	if (_localSpeechPerson != _speakerPerson) {
		_localSpeechMouth = -1;
		_localSpeechVisible.clear();
		_localSpeechWindow = Common::Rect();
	}
	// 9C90 keeps ordinary pose callbacks above the active mouth window.
	// Baron and guard deliberately skip that bound; their poses may replace
	// the mouth even while the cue itself has not changed.
	const bool retainMouth = _localSpeechPerson == _speakerPerson && _localSpeechMouth >= 0 &&
		_speakerPerson != 9 && _speakerPerson != 12 && _mixer->isSoundHandleActive(_speechHandle) &&
		!_localSpeechWindow.isEmpty() &&
		_localSpeechVisible.size() == (uint)(_localSpeechWindow.width() * _localSpeechWindow.height());
	_localSpeechClean.resize(kScreenWidth * kScreenHeight);
	for (int y = 0; y < kScreenHeight; y++) {
		byte *clean = _localSpeechClean.begin() + y * kScreenWidth;
		byte *visible = _framebuffer1 + y * kScreenWidth;
		if (retainMouth && y >= _localSpeechWindow.top && y < _localSpeechWindow.bottom) {
			memcpy(clean, visible, _localSpeechWindow.left);
			memcpy(clean + _localSpeechWindow.right, visible + _localSpeechWindow.right,
				kScreenWidth - _localSpeechWindow.right);
			memcpy(visible + _localSpeechWindow.left,
				_localSpeechVisible.begin() + (y - _localSpeechWindow.top) * _localSpeechWindow.width(),
				_localSpeechWindow.width());
		} else {
			memcpy(clean, visible, kScreenWidth);
		}
	}
	_localSpeechPerson = _speakerPerson;
}

/** A82E/9DE3: a changed cue paints only the bank's local mouth window. */
bool DuneEngine::pollLocalSpeechMouth() {
	const bool flightTalk = _inDesert && _flyingTo >= 0 &&
		_flightDiscoveryPrompt && _menuKind == kMenuDiscovery;
	if (_onWorld || _onGlobe || _inCabin || _onMixer || _takingOff ||
			(_inDesert && !flightTalk && (_flyingTo >= 0 ||
				(_menuKind != kMenuTalk && _menuKind != kMenuPalaceGathering))) || _speakerPerson < 0 ||
			_speakerPerson >= 16 || _menuKind == kMenuTroopNotice || (int8)_state[0xea] > 0 ||
			_localSpeechPerson != _speakerPerson || _localSpeechClean.size() != kScreenWidth * kScreenHeight)
		return false;
	const int mouth = speechMouth();
	if (mouth < 0 || mouth == _localSpeechMouth)
		return false;
	Common::Array<SpeakerSprite> sprites;
	collectSpeakerSprites(_speakerPerson, 0, 0, mouth, sprites, nullptr, true, &_localSpeechWindow);
	_localSpeechMouth = mouth;
	if (sprites.empty() || _localSpeechWindow.isEmpty())
		return false;
	// Multiple sprites paint the clean page and copy the window. A single
	// sprite paints the visible page; transparent pixels retain its history.
	const bool compound = sprites.size() >= 2;
	const byte *base = compound ? _localSpeechClean.begin() : _framebuffer1;
	memcpy(_framebuffer2, base, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	for (uint i = 0; i < sprites.size(); i++)
		drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
	// 908C restores subtitles intersecting the repainted mouth rectangle.
	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour, -1, _framebuffer2);
	_localSpeechVisible.resize(_localSpeechWindow.width() * _localSpeechWindow.height());
	for (int y = _localSpeechWindow.top; y < _localSpeechWindow.bottom; y++) {
		const int at = y * kScreenWidth + _localSpeechWindow.left;
		memcpy(_framebuffer1 + at, _framebuffer2 + at, _localSpeechWindow.width());
		memcpy(_localSpeechVisible.begin() + (y - _localSpeechWindow.top) * _localSpeechWindow.width(),
			_framebuffer2 + at, _localSpeechWindow.width());
		if (compound)
			memcpy(_localSpeechClean.begin() + at, _framebuffer2 + at, _localSpeechWindow.width());
	}
	_graphics.setFrameBuffer(_framebuffer1);
	return true;
}

void DuneEngine::drawTroopPortrait(int person, int sourceLeft, int sourceTop, int animation, int mouth) {
	enum { kSize = 59 };
	const int variant = person == 12 ? 0 : (animation ? animation : fremenAnimation(person));
	const int key = fremenPicture(person) * 256 + variant;
	const bool initial = _troopPortraitKey != key || _troopPortraitVisible.empty();
	Common::Array<SpeakerSprite> sprites;
	// Reinstall the bank's private palette even on an unchanged card, since
	// drawing the underlying map opens PERS/ONMAP first.
	collectSpeakerSprites(person, 0, variant, initial ? -1 : MAX(mouth, 0), sprites);
	_graphics.setFrameBuffer(_framebuffer2);

	auto restore = [&](const Common::Array<byte> &pixels) {
		for (int y = 0; y < kSize; y++)
			memcpy(_framebuffer2 + (sourceTop + y) * kScreenWidth + sourceLeft,
					pixels.begin() + y * kSize, kSize);
	};
	auto capture = [&](Common::Array<byte> &pixels) {
		pixels.resize(kSize * kSize);
		for (int y = 0; y < kSize; y++)
			memcpy(pixels.begin() + y * kSize,
					_framebuffer2 + (sourceTop + y) * kScreenWidth + sourceLeft, kSize);
	};
	auto paint = [&]() {
		for (uint i = 0; i < sprites.size(); i++)
			drawSprite(sprites[i].index, sprites[i].x, sprites[i].y);
	};

	if (initial) {
		memset(_framebuffer2, 0, kScreenWidth * kScreenHeight);
		// 7A67/7B1B fills the portrait interior with DS:18FC before 9D6A
		// paints its transparent parts. Preserve that background in both
		// retained mouth pages, rather than introducing palette index zero.
		_graphics.fillRect(sourceLeft, sourceTop, sourceLeft + kSize,
				sourceTop + kSize, _state[0x18fc]);
		paint();
		capture(_troopPortraitVisible);
		_troopPortraitClean = _troopPortraitVisible; // 7B0F/C4AA
		_troopPortraitKey = key;
	}
	// 9F1C/A75C/9DE3 reload 47C4's bank without repainting 79EE's
	// retained head. A710's PO02C/PO02D guard switch is independent of
	// that head's HARK/FRM identity and original crop rectangle.
	const int mouthPerson = _mapSpeechPerson;
	if (mouthPerson >= 0)
		collectSpeakerSprites(mouthPerson, 0, 0, MAX(mouth, 0), sprites);
	if (mouth >= 0 && (initial || mouth != _troopContactMouth)) {
		if (initial && mouthPerson < 0)
			collectSpeakerSprites(person, 0, variant, mouth, sprites);
		// 9EB1: compound parts repaint the clean page then copy the window;
		// a single mouth sprite paints only the visible page. Rebuilding the
		// face from frame zero on every cue loses pixels retained by this path.
		const bool compound = sprites.size() > 1;
		restore(compound ? _troopPortraitClean : _troopPortraitVisible);
		paint();
		capture(_troopPortraitVisible);
		if (compound)
			_troopPortraitClean = _troopPortraitVisible;
	}
	// A7A5 stops the voice without repainting the last mouth. Keep the card
	// until a new contact/variant or a new cue replaces it.
	restore(_troopPortraitVisible);
	_troopContactMouth = mouth;
}

/** 9AB4 paints completed stock descriptors; invoice/price digits are sprites. */
void DuneEngine::drawSpeakerReport(byte *screen) {
	if (!screen)
		screen = _framebuffer1;
	if ((_speakerPerson == 3 || _speakerPerson == 11) && _state[0x47e1] == 0x80 && !_speakerCardText.empty()) {
		const uint descriptor = READ_LE_UINT16(_state.begin() + 0x47e4);
		if (descriptor + 8 > _state.size())
			return;
		const int left = READ_LE_UINT16(_state.begin() + descriptor + 2);
		int top = READ_LE_UINT16(_state.begin() + descriptor + 4);
		const uint16 colours = READ_LE_UINT16(_state.begin() + descriptor + 6);
		_font.setFace(Font::kFaceAlternate); // 9AB4's D075, not dialogue's tall face.
		uint first = 0;
		for (uint i = 0; i <= _speakerCardText.size(); i++) {
			if (i == _speakerCardText.size() || _speakerCardText[i] == '\r') {
				_font.drawString(screen, left, top, _speakerCardText.substr(first, i - first),
						colours & 255, colours >> 8);
				first = i + 1;
				top += 7;
			}
		}
		_font.setFace(Font::kFaceNormal); // D068 restores the ordinary font.
	}
	// A25B only arms a descriptor. 9AB4 paints its text after all twenty
	// opening frames, not while a shipment answer is being selected. Do not
	// add an immediate amount guessed from B4/1A over the deferred report.
}

/** 9AB4/8865/C4DD: format the descriptor once and present the clean scene. */
void DuneEngine::completeLocalSpeakerReport() {
	if ((_speakerPerson != 3 && _speakerPerson != 11) || _state[0x47e1] != 0x80)
		return;
	const uint descriptor = READ_LE_UINT16(_state.begin() + 0x47e4);
	if (descriptor + 8 > _state.size())
		return;
	const uint16 kind = READ_LE_UINT16(_state.begin() + descriptor);
	if (!kind || kind >= 0x38)
		return; // 9B09 already installed invoice/price sprite digits.
	_speakerCardText = expandDialogueText(command(kind - 1), 0, 0);
	if (_localSpeechPerson != _speakerPerson || _localSpeechClean.size() != kScreenWidth * kScreenHeight) {
		drawScene();
		return;
	}
	drawSpeakerReport(_localSpeechClean.begin());
	Common::Rect bounds(READ_LE_UINT16(_state.begin() + 0x1470), READ_LE_UINT16(_state.begin() + 0x1472),
		READ_LE_UINT16(_state.begin() + 0x1474), MIN<int>(READ_LE_UINT16(_state.begin() + 0x1476), kSceneHeight));
	bounds.clip(Common::Rect(0, 0, kScreenWidth, kSceneHeight));
	if (bounds.isEmpty())
		return;
	// C4DD restores the complete crest on the clean page, then copies only
	// the scene clip. Painting it directly on the display would spill below
	// row 152 into the otherwise retained bottom panel.
	if (!_state[0x227d] && bounds.bottom >= 137 && bounds.right >= 126 && bounds.left < 194) {
		_graphics.setFrameBuffer(_localSpeechClean.begin());
		drawFace();
		const char *name = getResourceFilenameByIndex(localSpeakerPicture() + 2);
		if (name)
			openSpriteSheet(name); // 1797 restores its caller's portrait bank.
	}
	for (int y = bounds.top; y < bounds.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + bounds.left,
			_localSpeechClean.begin() + y * kScreenWidth + bounds.left, bounds.width());
	_graphics.setFrameBuffer(_framebuffer1);
	copyFrameRectToScreen(_framebuffer1 + bounds.top * kScreenWidth + bounds.left,
		kScreenWidth, bounds.left, bounds.top, bounds.width(), bounds.height());
	_system->updateScreen();
}

/**
 * Holds a scene while the person in it talks, walking their animation a frame
 * at a time.  Only the last piece of a frame changes, which is the mouth, so
 * what this shows is a mouth moving on a face that stays put.
 */
PlaybackResult DuneEngine::holdSpeaking(uint32 ms, int speaker) {
	if (speaker < 0)
		return hold(ms);

	const uint32 until = _system->getMillis() + ms;
	while (_system->getMillis() < until && !shouldQuit()) {
		// DD63/D9D2 advances99BE and the VOC cues during the intro too.
		// Retained dirty rectangles restore the old body and mouth pixels.
		updateLocalSpeaker();
		const PlaybackResult asked = hold(MIN<uint32>(10, until - _system->getMillis()));
		if (asked != kPlaybackFinished)
			return asked;
	}
	return kPlaybackFinished;
}

/**
 * Says a line out loud.  The game keeps one recording to a line, named after
 * the line's own number written in hexadecimal: the template at DS:0x37E4 is
 * "PF\\PF001I .VOC", and the four hundredth line, which is 0x190, is the file
 * PF\\PF190I.VOC.  Two hundred and thirty-two of them are in the archive.
 */
/**
 * Ends a line that is being said, so that cutting a scene short does not leave
 * its voice running on over the next one.
 */
void DuneEngine::stopSpeech() {
	_mixer->stopHandle(_speechHandle);
	_speechCues.clear();
	// A7A5 leaves the map's loaded mouth bank and last painted frame intact.
	// It also leaves 99BE's local pose cursor, speech-mode bit and 47CE
	// callback counter intact. Stopping DMA must not switch a completed
	// report back to an unrelated idle sequence with the old frame count.
	_localSpeechMouth = -1;
	_localSpeechVisible.clear();
}

/** AB15: speech owns the shared digital channel; effects never drive mouths. */
void DuneEngine::playSoundEffect(uint effect) {
	if (effect > 10 || !_mixer->isReady() || _mixer->isSoundHandleActive(_speechHandle))
		return;
	// DNSDB 791 refuses to restart an already-active DMA program. A repeated
	// exterior repaint must not restart the harvester every animation frame.
	if (effect && effect == _soundEffect && _mixer->isSoundHandleActive(_soundEffectHandle))
		return;
	stopSoundEffect();
	if (!effect)
		return;
	Resource resource = _archive.openMember(getResourceFilenameByIndex(INDEX_SN1_HSQ + effect - 1));
	if (!resource)
		return;
	Common::Array<byte> data;
	data.resize(resource->size());
	if (resource->read(data.begin(), data.size()) != data.size())
		return;
	_soundEffectStream = makeSoundEffectStream(data.begin(), data.size());
	if (!_soundEffectStream)
		return;
	_soundEffect = effect;
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundEffectHandle, _soundEffectStream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

/** AC14: retire DMA before releasing the stream, including finished effects. */
void DuneEngine::stopSoundEffect() {
	_mixer->stopHandle(_soundEffectHandle);
	delete _soundEffectStream;
	_soundEffectStream = nullptr;
	_soundEffect = 0;
}

/** AC30 is loop release, NOT pause: the current iteration finishes normally. */
void DuneEngine::finishSoundEffectLoop() {
	if (_soundEffectStream)
		_soundEffectStream->finishLoop();
}

int DuneEngine::speechMouth() const {
	return _mixer->isSoundHandleActive(_speechHandle)
		? _speechCues.mouthAt(_mixer->getSoundElapsedTime(_speechHandle)) : -1;
}

/**
 * Says a line out loud.  0xA8BC builds the name into the buffer at DS:0x37DA,
 * which begins as "PF\PF001I .VOC": the letter of the speaker's own set twice
 * over, then three hex figures of the line, then I where the line is being
 * said indoors and O where it is not, and then a letter of its own where the
 * line's top nibble asks for one.
 *
 * The figures are not the line's own number but how far it is past the first
 * line the speaker has, which 0xA70A takes out of the table at DS:0xD7F4;
 * every set counts from one of its own.  Reading them as the line's number and
 * always out of the sixth set is what had everyone saying somebody else's
 * words.
 *
 * 0xA6EC holds the letter at the fifteenth set, so the group of lines that
 * belong to nobody in particular is said in that voice, and 0xAB54 gives the
 * game's own lines the twenty-sixth, which is the set named Z.
 */
void DuneEngine::playSpeech(uint16 line, int speaker, bool report, bool dialogueRequest) {
	enum { kLastVoice = 14, kGameVoice = 25 };
	// 9EFD retains the actual recording request for 9ED5's WHAT replay.
	// It need not be the current dialogue record: room announcements and
	// troop/order replies can speak directly. These DOS fields are saved
	// with DS, so the same recording remains available after loading.
	if (speaker >= 0 && _state.size() > 0x47dd) {
		// 88AF stores its request unchanged. Dialogue records carry 0800h;
		// 80DF/8117's direct route instruction does not. 9EFD plays both
		// kinds without adding that bit itself.
		WRITE_LE_UINT16(_state.begin() + 0x4780, line | (dialogueRequest ? 0x800 : 0));
		WRITE_LE_UINT16(_state.begin() + 0x47c4, (uint16)speaker);
		_state[0x47dd] = report ? 1 : 0;
	}
	// A72A ends the previous line even when neither take of the new one exists.
	stopSpeech();
	stopSoundEffect(); // A83F/AC14 stops effects before trying either voice asset.

	// Whether the line is being said indoors, which 0xA8E3 reads out of the
	// game's own memory: the player must be at a place and in a room of it
	// other than the one that looks out of doors.
	const bool indoors = _state.size() > 0xea && (int8)_state[0xea] <= 0
						 && _state[6] == 0x80 && _state[4] != 1;

	int set = kGameVoice;
	uint16 which = line;

	if (speaker >= 0) {
		set = MIN(speaker, (int)kLastVoice);

		// 0xA6E6 keeps the speaker on the stack, holds BL down to fourteen and
		// then subtracts the word at DS:0xD7F4 indexed by that held-down number
		// - so the number a line is filed under is worked out from the voice it
		// is recorded in, not from who says it.  The Fremen, their chiefs and
		// the lines that belong to nobody all speak in PO, and PO's two hundred
		// and twenty-seven recordings are exactly the Fremen's own line 211 up
		// to the last chief's line 437.  Taking each of those three their own
		// beginning put a chief's every line thirty-two recordings early and the
		// nobody group's two hundred and twenty-seven early, so a chief asked to
		// work for Paul answered in another Fremen's words.
		// A6F1 handles automatic reports before A701 checks227D. Intro
		// recordings bypass the ordinary phrase base, even for named people.
		which = speechRecordingNumber(line, _dialogue.speakerBase((uint)set),
				_dialogue.speakerBase(16), report,
				_state.size() > 0x227d && _state[0x227d] != 0);
	}

	const char letter = (char)('A' + set);
	// 94E8 adds 0x1000 to the retained request for each subsequent PHRASE
	// page. A8FD/A8FF OR that high nibble with the contextual voice variant:
	// page two is B, page three C, not another take of the first page.
	const uint variant = (which >> 12) | (_state.size() > 0x47e0 ? _state[0x47e0] : 0);
	const Common::String suffix = variant ? Common::String::format("%c", (char)('A' + variant)) : "";

	// The name carries a space where the last letter would go, and the archive
	// keeps the names without it.
	Common::String name = Common::String::format("P%c\\P%c%03X%c%s.VOC", letter, letter,
												which & 0xfff, speaker < 0 || indoors ? 'I' : 'O', suffix.c_str());

	Resource voice = _archive.openMember(name);

	if (!voice && speaker >= 0) {
		// A745/A74A retries the opposite contextual take when the first
		// archive lookup fails.
		const Common::String other = Common::String::format("P%c\\P%c%03X%c%s.VOC", letter, letter,
														which & 0xfff, indoors ? 'O' : 'I', suffix.c_str());
		voice = _archive.openMember(other);

		if (voice) {
			debugC(2, kDebugSound, "line %d is only recorded as %s", line, other.c_str());
			name = other;
		}
	}

	if (!voice) {
		debugC(2, kDebugSound, "no recording of line %d (%s)", line, name.c_str());
		return;
	}
	// A710 changes the pictured speaker for the two guard warnings, but
	// A8BC still names the recording in PO. Do not change the voice filename.
	const int portraitPerson = set == 14 && (which & 0xffe) == 0x2c ? 12 : speaker;
	playVoice(voice, name, portraitPerson >= 0 && portraitPerson < 16 ? portraitPerson : -1);
}

/** 9985: finish the current pose block without advancing the callback timer. */
void DuneEngine::finishLocalSpeakerBlock() {
	if (_onWorld || _onGlobe || _inCabin || _onMixer || _takingOff ||
			_speakerPerson < 0 || _speakerPerson >= 16 || (int8)_state[0xea] > 0)
		return;
	const int variant = localSpeakerVariant();
	const bool reportGesture = (_speakerPerson == 3 &&
		((_speakerReport >= 1 && _speakerReport <= 4) || _speakerReport == 6)) ||
		(_speakerPerson == 11 && _speakerReport == 4) ||
		(_speakerPerson == 13 && _speakerReport == 5);
	const int opening = _speakerPerson == 13 ? 6 + 2 * variant : (_speakerReport == 6 ? 8 : 5);
	// Other legacy action animations do not yet use the 99BE counters.
	if ((_speakerReport && !reportGesture) ||
			(_speakerAnimation > 3 && _speakerPerson != 12 && variant < 0 && !reportGesture && !_speakerReportLowered) ||
			(!_speakerSpeechMode && (!_speakerIdleActive || _speakerIdlePerson != _speakerPerson)))
		return;
	auto remaining = [&]() {
		// 9AB4 starts at twenty, then 99F6 consumes the first frame.
		if (reportGesture && _speakerAnimation == opening)
			return 19 - _speakerFrame;
		return _speakerSpeechMode ? _speakerVoiceRemaining : _speakerIdleRemaining;
	};
	while (remaining() & 7) {
		// This is a direct 99BE call, not a timer poll. In particular an
		// idle block ending here selects its successor through E3B7.
		if (advanceLocalSpeakerAnimation() && !repaintLocalSpeakerFigure()) {
			const bool flightTalk = (_inDesert && _flyingTo >= 0 && _flightDiscoveryPrompt &&
				_menuKind == kMenuDiscovery) || carriedPortraitVisible();
			if (flightTalk)
				showFlightDiscovery();
			else if (_inDesert)
				drawDesertDialogue();
			else
				drawScene();
		}
	}
}

/** 9ED5 completes the pose block, then repeats the selected recording. */
void DuneEngine::repeatSpeech() {
	if (_state.size() > 0x47e1 && READ_LE_UINT16(_state.begin() + 0x47c4) < 16) {
		finishLocalSpeakerBlock();
		if (_state[0x47e1] == 0x81)
			_state[0x47e1] = 1; // 9EDF tests the raw caller, including remote portraits.
	}
	if (_state.size() > 0x47dd) {
		const uint16 line = READ_LE_UINT16(_state.begin() + 0x4780);
		const uint16 person = READ_LE_UINT16(_state.begin() + 0x47c4);
		if (line && person <= 16) {
			// 9ED5 repeats a direct instruction too, retaining its request
			// kind rather than turning it into a dialogue-record request.
			playSpeech(line & ~0x0c00, person, _state[0x47dd] != 0, (line & 0x800) != 0);
			return;
		}
	}
	// Older native saves did not retain these DOS audio fields. Recover
	// their current record without selecting news or rerunning its action.
	if (_speakerPerson >= 0 && _dialoguePart < _dialogue.count() &&
			_dialogueLine < _dialogue.entry(_dialoguePart).size()) {
		const uint page = _speechPart < _speechParts.size() ? _speechPart : 0;
		playSpeech((uint16)(_dialogue.entry(_dialoguePart)[_dialogueLine].line + page * 0x1000),
				_speakerPerson, _dialoguePart == 132 || _dialoguePart == 133);
	}
}

void DuneEngine::playVoice(Resource voice, const Common::String &name, int portraitPerson, bool dispatchAudioHours,
		uint32 sampleLimit) {
	stopSoundEffect();
	// A83F/AE2F rejects unavailable digital output before 9EFD can enter
	// 9F1C's speech pose. Do not consume its random draw, listening credit
	// or cue state when the backend cannot start a stream.
	if (!_mixer->isReady())
		return;
	const uint32 size = voice->size();
	if (size < 32)
		return;

	byte *samples = new byte[size];
	voice->read(samples, size);

	// The header is followed by VOC blocks, including Dune's mouth-cue block.
	static const char *const kSignature = "Creative Voice File\x1a";
	if (memcmp(samples, kSignature, 20) != 0) {
		delete[] samples;
		return;
	}

	uint32 at = READ_LE_UINT16(samples + 20);
	uint rate = 0;
	uint32 from = 0;
	uint32 samplesSize = 0;

	// The file is a run of blocks; the one that carries sound is the first,
	// and the rate is kept as the divisor the DOS driver loaded its timer
	// with.  These recordings open with a block of another kind, so the blocks
	// are walked rather than the first one taken.
	while (at + 4 <= size) {
		const byte kind = samples[at];
		if (kind == 0)
			break;

		const uint32 length = samples[at + 1] | (samples[at + 2] << 8) | (samples[at + 3] << 16);
		if (at + 4 + length > size)
			break;

		if (kind == 1 && length > 2) {
			rate = 1000000 / (256 - samples[at + 4]);
			from = at + 6;
			samplesSize = length - 2;
			break;
		}

		at += 4 + length;
	}

	if (!samplesSize) {
		debugC(2, kDebugSound, "%s carries no sound", name.c_str());
		delete[] samples;
		return;
	}
	if (sampleLimit)
		samplesSize = MIN(samplesSize, sampleLimit);

	debugC(1, kDebugSound, "speaking %s at %d Hz", name.c_str(), rate);
	// 9EFD/9F1C initialize a speech pose after A6CC successfully loads a
	// recording, for local speech too. Even though positive-EA reports suppress
	// its drawing, 9A7B consumes one secondary random word. Do this before
	// playback advances the diagnostic clock; failed/missing recordings must
	// not consume it.
	const uint16 poseRandom = portraitPerson >= 0 ? troopRandom(true) : 0;

	// Scripted reference runs press through a conversation without waiting for
	// its recording. In ordinary play the displayed hour keeps advancing while
	// 1B0D holds troop/day work until the voice finishes. Account for that
	// omitted listening time before starting the stream in the no-delay
	// diagnostic, so the next scripted action sees the resulting world state.
	if (ConfMan.getBool("dune_nodelay") && rate != 0) {
		const uint32 duration = (uint32)(((uint64)samplesSize * 1000) / rate);
		if (dispatchAudioHours)
			advanceClock(duration);
		else
			advanceClockElapsed(duration, false, false); // AB45/E3A0 never enter1B0D.
	}

	Audio::SeekableAudioStream *speech = Audio::makeRawStream(samples + from, samplesSize, rate,
															  Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);

	stopSpeech();
	if (portraitPerson >= 0) {
		if (!_speechCues.calibrated()) {
			Resource frequency = _archive.openMember("FREQ.HSQ");
			if (frequency) {
				Common::Array<byte> calibration;
				calibration.resize(frequency->size());
				if (frequency->read(calibration.begin(), calibration.size()) == calibration.size())
					_speechCues.calibrate(calibration.begin(), calibration.size());
			}
		}
		_speechCues.load(samples, size);
	}
	if (_onWorld && hasTroopContact() &&
			(portraitPerson == 12 || portraitPerson == 14 || portraitPerson == 15)) {
		_mapSpeechPerson = portraitPerson;
		_troopContactMouth = -1; // A740 resets DC2A even for a repeated cue.
		Common::Array<SpeakerSprite> sprites;
		// 7A9C binds the map chief through 4758, independently of the
		// unrecruited room Fremen at 4756. Keep that identity for speech too.
		collectSpeakerSprites(portraitPerson, 0, 0, -1, sprites);
	}
	// 9F1C uses the one E3B7 draw above to enter the named speaker's fifth
	// animation, at an eight-frame boundary selected by its low 0x38 bits.
	// Keep the room's idle/gesture pose separately: a stopped recording must
	// not leave a speech-only mouth on the portrait. Map cards and positive-EA
	// reports have their own painters and do not display this local sequence.
	if (portraitPerson >= 0 && portraitPerson < 16 && !_onWorld &&
			_menuKind != kMenuTroopNotice && _speakerPerson == portraitPerson) {
		const int variant = localSpeakerVariant();
		const bool heldReport = _state[0x47e1] == 1 &&
			((_speakerPerson == 3 && ((_speakerReport >= 1 && _speakerReport <= 4) || _speakerReport == 6) &&
				_speakerAnimation == (_speakerReport == 6 ? 8 : 5)) ||
			 (_speakerPerson == 11 && _speakerReport == 4 && _speakerAnimation == 5) ||
			 (_speakerPerson == 13 && _speakerReport == 5 && _speakerAnimation == 6 + 2 * variant));
		if (heldReport) {
			// Successful 9F1C replaces the report cursor with the speech
			// cursor. A failed audio request must retain the held pose.
			_speakerAnimation = MAX(variant, 0);
			_speakerFrame = 0;
		}
		_speakerVoiceActive = true;
		_speakerSpeechMode = true;
		_speakerVoiceAnimation = variant >= 0 ? variant :
			(portraitPerson == 7 && storyStage() >= 200 ? 5 : 4);
		if (variant < 0 && portraitPerson == 12 && (_state[0x10a7] & 0x10))
			_speakerVoiceAnimation += 5;
		_speakerVoiceFrame = poseRandom & (variant >= 0 ? 0x18 : 0x38);
		_speakerVoiceRemaining = 8;
		// 9F1C does not re-register 99BE: D9D2 keeps its partial interval.
	}
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, speech);

	// The stream keeps a pointer into what was read, so it is not freed here.
	delete[] _spokenLine;
	_spokenLine = samples;
}

/**
 * Slides two sprites of a bank in from the left, which the step at 0x08B6
 * does: it starts the x at ninety-six to the left of the screen and counts it
 * up by thirty-two until it reaches nought, drawing the fifth and sixth
 * sprites of BACK.HSQ at thirteen down each time.
 */
void DuneEngine::slideIn(uint resource, int first, int second, int y) {
	const char *name = getResourceFilenameByIndex(resource);
	if (!name)
		return;

	openSpriteSheet(name);

	// The original slides it in from ninety-six off the left; the blitter here
	// cannot draw a sprite that hangs over an edge, so only the places it fits
	// are drawn and the sliding is not seen.
	for (int x = -96; x <= 0 && !shouldQuit(); x += 32) {
		_graphics.setFrameBuffer(_framebuffer1);
		drawSprite(first, x, y);
		drawSprite(second, x, y);

		copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (hold(1000 / 12) == kPlaybackAborted)
			return;
	}
}

/**
 * Which room a place is drawn in, which 0x5E4F decides from the place's own
 * byte at +8 and 0x2D74 turns into a resource by adding it to a hundred and
 * sixty-one: under 0x20 is a sietch, 0x20 exactly is the palace, up to 0x27 a
 * village, and anything above that a Harkonnen hold.  Of the seventy places
 * one is the palace, six are villages, two and twenty are Harkonnen and the
 * rest are sietches.
 */
const byte *DuneEngine::roomRecord(byte kindOfPlace, byte room) const {
	const uint32 pointer = 0x13c4 + 2 * kindOfPlace;

	if (room == 0 || pointer + 1 >= _state.size())
		return nullptr;

	const uint32 rows = READ_LE_UINT16(_state.begin() + pointer);
	if (rows == 0)
		return nullptr;

	const uint32 row = rows + 5 * (room - 1u);
	if (row + 5 > _state.size())
		return nullptr;

	debugC(3, kDebugScript,
		   "room table kind %02x room %u: pointer %04x, row %04x = %02x %02x %02x %02x %02x",
		   kindOfPlace, room, rows, row, _state[row], _state[row + 1], _state[row + 2],
		   _state[row + 3], _state[row + 4]);

	return _state.begin() + row;
}

void DuneEngine::roomFor(uint place) {
	const uint32 at = 0x100 + 28 * place;
	const byte what = at + 8 < _state.size() ? _state[at + 8] : 0x20;
	const byte kind = _state.size() > 0xb ? _state[0xb] : 1;

	// Which file the room is in, from 0x5E4F and 0x2D74.
	if (what < 0x20)
		_salFile = "SIET.SAL";
	else if (what == 0x20)
		_salFile = "PALACE.SAL";
	else if (what < 0x28)
		_salFile = "VILG.SAL";
	else
		_salFile = "HARK.SAL";

	// Which room of it, and out of which bank, is not a matter of the kind of
	// place: 0x3EFE reads the pointer the table at DS:0x13C4 keeps for the
	// place's own byte at +8, steps five bytes for every kind of arrival past
	// the first, and the byte it lands on says both.  0x3B59 takes one off it,
	// puts the high nibble to the bank - nineteen resources on from ICONES -
	// and leaves the low nibble the room.
	//
	// So the palace under the tenth kind, which is where the game begins, is
	// the byte 0x61: the sixth bank, POR.HSQ, and the first of its rooms.
	_place = 0x60;

	// Which room of a place the player comes to is the low byte of the word at
	// DS:0x4, which arriving sets, and 0x3EFE reads the room's record out of
	// the place's own table.  The first room is the place seen from outside:
	// its byte names the empty room of GENERIC.HSQ, because what is seen there
	// is the picture drawn under it rather than a scene of its own.  Stepping
	// past it to find a room with a scene, which is what used to happen here,
	// put the player inside every place they came to.
	const byte *row = roomRecord(what, (byte)(kind > 0 ? kind : 1));

	if (row && row[0])
		_place = (byte)(row[0] - 1);

	// 39EC/3A0B replaces the exterior's room command with this village's
	// own identifier before 3B59 selects VILG.SAL. Its six distinct layouts
	// place the merchant on different marks; using the generic row's 01
	// for every village also magnified the wrong corner during contact.
	if (what == 0x21 && kind == 1)
		_place = (byte)(_state[at] - 1);

	// The room draws its own picture; nothing is laid under it.
	_background = "";

	debugC(1, kDebugScript, "place %u (0x%02x at +8) under kind %d: %s, bank %d, room %d",
		   place, what, kind, _salFile.c_str(), (_place >> 4) & 0xf, _place & 0xf);
}

void DuneEngine::advanceStory(byte stage) {
	if (_state.size() <= 0xff || stage <= _state[0x2a])
		return;

	_state[0x2a] = stage;
	_state[0xff] = 0;
	// 121F/122D selects and marks the automatic story record silently before
	// dispatching this stage's setup. Preserve the current character's text,
	// voice and menu; this is not a narrator presentation.
	markAutomaticStoryRecord();

	debugC(1, kDebugScript, "the story moves on to stage %d", stage);

	const bool wasSettingUp = _inStageSetup;
	_inStageSetup = true;
	runStageSetup(stage);
	_inStageSetup = wasSettingUp;
}

/** DOS 5323/5133: bearing of a place from the supplied world point. */
byte DuneEngine::placeBearing(uint16 longitude, int16 latitude, uint16 place) const {
	if (_state.size() < 65536 && (uint32)place + 6 > _state.size())
		return 0;
	auto wordAt = [&](uint16 at) -> uint16 {
		if (at == 0xffff)
			return (uint16)(_state[at] | (_state[0] << 8));
		return READ_LE_UINT16(_state.begin() + at);
	};
	int across = (int16)(wordAt((uint16)(place + 2)) - longitude);
	int down = (int16)(wordAt((uint16)(place + 4)) - latitude);
	if (down < -128 || down >= 128) {
		// SAR rounds negative odd numbers down, unlike C++ division.
		across = across >= 0 ? across / 2 : (across - 1) / 2;
		down = down >= 0 ? down / 2 : (down - 1) / 2;
	}
	// 514A copies only BL into BH; latitude keeps its low byte.
	const int scaledDown = (int8)down * 256;
	int bearing = 0;
	if (ABS(across) >= ABS(scaledDown)) {
		if (across)
			bearing = 32 * scaledDown / across + (across >= 0 ? 0x40 : 0xc0);
	} else {
		const int part = 32 * across / scaledDown;
		bearing = scaledDown >= 0 ? 0x80 - part : -part;
	}
	return (byte)(((bearing + 0x10) & 0xff) >> 5);
}

/**
 * What lies nearest a place, which 0x5274 works out and keeps in five slots of
 * six bytes each - how far off the nearest of that sort is, which place it is,
 * and which of the eight bearings it lies on:
 *
 *   DS:0xCA  the nearest place of any sort at all
 *   DS:0xD0  the nearest sietch already found
 *   DS:0xD6  the nearest sietch still to be found, of those the story has
 *            come far enough for - a place's byte at +0x0B says which stage
 *            that is, and one beyond it is passed over altogether
 *   DS:0xDC  the nearest village or fort already found
 *   DS:0xE2  the nearest village or fort still to be found
 *
 * This is how a chief knows there is a sietch to be found and which way it
 * lies: his line's condition reads the slot at DS:0xD6, and the bearing at
 * DS:0xDA picks which of the eight ways his sentence names.  Without it the
 * slots held whatever they were left with and he never offered the course.
 */
void DuneEngine::nearestPlaces(uint from) {
	enum {
		kPlacesAt = 0x100, kPlaceSize = 28, kEndOfPlaces = 0xff,
		kAnything = 0xca, kFoundSietch = 0xd0, kHiddenSietch = 0xd6,
		kFoundVillage = 0xdc, kHiddenVillage = 0xe2, kNothing = 0xffff,
		kSlot = 6, kBearingName = 0x11fd, kFirstBearingWord = 0xda
	};

	if (from < kPlacesAt || from + kPlaceSize > _state.size() ||
			_state.size() <= kHiddenVillage + 4)
		return;

	static const uint kSlots[] = { kAnything, kFoundSietch, kHiddenSietch,
								   kFoundVillage, kHiddenVillage };

	for (uint i = 0; i != ARRAYSIZE(kSlots); i++) {
		WRITE_LE_UINT16(_state.begin() + kSlots[i], (uint16)kNothing);
	}

	const uint16 longitude = READ_LE_UINT16(_state.begin() + from + 2);
	const int16 latitude = (int16)READ_LE_UINT16(_state.begin() + from + 4);

	for (uint32 at = kPlacesAt; at + kPlaceSize <= _state.size(); at += kPlaceSize) {
		if (_state[at] == kEndOfPlaces)
			break;
		if (at == from)
			continue;

		// How far off it is, which 0x5299 takes as the greater of how far it is
		// round the world - counted in the top byte of the sixteen-bit longitude
		// - and how far it is up or down.
		const int across = ABS((int)(int16)(READ_LE_UINT16(_state.begin() + at + 2)
										 - longitude)) >> 8;
		const int down = ABS((int)(int16)READ_LE_UINT16(_state.begin() + at + 4)
							 - latitude);
		const uint16 off = (uint16)MAX(across, down);

		const bool hidden = (_state[at + 0xa] & 0x80) != 0;
		uint slot;

		if (_state[at + 8] >= 0x28) {
			slot = hidden ? (uint)kHiddenVillage : (uint)kFoundVillage;
		} else if (!hidden) {
			slot = kFoundSietch;
		} else {
			// A sietch still to be found is only counted once the story has
			// reached the stage its own byte at +0x0B names.
			if (storyStage() < _state[at + 0xb])
				continue;
			slot = kHiddenSietch;
		}

		if (off < READ_LE_UINT16(_state.begin() + slot)) {
			WRITE_LE_UINT16(_state.begin() + slot, off);
			WRITE_LE_UINT16(_state.begin() + slot + 2, (uint16)at);
		}

		if (off < READ_LE_UINT16(_state.begin() + kAnything)) {
			WRITE_LE_UINT16(_state.begin() + kAnything, off);
			WRITE_LE_UINT16(_state.begin() + kAnything + 2, (uint16)at);
		}
	}

	// And which way each of them lies, which 0x5323 works out of the two places
	// and keeps as one of eight. 5133 scales latitude by 256 and divides the
	// smaller signed component by the larger; it is not an atan2 angle. 5274
	// resets only the five distance words, leaving an unmatched slot's old
	// place pointer for 5323 to read as the original does.
	// Even an unmatched 0xFFFF pointer is dereferenced by DOS, with 16-bit
	// address wrap; that matters before a hidden sietch becomes discoverable.
	for (uint i = 0; i != ARRAYSIZE(kSlots); i++) {
		const uint16 place = READ_LE_UINT16(_state.begin() + kSlots[i] + 2);
		_state[kSlots[i] + 4] = placeBearing(longitude, latitude, place);
	}

	// 0x530E keeps which of the eight words names the way the hidden sietch
	// lies, for the sentence that offers the course to say.
	if (_state.size() > kBearingName + 1)
		WRITE_LE_UINT16(_state.begin() + kBearingName,
						(uint16)(kFirstBearingWord + _state[kHiddenSietch + 4]));

	debugC(2, kDebugScript,
		   "nearest to %04x: any %04x, sietch %04x, hidden sietch %04x %d off "
		   "the %u way, village %04x, hidden village %04x",
		   from, READ_LE_UINT16(_state.begin() + kAnything + 2),
		   READ_LE_UINT16(_state.begin() + kFoundSietch + 2),
		   READ_LE_UINT16(_state.begin() + kHiddenSietch + 2),
		   READ_LE_UINT16(_state.begin() + kHiddenSietch),
		   _state[kHiddenSietch + 4],
		   READ_LE_UINT16(_state.begin() + kFoundVillage + 2),
		   READ_LE_UINT16(_state.begin() + kHiddenVillage + 2));
}

/**
 * Lets the player in on a place, which is what the routine at 0x425B does: if
 * the place is still to be found it clears the flag at +0x0A, empties +0x0B,
 * counts it at 0x27 when the byte at +8 is under 0x20, and moves the story to
 * stage 16 when the place it has just revealed is the one whose first word is
 * 0x0603.
 */
void DuneEngine::revealPlace(uint place) {
	if (place + 0xc > _state.size())
		return;

	if (!(_state[place + 0xa] & 0x80))
		return;

	_state[place + 0xa] &= 0x7f;
	_state[place + 0xb] = 0;

	if (_state[place + 8] < 0x20 && _state.size() > 0x27)
		_state[0x27]++;

	_map.readPlaces(_state.begin(), _state.size());

	debugC(1, kDebugScript, "the place at %04x is known now", place);

	if (READ_LE_UINT16(_state.begin() + place) == 0x0603)
		advanceStory(16);
}

/** DOS 1771 installs an event-bytecode cursor and makes its callback active. */
void DuneEngine::beginEventScript(uint16 cursor) {
	if (_state.size() <= 0x477b)
		return;
	WRITE_LE_UINT16(_state.begin() + 0x477a, cursor);
	WRITE_LE_UINT16(_state.begin() + 0x4778, 0);
	_state[0x4774]++;
	_state[0x2788]++; // 177E/B2B9 freezes every event, not only MAP2.
	playTheRightSong(); // 1781/AD5E observes the incremented active-event byte.
	WRITE_LE_UINT16(_state.begin() + 0x4776,
		(uint16)(_state[4] | ((uint16)_state[0x46e0] << 8)));
	// 1794/DA25 registers 176B at a period of 100 five-millisecond ticks.
	// Keep the DOS phase byte, but use the native callback clock/table.
	_eventMenuTime = _system->getMillis() / 5;
	_eventMenuRemainder = 0;
}

/** 1748/1755: release the event and its clock lock, retaining its FF cursor. */
void DuneEngine::releaseEventScript() {
	_state[0x4774] = 0;
	_state[0x2788] = 0; // B2BE clears, rather than decrements, the lock.
}

/**
 * Native presentation helper for a game-owned PHRASE line. Unlike DOS 1771,
 * this does not start an event script. Numeric 0x92 marks use the same
 * scratch-state words as character dialogue.
 */
void DuneEngine::sayLine(uint16 packed) {
	// Story setup can produce an economy/status notice as an action of the
	// character record currently being read. Replacing _speech and clearing
	// the talk menu here erased Jessica's communication-room description before
	// the completed record was drawn. Leave the active character UI and voice
	// alone; a later game-line consumer can present the recorded notice.
	if (_inStageSetup && _speakerPerson >= 0) {
		debugC(1, kDebugScript, "game line %04x waits behind the active character line", packed);
		return;
	}

	// A line of the game's own, which is said in the set named Z.
	playSpeech(packed & 0x3ff, -1);

	_speech = fillInLine(phrase(packed & 0x3ff));
	_speaker = "";
	_menu.clear();

	debugC(1, kDebugScript, "the game says: %s", _speech.c_str());
}

/**
 * Puts something in the queue of what is waiting to happen, which the routine
 * at 0x26DA keeps at DS:0x1179: ten retained messages, with their count at
 * 0xC8 and the unread count at 0xC9. Something already waiting is not
 * queued twice, and when the ten are full the oldest gives way.
 */
void DuneEngine::queueEvent(uint16 what) {
	const uint32 queue = 0x1179;
	const uint kDepth = 10;

	if (_state.size() <= queue + 2 * kDepth)
		return;

	byte waiting = _state[0xc8];

	for (uint i = 0; i != MIN<uint>(waiting, kDepth); i++) {
		if (READ_LE_UINT16(_state.begin() + queue + 2 * i) == what)
			return;
	}

	if (waiting >= kDepth) {
		for (uint i = 0; i != kDepth - 1; i++) {
			WRITE_LE_UINT16(_state.begin() + queue + 2 * i,
							READ_LE_UINT16(_state.begin() + queue + 2 * (i + 1)));
		}

		waiting = kDepth - 1;
	}

	WRITE_LE_UINT16(_state.begin() + queue + 2 * waiting, what);
	_state[0xc8] = waiting + 1;
	_state[0xc9]++;
	// DOS 2717 notifies Paul about a newly retained palace transmission
	// once the story reaches stage 56, unless he is already in room eight.
	// 29EE keeps the separate automatic queue from repeating this notice.
	if (storyStage() >= 56 && _state[0xb] != 8)
		queueTroopNotice(0x0201, 0);

	debugC(1, kDebugScript, "%04x is waiting to happen, %d in the queue", what, _state[0xc8]);
}

/**
 * What a stage of the story sets up.  The original keeps a routine for each
 * even stage from 0 to 54 in the table at 0x11E7 and runs it as the story
 * moves on.  Most of them let the player in on places and set bytes of the
 * game's memory; those are written out here, and the ones that go on to start
 * something - a movie, a scene, a flight - are not, so the story moves on
 * without them rather than with something invented in their place.
 */
void DuneEngine::runStageSetup(byte stage) {
	auto revealPlaces = [this](uint16 offset) {
		const byte *entry = _dosCode.begin() + offset;
		while (const uint16 place = READ_LE_UINT16(entry)) {
			revealPlace(place);
			entry += 2;
		}
	};

	if (_state.size() < 0x1300)
		return;

	switch (stage) {
	case 4:  // 0x1011
		_state[0x122a]--;
		revealPlaces(0x1021);
		_state[0xfe8] = 9;
		break;

	case 8:  // 0x1027
		_state[0x122e] &= 0x7f;
		updateCompass(); // 0x102C jumps to 0x2FFB, before any scene redraw
		break;

	case 12:  // 0x102F
		_state[0x124a] &= 0x7f;
		_state[0x1247] &= 0x7f;
		WRITE_LE_UINT16(_state.begin() + 0x121d, 0xffff);
		beginEventScript(0x1321);
		break;

	case 13: // Duncan's advice; revealing the harvester site starts stage 16
		break;

	case 16:  // 0x1045
		_state[0xfd8] = 5;
		_state[0xfe8] = 9;
		queueEvent(0x010b);
		break;

	case 20: // 0x1053
		_state[0xfd8] = 10;
		revealPlaces(0x1069);
		break;

	case 28: // 0x10A4
		_state[0x1245] &= 0x7f;
		WRITE_LE_UINT16(_state.begin() + 0x1217, 0xffff);
		// 10AF tail-calls 2FFB after unlocking the east corridor. Gurney's
		// existing record belongs to 1071, not this door/compass update.
		updateCompass();
		break;

	case 32: // 0x10B2
		// 10B2 activates Thufir's existing standing-position word. It does
		// not relocate him or replace the low byte of that position.
		_state[0xffb] = 1;
		break;

	case 40: // 0x10B8
		revealPlace(0x800);
		break;

	case 44: // 0x10BE
		WRITE_LE_UINT16(_state.begin() + 0x1154, hour());
		WRITE_LE_UINT16(_state.begin() + 0x1018, 0x2006);
		WRITE_LE_UINT16(_state.begin() + 0xff8, 0x2008);
		WRITE_LE_UINT16(_state.begin() + 0xffa, 0x180);
		_state[0xfe8] = 0x0a;
		WRITE_LE_UINT16(_state.begin() + 0xfea, 0x180);
		WRITE_LE_UINT16(_state.begin() + 0x1201, 0x109);
		addCharisma(20);
		_state[0xa] |= 0x10;

		revealPlaces(0x10f7);
		break;

	case 48: // 0x1103
		// 0x1103 first calls 0x29EE with AX=4 and DI=0, putting Leto's
		// immediate palace warning in the four-byte automatic-event queue.
		// Only then does it put 0x1409 in the separate retained-message
		// queue: that word is edition 0x14 from person nine, the Baron's
		// transmission which Thufir directs Paul to view afterwards.
		queueTroopNotice(4, 0);
		queueEvent(0x1409);
		break;

	case 52: // 0x110F
		if (_state[0xfe8] != 8) {
			_state[0xfe8] = 0x0a;
			WRITE_LE_UINT16(_state.begin() + 0xfea, 0x180);
		}
		queueEvent(0x280a);
		break;

	case 56: // 0x1127
		_state[0xfdb] = 0xff;
		break;

	case 64: // 0x112D
		_state[0x1067] |= 2;
		break;

	case 68: // 0x1133
		revealPlace(0x3d8);
		break;

	case 72: // 0x1139
		addCharisma(10);
		beginEventScript(0x1313);
		_state[0x1057] = (byte)((_state[0x1057] | 0x10) & 0xfd);
		_state[0x1178] = (byte)(_state[0x28] + 2);

		revealPlaces(0x115c);
		break;

	case 76: // 0x1166: Jessica returns to the palace and sends a notice.
		_state[0x1141]++;
		_state[0xfe8] = 2;
		WRITE_LE_UINT16(_state.begin() + 0xfea, 0x180);
		// 1175 loads AX=0105 and jumps to 29EE, which clears DI before
		// enqueuing the automatic report. Without it, filling Chani's new
		// rally quota advances the story but loses Jessica's notification.
		queueTroopNotice(0x0105, 0);
		break;

	case 80: // 0x117B, as far as it is state
		_state[0xa] |= 0x40;
		addCharisma(0x28);
		// The common tail at 0x101B is part of this setup as well.  It sends
		// Jessica to the palace greenhouse (room nine); omitting the jump left
		// her in the old store room and made the newly opened greenhouse clue
		// impossible to complete after the first worm ride.
		_state[0xfe8] = 9;
		break;

	case 84: // 0x1188, as far as it is state
		_state[0x1259] &= 0x7f;
		WRITE_LE_UINT16(_state.begin() + 0x1211, 0xffff);
		updateCompass(); // 0x1193 jumps to the same room-control refresh
		break;

	case 88: // 0x1196 - the prospectors say they are no fighters, and the
		// places the list at 0x11A7 names are let in on
		_state[0xa] |= 0x20;
		beginEventScript(0x12fb);
		revealPlaces(0x11a7);
		break;

	case 92: // 0x11B3
		_state[0x1038] = 5;
		_state[0x11d0] += 0xc;
		// 0x11BD asks 0x1AC5 for the day - the clock's high twelve bits - and
		// keeps three days on from it at DS:0x1156.  That word is the day the
		// illness may take a sietch, and 0x1E46 will not strike before it.
		// Leaving it at the 0xFFFF it starts life as meant the day never came,
		// so no sietch ever fell ill and the story stopped at ninety-two with
		// nothing able to move it: no line of the dialogue advances from there.
		WRITE_LE_UINT16(_state.begin() + 0x1156, (uint16)((hour() >> 4) + 3));
		_state[0x1141]++;
		break;

	case 93: // asynchronous illness chapter; no separate table setup
		break;

	case 96: // 0x11CB - put Chani in the Harkonnen palace's second room
		_state[0xff] = 0;
		_state[0x2a] = 96;

		// 0x40AE is handed the second place, at DS:0x11C, not Paul's palace
		// at 0x100. Its ordinary kind 0x30 gives room 0x3002/location 0x0280.
		// Room two and that place's location are kept in Chani's person record
		// at DS:0x1048. The separate 0x4072 ending gate still tests literal
		// room 0x3002; retain the current place kind here instead of hardcoding it.
		if (_state.size() > 0x104b) {
			const byte what = _state[0x11c + 8];
			WRITE_LE_UINT16(_state.begin() + 0x1048, (what << 8) | 2);
			WRITE_LE_UINT16(_state.begin() + 0x104a, (2 << 8) | 0x80);
		}
		break;

	case 100: { // 0x11E6, which is nothing but a jump to 0x1F13
		// Feyd takes Chani.  0x1F13 walks the places from the third onward
		// while the first byte of the record is under eight and keeps the one
		// lying furthest south - the greatest signed latitude at +4 - passing
		// over non-Harkonnen sites and Harkonnen places already marked at
		// +0x0A, which is 0x5D36,
		// and over any place one of Paul's own troops is already attacking,
		// which is what 0x5082 counts into DX for 0x5098.  0x1F44 falls back
		// on the palace if nothing else will serve.
		enum { kTroops = 0x8aa, kTroopSize = 27, kAttacking = 6 };

		uint32 held = 0;
		int furthestSouth = -100;

		for (uint i = 0; i != _map.places().size(); i++) {
			const uint32 place = 0x100 + 28 * i;

			if (place + 27 > _state.size())
				break;
			if (place < 0x138)
				continue;
			// 0x1F3A stops the walk at the first record under eight.
			if (_state[place] >= 8)
				break;

			const int lies = (int16)READ_LE_UINT16(_state.begin() + place + 4);

			if (lies <= furthestSouth)
				continue;
			// 5D36 returns carry for every kind below 0x28, and for a
			// Harkonnen place whose +0A bit three is already set.
			if (_state[place + 8] < 0x28 || (_state[place + 0xa] & 8) != 0)
				continue;

			bool besieged = false;
			// 5098 uses 6603's standing chain, not a global search for a
			// matching +4 place word. An incoming or unlinked attacker can name
			// this fort without preventing Feyd from choosing it.
			const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
			for (uint j = 0; j < standing.size() && !besieged; j++) {
				const uint32 record = kTroops + standing[j] * kTroopSize;
				if (record + kTroopSize > _state.size())
					break;
				if ((_state[record + 3] & 0x20) == 0 &&
						(_state[record + 0x10] & 0x80) == 0 &&
						_state[record + 3] == kAttacking)
					besieged = true;
			}

			if (besieged)
				continue;

			furthestSouth = lies;
			held = place;
		}

		if (held == 0)
			held = 0x100;

		// 0x1F4C puts her in the third room of it. This updates Chani's person
		// record; it does not change 0x4072's literal throne-room ending gate.
		if (_state.size() > 0x104b) {
			const uint which = (held - 0x100) / 28 + 1;

			WRITE_LE_UINT16(_state.begin() + 0x1048,
							(uint16)((_state[held + 8] << 8) | 3));
			WRITE_LE_UINT16(_state.begin() + 0x104a, (uint16)((which << 8) | 0x80));
		}

		if (_state.size() > 0xf3)
			WRITE_LE_UINT16(_state.begin() + 0xf2,
							(uint16)((_state[held] << 8) | _state[held + 1]));

		debugC(1, kDebugScript, "Feyd holds Chani at the place at %04x", (uint)held);

		// 0x1F5E asks for Feyd's own transmission, which is edition 0x2B from
		// the tenth person.
		queueEvent(0x2b0a);
		break;
	}

	case 24:
	case 36:
	case 60:
	case 104:
	case 108: // 0x10B7, which does nothing at all
	case 112: // the final dialogue round-up has no corresponding setup routine
		break;

	default:
		debugC(1, kDebugScript, "what stage %d sets up is not written yet", stage);
		break;
	}
}

bool DuneEngine::conditionHolds(uint index) const {
	// A record that names no condition is always reached.
	if (index == 0xffff)
		return true;

	// A396 tests the actual place kind at DS:8. Room one can be a sietch
	// entrance, not open desert: substituting FF for Gurney makes his STAY
	// request refuse there (PHRASE11:379) instead of accepting (376).
	return _condit.evaluate(index, _state.begin(), _state.size());
}

Common::String DuneEngine::phrase(uint line) {
	// 0xCF71 takes one off the number before it looks a line up, so a line is
	// counted from one.  Reading it from nought showed everyone the line after
	// the one they were saying.
	if (line == 0)
		return Common::String();

	return phraseByIndex(line - 1);
}

/**
 * A line of one of the dialogue's own groups.  There are two files of lines to
 * a language, and which of them a line belongs to is decided by who is saying
 * it rather than by its number: the dialogue's seventeen groups number their
 * lines 1 to 479 over the first six and then start again at 1 and run to 470
 * over the eleven after them, and 479 and 470 are exactly how many lines the
 * two files hold.
 *
 * So Duke Leto, Jessica, Thufir Hawat, Duncan Idaho, Gurney Halleck and
 * Stilgar read out of PHRASE?1, and Kynes, Chani, Harah, the Baron,
 * Feyd-Rautha, the Emperor, the Harkonnen Captain, the Smuggler, the Fremen,
 * the Fremen Chief and the group of lines that belong to nobody read out of
 * PHRASE?2.
 *
 * Reading them all out of the first file and running into the second only
 * once its lines were used up put another person's words in the mouth of
 * everybody from Kynes on: a Fremen asked to work for Paul answered "Your
 * mother was searching for you", which is PHRASE11's, where his own answer -
 * "Well, you're talking to the right person! My troop specializes in spice
 * prospecting" - stands at the same number in PHRASE12.
 */
Common::String DuneEngine::phraseOf(uint group, uint line) {
	enum { kFirstOfTheSecondFile = 6 };

	if (line == 0)
		return Common::String();

	return phraseByIndex(line - 1, group >= kFirstOfTheSecondFile);
}

Common::String DuneEngine::phraseByIndex(uint index, bool second) {
	// Each file opens with an index of its own: offsets to each line, the
	// first of them saying how many there are, and a line ends at 0xFF.
	Resource file = _archive.openMember(second ? "PHRASE12.HSQ" : "PHRASE11.HSQ");
	if (!file)
		return Common::String();

	uint16 count = file->readUint16LE() / 2;

	// The lines the game says of its own accord are numbered past the end of
	// the first file and run on into the second, which is what this used to
	// do for everybody.
	if (!second && index >= count) {
		index -= count;

		file = _archive.openMember("PHRASE12.HSQ");
		if (!file)
			return Common::String();

		count = file->readUint16LE() / 2;
	}

	if (index >= count)
		return Common::String();

	file->seek(2 * index);
	file->seek(file->readUint16LE());

	// A line carries marks of its own and one of them is a nought, so the
	// bytes are gathered and made into a string in one go: adding a nought to
	// a string a character at a time is complained about.
	Common::Array<char> bytes;
	for (byte c = file->readByte(); c != 0xff && !file->eos(); c = file->readByte())
		bytes.push_back((char)c);

	return Common::String(bytes.begin(), bytes.size());
}

/**
 * The name of a place.  COMMAND1.HSQ keeps the twelve regions from its first
 * word on and the nine names a place may carry within one from its fifteenth,
 * and a place's own two bytes pick one of each: PHRASE11's fifty-fourth line
 * carries the marks for them with a dash between, which is how "Carthag-Tuek"
 * is written.
 */
/**
 * What a place is called.  0x62A6 puts two of COMMAND1.HSQ's words together:
 * the byte at the place's +0 names one of the twelve regions, counting from
 * one, and the byte at its +1 names what follows at twelve past that - the two
 * words `(Atreides)` and `(Harkonnen)` for the two seats and then the nine
 * names a sietch may carry.  A space joins them where that second byte is
 * under three, which is what puts the two seats in brackets after their
 * region, and a hyphen everywhere else.
 *
 * Both halves used to be read a word out: the region was taken from the byte
 * less one and the name from fourteen past it rather than twelve, so the
 * sietch Gurney Halleck is sent to came out " Arrakeen-Harg" where the game
 * itself calls it "Carthag-Tuek".
 */
Common::String DuneEngine::nameOfPlace(const Place &place) {
	enum { kFirstRegion = 1, kFirstName = 12, kRegions = 12, kLastName = 11, kJoined = 3 };

	Common::String name;

	if (place.type >= kFirstRegion && place.type < kFirstRegion + kRegions)
		name = command(place.type - 1);

	if (place.id > 0 && place.id <= kLastName) {
		if (!name.empty())
			name += place.id < kJoined ? ' ' : '-';

		name += command(kFirstName + place.id - 1);
	}

	return name;
}

/**
 * What kind of place it is, which 0x629D writes before the name: the four
 * words `Sietch: `, `Palace: `, `Village: ` and `Fort: `.  0x6231 sorts the
 * place by the byte at its +8. Unlike the scenery sprite class (5E4F), the
 * name class maps kinds >= 0x30 back to Palace, not Village.
 */
Common::String DuneEngine::kindOfPlaceName(byte kindOfPlace) {
	enum { kFirstKind = 68 };

	const int which = kindOfPlace < 0x20 ? 0 : kindOfPlace < 0x21 ? 1
		: kindOfPlace < 0x28 ? 2 : kindOfPlace < 0x30 ? 3 : 1;

	return command(kFirstKind + which - 1);
}

/**
 * Fills the marks of a line in.  What the phrase files hold is not plain text:
 *
 *   0x0D          break the line here
 *   0xFE          break it here as well, which is how one line carries two
 *                 sentences
 *   0x80 hi lo    a word of COMMAND1.HSQ named outright, counted from one, so
 *                 that a line can say a place by name - the fifty-fourth line
 *                 of PHRASE11 sends Gurney to "Carthag-Tuek" that way
 *   0x81, 0x82    the region and the name of the place the line is about
 *   0x83 to 0x8C  contextual words, including 0x8A's DS:11FF lookup
 *   0x91 nn       a number of the game's own, and 0x92 nn one with a letter
 *
 * and 0x06 and 0x08 change the face, which the font itself knows about.
 */
Common::String DuneEngine::fillInLine(const Common::String &raw, uint16 reportTroop) {
	return expandDialogueText(raw, reportTroop, 0);
}

Common::String DuneEngine::expandDialogueWord(uint16 which, uint16 reportTroop, uint depth) {
	if (!which || depth >= 12)
		return Common::String();
	Common::String word = command(which - 1);
	if (which == 115 && _state.size() > 0x43) {
		// 32C7 finds the first number in the cached duration word via D03C,
		// then E2E3 overwrites the preceding three character cells. Preserve
		// its fixed-width spaces and 999 cap, not the asset's initial "12".
		uint at = 0;
		while (at < word.size() && (word[at] < '0' || word[at] > '9'))
			at++;
		if (at < word.size()) {
			while (at < word.size() && word[at] >= '0' && word[at] <= '9')
				at++;
			if (at >= 3)
				word.replace(at - 3, 3, Common::String::format("%3u",
					MIN<uint>(READ_LE_UINT16(_state.begin() + 0x42) >> 4, 999)));
		}
	}
	// 8944 resumes the parent after each COMMAND word, expanding any
	// contextual or numeric controls inside that word too.
	return expandDialogueText(word, reportTroop, depth + 1);
}

Common::String DuneEngine::expandDialogueText(const Common::String &raw, uint16 reportTroop, uint depth) {
	Common::String out;
	if (depth >= 12)
		return out;

	uint from = 0;
	// 894B skips literal leading blanks on entry to each phrase page.
	// Nested COMMAND expansions resume at8953, preserving their padding.
	if (!depth) {
		while (from < raw.size() && raw[from] == ' ')
			from++;
	}
	for (uint i = from; i != raw.size(); i++) {
		const byte c = (byte)raw[i];

		switch (c) {
		case 0x0d:
			out += '\r';
			break;

		// The rest of the line waits for a press: 0xFE is kept as a mark of
		// its own and sayLineInParts breaks the line there.
		case 0xfe:
			out += '\f';
			if (!depth) {
				while (i + 1 < raw.size() && raw[i + 1] == ' ')
					i++;
			}
			break;

		case 0x80:
			if (i + 2 < raw.size()) {
				const uint16 which = (uint16)(((byte)raw[i + 1] << 8) | (byte)raw[i + 2]);
				out += expandDialogueWord(which, reportTroop, depth);
				i += 2;
			}
			break;

		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f: {
			// 8979/8980 uses the prepared, one-based COMMAND words. In
			// particular 81/82 retain a remote name supplied by 2E98, and
			// 84 differs between speech, compact cards and flight;
			// 8C names the requested tool, not a guess from the current job.
			// Do not synthesize/clamp these again from troop data on redraw.
			const uint address = 0x11eb + 2 * (c & 0xf);
			if (address + 1 < _state.size()) {
				const uint16 which = READ_LE_UINT16(_state.begin() + address);
				out += expandDialogueWord(which, reportTroop, depth);
			}
			break;
		}

		default:
			if (c == 0x91 && i + 1 < raw.size()) {
				const byte address = (byte)raw[++i];
				if (address < _state.size())
					out += Common::String::format("%u", (uint)_state[address]);
				break;
			}
			if (c == 0x92 && i + 1 < raw.size()) {
				const byte address = (byte)raw[++i];
				if ((uint)address + 1 < _state.size())
					out += Common::String::format("%u",
										 (uint)READ_LE_UINT16(_state.begin() + address));
				break;
			}
			if (c < 0x20 && c != Font::kSwitchToAlternate && c != Font::kSwitchToNormal)
				break;

			out += (char)c;
			break;
		}
	}

	return out;
}

Common::String DuneEngine::command(uint index) {
	// The command file is laid out like the phrase files: an index of offsets,
	// the first of them giving the count, and lines ended by 0xFF.  It holds
	// the names of the regions and the sietches and of every order that can be
	// given.
	Resource file = _archive.openMember("COMMAND1.HSQ");
	if (!file)
		return Common::String();

	const uint16 count = file->readUint16LE() / 2;
	if (index >= count)
		return Common::String();

	file->seek(2 * index);
	file->seek(file->readUint16LE());

	Common::String line;
	for (byte c = file->readByte(); c != 0xff && !file->eos(); c = file->readByte())
		line += (char)c;

	// The words are written as they stand.  Trimming them took the space off
	// the end of `Sietch: `, so a place read "Sietch:Carthag-Tuek"; took the
	// one off the front of ` Arrakeen`; and took away the two that indent
	// `  Others...`, `  Cancel` and `  Done` in a list.
	return line;
}

void DuneEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_music)
		_music->syncVolume();
}

bool DuneEngine::runIntro() {
	// DOS begins the intro with 227D set. 029D clears it for gameplay;
	// loadStaticData has already cleared it here, so retain it for this call.
	const byte previousOverlay = _state[0x227d];
	_state[0x227d] = 1;
	_introPage.resize(kScreenWidth * kScreenHeight);
	memset(_introPage.begin(), 0, _introPage.size());
	Intro intro(this);
	const bool finished = intro.run();
	_introPage.clear();
	_introYOffset = 0;
	_introSpeakerX = 0;
	_state[0x227d] = previousOverlay;
	return finished;
}

bool DuneEngine::isCD() {
	return _gameDescription->flags & ADGF_CD;
}

void DuneEngine::loadBank(uint16 index) {
	if (index == _currentBankId) {
		return;
	}
}

void DuneEngine::applyPalette(int index, int count, const byte *sixBit) {
	byte buf[3 * 256];

	if (index < 0 || count <= 0 || index + count > 256)
		return;

	for (int i = 0; i != 3 * count; i++)
		buf[i] = (sixBit[i] * 255 + 31) / 63;

	_system->getPaletteManager()->setPalette(buf, index, count);
}

/**
 * Which of the palettes the two banks of the sky keep belongs to the hour.
 * 0x395C takes the low nibble of the hour at DS:0x2 through the table at
 * DS:0x2280 and adds bits 2 to 4 of the hour shifted down by two, so the sky
 * comes round with the day and drifts as the days pass.
 */
int DuneEngine::skyPaletteForHour() const {
	return skyPaletteForHour(_state.size() > 2 ? _state[2] : 0);
}

int DuneEngine::skyPaletteForHour(uint16 hour) const {
	return _staticState[0x2280 + (hour & 0xf)] + ((hour >> 2) & 0x1c);
}

/**
 * Lays the hour's sky palette down.  0x398C sets 151 colours from the
 * seventy-third for the wide one and 80 from the hundred and twenty-eighth
 * otherwise, and then the panel's own sixteen from the two hundred and
 * fortieth, all out of one run that begins six bytes into the entry.
 */
bool DuneEngine::readSkyPalette(bool wide, int selected, byte *run, bool &panel) {
	enum { kSkip = 6, kPanelColours = 16 };
	const int colours = wide ? 151 : 80;
	openSpriteSheet(wide ? "SKYDN.HSQ" : "SKY.HSQ", false);
	const uint32 wanted = 3 * (colours + kPanelColours);
	panel = currentSpriteSheet->read(selected, kSkip, run, wanted);
	if (!panel) {
		// The last entry of SKY.HSQ is three bytes short of the whole run; the
		// original reads on past it and takes whatever follows, so the panel's
		// own colours are left as they are rather than made up here.
		return currentSpriteSheet->read(selected, kSkip, run, 3 * colours);
	}
	return true;
}

/** 3971/39B9: update only the target ranges which the original loads. */
bool DuneEngine::loadSkyFadeTarget(bool wide, int selected) {
	byte run[3 * (151 + 16)];
	bool panel;
	if (!readSkyPalette(wide, selected, run, panel))
		return false;
	const int first = wide ? 73 : 128, colours = wide ? 151 : 80;
	_state[0x46d6] = selected;
	memcpy(_skyFadeTarget + 3 * first, run, 3 * colours);
	if (panel && !_state[0x227d])
		memcpy(_skyFadeTarget + 720, run + 3 * colours, 48);
	return true;
}

/** 388D: a repaint retains an active fade; only a new target resets it to48. */
void DuneEngine::applySkyPalette(bool wide) {
	_state[0x22e3] = wide;
	_state[0x46df] = 1;
	const int selected = skyPaletteForHour();
	if (_skyFadeActive && _state[0x46d7]) {
		if (_state[0x46d6] != selected && loadSkyFadeTarget(wide, selected))
			_state[0x46d7] = 48;
		return;
	}
	byte run[3 * (151 + 16)];
	bool panel;
	if (!readSkyPalette(wide, selected, run, panel))
		return;
	stopSkyFade();
	_state[0x46d6] = selected;
	const int first = wide ? 73 : 128, colours = wide ? 151 : 80;
	applyPalette(first, colours, run);
	if (panel && !_state[0x227d])
		applyPalette(240, 16, run + 3 * colours);
}

/** 3805/017B: a landscape restart retains the displayed palette as target. */
void DuneEngine::restartFlightSkyPalette() {
	byte palette[768];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	for (uint i = 0; i < 768; i++)
		_skyFadeTarget[i] = (palette[i] * 63 + 127) / 255;
	// 388D may load a changed selection with divisor48, but a same-sky
	// restart keeps the existing divisor and registered sixteen-tick phase.
	applySkyPalette(true);
}

/** 38E1: the hour service retargets without losing a registered timer phase. */
void DuneEngine::retargetSkyFade() {
	if (!_state[0x46df] || _state[0x46d6] == skyPaletteForHour())
		return;
	startSkyFade(skyPaletteForHour());
}

/** 38F1 also accepts narration's explicit palette, independent of the hour. */
void DuneEngine::startSkyFade(int selected) {
	if (!loadSkyFadeTarget(_state[0x22e3] != 0, selected))
		return;
	_state[0x46d7] = 64;
	if (!_skyFadeActive) {
		_skyFadeActive = true;
		_skyFadeRemainder = 0;
		_skyFadeTime = animationClockTicks();
	}
}

void DuneEngine::stopSkyFade() {
	_skyFadeActive = false;
	_skyFadeRemainder = 0;
	_state[0x46d7] = 0;
}

/** 0FB2 enters395F with AX's future hour, without changing the actual clock. */
void DuneEngine::prepareWaitSkyPalette(uint16 targetHour) {
	const bool wide = _state[0x22e3] != 0;
	const int selected = skyPaletteForHour(targetHour);
	if (_skyFadeActive && _state[0x46d7]) {
		loadSkyFadeTarget(wide, selected);
		return;
	}
	byte run[3 * (151 + 16)];
	bool panel;
	if (!readSkyPalette(wide, selected, run, panel))
		return;
	_state[0x46d6] = selected;
	applyPalette(wide ? 73 : 128, wide ? 151 : 80, run);
	if (panel && !_state[0x227d])
		applyPalette(240, 16, run + 3 * (wide ? 151 : 80));
}

/** 390A: scripted waits/Chani's first pose drain3916 without adding clock ticks. */
void DuneEngine::finishSkyFade() {
	while (_skyFadeActive && _state[0x46d7]) {
		if (!_state[0x46df]) {
			stopSkyFade();
			break;
		}
		stepSkyFade();
		// 390A calls3916/driver0175 synchronously. Each step writes the
		// DAC even though no scene rectangle is copied. Like updateSkyFade,
		// commit SDL's pending colours before the following hour's work.
		_system->updateScreen();
	}
}

/** 391D/driver175: signed IDIV of the displayed six-bit channels, then DEC. */
void DuneEngine::stepSkyFade() {
	if (!_state[0x46d7])
		return;
	byte palette[768], current[768];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	for (uint i = 0; i < 768; i++)
		current[i] = (palette[i] * 63 + 127) / 255;
	const uint first = _state[0x22e3] ? 73 : 128;
	const uint count = _state[0x22e3] ? 151 : 80;
	for (uint i = 3 * first; i < 3 * (first + count); i++)
		current[i] += ((int)_skyFadeTarget[i] - current[i]) / _state[0x46d7];
	applyPalette(first, count, current + 3 * first);
	if (!_state[0x227d]) {
		for (uint i = 720; i < 768; i++)
			current[i] += ((int)_skyFadeTarget[i] - current[i]) / _state[0x46d7];
		applyPalette(240, 16, current + 720);
	}
	if (!--_state[0x46d7])
		stopSkyFade();
}

/** D9D2 invokes at most one3916 per due poll, even after a long host stall. */
bool DuneEngine::pollSkyFade(uint16 ticks) {
	if (!_skyFadeActive)
		return false;
	const uint16 elapsed = ticks + _skyFadeRemainder;
	_skyFadeRemainder = elapsed % 16;
	if (elapsed < 16)
		return false;
	if (!_state[0x46df]) {
		stopSkyFade();
		return false;
	}
	stepSkyFade();
	return true;
}

void DuneEngine::updateSkyFade() {
	const uint32 now = animationClockTicks();
	const uint16 ticks = animationElapsedTicks(_skyFadeTime, now);
	_skyFadeTime = now;
	if (pollSkyFade(ticks))
		_system->updateScreen(); // A DAC change is visible even without a page repaint.
}

int DuneEngine::placeClass(byte kindOfPlace) {
	int which = 0;

	if (kindOfPlace >= 0x20)
		which++;
	if (kindOfPlace >= 0x21)
		which++;
	if (kindOfPlace >= 0x28)
		which++;
	if (kindOfPlace >= 0x30)
		which++;

	return which;
}

/**
 * The pictures a place may be seen from outside as.  0x5E4F sorts a place
 * into one of five kinds by the byte at its +8 - a sietch under 0x20, the
 * palace at 0x20, a village under 0x28, a fort under 0x30 and a Harkonnen
 * place above it. DS:0x1972 keeps the first picture of each and how many
 * nearby views there are. Village base 0x7F is special: 3829 replaces it
 * with VIL1..VIL6 according to the place's first byte shifted right once.
 */
const char *DuneEngine::outsidePicture(byte kindOfPlace, int view, byte placeType) const {
	static const char *const kSietch[]  = { "DS0.HSQ", "DS1.HSQ", "DS2.HSQ", "DS3.HSQ", "DS4.HSQ" };
	static const char *const kPalace[]  = { "DP1.HSQ", "DP0.HSQ", "DP2.HSQ", "DP3.HSQ" };
	static const char *const kVillage[] = { nullptr, "DV1.HSQ", "DV2.HSQ", "DV3.HSQ", "DV4.HSQ" };
	static const char *const kVillageBase[] = {
		"VIL1.HSQ", "VIL2.HSQ", "VIL3.HSQ", "VIL4.HSQ", "VIL5.HSQ", "VIL6.HSQ"
	};
	static const char *const kFort[]    = { "DF1.HSQ", "DF2.HSQ", "DF3.HSQ", "DF4.HSQ" };
	static const char *const kHarkonnen[] = { "DH0.HSQ", "DH1.HSQ", "DH2.HSQ", "DH3.HSQ" };

	static const struct { const char *const *names; int count; } kOfKind[5] = {
		{ kSietch, ARRAYSIZE(kSietch) }, { kPalace, ARRAYSIZE(kPalace) },
		{ kVillage, ARRAYSIZE(kVillage) }, { kFort, ARRAYSIZE(kFort) },
		{ kHarkonnen, ARRAYSIZE(kHarkonnen) }
	};

	const int which = placeClass(kindOfPlace);

	// 3834 rejects a nearby view outside this place class's picture range.
	if (view < 0 || view >= kOfKind[which].count)
		return nullptr;
	if (which == 2 && view == 0) {
		const uint village = placeType >> 1;
		return village < ARRAYSIZE(kVillageBase) ? kVillageBase[village] : nullptr;
	}

	return kOfKind[which].names[view];
}

/**
 * The sky itself: 0x38BD draws the first four entries of SKY.HSQ, each of
 * them forty across, eight times over to fill the width of the screen, one
 * band under another twenty pixels apart.
 */
void DuneEngine::drawSky() {
	enum { kBands = 4, kBandWidth = 40, kBandHeight = 20 };

	openSpriteSheet("SKY.HSQ", false);

	for (int band = 0; band != kBands; band++) {
		for (int x = 0; x < kScreenWidth; x += kBandWidth)
			drawSprite(band, x, band * kBandHeight);
	}
}

/**
 * The map's own colours, which the sky's palette does not reach.  The map
 * screen wants everything ONMAP.HSQ sets below the eightieth; the little map
 * watched from the air is drawn out of the deeper ramp of sixteen that begins
 * there, so it wants those as well.
 */
void DuneEngine::applyMapColours(int below) {
	Resource onmap = _archive.openMember("ONMAP.HSQ");
	if (!onmap)
		return;

	// Past the word that says where the sprites begin.
	onmap->seek(2);
	applyPaletteBlock(&*onmap, below);
}

void DuneEngine::applyPaletteBlock(Common::ReadStream *reader, int onlyBelow) {
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

		if (index < onlyBelow)
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

/** Retain the movie's displayed page for the next scene, overlay or wipe. */
void DuneEngine::presentMovieFrame() {
	memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kScreenHeight);
	// The decoder keeps its own delta history. Normal drawing must not use
	// that page while a staged movie is paused or after playback returns.
	_graphics.setFrameBuffer(_framebuffer1);
	copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * Plays a movie only as far as a given frame, leaving it open so that a later
 * step can carry it on.  The title is run this way: the step at 0x0397 loads
 * and decodes its initial frame, the one at 0x03A3 runs it to its eighty-sixth
 * frame - which is what 0x06B5 waits for, comparing the frame counter at
 * DS:0xDBE8 with 0x56 - and the one at 0x03AF plays out the rest.  Between
 * those the title stands on the screen, which is most of the half minute it
 * is up for.
 */
PlaybackResult DuneEngine::playVideoUntil(uint16 videoId, int stopAt) {
	if (_openMovie != videoId) {
		const char *filename = getResourceFilenameByIndex(videoId + INDEX_FIRST_HNM);
		Resource r = _archive.openMember(filename);
		if (!r) {
			warning("Dune: the movie %s is missing", filename);
			return kPlaybackFinished;
		}

		debugC(1, kDebugVideo, "opening %s", filename);

		_video->setReader(r);
		_video->setInterlace(videoId >= HNM_IRULAN);
		_video->start();
		_openMovie = videoId;
		// CA1B prepares and copies one frame before incrementing DBE8.
		// A zero limit stages that frame for the caller's palette transition.
		if (!_video->done()) {
			_video->decodeAVFrame();
			if (stopAt == 0) {
				memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kScreenHeight);
				_graphics.setFrameBuffer(_framebuffer1);
			} else {
				presentMovieFrame();
			}
		}
	}

	const uint32 frameTime = 1000 / 12;
	uint32 nextFrameTime = _system->getMillis() + frameTime;

	while (!_video->done() && !shouldQuit()) {
		if (stopAt >= 0 && _video->_currentFrame >= stopAt)
			break;

		_video->decodeAVFrame();

		presentMovieFrame();

		const PlaybackResult asked = pollSkip();
		if (asked != kPlaybackFinished) {
			_video->stopAudio();
			_openMovie = 0xffff;
			return asked;
		}

		if (!ConfMan.getBool("dune_nodelay")) {
			const uint32 now = _system->getMillis();
			if (now < nextFrameTime)
				_system->delayMillis(nextFrameTime - now);
		}

		nextFrameTime += frameTime;
	}

	if (_video->done()) {
		_video->stopAudio();
		_openMovie = 0xffff;
	}

	// Loading a staged movie without playing any frames also leaves the
	// displayed engine page as the drawing destination.
	_graphics.setFrameBuffer(_framebuffer1);
	return shouldQuit() ? kPlaybackAborted : kPlaybackFinished;
}

/**
 * Runs a movie named by the resource it is.  The looping ones - MNT1 to MNT4,
 * SIET and PALACE, each four whole pictures of 320 by 152 - are what the
 * desert is drawn out of, and they are not in the list of movies the intro
 * names, so they cannot be reached by the number that list gives.
 */
PlaybackResult DuneEngine::playMovieResource(uint16 resource, bool loop) {
	const char *filename = getResourceFilenameByIndex(resource);
	if (!filename)
		return kPlaybackFinished;

	Resource r = _archive.openMember(filename);
	if (!r) {
		warning("Dune: the movie %s is missing", filename);
		return kPlaybackFinished;
	}

	debugC(1, kDebugVideo, "%s %s", loop ? "looping" : "playing", filename);

	_openMovie = 0xffff;
	_video->setReader(r);
	_video->setInterlace(false);
	_video->start();

	uint32 frameTime = 1000 / 10;
	uint32 nextFrameTime = _system->getMillis() + frameTime;

	while (!shouldQuit()) {
		if (_video->done()) {
			if (!loop)
				break;

			// A loop is begun again from its first frame.
			_video->setReader(r);
			_video->start();
		}

		_video->decodeAVFrame();

		presentMovieFrame();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-%04d.ppm", ConfMan.get("dune_dump").c_str(),
												 _video->_currentFrame), _video->_frameBuffer);

		const PlaybackResult asked = pollSkip();
		if (asked != kPlaybackFinished)
			return asked;

		if (ConfMan.getBool("dune_nodelay"))
			continue;

		const uint32 now = _system->getMillis();
		if (now < nextFrameTime)
			_system->delayMillis(nextFrameTime - now);
		nextFrameTime = _system->getMillis() + frameTime;
	}

	return kPlaybackFinished;
}

void DuneEngine::drawMovieFrame(uint16 resource, int frame) {
	const char *filename = getResourceFilenameByIndex(resource);
	if (!filename)
		return;

	Resource r = _archive.openMember(filename);
	if (!r)
		return;

	_openMovie = 0xffff;
	_video->setReader(r);
	_video->setInterlace(false);
	_video->start();

	// The frames of a loop are whole pictures, so the one wanted is reached by
	// running the ones before it.
	for (int i = 0; i <= frame && !_video->done(); i++)
		_video->decodeAVFrame();

	// A loop is 320 by 152, which is the picture and not the panel.
	memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * 152);

	// Setting a movie going points the drawing at the movie's own page; what
	// is drawn over the desert belongs on the page it was copied to.
	_graphics.setFrameBuffer(_framebuffer1);
}

PlaybackResult DuneEngine::playVideo(uint16 videoId, uint32 holdMs) {
	_openMovie = 0xffff;

	const char *filename = getResourceFilenameByIndex(videoId + INDEX_FIRST_HNM);
	Resource r = _archive.openMember(filename);
	if (!r) {
		warning("Dune: the movie %s is missing", filename);
		return kPlaybackFinished;
	}

	debugC(1, kDebugVideo, "playing %s", filename);
	_video->setReader(r);

	if (videoId == HNM_IRULAN) {
		_video->setSubtitles(_staticState.begin() + 0x35a8, _archive.openMember("IRUL1.HSQ"));
	}

	// The movies of the book are stored at half height and drawn onto every
	// other line of the screen.  Which ones those are is still read off the
	// order they come in, there being nothing found yet that says so outright.
	_video->setInterlace(videoId >= HNM_IRULAN);
	_video->start();

	// A movie that carries sound is paced by it: Irulan's slices are nine
	// hundred and twenty bytes at eleven thousand a second, which is twelve
	// frames a second and a little.  One that carries none is not held to
	// anything, and set beside a recording of the game they run slower than
	// that - the Virgin logo's hundred and eighty-nine frames take nineteen
	// and a half seconds, the Cryo logo's hundred and one take eleven, MTG1's
	// three hundred and thirty-five take four and thirty - which is ten a
	// second.
	// CREDITS.HNM is the silent 3,324-frame closing crawl. In the supplied
	// playthrough it starts around 391:19 and gives way to the final still
	// about 111 seconds later: thirty frames a second, not the ten used by
	// the opening's silent movies. At ten it would linger for five minutes.
	const bool credits = videoId == HNM_CREDITS;
	uint32 frameTime = credits ? 1000 / 30 : 1000 / 10;
	const uint32 startedAt = _system->getMillis();
	uint32 nextFrameTime = startedAt + frameTime;

	while (!_video->done() && !shouldQuit()) {
		_video->decodeAVFrame();

		if (_video->hasAudio())
			frameTime = 1000 / 12;

		presentMovieFrame();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-%04d.ppm", ConfMan.get("dune_dump").c_str(),
												 _video->_currentFrame), _video->_frameBuffer);

		const PlaybackResult asked = pollSkip();
		if (asked != kPlaybackFinished) {
			// Cutting a movie short cuts its sound short with it.
			_video->stopAudio();
			return asked;
		}

		if (ConfMan.getBool("dune_nodelay"))
			continue;

		const uint32 now = _system->getMillis();
		if (now < nextFrameTime)
			_system->delayMillis(nextFrameTime - now);

		// Thirty frames do not fit into whole milliseconds: deriving each
		// deadline from the frame count retains the extra third of a millisecond
		// instead of ending the 3,324-frame crawl over a second early.
		if (credits && !_video->hasAudio())
			nextFrameTime = startedAt + ((uint32)_video->_currentFrame + 1) * 1000 / 30;
		else
			nextFrameTime += frameTime;
	}

	_video->stopAudio();

	if (shouldQuit())
		return kPlaybackAborted;

	// A movie of a handful of frames, the Cryo logo among them, is over in a
	// blink; the original holds the last of them while the music plays on.
	return hold(holdMs);
}

// Sits on the last frame, watching for the player to grow impatient.
PlaybackResult DuneEngine::hold(uint32 ms) {
	const uint32 until = _system->getMillis() + ms;

	while (_system->getMillis() < until && !shouldQuit()) {
		const PlaybackResult asked = pollSkip();
		if (asked != kPlaybackFinished) {
			// Cutting a movie short cuts its sound short with it.
			_video->stopAudio();
			return asked;
		}

		if (ConfMan.getBool("dune_nodelay"))
			break;

		_system->delayMillis(10);
		_system->updateScreen();
	}

	return shouldQuit() ? kPlaybackAborted : kPlaybackFinished;
}

void DuneEngine::resetPointerButtons() {
	_pointerClickKeys = 0;
	_mapPointerKeys = 0;
	_mapPointerGlide.steps = 0;
	_mapPointerPendingButtons = 0;
	_mapPointerDistance = _mapPointerFraction = Common::Point();
	_mapPointerControl = false;
	_heldDesertKeyWay = -1;
	_nextDesertWalk = 0;
	// Dialog/load teardown must not turn an already held physical button into
	// a new command on the next idle sample. This is hardware, not save data.
	_pointerButtons.reset((_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON) != 0);
}

/** Keep button samples visible even when a blocking wait consumes them. */
byte DuneEngine::updatePointerClickKeys(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
		_mapPointerControl = (event.kbd.flags & Common::KBD_CTRL) != 0;
		const int direction = mapPointerKey(event.kbd.keycode);
		if (event.type == Common::EVENT_KEYUP && direction >= 0)
			_mapPointerKeys &= ~(1 << direction);
	}
	// Playback and remote reports can consume the release before runGame
	// sees it. Stop walking here as well as on ordinary input-loop releases.
	if (event.type == Common::EVENT_KEYUP && _heldDesertKeyWay >= 0) {
		const Common::KeyCode released[] = {
			Common::KEYCODE_UP, Common::KEYCODE_RIGHT,
			Common::KEYCODE_DOWN, Common::KEYCODE_LEFT
		};
		if (event.kbd.keycode == released[_heldDesertKeyWay]) {
			_heldDesertKeyWay = -1;
			_nextDesertWalk = 0;
		}
	}
	if (event.type == Common::EVENT_FOCUS_LOST || event.type == Common::EVENT_MAINMENU) {
		resetPointerButtons();
		return 0;
	}
	const bool mouse = event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP;
	if (!mouse && event.type != Common::EVENT_KEYDOWN && event.type != Common::EVENT_KEYUP)
		return 0;
	const byte key = mouse ? 0 : event.kbd.keycode == Common::KEYCODE_SPACE ? kPointerClickSpace :
		event.kbd.keycode == Common::KEYCODE_RETURN ? kPointerClickEnter :
		event.kbd.keycode == Common::KEYCODE_KP_ENTER ? kPointerClickKeypadEnter : 0;
	if (!key && !mouse)
		return 0;
	if (event.type == Common::EVENT_KEYDOWN) {
		if (!event.kbdRepeat)
			_pointerClickKeys |= key;
	} else {
		_pointerClickKeys &= ~key;
	}
	_pointerButtons.sample(_pointerClickKeys != 0,
			(_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON) != 0);
	return mouse ? 8 : key;
}

/** Sample pointer edges once per source event, then on otherwise idle frames. */
bool DuneEngine::pollPointerEvent(Common::Event &event, bool &idleSample) {
	while (_system->getEventManager()->pollEvent(event)) {
		const byte source = updatePointerClickKeys(event);
		// D893 belongs after the ordinary input sample, not before 1B0D's
		// notice/vision dispatch. Held samples reset it even without an edge.
		// Do not reset again while draining repeats after a command such as
		// WAIT: DOS returns to 1B0D before sampling the held input again.
		// DF1E's logical left includes click keys and their release mask;
		// physical middle and unheld pointer motion do not reset the timer.
		if (idleSample && (_pointerButtons.down() ||
				(_system->getEventManager()->getButtonState() & Common::EventManager::RBUTTON)))
			resetTroopNoticeIdle();
		if (source) {
			idleSample = false;
			if (!(_pointerButtons.buttons() & 4))
				continue;
			if (source != 8)
				event.mouse = _system->getEventManager()->getMousePos();
			event.type = _pointerButtons.down() ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
		}
		return true;
	}
	if (_mapPointerPendingButtons && !_mapPointerGlide.steps) {
		// D854 resumes ordinary button sampling after D962 finishes. A
		// press that stayed down can therefore act at the arrived cursor.
		const byte pending = _mapPointerPendingButtons;
		_mapPointerPendingButtons = 0;
		const bool left = _pointerButtons.down();
		const bool right = (_system->getEventManager()->getButtonState() & Common::EventManager::RBUTTON) != 0;
		if ((left && (pending & Common::EventManager::LBUTTON)) ||
				(!left && right && (pending & Common::EventManager::RBUTTON))) {
			idleSample = false;
			event = Common::Event();
			event.type = left ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN;
			event.mouse = _system->getEventManager()->getMousePos();
			return true;
		}
	}
	if (!idleSample)
		return false;
	idleSample = false;
	// Releasing the last keyboard source masks physical left for one sample.
	// A still-held mouse resumes on the next sample, without a new host event.
	const byte buttons = _pointerButtons.sample(_pointerClickKeys != 0,
			(_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON) != 0);
	if (_pointerButtons.down() ||
			(_system->getEventManager()->getButtonState() & Common::EventManager::RBUTTON))
		resetTroopNoticeIdle();
	if (!(buttons & 4))
		return false;
	event.type = _pointerButtons.down() ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
	event.mouse = _system->getEventManager()->getMousePos();
	return true;
}

// Escape and the left button give up on the whole of what is playing and go
// to the game; anything else asks only for the next part of it.
PlaybackResult DuneEngine::pollSkip() {
	Common::Event event;

	while (_system->getEventManager()->pollEvent(event)) {
		updatePointerClickKeys(event);
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			return event.kbd.keycode == Common::KEYCODE_ESCAPE ? kPlaybackAborted : kPlaybackSkipped;
		case Common::EVENT_LBUTTONDOWN:
			return kPlaybackAborted;
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			return kPlaybackSkipped;
		default:
			break;
		}
	}

	return shouldQuit() ? kPlaybackAborted : kPlaybackFinished;
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

void DuneEngine::openSpriteSheet(const char *filename, bool takeItsPalette) {
	Resource r = _archive.openMember(filename);

	debugC(2, kDebugResource, "opening sprite sheet %s", filename);

	delete currentSpriteSheet;
	currentSpriteSheet = new SpriteSheet(this, r, takeItsPalette);
}

void DuneEngine::drawSprite(int index, int dst_x, int dst_y, uint16 word, byte palOverride) {
	assert(currentSpriteSheet);

	currentSpriteSheet->draw(index, dst_x, dst_y, word, palOverride);
}

void DuneEngine::drawSpriteCentred(int index, int dst_x, int dst_y) {
	assert(currentSpriteSheet);

	int width = 0, height = 0;
	if (!currentSpriteSheet->size(index, &width, &height))
		return;

	currentSpriteSheet->draw(index, dst_x - width / 2, dst_y - height / 2, 0, 0);
}

namespace {

// Strings are kept as a length and then the bytes, so that a name that grows
// or shrinks does not move everything after it.
void writeSaveString(Common::WriteStream *stream, const Common::String &text) {
	stream->writeUint16LE(text.size());
	stream->writeString(text);
}

Common::String readSaveString(Common::SeekableReadStream *stream) {
	const uint16 length = stream->readUint16LE();

	Common::String text;
	for (uint16 i = 0; i != length; i++)
		text += (char)stream->readByte();

	return text;
}

// Engine::loadGameState passes the extended metadata/thumbnail too. A short
// retained page must not silently consume that footer as indexed pixels.
int64 duneSavePayloadEnd(Common::SeekableReadStream *stream) {
	const int64 here = stream->pos(), end = stream->size();
	int64 result = end;
	if (end >= 10) {
		stream->seek(end - 4);
		const uint32 header = stream->readUint32LE();
		if (header < end - 10) {
			char signature[6];
			stream->seek(header);
			if (stream->read(signature, sizeof(signature)) == sizeof(signature) &&
					!memcmp(signature, "SVMCR", sizeof(signature)))
				result = header;
		}
	}
	stream->seek(here);
	return result;
}

} // End of anonymous namespace

// The saved game keeps the whole of the game's own memory - the data segment
// the engine works out of, where the clock, the story, the troops, the places
// and the spice all live - and after it the few things the engine holds beside
// it: where the player stands, what they are looking at, what is being said
// and who is saying it, and the list the panel is showing.
enum {
	kSaveMark = MKTAG('D', 'U', 'N', 'E'),
	kSaveVersion = 69,
	kSkyFadeSaveSize = 777,
	kTroopPortraitPixels = 59 * 59
};

bool DuneEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher
		|| f == kSupportsLoadingDuringRuntime
		|| f == kSupportsSavingDuringRuntime;
}

bool DuneEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	// The synchronous MASSIVE ATTACK display has resolved combat, but has
	// not yet finished its shared-RNG waits or post-battle transition.
	// Defeat offers load/restart, never saving over a playable checkpoint.
	// The third route click is also a synchronous, uncommitted transition.
	// Saving during 9B49 would capture a temporary pose before the next line.
	return !_state.empty() && !_state[0x473a] && !_state[0x46d9] &&
		!_defeatPlaying && !_routePreviewActive && !_noticeWarningActive && !_noticeTransitionActive &&
		!_waterOfLifeActive && !_speakerReportLowering &&
		_menuKind != kMenuBook;
}

bool DuneEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return !_state.empty() && !_state[0x473a] && !_defeatPlaying &&
		!_routePreviewActive && !_noticeWarningActive && !_noticeTransitionActive &&
		!_waterOfLifeActive && !_speakerReportLowering && _menuKind != kMenuBook;
}

void DuneEngine::pauseEngineIntern(bool pause) {
	// ScummVM's dialogs must not use up the report's reading time. Account
	// for the last active interval before pausing and rebase after resuming.
	if (pause) {
		// A dialog may consume the held arrow's release while gameplay is
		// paused. Require a fresh press when control returns to the desert.
		_heldDesertKeyWay = -1;
		_nextDesertWalk = 0;
		tick(false); // Retain active IRQ time without dispatching campaign work.
		updateSkyFade(); // Keep the active fraction, excluding subsequent GUI time.
		_flightStepPauseTime = _system->getMillis();
		_flightFilmPauseTime = _flightStepPauseTime / 5;
		if (_endingCoupleActive)
			updateEndingCouple();
		// Pausing may also happen during startup, before runGame's menu
		// context exists. A live report always has a remaining duration.
		if (_troopNoticeRemaining)
			updateTroopNotice();
	} else {
		_lastClockTime = _system->getMillis(); // GUI time must not expire the campaign countdown.
		_animationClock.rebase(_system->getMillis());
		_skyFadeTime = _system->getMillis() / 5;
		_troopNoticeTime = _system->getMillis() / 5 * 5;
		_mapAnimationTime = _system->getMillis() / 5;
		_battleAnimationTime = _system->getMillis() / 5 * 5;
		_battleAnimationClockSet = true;
		_globeAnimationTime = _system->getMillis() / 5;
		_resultsGaugeTime = _system->getMillis() / 5;
		_cabinMarkerTime = _system->getMillis() / 5;
		_cabinTypingTime = _system->getMillis() / 5;
		_speakerIdleTime = _system->getMillis() / 5;
		_endingCoupleTime = _system->getMillis() / 5;
		_harvesterTime = _system->getMillis() / 5;
		if (_menuKind == kMenuBook && _bookPage == -2)
			_bookFrameTime = resumeMovieTick(_bookFrameTime, _flightFilmPauseTime, _system->getMillis() / 5);
		if (_flightFilmClockSet)
			_flightFilmTime = resumeMovieTick(_flightFilmTime, _flightFilmPauseTime, _system->getMillis() / 5);
		if (_nextFlightStep)
			_nextFlightStep += _system->getMillis() - _flightStepPauseTime;
		resetTroopNoticeIdle(); // Returning from a ScummVM modal is new input.
	}
	Engine::pauseEngineIntern(pause);
}

Common::String DuneEngine::getSaveStateName(int slot) const {
	return getMetaEngine()->getSavegameFile(slot, _targetName.c_str());
}

void DuneEngine::saveRoomEntryCheckpoint(int slot) {
	const char *description = slot == kLastPlaceSaveSlot
		? "Last entering into a place" : "Last entering new sietch";
	const Common::Error saved = saveGameState(slot, description, true);
	if (saved.getCode() != Common::kNoError)
		warning("Dune: could not save entry checkpoint: %s", saved.getDesc().c_str());
}

Common::Error DuneEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeUint32BE(kSaveMark);
	stream->writeUint32LE(kSaveVersion);

	stream->writeUint32LE(_state.size());
	stream->write(_state.begin(), _state.size());

	// Where the player is and what they are looking at.  The fresco's doors
	// are kept as they stand, so that a game saved while the planet was open
	// does not come back with the frame hanging in mid air.
	stream->writeByte(_place);
	stream->writeSint32LE(_scene);
	stream->writeByte(_onWorld ? 1 : 0);
	stream->writeSint32LE(_worldLongitude);
	stream->writeSint32LE(_worldLatitude);
	stream->writeByte(_inDesert ? 1 : 0);
	stream->writeByte(_onGlobe ? 1 : 0);
	stream->writeSint32LE(_frescoDoors);
	stream->writeByte(_onMixer ? 1 : 0);

	// What is in the air, and where the flight came from.
	stream->writeSint32LE(_flyingTo);
	stream->writeUint32LE(_cameFromPlace);

	// What is being said, and who is saying it.
	stream->writeUint32LE(_dialoguePart);
	stream->writeUint32LE(_dialogueLine);
	stream->writeSint32LE(_speakerFrame);
	stream->writeSint32LE(_speakerPerson);

	// The list the panel is showing, and what each row of it stands for.
	stream->writeByte((byte)_menuKind);
	stream->writeUint32LE(_menuFrom);
	stream->writeSint32LE(_menuUnder);

	stream->writeUint32LE(_menu.size());
	for (uint i = 0; i != _menu.size(); i++)
		writeSaveString(stream, _menu[i]);

	stream->writeUint32LE(_menuParts.size());
	for (uint i = 0; i != _menuParts.size(); i++)
		stream->writeUint32LE(_menuParts[i]);

	stream->writeUint32LE(_menuFlags.size());
	for (uint i = 0; i != _menuFlags.size(); i++)
		stream->writeUint16LE(_menuFlags[i]);

	// What is written over the picture, in the parts it waits to be read in.
	writeSaveString(stream, _speech);

	stream->writeUint32LE(_speechParts.size());
	for (uint i = 0; i != _speechParts.size(); i++)
		writeSaveString(stream, _speechParts[i]);

	stream->writeUint32LE(_speechPart);

	writeSaveString(stream, _salFile);
	writeSaveString(stream, _background);
	writeSaveString(stream, _speaker);

	stream->writeUint32LE(_orderingTroop);
	stream->writeUint32LE(_troopInfoPage);
	stream->writeByte(_troopOrdersFromMap ? 1 : 0);

	stream->writeByte(_inCabin ? 1 : 0);
	stream->writeByte(_cabinFromMap ? 1 : 0);
	stream->writeUint32LE(_cabinTyped);
	stream->writeSint32LE(_cabinPointerPoint);

	// DIALOGUE.HSQ is mutable in the original: 0xA097 sets 0x80 on every
	// record after it is spoken.
	stream->writeUint32LE(_dialogue.count());
	for (uint part = 0; part != _dialogue.count(); part++) {
		const Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
		stream->writeUint32LE(records.size());
		for (uint line = 0; line != records.size(); line++)
			stream->writeByte(records[line].flags & 0x80);
	}

	stream->writeUint32LE(_hourElapsedMs);

	stream->writeSint32LE(_speakerAnimation);
	stream->writeByte(_speakerReport);

	stream->writeSint32LE(_guidedFlightPlace);
	stream->writeByte(_flightDiscoveryPrompt ? 1 : 0);

	stream->writeUint32LE(_troopRoutes.size());
	if (!_troopRoutes.empty())
		stream->write(_troopRoutes.begin(), _troopRoutes.size());
	stream->writeUint32LE(_moveDestinations.size());
	for (uint i = 0; i != _moveDestinations.size(); i++)
		stream->writeUint32LE(_moveDestinations[i]);

	stream->writeByte(_hasSpiceDensityMap ? 1 : 0);

	stream->writeByte(_flightToPoint ? 1 : 0);
	stream->writeUint16LE(_flightTargetLongitude);
	stream->writeUint16LE((uint16)_flightTargetLatitude);

	stream->writeSint32LE(_desertEntryWay);

	stream->writeSint32LE(_pointedAtPlace);
	stream->writeSint32LE(_pressedPlace);

	stream->writeSint32LE(_surveyLongitude);
	stream->writeSint32LE(_surveyLatitude);
	stream->writeUint32LE(_map.terrain().size());
	stream->write(_map.terrain().begin(), _map.terrain().size());
	stream->writeUint16LE(_map.vegetationSeed());
	// or the unit's animation crosses the middle after contact was opened.
	stream->writeByte(_troopContactTop);
	// A modal action may end/leave the battle before the next display poll.
	stream->writeUint16LE(!_inDesert && !(_onWorld && !_inCabin) && battleHere() &&
		_battleAnimationPlace == READ_LE_UINT16(_state.begin() + 0x114e) ? _battleAnimationPlace : 0);
	stream->writeUint16LE(_battleAnimationElapsed);
	stream->write(_battlePalette, sizeof(_battlePalette));
	stream->writeUint16LE(_menuKind == kMenuTroopNotice ? _troopNoticeRemaining : 0);
	stream->writeUint16LE(_noticeCloudRow);
	stream->writeUint16LE(_noticeCloudPrevious);
	stream->writeByte(_noticeCloudRemainder);
	stream->writeByte(_noticeCloudPhase);
	stream->writeUint32LE(_noticeCloudSource.size());
	if (!_noticeCloudSource.empty()) {
		stream->write(_noticeCloudSource.begin(), _noticeCloudSource.size());
		stream->write(_noticeCloudVisible.begin(), _noticeCloudVisible.size());
	}
	saveMapAnimations(stream);
	// The recording itself is not replayed on load, but 99BE's silent callback
	// continues from the same pose, frame, timeout and secondary-RNG position.
	// A map can cover a room while retaining the old 99BE fields in memory.
	// It has no local speaker to resume; serializing that dormant callback as
	// active made otherwise valid map checkpoints impossible to load.
	const bool saveIdleActive = _speakerIdleActive && !_onWorld &&
		_speakerPerson >= 0 && _speakerIdlePerson == _speakerPerson;
	stream->writeByte(saveIdleActive ? 1 : 0);
	stream->writeByte(_speakerSpeechMode ? 1 : 0);
	stream->writeByte(_speakerVoiceActive ? 1 : 0);
	stream->writeSint32LE(saveIdleActive ? _speakerIdlePerson : -1);
	stream->writeSint32LE(_speakerIdleNextFrame);
	stream->writeSint32LE(_speakerIdleFrameCount);
	stream->writeSint32LE(_speakerIdleRemaining);
	stream->writeSint32LE(_speakerVoiceAnimation);
	stream->writeSint32LE(_speakerVoiceFrame);
	stream->writeSint32LE(_speakerVoiceRemaining);
	stream->writeByte(_speakerIdleRemainder);
	const bool saveFremenIdentity = saveIdleActive && (_speakerPerson == 14 ||
		(_speakerPerson == 15 && storyStage() != 200));
	stream->writeSint32LE(saveFremenIdentity ? _speakerIdlePicture : -1);
	stream->writeSint32LE(saveFremenIdentity ? _speakerIdleVariant : -1);
	// Map cards use low palette entries inherited from the preceding scene;
	// PERS/ONMAP do not replace all of them when a checkpoint is reloaded.
	byte palette[256 * 3];
	_system->getPaletteManager()->grabPalette(palette, 0, 256);
	stream->write(palette, sizeof(palette));
	const bool saveWindow = _mapWindowGeometryValid && _onWorld && !_onGlobe && !_inCabin &&
		(_menuKind == kMenuMap || _menuKind == kMenuPlace) &&
		(mapTroopCard() >= 0 || _pressedPlace >= 0);
	stream->writeByte(saveWindow ? 1 : 0);
	stream->writeSint16LE(_mapWindowAnchor.x);
	stream->writeSint16LE(_mapWindowAnchor.y);
	stream->writeSint16LE(_mapWindowBounds.left);
	stream->writeSint16LE(_mapWindowBounds.top);
	stream->writeSint16LE(_mapWindowBounds.right);
	stream->writeSint16LE(_mapWindowBounds.bottom);
	const bool saveTroopWindow = _troopWindowGeometryValid && _onWorld && !_onGlobe && !_inCabin &&
		hasTroopContact() && READ_LE_UINT16(_state.begin() + 0x46ef);
	stream->writeByte(saveTroopWindow ? 1 : 0);
	stream->writeSint16LE(_troopWindowAnchor.x);
	stream->writeSint16LE(_troopWindowAnchor.y);
	stream->writeSint16LE(_troopWindowBounds.left);
	stream->writeSint16LE(_troopWindowBounds.top);
	stream->writeSint16LE(_troopWindowBounds.right);
	stream->writeSint16LE(_troopWindowBounds.bottom);
	const bool saveTroopPortrait = saveTroopWindow && _troopPortraitKey > 0 &&
		_troopPortraitVisible.size() == kTroopPortraitPixels && _troopPortraitClean.size() == kTroopPortraitPixels;
	stream->writeUint16LE(saveTroopPortrait ? _troopPortraitKey : 0);
	if (saveTroopPortrait) {
		stream->write(_troopPortraitVisible.begin(), kTroopPortraitPixels);
		stream->write(_troopPortraitClean.begin(), kTroopPortraitPixels);
	}
	if (_menuKind == kMenuEquipment)
		stream->write(_equipmentCardSpare, sizeof(_equipmentCardSpare));
	const uint resultsSize = _onGlobe && _frescoDoors && !_resultsPage.empty()
		? kScreenWidth * kSceneHeight : 0;
	stream->writeUint16LE(_resultsGaugeRemainder);
	stream->writeUint32LE(resultsSize);
	if (resultsSize)
		stream->write(_resultsPage.begin(), resultsSize);
	stream->writeByte(_dialogueActionPending ? 1 : 0);
	stream->writeByte(_localRoomRequestPending ? 1 : 0);
	stream->writeByte(_pendingRoomExit + 1);
	stream->writeByte(_pendingRoomLead);
	stream->writeByte(_speakerCardValueValid && (_speakerPerson == 3 || _speakerPerson == 13) ? 1 : 0);
	stream->writeUint16LE(_speakerCardValue);
	stream->writeByte(_speakerReportLowered && (_speakerPerson == 3 || _speakerPerson == 11 || _speakerPerson == 13) ? 1 : 0);
	writeSaveString(stream, (_speakerPerson == 3 || _speakerPerson == 11) && _state[0x47e1] == 0x80 ? _speakerCardText : Common::String());
	stream->writeUint16LE(_state[0x4774] ? _eventMenuRemainder : 0);
	const bool saveHarvester = _harvesterActive && _inDesert && _flyingTo < 0 &&
		!_onWorld && !_onGlobe && !_inCabin && !_onMixer;
	stream->writeByte(saveHarvester ? 1 : 0);
	stream->writeUint16LE(saveHarvester ? _harvesterRemainder : 0);
	if (saveHarvester) {
		stream->write(_harvesterBackdrop.begin(), _harvesterBackdrop.size());
		stream->write(_harvesterPage.begin(), _harvesterPage.size());
	}
	const bool saveCouple = _endingCoupleActive && !_onWorld && !_inCabin && !_inDesert && !_onMixer &&
		_menuKind == kMenuPalaceGathering && !_menuParts.empty() && _menuParts[0] == 518;
	stream->writeByte(saveCouple ? 1 : 0);
	stream->writeUint16LE(saveCouple ? _endingCoupleRemainder : 0);
	if (saveCouple) {
		stream->write(_endingCoupleBackdrop.begin(), _endingCoupleBackdrop.size());
		stream->write(_endingCouplePage.begin(), _endingCouplePage.size());
	}
	const bool savePersonNews = _menuKind == kMenuTalk && _speakerPerson >= 0 &&
		_dialoguePart / 8 == (uint)_speakerPerson && (_dialoguePart & 7) >= 5;
	stream->writeUint16LE(savePersonNews ? _personNewsPart : 0xffff);
	stream->writeUint16LE(_personNewsLine);
	stream->writeByte(_personNewsMask);
	stream->writeByte(_personNewsEnded ? 1 : 0);
	const uint parentCount = (_menuKind == kMenuTalk || _menuKind == kMenuShipmentResponse)
		? _conversationParentMenu.size() : 0;
	stream->writeUint16LE(parentCount);
	stream->writeByte(parentCount && _conversationParentKind == kMenuDesert ? 1 : 0);
	stream->writeByte(parentCount ? _conversationParentFrom : 0);
	for (uint i = 0; i < parentCount; i++) {
		writeSaveString(stream, _conversationParentMenu[i]);
		stream->writeUint32LE(_conversationParentParts[i]);
		stream->writeUint16LE(_conversationParentFlags[i]);
	}
	stream->writeUint16LE(_bookEntries.size());
	for (uint i = 0; i < _bookEntries.size(); i++)
		stream->writeUint16LE(_bookEntries[i]);
	stream->writeSint16LE(_pendingBoardingPlace);
	stream->writeByte(_pendingBoardingGuided ? 1 : 0);
	stream->writeByte(_speechCaptionMode == 0 && !_speech.empty() && _speechCaptionText == _speech
		? _speechCaptionShadow : 0);
	saveGlobeAnimation(stream);
	stream->writeSint16LE(_pendingRoomCompass);
	saveFlightMovie(stream);
	// The camera/cache themselves are already in DS. Keep validity separate:
	// hidden insets and selectors may not have prepared a new cache yet.
	const bool airborne = _flyingTo >= 0 && (_state[0x11c9] & 3);
	const uint trailCount = airborne ? _flightTrail.size() : 0;
	stream->writeUint32BE(0x46494e31); // FIN1, with a bounded self-describing tail.
	stream->writeByte(_flightInsetSet && airborne && !_onWorld ? 1 : 0);
	stream->writeUint16LE(trailCount);
	for (uint i = 0; i < trailCount; i++) {
		stream->writeUint16LE((uint16)_flightTrail[i].x);
		stream->writeSint16LE(_flightTrail[i].y);
	}
	stream->writeUint16LE(9 + 4 * trailCount);
	stream->writeUint32BE(0x534b5931); // SKY1; target page survives even while inactive.
	stream->writeByte(_skyFadeActive ? 1 : 0);
	stream->writeUint16LE(_skyFadeActive ? _skyFadeRemainder : 0);
	stream->write(_skyFadeTarget, sizeof(_skyFadeTarget));
	stream->writeUint16LE(kSkyFadeSaveSize);

	return Common::kNoError;
}

Common::Error DuneEngine::loadGameStream(Common::SeekableReadStream *stream) {
	// B473 restores physical state, terrain and read marks, not the calling
	// globe's orientation or doors. Only the explicit globe Load command uses
	// B400's return; ordinary native checkpoint loads retain their saved UI.
	const int callingSpin = _globeSpin, callingTilt = _globeTilt;
	const int callingDoors = _frescoDoors;
	const byte callingVision = _state[0xdd02];
	if (stream->readUint32BE() != kSaveMark)
		return Common::Error(Common::kReadingFailed, "not a Dune saved game");

	if (stream->readUint32LE() != kSaveVersion)
		return Common::Error(Common::kReadingFailed, "unsupported Dune save format");

	const uint32 size = stream->readUint32LE();
	if (size != _state.size())
		return Common::Error(Common::kReadingFailed, "saved from a different game");

	stopEndingCouple();
	stopDesertHarvester();
	stopSoundEffect(); // A checkpoint must not inherit the previous world's DMA.
	stopFlightMovie();
	_flightFrame = 0; // Reset the pose before restoring any active flight.
	stream->read(_state.begin(), size);
	_skyFadeActive = false;
	_skyFadeRemainder = 0;
	_skyFadeTime = _system->getMillis() / 5;
	memset(_skyFadeTarget, 0, sizeof(_skyFadeTarget));
	resetPointerButtons(); // Hardware holds are not checkpoint state.
	_panelMenuOldPage.clear();
	_conversationParentMenu.clear();
	_conversationParentParts.clear();
	_conversationParentFlags.clear();
	_bookEntries.clear();
	_bookPage = -1;
	_mapTroopCardPixels.clear();
	_resultsPage.clear();
	_surveyDragging = false;
	const Common::Point origin(READ_LE_UINT16(_state.begin() + 0x11c1),
		READ_LE_UINT16(_state.begin() + 0x11c3));
	// 79EE's initial upper-chief handover is one pixel below 59D1's
	// draggable range. It is a valid retained frame before its first drag.
	if (origin != clampSurveyWindowPosition(origin) && origin != Common::Point(92, 30))
		return Common::Error(Common::kReadingFailed, "invalid survey window position");

	// Restore immutable room and layout tables from the executable.
	enum { kFirstRoomTableByte = 0x1261, kAfterRoomTableBytes = 0x145c };
	if (_staticState.size() >= kAfterRoomTableBytes &&
		_state.size() >= kAfterRoomTableBytes)
		memcpy(_state.begin() + kFirstRoomTableByte,
			   _staticState.begin() + kFirstRoomTableByte,
			   kAfterRoomTableBytes - kFirstRoomTableByte);

	_place = stream->readByte();
	_scene = stream->readSint32LE();
	_onWorld = stream->readByte() != 0;
	_worldLongitude = stream->readSint32LE();
	_worldLatitude = stream->readSint32LE();
	_inDesert = stream->readByte() != 0;
	_onGlobe = stream->readByte() != 0;
	_frescoDoors = stream->readSint32LE();
	// The six DD17 counters are in the saved DOS state; rebase only the host
	// timer so an open results view resumes without replaying elapsed wall time.
	_animationClock.invalidate();
	_resultsGaugeTime = _system->getMillis() / 5;
	_onMixer = stream->readByte() != 0;
	// Preserve the saved event's selection phase, without counting time
	// spent outside the game toward its next half-second blink.
	_eventMenuTime = _system->getMillis() / 5;

	_flyingTo = stream->readSint32LE();
	_cameFromPlace = stream->readUint32LE();

	_dialoguePart = stream->readUint32LE();
	_dialogueLine = stream->readUint32LE();
	_speakerFrame = stream->readSint32LE();
	_speakerPerson = stream->readSint32LE();
	// The mask is derived from the saved part, not the previous conversation.
	// 96F1/9F8B uses 20 for room requests as well as the two party replies.
	// Rebuild it before validating an unfinished automatic paragraph below.
	_dialogueMask = ((_dialoguePart & 7) == kMakingOff ||
					 (_dialoguePart & 7) == kComingAlong ||
					 (_dialoguePart & 7) == kStayingBehind) ? 0x20 : 0x80;

	const byte menuKind = stream->readByte();
	if (menuKind > kMenuBook || menuKind == kMenuReservedPalaceAlert)
		return Common::Error(Common::kReadingFailed, "invalid Dune menu kind");
	_menuKind = (MenuKind)menuKind;
	// An open route editor owns the survey display.
	if (_onWorld && _menuKind == kMenuMoveTroop)
		_state[0x46eb] = 0xc0;
	_menuFrom = stream->readUint32LE();
	_menuUnder = stream->readSint32LE();

	_menu.clear();
	const uint32 menuSize = stream->readUint32LE();
	for (uint32 i = 0; i != menuSize; i++)
		_menu.push_back(readSaveString(stream));

	_menuParts.clear();
	const uint32 partsSize = stream->readUint32LE();
	for (uint32 i = 0; i != partsSize; i++)
		_menuParts.push_back(stream->readUint32LE());

	_menuFlags.clear();
	const uint32 flagsSize = stream->readUint32LE();
	for (uint32 i = 0; i != flagsSize; i++)
		_menuFlags.push_back(stream->readUint16LE());

	_speech = readSaveString(stream);

	_speechParts.clear();
	const uint32 speechSize = stream->readUint32LE();
	for (uint32 i = 0; i != speechSize; i++)
		_speechParts.push_back(readSaveString(stream));

	_speechPart = stream->readUint32LE();

	_salFile = readSaveString(stream);
	_background = readSaveString(stream);
	_speaker = readSaveString(stream);

	_mapAnimationFrame = 0;
	_troopRects.clear();
	_orderingTroop = stream->readUint32LE();
	_troopInfoPage = stream->readUint32LE();
	_troopOrdersFromMap = stream->readByte() != 0;
	// A saved handover retains its open survey.
	if (_onWorld && _menuKind == kMenuProspectorBriefing && _troopInfoPage == 1)
		_state[0x46eb] = 0xc0;

	_cabinMapPage.clear();
	_cabinMarkerRemainder = 0;
	_cabinMarkerTime = _system->getMillis() / 5;
	_cabinTypingRemainder = 0;
	_cabinTypingTime = _system->getMillis() / 5;
	_inCabin = stream->readByte() != 0;
	_cabinFromMap = stream->readByte() != 0;
	_cabinTyped = stream->readUint32LE();
	_cabinPointerPoint = stream->readSint32LE();
	if (_cabinTyped < command(86).size())
		_cabinPointerPoint = -2;

	// Loading must first undo anything said since the checkpoint was made;
	// otherwise a runtime load would inherit dialogue marks from the abandoned
	// timeline.  Restore the resource byte rather than merely clearing 0x80:
	// a number of records carry that bit from the beginning and remain valid in
	// dialogue passes selected by another mask.
	for (uint part = 0; part != _dialogue.count(); part++) {
		Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
		for (uint line = 0; line != records.size(); line++)
			records[line].flags = records[line].baseFlags;
	}

	const uint32 parts = stream->readUint32LE();
	for (uint32 part = 0; part != parts; part++) {
		const uint32 lines = stream->readUint32LE();
		for (uint32 line = 0; line != lines; line++) {
			const byte said = stream->readByte();
			if (part < _dialogue.count() && line < _dialogue.entry(part).size())
				_dialogue.entry(part)[line].flags |= said & 0x80;
		}
	}

	_hourElapsedMs = stream->readUint32LE();
	if (_hourElapsedMs >= 5)
		return Common::Error(Common::kReadingFailed, "invalid Dune timer fraction");
	_speakerAnimation = stream->readSint32LE();
	_speakerReport = stream->readByte();
	_guidedFlightPlace = stream->readSint32LE();
	_flightDiscoveryPrompt = stream->readByte() != 0;
	_troopRoutes.resize(67 * 3);
	memset(_troopRoutes.begin(), 0, _troopRoutes.size());
	_moveDestinations.clear();
	const uint32 routes = stream->readUint32LE();
	for (uint32 i = 0; i != routes; i++) {
		const byte route = stream->readByte();
		if (i < _troopRoutes.size())
			_troopRoutes[i] = route;
	}
	const uint32 moving = stream->readUint32LE();
	for (uint32 i = 0; i != moving; i++)
		_moveDestinations.push_back(stream->readUint32LE());
	_hasSpiceDensityMap = stream->readByte() != 0;
	syncProspectorRoute();
	_flightToPoint = stream->readByte() != 0;
	_flightTargetLongitude = stream->readUint16LE();
	_flightTargetLatitude = stream->readSint16LE();
	// These are already serialized in the original DS image. Restore their
	// native mirrors as well, or loading a steered flight resumes autopilot.
	_flightHeading = _state[0x11c7];
	_flightSteered = _state[0x11c8] != 0;
	_nextFlightTurn = 0;
	_nextFlightStep = 0;
	_flightInsetSet = false;
	_flightTrail.clear();
	_desertEntryWay = stream->readSint32LE();
	_pointedAtPlace = stream->readSint32LE();
	_pressedPlace = stream->readSint32LE();
	_surveyLongitude = stream->readSint32LE();
	_surveyLatitude = stream->readSint32LE();
	if (_surveyLongitude < 0 || _surveyLongitude > 65535 ||
			_surveyLatitude < -42 || _surveyLatitude > 42) {
		return Common::Error(Common::kReadingFailed, "invalid Dune survey centre");
	}
	Common::Array<byte> savedTerrain;
	const uint32 terrainSize = stream->readUint32LE();
	if (terrainSize != _map.terrain().size())
		return Common::Error(Common::kReadingFailed, "invalid saved Dune terrain size");
	savedTerrain.resize(terrainSize);
	if (stream->read(savedTerrain.begin(), terrainSize) != terrainSize)
		return Common::Error(Common::kReadingFailed, "truncated saved Dune terrain");
	const uint16 vegetationSeed = stream->readUint16LE();
	if (stream->err() || stream->eos())
		return Common::Error(Common::kReadingFailed, "truncated Dune vegetation state");
	_troopContactTop = stream->readByte();
	if (stream->err() || stream->eos() ||
			(_troopContactTop != 0 && _troopContactTop != 5 && _troopContactTop != 80))
		return Common::Error(Common::kReadingFailed, "invalid Dune contact layout");
	_battleAnimationTime = 0;
	_battleAnimationClockSet = false; // Native checkpoints rebase the host clock, not the callback remainder.
	_battleAnimationPlace = stream->readUint16LE();
	_battleAnimationElapsed = stream->readUint16LE();
	if (stream->read(_battlePalette, sizeof(_battlePalette)) != sizeof(_battlePalette) || stream->err())
		return Common::Error(Common::kReadingFailed, "truncated Dune battlefield state");
	if (_battleAnimationElapsed >= 15 || (_battleAnimationPlace &&
			(_battleAnimationPlace != READ_LE_UINT16(_state.begin() + 0x114e) ||
			!battleHere() || _inDesert || READ_LE_UINT16(_state.begin() + 0x3cbe) > kMaxBattleActors)))
		return Common::Error(Common::kReadingFailed, "invalid Dune battlefield state");
	for (uint i = 0; i < sizeof(_battlePalette); i++) {
		if (_battlePalette[i] > 63)
			return Common::Error(Common::kReadingFailed, "invalid Dune battlefield palette");
	}
	_troopNoticeRemaining = stream->readUint16LE();
	if (stream->err() || stream->eos() ||
			_troopNoticeRemaining > kTroopNoticeDuration ||
			(_menuKind != kMenuTroopNotice && _troopNoticeRemaining))
		return Common::Error(Common::kReadingFailed, "invalid Dune report timer");
	stopNoticeCloud();
	const uint16 cloudRow = stream->readUint16LE();
	const uint16 previous = stream->readUint16LE();
	const byte cloudRemainder = stream->readByte();
	const byte phase = stream->readByte();
	const uint32 cloudSize = stream->readUint32LE();
	if (stream->err() || stream->eos() || cloudRow > 148 ||
			(previous != 5 && (previous < 8 || previous > 47048 || (previous - 8) % kScreenWidth)) ||
			cloudRemainder >= 6 || phase >= 64 ||
			cloudSize != (_menuKind == kMenuTroopNotice ? kScreenWidth * kScreenHeight : 0) ||
			(!cloudSize && (cloudRemainder || phase)))
		return Common::Error(Common::kReadingFailed, "invalid Dune report cloud state");
	if (cloudSize) {
		_noticeCloudSource.resize(cloudSize);
		_noticeCloudVisible.resize(cloudSize);
		if (stream->read(_noticeCloudSource.begin(), cloudSize) != cloudSize ||
				stream->read(_noticeCloudVisible.begin(), cloudSize) != cloudSize || stream->err()) {
			stopNoticeCloud();
			return Common::Error(Common::kReadingFailed, "truncated Dune report cloud pages");
		}
	}
	_noticeCloudRow = cloudRow;
	_noticeCloudPrevious = previous;
	_noticeCloudRemainder = cloudRemainder;
	_noticeCloudPhase = phase;
	if (!loadMapAnimations(stream))
		return Common::Error(Common::kReadingFailed, "invalid Dune map animation state");
	bool savedIdleActive = stream->readByte() != 0;
	bool savedSpeechMode = stream->readByte() != 0;
	bool savedVoiceActive = stream->readByte() != 0;
	int savedIdlePerson = stream->readSint32LE();
	int savedIdleNext = stream->readSint32LE();
	int savedIdleFrames = stream->readSint32LE();
	int savedIdleRemaining = stream->readSint32LE();
	int savedVoiceAnimation = stream->readSint32LE();
	int savedVoiceFrame = stream->readSint32LE();
	int savedVoiceRemaining = stream->readSint32LE();
	const bool idleFieldsValid = savedIdlePerson >= 0 && savedIdlePerson < 16 &&
		savedIdleFrames >= 1 && savedIdleFrames <= 128 &&
		savedIdleNext >= 0 && savedIdleNext <= savedIdleFrames &&
		savedIdleRemaining >= -128 && savedIdleRemaining <= 1020;
	// A map save can retain a valid, but unregistered,
	// room portrait after 5A1A has cleared the visible speaker. Ignore only
	// that dormant callback; still reject malformed active portrait data.
	const bool dormantMapIdle = savedIdleActive && idleFieldsValid &&
		_onWorld && _speakerPerson == -1;
	if (stream->err() || stream->eos() ||
			(savedIdleActive && (!idleFieldsValid ||
				(!dormantMapIdle && (_onWorld || savedIdlePerson != _speakerPerson)))) ||
			(savedVoiceActive && (!savedSpeechMode || savedVoiceFrame < 0 ||
				savedVoiceFrame > 127 || savedVoiceAnimation < 0 ||
				savedVoiceAnimation > 16 || savedVoiceRemaining < -16 ||
				savedVoiceRemaining > 8)))
		return Common::Error(Common::kReadingFailed, "invalid Dune portrait callback state");
	if (dormantMapIdle) {
		savedIdleActive = false;
		savedIdlePerson = -1;
	}
	const byte savedIdleRemainder = stream->readByte();
	if (savedIdleRemainder >= 16 || stream->err() || stream->eos())
		return Common::Error(Common::kReadingFailed, "invalid Dune portrait callback phase");
	int savedIdlePicture = stream->readSint32LE();
	int savedIdleVariant = stream->readSint32LE();
	const bool fremen = savedIdleActive && (savedIdlePerson == 14 ||
		(savedIdlePerson == 15 && storyStage() != 200));
	if (stream->err() || stream->eos() || (fremen ?
			(savedIdlePicture < 14 || savedIdlePicture > 16 || savedIdleVariant < 0 ||
			 savedIdleVariant >= (savedIdlePicture == 14 ? 15 : 17)) :
			(savedIdlePicture != -1 || savedIdleVariant != -1)))
		return Common::Error(Common::kReadingFailed, "invalid Dune retained Fremen portrait");
	// 961B keeps caller14 but replaces RALLY with GIVE ORDERS. The saved
	// menu already retains that role; do not turn it back into another oath.
	_listAs = _menuKind == kMenuTalk && _speakerPerson == 14 &&
		_menuParts.size() > 1 && _menuParts[1] == 146 ? 15 : -1;
	byte savedPalette[256 * 3];
	if (stream->read(savedPalette, sizeof(savedPalette)) != sizeof(savedPalette) || stream->err())
		return Common::Error(Common::kReadingFailed, "truncated Dune palette");
	// CS:38C0..38D2 are driver-owned, not DS:D816 scratch. A travelling
	// actor may move and hourly gameplay may reuse that scratch while its
	// report remains open.
	{
		const byte valid = stream->readByte();
		_mapWindowAnchor.x = stream->readSint16LE();
		_mapWindowAnchor.y = stream->readSint16LE();
		_mapWindowBounds.left = stream->readSint16LE();
		_mapWindowBounds.top = stream->readSint16LE();
		_mapWindowBounds.right = stream->readSint16LE();
		_mapWindowBounds.bottom = stream->readSint16LE();
		if (stream->err() || stream->eos() || valid > 1 || (valid &&
			(_mapWindowBounds.width() != 106 || _mapWindowBounds.height() <= 0 ||
			 _mapWindowBounds.top < 4 || _mapWindowBounds.bottom > 148)))
			return Common::Error(Common::kReadingFailed, "invalid Dune map window geometry");
		_mapWindowGeometryValid = valid && _onWorld && !_onGlobe && !_inCabin &&
			(_menuKind == kMenuMap || _menuKind == kMenuPlace) &&
			(mapTroopCard() >= 0 || _pressedPlace >= 0);
	}
	{
		const byte valid = stream->readByte();
		_troopWindowAnchor.x = stream->readSint16LE();
		_troopWindowAnchor.y = stream->readSint16LE();
		_troopWindowBounds.left = stream->readSint16LE();
		_troopWindowBounds.top = stream->readSint16LE();
		_troopWindowBounds.right = stream->readSint16LE();
		_troopWindowBounds.bottom = stream->readSint16LE();
		if (stream->err() || stream->eos() || valid > 1 || (valid &&
			(_troopWindowBounds.left != 5 || _troopWindowBounds.right != 232 ||
			 _troopWindowBounds.height() != 67 ||
			 (_troopWindowBounds.top != 5 && _troopWindowBounds.top != 80) ||
			 _troopWindowBounds.top != _troopContactTop)))
			return Common::Error(Common::kReadingFailed, "invalid Dune troop window geometry");
		_troopWindowGeometryValid = valid && _onWorld && !_onGlobe && !_inCabin &&
			hasTroopContact() && READ_LE_UINT16(_state.begin() + 0x46ef);
	}
	_troopPortraitKey = -1;
	_troopPortraitVisible.clear();
	_troopPortraitClean.clear();
	const uint portraitKey = stream->readUint16LE();
	const uint bank = portraitKey >> 8, variant = portraitKey & 0xff;
	const bool validKey = (bank == 12 && variant == 0) ||
		(bank >= 14 && bank <= 16 && variant < (bank == 14 ? 15U : 17U));
	if (stream->err() || stream->eos() || (portraitKey && (!_troopWindowGeometryValid || !validKey)))
		return Common::Error(Common::kReadingFailed, "invalid Dune retained troop report portrait");
	if (portraitKey) {
		if (duneSavePayloadEnd(stream) - stream->pos() < 2 * kTroopPortraitPixels)
			return Common::Error(Common::kReadingFailed, "truncated Dune troop report portrait");
		_troopPortraitVisible.resize(kTroopPortraitPixels);
		_troopPortraitClean.resize(kTroopPortraitPixels);
		if (stream->read(_troopPortraitVisible.begin(), kTroopPortraitPixels) != kTroopPortraitPixels ||
				stream->read(_troopPortraitClean.begin(), kTroopPortraitPixels) != kTroopPortraitPixels || stream->err())
			return Common::Error(Common::kReadingFailed, "truncated Dune troop report portrait");
		_troopPortraitKey = portraitKey;
	}
	if (_menuKind == kMenuEquipment &&
			(stream->read(_equipmentCardSpare, sizeof(_equipmentCardSpare)) != sizeof(_equipmentCardSpare) || stream->err()))
		return Common::Error(Common::kReadingFailed, "truncated Dune spare equipment row");
	_resultsGaugeRemainder = stream->readUint16LE();
	const uint32 resultsSize = stream->readUint32LE();
	if (stream->err() || _resultsGaugeRemainder >= 12 ||
			(resultsSize && resultsSize != kScreenWidth * kSceneHeight))
		return Common::Error(Common::kReadingFailed, "invalid Dune retained results page");
	if (resultsSize) {
		_resultsPage.resize(kScreenWidth * kScreenHeight);
		if (stream->read(_resultsPage.begin(), resultsSize) != resultsSize || stream->err()) {
			_resultsPage.clear();
			return Common::Error(Common::kReadingFailed, "truncated Dune retained results page");
		}
		if (!_onGlobe || !_frescoDoors)
			_resultsPage.clear();
	}
	{
		const byte pending = stream->readByte();
		if (stream->err() || stream->eos() || pending > 1 || (pending &&
				(_speakerPerson < 0 || _dialoguePart >= _dialogue.count() ||
				 _dialogueLine >= _dialogue.entry(_dialoguePart).size() ||
				 _speechPart >= _speechParts.size() ||
				 _speechParts.size() - _speechPart < 2)))
			return Common::Error(Common::kReadingFailed, "invalid Dune pending dialogue action");
		_dialogueActionPending = pending != 0;
	}
	{
		const byte pending = stream->readByte();
		if (stream->err() || stream->eos() || pending > 1 || (pending &&
				(!_dialogueActionPending || _onWorld || _onGlobe || _inCabin || _onMixer ||
				 _dialogueMask != 0x20 || (_dialoguePart & 7) != 4 ||
				 _speakerPerson < 0 || _speakerPerson >= 16 ||
				 _dialoguePart / 8 != (uint)_speakerPerson)))
			return Common::Error(Common::kReadingFailed, "invalid Dune automatic room paragraph");
		_localRoomRequestPending = pending != 0;
	}
	{
		const byte exit = stream->readByte();
		const byte leads = stream->readByte();
		const byte *room = roomRecord(_state[8], _state[0x0b]);
		if (stream->err() || stream->eos() || exit > 4 || (!exit && leads) || (exit &&
				(_onWorld || _onGlobe || _inCabin || _inDesert || _onMixer ||
				 !room || !leads || leads >= 0x80 || room[exit] != leads ||
				 _state[0x0c] != leads || _state[0x23] != (_localRoomRequestPending ? 1 : 0))))
			return Common::Error(Common::kReadingFailed, "invalid Dune pending room departure");
		_pendingRoomExit = exit - 1;
		_pendingRoomLead = leads;
	}
	{
		const byte valid = stream->readByte();
		const uint16 value = stream->readUint16LE();
		const byte lowered = stream->readByte();
		if (stream->err() || stream->eos() || valid > 1 || (valid &&
				((_speakerPerson != 3 && _speakerPerson != 13) || (_speakerPerson == 13 && value > 255))))
			return Common::Error(Common::kReadingFailed, "invalid Dune numeric card value");
		_speakerCardValueValid = valid != 0;
		_speakerCardValue = value;
		const bool reversePose = _speakerPerson == 3 ? _speakerAnimation == 6 || _speakerAnimation == 9 :
			_speakerPerson == 11 ? _speakerAnimation == 6 :
			_speakerPerson == 13 && _speakerAnimation == 7 + 2 * localSpeakerVariant();
		// 9A3B and 9B49 can leave a negative counter before the next speech
		// block starts. Keep its low bits for 9985.
		if (lowered > 1 || (lowered && (!reversePose || _speakerReport != 0 || !savedSpeechMode ||
				READ_LE_UINT16(_state.begin() + 0x47e1) != 0)))
			return Common::Error(Common::kReadingFailed, "invalid Dune lowered report pose");
		_speakerReportLowered = lowered != 0;
	}
	{
		_speakerCardText = readSaveString(stream);
		const uint descriptor = READ_LE_UINT16(_state.begin() + 0x47e4);
		const uint kind = descriptor + 8 <= _state.size() ? READ_LE_UINT16(_state.begin() + descriptor) : 0;
		bool valid = _speakerCardText.size() <= 128;
		for (uint i = 0; i < _speakerCardText.size(); i++)
			valid &= _speakerCardText[i] == '\r' || (_speakerCardText[i] >= 32 && _speakerCardText[i] < 127);
		if (stream->err() || stream->eos() || !valid || (!_speakerCardText.empty() &&
				((_speakerPerson != 3 && _speakerPerson != 11) || _state[0x47e1] != 0x80 || _speakerReportLowered ||
				 (_speakerPerson == 11 ? kind != 0x37 :
				 (kind < 0x30 || kind > 0x37)))))
			return Common::Error(Common::kReadingFailed, "invalid Dune completed report text");
	}
	_eventMenuRemainder = stream->readUint16LE();
	if (stream->err() || stream->eos() || _eventMenuRemainder >= 100)
		return Common::Error(Common::kReadingFailed, "invalid Dune event menu timer");
	if (!_state[0x4774])
		_eventMenuRemainder = 0;
	const byte activeHarvester = stream->readByte();
	_harvesterRemainder = stream->readUint16LE();
	if (activeHarvester > 1 || _harvesterRemainder >= 16 || stream->err() || stream->eos())
		return Common::Error(Common::kReadingFailed, "invalid Dune harvester callback phase");
	_harvesterActive = activeHarvester != 0;
	if (_harvesterActive) {
		_harvesterBackdrop.resize(kScreenWidth * kSceneHeight);
		_harvesterPage.resize(kScreenWidth * kSceneHeight);
		if (duneSavePayloadEnd(stream) - stream->pos() < (int64)(_harvesterBackdrop.size() + _harvesterPage.size() + 3) ||
				stream->read(_harvesterBackdrop.begin(), _harvesterBackdrop.size()) != _harvesterBackdrop.size() ||
				stream->read(_harvesterPage.begin(), _harvesterPage.size()) != _harvesterPage.size() ||
				stream->err() || !validDesertHarvesterState()) {
			stopDesertHarvester();
			return Common::Error(Common::kReadingFailed, "invalid Dune harvester composite state");
		}
		_restoreHarvesterPage = true;
	}
	const byte activeCouple = stream->readByte();
	_endingCoupleRemainder = stream->readUint16LE();
	if (activeCouple > 1 || _endingCoupleRemainder >= 16 || stream->err() || stream->eos())
		return Common::Error(Common::kReadingFailed, "invalid Dune ending portrait phase");
	_endingCoupleActive = activeCouple != 0;
	if (_endingCoupleActive) {
		_endingCoupleBackdrop.resize(kScreenWidth * kScreenHeight);
		_endingCouplePage.resize(kScreenWidth * kSceneHeight);
		if (_onWorld || _inCabin || _inDesert || _onMixer || _speakerPerson != -1 ||
				_menuKind != kMenuPalaceGathering || _menuParts.empty() || _menuParts[0] != 518 ||
				duneSavePayloadEnd(stream) - stream->pos() <
					(int64)(_endingCoupleBackdrop.size() + _endingCouplePage.size() + 8) ||
				stream->read(_endingCoupleBackdrop.begin(), _endingCoupleBackdrop.size()) != _endingCoupleBackdrop.size() ||
				stream->read(_endingCouplePage.begin(), _endingCouplePage.size()) != _endingCouplePage.size() ||
				stream->err() || !validEndingCoupleState()) {
			stopEndingCouple();
			return Common::Error(Common::kReadingFailed, "invalid Dune paired ending portrait state");
		}
	}
	const uint newsPart = stream->readUint16LE();
	const uint line = stream->readUint16LE();
	const byte mask = stream->readByte();
	const byte ended = stream->readByte();
	if (stream->err() || stream->eos() || ended > 1 || (mask != 0x80 && mask != 0x20) ||
			(newsPart != 0xffff && (newsPart >= _dialogue.count() || (newsPart & 7) >= 4 ||
			 _speakerPerson < 0 || newsPart / 8 != (uint)_speakerPerson ||
			 _menuKind != kMenuTalk || _dialoguePart / 8 != (uint)_speakerPerson || (_dialoguePart & 7) < 5 ||
			 (line != 0xffff && line >= _dialogue.entry(newsPart).size()))))
		return Common::Error(Common::kReadingFailed, "invalid Dune interrupted person news");
	_personNewsPart = newsPart;
	_personNewsLine = line;
	_personNewsMask = mask;
	_personNewsEnded = ended != 0;
	{
		const uint count = stream->readUint16LE();
		const byte desert = stream->readByte();
		const byte from = stream->readByte();
		if (count > 64 || desert > 1 || (from && (from >= count || from % 4)) ||
				stream->err() || duneSavePayloadEnd(stream) - stream->pos() < 8 * count + 2 ||
				(count && _menuKind != kMenuTalk && _menuKind != kMenuShipmentResponse))
			return Common::Error(Common::kReadingFailed, "invalid Dune conversation parent menu");
		_conversationParentKind = desert ? kMenuDesert : kMenuRoom;
		_conversationParentFrom = from;
		for (uint i = 0; i < count; i++) {
			const Common::String label = readSaveString(stream);
			const uint part = stream->readUint32LE();
			const uint16 flags = stream->readUint16LE();
			if (label.empty() || label.size() > 256 || part > 0xffff || (flags & ~0xc000) ||
					stream->err() || stream->eos() || stream->pos() >= duneSavePayloadEnd(stream))
				return Common::Error(Common::kReadingFailed, "invalid Dune conversation parent entry");
			_conversationParentMenu.push_back(label);
			_conversationParentParts.push_back(part);
			_conversationParentFlags.push_back(flags);
		}
	}
	{
		const uint count = stream->readUint16LE();
		if (count > 2048 || stream->err() || duneSavePayloadEnd(stream) - stream->pos() < 2 * count)
			return Common::Error(Common::kReadingFailed, "invalid Dune book log length");
		for (uint i = 0; i < count; i++) {
			const uint16 key = stream->readUint16LE();
			const Dialogue::Record *record = _dialogue.bookRecord(key);
			if (stream->err() || !record || !record->bookTopic || (key >> 11) > 16)
				return Common::Error(Common::kReadingFailed, "invalid Dune book record");
			_bookEntries.push_back(key);
		}
	}
	{
		const int place = stream->readSint16LE();
		const byte guided = stream->readByte();
		const uint room = READ_LE_UINT16(_state.begin() + 0x114e);
		const bool roomWormDeparture = !_inDesert && _state[6] == 0x80 &&
			_cameFromPlace < _map.places().size() && room == 0x100 + 28 * _cameFromPlace;
		const bool wormDeparture = _state[0x11c9] == 10 && _flyingTo < 0 && !guided &&
			(_inDesert || roomWormDeparture);
		if (stream->err() || stream->eos() || place < -1 ||
				place >= (int)_map.places().size() || guided > 1 || (place < 0 && guided) ||
				(place >= 0 && (_pendingRoomExit >= 0 ||
				 ((_state[0x11c9] & 3) && !wormDeparture) ||
				 _onWorld || _onGlobe || _inCabin || _onMixer ||
				 _state[0x23] != 7)))
			return Common::Error(Common::kReadingFailed, "invalid Dune pending vehicle boarding");
		_pendingBoardingPlace = place;
		_pendingBoardingGuided = guided != 0;
	}
	// On-foot arrivals can own automatic paragraphs in the desert too.
	// A carried paragraph needs the validated boarding transaction;
	// defer this check until that transaction has been read and validated.
	if (_localRoomRequestPending && _inDesert && _state[0x11c9] &&
			_pendingBoardingPlace < 0)
		return Common::Error(Common::kReadingFailed, "invalid Dune automatic desert paragraph");
	const byte savedCaptionShadow = stream->readByte();
	if (stream->err() || stream->eos() ||
			(savedCaptionShadow != 0 && savedCaptionShadow != 8 && savedCaptionShadow != 240) ||
			(savedCaptionShadow && _speech.empty()))
		return Common::Error(Common::kReadingFailed, "invalid Dune retained subtitle ink");
	if (!loadGlobeAnimation(stream))
		return Common::Error(Common::kReadingFailed, "invalid Dune incremental globe drawing");
	const int way = stream->readSint16LE();
	if (stream->err() || stream->eos() || way < -1 || way > 3)
		return Common::Error(Common::kReadingFailed, "invalid Dune deferred compass exit");
	_pendingRoomCompass = way;
	uint32 savedFlightElapsed = 0, savedFlightStep = 0;
	Common::Array<byte> savedFlightInset;
	if (!loadFlightMovie(stream, duneSavePayloadEnd(stream) - kSkyFadeSaveSize - 9,
			savedFlightElapsed, savedFlightStep, savedFlightInset))
		return Common::Error(Common::kReadingFailed, "invalid Dune retained flight movie");
	{
		const int64 insetEnd = duneSavePayloadEnd(stream) - kSkyFadeSaveSize;
		if (insetEnd - stream->pos() < 9)
			return Common::Error(Common::kReadingFailed, "truncated Dune flight inset");
		if (stream->readUint32BE() != 0x46494e31)
			return Common::Error(Common::kReadingFailed, "invalid Dune flight inset tag");
		const byte inset = stream->readByte();
		const uint count = stream->readUint16LE();
		const int latitude = (int16)READ_LE_UINT16(_state.begin() + 0x197e);
		if (stream->err() || inset > 1 || count > 276 ||
				insetEnd - stream->pos() != 4 * count + 2 ||
				(count && (_flyingTo < 0 || !(_state[0x11c9] & 3))) ||
				(inset && (_flyingTo < 0 || _onWorld ||
				!(_state[0x11c9] & 3) || latitude != clampSurveyLatitude(latitude, 56))))
			return Common::Error(Common::kReadingFailed, "invalid Dune retained flight inset");
		for (uint i = 0; i < count; i++) {
			const int16 longitude = (int16)stream->readUint16LE();
			const int16 row = stream->readSint16LE();
			if (stream->err() || row < -98 || row > 98)
				return Common::Error(Common::kReadingFailed, "invalid Dune flight route point");
			_flightTrail.push_back(Common::Point(longitude, row));
		}
		if (stream->readUint16LE() != 9 + 4 * count || stream->err())
			return Common::Error(Common::kReadingFailed, "invalid Dune flight inset length");
		_flightInsetSet = inset != 0;
	}
	{
		if (duneSavePayloadEnd(stream) - stream->pos() != kSkyFadeSaveSize ||
				stream->readUint32BE() != 0x534b5931)
			return Common::Error(Common::kReadingFailed, "invalid Dune sky fade tag or length");
		const byte active = stream->readByte();
		const uint remainder = stream->readUint16LE();
		if (stream->read(_skyFadeTarget, sizeof(_skyFadeTarget)) != sizeof(_skyFadeTarget) ||
				stream->readUint16LE() != kSkyFadeSaveSize || stream->err() || active > 1 ||
				remainder >= 16 || (!active && remainder) ||
				(active && (!_state[0x46d7] || _state[0x46d7] > 64 || _state[0x46d6] > 39)))
			return Common::Error(Common::kReadingFailed, "invalid Dune sky fade phase");
		for (uint i = 0; i < sizeof(_skyFadeTarget); i++)
			if (_skyFadeTarget[i] > 63)
				return Common::Error(Common::kReadingFailed, "invalid Dune sky target palette");
		_skyFadeActive = active != 0;
		_skyFadeRemainder = remainder;
	}
	// Worm rides and new boarding have no retained flight warning.
	if (((_state[0x11c9] & 3) == 2 || _pendingBoardingPlace >= 0) &&
			(_state[0x21fd] || _state[0x4726]))
		clearFlightHazard();
	if (_harvesterActive)
		_state[0xf6] = 1;

	// The places and the troops are read back out of the memory just loaded,
	// since they are only ever a reading of it.
	initialisePlaceSpiceData();
	_map.restoreTerrain(savedTerrain, vegetationSeed);
	_map.readPlaces(_state.begin(), _state.size());
	const uint16 loadedPlace = _state.size() > 0x114f
		? READ_LE_UINT16(_state.begin() + 0x114e) : 0;
	debugC(1, kDebugScript,
		   "loaded story stage %u, charisma %u, war %u, player place record "
		   "0x%04x, scene place byte 0x%02x", (uint)storyStage(),
		   _state.size() > 0x29 ? _state[0x29] : 0,
		   _state.size() > 0xc2 ? _state[0xc2] : 0, loadedPlace, _place);
	// 331E's packed name in 4E belongs to the saved expanded context,
	// not necessarily Paul's place. Preserve it with the other DS fields;
	// rebuilding the room's runtime bank must not replace a remote query.
	// _salFile and the scene bank are derived runtime context, not save fields.
	// Rebuild them from the retained place and room even when the checkpoint was
	// made on the map, in the cabin, or in a troop console.  Otherwise EXIT MAPS
	// after loading tried to draw the saved room out of an empty/stale SAL bank,
	// leaving a black screen with only people and the panel visible.
	if (loadedPlace >= 0x100 && loadedPlace + 8 < _state.size()) {
		const uint loadedIndex = (loadedPlace - 0x100) / 28;
		if (loadedIndex < _map.places().size())
			roomFor(loadedIndex);
	}
	_map.readTroops(_state.begin(), _state.size());
	// A remote report owns the lowered panel face.
	if (_menuKind == kMenuTroopNotice)
		_state[0xe8] = 0;
	// The selected contact is stored in the native menu/index as well as the
	// DS:1954 latch. Restore it before clock updates resume.
	_state[0x1954] = _onWorld && hasTroopContact() && _orderingTroop < _map.troops().size()
		? _state[0x8aa + 27 * _orderingTroop] : 0;
	WRITE_LE_UINT16(_state.begin() + 0x46ef, _state[0x1954]
		? 0x8aa + 27 * _orderingTroop : 0);

	// The panel is written over as the game runs, so it starts again from the
	// data's own copy, and the compass is laid out for wherever the player
	// has been put down by the redraw that follows.
	resetPanel();

	// Whatever was playing belongs to the place that was left, so the song is
	// picked again for the place that is come to.
	_song.clear();
	_defeatReason = 0;
	_defeatScreen.clear();
	stopSpeech();
	_speakerIdleActive = savedIdleActive;
	_speakerSpeechMode = savedSpeechMode;
	_speakerVoiceActive = savedVoiceActive;
	_speakerIdlePerson = savedIdlePerson;
	const int idleIdentity = savedIdlePerson == 14 && _listAs == 15 ? 15 : savedIdlePerson;
	_speakerIdlePicture = savedIdlePicture >= 0 ? savedIdlePicture : (savedIdleActive
		? fremenPicture(idleIdentity, idleIdentity == 15 ? _state[0x476c] & 7 : 0) : -1);
	_speakerIdleVariant = savedIdleVariant >= 0 ? savedIdleVariant :
		(savedIdleActive && (savedIdlePerson == 14 || (savedIdlePerson == 15 && storyStage() != 200))
			? fremenAnimation(idleIdentity, idleIdentity == 15 ? _state[0x476c] & 7 : 0) : -1);
	_speakerIdleNextFrame = savedIdleNext;
	_speakerIdleFrameCount = savedIdleFrames;
	_speakerIdleRemaining = savedIdleRemaining;
	_speakerVoiceAnimation = savedVoiceAnimation;
	_speakerVoiceFrame = savedVoiceFrame;
	_speakerVoiceRemaining = savedVoiceRemaining;
	_speakerIdleRemainder = savedIdleRemainder;
	_speakerIdleTime = _system->getMillis() / 5;
	_endingCoupleTime = _system->getMillis() / 5;
	_harvesterTime = _system->getMillis() / 5;
	// Native saves retain DOS's elapsed song-gap ticks in CE7A/DBD2. Rebase
	// their common origin to this process's 200-Hz clock after loading.
	rebasePlaylistWait((uint16)(_system->getMillis() / 5));
	_lastClockTime = 0;
	_nextMouthTime = savedIdleActive ? _system->getMillis() + 1000 / 12 : 0;
	_speechDisplayForced = _state[0x28e8] == 2 && !_state[0x46eb] && READ_LE_UINT16(_state.begin() + 0x47c4) < 16 &&
		_dialoguePart < _dialogue.count() && _dialogueLine < _dialogue.entry(_dialoguePart).size() &&
		_dialogue.entry(_dialoguePart)[_dialogueLine].forceBalloon;
	_speechCaptionPixels.clear();
	_speechCaptionText = savedCaptionShadow ? _speech : Common::String();
	_speechCaptionShadow = savedCaptionShadow;
	_speechCaptionMode = 0; // Retained DOS box geometry rebuilds without a new RNG draw.
	_localSpeakerBalloonText.clear();
	_mapSpeechPerson = -1;
	_troopContactMouth = -1;
	_system->getPaletteManager()->setPalette(savedPalette, 0, 256);
	restoreChaniSequence();
	// Only an open route editor owns the pending stops.
	if (_menuKind == kMenuMoveTroop)
		syncPendingTroopRoute();

	// The mixer's knobs are drawn alone by redraw; the panel and the list
	// around them are showMixer's own work.
	if (_loadingGlobeMenu)
		restoreCallingGlobe(callingSpin, callingTilt, callingDoors, callingVision);
	else if (_onMixer)
		showMixer();
	else {
		// Ordinary worm-selector paints retain the landscape/room. Only a
		// fresh load needs its underlying page rebuilt; drawDesert also
		// restores an active harvester's saved frame without restarting it.
		if (_menuKind == kMenuWormMap) {
			if (_inDesert)
				drawDesert(false, false);
			else
				drawScene(false);
		}
		redraw();
	}
	if (!savedFlightInset.empty() && _inDesert && !_onWorld && !_onGlobe && !_onMixer && !_inCabin) {
		for (uint y = 0; y < 64; y++)
			memcpy(_framebuffer1 + y * kScreenWidth + 200, savedFlightInset.begin() + y * 120, 120);
		_system->copyRectToScreen(_framebuffer1 + 200, kScreenWidth, 200, 0, 120, 64);
		_system->updateScreen();
	}
	// A retained battle must rebuild transient digital playback without
	// reinstalling B45 or resetting its saved actors, flash and partial tick.
	// Map redraw above removes B45, so covered checkpoints remain silent.
	if (_battleAnimationPlace)
		playSoundEffect(3);

	playTheRightSong();
	_troopNoticeTime = _system->getMillis() / 5 * 5;
	_troopNoticeButtons = _system->getEventManager()->getButtonState() & 3;
	// Older native saves copied the unused DOS startup lock verbatim.
		// Preserve the real lock of every retained script, including nested counts.
	// Repair the missing lock in older native event checkpoints, but not the
	// deliberate FD9 unlock after Chani's first night-wait pose (1316..1320).
	// That sequence can legitimately retain zero while the event is active.
	const uint16 eventCursor = READ_LE_UINT16(_state.begin() + 0x477a);
	if (_menuKind == kMenuTroopNotice)
		_state[0x2788] = 1; // Restore B2B9.
	else if (!_state[0x4774])
		_state[0x2788] = 0;
	else if (!_state[0x2788] && !(eventCursor >= 0x1316 && eventCursor <= 0x1320))
		_state[0x2788] = 1;
	_noticeWarningActive = false;
	_noticeTransitionActive = false;
	resetTroopNoticeIdle();
	const uint32 resumed = _system->getMillis();
	_animationClock.rebase(resumed); // Do not discard the first post-load interval.
	_skyFadeTime = resumed / 5; // Loading/redrawing is not active timer credit.
	if (_flightMovie >= 0) {
		_flightFilmTime = resumed / 5 - savedFlightElapsed;
		_nextFlightStep = savedFlightStep ? resumed + savedFlightStep : 0;
	}
	_flightFilmPauseTime = resumed / 5;
	_flightStepPauseTime = resumed;

	return Common::kNoError;
}

} // End of namespace Dune
