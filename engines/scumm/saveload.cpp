/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/translation.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse/imuse.h"
#include "scumm/macgui/macgui.h"
#include "scumm/players/player_towns.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v7.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/he/sprite_he.h"
#include "scumm/verbs.h"

#include "backends/audiocd/audiocd.h"

#include "graphics/thumbnail.h"
#include "gui/message.h"

namespace Scumm {

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 ver;
	char name[32] = {};
};

struct SaveInfoSection {
	uint32 type;
	uint32 version;
	uint32 size;

	uint32 timeTValue;  // Obsolete since version 2, but kept for compatibility
	uint32 playtime;

	uint32 date;
	uint16 time;
};

#define SaveInfoSectionSize (4+4+4 + 4+4 + 4+2)

#define CURRENT_VER 124
#define INFOSECTION_VERSION 2

#pragma mark -

Common::Error ScummEngine::loadGameState(int slot) {
	requestLoad(slot);
	return Common::kNoError;
}

bool ScummEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	if (!_setupIsComplete)
		return false;

	// FIXME: For now always allow loading in V0-V3 games
	// FIXME: Actually, we might wish to support loading in more places.
	// As long as we are sure it won't cause any problems... Are we
	// aware of *any* spots where loading is not supported?

	// HE games are limited to original load and save interface only,
	// due to numerous glitches (see bug #3210) that can occur.
	//
	// Except the earliest HE Games (3DO and initial DOS version of
	// puttputt), which didn't offer scripted load/save screens.
	if (_game.heversion >= 62) {
		if (msg)
			*msg = _("This game does not support loading from the menu. Use in-game interface");

		return false;
	}

	// COMI always disables saving/loading (to tell the truth:
	// the main menu) via its scripts, thus we need to make an
	// exception here. This the same forced overwriting of the
	// script decisions as in ScummEngine::processKeyboard.
	if (_game.id == GID_CMI)
		return true;

	bool isOriginalMenuActive = isUsingOriginalGUI() && _mainMenuIsActive;

	if (_game.version <= 3) {
		int saveRoom = -1;
		int saveMenuScript = -1;
		if (_game.id == GID_MANIAC) {
			saveRoom = 50;
			if (_game.version == 0) {
				saveMenuScript = 2;
			} else {
				saveMenuScript = _game.version == 1 ? 162 : 163;
			}
		} else if (_game.id == GID_ZAK) {
			saveRoom = 50;
			saveMenuScript = (_game.version == 3) ? 169 : 7;
		} else if (_game.id == GID_INDY3) {
			saveRoom = 14;
			saveMenuScript = 9;
		} else if (_game.id == GID_LOOM) {
			saveRoom = 70;
			saveMenuScript = (_game.platform == Common::kPlatformFMTowns) ? 42 : 4;
		}

		// Also deny persistence operations while the script opening the save menu is running...
		isOriginalMenuActive = _currentRoom == saveRoom || currentScriptSlotIs(saveMenuScript);
	}

	return (VAR_MAINMENU_KEY == 0xFF || VAR(VAR_MAINMENU_KEY) != 0) && !isOriginalMenuActive;
}

Common::Error ScummEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	requestSave(slot, desc);
	return Common::kNoError;
}

bool ScummEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (!_setupIsComplete)
		return false;

	// Disallow saving in v0-v3 games when a 'prequel' to a cutscene is shown.
	// This is a blank screen with text, and while this is shown, saving should
	// be disabled, as no room is set.
	if (_game.version <= 3 && _currentScript == 0xFF && _roomResource == 0 && _currentRoom == 0)
		return false;

	// TODO: Should we disallow saving in some more places,
	// e.g. when a SAN movie is playing? Not sure whether the
	// original EXE allowed this.

	// HE games are limited to original load and save interface only,
	// due to numerous glitches (see bug #3210) that can occur.
	//
	// Except the earliest HE Games (3DO and initial DOS version of
	// puttputt), which didn't offer scripted load/save screens.
	if (_game.heversion >= 62) {
		if (msg)
			*msg = _("This game does not support saving from the menu. Use in-game interface");

		return false;
	}

#ifdef ENABLE_SCUMM_7_8
	// COMI always disables saving/loading (to tell the truth:
	// the main menu) via its scripts, thus we need to make an
	// exception here, and always enable it unless we're on the
	// original save/load screen. This the same forced overwriting
	// of the script decisions as in ScummEngine::processKeyboard.
	// Also, disable saving when a SAN video is playing.
	if (_game.version >= 7 && ((ScummEngine_v7 *)this)->isSmushActive())
		return false;

	if (_game.id == GID_CMI)
		return _currentRoom != 92;
#endif

	bool isOriginalMenuActive = isUsingOriginalGUI() && _mainMenuIsActive;

	if (_game.version <= 3) {
		int saveRoom = -1;
		int saveMenuScript = -1;
		if (_game.id == GID_MANIAC) {
			saveRoom = 50;
			if (_game.version == 0) {
				saveMenuScript = 2;
			} else {
				saveMenuScript = _game.version == 1 ? 162 : 163;
			}
		} else if (_game.id == GID_ZAK) {
			saveRoom = 50;
			saveMenuScript = (_game.version == 3) ? 169 : 7;
		} else if (_game.id == GID_INDY3) {
			saveRoom = 14;
			saveMenuScript = 9;
		} else if (_game.id == GID_LOOM) {
			saveRoom = 70;
			saveMenuScript = (_game.platform == Common::kPlatformFMTowns) ? 42 : 4;
		}

		// Also deny persistence operations while the script opening the save menu is running...
		isOriginalMenuActive = _currentRoom == saveRoom || currentScriptSlotIs(saveMenuScript);
	}

	// SCUMM v4+ doesn't allow saving in room 0 or if
	// VAR(VAR_MAINMENU_KEY) to set to zero.
	return (VAR_MAINMENU_KEY == 0xFF || (VAR(VAR_MAINMENU_KEY) != 0 && _currentRoom != 0)) && !isOriginalMenuActive;
}


void ScummEngine::requestSave(int slot, const Common::String &name) {
	_saveLoadSlot = slot;
	_saveTemporaryState = false;
	_saveLoadFlag = 1;		// 1 for save
	_saveLoadDescription = name;
}

void ScummEngine::requestLoad(int slot) {
	_saveLoadSlot = slot;
	_saveTemporaryState = (slot == 100);
	_saveLoadFlag = 2;		// 2 for load
}

static bool saveSaveGameHeader(Common::WriteStream *out, SaveGameHeader &hdr) {
	hdr.type = MKTAG('S','C','V','M');
	hdr.size = 0;
	hdr.ver = CURRENT_VER;

	out->writeUint32BE(hdr.type);
	out->writeUint32LE(hdr.size);
	out->writeUint32LE(hdr.ver);
	out->write(hdr.name, sizeof(hdr.name));
	return true;
}

static bool loadSaveGameHeader(Common::SeekableReadStream *in, SaveGameHeader &hdr) {
	hdr.type = in->readUint32BE();
	hdr.size = in->readUint32LE();
	hdr.ver = in->readUint32LE();
	in->read(hdr.name, sizeof(hdr.name));
	return !in->err() && hdr.type == MKTAG('S','C','V','M');
}

namespace {
bool loadAndCheckSaveGameHeader(Common::InSaveFile *in, int heversion, SaveGameHeader &hdr, Common::String *error = nullptr) {
	if (!loadSaveGameHeader(in, hdr)) {
		if (error) {
			*error = "Invalid savegame";
		}
		return false;
	}

	if (hdr.ver > CURRENT_VER) {
		hdr.ver = TO_LE_32(hdr.ver);
	}

	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER) {
		if (error) {
			*error = "Invalid version";
		}
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(57) && heversion >= 60) {
		if (error) {
			*error = "Unsupported version";
		}
		return false;
	}

	hdr.name[sizeof(hdr.name) - 1] = 0;
	return true;
}
} // End of anonymous namespace

