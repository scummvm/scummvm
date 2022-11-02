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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_BURIED_H
#define BURIED_BURIED_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/str-array.h"

#include "engines/engine.h"

class OSystem;

struct ADGameDescription;

namespace Common {
class SeekableReadStream;
class Serializer;
class WinResources;
class WriteStream;
}

namespace Graphics {
struct WinCursorGroup;
}

namespace Buried {

enum {
	GF_TRUECOLOR  = (1 << 1),
	GF_WIN95      = (1 << 2),
	GF_COMPRESSED = (1 << 3),
	GF_TRIAL      = (1 << 4)
};

class BuriedConsole;
struct GlobalFlags;
class GraphicsManager;
struct Location;
class Message;
class SoundManager;
class Window;
class VideoWindow;

class BuriedEngine : public ::Engine {
protected:
	Common::Error run() override;

public:
	BuriedEngine(OSystem *syst, const ADGameDescription *gamedesc);
	virtual ~BuriedEngine();

	// Detection related functions
	const ADGameDescription *_gameDescription;
	bool isDemo() const;
	bool isTrial() const;
	bool isTrueColor() const;
	bool isWin95() const;
	bool isCompressed() const;
	Common::String getEXEName() const;
	Common::String getLibraryName() const;
	Common::Language getLanguage() const;

	bool hasFeature(EngineFeature f) const override;
	void pauseEngineIntern(bool pause) override;

	// Resources
	Common::String getString(uint32 stringID);
	Common::String getFilePath(uint32 stringID);
	Common::String getFilePath(int timeZone, int environment, int fileOffset);
	Graphics::WinCursorGroup *getCursorGroup(uint32 cursorGroupID);
	Common::SeekableReadStream *getBitmapStream(uint32 bitmapID);
	Common::SeekableReadStream *getNavData(uint32 resourceID);
	Common::SeekableReadStream *getSndData(uint32 resourceID);
	Common::SeekableReadStream *getAnimData(uint32 resourceID);
	Common::SeekableReadStream *getAIData(uint32 resourceID);
	Common::SeekableReadStream *getItemData(uint32 resourceID);
	Common::SeekableReadStream *getBookData(uint32 resourceID);
	Common::SeekableReadStream *getFileBCData(uint32 resourceID);
	Common::SeekableReadStream *getINNData(uint32 resourceID);
	uint32 getVersion();
	uint32 computeNavDBResourceID(int timeZone, int environment);
	uint32 computeAnimDBResourceID(int timeZone, int environment);
	uint32 computeAIDBResourceID(int timeZone, int environment);
	uint32 computeFileNameResourceID(int timeZone, int environment, int fileOffset);

	GraphicsManager *_gfx;
	SoundManager *_sound;
	Window *_mainWindow; // Only one main window is supported.
	Window *_focusedWindow;
	Window *_captureWindow;

	// Timers
	uint createTimer(Window *window, uint period);
	bool killTimer(uint timer);
	void removeAllTimers(Window *window);

	// Video
	void addVideo(VideoWindow *window);
	void removeVideo(VideoWindow *window);
	void updateVideos();

	// Messaging
	void postMessageToWindow(Window *dest, Message *message);
	void sendAllMessages();
	void processAudioVideoSkipMessages(VideoWindow *video, int soundId);
	void removeKeyboardMessages(Window *window);
	void removeMouseMessages(Window *window);
	void removeAllMessages(Window *window);
	void removeMessages(Window *window, int messageBegin, int messageEnd);
	bool hasMessage(Window *window, int messageBegin, int messageEnd) const;

	// Miscellaneous
	void yield(VideoWindow *video, int soundId);
	int getTransitionSpeed();
	void setTransitionSpeed(int newSpeed);
	void releaseCapture() { _captureWindow = 0; }
	bool runQuitDialog();
	bool isControlDown() const;
	void pauseGame();
	void showPoints();

	// Save/Load
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::String getSaveStateName(int slot) const override {
		return Common::String::format("buried.%03d", slot);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void handleSaveDialog();
	void handleRestoreDialog();

private:
	Common::WinResources *_mainEXE, *_library;

	struct Timer {
		Window *owner;
		uint32 period;
		uint32 nextTrigger;
	};

	typedef Common::HashMap<uint, Timer> TimerMap;
	TimerMap _timers;
	uint _timerSeed;
	uint32 _pauseStartTime;

	typedef Common::List<VideoWindow *> VideoList;
	VideoList _videos;

	bool _yielding;
	bool _allowVideoSkip;

	struct MessageInfo { // I did think about calling this "Envelope"
		Window *dest;
		Message *message;
	};

	// LordHoto didn't want me to add an iterator to Common::Queue.
	typedef Common::List<MessageInfo> MessageQueue;
	MessageQueue _messageQueue;
	void pollForEvents();

	// Saves
	bool syncLocation(Common::Serializer &s, Location &location);
	bool syncGlobalFlags(Common::Serializer &s, GlobalFlags &flags);
	Common::Error syncSaveData(Common::Serializer &ser);
	Common::Error syncSaveData(Common::Serializer &ser, Location &location, GlobalFlags &flags, Common::Array<int> &inventoryItems);
	void checkForOriginalSavedGames();
	void convertSavedGame(Common::String oldFile, Common::String newFile);
};

// Macro for creating a version field
#define MAKEVERSION(a, b, c, d) \
	(((uint32)((a) & 0xFF) << 24) | ((uint32)((b) & 0xFF) << 16) | ((uint32)((c) & 0xFF) << 8) | ((uint32)((d) & 0xFF)))

} // End of namespace Buried

#endif
