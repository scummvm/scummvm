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

#include "common/config-manager.h"
#include "common/events.h"
#include "base/plugins.h"
#include "base/version.h"

#include "pegasus/pegasus.h"

#include "common/file.h"

//#define RUN_SUB_MOVIE // :D :D :D :D :D :D
//#define RUN_INTERFACE_TEST

namespace Pegasus {

PegasusEngine::PegasusEngine(OSystem *syst, const PegasusGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
}

PegasusEngine::~PegasusEngine() {
	delete _video;
	delete _sound;
	delete _gfx;
	delete _resFork;
	delete _inventoryLid;
	delete _biochipLid;
}

Common::Error PegasusEngine::run() {
	_gfx = new GraphicsManager(this);
	_video = new VideoManager(this);
	_sound = new SoundManager(this);
	_resFork = new Common::MacResManager();
	_inventoryLid = new Common::MacResManager();
	_biochipLid = new Common::MacResManager();
	_gameMode = kMainMenuMode;
	_adventureMode = true;
	
	if (!_resFork->open("JMP PP Resources") || !_resFork->hasResFork())
		error("Could not load JMP PP Resources");

	if (!_inventoryLid->open("Images/Lids/Inventory Lid Sequence") || !_inventoryLid->hasResFork())
		error("Could not open Inventory Lid Sequence");

	if (!_biochipLid->open("Images/Lids/Biochip Lid Sequence") || !_biochipLid->hasResFork())
		error("Could not open Biochip Lid Sequence");

	loadItemLocationData();

#if 0
	Common::MacResIDArray pictIds = _biochipLid->getResIDArray(MKID_BE('PICT'));
	for (uint32 i = 0; i < pictIds.size(); i++) {
		Common::String filename = Common::String::printf("PICT_%d.pict", pictIds[i]);
		Common::DumpFile file;
		assert(file.open(filename));
		Common::SeekableReadStream *res = _biochipLid->getResource(MKID_BE('PICT'), pictIds[i]);
		byte *data = new byte[res->size()];
		res->read(data, res->size());
		for (int j = 0; j < 512; j++)
			file.writeByte(0);
		file.write(data, res->size());
		file.close();
		delete res;
		delete[] data;
	}
#endif

#if defined(RUN_SUB_MOVIE)
	_video->playMovie("Images/Norad Alpha/Sub Chase Movie");
#elif defined(RUN_INTERFACE_TEST)
	drawInterface();
	_gfx->setCursor(kMainCursor);
	_sound->playSound("Sounds/Caldoria/Apartment Music.aiff", true);

	while (!shouldQuit()) {
		Common::Event event;
		// Ignore events for now
		while (_eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_MOUSEMOVE)
				_system->updateScreen();
		}
		
		_system->delayMillis(10);
	}
#else
	while (!shouldQuit()) {
		switch (_gameMode) {
		case kMainMenuMode:
			if (!isDemo())
				runIntro();

			runMainMenu();
			break;
		case kMainGameMode:
			if (isDemo())
				changeLocation(kLocPrehistoric);
			else
				changeLocation(kLocCaldoria);
			mainGameLoop();
			break;
		case kQuitMode:
			return Common::kNoError;
		default:
			_gameMode = kMainMenuMode;
			break;
		}
	}
#endif

	return Common::kNoError;
}

void PegasusEngine::loadItemLocationData() {
	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('NItm'), 0x80);

	uint16 entryCount = res->readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		ItemLocationData loc;
		loc.id = res->readUint16BE(); // Which is always == i, anyway
		loc.location = (ItemLocation)res->readUint16BE();
		loc.u0 = res->readUint16BE();
		loc.u1 = res->readByte();
		debug(1, "Item[%d]: ID = %d, location = %x, u0 = %d, u1 = %d", i, loc.id, loc.location, loc.u0, loc.u1);
		res->readByte();
		_itemLocationData.push_back(loc);
	}

	delete res;
}

void PegasusEngine::runIntro() {
	// The Opening/Closing folder will need to be renamed to something else. Windows
	// and other OS's/FS's do not support a '/' in the filename. I arbitrarily chose
	// to rename my folder with the underscore.
	_video->playMovieCentered("Images/Opening_Closing/BandaiLogo.movie");
	VideoHandle handle = _video->playBackgroundMovie("Images/Opening_Closing/Big Movie.movie");
	_video->seekToTime(handle, 10 * 600);
	_video->waitUntilMovieEnds(handle);
}

static const int kViewScreenOffset = 64;

void PegasusEngine::drawInterface() {
	_gfx->drawPict("Images/Interface/3DInterface Top", 0, 0, false);
	_gfx->drawPict("Images/Interface/3DInterface Left", 0, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/3DInterface Right", 640 - kViewScreenOffset, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/3DInterface Bottom", 0, kViewScreenOffset + 256, false);
	//drawCompass();
	_system->updateScreen();
}