void ScummEngine::copyHeapSaveGameToFile(int slot, const char *saveName) {
	Common::String fileName;
	SaveGameHeader hdr;
	bool saveFailed = false;

	Common::SeekableReadStream *heapSaveFile = openSaveFileForReading(1, true, fileName);
	saveFailed = !loadAndCheckSaveGameHeader(heapSaveFile, _game.heversion, hdr);

	Common::WriteStream *saveFile = openSaveFileForWriting(slot, false, fileName);
	if (!saveFile) {
		saveFailed = true;
	} else {
		Common::String temp = Common::U32String(saveName,  getDialogCodePage()).encode(Common::kUtf8);
		Common::strlcpy(hdr.name, temp.c_str(), sizeof(hdr.name));
		saveSaveGameHeader(saveFile, hdr);

		heapSaveFile->seek(sizeof(hdr), SEEK_SET);
		while (!heapSaveFile->eos()) {
			byte b = heapSaveFile->readByte();
			saveFile->writeByte(b);
		}

		saveFile->finalize();
		if (saveFile->err())
			saveFailed = true;

		delete saveFile;
	}

	delete heapSaveFile;

	if (saveFailed)
		debug(1, "State save as '%s' FAILED", fileName.c_str());
	else
		debug(1, "State saved as '%s'", fileName.c_str());
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::stampShotEnqueue(int slot, int boxX, int boxY, int boxWidth, int boxHeight, int brightness) {
	if (_stampShotsInQueue >= (int)ARRAYSIZE(_stampShots))
		error("ScummEngine_v8::stampShotEnqueue(): overflow in the queue");

	_stampShots[_stampShotsInQueue].slot = slot;
	_stampShots[_stampShotsInQueue].boxX = boxX;
	_stampShots[_stampShotsInQueue].boxY = boxY;
	_stampShots[_stampShotsInQueue].boxWidth = boxWidth;
	_stampShots[_stampShotsInQueue].boxHeight = boxHeight;
	_stampShots[_stampShotsInQueue].brightness = brightness;
	_stampShotsInQueue++;
}

void ScummEngine_v8::stampShotDequeue() {
	for (int i = 0; i < _stampShotsInQueue; i++) {
		stampScreenShot(
			_stampShots[i].slot,
			_stampShots[i].boxX,
			_stampShots[i].boxY,
			_stampShots[i].boxWidth,
			_stampShots[i].boxHeight,
			_stampShots[i].brightness);
	}

	_stampShotsInQueue = 0;
}

void ScummEngine_v8::stampScreenShot(int slot, int boxX, int boxY, int boxWidth, int boxHeight, int brightness) {
	int pixelX, pixelY;
	int color, pixelColor, rgb;
	int heightSlice, widthSlice;

	bool foundInternalThumbnail = false;
	byte tmpPalette[256];
	uint32 *thumbSurface = nullptr;

	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	foundInternalThumbnail = fetchInternalSaveStateThumbnail(slot == 0 ? 1 : slot, slot == 0);

	if (foundInternalThumbnail) {
		for (int i = 0; i < 256; i++) {
			rgb = _savegameThumbnailV8Palette[i];
			tmpPalette[i] = remapPaletteColor(
				brightness * ((rgb & 0xFF)     >> 0)  / 0xFF,
				brightness * ((rgb & 0xFF00)   >> 8)  / 0xFF,
				brightness * ((rgb & 0xFF0000) >> 16) / 0xFF,
				-1);
		}
	} else {
		// The savegame does not contain an internal SCUMM v8 thumbnail: fetch the default ScummVM one,
		// and process it with the brightness parameter beforehand...
		thumbSurface = fetchScummVMSaveStateThumbnail(slot == 0 ? 1 : slot, slot == 0, brightness);

		// Fallback: this is a savegame which does not have any of the two possible,
		// thumbnails so let's just show a brownish box which looks nice enough
		// superimposed on the Captain's log yellowish background...
		// This is some kind of last resort fallback. We shouldn't arrive here,
		// but still, better safe than sorry... :-)
		if (!thumbSurface) {
			rgb = 0x001627;
			color = remapPaletteColor(
				brightness * ((rgb & 0xFF) >> 0) / 0xFF,
				brightness * ((rgb & 0xFF00) >> 8) / 0xFF,
				brightness * ((rgb & 0xFF0000) >> 16) / 0xFF,
				-1);

			// The -1 after boxHeight is done to compensate for the fact that
			// we can't directly control the back and front buffers (see below)
			drawBox(boxX, boxY, boxWidth, boxHeight - 1, color);
			return;
		}
	}

	// If we got here, it means we managed to fetch one of the
	// thumbnails, so let's actually draw it to screen!
	heightSlice = 0;
	for (int i = 0; i < boxHeight; i++) {
		pixelY = boxY + i;
		widthSlice = 0;
		for (int j = 0; j < boxWidth; j++) {
			pixelX = j + boxX;

			// Remember, the internal one is paletted, while the ScummVM one
			// is blitted without going through a palette index...
			if (foundInternalThumbnail) {
				color = _savegameThumbnailV8[160 * (heightSlice / boxHeight) + (widthSlice / boxWidth)];
				pixelColor = tmpPalette[color];
			} else {
				pixelColor = thumbSurface[160 * (heightSlice / boxHeight) + (widthSlice / boxWidth)];
			}

			// Draw twice; once in the frontbuffer, once in the backbuffer:
			// this ensures that the lowest row of the image doesn't get overwritten
			// by the blastText rect just below, containing the savegame name...
			drawPixel(vs, pixelX, pixelY, pixelColor, false);
			drawPixel(vs, pixelX, pixelY, pixelColor, true);

			widthSlice += 160;
		}
		heightSlice += 120;
	}

	if (thumbSurface)
		delete[] thumbSurface;
}

void ScummEngine_v8::createInternalSaveStateThumbnail() {
	byte *tempBitmap = (byte *)malloc(_screenWidth * _screenHeight * sizeof(byte));
	VirtScreen *vs = &_virtscr[kMainVirtScreen];


	byte *screen = vs->getPixels(0, _screenTop);

	if (tempBitmap) {
		for (int i = 0; i < _screenHeight; i++) {
			screen = vs->getPixels(0, _screenTop + i);
			memcpy(&tempBitmap[_screenWidth * i], screen, _screenWidth * sizeof(byte));
		}

		for (int i = 0; i < 256; i++) {
			_savegameThumbnailV8Palette[i] = getPackedRGBColorFromPalette(_currentPalette, i);
		}

		for (int i = 0; i < 120; i++) {
			for (int j = 0; j < 160; j++) {
				_savegameThumbnailV8[i * 160 + j] = tempBitmap[4 * (i * _screenWidth + j)];
			}
		}

		free(tempBitmap);
	}
}

bool ScummEngine_v8::fetchInternalSaveStateThumbnail(int slotId, bool isHeapSave) {
	SaveGameHeader hdr;
	Common::String filename;
	Common::SeekableReadStream *in = openSaveFileForReading(slotId, isHeapSave, filename);
	if (!in)
		return false;

	// In order to fetch the internal COMI thumbnail, we perform the same routine
	// used during normal loading, stripped down to support only version 106 onwards...
	if (!loadAndCheckSaveGameHeader(in, _game.heversion, hdr)) {
		delete in;
		return false;
	}

	if (hdr.ver > 0xFFFFFF)
		hdr.ver = SWAP_BYTES_32(hdr.ver);

	// Reject save games which do not contain the internal thumbnail...
	if (hdr.ver < VER(106)) {
		delete in;
		return false;
	}

	Graphics::skipThumbnail(*in);

	SaveStateMetaInfos infos;
	if (!loadInfos(in, &infos)) {
		warning("Info section could not be found");
		delete in;
		return false;
	}

	hdr.name[sizeof(hdr.name) - 1] = 0;
	_saveLoadDescription = hdr.name;


	// Now do the actual loading
	Common::Serializer ser(in, nullptr);
	ser.setVersion(hdr.ver);
	ser.syncArray(_savegameThumbnailV8, 19200, Common::Serializer::Byte, VER(106));
	ser.syncArray(_savegameThumbnailV8Palette, 256, Common::Serializer::Uint32LE, VER(106));

	delete in;
	return true;
}

uint32 *ScummEngine_v8::fetchScummVMSaveStateThumbnail(int slotId, bool isHeapSave, int brightness) {
	Common::String filename;
	Graphics::Surface *thumbnailSurface;

	// Perform the necessary steps to arrive at the thumbnail section of the save file...
	Common::SeekableReadStream *in = openSaveFileForReading(slotId, isHeapSave, filename);
	if (in) {
		// We don't perform checks on the header: if we're here it means that the
		// savestate follows the correct format and it is loadable.
		in->skip(sizeof(uint32) * 3 + sizeof(SaveGameHeader::name));

		// Load the thumbnail.
		// We're under the assumption that its resolution will always be 160x120,
		// which is a fourth of the original 640x480 internal resolution, so there's
		// no need to scale the surface.
		bool thumbSuccess = Graphics::loadThumbnail(*in, thumbnailSurface);
		delete in;

		if (thumbSuccess) {
			// Now take the pixels from the surface, extract the RGB components, process them
			// with the brightness parameter, and store them in an appropriate structure
			// which the SCUMM graphics pipeline can use...
			byte r = 0, g = 0, b = 0;
			byte bpp = thumbnailSurface->format.bpp();
			uint32 *processedThumbnail = new uint32[thumbnailSurface->w * thumbnailSurface->h];
			for (int i = 0; i < thumbnailSurface->h; i++) {
				for (int j = 0; j < thumbnailSurface->w; j++) {
					if (bpp == 32) {
						uint32 *ptr = (uint32 *)thumbnailSurface->getBasePtr(j, i);
						thumbnailSurface->format.colorToRGB(*ptr, r, g, b);
					} else if (bpp == 16) {
						uint16 *ptr = (uint16 *)thumbnailSurface->getBasePtr(j, i);
						thumbnailSurface->format.colorToRGB(*ptr, r, g, b);
					} else if (bpp == 8) {
						uint8 *ptr = (uint8 *)thumbnailSurface->getBasePtr(j, i);
						thumbnailSurface->format.colorToRGB(*ptr, r, g, b);
					}

					processedThumbnail[i * thumbnailSurface->w + j] = getPaletteColorFromRGB(
						_currentPalette,
						brightness * r / 0xFF,
						brightness * g / 0xFF,
						brightness * b / 0xFF);
				}
			}

			thumbnailSurface->free();
			delete thumbnailSurface;
			return processedThumbnail;
		}
	}

	return nullptr;
}
#endif

Common::SeekableReadStream *ScummEngine::openSaveFileForReading(int slot, bool compat, Common::String &fileName) {
	fileName = makeSavegameName(slot, compat);
	return _saveFileMan->openForLoading(fileName);
}

Common::SeekableWriteStream *ScummEngine::openSaveFileForWriting(int slot, bool compat, Common::String &fileName) {
	fileName = makeSavegameName(slot, compat);
	return _saveFileMan->openForSaving(fileName);
}

bool ScummEngine::saveState(Common::SeekableWriteStream *out, bool writeHeader) {
	SaveGameHeader hdr;

	if (writeHeader) {
		Common::strlcpy(hdr.name, _saveLoadDescription.c_str(), sizeof(hdr.name));
		saveSaveGameHeader(out, hdr);
	}
#if !defined(__DS__) && !defined(__N64__)
	if (isUsingOriginalGUI() && _mainMenuIsActive) {
		Graphics::saveThumbnail(*out, _savegameThumbnail);
	} else {
		Graphics::saveThumbnail(*out);
	}
#endif
	saveInfos(out);

	Serializer ser(nullptr, out);
	ser.setVersion(CURRENT_VER);
	saveLoadWithSerializer(ser);
	return true;
}

bool ScummEngine::saveState(int slot, bool compat, Common::String &filename) {
	bool saveFailed = false;

	if (_game.heversion != 0)
		_sound->stopAllSounds();

	// We can't just use _saveTemporaryState here, because at
	// this point it might not contain an updated value.
	_pauseSoundsDuringSave = !compat;

	PauseToken pt = pauseEngine();

	_pauseSoundsDuringSave = true;

	Common::SeekableWriteStream *out = openSaveFileForWriting(slot, compat, filename);
	if (!out) {
		saveFailed = true;
	} else {
		if (!saveState(out))
			saveFailed = true;

		out->finalize();
		if (out->err())
			saveFailed = true;
		delete out;
	}

	if (saveFailed)
		debug(1, "State save as '%s' FAILED", filename.c_str());
	else
		debug(1, "State saved as '%s'", filename.c_str());

	return !saveFailed;
}

bool ScummEngine::loadState(int slot, bool compat) {
	// Wrapper around the other variant
	Common::String filename;
	return loadState(slot, compat, filename);
}

bool ScummEngine::loadState(int slot, bool compat, Common::String &filename) {
	SaveGameHeader hdr;
	int sb, sh;

	Common::SeekableReadStream *in = openSaveFileForReading(slot, compat, filename);
	if (!in)
		return false;

	if (!loadSaveGameHeader(in, hdr)) {
		warning("Invalid savegame '%s'", filename.c_str());
		delete in;
		return false;
	}

	// In older versions of ScummVM, the header version was not endian safe.
	// We account for that by retrying once with swapped byte order in case
	// we see a version that is higher than anything we'd expect...
	if (hdr.ver > 0xFFFFFF)
		hdr.ver = SWAP_BYTES_32(hdr.ver);

	// Reject save games which are too old or too new. Note that
	// We do not really support V7 games, but still accept them here
	// to work around a bug from the stone age (see below for more
	// information).
	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER) {
		warning("Invalid version of '%s'", filename.c_str());
		delete in;
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(50) && _game.heversion >= 71) {
		warning("Unsupported version of '%s'", filename.c_str());
		delete in;
		return false;
	}

	// Since version 52 a thumbnail is saved directly after the header.
	if (hdr.ver >= VER(52)) {
		// Prior to version 75 we always required a thumbnail to be present
		if (hdr.ver <= VER(74)) {
			if (!Graphics::checkThumbnailHeader(*in)) {
				warning("Can not load thumbnail");
				delete in;
				return false;
			}
		}

		Graphics::skipThumbnail(*in);
	}

	// Since version 56 we save additional information about the creation of
	// the save game and the save time.
	if (hdr.ver >= VER(56)) {
		SaveStateMetaInfos infos;
		if (!loadInfos(in, &infos)) {
			warning("Info section could not be found");
			delete in;
			return false;
		}

		setTotalPlayTime(infos.playtime * 1000);
	} else {
		// start time counting
		setTotalPlayTime();
	}

	// Due to a bug in scummvm up to and including 0.3.0, save games could be saved
	// in the V8/V9 format but were tagged with a V7 mark. Ouch. So we just pretend V7 == V8 here
	if (hdr.ver == VER(7))
		hdr.ver = VER(8);

	hdr.name[sizeof(hdr.name) - 1] = 0;
	_saveLoadDescription = hdr.name;

	// Set to 0 during load to minimize stuttering
	if (_musicEngine && !compat)
		_musicEngine->setMusicVolume(0);

	// Unless specifically requested with _saveSound, we do not save the iMUSE
	// state for temporary state saves - such as certain cutscenes in DOTT,
	// FOA, Sam and Max, etc.
	//
	// Thus, we should probably not stop music when restoring from one of
	// these saves. This change stops the Mole Man theme from going quiet in
	// Sam & Max when Doug tells you about the Ball of Twine, as mentioned in
	// patch #8316.
	//
	// If we don't have iMUSE at all we may as well stop the sounds. The previous
	// default behavior here was to stopAllSounds on all state restores.
	//
	// HE games explicitly do not stop sounds when loading a "heap save",
	// under version 80!

	if ((!_imuse || _saveSound || !compat) &&
		!(compat && _game.heversion < 80)) {
		_sound->stopAllSounds();
	} else if (compat && !_imuseDigital && _game.heversion == 0) {
		// Still, we have to stop the talking sound even
		// if the save state is temporary.
		_sound->stopTalkSound();
	}

#ifdef ENABLE_SCUMM_7_8
	if (_imuseDigital) {
		_imuseDigital->stopAllSounds();
	}
#endif

	_sound->stopCD();

	if (!_saveTemporaryState)
		_sound->pauseSounds(true);

	closeRoom();

	memset(_inventory, 0, sizeof(_inventory[0]) * _numInventory);
	memset(_newNames, 0, sizeof(_newNames[0]) * _numNewNames);

	// Because old savegames won't fill the entire gfxUsageBits[] array,
	// clear it here just to be sure it won't hold any unforeseen garbage.
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	// Nuke all resources
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1))
		if (type != rtTemp && type != rtBuffer && (type != rtSound || _saveSound || !compat))
			for (ResId idx = 0; idx < _res->_types[type].size(); idx++) {
				_res->nukeResource(type, idx);
			}

	resetScummVars();

	if (_game.features & GF_OLD_BUNDLE)
		loadCharset(0); // FIXME - HACK ?

	//
	// Now do the actual loading
	//
	Serializer ser(in, nullptr);
	ser.setVersion(hdr.ver);
	saveLoadWithSerializer(ser);
	delete in;

	// Init NES costume data
	if (_game.platform == Common::kPlatformNES) {
		if (hdr.ver < VER(47))
			_NESCostumeSet = 0;
		NES_loadCostumeSet(_NESCostumeSet);
	}

	// Normally, _vm->_screenTop should always be >= 0, but for some old save games
	// it is not, hence we check & correct it here.
	if (_screenTop < 0)
		_screenTop = 0;

	// WORKAROUND bug #1191: For unknown reasons, object 819 sometimes is in
	// state 1 in old save games, implying it should be drawn. This in turn
	// results in a crash when entering the church, as object 819 is part of the
	// exitof the church and there are no graphics assigned to it.
	if (_game.id == GID_MONKEY_VGA) {
		putState(819, 0);
	}

	if (hdr.ver < VER(33) && _game.version >= 7) {
		// For a long time, we didn't set these vars to default values.
		VAR(VAR_DEFAULT_TALK_DELAY) = 60;
		if (_game.version == 7)
			VAR(VAR_NUM_GLOBAL_OBJS) = _numGlobalObjects - 1;
	}

	if (hdr.ver < VER(30)) {
		// For a long time, we used incorrect location, causing it to default to zero.
		if (_game.version == 8)
			_scummVars[VAR_CHARINC] = (_game.features & GF_DEMO) ? 3 : 1;
		// Needed due to subtitle speed changes
		_defaultTextSpeed /= 20;
	}

	// For a long time, we used incorrect locations for some camera related
	// scumm vars. We now know the proper locations. To be able to properly use
	// old save games, we update the old (bad) variables to the new (correct)
	// ones.
	if (hdr.ver < VER(28) && _game.version == 8) {
		_scummVars[VAR_CAMERA_MIN_X] = _scummVars[101];
		_scummVars[VAR_CAMERA_MAX_X] = _scummVars[102];
		_scummVars[VAR_CAMERA_MIN_Y] = _scummVars[103];
		_scummVars[VAR_CAMERA_MAX_Y] = _scummVars[104];
		_scummVars[VAR_CAMERA_THRESHOLD_X] = _scummVars[105];
		_scummVars[VAR_CAMERA_THRESHOLD_Y] = _scummVars[106];
		_scummVars[VAR_CAMERA_SPEED_X] = _scummVars[107];
		_scummVars[VAR_CAMERA_SPEED_Y] = _scummVars[108];
		_scummVars[VAR_CAMERA_ACCEL_X] = _scummVars[109];
		_scummVars[VAR_CAMERA_ACCEL_Y] = _scummVars[110];
	}

	// For a long time, we used incorrect values for some camera related
	// scumm vars. We now know the proper values. To be able to properly use
	// old save games, we update the old (bad) values to the new (correct)
	// ones.
	if (hdr.ver < VER(77) && _game.version >= 7) {
		_scummVars[VAR_CAMERA_THRESHOLD_X] = 100;
		_scummVars[VAR_CAMERA_THRESHOLD_Y] = 70;
		_scummVars[VAR_CAMERA_ACCEL_X] = 100;
		_scummVars[VAR_CAMERA_ACCEL_Y] = 100;
	}

	// With version 22, we replaced the scale items with scale slots. So when
	// loading such an old save game, try to upgrade the old to new format.
	if (hdr.ver < VER(22)) {
		// Convert all rtScaleTable resources to matching scale items
		for (ResId idx = 1; idx < _res->_types[rtScaleTable].size(); idx++) {
			convertScaleTableToScaleSlot(idx);
		}
	}

	// Reset the palette.
	resetPalette();

	if (hdr.ver < VER(35) && _game.id == GID_MANIAC && _game.version <= 1)
		resetV1ActorTalkColor();

	// Load the static room data
	setupRoomSubBlocks();

	if (_game.version < 7) {
		camera._last.x = camera._cur.x;
	}

	sb = _screenB;
	sh = _screenH;

