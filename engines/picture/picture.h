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
 * $URL$
 * $Id$
 *
 */

#ifndef PICTURE_H
#define PICTURE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

struct PictureGameDescription;

class AnimationPlayer;
class ArchiveReader;
class Input;
class MenuSystem;
class MoviePlayer;
class Palette;
class ResourceCache;
class ScriptInterpreter;
class Screen;
class SegmentMap;
class Sound;

class PictureEngine : public ::Engine {
	Common::KeyState _keyPressed;

protected:
	Common::Error init();
	Common::Error go();
//	void shutdown();

public:
	PictureEngine(OSystem *syst, const PictureGameDescription *gameDesc);
	virtual ~PictureEngine();

	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();

	Common::RandomSource *_rnd;
	const PictureGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	void loadScene(uint resIndex);

	void updateScreen();
	void updateInput();

	void setGuiHeight(int16 guiHeight);

	void setCamera(int16 x, int16 y);
	void scrollCameraUp(int16 delta);
	void scrollCameraDown(int16 delta);
	void scrollCameraLeft(int16 delta);
	void scrollCameraRight(int16 delta);
	void updateCamera();
	
	void talk(int16 slotIndex, int16 slotOffset);
	void playText(int16 slotIndex, int16 slotOffset);

	void walk(byte *walkData);
	
	int16 findRectAtPoint(byte *rectData, int16 x, int16 y, int16 index, int16 itemSize);

public:

	AnimationPlayer *_anim;
	ArchiveReader *_arc;
	Input *_input;
	MenuSystem *_menuSystem;
	MoviePlayer *_moviePlayer;
	Palette *_palette;
	ResourceCache *_res;
	ScriptInterpreter *_script;
	Screen *_screen;
	SegmentMap *_segmap;
	Sound *_sound;

	uint _sceneResIndex;
	int16 _sceneWidth, _sceneHeight;
	
	int _counter01, _counter02;
	bool _movieSceneFlag;
	byte _flag01;

	int16 _cameraX, _cameraY;
	int16 _newCameraX, _newCameraY;
	int16 _cameraHeight;
	int16 _guiHeight;

	bool _doSpeech, _doText;
	
	int16 _walkSpeedY, _walkSpeedX;

	int16 _mouseX, _mouseY;
	int16 _mouseCounter;
	bool _mouseButtonPressedFlag;
	byte _mouseButton;
	int16 _mouseDisabled;
	bool _leftButtonDown, _rightButtonDown;

	/* Save/load */

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);

	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);

	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

};

} // End of namespace Picture

#endif /* PICTURE_H */