void PegasusEngine::drawInterfaceOverview() {
	_gfx->drawPict("Images/Interface/OVTop.mac", 0, 0, false);
	_gfx->drawPict("Images/Interface/OVLeft.mac", 0, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVRight.mac", 640 - kViewScreenOffset, kViewScreenOffset, false);
	_gfx->drawPict("Images/Interface/OVBottom.mac", 0, kViewScreenOffset + 256, false);
	_system->updateScreen();
}

void PegasusEngine::mainGameLoop() {
	// TODO: Yeah...
	_system->fillScreen(0);
	_video->playMovieCentered("Images/Caldoria/Pullback.movie");
	drawInterface();
	if (isDemo())
		_video->playMovie("Images/Prehistoric/Prehistoric.movie", kViewScreenOffset, kViewScreenOffset);
	else
		_video->playMovie("Images/Caldoria/Caldoria.movie", kViewScreenOffset, kViewScreenOffset);
	_gameMode = kQuitMode;
}

void PegasusEngine::changeLocation(TimeZone timeZone) {
	_timeZone = timeZone;
	loadViews(_timeZone);
	//loadExits(_timeZone);
	loadDoors(_timeZone);
	//loadHSLs(_timeZone);
	//loadHSIn(_timeZone);
	loadSoundSpots(_timeZone);
	//loadTurns(_timeZone);
	loadZooms(_timeZone);
	loadExtras(_timeZone);
}

void PegasusEngine::loadViews(TimeZone timeZone) {
	_currentViews.clear();

	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('View'), getTimeZoneDesc(timeZone));

	uint32 entryCount = res->readUint32BE();

	for (uint32 i = 0; i < entryCount; i++) {
		View view;
		view.u0 = res->readUint16BE(); // Compass reading?
		view.u1 = res->readByte(); // Always 0-3, direction?
		view.u2 = res->readByte(); // Usually 0, rarely 3
		view.frameTime = res->readUint32BE();
		debug(1, "View[%d]: u0 = %d, u1 = %d, u2 = %d, time = %d", i, view.u0, view.u1, view.u2, view.frameTime);
		_currentViews.push_back(view);
	}

	delete res;
}

void PegasusEngine::loadDoors(TimeZone timeZone) {
	_currentDoors.clear();

	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('Door'), getTimeZoneDesc(timeZone));

	uint32 entryCount = res->readUint32BE();

	for (uint32 i = 0; i < entryCount; i++) {
		Door door;
		door.u0 = res->readUint16BE();
		door.u1 = res->readUint16BE(); // Always divisible by 256?
		door.startTime = res->readUint32BE();
		door.endTime = res->readUint32BE();
		door.u2 = res->readUint16BE();
		debug(1, "Door[%d]: u0 = %d, u1 = %d, startTime = %d, endTime = %d, u2 = %d", i, door.u0, door.u1, door.startTime, door.endTime, door.u2);
		_currentDoors.push_back(door);
	}

	delete res;
}

void PegasusEngine::loadSoundSpots(TimeZone timeZone) {
	_currentSoundSpots.clear();

	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('Spot'), getTimeZoneDesc(timeZone));

	uint32 entryCount = res->readUint32BE();

	for (uint32 i = 0; i < entryCount; i++) {
		SoundSpot spot;
		spot.u0 = res->readUint16BE();
		spot.u1 = res->readUint16BE();
		spot.u2 = res->readUint16BE(); // 0/1 or 768/769
		spot.startTime = res->readUint32BE();
		spot.endTime = res->readUint32BE();
		spot.u3 = res->readUint16BE();
		debug(1, "Sound Spot[%d]: u0 = %d, u1 = %d, u2 = %d, startTime = %d, endTime = %d, u3 = %d", i, spot.u0, spot.u1, spot.u2, spot.startTime, spot.endTime, spot.u3);
		_currentSoundSpots.push_back(spot);
	}

	delete res;
}

void PegasusEngine::loadZooms(TimeZone timeZone) {
	_currentZooms.clear();

	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('Zoom'), getTimeZoneDesc(timeZone));

	uint32 entryCount = res->readUint32BE();

	for (uint32 i = 0; i < entryCount; i++) {
		Zoom zoom;
		zoom.u0 = res->readUint16BE();
		zoom.u1 = res->readUint16BE();
		zoom.startTime = res->readUint32BE();
		zoom.endTime = res->readUint32BE();
		zoom.u2 = res->readUint16BE();
		debug(1, "Zoom[%d]: u0 = %d, u1 = %d, startTime = %d, endTime = %d, u2 = %d", i, zoom.u0, zoom.u1, zoom.startTime, zoom.endTime, zoom.u2);
		_currentZooms.push_back(zoom);
	}

	delete res;
}

void PegasusEngine::loadExtras(TimeZone timeZone) {
	_currentExtras.clear();

	Common::SeekableReadStream *res = _resFork->getResource(MKID_BE('Xtra'), getTimeZoneDesc(timeZone));

	uint32 entryCount = res->readUint32BE();

	for (uint32 i = 0; i < entryCount; i++) {
		Extra extra;
		extra.u0 = res->readUint32BE();
		extra.startTime = res->readUint32BE();
		extra.endTime = res->readUint32BE();
		debug(1, "Extra[%d]: u0 = %d, startTime = %d, endTime = %d", i, extra.u0, extra.startTime, extra.endTime);
		_currentExtras.push_back(extra);
	}

	delete res;
}

Common::String PegasusEngine::getTimeZoneDesc(TimeZone timeZone) {
	static const char *names[] = { "Prehistoric", "Mars", "WSC", "Tiny TSA", "Full TSA", "Norad Alpha", "Caldoria", "Norad Delta" };
	return names[timeZone];
}

} // End of namespace Pegasus