#ifdef ENABLE_SCUMM_7_8
	// Remove any blastText/blastObject leftovers
	if (_game.version >= 7) {
		((ScummEngine_v6 *)this)->removeBlastObjects();
		((ScummEngine_v7 *)this)->removeBlastTexts();
	}

	if (_game.version == 8 && isUsingOriginalGUI()) {
		// If we are loading a savegame from the ScummVM launcher these two
		// variables are going to be unassigned, since the game does not save these
		((ScummEngine_v8 *)this)->setKeyScriptVars(0x13B, 0x1C0);
	}
#endif

	// Restore the virtual screens and force a fade to black.
	initScreens(0, _screenHeight);

	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	vs->setDirtyRange(0, vs->h);
	updateDirtyScreen(kMainVirtScreen);
	updatePalette();

	initScreens(sb, sh);

	_completeScreenRedraw = true;

	// Reset charset mask
	_charset->_hasMask = false;
	if (_macScreen)
		_macScreen->fillRect(Common::Rect(_macScreen->w, _macScreen->h), 0);
	clearTextSurface();

	if (_macGui)
		_macGui->resetAfterLoad();

	_lastCodePtr = nullptr;
	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

	cameraMoved();

	initBGBuffers(_roomHeight);

	if (VAR_ROOM_FLAG != 0xFF)
		VAR(VAR_ROOM_FLAG) = 1;

	// Sync with current config setting
	if (VAR_VOICE_MODE != 0xFF)
		VAR(VAR_VOICE_MODE) = ConfMan.getBool("subtitles");

	debug(1, "State loaded from '%s'", filename.c_str());

	_sound->pauseSounds(false);

	_sound->restoreAfterLoad();

	return true;
}

Common::String ScummEngine::makeSavegameName(const Common::String &target, int slot, bool temporary) {
	Common::String extension;
	extension = Common::String::format(".%c%02d", temporary ? 'c' : 's', slot);
	return target + extension;
}

void ScummEngine::listSavegames(bool *marks, int num) {
	assert(marks);

	char slot[3];
	int slotNum;
	Common::StringArray files;

	Common::String prefix = makeSavegameName(99, false);
	prefix.setChar('*', prefix.size() - 2);
	prefix.setChar(0, prefix.size() - 1);
	memset(marks, false, num * sizeof(bool));	// Assume no savegames for this title
	files = _saveFileMan->listSavefiles(prefix);

	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		slot[0] = file->c_str()[file->size() - 2];
		slot[1] = file->c_str()[file->size() - 1];
		slot[2] = 0;

		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum < num)
			marks[slotNum] = true;	// Mark this slot as valid
	}
}

bool getSavegameName(Common::InSaveFile *in, Common::String &desc, int heversion);

bool ScummEngine::getSavegameName(int slot, Common::String &desc) {
	Common::InSaveFile *in = nullptr;
	bool result = false;

	desc.clear();
	Common::String filename = makeSavegameName(slot, false);
	in = _saveFileMan->openForLoading(filename);
	if (in) {
		result = Scumm::getSavegameName(in, desc, _game.heversion);
		delete in;
	}

	Common::U32String temp(desc.c_str(), Common::kUtf8);
	desc = temp.encode(getDialogCodePage());

	return result;
}

bool getSavegameName(Common::InSaveFile *in, Common::String &desc, int heversion) {
	SaveGameHeader hdr;

	if (!loadAndCheckSaveGameHeader(in, heversion, hdr, &desc)) {
		return false;
	}

	desc = hdr.name;
	return true;
}

bool ScummEngine::querySaveMetaInfos(const char *target, int slot, int heversion, Common::String &desc, Graphics::Surface *&thumbnail, SaveStateMetaInfos *&timeInfos) {
	if (slot < 0) {
		return false;
	}

	SaveGameHeader hdr;
	const Common::String filename = ScummEngine::makeSavegameName(target, slot, false);
	Common::ScopedPtr<Common::SeekableReadStream> in(g_system->getSavefileManager()->openForLoading(filename));

	if (!in) {
		return false;
	}

	if (!loadAndCheckSaveGameHeader(in.get(), heversion, hdr)) {
		return false;
	}

	desc = hdr.name;

	if (hdr.ver > VER(52)) {
		if (Graphics::checkThumbnailHeader(*in)) {
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				return false;
			}
		}

		if (hdr.ver > VER(57)) {
			if (!loadInfos(in.get(), timeInfos)) {
				return false;
			}
		} else {
			timeInfos = nullptr;
		}
	}

	return true;
}

bool ScummEngine::loadInfos(Common::SeekableReadStream *file, SaveStateMetaInfos *stuff) {
	memset(stuff, 0, sizeof(SaveStateMetaInfos));

	SaveInfoSection section;
	section.type = file->readUint32BE();
	if (section.type != MKTAG('I','N','F','O')) {
		return false;
	}

	section.version = file->readUint32BE();
	section.size = file->readUint32BE();

	// If we ever extend this we should add a table containing the sizes corresponding to each
	// version, so that we are able to properly verify their correctness.
	if (section.version == INFOSECTION_VERSION && section.size != SaveInfoSectionSize) {
		warning("Info section is corrupt");
		file->skip(section.size);
		return false;
	}

	section.timeTValue = file->readUint32BE();
	section.playtime = file->readUint32BE();

	// For header version 1, we load the data in with our old method
	if (section.version == 1) {
		//time_t tmp = section.timeTValue;
		//tm *curTime = localtime(&tmp);
		//stuff->date = (curTime->tm_mday & 0xFF) << 24 | ((curTime->tm_mon + 1) & 0xFF) << 16 | (curTime->tm_year + 1900) & 0xFFFF;
		//stuff->time = (curTime->tm_hour & 0xFF) << 8 | (curTime->tm_min) & 0xFF;
		stuff->date = 0;
		stuff->time = 0;
	}

	if (section.version >= 2) {
		section.date = file->readUint32BE();
		section.time = file->readUint16BE();

		stuff->date = section.date;
		stuff->time = section.time;
	}

	stuff->playtime = section.playtime;

	// Skip over the remaining (unsupported) data
	if (section.size > SaveInfoSectionSize)
		file->skip(section.size - SaveInfoSectionSize);

	return true;
}

void ScummEngine::saveInfos(Common::WriteStream *file) {
	SaveInfoSection section;
	section.type = MKTAG('I','N','F','O');
	section.version = INFOSECTION_VERSION;
	section.size = SaveInfoSectionSize;

	// still save old format for older versions
	section.timeTValue = 0;
	section.playtime = getTotalPlayTime() / 1000;

	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	section.date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	section.time = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);

	file->writeUint32BE(section.type);
	file->writeUint32BE(section.version);
	file->writeUint32BE(section.size);
	file->writeUint32BE(section.timeTValue);
	file->writeUint32BE(section.playtime);
	file->writeUint32BE(section.date);
	file->writeUint16BE(section.time);
}

static void syncWithSerializer(Common::Serializer &s, ObjectData &od) {
	s.syncAsUint32LE(od.OBIMoffset, VER(8));
	s.syncAsUint32LE(od.OBCDoffset, VER(8));
	s.syncAsUint16LE(od.walk_x, VER(8));
	s.syncAsUint16LE(od.walk_y, VER(8));
	s.syncAsUint16LE(od.obj_nr, VER(8));
	s.syncAsSint16LE(od.x_pos, VER(8));
	s.syncAsSint16LE(od.y_pos, VER(8));
	s.syncAsUint16LE(od.width, VER(8));
	s.syncAsUint16LE(od.height, VER(8));
	s.syncAsByte(od.actordir, VER(8));
	s.syncAsByte(od.parentstate, VER(8));
	s.syncAsByte(od.parent, VER(8));
	s.syncAsByte(od.state, VER(8));
	s.syncAsByte(od.fl_object_index, VER(8));
	s.syncAsByte(od.flags, VER(46));
}

