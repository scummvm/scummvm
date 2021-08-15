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

#ifndef SUPERNOVA_SUPERNOVA_H
#define SUPERNOVA_SUPERNOVA_H

#include "common/array.h"
#include "common/events.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "engines/engine.h"
#include "common/file.h"

#include "supernova/console.h"
#include "supernova/graphics.h"
#include "supernova/msn_def.h"
#include "supernova/room.h"
#include "supernova/sound.h"
#include "supernova/imageid.h"
#include "supernova/game-manager.h"

namespace Common {
	class MemoryReadWriteStream;
}

namespace Supernova {

#define SAVEGAME_HEADER MKTAG('M','S','N','1')
#define SAVEGAME_HEADER2 MKTAG('M','S','N','2')
#define SAVEGAME_VERSION 10

#define SUPERNOVA_DAT "supernova.dat"
#define SUPERNOVA_DAT_VERSION 3

class GuiElement;
class ResourceManager;
class Sound;
class console;
class GameManager;
class GameManager1;
class Screen;

class SupernovaEngine : public Engine {
public:
	explicit SupernovaEngine(OSystem *syst);
	~SupernovaEngine() override;

	Common::Error run() override;
	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveGameStateCurrently() override;
	bool hasFeature(EngineFeature f) const override;
	void pauseEngineIntern(bool pause) override;

	GameManager *_gm;
	Sound *_sound;
	ResourceManager *_resMan;
	Screen *_screen;
	bool _allowLoadGame;
	bool _allowSaveGame;
	Common::StringArray _gameStrings;
	Common::String _nullString;
	int _sleepAuoSaveVersion;
	Common::MemoryReadWriteStream* _sleepAutoSave;

	uint _delay;
	int  _textSpeed;
	char _MSPart;
	bool _improved;

	Common::Error loadGameStrings();
	void init();
	virtual Common::String getSaveStateName(int slot) const override;
	bool loadGame(int slot);
	bool saveGame(int slot, const Common::String &description);
	bool serialize(Common::WriteStream *out);
	bool deserialize(Common::ReadStream *in, int version);
	bool quitGameDialog();
	void errorTempSave(bool saving);
	void setTextSpeed();
	const Common::String &getGameString(int idx) const;
	void setGameString(int idx, const Common::String &string);
	void showHelpScreen1();
	void showHelpScreen2();
	Common::SeekableReadStream *getBlockFromDatFile(Common::String name);
	Common::Error showTextReader(const char *extension);

	// forwarding calls
	void playSound(AudioId sample);
	void playSound(MusicId index);
	void paletteFadeIn();
	void paletteFadeOut(int minBrightness = 0);
	void paletteBrightness();
	void renderImage(int section);
	void renderImage(ImageId id, bool removeImage = false);
	bool setCurrentImage(int filenumber);
	void saveScreen(int x, int y, int width, int height);
	void saveScreen(const GuiElement &guiElement);
	void restoreScreen();
	void renderRoom(Room &room);
	void renderMessage(const char *text, MessagePosition position = kMessageNormal);
	void renderMessage(const Common::String &text, MessagePosition position = kMessageNormal);
	void renderMessage(int stringId, MessagePosition position = kMessageNormal,
					   Common::String var1 = "", Common::String var2 = "");
	void renderMessage(int stringId, int x, int y);
	void removeMessage();
	void renderText(const uint16 character);
	void renderText(const char *text);
	void renderText(const Common::String &text);
	void renderText(int stringId);
	void renderText(const uint16 character, int x, int y, byte color);
	void renderText(const char *text, int x, int y, byte color);
	void renderText(const Common::String &text, int x, int y, byte color);
	void renderText(int stringId, int x, int y, byte color);
	void renderText(const GuiElement &guiElement);
	void renderBox(int x, int y, int width, int height, byte color);
	void renderBox(const GuiElement &guiElement);
	void setColor63(byte value);
	void stopSound();
};

}

#endif
