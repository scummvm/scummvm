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

#ifndef SUPERNOVA_SUPERNOVA2_H
#define SUPERNOVA_SUPERNOVA2_H

#include "common/array.h"
#include "common/events.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "engines/engine.h"
#include "common/file.h"

#include "supernova2/console.h"
#include "supernova2/graphics.h"
#include "supernova2/ms2_def.h"
#include "supernova2/rooms.h"
#include "supernova2/sound.h"
#include "supernova2/imageid.h"

namespace Common {
	class MemoryReadWriteStream;
}

namespace Supernova2 {

#define SAVEGAME_HEADER MKTAG('M','S','N','2')
#define SAVEGAME_VERSION 9

#define SUPERNOVA2_DAT "supernova2.dat"
#define SUPERNOVA2_DAT_VERSION 1

class GuiElement;
class ResourceManager;
class Sound;
class console;
class GameManager;
class Screen;

class Supernova2Engine : public Engine {
public:
	explicit Supernova2Engine(OSystem *syst);
	~Supernova2Engine();

	virtual Common::Error run();
	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual bool canSaveGameStateCurrently();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void pauseEngineIntern(bool pause);

	GameManager *_gm;
	Console *_console;
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

	Common::Error loadGameStrings();
	void init();
	bool loadGame(int slot);
	bool saveGame(int slot, const Common::String &description);
	bool serialize(Common::WriteStream *out);
	bool deserialize(Common::ReadStream *in, int version);
	bool quitGameDialog();
	void errorTempSave(bool saving);
	void setTextSpeed();
	const Common::String &getGameString(int idx) const;
	void setGameString(int idx, const Common::String &string);

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
	void renderMessage(StringId stringId, MessagePosition position = kMessageNormal,
					   Common::String var1 = "", Common::String var2 = "");
	void renderMessage(StringId stringId, int x, int y);
	void removeMessage();
	void renderText(const uint16 character);
	void renderText(const char *text);
	void renderText(const Common::String &text);
	void renderText(StringId stringId);
	void renderText(const uint16 character, int x, int y, byte color);
	void renderText(const char *text, int x, int y, byte color);
	void renderText(const Common::String &text, int x, int y, byte color);
	void renderText(StringId stringId, int x, int y, byte color);
	void renderText(const GuiElement &guiElement);
	void renderBox(int x, int y, int width, int height, byte color);
	void renderBox(const GuiElement &guiElement);
	void setColor63(byte value);
	void stopSound();
};

}

#endif