static void syncWithSerializer(Common::Serializer &s, VerbSlot &vs, bool isV7orISR) {
	s.syncAsSint16LE(!isV7orISR ? vs.curRect.left : vs.origLeft, VER(8));
	s.syncAsSint16LE(vs.curRect.top, VER(8));
	s.syncAsSint16LE(vs.curRect.right, VER(8));
	s.syncAsSint16LE(vs.curRect.bottom, VER(8));
	s.syncAsSint16LE(vs.oldRect.left, VER(8));
	s.syncAsSint16LE(vs.oldRect.top, VER(8));
	s.syncAsSint16LE(vs.oldRect.right, VER(8));
	s.syncAsSint16LE(vs.oldRect.bottom, VER(8));
	s.syncAsByte(vs.verbid, VER(8), VER(11));
	s.syncAsSint16LE(vs.verbid, VER(12));
	s.syncAsByte(vs.color, VER(8));
	s.syncAsByte(vs.hicolor, VER(8));
	s.syncAsByte(vs.dimcolor, VER(8));
	s.syncAsByte(vs.bkcolor, VER(8));
	s.syncAsByte(vs.type, VER(8));
	s.syncAsByte(vs.charset_nr, VER(8));
	s.syncAsByte(vs.curmode, VER(8));
	s.syncAsByte(vs.saveid, VER(8));
	s.syncAsByte(vs.key, VER(8));
	s.syncAsByte(vs.center, VER(8));
	s.syncAsByte(vs.prep, VER(8));
	s.syncAsUint16LE(vs.imgindex, VER(8));
	if (isV7orISR && s.isLoading() && s.getVersion() >= 8)
		vs.curRect.left = vs.origLeft;
}

static void syncWithSerializerDef(Common::Serializer &s, VerbSlot &vs) {
	syncWithSerializer(s, vs, false);
}

static void syncWithSerializerV7orISR(Common::Serializer &s, VerbSlot &vs) {
	syncWithSerializer(s, vs, true);
}

static void syncWithSerializer(Common::Serializer &s, ScriptSlot &ss) {
	s.syncAsUint32LE(ss.offs, VER(8));
	s.syncAsSint32LE(ss.delay, VER(8));
	s.syncAsUint16LE(ss.number, VER(8));
	s.syncAsUint16LE(ss.delayFrameCount, VER(8));
	s.syncAsByte(ss.status, VER(8));
	s.syncAsByte(ss.where, VER(8));
	s.syncAsByte(ss.freezeResistant, VER(8));
	s.syncAsByte(ss.recursive, VER(8));
	s.syncAsByte(ss.freezeCount, VER(8));
	s.syncAsByte(ss.didexec, VER(8));
	s.syncAsByte(ss.cutsceneOverride, VER(8));
	s.syncAsByte(ss.cycle, VER(46));
	s.skip(1, VER(8), VER(10)); // unk5
}

static void syncWithSerializer(Common::Serializer &s, NestedScript &ns) {
	s.syncAsUint16LE(ns.number, VER(8));
	s.syncAsByte(ns.where, VER(8));
	s.syncAsByte(ns.slot, VER(8));
}

static void syncWithSerializer(Common::Serializer &s, SentenceTab &st) {
	s.syncAsByte(st.verb, VER(8));
	s.syncAsByte(st.preposition, VER(8));
	s.syncAsUint16LE(st.objectA, VER(8));
	s.syncAsUint16LE(st.objectB, VER(8));
	s.syncAsByte(st.freezeCount, VER(8));
}

static void syncWithSerializer(Common::Serializer &s, StringTab &st) {
	s.syncAsSint16LE(st.xpos, VER(8));
	s.syncAsSint16LE(st._default.xpos, VER(8));
	s.syncAsSint16LE(st.ypos, VER(8));
	s.syncAsSint16LE(st._default.ypos, VER(8));
	s.syncAsSint16LE(st.right, VER(8));
	s.syncAsSint16LE(st._default.right, VER(8));
	s.syncAsSByte(st.color, VER(8));
	s.syncAsSByte(st._default.color, VER(8));
	s.syncAsSByte(st.charset, VER(8));
	s.syncAsSByte(st._default.charset, VER(8));
	s.syncAsByte(st.center, VER(8));
	s.syncAsByte(st._default.center, VER(8));
	s.syncAsByte(st.overhead, VER(8));
	s.syncAsByte(st._default.overhead, VER(8));
	s.syncAsByte(st.no_talk_anim, VER(8));
	s.syncAsByte(st._default.no_talk_anim, VER(8));
	s.syncAsByte(st.wrapping, VER(71));
	s.syncAsByte(st._default.wrapping, VER(71));
}

static void syncWithSerializer(Common::Serializer &s, ColorCycle &cc) {
	s.syncAsUint16LE(cc.delay, VER(8));
	s.syncAsUint16LE(cc.counter, VER(8));
	s.syncAsUint16LE(cc.flags, VER(8));
	s.syncAsByte(cc.start, VER(8));
	s.syncAsByte(cc.end, VER(8));
}

void syncWithSerializer(Common::Serializer &s, ScummEngine::ScaleSlot &ss) {
	s.syncAsUint16LE(ss.x1, VER(13));
	s.syncAsUint16LE(ss.y1, VER(13));
	s.syncAsUint16LE(ss.scale1, VER(13));
	s.syncAsUint16LE(ss.x2, VER(13));
	s.syncAsUint16LE(ss.y2, VER(13));
	s.syncAsUint16LE(ss.scale2, VER(13));
}

static void syncWithSerializer(Common::Serializer &s, AudioCDManager::Status &as) {
	s.syncAsUint32LE(as.playing, VER(24));
	s.syncAsSint32LE(as.track, VER(24));
	s.syncAsUint32LE(as.start, VER(24));
	s.syncAsUint32LE(as.duration, VER(24));
	s.syncAsSint32LE(as.numLoops, VER(24));
}

static void syncWithSerializer(Common::Serializer &s, Common::Rect &rect) {
	s.syncAsSint16LE(rect.left);
	s.syncAsSint16LE(rect.top);
	s.syncAsSint16LE(rect.right);
	s.syncAsSint16LE(rect.bottom);
}

template <typename T, size_t N, size_t M>
static void sync2DArray(Common::Serializer &s, T (&array)[N][M], const size_t dim1, const size_t dim2, void (*serializer)(Common::Serializer &, T &), Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion) {

	if (s.getVersion() < minVersion || s.getVersion() > maxVersion) {
		return;
	}

	assert(dim1 <= N);
	assert(dim2 <= M);
	for (size_t i = 0; i < dim1; ++i) {
		for (size_t j = 0; j < dim2; ++j) {
			serializer(s, array[i][j]);
		}
	}
}

bool ScummEngine::changeSavegameName(int slot, char *newName) {
	Common::String filename;
	SaveGameHeader hdr;

	// In order to do this, we're going to:
	// - Open the savegame file;
	// - Load its header and check if there's a necessity to change the name or not;
	// - Construct a new header;
	// - Build a buffer with the remaining data of the savestate and then close the input:
	//   stream: this is done since we are not copying data from one file to another, but we
	//   are performing an intervention on a single file;
	// - Open the output stream for the same file;
	// - Save the new header and then pour the data buffer in the stream;
	// - Finalize the stream.

	Common::SeekableReadStream *in = openSaveFileForReading(slot, false, filename);

	if (!in) {
		warning("ScummEngine::changeSavegameName(): Could not open savegame '%s', aborting...", filename.c_str());
		return false;
	}

	if (!loadSaveGameHeader(in, hdr)) {
		warning("ScummEngine::changeSavegameName(): Invalid savegame '%s', aborting...", filename.c_str());
		delete in;
		return false;
	}

	if (!scumm_strnicmp(newName, hdr.name, sizeof(hdr.name))) {
		// No name to change, abort...
		delete in;
		return true;
	}

	Common::strlcpy(hdr.name, newName, sizeof(hdr.name));

	size_t bufferSizeNoHdr = in->size() - sizeof(hdr);
	byte *saveBuffer = (byte *)malloc(bufferSizeNoHdr * sizeof(byte));

	if (!saveBuffer) {
		warning("ScummEngine::changeSavegameName(): Couldn't create save buffer, aborting...");
		delete in;
		return false;
	}

	in->seek(sizeof(hdr), SEEK_SET);

	for (uint i = 0; i < (uint)bufferSizeNoHdr; i++) {
		saveBuffer[i] = in->readByte();

		if (in->err()) {
			warning("ScummEngine::changeSavegameName(): Error in input file stream, aborting...");
			delete in;
			free(saveBuffer);
			return false;
		}
	}

	delete in;

	Common::WriteStream *out = openSaveFileForWriting(slot, false, filename);

	if (!out) {
		warning("ScummEngine::changeSavegameName(): Couldn't open output file, aborting...");
		free(saveBuffer);
		return false;
	}

	saveSaveGameHeader(out, hdr);

	for (uint i = 0; i < (uint)bufferSizeNoHdr; i++) {
		out->writeByte(saveBuffer[i]);

		if (out->err()) {
			warning("ScummEngine::changeSavegameName(): Error in output file stream, aborting...");
			free(saveBuffer);
			delete out;
			return false;
		}
	}

	out->finalize();

	if (out->err()) {
		warning("ScummEngine::changeSavegameName(): Error in output file stream after finalizing...");
		free(saveBuffer);
		delete out;
		return false;
	}

	free(saveBuffer);
	delete out;

	return true;
}


