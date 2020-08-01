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

#ifndef SKY_SKY_H
#define SKY_SKY_H


#include "common/error.h"
#include "common/keyboard.h"
#include "engines/engine.h"

/**
 * This is the namespace of the Sky engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Beneath a Steel Sky
 */
namespace Sky {

struct SystemVars {
	uint32 systemFlags;
	uint32 gameVersion;
	uint32 mouseFlag;
	uint16 language;
	uint32 currentPalette;
	uint16 gameSpeed;
	uint16 currentMusic;
	bool pastIntro;
	bool paused;
};

class Sound;
class Disk;
class Text;
class Logic;
class Mouse;
class Screen;
class Control;
class MusicBase;
class Debugger;
class SkyCompact;

enum SkyAction {
	kSkyActionNone,
	kSkyActionToggleFastMode,
	kSkyActionToggleReallyFastMode,
	kSkyActionOpenControlPanel,
	kSkyActionConfirm,
	kSkyActionSkip,
	kSkyActionSkipLine,
	kSkyActionPause
};

class SkyEngine : public Engine {
protected:
	SkyAction _action;
	Common::KeyState _keyPressed;

	Sound *_skySound;
	Disk *_skyDisk;
	Text *_skyText;
	Logic *_skyLogic;
	Mouse *_skyMouse;
	Screen *_skyScreen;
	Control *_skyControl;
	SkyCompact *_skyCompact;
	Debugger *_debugger;

	MusicBase *_skyMusic;

public:
	SkyEngine(OSystem *syst);
	~SkyEngine() override;

	void syncSoundSettings() override;

	static bool isDemo();
	static bool isCDVersion();

	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	virtual Common::String getSaveStateName(int slot) const override {
		return Common::String::format("SKY-VM.%03d", slot);
	}

	static void *fetchItem(uint32 num);
	static void *_itemList[300];
	static SystemVars *_systemVars;
	static const char *shortcutsKeymapId;

protected:
	// Engine APIs
	Common::Error init();
	Common::Error go();
	Common::Error run() override {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}

	bool hasFeature(EngineFeature f) const override;

	byte _fastMode;

	void delay(int32 amount);
	void handleKey();

	void initItemList();

	void initVirgin();
	void loadFixedItems();
};

} // End of namespace Sky

#endif