void ScummEngine::saveLoadWithSerializer(Common::Serializer &s) {
	int i;
	int var98Backup;
	uint8 md5Backup[16];

	// MD5 Operations: Backup on load, compare, and reset.
	if (s.isLoading())
		memcpy(md5Backup, _gameMD5, 16);


	//
	// Save/load main state (many members of class ScummEngine get saved here)
	//
	s.syncBytes(_gameMD5, sizeof(_gameMD5), VER(39));
	s.skip(2, VER(8), VER(50)); // _roomWidth
	s.skip(2, VER(8), VER(50)); // _roomHeight
	s.skip(4, VER(8), VER(50)); // _ENCD_offs
	s.skip(4, VER(8), VER(50)); // _EXCD_offs
	s.skip(4, VER(8), VER(50)); // _IM00_offs
	s.skip(4, VER(8), VER(50)); // _CLUT_offs
	s.skip(4, VER(8), VER(9)); // _EPAL_offs
	s.skip(4, VER(8), VER(50)); // _PALS_offs
	s.syncAsByte(_curPalIndex, VER(8));
	s.syncAsByte(_currentRoom, VER(8));
	s.syncAsByte(_roomResource, VER(8));
	s.syncAsByte(_numObjectsInRoom, VER(8));
	s.syncAsByte(_currentScript, VER(8));
	s.skip(4 * _numLocalScripts, VER(8), VER(50)); // _localScriptOffsets

	// vm.localvar grew from 25 to 40 script entries and then from
	// 16 to 32 bit variables (but that wasn't reflect here)... and
	// THEN from 16 to 25 variables.
	sync2DArray(s, vm.localvar, 25, 17, Common::Serializer::Uint16LE, VER(8), VER(8));
	sync2DArray(s, vm.localvar, 40, 17, Common::Serializer::Uint16LE, VER(9), VER(14));

	// We used to save 25 * 40 = 1000 blocks; but actually, each 'row consisted of 26 entry,
	// i. 26 * 40 = 1040. Thus the last 40 blocks of localvar where not saved at all. To be
	// able to load this screwed format, we use a trick: We load 26 * 38 = 988 blocks.
	// Then, we mark the followin 12 blocks (24 bytes) as obsolete.
	sync2DArray(s, vm.localvar, 38, 26, Common::Serializer::Uint16LE, VER(15), VER(17));
	s.skip(2 * 12, VER(15), VER(17));

	// This was the first proper multi dimensional version of the localvars, with 32 bit values
	sync2DArray(s, vm.localvar, 40, 26, Common::Serializer::Uint32LE, VER(18), VER(19));

	// Then we doubled the script slots again, from 40 to 80
	sync2DArray(s, vm.localvar, NUM_SCRIPT_SLOT, 26, Common::Serializer::Uint32LE, VER(20));

	s.syncBytes(_resourceMapper, 128, VER(8));
	s.syncBytes(_charsetColorMap, 16, VER(8));

	// _charsetData grew from 10*16, to 15*16, to 23*16 bytes
	for (i = 0; i < 10; ++i) {
		s.syncBytes(_charsetData[i], 16, VER(8));
	}
	for (; i < 15; ++i) {
		s.syncBytes(_charsetData[i], 16, VER(10));
	}
	for (; i < 23; ++i) {
		s.syncBytes(_charsetData[i], 16, VER(67));
	}

	s.skip(2, VER(8), VER(62)); // _curExecScript

	s.syncAsSint16LE(camera._dest.x, VER(8));
	s.syncAsSint16LE(camera._dest.y, VER(8));
	s.syncAsSint16LE(camera._cur.x, VER(8));
	s.syncAsSint16LE(camera._cur.y, VER(8));
	if (_game.platform == Common::kPlatformFMTowns)
		// WORKAROUND: FM-TOWNS original _screenHeight is 240. if we use trim_fmtowns_to_200_pixels, it's reduced to 200
		// camera's y is always half of the screen. in order to share save games between the two modes, we need to update the y
		camera._cur.y = _screenHeight / 2;

	s.syncAsSint16LE(camera._last.x, VER(8));
	s.syncAsSint16LE(camera._last.y, VER(8));
	s.syncAsSint16LE(camera._accel.x, VER(8));
	s.syncAsSint16LE(camera._accel.y, VER(8));
	s.syncAsSint16LE(_screenStartStrip, VER(8));
	s.syncAsSint16LE(_screenEndStrip, VER(8));
	s.syncAsByte(camera._mode, VER(8));
	s.syncAsByte(camera._follows, VER(8));
	s.syncAsSint16LE(camera._leftTrigger, VER(8));
	s.syncAsSint16LE(camera._rightTrigger, VER(8));
	s.syncAsUint16LE(camera._movingToActor, VER(8));
	s.syncAsByte(_cameraIsFrozen, VER(108));

	// Old stuff for Mac versions, see below...
	s.skip(2, VER(112), VER(121)); // Old _screenDrawOffset
	s.syncAsByte(_useMacScreenCorrectHeight, VER(112));

	// Post-load fix for some savegame versions which offset the engine elements
	// instead of offsetting the final screen texture and the mouse coordinates...
	if (s.isLoading()) {
		if (_game.version == 3 && _game.platform == Common::kPlatformMacintosh && s.getVersion() >= VER(112) && s.getVersion() < VER(121)) {
			camera._cur.y -= 20;
			camera._last.y -= 20;
		}
	}

	s.syncAsByte(_actorToPrintStrFor, VER(8));
	s.syncAsByte(_charsetColor, VER(8));

	// _charsetBufPos was changed from byte to int
	s.syncAsByte(_charsetBufPos, VER(8), VER(9));
	s.syncAsSint16LE(_charsetBufPos, VER(10));

	s.syncAsByte(_haveMsg, VER(8));
	s.syncAsByte(_haveActorSpeechMsg, VER(61));
	s.syncAsByte(_useTalkAnims, VER(8));

	s.syncAsSint16LE(_talkDelay, VER(8));
	s.syncAsSint16LE(_defaultTextSpeed, VER(8));
	s.skip(2, VER(8), VER(27)); // _numInMsgStack
	s.syncAsByte(_sentenceNum, VER(8));

	s.syncAsByte(vm.cutSceneStackPointer, VER(8));
	s.syncArray(vm.cutScenePtr, 5, Common::Serializer::Uint32LE, VER(8));
	s.syncBytes(vm.cutSceneScript, 5, VER(8));
	s.syncArray(vm.cutSceneData, 5, Common::Serializer::Sint16LE, VER(8));
	s.syncAsSint16LE(vm.cutSceneScriptIndex, VER(8));

	s.syncAsByte(vm.numNestedScripts, VER(8));
	s.syncAsByte(_userPut, VER(8));
	s.syncAsUint16LE(_userState, VER(17));
	s.syncAsByte(_cursor.state, VER(8));
	s.skip(1, VER(8), VER(20)); // _gdi->_cursorActive
	s.syncAsByte(_currentCursor, VER(8));

	if (_outputPixelFormat.bytesPerPixel == 2) {
		if (s.getVersion() >= VER(107)) {
			uint16 *pos = (uint16*)_grabbedCursor;
			for (i = 0; i < 4096; ++i)
				s.syncAsUint16LE(*pos++, VER(20));
		} else if (s.getVersion() >= VER(20)) {
			s.syncBytes(_grabbedCursor, 8192, VER(20));
			// Patch older savegames if they were saved on a system with a
			// different endianness than the current system's endianness
			// which is now used for loading. We just check the format of
			// the transparency color and then swap bytes if needed.
			// We read the transparent color from far back inside the buffer
			// where actual cursor data would never get stored (at least not
			// for the games concerned).
			uint16 transCol = (_game.heversion >= 80) ? 5 : 255;
			if (READ_UINT16(&_grabbedCursor[2046]) == (transCol << 8)) {
				uint16 *pos = (uint16*)_grabbedCursor;
				for (i = 0; i < 4096; ++i) {
					*pos = SWAP_BYTES_16(*pos);
					pos++;
				}
			}
		}
	} else {
		s.syncBytes(_grabbedCursor, 8192, VER(20));
	}

	s.syncAsSint16LE(_cursor.width, VER(20));
	s.syncAsSint16LE(_cursor.height, VER(20));
	s.syncAsSint16LE(_cursor.hotspotX, VER(20));
	s.syncAsSint16LE(_cursor.hotspotY, VER(20));

	// Post-load fix for broken SAMNMAX savegames which contain invalid
	// cursor values; the value we're setting here should not count since
	// it's being replaced by the post-load script, as long as it's not zero.
	// The same also happens for the Mac versions of INDY3 and Loom: the cursor
	// was being handled directly with a CursorMan.replaceCursor() without
	// specifying any values for the _cursor object (#14498). Let's fix that
	// with the proper values.
	if (_cursor.width == 0 || _cursor.height == 0) {
		if (_game.version == 6 || (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh)) {
			_cursor.width = 15;
			_cursor.height = 15;
			_cursor.hotspotX = 7;
			_cursor.hotspotY = 7;
		} else if (_game.id == GID_LOOM) {
			_cursor.width = 16;
			_cursor.height = 16;
			if (_game.platform == Common::kPlatformMacintosh) {
				_cursor.hotspotX = 3;
				_cursor.hotspotY = 2;
			} else { // DOS, Amiga, FM-Towns and PCE
				_cursor.hotspotX = _cursor.hotspotY = 0;
			}
		}
	} else if ((_cursor.width <= 0 || _cursor.width > 640 || _cursor.height <= 0 || _cursor.height > 480) && _game.platform == Common::kPlatformMacintosh) {
		_cursor.width = 11;
		_cursor.height = 16;
		_cursor.hotspotX = 1;
		_cursor.hotspotY = 1;
	}

	s.syncAsByte(_cursor.animate, VER(20));
	s.syncAsByte(_cursor.animateIndex, VER(20));

	// Don't restore the mouse position when using
	// the original GUI, since the originals didn't
	if (s.isLoading() && isUsingOriginalGUI()) {
		s.skip(2);
		s.skip(2);
	} else {
		s.syncAsSint16LE(_mouse.x, VER(20));
		s.syncAsSint16LE(_mouse.y, VER(20));
	}

	s.syncBytes(_colorUsedByCycle, 256, VER(60));
	s.syncAsByte(_doEffect, VER(8));
	s.syncAsByte(_switchRoomEffect, VER(8));
	s.syncAsByte(_newEffect, VER(8));
	s.syncAsByte(_switchRoomEffect2, VER(8));
	s.syncAsByte(_bgNeedsRedraw, VER(8));

	// The state of palManipulate is stored only since V10
	s.syncAsByte(_palManipStart, VER(10));
	s.syncAsByte(_palManipEnd, VER(10));
	s.syncAsUint16LE(_palManipCounter, VER(10));

	// gfxUsageBits grew from 200 to 410 entries. Then 3 * 410 entries:
	s.syncArray(gfxUsageBits, 200, Common::Serializer::Uint32LE, VER(8), VER(9));
	s.syncArray(gfxUsageBits, 410, Common::Serializer::Uint32LE, VER(10), VER(13));
	s.syncArray(gfxUsageBits, 3 * 410, Common::Serializer::Uint32LE, VER(14));

	s.skip(1, VER(8), VER(50)); // _gdi->_transparentColor
	s.syncBytes(_currentPalette, 768, VER(8));
	s.syncBytes(_darkenPalette, 768, VER(53));

	// Sam & Max specific palette replaced by _shadowPalette now.
	s.skip(256, VER(8), VER(33)); // _proc_special_palette

	s.syncBytes(_charsetBuffer, 256, VER(8));

	s.syncAsByte(_egoPositioned, VER(8));

	// _gdi->_imgBufOffs grew from 4 to 5 entries. Then one day we realized
	// that we don't have to store it since initBGBuffers() recomputes it.
	s.skip(2 * 4, VER(8), VER(9)); // _gdi->_imgBufOffs
	s.skip(2 * 5, VER(10), VER(26)); // _gdi->_imgBufOffs

	// See _imgBufOffs: _numZBuffer is recomputed by initBGBuffers().
	s.skip(1, VER(8), VER(26)); // _gdi->_numZBuffer

	s.syncAsByte(_screenEffectFlag, VER(8));

	s.skip(4, VER(8), VER(9)); // _randSeed1
	s.skip(4, VER(8), VER(9)); // _randSeed2

	// Converted _shakeEnabled to boolean and added a _shakeFrame field.
	s.syncAsSint16LE(_shakeEnabled, VER(8), VER(9));
	s.syncAsByte(_shakeEnabled, VER(10));
	s.syncAsUint32LE(_shakeFrame, VER(10));

	s.syncAsByte(_keepText, VER(8));

	s.syncAsUint16LE(_screenB, VER(8));
	s.syncAsUint16LE(_screenH, VER(8));

	// Post-load fix for some savegame versions which offset the engine elements
	// instead of offsetting the final screen texture and the mouse coordinates...
	if (s.isLoading()) {
		if (_game.version == 3 && _game.platform == Common::kPlatformMacintosh && s.getVersion() >= VER(112) && s.getVersion() < VER(121)) {
			_screenB -= 20;
			_screenH -= 20;
		}
	}

	s.syncAsUint16LE(_NESCostumeSet, VER(47));

	s.skip(2, VER(9), VER(9)); // _cd_track
	s.skip(2, VER(9), VER(9)); // _cd_loops
	s.skip(2, VER(9), VER(9)); // _cd_frame
	s.skip(2, VER(9), VER(9)); // _cd_end

	// MD5 Operations: Backup on load, compare, and reset.
	if (s.isLoading()) {
		char md5str1[32+1], md5str2[32+1];
		for (i = 0; i < 16; i++) {
			Common::sprintf_s(md5str1 + i*2, 3, "%02x", (int)_gameMD5[i]);
			Common::sprintf_s(md5str2 + i*2, 3, "%02x", (int)md5Backup[i]);
		}

		debug(2, "Save version: %d", s.getVersion());
		debug(2, "Saved game MD5: %s", (s.getVersion() >= 39) ? md5str1 : "unknown");

		if (memcmp(md5Backup, _gameMD5, 16) != 0) {
			warning("Game was saved with different gamedata - you may encounter problems");
			debug(1, "You have %s and save is %s.", md5str2, md5str1);
			memcpy(_gameMD5, md5Backup, 16);
		}
	}


	// Starting V14, we extended the usage bits, to be able to cope with games
	// that have more than 30 actors (up to 94 are supported now, in theory).
	// Since the format of the usage bits was changed by this, we have to
	// convert them when loading an older savegame.
	if (s.isLoading() && s.getVersion() < VER(14))
		upgradeGfxUsageBits();

	// When loading, reset the ShakePos. Fixes one part of bug #7141
	if (s.isLoading() && s.getVersion() >= VER(10))
		_system->setShakePos(0, 0);

	// When loading, move the mouse to the saved mouse position.
	if (s.isLoading() && s.getVersion() >= VER(20)) {
		int x = _mouse.x;
		int y = _mouse.y;

		// Convert the mouse position, which uses game coordinates, to
		// screen coordinates for the rendering modes that need it.

		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			x *= 2;
			x += (kHercWidth - _screenWidth * 2) / 2;
			y = y * 7 / 4;
		} else if (_textSurfaceMultiplier == 2 || _renderMode == Common::kRenderCGA_BW || _enableEGADithering) {
			x *= 2;
			y *= 2;
		} else if (_macScreen) {
			x *= 2;
			y *= 2;
		}

		updateCursor();
		_system->warpMouse(x, y);
	}

	// Before V61, we re-used the _haveMsg flag to handle "alternative" speech
	// sound files (see charset code 10).
	if (s.isLoading() && s.getVersion() < VER(61)) {
		if (_haveMsg == 0xFE) {
			_haveActorSpeechMsg = false;
			_haveMsg = 0xFF;
		} else {
			_haveActorSpeechMsg = true;
		}
	}

	//
	// Save/load actors
	//
	for (i = 0; i < _numActors; i++)
		_actors[i]->saveLoadWithSerializer(s);


	//
	// Save/load sound data
	//
	_sound->saveLoadWithSerializer(s);


	//
	// Save/load script data
	//
	s.syncArray(vm.slot, 25, syncWithSerializer, VER(0), VER(8));
	s.syncArray(vm.slot, 40, syncWithSerializer, VER(9), VER(19));
	s.syncArray(vm.slot, NUM_SCRIPT_SLOT, syncWithSerializer, VER(20));

	if (s.getVersion() < VER(46)) {
		// When loading an old savegame, make sure that the 'cycle'
		// field is set to something sensible, otherwise the scripts
		// that were running probably won't be.

		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			vm.slot[i].cycle = 1;
		}
	}


	//
	// Save/load local objects
	//
	s.syncArray(_objs, _numLocalObjects, syncWithSerializer);
	if (s.isLoading()) {
		if (s.getVersion() < VER(13)) {
			// Since roughly v13 of the save games, the objs storage has changed a bit
			for (i = _numObjectsInRoom; i < _numLocalObjects; i++)
				_objs[i].obj_nr = 0;
		} else if (_game.version == 0 && s.getVersion() < VER(91)) {
			for (i = 0; i < _numLocalObjects; i++) {
				// Merge object id and type (previously stored in flags)
				if (_objs[i].obj_nr != 0 && OBJECT_V0_TYPE(_objs[i].obj_nr) == 0 && _objs[i].flags != 0)
					_objs[i].obj_nr = OBJECT_V0(_objs[i].obj_nr, _objs[i].flags);
				_objs[i].flags = 0;
			}
		}
	}


	//
	// Save/load misc stuff
	//
	s.syncArray(_verbs, _numVerbs, (_game.version < 7 && _language != Common::HE_ISR) ? syncWithSerializerDef : syncWithSerializerV7orISR);
	s.syncArray(vm.nest, 16, syncWithSerializer, VER(0), VER(119));
	s.syncArray(vm.nest, kMaxScriptNestingHE, syncWithSerializer, VER(120));
	s.syncArray(_sentence, 6, syncWithSerializer);
	s.syncArray(_string, 6, syncWithSerializer);
	s.syncArray(_colorCycle, 16, syncWithSerializer);
	s.syncArray(_scaleSlots, 20, syncWithSerializer, VER(13));


	//
	// Save/load resources
	//
	ResType type;
	ResId idx;
	if (s.getVersion() >= VER(26)) {
		// New, more robust resource save/load system. This stores the type
		// and index of each resource. Thus if we increase e.g. the maximum
		// number of script resources, savegames won't break.
		if (s.isSaving()) {
			uint16 endMarker = 0xFFFF;
			for (type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
				if (_res->_types[type]._mode != kStaticResTypeMode && type != rtTemp && type != rtBuffer) {
					s.syncAsUint16LE(type);	// Save the res type...
					for (idx = 0; idx < _res->_types[type].size(); idx++) {
						// Only save resources which actually exist...
						if (_res->_types[type][idx]._address) {
							s.syncAsUint16LE(idx);	// Save the index of the resource
							saveResource(s, type, idx);
						}
					}
					s.syncAsUint16LE(endMarker);
				}
			}
			s.syncAsUint16LE(endMarker);
		} else {
			uint16 tmp;
			while (s.syncAsUint16LE(tmp), tmp != 0xFFFF) {
				type = (ResType)tmp;
				while (s.syncAsUint16LE(idx), idx != 0xFFFF) {
					assert(idx < _res->_types[type].size());
					loadResource(s, type, idx);
					applyWorkaroundIfNeeded(type, idx);
				}
			}
		}
	} else {
		// Old, fragile resource save/load system. Doesn't save resources
		// with index 0, and breaks whenever we change the limit on a given
		// resource type.
		for (type = rtFirst; type <= rtLast; type = ResType(type + 1))
			if (_res->_types[type]._mode != kStaticResTypeMode && type != rtTemp && type != rtBuffer) {
				// For V1-V5 games, there used to be no object name resources.
				// At some point this changed. But since old savegames rely on
				// unchanged resource counts, we have to hard code the following check
				if (_game.version < 6 && type == rtObjectName)
					continue;
				for (idx = 1; idx < _res->_types[type].size(); idx++)
					loadResourceOLD(s, type, idx);
			}
	}


	//
	// Save/load global object state
	//
	s.syncBytes(_objectOwnerTable, _numGlobalObjects);
	s.syncBytes(_objectStateTable, _numGlobalObjects);
	if (_objectRoomTable)
		s.syncBytes(_objectRoomTable, _numGlobalObjects);


	//
	// Save/load palette data
	// Don't save 16 bit palette in FM-Towns and PCE games, since it gets regenerated afterwards anyway.
	if (_16BitPalette && !(_game.platform == Common::kPlatformFMTowns && s.getVersion() < VER(82)) && !((_game.platform == Common::kPlatformFMTowns || _game.platform == Common::kPlatformPCEngine) && s.getVersion() > VER(87))) {
		s.syncArray(_16BitPalette, 512, Common::Serializer::Uint16LE);
	}


	// FM-Towns specific (extra palette data, color cycle data, etc.)
	// In earlier save game versions (below 87) the FM-Towns specific data would get saved (and loaded) even in non FM-Towns games.
	// This would cause an unnecessary save file incompatibility between DS (which uses the DISABLE_TOWNS_DUAL_LAYER_MODE setting)
	// and other ports.
	// In version 88 and later the save files from FM-Towns targets are compatible between DS and other platforms, too.

#ifdef DISABLE_TOWNS_DUAL_LAYER_MODE
	byte hasTownsData = 0;
	if (_game.platform == Common::kPlatformFMTowns && s.getVersion() > VER(87))
		s.syncAsByte(hasTownsData);

	if (hasTownsData) {
		// Skip FM-Towns specific data
		s.skip(69 + 44 * sizeof(int16));
	}

#else
	byte hasTownsData = ((_game.platform == Common::kPlatformFMTowns && s.getVersion() >= VER(87)) || (s.getVersion() >= VER(82) && s.getVersion() < VER(87))) ? 1 : 0;
	if (_game.platform == Common::kPlatformFMTowns && s.getVersion() > VER(87))
		s.syncAsByte(hasTownsData);

	if (hasTownsData) {
		s.syncBytes(_textPalette, 48);
		s.syncArray(_cyclRects, 10, syncWithSerializer, VER(82));
		if (s.getVersion() >= VER(82))
			syncWithSerializer(s, _curStringRect);
		s.syncBytes(_townsCharsetColorMap, 16);
		s.syncAsByte(_townsOverrideShadowColor, VER(82));
		s.syncAsByte(_numCyclRects, VER(82));
		s.syncAsByte(_townsPaletteFlags, VER(82));
		s.syncAsByte(_townsClearLayerFlag, VER(82));
		s.syncAsByte(_townsActiveLayerFlags, VER(82));
	} else if (_game.platform == Common::kPlatformFMTowns && s.getVersion() >= VER(82)) {
		warning("Save file is missing FM-Towns specific graphic data (game was apparently saved on another platform)");
	}
#endif

	if (_shadowPaletteSize) {
		s.syncBytes(_shadowPalette, _shadowPaletteSize);
		// _roomPalette didn't show up until V21 save games
		// Note that we also save the room palette for Indy4 Amiga, since it
		// is used as palette map there too, but we do so slightly a bit
		// further down to group it with the other special palettes needed.
		if (s.getVersion() >= VER(21) && _game.version < 5)
			s.syncBytes(_roomPalette, sizeof(_roomPalette));
	}

	// PalManip data was not saved before V10 save games
	if (s.getVersion() < VER(10))
		_palManipCounter = 0;
	if (_palManipCounter) {
		if (!_palManipPalette)
			_palManipPalette = (byte *)calloc(0x300, 1);
		if (!_palManipIntermediatePal)
			_palManipIntermediatePal = (byte *)calloc(0x600, 1);
		s.syncBytes(_palManipPalette, 0x300);
		s.syncBytes(_palManipIntermediatePal, 0x600);
	}

	// darkenPalette was not saved before V53
	if (s.isLoading() && s.getVersion() < VER(53)) {
		memcpy(_darkenPalette, _currentPalette, 768);
	}

	// _colorUsedByCycle was not saved before V60
	if (s.isLoading() && s.getVersion() < VER(60)) {
		memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
	}

	// Indy4 Amiga specific palette tables were not saved before V85
	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		if (s.getVersion() >= 85) {
			s.syncBytes(_roomPalette, 256);
			s.syncBytes(_verbPalette, 256);
			s.syncBytes(_amigaPalette, 3 * 64);

			// Starting from version 86 we also save the first used color in
			// the palette beyond the verb palette. For old versions we just
			// look for it again, which hopefully won't cause any troubles.
			if (s.getVersion() >= VER(86)) {
				s.syncAsUint16LE(_amigaFirstUsedColor);
			} else {
				amigaPaletteFindFirstUsedColor();
			}
		} else {
			warning("Save with old Indiana Jones 4 Amiga palette handling detected");
			// We need to restore the internal state of the Amiga palette for Indy4
			// Amiga. This might lead to graphics glitches!
			setAmigaPaletteFromPtr(_currentPalette);
		}
	}

	// Before version 109, palette cycling for v4 games was handled in a different
	// way (which, by retrofitting v5 code, caused a class of bugs like #10854).
	// The proper v4 code has now been implemented from disasm (specifically, only the
	// LOOM CD and MI1 VGA executables have said code).
	//
	// Given that the previous implementation mangled the cycling data during the init
	// phase, we have to resort to the following post-load fix, otherwise the color
	// cycling will not occur on game load.
	if (_game.version == 4 && (_game.id == GID_LOOM || _game.id == GID_MONKEY_VGA) &&
		s.getVersion() < VER(109)) {
		byte *roomptr = getResourceAddress(rtRoom, _roomResource);
		const byte *ptr = findResourceData(MKTAG('C', 'Y', 'C', 'L'), roomptr);
		if (ptr) {
			initCycl(ptr);
		}
	}

	//
	// Save/load more global object state
	//
	s.syncArray(_classData, _numGlobalObjects, Common::Serializer::Uint32LE);


	//
	// Save/load script variables
	//

	// From disasm...
	int32 dottVarsBackup[5];
	if (_game.id == GID_TENTACLE) {
		for (int j = 0; j < 5; j++)
			dottVarsBackup[j] = _scummVars[120 + j];
	}

	var98Backup = _scummVars[98];

	s.syncArray(_roomVars, _numRoomVariables, Common::Serializer::Sint32LE, VER(38));

	int currentSoundCard = VAR_SOUNDCARD != 0xFF ? VAR(VAR_SOUNDCARD) : -1;

	// The variables grew from 16 to 32 bit.
	if (s.getVersion() < VER(15))
		s.syncArray(_scummVars, _numVariables, Common::Serializer::Sint16LE);
	else
		s.syncArray(_scummVars, _numVariables, Common::Serializer::Sint32LE);

	if (_game.platform == Common::kPlatformDOS && s.isLoading() && VAR_SOUNDCARD != 0xFF && (_game.heversion < 70 && _game.version <= 6)) {
		if (currentSoundCard != VAR(VAR_SOUNDCARD)) {
			const char *soundCards[] = {
				"PC Speaker", "IBM PCjr/Tandy", "Creative Music System", "AdLib", "Roland MT-32/CM-32L"
			};

			GUI::MessageDialog dialog(
				Common::U32String::format(_("Warning: incompatible sound settings detected between the current configuration and this saved game.\n\n"
					"Current music device: %s (id %d)\nSave file music device: %s (id %d)\n\n"
					"Loading will be attempted, but the game may behave incorrectly or crash.\n"
					"Please change the audio configuration accordingly in order to properly load this save file."),
					currentSoundCard < ARRAYSIZE(soundCards) ? soundCards[currentSoundCard] : "invalid", currentSoundCard,
					VAR(VAR_SOUNDCARD) < ARRAYSIZE(soundCards) ? soundCards[VAR(VAR_SOUNDCARD)] : "invalid", VAR(VAR_SOUNDCARD))
			);
			runDialog(dialog);
		}
	}

	// This is again from disasm...
	if (_game.id == GID_TENTACLE) {
		for (int j = 0; j < 5; j++)
			_scummVars[120 + j] = dottVarsBackup[j];

		_scummVars[70] = 1;
	}

	if (_game.id == GID_INDY4)
		_scummVars[98] = var98Backup;

	s.syncBytes(_bitVars, _numBitVariables / 8);

	// Set video mode var to the current actual mode, not the one that was enabled when the game was saved.
	// At least for Loom this fixes glitches, since the game actually reads the var and makes actor palette
	// adjustments based on that. This is a bug that happens in the original interpreter, too.
	if (s.isLoading() && VAR_VIDEOMODE != 0xFF && _game.heversion == 0) {
		int videoModeSaved = VAR(VAR_VIDEOMODE);
		setVideoModeVarToCurrentConfig();
		// For MI1EGA we need to know if the savegame is from a different render mode, so we can apply some
		// post-load fixes if necessary.
		_videoModeChanged = (videoModeSaved != VAR(VAR_VIDEOMODE));
	}

	// WORKAROUND: FM-TOWNS Zak used the extra 40 pixels at the bottom to increase the inventory to 10 items
	// if we trim to 200 pixels, we can show only 6 items
	// therefore we need to make sure that the inventory is now display correctly, regardless of the mode that the game was saved with
	if (s.isLoading() && _game.platform == Common::kPlatformFMTowns && _game.id == GID_ZAK) {
		if (ConfMan.getBool("trim_fmtowns_to_200_pixels"))
			_verbs[getVerbSlot(116, 0)].curRect.top = 208 - 18;		// make down arrow higher
		else
			_verbs[getVerbSlot(116, 0)].curRect.top = 208;			// return down arrow to its original location

		if (ConfMan.getBool("trim_fmtowns_to_200_pixels"))
			// VAR(102) to VAR(111) originally keep the 10 displayed inventory items; clean the last 4 ones
			for (int v = 102 + 6; v <= 111; v++)
				VAR(v) = 0;

		// Make sure the appropriate verbs and arrows are displayed.
		// We avoid doing that in room 50 (save room) since it can crash
		// the game and trigger several unwanted side effects (bug #14387).
		if (_currentRoom != 50)
			runInventoryScript(0);
	}

	//
	// Save/load a list of the locked objects
	//
	if (s.isSaving()) {
		byte endMarker = 0xFF;
		for (type = rtFirst; type <= rtLast; type = ResType(type + 1))
			for (idx = 1; idx < _res->_types[type].size(); idx++) {
				if (_res->isLocked(type, idx)) {
					s.syncAsByte(type);
					s.syncAsUint16LE(idx);
				}
			}
		s.syncAsByte(endMarker);
	} else {
		uint8 tmp;
		while (s.syncAsByte(tmp), tmp != 0xFF) {
			type = (ResType)tmp;
			s.syncAsUint16LE(idx);
			_res->lock(type, idx);
		}
	}


	//
	// Save/load the Audio CD status
	//
	if (s.getVersion() >= VER(24)) {
		AudioCDManager::Status info;
		if (s.isSaving())
			info = _system->getAudioCDManager()->getStatus();
		syncWithSerializer(s, info);

		if (s.isLoading() && info.playing) {
			_sound->restoreCDAudioAfterLoad(info);
		}
	}

	// The following code tries to cope with the unfortunate fact that savegames
	// vary in size/content depending on the sound setting. And historically, until
	// version 124, it was not even possible to make a definite assumption about the
	// size of the sound data block. Loading savegames that were made with a different
	// sound setting than the current one would lead to a crash or to a corrupted
	// engine state. Now, the engine should recover in most cases, but some issues
	// still have to be addressed.
	// TODO: identify the device for the saved sound data blocks (preferably even the
	// old ones which are prone to have an unreliable VAR_SOUNDCARD value) and/or reset
	// the sound state, so that even changes from AdLib to MT-32 or vice versa could
	// eventually work.
	Serializer *pSer = static_cast<Serializer*>(&s);

	int32 sndDataBlockSize = 0;
	if (s.isLoading()) {
		if (s.getVersion() > VER(123))
			s.syncAsSint32BE(sndDataBlockSize);
		else if (_game.version < 7 && _game.heversion == 0)
			sndDataBlockSize = checkSoundEngineSaveDataSize(*pSer);
	}

	int64 before = pSer->pos();

	// Unfortunately, it is not totally easy to write the size of the sound engine data before that data,
	// because we cannot seek around in the compressed write stream. So, we write to a temp stream, first.
	Common::MemoryWriteStreamDynamic *ws = nullptr;
	if (s.isSaving()) {
		ws = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
		pSer = new Serializer(nullptr, ws);
		pSer->setVersion(s.getVersion());
	}

	// This sndDataBlockSize check catches e. g. MI1 saves with a CMS setting when trying to load with AdLib or MT-32.
	// But it only works because CMS really saves nothing. If it were just a smaller data block, it would still load
	// garbage data (and crash or corrupt the game state).
	if (s.isSaving() || sndDataBlockSize > 0) {
		//
		// Save/load the iMuse status
		if (_imuse && (_saveSound || !_saveTemporaryState))
			_imuse->saveLoadIMuse(*pSer, this);
		//
		// Save/load music engine status
		if (_musicEngine)
			_musicEngine->saveLoadWithSerializer(*pSer);
	}

	if (s.isSaving()) {
		// Write the sound data size, then the actual data from the temp stream.
		ws->finalize();
		sndDataBlockSize = ws->size();
		s.syncAsSint32BE(sndDataBlockSize);
		s.syncBytes(ws->getData(), sndDataBlockSize);
		delete pSer;
		delete ws;
	} else {
		// Check if the sound engine read the expected number of bytes from the save file.
		// If not, then the sound device selection was probably changed after the save was made.
		int64 now = pSer->pos();
		if (s.err() || (before + sndDataBlockSize != now)) {
			static const char wmsg1[] = "more than the %d bytes contained in the savegame";
			static const char wmsg2[] = "%d bytes, savegame has %d bytes";
			// For SegaCD, we don't need a warning, since nothing can glitch there. We have to compensate
			// for the fact that there are old savegames that have an unused imuse state inside of them.
			// But fixing that will not lead to glitches or other surprises.
			if (_game.platform == Common::kPlatformSegaCD) {
				Common::String msg = s.err() ? Common::String::format(wmsg1, sndDataBlockSize) : Common::String::format(wmsg2, (int)(now - before), sndDataBlockSize);
				warning("Savegame sound data mismatch (sound engine tried to read %s). \r\nAdjusting file read position. Sound might start up with glitches...", msg.c_str());
			}
			// This will save the day in cases that are the opposite from the ones mentioned above:
			// When loading a MI1 savegame that was made with a MT-32 or AdLib setting in a CMS setup,
			// we skip the sound data block here.
			pSer->seek(before + sndDataBlockSize);
		}
	}

	// At least from now on, VAR_SOUNDCARD will have a reliable value.
	if (s.isLoading() && (_game.heversion < 70 && _game.version <= 6))
		setSoundCardVarToCurrentConfig();

	//
	// Save/load the charset renderer state
	//
	if (s.getVersion() >= VER(73)) {
		_charset->saveLoadWithSerializer(s);
	} else if (s.isLoading()) {
		if (s.getVersion() == VER(72)) {
			byte curId;
			s.syncAsByte(curId);
			_charset->setCurID(curId);
		} else {
			// Before V72, the charset id wasn't saved. This used to cause issues such
			// as the one described in the bug report #3194. For these savegames,
			// we reinitialize the id using a, hopefully, sane value.
			_charset->setCurID(_string[0]._default.charset);
		}
	}
}

void ScummEngine_v0::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v2::saveLoadWithSerializer(s);

	s.syncAsByte(_currentMode, VER(78));
	s.syncAsByte(_currentLights, VER(78));
	s.syncAsByte(_activeVerb, VER(92));
	s.syncAsUint16LE(_activeObject, VER(92));
	s.syncAsUint16LE(_activeObject2, VER(92));
	s.syncAsByte(_cmdVerb, VER(92));
	s.syncAsUint16LE(_cmdObject, VER(92));
	s.syncAsUint16LE(_cmdObject2, VER(92));
	s.syncAsUint16LE(_walkToObject, VER(92));
	s.syncAsByte(_walkToObjectState, VER(92));
}


void ScummEngine_v2::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine::saveLoadWithSerializer(s);

	s.syncAsUint16LE(_inventoryOffset, VER(79));

	// In old saves we didn't store _inventoryOffset -> reset it to
	// a sane default when loading one of those.
	if (s.getVersion() < VER(79) && s.isLoading()) {
		_inventoryOffset = 0;
	}

	s.syncAsByte(_flashlight.xStrips, VER(99));
	s.syncAsByte(_flashlight.yStrips, VER(99));

	// Old saves are based on a different color mapping, so the verb colors need to be adjusted.
	if (s.getVersion() < VER(106) && s.isLoading() && _game.platform == Common::kPlatformDOS) {
		initV2MouseOver();
		for (int i = 0; i < _numVerbs; ++i) {
			if (!_verbs[i].verbid)
				continue;
			_verbs[i].color = 2;
			_verbs[i].hicolor = _hiLiteColorVerbArrow;
			_verbs[i].dimcolor = 8;
		}
	}
}

void ScummEngine_v5::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine::saveLoadWithSerializer(s);

	// This is probably only needed for Loom.
	// TODO: This looks wrong, _cursorImages is [4][17]
	sync2DArray(s, _cursorImages, 4, 16, Common::Serializer::Uint16LE, VER(44));
	s.syncBytes(_cursorHotspots, 8, VER(44));

	// Reset cursors for old FM-Towns savegames saved with 256 color setting.
	// Otherwise the cursor will be messed up when displayed in the new hi color setting.
	if (_game.platform == Common::kPlatformFMTowns && _outputPixelFormat.bytesPerPixel == 2 && s.isLoading() && s.getVersion() < VER(82)) {
		if (_game.id == GID_LOOM) {
			redefineBuiltinCursorFromChar(1, 1);
			redefineBuiltinCursorHotspot(1, 0, 0);
		} else {
			resetCursors();
		}
	}

	// Reset Mac cursors for Loom and Indy 3, otherwise the cursor will be
	// invisible after loading.

	if (s.isLoading() && _game.platform == Common::kPlatformMacintosh) {
		if ((_game.id == GID_LOOM && !_macCursorFile.empty()) || _macGui) {
			setBuiltinCursor(0);
		}

		// Also reset Mac cursors if the original GUI isn't enabled for games
		// which replace cursors that override the default cursor palette - bug #15520.
		if (_game.version == 5 && !_macGui) {
			setBuiltinCursor(0);
		}
	}

	// Regenerate 16bit palette after loading.
	// This avoids color issues when loading savegames that have been saved with a different ScummVM port
	// that uses a different 16bit color mode than the ScummVM port which is currently used.
#ifdef USE_RGB_COLOR
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && s.isLoading()) {
		for (int i = 0; i < 256; ++i)
			_16BitPalette[i] = get16BitColor(_currentPalette[i * 3 + 0], _currentPalette[i * 3 + 1], _currentPalette[i * 3 + 2]);
	}
#endif
}

#ifdef ENABLE_SCUMM_7_8
void syncWithSerializer(Common::Serializer &s, ScummEngine_v7::SubtitleText &st) {
	s.syncBytes(st.text, 256, VER(61));
	s.syncAsByte(st.charset, VER(61));
	s.syncAsByte(st.color, VER(61));
	s.syncAsSint16LE(st.xpos, VER(61));
	s.syncAsSint16LE(st.ypos, VER(61));
	s.syncAsByte(st.actorSpeechMsg, VER(61));
	s.syncAsByte(st.center, VER(106));
	s.syncAsByte(st.wrap, VER(106));
}

void ScummEngine_v8::saveLoadWithSerializer(Common::Serializer &s) {
	// Save/load the savegame thumbnail for COMI
	s.syncArray(_savegameThumbnailV8, 19200, Common::Serializer::Byte, VER(106));
	s.syncArray(_savegameThumbnailV8Palette, 256, Common::Serializer::Uint32LE, VER(106));

	// Also save the banner colors for the GUI
	s.syncArray(_bannerColors, 50, Common::Serializer::Uint32LE, VER(106));

	ScummEngine_v7::saveLoadWithSerializer(s);
}

void ScummEngine_v7::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine::saveLoadWithSerializer(s);

	_imuseDigital->saveLoadEarly(s);

	s.syncArray(_subtitleQueue, ARRAYSIZE(_subtitleQueue), syncWithSerializer);
	s.syncAsSint32LE(_subtitleQueuePos, VER(61));
	s.skip(4, VER(68), VER(68)); // _verbCharset
	s.syncAsSint32LE(_verbLineSpacing, VER(68));

	if (s.getVersion() <= VER(68) && s.isLoading()) {
		// WORKAROUND bug #3483: Reset the default charset color to a sane value.
		_string[0]._default.charset = _game.version == 7 ? 2 : 1;
	}

	// The original Save/Load screen for COMI saves a heap savegame when it is entered
	// and the same heap savegame is restored when it is exited, so let's refresh these
	// variables so that they are not lost. The original doesn't do this as it appears
	// to handle these temporary heap savegames a little differently, but this should
	// suffice...
	if (isUsingOriginalGUI() && _game.version == 8) {
		if (ConfMan.hasKey("original_gui_saveload_page", _targetName))
			VAR(VAR_SAVELOAD_PAGE) = ConfMan.getInt("original_gui_saveload_page");

		if (ConfMan.hasKey("original_gui_object_labels", _targetName))
			VAR(VAR_OBJECT_LABEL_FLAG) = ConfMan.getInt("original_gui_object_labels");
	}
}
#endif

void ScummEngine_v60he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine::saveLoadWithSerializer(s);

	s.syncBytes(_arraySlot, _numArray);
}

void ScummEngine_v70he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v60he::saveLoadWithSerializer(s);

	s.syncAsSint32LE(_heSndSoundId, VER(51));
	s.syncAsSint32LE(_heSndOffset, VER(51));
	s.syncAsSint32LE(_heSndChannel, VER(51));
	s.syncAsSint32LE(_heSndFlags, VER(51));
}

#ifdef ENABLE_HE
static void syncWithSerializer(Common::Serializer &s, WizPolygon &wp) {
	s.syncAsSint16LE(wp.points[0].x, VER(40));
	s.syncAsSint16LE(wp.points[0].y, VER(40));
	s.syncAsSint16LE(wp.points[1].x, VER(40));
	s.syncAsSint16LE(wp.points[1].y, VER(40));
	s.syncAsSint16LE(wp.points[2].x, VER(40));
	s.syncAsSint16LE(wp.points[2].y, VER(40));
	s.syncAsSint16LE(wp.points[3].x, VER(40));
	s.syncAsSint16LE(wp.points[3].y, VER(40));
	s.syncAsSint16LE(wp.points[4].x, VER(40));
	s.syncAsSint16LE(wp.points[4].y, VER(40));
	s.syncAsSint16LE(wp.boundingRect.left, VER(40));
	s.syncAsSint16LE(wp.boundingRect.top, VER(40));
	s.syncAsSint16LE(wp.boundingRect.right, VER(40));
	s.syncAsSint16LE(wp.boundingRect.bottom, VER(40));
	s.syncAsSint16LE(wp.id, VER(40));
	s.syncAsSint16LE(wp.numPoints, VER(40));
	s.syncAsByte(wp.flag, VER(40));
}

void ScummEngine_v71he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v70he::saveLoadWithSerializer(s);

	s.syncArray(_wiz->_polygons, ARRAYSIZE(_wiz->_polygons), syncWithSerializer);
}

void syncWithSerializer(Common::Serializer &s, FloodFillCommand &ffc) {
	s.syncAsSint32LE(ffc.box.left, VER(51));
	s.syncAsSint32LE(ffc.box.top, VER(51));
	s.syncAsSint32LE(ffc.box.right, VER(51));
	s.syncAsSint32LE(ffc.box.bottom, VER(51));
	s.syncAsSint32LE(ffc.x, VER(51));
	s.syncAsSint32LE(ffc.y, VER(51));
	s.syncAsSint32LE(ffc.flags, VER(51));
	s.skip(4, VER(51), VER(62)); // color
	s.syncAsSint32LE(ffc.color, VER(119));
}

void ScummEngine_v90he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v71he::saveLoadWithSerializer(s);

	_sprite->saveLoadWithSerializer(s);

	syncWithSerializer(s, _floodFillCommand);

	s.syncAsSint32LE(_maxSpriteNum, VER(51));
	s.syncAsSint32LE(_minSpriteNum, VER(51));
	s.syncAsSint32LE(_curSpriteGroupId, VER(51));
	s.skip(4, VER(51), VER(63)); // _activeSpriteCount
	s.syncAsSint32LE(_heObject, VER(51));
	s.syncAsSint32LE(_heObjectNum, VER(51));
	s.syncAsSint32LE(_hePaletteNum, VER(51));
}

void ScummEngine_v99he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v90he::saveLoadWithSerializer(s);

	s.syncBytes(_hePalettes, (_numPalettes + 1) * _hePaletteSlot);
}

void ScummEngine_v100he::saveLoadWithSerializer(Common::Serializer &s) {
	ScummEngine_v99he::saveLoadWithSerializer(s);

	s.syncAsSint32LE(_heResId, VER(51));
	s.syncAsSint32LE(_heResType, VER(51));
}
#endif

void ScummEngine::loadResourceOLD(Common::Serializer &ser, ResType type, ResId idx) {
	if (type == rtSound && ser.getVersion() >= VER(23)) {
		// Save/load only a list of resource numbers that need to be reloaded.
		uint16 tmp;
		ser.syncAsUint16LE(tmp);
		if (tmp)
			ensureResourceLoaded(rtSound, idx);
	} else if (_res->_types[type]._mode == kDynamicResTypeMode) {
		uint32 size = 0;
		ser.syncAsUint32LE(size);
		if (size) {
			_res->createResource(type, idx, size);
			ser.syncBytes(getResourceAddress(type, idx), size);
			if (type == rtInventory) {
				ser.syncAsUint16LE(_inventory[idx]);
			}
			if (type == rtObjectName && ser.getVersion() >= VER(25)) {
				// Paranoia: We increased the possible number of new names
				// to fix bugs #1591 and #1600. The savegame format
				// didn't change, but at least during the transition
				// period there is a slight chance that we try to load
				// more names than we have allocated space for. If so,
				// discard them.
				if (idx < _numNewNames)
					ser.syncAsUint16LE(_newNames[idx]);
			}
		}
	}
}

void ScummEngine::saveResource(Common::Serializer &ser, ResType type, ResId idx) {
	assert(_res->_types[type][idx]._address);

	if (_res->_types[type]._mode == kDynamicResTypeMode) {
		byte *ptr = _res->_types[type][idx]._address;
		uint32 size = _res->_types[type][idx]._size;

		ser.syncAsUint32LE(size);
		ser.syncBytes(ptr, size);

		if (type == rtInventory) {
			ser.syncAsUint16LE(_inventory[idx]);
		}
		if (type == rtObjectName) {
			ser.syncAsUint16LE(_newNames[idx]);
		}
	}
}

void ScummEngine::loadResource(Common::Serializer &ser, ResType type, ResId idx) {
	if (_game.heversion >= 60 && ser.getVersion() <= VER(65) &&
		((type == rtSound && idx == 1) || (type == rtSpoolBuffer))) {
		uint32 size = 0;
		ser.syncAsUint32LE(size);
		assert(size);
		_res->createResource(type, idx, size);
		ser.syncBytes(getResourceAddress(type, idx), size);
	} else if (type == rtSound) {
		// HE Games use sound resource 1 for speech
		if (_game.heversion >= 60 && idx == 1)
			return;

		ensureResourceLoaded(rtSound, idx);
	} else if (_res->_types[type]._mode == kDynamicResTypeMode) {
		uint32 size = 0;
		ser.syncAsUint32LE(size);
		assert(size);
		byte *ptr = _res->createResource(type, idx, size);
		ser.syncBytes(ptr, size);

		if (type == rtInventory) {
			ser.syncAsUint16LE(_inventory[idx]);
		}
		if (type == rtObjectName) {
			ser.syncAsUint16LE(_newNames[idx]);
		}
	}
}

int ScummEngine::checkSoundEngineSaveDataSize(Serializer &s) {
	if (!s.isLoading())
		return 0;

	uint8 d8 = 0;
	uint32 d32 = 0;
	int64 start = s.pos();
	// We just perform the steps we would do in the actual sync function, but on dummy vars.
	s.syncAsByte(d8, VER(73), VER(73));
	s.syncAsSint32LE(d32, VER(74));
	s.syncAsByte(d8, VER(73));
	if (s.getVersion() == VER(72))
		s.syncAsByte(d8);
	int64 diff = s.pos() - start;
	s.seek(start);

	return s.size() - diff - start;
}

int ScummEngine_v0::checkSoundEngineSaveDataSize(Serializer &s) {
	if (!s.isLoading())
		return 0;

	uint8 d8 = 0;
	uint16 d16 = 0;
	// We just perform the steps we would do in the actual sync function, but on dummy vars.
	int64 start = s.pos();
	s.syncAsByte(d8, VER(78));
	s.syncAsByte(d8, VER(78));
	s.syncAsByte(d8, VER(92));
	s.syncAsUint16LE(d16, VER(92));
	s.syncAsUint16LE(d16, VER(92));
	s.syncAsByte(d8, VER(92));
	s.syncAsUint16LE(d16, VER(92));
	s.syncAsUint16LE(d16, VER(92));
	s.syncAsUint16LE(d16, VER(92));
	s.syncAsByte(d8, VER(92));
	int64 diff = s.pos() - start;
	s.seek(start);

	return ScummEngine_v2::checkSoundEngineSaveDataSize(s) - diff;
}

int ScummEngine_v2::checkSoundEngineSaveDataSize(Serializer &s) {
	if (!s.isLoading())
		return 0;

	uint8 d8 = 0;
	uint16 d16 = 0;
	// We just perform the steps we would do in the actual sync function, but on dummy vars.
	int64 start = s.pos();
	s.syncAsUint16LE(d16, VER(79));
	s.syncAsByte(d8, VER(99));
	s.syncAsByte(d8, VER(99));
	int64 diff = s.pos() - start;
	s.seek(start);

	return ScummEngine::checkSoundEngineSaveDataSize(s) - diff;
}

int ScummEngine_v5::checkSoundEngineSaveDataSize(Serializer &s) {
	if (!s.isLoading())
		return 0;

	uint8 d[8] = { 0 };
	int64 start = s.pos();
	// We just perform the steps we would do in the actual sync function. I doesn't
	// matter if we fill in a bit garbage data here, since it will be overwritten later.
	sync2DArray(s, _cursorImages, 4, 16, Common::Serializer::Uint16LE, VER(44));
	s.syncBytes(d, 8, VER(44));
	int64 diff = s.pos() - start;
	s.seek(start);

	return ScummEngine::checkSoundEngineSaveDataSize(s) - diff;
}

} // End of namespace Scumm
